#include "Krystal.Gfx.OpenGL/OpenGLContext.hpp"

#include "Krystal.Core/Core.hpp"
#include "Krystal.Core/Detection.hpp"
#include "Krystal.Gfx/IContext.hpp"

#ifdef KRYS_PLATFORM_WINDOWS
  #include "Krystal.Gfx.OpenGL/Win32/GLContextPlatformImpl.hpp"
#else
  #error "Unsupported platform for OpenGL context creation."
#endif

#include "Krystal.Gfx.OpenGL/Hooks/gl.hpp"

namespace
{
  static float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};

  static unsigned int VAO;
  static unsigned int VBO;

  static unsigned int vertexShader;
  static unsigned int fragmentShader;
  static unsigned int shaderProgram;

  static const char *vertexShaderSource = "#version 330 core\n"
                                          "layout (location = 0) in vec3 aPos;\n"
                                          "void main()\n"
                                          "{\n"
                                          "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                          "}\0";
  static const char *fragmentShaderSource = "#version 330 core\n"
                                            "out vec4 FragColor;\n"
                                            "void main()\n"
                                            "{\n"
                                            "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                                            "}\n\0";
}

namespace Krys::Gfx
{
  Expected<Unique<IContext>> CreateContext(NativeHandle windowHandle, uint32 width, uint32 height) noexcept
  {
    try
    {
      return Expected<Unique<IContext>>(CreateUnique<OpenGL::OpenGLContext>(windowHandle, width, height));
    }
    catch (const std::exception &e)
    {
      return Unexpected(e.what());
    }
  }
}

namespace Krys::Gfx::OpenGL
{
  OpenGLContext::OpenGLContext(NativeHandle windowHandle, uint32 width, uint32 height)
      : _windowHandle(windowHandle), _platformImpl(CreateUnique<GLContextPlatformImpl>(windowHandle))
  {
  }

  void OpenGLContext::Setup() noexcept
  {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
  }

  void OpenGLContext::Render() noexcept
  {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
  }

  void OpenGLContext::Present() noexcept
  {
    _platformImpl->Present();
  }

  void OpenGLContext::Resize(uint32 width, uint32 height) noexcept
  {
    glViewport(0, 0, width, height);
  }
}
