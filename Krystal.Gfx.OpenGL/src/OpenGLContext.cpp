#include "Krystal.Gfx.OpenGL/OpenGLContext.hpp"

#include "Krystal.Core/Core.hpp"
#include "Krystal.Core/Detection.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.IO/Streams/NativeFileStream.hpp"
#include "Krystal.IO/Streams/StreamUtils.hpp"

#ifdef KRYS_PLATFORM_WINDOWS
  #include "Krystal.Gfx.OpenGL/Win32/GLContextPlatformImpl.hpp"
#else
  #error "Unsupported platform for OpenGL context creation."
#endif

#include "Krystal.Gfx.OpenGL/Hooks/gl.hpp"

namespace
{
  static float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};
  float skyboxVertices[] = {
    // positions
    -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
    -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

    1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

  static unsigned int triangleVAO;
  static unsigned int triangleVBO;
  static unsigned int triangleShaderProgram;

  static unsigned int skyboxVAO;
  static unsigned int skyboxVBO;
  static unsigned int skyboxShaderProgram;
  static unsigned int skyboxTexture;
  static bool skyboxSet = false;

  static void CreateShader(const Krys::IO::Path &filepath, unsigned int shader) noexcept
  {
    auto reader = Krys::IO::NativeFileReader(filepath);
    auto sourceResult = Krys::IO::StreamUtils::ReadAllText(reader);
    assert(sourceResult.has_value());
    auto source = sourceResult->c_str();
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
  }

  static unsigned int CreateProgram(const Krys::IO::Path &vertexFilepath,
                                    const Krys::IO::Path &fragmentFilepath) noexcept
  {
    auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
    CreateShader(vertexFilepath, vertexShader);

    auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    CreateShader(fragmentFilepath, fragmentShader);

    auto shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
  }

  template <typename T>
  static void SetUniform(GLuint shaderProgram, const Krys::string &uniformName, const T &value) noexcept
  {
    using namespace Krys;
    using namespace Krys::Maths;

    GLint location = glGetUniformLocation(shaderProgram, uniformName.c_str());
    if (location == -1)
    {
      // Uniform not found, possibly optimized out by the compiler
      return;
    }

    if constexpr (std::is_same_v<T, bool>)
      glProgramUniform1i(shaderProgram, location, value);
    else if constexpr (std::is_same_v<T, int32>)
      glProgramUniform1i(shaderProgram, location, value);
    else if constexpr (std::is_same_v<T, List<int32>>)
      glProgramUniform1iv(shaderProgram, location, value.size(), value.data());
    else if constexpr (std::is_same_v<T, uint32>)
      glProgramUniform1ui(shaderProgram, location, value);
    else if constexpr (std::is_same_v<T, List<uint32>>)
      glProgramUniform1uiv(shaderProgram, location, value.size(), value.data());
    else if constexpr (std::is_same_v<T, float32>)
      glProgramUniform1f(shaderProgram, location, value);
    else if constexpr (std::is_same_v<T, List<float32>>)
      glProgramUniform1fv(shaderProgram, location, value.size(), value.data());
    else if constexpr (std::is_same_v<T, Vec2>)
      glProgramUniform2f(shaderProgram, location, value.x, value.y);
    else if constexpr (std::is_same_v<T, List<Vec2>>)
      glProgramUniform2fv(shaderProgram, location, value.size(), &value[0].x);
    else if constexpr (std::is_same_v<T, Vec3>)
      glProgramUniform3f(shaderProgram, location, value.x, value.y, value.z);
    else if constexpr (std::is_same_v<T, List<Vec3>>)
      glProgramUniform3fv(shaderProgram, location, value.size(), &value[0].x);
    else if constexpr (std::is_same_v<T, Vec4>)
      glProgramUniform4f(shaderProgram, location, value.x, value.y, value.z, value.w);
    else if constexpr (std::is_same_v<T, List<Vec4>>)
      glProgramUniform4fv(shaderProgram, location, value.size(), &value[0].x);
    else if constexpr (std::is_same_v<T, Mat2>)
      glProgramUniformMatrix2fv(shaderProgram, location, 1, GL_FALSE, &value[0].x);
    else if constexpr (std::is_same_v<T, List<Mat2>>)
      glProgramUniformMatrix2fv(shaderProgram, location, value.size(), GL_FALSE, &value[0][0].x);
    else if constexpr (std::is_same_v<T, Mat3>)
      glProgramUniformMatrix3fv(shaderProgram, location, 1, GL_FALSE, &value[0].x);
    else if constexpr (std::is_same_v<T, List<Mat3>>)
      glProgramUniformMatrix3fv(shaderProgram, location, value.size(), GL_FALSE, &value[0][0].x);
    else if constexpr (std::is_same_v<T, Mat4>)
      glProgramUniformMatrix4fv(shaderProgram, location, 1, GL_FALSE, &value[0].x);
    else if constexpr (std::is_same_v<T, List<Mat4>>)
      glProgramUniformMatrix4fv(shaderProgram, location, value.size(), GL_FALSE, &value[0][0].x);
    else
      assert(false && "Unsupported uniform type.");
  }
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
    {
      using namespace IO;
      triangleShaderProgram =
        CreateProgram(Path("data/shaders/opengl/triangle.vert"), Path("data/shaders/opengl/triangle.frag"));
      skyboxShaderProgram =
        CreateProgram(Path("data/shaders/opengl/skybox.vert"), Path("data/shaders/opengl/skybox.frag"));
    }

    glGenVertexArrays(1, &triangleVAO);
    glBindVertexArray(triangleVAO);
    glGenBuffers(1, &triangleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glGenVertexArrays(1, &skyboxVAO);
    glBindVertexArray(skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);
  }

  void OpenGLContext::Render(ICamera &camera) noexcept
  {
    auto &view = camera.ViewMatrix();
    auto &projection = camera.ProjectionMatrix();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(triangleShaderProgram);

    SetUniform(triangleShaderProgram, "view", view);
    SetUniform(triangleShaderProgram, "projection", projection);

    glBindVertexArray(triangleVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    if (skyboxSet)
    {
      glDepthFunc(GL_LEQUAL);
      glDepthMask(GL_FALSE);
      glUseProgram(skyboxShaderProgram);

      SetUniform(skyboxShaderProgram, "view", view);
      SetUniform(skyboxShaderProgram, "projection", projection);

      glBindVertexArray(skyboxVAO);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
      glDrawArrays(GL_TRIANGLES, 0, 36);
      glDepthMask(GL_TRUE);
      glDepthFunc(GL_LESS);
    }
  }

  void OpenGLContext::SetSkybox(const IO::CubeMapImage &skybox) noexcept
  {
    if (skyboxSet)
    {
      glDeleteTextures(1, &skyboxTexture);
    }

    glGenTextures(1, &skyboxTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, skybox.Width, skybox.Height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, skybox.Right.data());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, skybox.Width, skybox.Height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, skybox.Left.data());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, skybox.Width, skybox.Height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, skybox.Top.data());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, skybox.Width, skybox.Height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, skybox.Bottom.data());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, skybox.Width, skybox.Height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, skybox.Front.data());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, skybox.Width, skybox.Height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, skybox.Back.data());

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    skyboxSet = true;
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
