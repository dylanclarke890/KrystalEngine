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
#include "Krystal.Lib/Expected.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Maths/Clipspace.hpp"
#include "Krystal.Maths/Convert.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Transform.hpp"
#include "Krystal.Maths/Vector.hpp"
#include "Krystal.Platform/Platform.hpp"
#include <random>

namespace
{
  using namespace Krys;
  using namespace Krys::Gfx;
  using namespace Krys::Gfx::OpenGL;
  using namespace Krys::Maths;

  struct FrameBufferData
  {
    GLuint FBO {};
    List<GLuint> ColorTextures {};
    GLuint DepthTexture {};
    uint32 Width {};
    uint32 Height {};
  };

  Vec3 lightPos = Vec3(2.0f, 4.0f, -2.0f);
  Vec3 lightColor = Vec3(0.2f, 0.2f, 0.7f);

  Map<string, TextureHandle> textureHandles;
  Map<string, ShaderHandle> shaderHandles;
  Map<string, MeshHandle> meshHandles;
  Map<string, FrameBufferData> shadowMaps;
  Map<string, Unique<UniformBuffer>> ubos;

  FrameBufferData pingPongFBOs[2];

  GLuint noiseTexture;
  List<Vec3> ssaoKernel;

#pragma region Lights

  static LightAttenuation attenuation {1.0f, 0.09f, 0.032f};
  static Colour ambientColor = {0.2f, 0.2f, 0.2f};
  static Colour diffuseColor = {0.5f, 0.5f, 0.5f};
  static Colour specularColor = {1.0f, 1.0f, 1.0f};

  static DirectionalLight directionalLight {
    {-0.2f, -1.0f, -0.3f}, LightColour {{0.05f, 0.05f, 0.05f}, {0.4f, 0.4f, 0.4f}, {0.5f, 0.5f, 0.5f}}};

  static SpotLight spotLight {
    {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f},   LightColour {ambientColor, diffuseColor, specularColor},
    attenuation,        Maths::Radians(12.5f), Maths::Radians(15.0f)};

  static PointLight pointLights[4] = {
    {Maths::Vec3(0.7f, 0.2f, 2.0f), LightColour {ambientColor, {1.0f, 0.6f, 0.0f}, specularColor},
     attenuation},
    {Maths::Vec3(2.3f, -3.3f, -4.0f), LightColour {ambientColor, {1.0f, 0.0f, 0.0f}, specularColor},
     attenuation},
    {Maths::Vec3(-4.0f, 2.0f, -12.0f), LightColour {ambientColor, {1.0f, 1.0f, 0.0f}, specularColor},
     attenuation},
    {Maths::Vec3(0.0f, 0.0f, -3.0f), LightColour {ambientColor, {0.2f, 0.2f, 1.0f}, specularColor},
     attenuation}};

  static void SetLightUniforms(Shader &shader, ICamera &camera) noexcept
  {
    Utils::SetDirectionalLightUniforms(shader, directionalLight);

    for (uint i = 0; i < 4; i++)
    {
      Utils::SetPointLightUniforms(shader, pointLights[i], "pointLights[" + std::to_string(i) + "]");
    }

    spotLight.Position = camera.Position();
    spotLight.Direction = camera.Forward();
    Utils::SetSpotLightUniforms(shader, spotLight);
  }

#pragma endregion

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

  void CreateEnvironmentAndIrradianceCubemaps(uint32 width, uint32 height, TextureSystem &textureSystem,
                                              ShaderSystem &shaderSystem, MeshSystem &meshSystem)
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

