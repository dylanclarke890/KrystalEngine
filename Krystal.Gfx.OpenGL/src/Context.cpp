#include "Krystal.Gfx.OpenGL/Context.hpp"

#include "Krystal.Core/Core.hpp"
#include "Krystal.Core/Detection.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.IO/Streams/NativeFileStream.hpp"
#include "Krystal.IO/Streams/StreamUtils.hpp"

#ifdef KRYS_PLATFORM_WINDOWS
  #include "Krystal.Gfx.OpenGL/Win32/ContextPlatformImpl.hpp"
  #undef CreateWindow
  #undef LoadImage
  #undef min
  #undef max
#else
  #error "Unsupported platform for OpenGL context creation."
#endif

#include "Krystal.Gfx.OpenGL/Buffer.hpp"
#include "Krystal.Gfx.OpenGL/Hooks/gl.hpp"
#include "Krystal.Gfx.OpenGL/Model.hpp"
#include "Krystal.Gfx.OpenGL/Shader.hpp"
#include "Krystal.Gfx.OpenGL/Texture.hpp"
#include "Krystal.Gfx.OpenGL/Utils.hpp"
#include "Krystal.Gfx.OpenGL/VertexArray.hpp"
#include "Krystal.Gfx/Light.hpp"
#include "Krystal.Gfx/Material.hpp"
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

  struct ShadowMapData
  {
    GLuint FBO {};
    GLuint Texture {};
    uint32 Width {};
    uint32 Height {};
    Mat4 LightSpaceMatrix {};
  };

  static Map<string, Unique<Shader>> shaders;
  static Map<string, Unique<Texture2D>> textures;
  static Map<string, Unique<CubeMap>> cubemaps;
  static Map<string, Unique<Model>> models;
  static Map<string, Unique<VertexArray>> vaos;
  static Map<string, Unique<VertexBuffer>> vbos;
  static Map<string, Unique<UniformBuffer>> ubos;
  static Map<string, ShadowMapData> shadowMaps;

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

  static Vec3 lightPos(-2.0f, 4.0f, -1.0f);

  static float nearPlane = 1.0f;
  static float farPlane = 7.5f;

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

    Mat4 lightProjection = Ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
    Mat4 lightView = LookAt(lightPos, Vec3(0.0f), Vec3(0.0f, 1.0f, 0.0f));

    shadowMaps[name] = {depthMapFBO, depthMap, width, height, lightProjection * lightView};

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE
           && "framebuffer is incomplete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
      shaders["depth"] =
        CreateUnique<Shader>(base / Path("directional-shadow-map.vert"), base / Path("empty.frag"));
      shaders["shadow-mapping"] =
        CreateUnique<Shader>(base / Path("shadow-mapping.vert"), base / Path("shadow-mapping.frag"));
      shaders["debug-quad"] = CreateUnique<Shader>(base / Path("debug-quad-shadow-map.vert"),
                                                   base / Path("debug-quad-shadow-map.frag"));
    }

    // Textures
    {
      using namespace IO;
      Path base = Path("data/assets");

      textures["wood"] = CreateUnique<Texture2D>(base / Path("wood.png"));
      textures.at("wood")->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
      textures.at("wood")->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    // Buffers
    {
      ubos["matrices"] = CreateUnique<UniformBuffer>(2 * sizeof(Mat4));
      ubos["matrices"]->Bind(0);
    }

    // Cube
    {
      vaos["cube"] = CreateUnique<VertexArray>();
      vaos["cube"]->Bind();

      List<float> vertices = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.5f,  -0.5f, -0.5f, 0.0f,
        0.0f,  -1.0f, 1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f,  1.0f,
        0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f,  1.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,
        0.0f,  -1.0f, 0.0f,  1.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,

        -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,
        0.0f,  1.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,  -0.5f, 0.5f,  0.5f,  0.0f,
        0.0f,  1.0f,  0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, -1.0f,
        0.0f,  0.0f,  1.0f,  1.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  -1.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 1.0f,
        0.0f,  0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  1.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 0.0f,
        -1.0f, 0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  1.0f,  0.0f,
        0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  1.0f,  0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,
        -1.0f, 0.0f,  0.0f,  0.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  1.0f,

        -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  -0.5f, 0.0f,
        1.0f,  0.0f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  0.0f,
        1.0f,  0.0f,  0.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f};
      vbos["cube"] = CreateUnique<VertexBuffer>(vertices);
      vbos["cube"]->Bind();
      Utils::ApplyVertexBufferLayout({
        {VertexAttributeType::Float, 3}, // position
        {VertexAttributeType::Float, 3}, // normal
        {VertexAttributeType::Float, 2}  // texcoord
      });
    }

    // Plane
    {
      vaos["plane"] = CreateUnique<VertexArray>();
      vaos["plane"]->Bind();

      List<float> vertices = {// positions            // normals         // texcoords
                              25.0f, -0.5f, 25.0f,  0.0f,  1.0f,   0.0f,  25.0f,  0.0f,  -25.0f, -0.5f,
                              25.0f, 0.0f,  1.0f,   0.0f,  0.0f,   0.0f,  -25.0f, -0.5f, -25.0f, 0.0f,
                              1.0f,  0.0f,  0.0f,   25.0f, 25.0f,  -0.5f, 25.0f,  0.0f,  1.0f,   0.0f,
                              25.0f, 0.0f,  -25.0f, -0.5f, -25.0f, 0.0f,  1.0f,   0.0f,  0.0f,   25.0f,
                              25.0f, -0.5f, -25.0f, 0.0f,  1.0f,   0.0f,  25.0f,  25.0f};

      vbos["plane"] = CreateUnique<VertexBuffer>(vertices);
      vbos["plane"]->Bind();
      Utils::ApplyVertexBufferLayout({
        {VertexAttributeType::Float, 3}, // position
        {VertexAttributeType::Float, 3}, // normal
        {VertexAttributeType::Float, 2}  // texcoord
      });
    }

    // Screen quad
    {
      vaos["quad"] = CreateUnique<VertexArray>();
      vaos["quad"]->Bind();
      List<float> vertices = {
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
      };
      vbos["quad"] = CreateUnique<VertexBuffer>(vertices);
      vbos["quad"]->Bind();
      Utils::ApplyVertexBufferLayout({
        {VertexAttributeType::Float, 2}, // position
        {VertexAttributeType::Float, 2}  // texcoord
      });
    }

    // Shadow maps
    {
      CreateShadowMapFramebuffer("directional", 1'024, 1'024);
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    shaders.at("shadow-mapping")->Bind();
    shaders.at("shadow-mapping")->SetUniform("diffuseTexture", 0);
    shaders.at("shadow-mapping")->SetUniform("shadowMap", 1);

    shaders.at("debug-quad")->Bind();
    shaders.at("debug-quad")->SetUniform("depthMap", 0);
  }

  void Context::Render(ICamera &camera) noexcept
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto view = camera.ViewMatrix();
    auto projection = camera.ProjectionMatrix();

    ubos.at("matrices")->Update(ByteUtils::AsBytesView(view));
    ubos.at("matrices")->Update(ByteUtils::AsBytesView(projection), sizeof(Mat4));

    {
      Mat4 lightSpaceMatrix = shadowMaps["directional"].LightSpaceMatrix;

      auto &shader = shaders.at("depth");
      shader->Bind();
      shader->SetUniform("lightSpaceMatrix", lightSpaceMatrix);
      glViewport(0, 0, shadowMaps["directional"].Width, shadowMaps["directional"].Height);
      glBindFramebuffer(GL_FRAMEBUFFER, shadowMaps["directional"].FBO);
      glClear(GL_DEPTH_BUFFER_BIT);
      RenderScene(*shader);
    }

    {
      auto &shader = shaders.at("shadow-mapping");
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glViewport(0, 0, _width, _height);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      shader->Bind();
      shader->SetUniform("lightSpaceMatrix", shadowMaps["directional"].LightSpaceMatrix);
      shader->SetUniform("viewPos", camera.Position());
      shader->SetUniform("lightPos", lightPos);

      textures.at("wood")->Bind(0);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, shadowMaps["directional"].Texture);
      RenderScene(*shader);
    }

    {
      auto &shader = shaders.at("debug-quad");
      shader->Bind();
      shader->SetUniform("near_plane", nearPlane);
      shader->SetUniform("far_plane", farPlane);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, shadowMaps["directional"].Texture);

      vaos.at("quad")->Bind();
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
