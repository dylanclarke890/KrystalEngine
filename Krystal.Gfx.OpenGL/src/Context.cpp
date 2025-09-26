#include "Krystal.Gfx.OpenGL/Context.hpp"
#include "Krystal.Lib/Detection.hpp"

#ifdef KRYS_PLATFORM_WINDOWS
  #include "Krystal.Gfx.OpenGL/Win32/ContextPlatformImpl.hpp"
#else
  #error "Unsupported platform for OpenGL context creation."
#endif

#include "Krystal.Gfx.OpenGL/Buffer.hpp"
#include "Krystal.Gfx.OpenGL/Hooks/gl.hpp"
#include "Krystal.Gfx.OpenGL/Utils.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Gfx/Light.hpp"
#include "Krystal.Gfx/VertexBufferLayout.hpp"
#include "Krystal.Lib/DebugBreak.hpp"
#include "Krystal.Lib/Expected.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Log/ILogger.hpp"
#include "Krystal.Maths/Clipspace.hpp"
#include "Krystal.Maths/Convert.hpp"
#include "Krystal.Maths/Interpolate.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Transform.hpp"
#include "Krystal.Maths/Vector.hpp"
#include "Krystal.Platform/Platform.hpp"
#include <format>
#include <random>

namespace
{
  using namespace Krys;
  using namespace Krys::Gfx;
  using namespace Krys::Gfx::OpenGL;
  using namespace Krys::Maths;

#pragma region Debug Output

  void DebugMessageCallback(GLenum source, GLenum type, uint id, GLenum severity, GLsizei, const char *msg,
                            const void *)
  {
    auto *logger = Log::GetGlobalLogger();
    if (logger == nullptr)
    {
      return;
    }

    string message {};

    switch (severity)
    {
      case GL_DEBUG_SEVERITY_HIGH:         message += std::format("OPENGL ({0}): {1}", id, msg); break;
      case GL_DEBUG_SEVERITY_MEDIUM:       message += std::format("OPENGL ({0}): {1}", id, msg); break;
      case GL_DEBUG_SEVERITY_LOW:          message += std::format("OPENGL ({0}): {1}", id, msg); break;
      case GL_DEBUG_SEVERITY_NOTIFICATION: message += std::format("OPENGL ({0}): {1}", id, msg); break;
      default:                             assert(false && "Unknown enum value: OpenGL severity level"); break;
    }
    message += "\n";

    message += " - Type: ";
    switch (type)
    {
      case GL_DEBUG_TYPE_ERROR:               message += "Error"; break;
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: message += "Deprecated Behavior"; break;
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  message += "Undefined Behavior"; break;
      case GL_DEBUG_TYPE_PORTABILITY:         message += "Portability"; break;
      case GL_DEBUG_TYPE_PERFORMANCE:         message += "Performance"; break;
      case GL_DEBUG_TYPE_MARKER:              message += "Marker"; break;
      case GL_DEBUG_TYPE_PUSH_GROUP:          message += "Push Group"; break;
      case GL_DEBUG_TYPE_POP_GROUP:           message += "Pop Group"; break;
      case GL_DEBUG_TYPE_OTHER:               message += "Other"; break;
      default:                                assert(false && "Unknown enum value: OpenGL message type"); break;
    }
    message += "\n";

    message += " - Source: ";
    switch (source)
    {
      case GL_DEBUG_SOURCE_API:             message += "API"; break;
      case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   message += "Window System"; break;
      case GL_DEBUG_SOURCE_SHADER_COMPILER: message += "Shader Compiler"; break;
      case GL_DEBUG_SOURCE_THIRD_PARTY:     message += "Third Party"; break;
      case GL_DEBUG_SOURCE_APPLICATION:     message += "Application"; break;
      case GL_DEBUG_SOURCE_OTHER:           message += "Other"; break;
      default:                              assert(false && "Unknown enum value: OpenGL source type"); break;
    }

    switch (severity)
    {
      case GL_DEBUG_SEVERITY_HIGH:         logger->Critical(message); break;
      case GL_DEBUG_SEVERITY_MEDIUM:       logger->Error(message); break;
      case GL_DEBUG_SEVERITY_LOW:          logger->Warn(message); break;
      case GL_DEBUG_SEVERITY_NOTIFICATION: logger->Info(message); break;
      default:                             assert(false && "Unknown enum value: OpenGL severity level"); break;
    }

    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION && severity != GL_DEBUG_SEVERITY_LOW)
      KRYS_DEBUG_BREAK();
  }

  void DisableDebugMessageIds(GLenum source, GLenum type, const List<uint32> &ids)
  {
    glDebugMessageControl(source, type, GL_DONT_CARE, static_cast<GLsizei>(ids.size()), ids.data(), GL_FALSE);
  }

  void EnableDebugOutput()
  {
    glDebugMessageCallback(DebugMessageCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);

    DisableDebugMessageIds(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER,
                           {
                             131'169, // Driver allocated storage for renderbuffer
                             131'185, // Buffer detailed info
                           });

    DisableDebugMessageIds(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_PERFORMANCE,
                           {
                             131'218, // Vertex shader is being recompiled based on GL state
                           });

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  }

