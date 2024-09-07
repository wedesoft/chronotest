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

const char *vertex_wheel = "#version 410 core\n\
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

const char *fragment_wheel = "#version 410 core\n\
in vec3 color;\n\
out vec3 fragColor;\n\
void main()\n\
{\n\
  fragColor = color;\n\
  fragColor = vec3(1, 1, 1);\n\
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

GLfloat vertices_wheel[] = {
   0.0f,  0.0f,  0.0f
};

unsigned int indices_wheel[] = {
   0
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
  GLFWwindow *window = glfwCreateWindow(width, height, "Vehicle with gears with Project Chrono", NULL, NULL);
  glfwMakeContextCurrent(window);
  glewInit();

  glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
  glViewport(0, 0, width, height);

  // Cuboid shaders
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

  // Wheel shaders
  GLuint vertex_shader_wheel = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader_wheel, 1, &vertex_wheel, NULL);
  glCompileShader(vertex_shader_wheel);
  handleCompileError("Vertex shader", vertex_shader_wheel);

  GLuint fragment_shader_wheel = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader_wheel, 1, &fragment_wheel, NULL);
  glCompileShader(fragment_shader_wheel);
  handleCompileError("Fragment shader", fragment_shader_wheel);

  GLuint program_wheel = glCreateProgram();
  glAttachShader(program_wheel, vertex_shader_wheel);
  glAttachShader(program_wheel, fragment_shader_wheel);
  glLinkProgram(program_wheel);
  handleLinkError("Shader program", program_wheel);

  GLuint vao_cuboid;
  GLuint vbo_cuboid;
  GLuint idx_cuboid;

  glGenVertexArrays(1, &vao_cuboid);
  glBindVertexArray(vao_cuboid);

  glGenBuffers(1, &vbo_cuboid);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_cuboid);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_cuboid), vertices_cuboid, GL_STATIC_DRAW);
  glGenBuffers(1, &idx_cuboid);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_cuboid);
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

  float light[3] = {0.36f, 0.8f, -0.48f};
  glUniform3fv(glGetUniformLocation(program_cuboid, "light"), 1, light);
  glUniform1f(glGetUniformLocation(program_cuboid, "aspect"), (float)width / (float)height);
  float a = 0.3;
  float b = 0.06;
  float c = 0.08;
  float axes[3] = {a, b, c};
  glUniform3fv(glGetUniformLocation(program_cuboid, "axes"), 1, axes);

  GLuint vao_wheel;
  GLuint vbo_wheel;
  GLuint idx_wheel;

  glGenVertexArrays(1, &vao_wheel);
  glBindVertexArray(vao_wheel);

  glGenBuffers(1, &vbo_wheel);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_wheel);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_wheel), vertices_wheel, GL_STATIC_DRAW);
  glGenBuffers(1, &idx_wheel);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_wheel);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_wheel), indices_wheel, GL_STATIC_DRAW);

  glUseProgram(program_wheel);

  glVertexAttribPointer(glGetAttribLocation(program_wheel, "point"),
                        3, GL_FLOAT, GL_FALSE,
                        3 * sizeof(float), (void *)0);

  glEnableVertexAttribArray(0);

  float radius = 0.03;
  float length = 0.01;
  int num_points = 18;
  glUniform1f(glGetUniformLocation(program_wheel, "aspect"), (float)width / (float)height);
  glUniform1f(glGetUniformLocation(program_wheel, "radius"), radius);
  glUniform1i(glGetUniformLocation(program_wheel, "num_points"), num_points);

  glDisable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glPointSize(2.0f);

  chrono::ChSystemNSC sys;
  sys.SetTimestepperType(chrono::ChTimestepper::Type::RUNGEKUTTA45);
  sys.SetGravitationalAcceleration(chrono::ChVector3(0.0, -0.25, 0.0));
  sys.SetCollisionSystemType(chrono::ChCollisionSystem::Type::BULLET);
  sys.SetTimestepperType(chrono::ChTimestepper::Type::EULER_IMPLICIT_PROJECTED);
  sys.SetSolverType(chrono::ChSolver::Type::PSOR);
  sys.GetSolver()->AsIterative()->SetMaxIterations(10);

  auto material = chrono_types::make_shared<chrono::ChContactMaterialNSC>();
  material->SetStaticFriction(0.9f);
  material->SetSlidingFriction(0.5f);
  material->SetRestitution(0.3f);

  float margin = 0.0001f;
  float envelope = 0.0001f;

  auto ground = chrono_types::make_shared<chrono::ChBody>();
  ground->SetFixed(true);
  ground->SetMass(1e+6);
  ground->SetInertiaXX(chrono::ChVector3(1e+5, 1e+5, 1e+5));
  ground->SetPos(chrono::ChVector3(0.0, -0.4, 0.0));
  sys.AddBody(ground);

  auto coll_model_ground = chrono_types::make_shared<chrono::ChCollisionModel>();
  coll_model_ground->SetSafeMargin(margin);
  coll_model_ground->SetEnvelope(envelope);
  auto shape_ground = chrono_types::make_shared<chrono::ChCollisionShapeBox>(material, 200.0, 0.3, 2.0);
  coll_model_ground->AddShape(shape_ground);
  ground->AddCollisionModel(coll_model_ground);
  ground->EnableCollision(true);

  // https://math.stackexchange.com/questions/4501028/calculating-moment-of-inertia-for-a-cuboid
  auto body = chrono_types::make_shared<chrono::ChBody>();
  body->SetName("cuboid");
  float mass = 10.0;
  body->SetMass(mass);
  body->SetInertiaXX(chrono::ChVector3(mass * (b * b + c * c) / 12.0,
                                       mass * (a * a + c * c) / 12.0,
                                       mass * (a * a + b * b) / 12.0));
  body->SetPos(chrono::ChVector3(0.0, 0.0, 0.0));
  body->SetPosDt(chrono::ChVector3(0.0, 0.0, 0.0));
  body->SetAngVelLocal(chrono::ChVector3(0.0, 0.0, 0.0));
  sys.AddBody(body);

  std::vector<std::shared_ptr<chrono::ChBody>> wheels;

  float mass_wheel = 0.2;
  for (int i=0; i<4; i++) {
    auto wheel = chrono_types::make_shared<chrono::ChBody>();
    double mass = mass_wheel;
    wheel->SetName("wheel");
    wheel->SetMass(mass);
    wheel->SetInertiaXX(chrono::ChVector3d(0.25 * mass * radius * radius + 1.0 / 12.0 * mass * length * length,
                                           0.25 * mass * radius * radius + 1.0 / 12.0 * mass * length * length,
                                           0.5 * mass * radius * radius));
    int x = i / 2;
    int z = i % 2;
    wheel->SetPos(chrono::ChVector3d(x * a - 0.5 * a, - b - radius, z * c - 0.5 * c));
    wheel->SetPosDt(chrono::ChVector3(0.0, 0.0, 0.0));
    wheel->SetAngVelLocal(chrono::ChVector3(0.0, 0.0, 0.0));
    sys.AddBody(wheel);
    wheels.push_back(wheel);

    auto coll_model_wheel = chrono_types::make_shared<chrono::ChCollisionModel>();
    coll_model_wheel->SetSafeMargin(margin);
    coll_model_wheel->SetEnvelope(envelope);
    auto shape_wheel = chrono_types::make_shared<chrono::ChCollisionShapeCylinder>(material, radius, length);
    coll_model_wheel->AddShape(shape_wheel);
    wheel->AddCollisionModel(coll_model_wheel);
    wheel->EnableCollision(true);

    auto prismatic = chrono_types::make_shared<chrono::ChLinkLockPrismatic>();
    prismatic->Initialize(body, wheel, chrono::ChFrame<>(body->GetPos(), chrono::QuatFromAngleX(-chrono::CH_PI_2)));
    sys.AddLink(prismatic);

    auto link = chrono_types::make_shared<chrono::ChLinkTSDA>();
    link->Initialize(body, wheel, false, wheel->GetPos() + chrono::ChVector3d(0.0, b + radius, 0.0), wheel->GetPos());
    link->SetSpringCoefficient(80.0f);
    link->SetDampingCoefficient(8.0f);
    sys.AddLink(link);
  }

  double t = glfwGetTime();
  while (!glfwWindowShouldClose(window)) {
    double dt = glfwGetTime() - t;

    glUseProgram(program_cuboid);
    glBindVertexArray(vao_cuboid);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cuboid);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_cuboid);

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

    glUseProgram(program_wheel);
    glBindVertexArray(vao_wheel);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_wheel);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_wheel);

    for (int i=0; i<4; i++) {
      auto wheel = wheels[i];
      chrono::ChQuaternion quat = wheel->GetRot();
      chrono::ChMatrix33 mat(quat);
      chrono::ChVector3 x = mat.GetAxisX();
      chrono::ChVector3 y = mat.GetAxisY();
      chrono::ChVector3 z = mat.GetAxisZ();

      float rotation[9] = {
        (float)x.x(), (float)y.x(), (float)z.x(),
        (float)x.y(), (float)y.y(), (float)z.y(),
        (float)x.z(), (float)y.z(), (float)z.z()
      };

      glUniformMatrix3fv(glGetUniformLocation(program_wheel, "rotation"), 1, GL_TRUE, rotation);

      chrono::ChVector3 position = wheel->GetPos();
      float translation[3] = {(float)position.x(), (float)position.y(), (float)position.z()};
      glUniform3fv(glGetUniformLocation(program_wheel, "translation"), 1, translation);

      glDrawElementsInstanced(GL_POINTS, 1, GL_UNSIGNED_INT, (void *)0, num_points);
    };

    glfwSwapBuffers(window);
    glfwPollEvents();
    sys.DoStepDynamics(dt);
    t += dt;
  };

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glDeleteBuffers(1, &idx_cuboid);
  glDeleteBuffers(1, &vbo_cuboid);
  glDeleteVertexArrays(1, &vao_cuboid);

  glDeleteBuffers(1, &idx_wheel);
  glDeleteBuffers(1, &vbo_wheel);
  glDeleteVertexArrays(1, &vao_wheel);

  glDeleteProgram(program_wheel);
  glDeleteShader(vertex_shader_wheel);
  glDeleteShader(fragment_shader_wheel);

  glDeleteProgram(program_cuboid);
  glDeleteShader(vertex_shader_cuboid);
  glDeleteShader(fragment_shader_cuboid);

  glfwTerminate();
  return 0;
}
