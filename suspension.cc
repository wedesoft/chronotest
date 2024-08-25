#include <cmath>
#include <cstdio>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono/core/ChQuaternion.h>
#include <chrono/physics/ChBody.h>
#include <chrono/physics/ChLinkTSDA.h>
#include <chrono/physics/ChLinkLock.h>
#include <chrono/physics/ChSystemNSC.h>

int width = 1280;
int height = 720;

const char *vertexSource = "#version 410 core\n\
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

const char *fragmentSource = "#version 410 core\n\
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
GLfloat vertices[] = {
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

unsigned int indices[] = {
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
  GLFWwindow *window = glfwCreateWindow(width, height, "Spring-damper system with Chrono", NULL, NULL);
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
                        6 * sizeof(float), (void *)0);
  glVertexAttribPointer(glGetAttribLocation(program, "normal"),
                        3, GL_FLOAT, GL_FALSE,
                        6 * sizeof(float), (void *)(3 * sizeof(float)));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glDisable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  float light[3] = {0.36f, 0.8f, -0.48f};
  glUniform3fv(glGetUniformLocation(program, "light"), 1, light);
  glUniform1f(glGetUniformLocation(program, "aspect"), (float)width / (float)height);
  float a = 0.1;
  float b = 0.1;
  float c = 0.1;
  float axes[3] = {a, b, c};
  glUniform3fv(glGetUniformLocation(program, "axes"), 1, axes);

  chrono::ChSystemNSC sys;
  sys.SetCollisionSystemType(chrono::ChCollisionSystem::Type::BULLET);
  sys.SetTimestepperType(chrono::ChTimestepper::Type::EULER_IMPLICIT_PROJECTED);
  sys.SetSolverType(chrono::ChSolver::Type::PSOR);
  sys.GetSolver()->AsIterative()->SetMaxIterations(100);
  sys.SetGravitationalAcceleration(chrono::ChVector3(0.0, -0.4, 0.0));

  // https://math.stackexchange.com/questions/4501028/calculating-moment-of-inertia-for-a-cuboid

  auto material = chrono_types::make_shared<chrono::ChContactMaterialNSC>();
  material->SetStaticFriction(0.9f);
  material->SetSlidingFriction(0.5f);
  material->SetRestitution(0.3f);

  float upper_mass = 1000.0;
  float mass = 10.0;

  auto upper = chrono_types::make_shared<chrono::ChBody>();
  upper->SetMass(upper_mass);
  upper->SetInertiaXX(chrono::ChVector3(upper_mass * (b * b + c * c) / 12.0,
                                        upper_mass * (a * a + c * c) / 12.0,
                                        upper_mass * (a * a + b * b) / 12.0));
  upper->SetPos(chrono::ChVector3(0.0, 0.6, 0.0));
  sys.AddBody(upper);

  auto lower = chrono_types::make_shared<chrono::ChBody>();
  lower->SetMass(mass);
  lower->SetInertiaXX(chrono::ChVector3(mass * (b * b + c * c) / 12.0,
                                        mass * (a * a + c * c) / 12.0,
                                        mass * (a * a + b * b) / 12.0));
  lower->SetPos(chrono::ChVector3(0.0, 0.3, 0.0));
  sys.AddBody(lower);

  auto coll_model = chrono_types::make_shared<chrono::ChCollisionModel>();
  coll_model->SetSafeMargin(0.1f);
  coll_model->SetEnvelope(0.001f);
  auto shape = chrono_types::make_shared<chrono::ChCollisionShapeBox>(material, a, b, c);
  coll_model->AddShape(shape);
  lower->AddCollisionModel(coll_model);
  lower->EnableCollision(true);

  auto link = chrono_types::make_shared<chrono::ChLinkTSDA>();
  link->Initialize(upper, lower, false, upper->GetPos(), lower->GetPos());
  link->SetSpringCoefficient(10000.0f);
  link->SetDampingCoefficient(1000.0f);
  sys.AddLink(link);

  auto prismatic = chrono_types::make_shared<chrono::ChLinkLockPrismatic>();
  prismatic->Initialize(upper, lower, chrono::ChFrame<>(upper->GetPos(), chrono::QuatFromAngleX(-chrono::CH_PI_2)));
  sys.AddLink(prismatic);

  auto ground = chrono_types::make_shared<chrono::ChBody>();
  ground->SetFixed(true);
  ground->SetMass(1e+6);
  ground->SetInertiaXX(chrono::ChVector3(1e+5, 1e+5, 1e+5));
  ground->SetPos(chrono::ChVector3(0.0, -0.5, 0.0));
  sys.AddBody(ground);

  auto coll_model_ground = chrono_types::make_shared<chrono::ChCollisionModel>();
  coll_model_ground->SetSafeMargin(0.1f);
  coll_model_ground->SetEnvelope(0.001f);
  auto shape_ground = chrono_types::make_shared<chrono::ChCollisionShapeBox>(material, 2.0, 0.2, 2.0);
  coll_model_ground->AddShape(shape_ground);
  ground->AddCollisionModel(coll_model_ground);
  ground->EnableCollision(true);

  double t = glfwGetTime();
  while (!glfwWindowShouldClose(window)) {
    double dt = glfwGetTime() - t;

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    for (auto body=sys.GetBodies().begin(); body!=sys.GetBodies().end(); body++) {
      if ((*body)->IsFixed()) continue;
      chrono::ChQuaternion quat = (*body)->GetRot();
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

      chrono::ChVector3 position = (*body)->GetPos();
      float translation[3] = {(float)position.x(), (float)position.y(), (float)position.z()};
      glUniform3fv(glGetUniformLocation(program, "translation"), 1, translation);

      glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, (void *)0);
    };

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