#pragma endregion

  struct FrameBufferData
  {
    GLuint FBO {};
    List<GLuint> ColorTextures {};
    GLuint DepthTexture {};
    uint32 Width {};
    uint32 Height {};
  };

  Map<string, TextureHandle> textureHandles;
  Map<string, ShaderHandle> shaderHandles;
  Map<string, MaterialHandle> materialHandles;
  Map<string, MeshHandle> meshHandles;

  Mat4 ScreenOrthoProjection;
  FontHandle DefaultBitmapFont;
  FontHandle DefaultSDFFont;
  FontHandle DefaultMSDFFont;
  FontHandle DefaultMTSDFFont;

  Map<string, FrameBufferData> shadowMaps;
  Map<string, Unique<UniformBuffer>> ubos;

  FrameBufferData pingPongFBOs[2];

  GLuint noiseTexture;
  List<Vec3> ssaoKernel;

  void CreateShadowMapFramebuffer(const string &name, uint32 width, uint32 height) noexcept
  {
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    // create depth texture
    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    shadowMaps[name] = {depthMapFBO, {}, depthMap, width, height};

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE
           && "framebuffer is incomplete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void CreateCubeShadowMapFramebuffer(const string &name, uint32 width, uint32 height) noexcept
  {
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth cubemap texture
    GLuint depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (uint i = 0; i < 6; i++)
    {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0,
                   GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    shadowMaps[name] = {depthMapFBO, {}, depthCubemap, width, height};
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE
           && "framebuffer is incomplete");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void CreateFloatingPointFramebuffer(const string &name, uint32 width, uint32 height)
  {
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
    uint colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
      glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                      GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample
                                         // repeated texture values!
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      // attach texture to framebuffer
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);
    // finally check if framebuffer is complete
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE
           && "framebuffer is incomplete");
    shadowMaps[name] = {hdrFBO, {colorBuffers[0], colorBuffers[1]}, rboDepth, width, height};
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void CreatePingPongFramebuffer(const string &name, uint32 width, uint32 height)
  {
    uint pingpongFBO[2];
    uint pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (uint i = 0; i < 2; i++)
    {
      glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
      glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                      GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample
                                         // repeated texture values!
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
      // also check if framebuffers are complete (no need for depth buffer)
      assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE
             && "framebuffer is incomplete");
      shadowMaps[name + std::to_string(i)] = {pingpongFBO[i], {pingpongColorbuffers[i]}, 0, width, height};
    }
  }

  void CreateGFramebuffer(uint32 width, uint32 height)
  {
    uint gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    uint gPosition, gNormal, gColorSpec;

    // - position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // - normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // - color + specular color buffer
    glGenTextures(1, &gColorSpec);
    glBindTexture(GL_TEXTURE_2D, gColorSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColorSpec, 0);

    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    List<uint> attachments = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments.data());

    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE
           && "framebuffer is incomplete");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    shadowMaps["g-buffer"] = {gBuffer, {gPosition, gNormal, gColorSpec}, rboDepth, width, height};
  }

  void CreateSSAOFramebuffer(uint32 width, uint32 height)
  {
    unsigned int ssaoFBO, ssaoBlurFBO;
    glGenFramebuffers(1, &ssaoFBO);
    glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    unsigned int ssaoColorBuffer, ssaoColorBufferBlur;

    // SSAO color buffer
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE
           && "framebuffer is incomplete");

    // and blur stage
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE
           && "framebuffer is incomplete");

    shadowMaps["ssao"] = {ssaoFBO, {ssaoColorBuffer}, 0, width, height};
    shadowMaps["ssao-blur"] = {ssaoBlurFBO, {ssaoColorBufferBlur}, 0, width, height};

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void CreateEnvironmentAndIrradianceCubemaps(uint32 width, uint32 height, TextureRegistry &textureSystem,
                                              ShaderRegistry &shaderSystem, MeshRegistry &meshSystem)
  {
    uint captureFBO;
    uint captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    uint envCubemap;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (uint i = 0; i < 6; ++i)
    {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT,
                   nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Mat4 captureProjection = Perspective(Radians(90.0f), 1.0f, 0.1f, 10.0f);
    Mat4 captureViews[] = {LookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                           LookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                           LookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
                           LookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)),
                           LookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f)),
                           LookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, -1.0f, 0.0f))};

    auto &cube = meshSystem.Get(meshHandles.at("cube"));
    cube.Bind();

    {
      auto &shader = shaderSystem.Get(shaderHandles.at("hdr-to-cubemap"));
      shader.Bind();
      shader.SetUniform("equirectangularMap", 0);
      shader.SetUniform("projection", captureProjection);

      textureSystem.Get(textureHandles.at("hdr-environment")).Bind(0);

      glViewport(0, 0, width, height);
      glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
      for (uint i = 0; i < 6; ++i)
      {
        shader.SetUniform("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cube.Draw();
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
      glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

      shadowMaps["hdr-cubemap"] = {captureFBO, {envCubemap}, captureRBO, width, height};
    }

    {
      auto &shader = shaderSystem.Get(shaderHandles.at("irradiance-convolution"));
      GLuint irradianceMap;
      glGenTextures(1, &irradianceMap);
      glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
      for (GLuint i = 0; i < 6; ++i)
      {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
      }
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
      glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

      shader.Bind();
      shader.SetUniform("environmentMap", 0);
      shader.SetUniform("projection", captureProjection);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

      glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
      glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
      for (uint i = 0; i < 6; ++i)
      {
        shader.SetUniform("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cube.Draw();
      }

      shadowMaps["irradiance-cubemap"] = {captureFBO, {irradianceMap}, captureRBO, width, height};
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    {
      unsigned int prefilterMap;
      glGenTextures(1, &prefilterMap);
      glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
      for (unsigned int i = 0; i < 6; ++i)
      {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT,
                     nullptr);
      }
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
      glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

      auto &shader = shaderSystem.Get(shaderHandles.at("prefilter"));
      shader.Bind();
      shader.SetUniform("environmentMap", 0);
      shader.SetUniform("projection", captureProjection);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
      glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
      unsigned int maxMipLevels = 5;
      for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
      {
        // reisze framebuffer according to mip-level size.
        uint mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        uint mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);
        float roughness = (float)mip / (float)(maxMipLevels - 1);
        shader.SetUniform("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
          shader.SetUniform("view", captureViews[i]);
          glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                 prefilterMap, mip);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          cube.Draw();
        }
      }

      shadowMaps["prefilter-cubemap"] = {captureFBO, {prefilterMap}, captureRBO, width, height};
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    {
      auto &shader = shaderSystem.Get(shaderHandles.at("brdf"));
      // pbr: generate a 2D LUT from the BRDF equations used.
      // ----------------------------------------------------
      unsigned int brdfLUTTexture;
      glGenTextures(1, &brdfLUTTexture);

      // pre-allocate enough memory for the LUT texture.
      glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
      // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
      glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
      glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

      glViewport(0, 0, 512, 512);
      shader.Bind();
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      auto &mesh = meshSystem.Get(meshHandles.at("screen-quad"));
      mesh.Bind();
      mesh.Draw();

      shadowMaps["brdf-lut"] = {captureFBO, {brdfLUTTexture}, captureRBO, 512, 512};
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void CreateNoiseTexture()
  {
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    for (unsigned int i = 0; i < 64; ++i)
    {
      Vec3 sample(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f,
                  randomFloats(generator));
      sample = Normalize(sample);
      sample *= randomFloats(generator);
      float scale = float(i) / 64.0f;

      // scale samples s.t. they're more aligned to center of kernel
      scale = Lerp(0.1f, 1.0f, scale * scale);
      sample *= scale;
      ssaoKernel.push_back(sample);
    }

    List<Vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
      Vec3 noise(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f,
                 0.0f); // rotate around z-axis (in tangent space)
      ssaoNoise.push_back(noise);
    }
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, ssaoNoise.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }

  void DrawPBRObject(Mesh &mesh, Shader &shader, Material &material, const Mat4 &model,
                     TextureRegistry &textures)
  {
    shader.SetUniform("model", model);
    shader.SetUniform("normalMatrix", Inverse(Transpose(Mat3(model))));
    textures.Get(std::get<TextureHandle>(material.Parameters[6].Value)).Bind(3);
    textures.Get(std::get<TextureHandle>(material.Parameters[7].Value)).Bind(5);
    textures.Get(std::get<TextureHandle>(material.Parameters[8].Value)).Bind(6);
    textures.Get(std::get<TextureHandle>(material.Parameters[9].Value)).Bind(7);
    textures.Get(std::get<TextureHandle>(material.Parameters[10].Value)).Bind(4);
    mesh.Draw();
  }
}

namespace Krys::Gfx
{
  Expected<Unique<IContext>> CreateContext(NativeHandle windowHandle, uint32 width, uint32 height) noexcept
  {
    try
    {
      return Expected<Unique<IContext>>(CreateUnique<OpenGL::Context>(windowHandle, width, height));
    }
    catch (const std::exception &e)
    {
      return Unexpected(e.what());
    }
  }
}

namespace Krys::Gfx::OpenGL
{
  Context::Context(NativeHandle windowHandle, uint32 width, uint32 height)
      : _windowHandle(windowHandle), _width(width), _height(height),
        _dpi(Platform::GetDPIForWindow(_windowHandle)),
        _platformImpl(CreateUnique<ContextPlatformImpl>(windowHandle)), _textures(), _samplers(), _shaders(),
        _meshes(), _materials(_textures), _fonts(_dpi), _text(_fonts, _shaders, _dpi)
  {
  }

  void Context::Setup() noexcept
  {
    EnableDebugOutput();

    auto *logger = Log::GetGlobalLogger();

    // Shaders
    {
      using namespace IO;
      Path base = Path("data/shaders/opengl");

      shaderHandles["model"] = _shaders.Load(base / Path("model.vert"), base / Path("model.frag"));
      shaderHandles["instanced-model"] =
        _shaders.Load(base / Path("instanced-model.vert"), base / Path("instanced-model.frag"));
      shaderHandles["visualise-normals"] =
        _shaders.Load(base / Path("visualise-normals.vert"), base / Path("visualise-normals.geo"),
                      base / Path("visualise-normals.frag"));
      shaderHandles["skybox"] = _shaders.Load(base / Path("skybox.vert"), base / Path("skybox.frag"));
      shaderHandles["light-source"] =
        _shaders.Load(base / Path("lightsource.vert"), base / Path("lightsource.frag"));
      shaderHandles["lighting"] = _shaders.Load(base / Path("basic.vert"), base / Path("lighting.frag"));
      shaderHandles["flat-colour-phong-material"] =
        _shaders.Load(base / Path("basic.vert"), base / Path("flat-colour-phong-material.frag"));
      shaderHandles["phong-material"] =
        _shaders.Load(base / Path("basic.vert"), base / Path("phong-material.frag"));
      shaderHandles["directional-depth"] =
        _shaders.Load(base / Path("directional-shadow-map.vert"), base / Path("empty.frag"));
      shaderHandles["point-depth"] =
        _shaders.Load(base / Path("point-shadows.vert"), base / Path("point-shadows.geo"),
                      base / Path("point-shadows.frag"));
      shaderHandles["directional-shadow-mapping"] =
        _shaders.Load(base / Path("shadow-mapping.vert"), base / Path("shadow-mapping.frag"));
      shaderHandles["point-shadow-mapping"] =
        _shaders.Load(base / Path("point-shadow-mapping.vert"), base / Path("point-shadow-mapping.frag"));
      shaderHandles["debug-quad"] =
        _shaders.Load(base / Path("debug-quad-shadow-map.vert"), base / Path("debug-quad-shadow-map.frag"));
      shaderHandles["normal-mapping"] =
        _shaders.Load(base / Path("normal-mapping.vert"), base / Path("normal-mapping.frag"));

      shaderHandles["parallax-mapping"] =
        _shaders.Load(base / Path("parallax-mapping.vert"), base / Path("parallax-mapping.frag"));

      shaderHandles["hdr"] = _shaders.Load(base / Path("hdr.vert"), base / Path("hdr.frag"));
      shaderHandles["hdr-lighting"] =
        _shaders.Load(base / Path("hdr-test-lighting.vert"), base / Path("hdr-test-lighting.frag"));

      shaderHandles["bloom"] = _shaders.Load(base / Path("7/bloom.vert"), base / Path("7/bloom.frag"));
      shaderHandles["bloom-light"] =
        _shaders.Load(base / Path("7/bloom.vert"), base / Path("7/light-box.frag"));
      shaderHandles["bloom-final"] =
        _shaders.Load(base / Path("7/bloom-final.vert"), base / Path("7/bloom-final.frag"));
      shaderHandles["blur"] = _shaders.Load(base / Path("7/blur.vert"), base / Path("7/blur.frag"));

      shaderHandles["g-buffer"] =
        _shaders.Load(base / Path("8/g-buffer.vert"), base / Path("8/g-buffer.frag"));
      shaderHandles["deferred-shading"] =
        _shaders.Load(base / Path("8/deferred-shading.vert"), base / Path("8/deferred-shading.frag"));
      shaderHandles["deferred-light"] =
        _shaders.Load(base / Path("8/deferred-light-box.vert"), base / Path("8/deferred-light-box.frag"));

      shaderHandles["ssao-geometry"] =
        _shaders.Load(base / Path("9/ssao-geometry.vert"), base / Path("9/ssao-geometry.frag"));
      shaderHandles["ssao"] = _shaders.Load(base / Path("9/ssao.vert"), base / Path("9/ssao.frag"));
      shaderHandles["ssao-blur"] = _shaders.Load(base / Path("9/ssao.vert"), base / Path("9/ssao-blur.frag"));
      shaderHandles["ssao-lighting"] =
        _shaders.Load(base / Path("9/ssao.vert"), base / Path("9/ssao-lighting.frag"));

      shaderHandles["pbr"] = _shaders.Load(base / Path("11/pbr.vert"), base / Path("11/pbr.frag"));
      shaderHandles["pbr-with-maps"] =
        _shaders.Load(base / Path("11/pbr-with-maps.vert"), base / Path("11/pbr-with-maps.frag"));
      shaderHandles["hdr-to-cubemap"] =
        _shaders.Load(base / Path("11/cubemap.vert"), base / Path("11/equirectangular-to-cubemap.frag"));
      shaderHandles["irradiance-convolution"] =
        _shaders.Load(base / Path("11/cubemap.vert"), base / Path("11/irradiance-convolution.frag"));
      shaderHandles["prefilter"] =
        _shaders.Load(base / Path("11/cubemap.vert"), base / Path("11/prefilter.frag"));
      shaderHandles["brdf"] = _shaders.Load(base / Path("11/brdf.vert"), base / Path("11/brdf.frag"));
      shaderHandles["hdr-background"] =
        _shaders.Load(base / Path("11/background.vert"), base / Path("11/background.frag"));

      shaderHandles["bitmap-font"] =
        _shaders.Load(base / Path("font/glyph.vert"), base / Path("font/bitmap.frag"));
      shaderHandles["sdf-font"] = _shaders.Load(base / Path("font/glyph.vert"), base / Path("font/sdf.frag"));
      shaderHandles["msdf-font"] =
        _shaders.Load(base / Path("font/glyph.vert"), base / Path("font/msdf.frag"));
      shaderHandles["mtsdf-font"] =
        _shaders.Load(base / Path("font/glyph.vert"), base / Path("font/mtsdf.frag"));
    }

    {
      // models["backpack"] = CreateUnique<Model>(IO::Path("data/assets/models/backpack/backpack.obj"));
    }

    // Textures
    {
      using namespace IO;
      Path base = Path("data/assets");

      textureHandles["hdr-environment"] = _textures.Load(base / Path("newport-loft.hdr"));
    }

    // PBR textures
    {
      using namespace IO;
      ShaderHandle shader = shaderHandles.at("pbr-with-maps");
      materialHandles["rusted-iron"] = _materials.LoadPBRMaterial("rusted-iron", shader);
      materialHandles["gold"] = _materials.LoadPBRMaterial("gold", shader);
      materialHandles["grass"] = _materials.LoadPBRMaterial("grass", shader);
      materialHandles["plastic"] = _materials.LoadPBRMaterial("plastic", shader);
      materialHandles["wall"] = _materials.LoadPBRMaterial("wall", shader);
    }

    meshHandles["cube"] = _meshes.CreateCube();
    meshHandles["sphere"] = _meshes.CreateSphere();
    meshHandles["screen-quad"] = _meshes.CreateScreenQuad();

    // Shadow maps
    CreateGFramebuffer(_width, _height);
    CreateSSAOFramebuffer(_width, _height);
    CreateNoiseTexture();

    // Uniform buffers
    {
      ubos["matrices"] = CreateUnique<UniformBuffer>(3 * sizeof(Mat4));
      ubos.at("matrices")->Bind(0);
    }

    CreateEnvironmentAndIrradianceCubemaps(1'024, 1'024, _textures, _shaders, _meshes);

    {
      auto &shader = _shaders.Get(shaderHandles.at("pbr-with-maps"));
      shader.SetUniform("irradianceMap", 0);
      shader.SetUniform("prefilterMap", 1);
      shader.SetUniform("brdfLUT", 2);
      shader.SetUniform("albedoMap", 3);
      shader.SetUniform("normalMap", 4);
      shader.SetUniform("metallicMap", 5);
      shader.SetUniform("roughnessMap", 6);
      shader.SetUniform("aoMap", 7);
    }
    {
      auto &shader = _shaders.Get(shaderHandles.at("hdr-background"));
      shader.SetUniform("environmentMap", 0);
    }

    glViewport(0, 0, _width, _height);
    logger->Info("Viewport set to {}x{}", _width, _height);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glClearColor(0.1f, 0.1f, 0.1f, 1.f);

    const auto fontPath = IO::Path("data/assets/fonts/Antonio-Bold.ttf");
    const auto fontSize = 96;
    DefaultBitmapFont = _fonts.Load(fontPath, fontSize, FontType::Bitmap);
    DefaultSDFFont = _fonts.Load(fontPath, fontSize, FontType::SDF);
    DefaultMSDFFont = _fonts.Load(fontPath, fontSize, FontType::MSDF);
    DefaultMTSDFFont = _fonts.Load(fontPath, fontSize, FontType::MTSDF);
    ScreenOrthoProjection = Ortho(0.0f, static_cast<float>(_width), 0.0f, static_cast<float>(_height));
    ubos.at("matrices")->Update(ScreenOrthoProjection, 2 * sizeof(Mat4));
  }

  void Context::Render(ICamera &camera) noexcept
  {
    auto view = camera.ViewMatrix();
    auto projection = camera.ProjectionMatrix();
    ubos.at("matrices")->Update(List<Mat4> {view, projection});
    _shaders.Get(shaderHandles.at("hdr-background")).SetUniform("projection", projection);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto &shader = _shaders.Get(shaderHandles.at("pbr-with-maps"));
    shader.Bind();
    shader.SetUniform("camPos", camera.Position());

    auto &sphere = _meshes.Get(meshHandles.at("sphere"));
    sphere.Bind();

    glBindTextureUnit(0, shadowMaps["irradiance-cubemap"].ColorTextures[0]);
    glBindTextureUnit(1, shadowMaps["prefilter-cubemap"].ColorTextures[0]);
    glBindTextureUnit(2, shadowMaps["brdf-lut"].ColorTextures[0]);

    {
      Mat4 model = Translate(Identity<Mat4>(), Vec3(-5.0, 0.0, 2.0));
      auto &material = _materials.Get(materialHandles.at("rusted-iron"));
      DrawPBRObject(sphere, shader, material, model, _textures);
    }
    {
      Mat4 model = Translate(Identity<Mat4>(), Vec3(-3.0, 0.0, 2.0));
      auto &material = _materials.Get(materialHandles.at("gold"));
      DrawPBRObject(sphere, shader, material, model, _textures);
    }
    {
      Mat4 model = Translate(Identity<Mat4>(), Vec3(-1.0, 0.0, 2.0));
      auto &material = _materials.Get(materialHandles.at("grass"));
      DrawPBRObject(sphere, shader, material, model, _textures);
    }
    {
      Mat4 model = Translate(Identity<Mat4>(), Vec3(1.0, 0.0, 2.0));
      auto &material = _materials.Get(materialHandles.at("plastic"));
      DrawPBRObject(sphere, shader, material, model, _textures);
    }
    {
      Mat4 model = Translate(Identity<Mat4>(), Vec3(3.0, 0.0, 2.0));
      auto &material = _materials.Get(materialHandles.at("wall"));
      DrawPBRObject(sphere, shader, material, model, _textures);
    }

    Vec3 lightPositions[] = {
      Vec3(-10.0f, 10.0f, 10.0f),
      Vec3(10.0f, 10.0f, 10.0f),
      Vec3(-10.0f, -10.0f, 10.0f),
      Vec3(10.0f, -10.0f, 10.0f),
    };
    Vec3 lightColors[] = {Vec3(300.0f, 300.0f, 300.0f), Vec3(300.0f, 300.0f, 300.0f),
                          Vec3(300.0f, 300.0f, 300.0f), Vec3(300.0f, 300.0f, 300.0f)};

    for (uint i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
    {
      Vec3 newPos = lightPositions[i] + Vec3(std::sin((float)Platform::GetTime() * 5.0f) * 5.0f, 0.0f, 0.0f);
      shader.SetUniform("lightPositions[" + std::to_string(i) + "]", newPos);
      shader.SetUniform("lightColors[" + std::to_string(i) + "]", lightColors[i]);

      Mat4 model = Identity<Mat4>();
      model = Translate(model, newPos);
      model = Scale(model, Vec3(0.5f));
      shader.SetUniform("model", model);
      shader.SetUniform("normalMatrix", Transpose(Inverse(Mat3(model))));
      sphere.Draw();
    }

    {
      auto &shader = _shaders.Get(shaderHandles.at("hdr-background"));
      shader.Bind();
      shader.SetUniform("view", view);

      glBindTextureUnit(0, shadowMaps["hdr-cubemap"].ColorTextures[0]);

      auto &cube = _meshes.Get(meshHandles.at("cube"));
      cube.Bind();
      cube.Draw();
    }

    glClear(GL_COLOR_BUFFER_BIT); // This is just to make the text easier to see

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const float xPos = 25.f;
    const auto height = static_cast<float>(_height);

    _text.Draw("Font Rendering - Bitmap", DefaultBitmapFont, {xPos, height - 150.f}, Colours::Black);
    _text.Draw("Font Rendering - SDF", DefaultSDFFont, {xPos, height - 300.f}, Colours::Black);
    _text.Draw("Font Rendering - MSDF", DefaultMSDFFont, {xPos, height - 450.f}, Colours::Purple);
    _text.Draw("Font Rendering - MTSDF", DefaultMTSDFFont, {xPos, height - 600.f}, Colours::Black);

    glDisable(GL_BLEND);
  }

  void Context::Present() noexcept
  {
    _platformImpl->Present();
  }

  void Context::Resize(uint32 width, uint32 height) noexcept
  {
    _width = width;
    _height = height;
    glViewport(0, 0, _width, _height);
    ScreenOrthoProjection = Ortho(0.0f, static_cast<float>(_width), 0.0f, static_cast<float>(_height));
  }

  void Context::DPIChanged(int dpi) noexcept
  {
    _dpi = dpi;
    _fonts.DPIChanged(dpi);
  }

  ISamplerRegistry &Context::Samplers() noexcept
  {
    return _samplers;
  }

  ITextureRegistry &Context::Textures() noexcept
  {
    return _textures;
  }

  IShaderRegistry &Context::Shaders() noexcept
  {
    return _shaders;
  }

  IMeshRegistry &Context::Meshes() noexcept
  {
    return _meshes;
  }

  IMaterialRegistry &Context::Materials() noexcept
  {
    return _materials;
  }

  IFontRegistry &Context::Fonts() noexcept
  {
    return _fonts;
  }
}