  float Lerp(float a, float b, float f)
  {
    return a + f * (b - a);
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

  void LoadPBRTextures(const string &name, TextureSystem &textureSystem)
  {
    using namespace IO;
    Path base = Path("data/assets/pbr/") / Path(name);

    textureHandles[name + "-albedo"] = textureSystem.Load(base / Path("albedo.png"));
    textureSystem.Get(textureHandles.at(name + "-albedo")).SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    textureSystem.Get(textureHandles.at(name + "-albedo")).SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

    textureHandles[name + "-normal"] = textureSystem.Load(base / Path("normal.png"));
    textureSystem.Get(textureHandles.at(name + "-normal")).SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    textureSystem.Get(textureHandles.at(name + "-normal")).SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

    textureHandles[name + "-metallic"] = textureSystem.Load(base / Path("metallic.png"));
    textureSystem.Get(textureHandles.at(name + "-metallic")).SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    textureSystem.Get(textureHandles.at(name + "-metallic")).SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

    textureHandles[name + "-roughness"] = textureSystem.Load(base / Path("roughness.png"));
    textureSystem.Get(textureHandles.at(name + "-roughness")).SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    textureSystem.Get(textureHandles.at(name + "-roughness")).SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

    textureHandles[name + "-ao"] = textureSystem.Load(base / Path("ao.png"));
    textureSystem.Get(textureHandles.at(name + "-ao")).SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    textureSystem.Get(textureHandles.at(name + "-ao")).SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
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
      : _windowHandle(windowHandle), _platformImpl(CreateUnique<ContextPlatformImpl>(windowHandle)),
        _width(width), _height(height)
  {
  }

  void Context::Setup() noexcept
  {
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

      LoadPBRTextures("rusted-iron", _textures);
      LoadPBRTextures("gold", _textures);
      LoadPBRTextures("grass", _textures);
      LoadPBRTextures("plastic", _textures);
      LoadPBRTextures("wall", _textures);
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
      ubos["matrices"] = CreateUnique<UniformBuffer>(2 * sizeof(Mat4));
      ubos.at("matrices")->Bind(0);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glClearColor(0.1f, 0.1f, 0.1f, 1.f);

    CreateEnvironmentAndIrradianceCubemaps(1'024, 1'024, _textures, _shaders, _meshes);
    glViewport(0, 0, _width, _height);

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
  }

  void Context::Render(ICamera &camera) noexcept
  {
    auto view = camera.ViewMatrix();
    auto projection = camera.ProjectionMatrix();
    ubos.at("matrices")->Update(List<Mat4> {view, projection});
    _shaders.Get(shaderHandles.at("hdr-background")).SetUniform("projection", projection);

    Vec3 lightPositions[] = {
      Vec3(-10.0f, 10.0f, 10.0f),
      // Vec3(10.0f, 10.0f, 10.0f),
      // Vec3(-10.0f, -10.0f, 10.0f),
      // Vec3(10.0f, -10.0f, 10.0f),
    };
    Vec3 lightColors[] = {
      Vec3(300.0f, 300.0f, 300.0f),
      // Vec3(300.0f, 300.0f, 300.0f),
      //                     Vec3(300.0f, 300.0f, 300.0f), Vec3(300.0f, 300.0f, 300.0f)
    };
    int nrRows = 7;
    int nrColumns = 7;
    float spacing = 2.5;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto &shader = _shaders.Get(shaderHandles.at("pbr-with-maps"));
    shader.Bind();
    shader.SetUniform("camPos", camera.Position());

    auto &sphere = _meshes.Get(meshHandles.at("sphere"));
    sphere.Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMaps["irradiance-cubemap"].ColorTextures[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMaps["prefilter-cubemap"].ColorTextures[0]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, shadowMaps["brdf-lut"].ColorTextures[0]);

    // rusted iron
    {
      _textures.Get(textureHandles.at("rusted-iron-albedo")).Bind(3);
      _textures.Get(textureHandles.at("rusted-iron-normal")).Bind(4);
      _textures.Get(textureHandles.at("rusted-iron-metallic")).Bind(5);
      _textures.Get(textureHandles.at("rusted-iron-roughness")).Bind(6);
      _textures.Get(textureHandles.at("rusted-iron-ao")).Bind(7);

      Mat4 model = Identity<Mat4>();
      model = Translate(model, Vec3(-5.0, 0.0, 2.0));
      shader.SetUniform("model", model);
      shader.SetUniform("normalMatrix", Transpose(Inverse(Mat3(model))));
      sphere.Draw();
    }

    // gold
    {
      _textures.Get(textureHandles.at("gold-albedo")).Bind(3);
      _textures.Get(textureHandles.at("gold-normal")).Bind(4);
      _textures.Get(textureHandles.at("gold-metallic")).Bind(5);
      _textures.Get(textureHandles.at("gold-roughness")).Bind(6);
      _textures.Get(textureHandles.at("gold-ao")).Bind(7);

      Mat4 model = Identity<Mat4>();
      model = Translate(model, Vec3(-3.0, 0.0, 2.0));
      shader.SetUniform("model", model);
      shader.SetUniform("normalMatrix", Transpose(Inverse(Mat3(model))));
      sphere.Draw();
    }

    // grass
    {
      _textures.Get(textureHandles.at("grass-albedo")).Bind(3);
      _textures.Get(textureHandles.at("grass-normal")).Bind(4);
      _textures.Get(textureHandles.at("grass-metallic")).Bind(5);
      _textures.Get(textureHandles.at("grass-roughness")).Bind(6);
      _textures.Get(textureHandles.at("grass-ao")).Bind(7);

      Mat4 model = Identity<Mat4>();
      model = Translate(model, Vec3(-1.0, 0.0, 2.0));
      shader.SetUniform("model", model);
      shader.SetUniform("normalMatrix", Transpose(Inverse(Mat3(model))));
      sphere.Draw();
    }

    // plastic
    {
      _textures.Get(textureHandles.at("plastic-albedo")).Bind(3);
      _textures.Get(textureHandles.at("plastic-normal")).Bind(4);
      _textures.Get(textureHandles.at("plastic-metallic")).Bind(5);
      _textures.Get(textureHandles.at("plastic-roughness")).Bind(6);
      _textures.Get(textureHandles.at("plastic-ao")).Bind(7);

      Mat4 model = Identity<Mat4>();
      model = Translate(model, Vec3(1.0, 0.0, 2.0));
      shader.SetUniform("model", model);
      shader.SetUniform("normalMatrix", Transpose(Inverse(Mat3(model))));
      sphere.Draw();
    }

    // wall
    {
      _textures.Get(textureHandles.at("wall-albedo")).Bind(3);
      _textures.Get(textureHandles.at("wall-normal")).Bind(4);
      _textures.Get(textureHandles.at("wall-metallic")).Bind(5);
      _textures.Get(textureHandles.at("wall-roughness")).Bind(6);
      _textures.Get(textureHandles.at("wall-ao")).Bind(7);

      Mat4 model = Identity<Mat4>();
      model = Translate(model, Vec3(3.0, 0.0, 2.0));
      shader.SetUniform("model", model);
      shader.SetUniform("normalMatrix", Transpose(Inverse(Mat3(model))));
      sphere.Draw();
    }

    // render light source (simply re-render sphere at light positions)
    // this looks a bit off as we use the same shader, but it'll make their positions obvious and
    // keeps the codeprint small.
    for (uint i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
    {
      Vec3 newPos = lightPositions[i] + Vec3(std::sin((float)Platform::GetTime() * 5.0f) * 5.0f, 0.0f, 0.0f);
      newPos = lightPositions[i];
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

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMaps["hdr-cubemap"].ColorTextures[0]);

      auto &cube = _meshes.Get(meshHandles.at("cube"));
      cube.Bind();
      cube.Draw();
    }
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
  }

  ITextureSystem &Context::Textures() noexcept
  {
    return _textures;
  }

  IShaderSystem &Context::Shaders() noexcept
  {
    return _shaders;
  }

  IMeshSystem &Context::Meshes() noexcept
  {
    return _meshes;
  }
}
