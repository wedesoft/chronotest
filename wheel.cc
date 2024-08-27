#include <iostream>
#include <cmath>
#include <cstdio>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono/physics/ChBody.h>
#include <chrono/physics/ChSystemNSC.h>
#include <chrono/physics/ChLoadsBody.h>
#include <chrono/physics/ChLoadContainer.h>

int width = 1280;
int height = 720;

const char *vertexSource = "#version 410 core\n\
uniform float aspect;\n\
uniform float radius;\n\
uniform int num_points;\n\
uniform vec3 translation;\n\
uniform mat3 rotation;\n\
in vec3 point;\n\
out vec3 color;\n\
void main()\n\
{\n\
  vec3 radius_vector = radius * vec3(cos(2.0 * 3.1415926 * gl_InstanceID / num_points), sin(2.0 * 3.1415926 * gl_InstanceID / num_points), 0);\n\
  if (gl_InstanceID == 0)\n\
    color = vec3(1, 0, 0);\n\
  else\n\
    color = vec3(1, 1, 1);\n\
  gl_Position = vec4((rotation * (point + radius_vector) + translation) * vec3(1, aspect, 1), 1);\n\
}";

const char *fragmentSource = "#version 410 core\n\
in vec3 color;\n\
out vec3 fragColor;\n\
void main()\n\
{\n\
  fragColor = color;\n\
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

  float mass = 0.5;
  float radius = 0.1;
  float length = 0.2;
  int num_points = 18;

  glPointSize(2.0f);

  glUniform1f(glGetUniformLocation(program, "aspect"), (float)width / (float)height);
  glUniform1f(glGetUniformLocation(program, "radius"), radius);
  glUniform1i(glGetUniformLocation(program, "num_points"), num_points);

  chrono::ChSystemNSC sys;
  sys.SetGravitationalAcceleration(chrono::ChVector3(0.0, -0.8, 0.0));
  sys.SetCollisionSystemType(chrono::ChCollisionSystem::Type::BULLET);
  sys.SetTimestepperType(chrono::ChTimestepper::Type::EULER_IMPLICIT_PROJECTED);
  sys.SetSolverType(chrono::ChSolver::Type::PSOR);
  sys.GetSolver()->AsIterative()->SetMaxIterations(100);

  auto material = chrono_types::make_shared<chrono::ChContactMaterialNSC>();
  material->SetStaticFriction(0.9f);
  material->SetSlidingFriction(0.5f);
  material->SetRestitution(0.3f);

  auto body = chrono_types::make_shared<chrono::ChBody>();
  body->SetMass(mass);
  body->SetInertiaXX(chrono::ChVector3d(0.5 * mass * radius * radius + 1.0 / 12 * mass * length * length,
                                        0.5 * mass * radius * radius + 1.0 / 12 * mass * length * length,
                                        0.5 * mass * radius * radius));
  body->SetPos(chrono::ChVector3(0.0, 0.2, 0.0));
  body->SetPosDt(chrono::ChVector3(5.0, 0.0, 0.0));
  sys.AddBody(body);

  auto coll_model_body = chrono_types::make_shared<chrono::ChCollisionModel>();
  coll_model_body->SetSafeMargin(0.1f);
  coll_model_body->SetEnvelope(0.001f);
  auto shape_body = chrono_types::make_shared<chrono::ChCollisionShapeCylinder>(material, radius, length);
  coll_model_body->AddShape(shape_body);
  body->AddCollisionModel(coll_model_body);
  body->EnableCollision(true);

  auto ground = chrono_types::make_shared<chrono::ChBody>();
  ground->SetFixed(true);
  ground->SetMass(1e+6);
  ground->SetInertiaXX(chrono::ChVector3(1e+5, 1e+5, 1e+5));
  ground->SetPos(chrono::ChVector3(0.0, -0.5, 0.0));
  sys.AddBody(ground);

  auto coll_model_ground = chrono_types::make_shared<chrono::ChCollisionModel>();
  coll_model_ground->SetSafeMargin(0.1f);
  coll_model_ground->SetEnvelope(0.001f);
  auto shape_ground = chrono_types::make_shared<chrono::ChCollisionShapeBox>(material, 200.0, 0.2, 2.0);
  coll_model_ground->AddShape(shape_ground);
  ground->AddCollisionModel(coll_model_ground);
  ground->EnableCollision(true);

  double t = glfwGetTime();
  while (!glfwWindowShouldClose(window)) {
    double dt = glfwGetTime() - t;

    glClear(GL_COLOR_BUFFER_BIT);

    chrono::ChQuaternion quat = body->GetRot();
    chrono::ChMatrix33 mat(quat);
    chrono::ChVector3 x = mat.GetAxisX();
    chrono::ChVector3 y = mat.GetAxisY();
    chrono::ChVector3 z = mat.GetAxisZ();

    float rotation[9] = {
      (float)x.x(), (float)y.x(), (float)z.x(),
      (float)x.y(), (float)y.y(), (float)z.y(),
      (float)x.z(), (float)y.z(), (float)z.z()
    };

    glUniformMatrix3fv(glGetUniformLocation(program, "rotation"), 1, GL_TRUE, rotation);

    chrono::ChVector3 position = body->GetPos();
    double px = position.x();
    while (px >= 1.0)
      px -= 2.0;
    float translation[3] = {(float)px, (float)position.y(), (float)position.z()};
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
