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

  static Map<string, Unique<Shader>> shaders;
  static Map<string, Unique<Texture2D>> textures;
  static Map<string, Unique<CubeMap>> cubemaps;
  static Map<string, Unique<Model>> models;
  static Map<string, Unique<VertexArray>> vaos;
  static Map<string, Unique<VertexBuffer>> vbos;
  static Map<string, Unique<UniformBuffer>> ubos;
  static Map<string, FrameBufferData> shadowMaps;

  static FrameBufferData pingPongFBOs[2];

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

  static void CreateShadowMapFramebuffer(const string &name, uint32 width, uint32 height) noexcept
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

  static void CreateCubeShadowMapFramebuffer(const string &name, uint32 width, uint32 height) noexcept
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

  static void CreateFloatingPointFramebuffer(const string &name, uint32 width, uint32 height)
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

  static void CreatePingPongFramebuffer(const string &name, uint32 width, uint32 height)
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

  static void RenderScene(Shader &shader)
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

  static void CreateVertexArray(const string &name, const BufferData &vertices,
                                const VertexBufferLayout &layout) noexcept
  {
    vaos[name] = CreateUnique<VertexArray>();
    vbos[name] = CreateUnique<VertexBuffer>(vertices);

    vaos.at(name)->Bind();
    vbos.at(name)->Bind();
    Utils::ApplyVertexBufferLayout(layout);
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

    // Shadow maps
    CreateShadowMapFramebuffer("directional", 1'024, 1'024);
    CreateCubeShadowMapFramebuffer("point", 1'024, 1'024);

    CreateFloatingPointFramebuffer("bloom", _width, _height);
    CreatePingPongFramebuffer("blur", _width, _height);
    pingPongFBOs[0] = shadowMaps.at("blur0");
    pingPongFBOs[1] = shadowMaps.at("blur1");

    // Uniform buffers
    {
      ubos["matrices"] = CreateUnique<UniformBuffer>(2 * sizeof(Mat4));
      ubos.at("matrices")->Bind(0);
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.f, 0.f, 0.f, 1.f);

    shaders.at("bloom")->SetUniform("diffuseTexture", 0);
    shaders.at("blur")->SetUniform("image", 0);
    shaders.at("bloom-final")->SetUniform("scene", 0);
    shaders.at("bloom-final")->SetUniform("bloomBlur", 1);
  }

  void Context::Render(ICamera &camera) noexcept
  {
    auto view = camera.ViewMatrix();
    auto projection = camera.ProjectionMatrix();

    ubos.at("matrices")->Update(view);
    ubos.at("matrices")->Update(projection, sizeof(Mat4));

    std::vector<Vec3> lightPositions;
    lightPositions.push_back(Vec3(0.0f, 0.5f, 1.5f));
    lightPositions.push_back(Vec3(-4.0f, 0.5f, -3.0f));
    lightPositions.push_back(Vec3(3.0f, 0.5f, 1.0f));
    lightPositions.push_back(Vec3(-.8f, 2.4f, -1.0f));

    // colors
    std::vector<Vec3> lightColors;
    lightColors.push_back(Vec3(5.0f, 5.0f, 5.0f));
    lightColors.push_back(Vec3(10.0f, 0.0f, 0.0f));
    lightColors.push_back(Vec3(0.0f, 0.0f, 15.0f));
    lightColors.push_back(Vec3(0.0f, 5.0f, 0.0f));

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMaps.at("bloom").FBO);
    {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      auto &shader = shaders.at("hdr-lighting");
      shader->Bind();

      // colors

      for (uint i = 0; i < lightPositions.size(); i++)
      {
        shader->SetUniform("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
        shader->SetUniform("lights[" + std::to_string(i) + "].Color", lightColors[i]);
      }
      shader->SetUniform("viewPos", camera.Position());
      vaos.at("cube")->Bind();

      // create one large cube that acts as the floor
      textures.at("wood")->Bind(0);
      {
        Mat4 model = Identity<Mat4>();
        model = Translate(model, Vec3(0.0f, -1.0f, 0.0));
        model = Scale(model, Vec3(12.5f, 0.5f, 12.5f));
        shader->SetUniform("model", model);
        Utils::Draw(GL_TRIANGLES, 36);
      }

      // then create multiple cubes as the scenery
      textures.at("container")->Bind(0);
      {
        Mat4 model = Identity<Mat4>();
        model = Translate(model, Vec3(0.0f, 1.5f, 0.0));
        model = Scale(model, Vec3(0.5f));
        shader->SetUniform("model", model);
        Utils::Draw(GL_TRIANGLES, 36);
      }

      {
        Mat4 model = Identity<Mat4>();
        model = Translate(model, Vec3(2.0f, 0.0f, 1.0));
        model = Scale(model, Vec3(0.5f));
        shader->SetUniform("model", model);
        Utils::Draw(GL_TRIANGLES, 36);
      }

      {
        Mat4 model = Identity<Mat4>();
        model = Translate(model, Vec3(-1.0f, -1.0f, 2.0));
        model = Rotate(model, Radians(60.0f), Normalize(Vec3(1.0, 0.0, 1.0)));
        shader->SetUniform("model", model);
        Utils::Draw(GL_TRIANGLES, 36);
      }

      {
        Mat4 model = Identity<Mat4>();
        model = Translate(model, Vec3(0.0f, 2.7f, 4.0));
        model = Rotate(model, Radians(23.0f), Normalize(Vec3(1.0, 0.0, 1.0)));
        model = Scale(model, Vec3(1.25));
        shader->SetUniform("model", model);
        Utils::Draw(GL_TRIANGLES, 36);
      }

      {
        Mat4 model = Identity<Mat4>();
        model = Translate(model, Vec3(-2.0f, 1.0f, -3.0));
        model = Rotate(model, Radians(124.0f), Normalize(Vec3(1.0, 0.0, 1.0)));
        shader->SetUniform("model", model);
        Utils::Draw(GL_TRIANGLES, 36);
      }

      {
        Mat4 model = Identity<Mat4>();
        model = Translate(model, Vec3(-3.0f, 0.0f, 0.0));
        model = Scale(model, Vec3(0.5f));
        shader->SetUniform("model", model);
        Utils::Draw(GL_TRIANGLES, 36);
      }
    }

    {
      auto &shader = shaders.at("bloom-light");
      shader->Bind();
      for (uint i = 0; i < lightPositions.size(); i++)
      {
        Mat4 model = Identity<Mat4>();
        model = Translate(model, lightPositions[i]);
        model = Scale(model, Vec3(0.25f));
        shader->SetUniform("model", model);
        shader->SetUniform("lightColor", lightColors[i]);
        Utils::Draw(GL_TRIANGLES, 36);
      }
    }

    vaos.at("screen-quad")->Bind();
    bool horizontal = true;
    {
      auto &shader = shaders.at("blur");
      shader->Bind();

      uint amount = 10;
      for (uint i = 0; i < amount; i++)
      {
        glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBOs[horizontal].FBO);
        shader->SetUniform("horizontal", horizontal);
        glActiveTexture(GL_TEXTURE0);
        GLuint texture = i == 0
                           ? shadowMaps.at("bloom").ColorTextures[1]     // (or scene if first iteration)
                           : pingPongFBOs[!horizontal].ColorTextures[0]; // bind texture of other framebuffer
        glBindTexture(GL_TEXTURE_2D, texture);

        Utils::Draw(GL_TRIANGLE_STRIP, 4);
        horizontal = !horizontal;
      }
    }

    //{
    //  vaos.at("cube")->Bind();
    //  glBindFramebuffer(GL_FRAMEBUFFER, shadowMaps.at("bloom").FBO);
    //  auto &shader = shaders.at("bloom-light");
    //  shader->Bind();
    //  shader->SetUniform("countsTowardsBloom", false);
    //  for (uint i = 0; i < lightPositions.size(); i++)
    //  {
    //    Mat4 model = Identity<Mat4>();
    //    model = Translate(model, lightPositions[i]);
    //    model = Scale(model, Vec3(0.275f));
    //    shader->SetUniform("model", model);
    //    shader->SetUniform("lightColor", lightColors[i]);
    //    Utils::Draw(GL_TRIANGLES, 36);
    //  }
    //  shader->SetUniform("countsTowardsBloom", true);
    //}

    // 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's
    // (clamped) color range
    // --------------------------------------------------------------------------------------------------------------------------
    {
      vaos.at("screen-quad")->Bind();
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      auto &shader = shaders.at("bloom-final");
      shader->Bind();
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, shadowMaps.at("bloom").ColorTextures[0]);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, pingPongFBOs[horizontal].ColorTextures[0]);
      shader->SetUniform("bloom", true);
      shader->SetUniform("exposure", 1.f);
      Utils::Draw(GL_TRIANGLE_STRIP, 4);
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
