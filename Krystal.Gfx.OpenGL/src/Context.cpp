#include "Krystal.Gfx.OpenGL/Context.hpp"

#include "Krystal.Core/Detection.hpp"
#ifdef KRYS_PLATFORM_WINDOWS
  #include "Krystal.Gfx.OpenGL/Win32/ContextPlatformImpl.hpp"
#else
  #error "Unsupported platform for OpenGL context creation."
#endif

#include "Krystal.Core/Core.hpp"
#include "Krystal.Gfx.OpenGL/Buffer.hpp"
#include "Krystal.Gfx.OpenGL/Hooks/gl.hpp"
#include "Krystal.Gfx.OpenGL/Model.hpp"
#include "Krystal.Gfx.OpenGL/Shader.hpp"
#include "Krystal.Gfx.OpenGL/Texture.hpp"
#include "Krystal.Gfx.OpenGL/Utils.hpp"
#include "Krystal.Gfx.OpenGL/VertexArray.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Gfx/Light.hpp"
#include "Krystal.Gfx/VertexBufferLayout.hpp"
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

  Map<string, Unique<Shader>> shaders;
  Map<string, Unique<Texture2D>> textures;
  Map<string, Unique<CubeMap>> cubemaps;
  Map<string, Unique<Model>> models;
  Map<string, Unique<VertexArray>> vaos;
  Map<string, Unique<VertexBuffer>> vbos;
  Map<string, Unique<IndexBuffer>> ebos;
  Map<string, Unique<UniformBuffer>> ubos;
  Map<string, FrameBufferData> shadowMaps;

  FrameBufferData pingPongFBOs[2];

  GLuint noiseTexture;
  List<Vec3> ssaoKernel;
  uint sphereIndexCount = 0;

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

  void CreateEnvironmentAndIrradianceCubemaps(uint32 width, uint32 height)
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

    {
      auto &shader = shaders.at("hdr-to-cubemap");
      shader->Bind();
      shader->SetUniform("equirectangularMap", 0);
      shader->SetUniform("projection", captureProjection);

      textures.at("hdr-environment")->Bind(0);
      vaos.at("cube")->Bind();

      glViewport(0, 0, width, height);
      glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
      for (uint i = 0; i < 6; ++i)
      {
        shader->SetUniform("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Utils::Draw(GL_TRIANGLES, 36);
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
      glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

      shadowMaps["hdr-cubemap"] = {captureFBO, {envCubemap}, captureRBO, width, height};
    }

    {
      auto &shader = shaders.at("irradiance-convolution");
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

      shader->Bind();
      shader->SetUniform("environmentMap", 0);
      shader->SetUniform("projection", captureProjection);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

      glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
      glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
      for (uint i = 0; i < 6; ++i)
      {
        shader->SetUniform("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Utils::Draw(GL_TRIANGLES, 36);
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

      auto &shader = shaders.at("prefilter");
      shader->Bind();
      shader->SetUniform("environmentMap", 0);
      shader->SetUniform("projection", captureProjection);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
      glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
      unsigned int maxMipLevels = 5;
      for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
      {
        // reisze framebuffer according to mip-level size.
        unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);
        float roughness = (float)mip / (float)(maxMipLevels - 1);
        shader->SetUniform("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
          shader->SetUniform("view", captureViews[i]);
          glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                 prefilterMap, mip);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          Utils::Draw(GL_TRIANGLES, 36);
        }
      }

      shadowMaps["prefilter-cubemap"] = {captureFBO, {prefilterMap}, captureRBO, width, height};
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    {
      auto &shader = shaders.at("brdf");
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
      shader->Bind();
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      vaos.at("screen-quad")->Bind();
      Utils::Draw(GL_TRIANGLE_STRIP, 4);

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

  void RenderScene(Shader &shader)
  {
    vaos.at("plane")->Bind();
    Mat4 model = Identity<Mat4>();
    shader.SetUniform("model", model);
    Utils::Draw(GL_TRIANGLES, 36);

    vaos.at("cube")->Bind();
    model = Identity<Mat4>();
    model = Translate(model, Vec3(0.0f, 1.5f, 0.0));
    model = Scale(model, Vec3(0.5f));
    shader.SetUniform("model", model);
    Utils::Draw(GL_TRIANGLES, 36);

    model = Identity<Mat4>();
    model = Translate(model, Vec3(2.0f, 0.0f, 1.0));
    model = Scale(model, Vec3(0.5f));
    shader.SetUniform("model", model);
    Utils::Draw(GL_TRIANGLES, 36);

    model = Identity<Mat4>();
    model = Translate(model, Vec3(-1.0f, 0.0f, 2.0));
    model = Rotate(model, Radians(60.0f), Normalize(Vec3(1.0, 0.0, 1.0)));
    model = Scale(model, Vec3(0.25));
    shader.SetUniform("model", model);
    Utils::Draw(GL_TRIANGLES, 36);
  }

  void CreateVertexArray(const string &name, const BufferData &vertices,
                         const VertexBufferLayout &layout) noexcept
  {
    vaos[name] = CreateUnique<VertexArray>();
    vbos[name] = CreateUnique<VertexBuffer>(vertices);

    vaos.at(name)->Bind();
    vbos.at(name)->Bind();
    Utils::ApplyVertexBufferLayout(layout);
  }

  void CreateVertexArray(const string &name, const BufferData &vertices, const BufferData &indices,
                         const VertexBufferLayout &layout) noexcept
  {
    vaos[name] = CreateUnique<VertexArray>();
    vbos[name] = CreateUnique<VertexBuffer>(vertices);
    ebos[name] = CreateUnique<IndexBuffer>(indices);

    vaos.at(name)->Bind();
    vbos.at(name)->Bind();
    ebos.at(name)->Bind();
    Utils::ApplyVertexBufferLayout(layout);
  }

  void LoadPBRTextures(const string &name)
  {
    using namespace IO;
    Path base = Path("data/assets/pbr/") / Path(name);

    textures[name + "-albedo"] = CreateUnique<Texture2D>(base / Path("albedo.png"));
    textures.at(name + "-albedo")->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    textures.at(name + "-albedo")->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

    textures[name + "-normal"] = CreateUnique<Texture2D>(base / Path("normal.png"));
    textures.at(name + "-normal")->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    textures.at(name + "-normal")->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

    textures[name + "-metallic"] = CreateUnique<Texture2D>(base / Path("metallic.png"));
    textures.at(name + "-metallic")->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    textures.at(name + "-metallic")->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

    textures[name + "-roughness"] = CreateUnique<Texture2D>(base / Path("roughness.png"));
    textures.at(name + "-roughness")->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    textures.at(name + "-roughness")->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

    textures[name + "-ao"] = CreateUnique<Texture2D>(base / Path("ao.png"));
    textures.at(name + "-ao")->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    textures.at(name + "-ao")->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
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

      shaders["model"] = CreateUnique<Shader>(base / Path("model.vert"), base / Path("model.frag"));
      shaders["instanced-model"] =
        CreateUnique<Shader>(base / Path("instanced-model.vert"), base / Path("instanced-model.frag"));
      shaders["visualise-normals"] =
        CreateUnique<Shader>(base / Path("visualise-normals.vert"), base / Path("visualise-normals.geo"),
                             base / Path("visualise-normals.frag"));
      shaders["skybox"] = CreateUnique<Shader>(base / Path("skybox.vert"), base / Path("skybox.frag"));
      shaders["light-source"] =
        CreateUnique<Shader>(base / Path("lightsource.vert"), base / Path("lightsource.frag"));
      shaders["lighting"] = CreateUnique<Shader>(base / Path("basic.vert"), base / Path("lighting.frag"));
      shaders["flat-colour-phong-material"] =
        CreateUnique<Shader>(base / Path("basic.vert"), base / Path("flat-colour-phong-material.frag"));
      shaders["phong-material"] =
        CreateUnique<Shader>(base / Path("basic.vert"), base / Path("phong-material.frag"));
      shaders["directional-depth"] =
        CreateUnique<Shader>(base / Path("directional-shadow-map.vert"), base / Path("empty.frag"));
      shaders["point-depth"] =
        CreateUnique<Shader>(base / Path("point-shadows.vert"), base / Path("point-shadows.geo"),
                             base / Path("point-shadows.frag"));
      shaders["directional-shadow-mapping"] =
        CreateUnique<Shader>(base / Path("shadow-mapping.vert"), base / Path("shadow-mapping.frag"));
      shaders["point-shadow-mapping"] = CreateUnique<Shader>(base / Path("point-shadow-mapping.vert"),
                                                             base / Path("point-shadow-mapping.frag"));
      shaders["debug-quad"] = CreateUnique<Shader>(base / Path("debug-quad-shadow-map.vert"),
                                                   base / Path("debug-quad-shadow-map.frag"));
      shaders["normal-mapping"] =
        CreateUnique<Shader>(base / Path("normal-mapping.vert"), base / Path("normal-mapping.frag"));

      shaders["parallax-mapping"] =
        CreateUnique<Shader>(base / Path("parallax-mapping.vert"), base / Path("parallax-mapping.frag"));

      shaders["hdr"] = CreateUnique<Shader>(base / Path("hdr.vert"), base / Path("hdr.frag"));
      shaders["hdr-lighting"] =
        CreateUnique<Shader>(base / Path("hdr-test-lighting.vert"), base / Path("hdr-test-lighting.frag"));

      shaders["bloom"] = CreateUnique<Shader>(base / Path("7/bloom.vert"), base / Path("7/bloom.frag"));
      shaders["bloom-light"] =
        CreateUnique<Shader>(base / Path("7/bloom.vert"), base / Path("7/light-box.frag"));
      shaders["bloom-final"] =
        CreateUnique<Shader>(base / Path("7/bloom-final.vert"), base / Path("7/bloom-final.frag"));
      shaders["blur"] = CreateUnique<Shader>(base / Path("7/blur.vert"), base / Path("7/blur.frag"));

      shaders["g-buffer"] =
        CreateUnique<Shader>(base / Path("8/g-buffer.vert"), base / Path("8/g-buffer.frag"));
      shaders["deferred-shading"] =
        CreateUnique<Shader>(base / Path("8/deferred-shading.vert"), base / Path("8/deferred-shading.frag"));
      shaders["deferred-light"] = CreateUnique<Shader>(base / Path("8/deferred-light-box.vert"),
                                                       base / Path("8/deferred-light-box.frag"));

      shaders["ssao-geometry"] =
        CreateUnique<Shader>(base / Path("9/ssao-geometry.vert"), base / Path("9/ssao-geometry.frag"));
      shaders["ssao"] = CreateUnique<Shader>(base / Path("9/ssao.vert"), base / Path("9/ssao.frag"));
      shaders["ssao-blur"] =
        CreateUnique<Shader>(base / Path("9/ssao.vert"), base / Path("9/ssao-blur.frag"));
      shaders["ssao-lighting"] =
        CreateUnique<Shader>(base / Path("9/ssao.vert"), base / Path("9/ssao-lighting.frag"));

      shaders["pbr"] = CreateUnique<Shader>(base / Path("11/pbr.vert"), base / Path("11/pbr.frag"));
      shaders["pbr-with-maps"] =
        CreateUnique<Shader>(base / Path("11/pbr-with-maps.vert"), base / Path("11/pbr-with-maps.frag"));
      shaders["hdr-to-cubemap"] = CreateUnique<Shader>(base / Path("11/cubemap.vert"),
                                                       base / Path("11/equirectangular-to-cubemap.frag"));
      shaders["irradiance-convolution"] =
        CreateUnique<Shader>(base / Path("11/cubemap.vert"), base / Path("11/irradiance-convolution.frag"));
      shaders["prefilter"] =
        CreateUnique<Shader>(base / Path("11/cubemap.vert"), base / Path("11/prefilter.frag"));
      shaders["brdf"] = CreateUnique<Shader>(base / Path("11/brdf.vert"), base / Path("11/brdf.frag"));
      shaders["hdr-background"] =
        CreateUnique<Shader>(base / Path("11/background.vert"), base / Path("11/background.frag"));
    }

    {
      // models["backpack"] = CreateUnique<Model>(IO::Path("data/assets/models/backpack/backpack.obj"));
    }

    // Textures
    {
      using namespace IO;
      Path base = Path("data/assets");

      textures["wood"] = CreateUnique<Texture2D>(base / Path("wood.png"), IsSRGBTexture(true));
      textures.at("wood")->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
      textures.at("wood")->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

      textures["container"] =
        CreateUnique<Texture2D>(base / Path("container-diffuse.png"), IsSRGBTexture(true));

      textures["toybox-normal"] = CreateUnique<Texture2D>(base / Path("toybox-normal.png"));
      textures.at("toybox-normal")->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
      textures.at("toybox-normal")->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

      textures["toybox-displacement"] = CreateUnique<Texture2D>(base / Path("toybox-displacement.png"));
      textures.at("toybox-displacement")->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
      textures.at("toybox-displacement")->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

      textures["brick-diffuse"] = CreateUnique<Texture2D>(base / Path("brick-diffuse.jpg"));
      textures.at("brick-diffuse")->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
      textures.at("brick-diffuse")->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

      textures["brick-normal"] = CreateUnique<Texture2D>(base / Path("brick-normal.jpg"));
      textures.at("brick-normal")->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
      textures.at("brick-normal")->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

      textures["brickwall-diffuse"] = CreateUnique<Texture2D>(base / Path("brickwall-diffuse.jpg"));
      textures.at("brickwall-diffuse")->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
      textures.at("brickwall-diffuse")->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

      textures["brickwall-normal"] = CreateUnique<Texture2D>(base / Path("brickwall-normal.jpg"));
      textures.at("brickwall-normal")->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
      textures.at("brickwall-normal")->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

      textures["brickwall-displacement"] = CreateUnique<Texture2D>(base / Path("brickwall-displacement.jpg"));
      textures.at("brickwall-displacement")->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
      textures.at("brickwall-displacement")->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

      textures["hdr-environment"] =
        CreateUnique<Texture2D>(base / Path("newport-loft.hdr"), IsHDRTexture(true));
    }

    // PBR textures
    {
      using namespace IO;

      LoadPBRTextures("rusted-iron");
      LoadPBRTextures("gold");
      LoadPBRTextures("grass");
      LoadPBRTextures("plastic");
      LoadPBRTextures("wall");
    }

    // Cube
    {
      List<float> vertices = {
        // back face
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
        1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,  // bottom-right
        1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  // top-left
        // front face
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
        1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
        -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
        // left face
        -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
        -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top-left
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
        -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
                                                            // right face
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
        1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,    // top-right
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
        1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,    // bottom-left
        // bottom face
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
        1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // top-left
        1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
        1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
        -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
        // top face
        -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
        1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // top-right
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
        -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
        -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f   // bottom-left
      };

      VertexBufferLayout layout = {
        {VertexAttributeType::Float, 3}, // position
        {VertexAttributeType::Float, 3}, // normal
        {VertexAttributeType::Float, 2}  // texcoord
      };

      CreateVertexArray("cube", ByteUtils::AsBytesView(vertices), layout);
    }

    // Plane
    {
      List<float> vertices = {// positions            // normals         // texcoords
                              25.0f, -0.5f, 25.0f,  0.0f,  1.0f,   0.0f,  25.0f,  0.0f,  -25.0f, -0.5f,
                              25.0f, 0.0f,  1.0f,   0.0f,  0.0f,   0.0f,  -25.0f, -0.5f, -25.0f, 0.0f,
                              1.0f,  0.0f,  0.0f,   25.0f, 25.0f,  -0.5f, 25.0f,  0.0f,  1.0f,   0.0f,
                              25.0f, 0.0f,  -25.0f, -0.5f, -25.0f, 0.0f,  1.0f,   0.0f,  0.0f,   25.0f,
                              25.0f, -0.5f, -25.0f, 0.0f,  1.0f,   0.0f,  25.0f,  25.0f};

      VertexBufferLayout layout = {
        {VertexAttributeType::Float, 3}, // position
        {VertexAttributeType::Float, 3}, // normal
        {VertexAttributeType::Float, 2}  // texcoord
      };

      CreateVertexArray("plane", ByteUtils::AsBytesView(vertices), layout);
    }

    // Quad with pos, normal, tex coords, tangent, bitangent
    {
      // positions
      Vec3 pos1(-1.0f, 1.0f, 0.0f);
      Vec3 pos2(-1.0f, -1.0f, 0.0f);
      Vec3 pos3(1.0f, -1.0f, 0.0f);
      Vec3 pos4(1.0f, 1.0f, 0.0f);
      // texture coordinates
      Vec2 uv1(0.0f, 1.0f);
      Vec2 uv2(0.0f, 0.0f);
      Vec2 uv3(1.0f, 0.0f);
      Vec2 uv4(1.0f, 1.0f);
      // normal vector
      Vec3 nm(0.0f, 0.0f, 1.0f);

      // calculate tangent/bitangent vectors of both triangles
      Vec3 tangent1, bitangent1;
      Vec3 tangent2, bitangent2;
      // triangle 1
      // ----------
      Vec3 edge1 = pos2 - pos1;
      Vec3 edge2 = pos3 - pos1;
      Vec2 deltaUV1 = uv2 - uv1;
      Vec2 deltaUV2 = uv3 - uv1;

      float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

      tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
      tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
      tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

      bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
      bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
      bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

      // triangle 2
      // ----------
      edge1 = pos3 - pos1;
      edge2 = pos4 - pos1;
      deltaUV1 = uv3 - uv1;
      deltaUV2 = uv4 - uv1;

      f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

      tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
      tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
      tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

      bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
      bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
      bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

      List<float> vertices = {
        // positions            // normal         // texcoords  // tangent                          //
        // bitangent
        pos1.x, pos1.y,     pos1.z,     nm.x,       nm.y,         nm.z,         uv1.x,
        uv1.y,  tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
        pos2.x, pos2.y,     pos2.z,     nm.x,       nm.y,         nm.z,         uv2.x,
        uv2.y,  tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
        pos3.x, pos3.y,     pos3.z,     nm.x,       nm.y,         nm.z,         uv3.x,
        uv3.y,  tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

        pos1.x, pos1.y,     pos1.z,     nm.x,       nm.y,         nm.z,         uv1.x,
        uv1.y,  tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
        pos3.x, pos3.y,     pos3.z,     nm.x,       nm.y,         nm.z,         uv3.x,
        uv3.y,  tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
        pos4.x, pos4.y,     pos4.z,     nm.x,       nm.y,         nm.z,         uv4.x,
        uv4.y,  tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z};

      VertexBufferLayout layout = {
        {VertexAttributeType::Float, 3}, // position
        {VertexAttributeType::Float, 3}, // normal
        {VertexAttributeType::Float, 2}, // texcoord
        {VertexAttributeType::Float, 3}, // tangent
        {VertexAttributeType::Float, 3}, // bitangent
      };

      CreateVertexArray("quad", ByteUtils::AsBytesView(vertices), layout);
    }

    // Screen quad
    {
      List<float> vertices = {
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
      };

      VertexBufferLayout layout = {
        {VertexAttributeType::Float, 2}, // position
        {VertexAttributeType::Float, 2}  // texcoord
      };

      CreateVertexArray("screen-quad", ByteUtils::AsBytesView(vertices), layout);
    }

    // Sphere
    {
      List<Vec3> positions;
      List<Vec2> uv;
      List<Vec3> normals;
      List<uint> indices;

      const uint X_SEGMENTS = 64;
      const uint Y_SEGMENTS = 64;
      const float PI = 3.14159265359f;
      for (uint x = 0; x <= X_SEGMENTS; ++x)
      {
        for (uint y = 0; y <= Y_SEGMENTS; ++y)
        {
          float xSegment = (float)x / (float)X_SEGMENTS;
          float ySegment = (float)y / (float)Y_SEGMENTS;
          float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
          float yPos = std::cos(ySegment * PI);
          float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

          positions.push_back(Vec3(xPos, yPos, zPos));
          uv.push_back(Vec2(xSegment, ySegment));
          normals.push_back(Vec3(xPos, yPos, zPos));
        }
      }

      bool oddRow = false;
      for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
      {
        if (!oddRow) // even rows: y == 0, y == 2; and so on
        {
          for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
          {
            indices.push_back(y * (X_SEGMENTS + 1) + x);
            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
          }
        }
        else
        {
          for (int x = X_SEGMENTS; x >= 0; --x)
          {
            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            indices.push_back(y * (X_SEGMENTS + 1) + x);
          }
        }
        oddRow = !oddRow;
      }
      sphereIndexCount = static_cast<uint>(indices.size());

      List<float> vertices;
      for (uint i = 0; i < positions.size(); ++i)
      {
        vertices.push_back(positions[i].x);
        vertices.push_back(positions[i].y);
        vertices.push_back(positions[i].z);
        if (normals.size() > 0)
        {
          vertices.push_back(normals[i].x);
          vertices.push_back(normals[i].y);
          vertices.push_back(normals[i].z);
        }
        if (uv.size() > 0)
        {
          vertices.push_back(uv[i].x);
          vertices.push_back(uv[i].y);
        }
      }

      VertexBufferLayout layout = {
        {VertexAttributeType::Float, 3}, // position
        {VertexAttributeType::Float, 3}, // normal
        {VertexAttributeType::Float, 2}  // texcoord
      };

      CreateVertexArray("sphere", ByteUtils::AsBytesView(vertices), ByteUtils::AsBytesView(indices), layout);
    }

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

    CreateEnvironmentAndIrradianceCubemaps(1'024, 1'024);
    glViewport(0, 0, _width, _height);

    shaders.at("pbr-with-maps")->Bind();
    shaders.at("pbr-with-maps")->SetUniform("irradianceMap", 0);
    shaders.at("pbr-with-maps")->SetUniform("prefilterMap", 1);
    shaders.at("pbr-with-maps")->SetUniform("brdfLUT", 2);
    shaders.at("pbr-with-maps")->SetUniform("albedoMap", 3);
    shaders.at("pbr-with-maps")->SetUniform("normalMap", 4);
    shaders.at("pbr-with-maps")->SetUniform("metallicMap", 5);
    shaders.at("pbr-with-maps")->SetUniform("roughnessMap", 6);
    shaders.at("pbr-with-maps")->SetUniform("aoMap", 7);

    shaders.at("hdr-background")->Bind();
    shaders.at("hdr-background")->SetUniform("environmentMap", 0);
  }

  void Context::Render(ICamera &camera) noexcept
  {
    auto view = camera.ViewMatrix();
    auto projection = camera.ProjectionMatrix();
    ubos.at("matrices")->Update(List<Mat4>{view, projection});
    shaders.at("hdr-background")->SetUniform("projection", projection);

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

    auto &shader = shaders.at("pbr-with-maps");
    shader->Bind();
    shader->SetUniform("camPos", camera.Position());

    vaos.at("sphere")->Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMaps["irradiance-cubemap"].ColorTextures[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMaps["prefilter-cubemap"].ColorTextures[0]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, shadowMaps["brdf-lut"].ColorTextures[0]);

    // rusted iron
    {
      textures.at("rusted-iron-albedo")->Bind(3);
      textures.at("rusted-iron-normal")->Bind(4);
      textures.at("rusted-iron-metallic")->Bind(5);
      textures.at("rusted-iron-roughness")->Bind(6);
      textures.at("rusted-iron-ao")->Bind(7);

      Mat4 model = Identity<Mat4>();
      model = Translate(model, Vec3(-5.0, 0.0, 2.0));
      shader->SetUniform("model", model);
      shader->SetUniform("normalMatrix", Transpose(Inverse(Mat3(model))));
      Utils::DrawElements(GL_TRIANGLE_STRIP, sphereIndexCount);
    }

    // gold
    {
      textures.at("gold-albedo")->Bind(3);
      textures.at("gold-normal")->Bind(4);
      textures.at("gold-metallic")->Bind(5);
      textures.at("gold-roughness")->Bind(6);
      textures.at("gold-ao")->Bind(7);

      Mat4 model = Identity<Mat4>();
      model = Translate(model, Vec3(-3.0, 0.0, 2.0));
      shader->SetUniform("model", model);
      shader->SetUniform("normalMatrix", Transpose(Inverse(Mat3(model))));
      Utils::DrawElements(GL_TRIANGLE_STRIP, sphereIndexCount);
    }

    // grass
    {
      textures.at("grass-albedo")->Bind(3);
      textures.at("grass-normal")->Bind(4);
      textures.at("grass-metallic")->Bind(5);
      textures.at("grass-roughness")->Bind(6);
      textures.at("grass-ao")->Bind(7);

      Mat4 model = Identity<Mat4>();
      model = Translate(model, Vec3(-1.0, 0.0, 2.0));
      shader->SetUniform("model", model);
      shader->SetUniform("normalMatrix", Transpose(Inverse(Mat3(model))));
      Utils::DrawElements(GL_TRIANGLE_STRIP, sphereIndexCount);
    }

    // plastic
    {
      textures.at("plastic-albedo")->Bind(3);
      textures.at("plastic-normal")->Bind(4);
      textures.at("plastic-metallic")->Bind(5);
      textures.at("plastic-roughness")->Bind(6);
      textures.at("plastic-ao")->Bind(7);

      Mat4 model = Identity<Mat4>();
      model = Translate(model, Vec3(1.0, 0.0, 2.0));
      shader->SetUniform("model", model);
      shader->SetUniform("normalMatrix", Transpose(Inverse(Mat3(model))));
      Utils::DrawElements(GL_TRIANGLE_STRIP, sphereIndexCount);
    }

    // wall
    {
      textures.at("wall-albedo")->Bind(3);
      textures.at("wall-normal")->Bind(4);
      textures.at("wall-metallic")->Bind(5);
      textures.at("wall-roughness")->Bind(6);
      textures.at("wall-ao")->Bind(7);

      Mat4 model = Identity<Mat4>();
      model = Translate(model, Vec3(3.0, 0.0, 2.0));
      shader->SetUniform("model", model);
      shader->SetUniform("normalMatrix", Transpose(Inverse(Mat3(model))));
      Utils::DrawElements(GL_TRIANGLE_STRIP, sphereIndexCount);
    }

    // render light source (simply re-render sphere at light positions)
    // this looks a bit off as we use the same shader, but it'll make their positions obvious and
    // keeps the codeprint small.
    for (uint i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
    {
      Vec3 newPos = lightPositions[i] + Vec3(std::sin((float)Platform::GetTime() * 5.0f) * 5.0f, 0.0f, 0.0f);
      newPos = lightPositions[i];
      shader->SetUniform("lightPositions[" + std::to_string(i) + "]", newPos);
      shader->SetUniform("lightColors[" + std::to_string(i) + "]", lightColors[i]);

      Mat4 model = Identity<Mat4>();
      model = Translate(model, newPos);
      model = Scale(model, Vec3(0.5f));
      shader->SetUniform("model", model);
      shader->SetUniform("normalMatrix", Transpose(Inverse(Mat3(model))));
      Utils::DrawElements(GL_TRIANGLE_STRIP, sphereIndexCount);
    }

    {
      auto &shader = shaders.at("hdr-background");
      vaos.at("cube")->Bind();
      shader->Bind();
      shader->SetUniform("view", view);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMaps["hdr-cubemap"].ColorTextures[0]);
      Utils::Draw(GL_TRIANGLES, 36);
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
}
