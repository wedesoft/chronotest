#include <cmath>
#include <cstdio>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono/core/ChQuaternion.h>
#include <chrono/physics/ChBody.h>
#include <chrono/physics/ChSystemNSC.h>

int width = 1280;
int height = 720;

const char *vertex_cuboid = "#version 410 core\n\
uniform float aspect;\n\
uniform vec3 axes;\n\
uniform vec3 translation;\n\
uniform mat3 rotation;\n\
in vec3 point;\n\
in vec3 normal;\n\
out vec3 n;\n\
void main()\n\
{\n\
  n = rotation * normal;\n\
  gl_Position = vec4((rotation * (point * axes) + translation) * vec3(1, aspect, 1), 1);\n\
}";

const char *fragment_cuboid = "#version 410 core\n\
uniform vec3 light;\n\
in vec3 n;\n\
out vec3 fragColor;\n\
void main()\n\
{\n\
  float ambient = 0.3;\n\
  float diffuse = 0.7 * max(dot(light, n), 0);\n\
  fragColor = vec3(1, 1, 1) * (ambient + diffuse);\n\
}";

// Vertex array data
GLfloat vertices_cuboid[] = {
  // Front face
  -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
   0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
   0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
  -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

  // Back face
  -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
   0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
   0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
  -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

  // Left face
  -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
  -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
  -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
  -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,

  // Right face
   0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
   0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
   0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
   0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,

  // Top face
  -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
   0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
   0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
  -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,

  // Bottom face
  -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
   0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
   0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
  -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f
};

unsigned int indices_cuboid[] = {
   0,  1,  2,  3,
   4,  5,  6,  7,
   8,  9, 10, 11,
  12, 13, 14, 15,
  16, 17, 18, 19,
  20, 21, 22, 23
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
  GLFWwindow *window = glfwCreateWindow(width, height, "Tumbling motion with Project Chrono", NULL, NULL);
  glfwMakeContextCurrent(window);
  glewInit();

  glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
  glViewport(0, 0, width, height);

  GLuint vertex_shader_cuboid = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader_cuboid, 1, &vertex_cuboid, NULL);
  glCompileShader(vertex_shader_cuboid);
  handleCompileError("Vertex shader", vertex_shader_cuboid);

  GLuint fragment_shader_cuboid = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader_cuboid, 1, &fragment_cuboid, NULL);
  glCompileShader(fragment_shader_cuboid);
  handleCompileError("Fragment shader", fragment_shader_cuboid);

  GLuint program_cuboid = glCreateProgram();
  glAttachShader(program_cuboid, vertex_shader_cuboid);
  glAttachShader(program_cuboid, fragment_shader_cuboid);
  glLinkProgram(program_cuboid);
  handleLinkError("Shader program", program_cuboid);

  GLuint vao;
  GLuint vbo;
  GLuint idx;

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_cuboid), vertices_cuboid, GL_STATIC_DRAW);
  glGenBuffers(1, &idx);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_cuboid), indices_cuboid, GL_STATIC_DRAW);

  glUseProgram(program_cuboid);

  glVertexAttribPointer(glGetAttribLocation(program_cuboid, "point"),
                        3, GL_FLOAT, GL_FALSE,
                        6 * sizeof(float), (void *)0);
  glVertexAttribPointer(glGetAttribLocation(program_cuboid, "normal"),
                        3, GL_FLOAT, GL_FALSE,
                        6 * sizeof(float), (void *)(3 * sizeof(float)));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glDisable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  float light[3] = {0.36f, 0.8f, -0.48f};
  glUniform3fv(glGetUniformLocation(program_cuboid, "light"), 1, light);
  glUniform1f(glGetUniformLocation(program_cuboid, "aspect"), (float)width / (float)height);
  float a = 0.3;
  float b = 0.03;
  float c = 0.08;
  float axes[3] = {a, b, c};
  glUniform3fv(glGetUniformLocation(program_cuboid, "axes"), 1, axes);

  chrono::ChSystemNSC sys;
  sys.SetTimestepperType(chrono::ChTimestepper::Type::RUNGEKUTTA45);
  sys.SetGravitationalAcceleration(chrono::ChVector3(0.0, 0.0, 0.0));

  // https://math.stackexchange.com/questions/4501028/calculating-moment-of-inertia-for-a-cuboid
  auto body = chrono_types::make_shared<chrono::ChBody>();
  body->SetName("Cuboid");
  float mass = 10.0;
  body->SetMass(mass);
  body->SetInertiaXX(chrono::ChVector3(mass * (b * b + c * c) / 12.0,
                                       mass * (a * a + c * c) / 12.0,
                                       mass * (a * a + b * b) / 12.0));
  body->SetPos(chrono::ChVector3(0.0, 0.0, 0.0));
  body->SetPosDt(chrono::ChVector3(0.0, 0.0, 0.0));
  body->SetAngVelLocal(chrono::ChVector3(0.0, 0.0, 0.0));
  sys.AddBody(body);

  double t = glfwGetTime();
  while (!glfwWindowShouldClose(window)) {
    double dt = glfwGetTime() - t;

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

    glUniformMatrix3fv(glGetUniformLocation(program_cuboid, "rotation"), 1, GL_TRUE, rotation);

    chrono::ChVector3 position = body->GetPos();
    float translation[3] = {(float)position.x(), (float)position.y(), (float)position.z()};
    glUniform3fv(glGetUniformLocation(program_cuboid, "translation"), 1, translation);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, (void *)0);
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

  glDeleteProgram(program_cuboid);
  glDeleteShader(vertex_shader_cuboid);
  glDeleteShader(fragment_shader_cuboid);

  glfwTerminate();
  return 0;
}
