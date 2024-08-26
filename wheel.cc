#include <iostream>
#include <cmath>
#include <cstdio>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono/physics/ChBody.h>
#include <chrono/physics/ChSystemNSC.h>
#include <chrono/physics/ChLoadsBody.h>
#include <chrono/physics/ChLoadContainer.h>

int width = 640;
int height = 480;

const char *vertexSource = "#version 410 core\n\
uniform float aspect;\n\
uniform float radius;\n\
uniform int num_points;\n\
uniform vec3 translation;\n\
in vec3 point;\n\
void main()\n\
{\n\
  vec3 radius_vector = radius * vec3(cos(2.0 * 3.1415926 * gl_InstanceID / num_points), sin(2.0 * 3.1415926 * gl_InstanceID / num_points), 0);\n\
  gl_Position = vec4((point + translation + radius_vector) * vec3(1, aspect, 1), 1);\n\
}";

const char *fragmentSource = "#version 410 core\n\
out vec3 fragColor;\n\
void main()\n\
{\n\
  fragColor = vec3(1, 1, 1);\n\
}";

GLfloat vertices[] = {
   0.0f,  0.0f,  0.0f
};

unsigned int indices[] = {
   0
};

void handleCompileError(const char *step, GLuint shader)
{
  GLint result = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
    char buffer[1024];
    glGetShaderInfoLog(shader, 1024, NULL, buffer);
    if (buffer[0])
      fprintf(stderr, "%s: %s\n", step, buffer);
  };
}

void handleLinkError(const char *step, GLuint program)
{
  GLint result = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &result);
  if (result == GL_FALSE) {
    char buffer[1024];
    glGetProgramInfoLog(program, 1024, NULL, buffer);
    if (buffer[0])
      fprintf(stderr, "%s: %s\n", step, buffer);
  };
}

class ChLoadGravity: public chrono::ChLoadBodyBody
{
  public:
    ChLoadGravity(std::shared_ptr<chrono::ChBody> bodyA, std::shared_ptr<chrono::ChBody> bodyB):
      chrono::ChLoadBodyBody(bodyA, bodyB, chrono::ChFrame<>()) {}
    virtual ChObj *Clone() const { return new ChLoadGravity(*this); }
  protected:
    virtual void ComputeBodyBodyForceTorque(const chrono::ChFrameMoving<>& rel_AB, chrono::ChVector3d& loc_force, chrono::ChVector3d& loc_torque) override
    {
      chrono::ChVector3 position = GetBodyA()->GetPos();
      double dist = position.Length();
      double gravity = 0.05 * GetBodyA()->GetMass() / (dist * dist);
      loc_force = position * gravity / dist;
      loc_torque = chrono::VNULL;
    }
    virtual bool IsStiff(void) {return false; }
};

int main(void)
{
  glfwInit();
  glfwWindowHint(GLFW_DEPTH_BITS, 0);
  GLFWwindow *window = glfwCreateWindow(width, height, "Orbiting mass with Project Chrono", NULL, NULL);
  glfwMakeContextCurrent(window);
  glewInit();

  glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
  glViewport(0, 0, width, height);

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSource, NULL);
  glCompileShader(vertexShader);
  handleCompileError("Vertex shader", vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShader);
  handleCompileError("Fragment shader", fragmentShader);

  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);
  handleLinkError("Shader program", program);

  GLuint vao;
  GLuint vbo;
  GLuint idx;

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glGenBuffers(1, &idx);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glUseProgram(program);

  glVertexAttribPointer(glGetAttribLocation(program, "point"),
                        3, GL_FLOAT, GL_FALSE,
                        3 * sizeof(float), (void *)0);

  glEnableVertexAttribArray(0);

  float radius = 0.1;
  int num_points = 18;

  glPointSize(1.0f);

  glUniform1f(glGetUniformLocation(program, "aspect"), (float)width / (float)height);
  glUniform1f(glGetUniformLocation(program, "radius"), radius);
  glUniform1i(glGetUniformLocation(program, "num_points"), num_points);

  chrono::ChSystemNSC sys;
  sys.SetGravitationalAcceleration(chrono::ChVector3(0.0, 0.0, 0.0));
  sys.SetTimestepperType(chrono::ChTimestepper::Type::RUNGEKUTTA45);

  auto body = chrono_types::make_shared<chrono::ChBody>();
  body->SetName("particle");
  body->SetMass(10.0);
  body->SetInertiaXX(chrono::ChVector3(1.0f, 1.0f, 1.0f));
  body->SetPos(chrono::ChVector3(0.5, 0.0, 0.0));
  body->SetFixed(false);
  sys.AddBody(body);

  double t = glfwGetTime();
  while (!glfwWindowShouldClose(window)) {
    double dt = glfwGetTime() - t;

    glClear(GL_COLOR_BUFFER_BIT);

    chrono::ChVector3 position = body->GetPos();
    float translation[3] = {(float)position.x(), (float)position.y(), (float)position.z()};
    glUniform3fv(glGetUniformLocation(program, "translation"), 1, translation);

    glDrawElementsInstanced(GL_POINTS, 1, GL_UNSIGNED_INT, (void *)0, num_points);
    glfwSwapBuffers(window);
    glfwPollEvents();
    sys.DoStepDynamics(dt);
    t += dt;
  };

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &idx);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &vbo);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &vao);

  glDeleteProgram(program);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  glfwTerminate();
  return 0;
}
