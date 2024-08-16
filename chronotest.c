#include <GL/glew.h>
#include <GLFW/glfw3.h>

int width = 640;
int height = 480;

int main(void)
{
  glfwInit();
  GLFWwindow *window = glfwCreateWindow(width, height, "Tumbling motion with Project Chrono", NULL, NULL);
  glfwMakeContextCurrent(window);
  glewInit();

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glfwSwapBuffers(window);
    glfwPollEvents();
  };

  glfwTerminate();
  return 0;
}
