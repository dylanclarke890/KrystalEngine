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

  struct FramebufferData
  {
    GLuint FBO;
    GLuint Texture;
    GLuint RBO;
    uint32 Width;
    uint32 Height;
  };

  static Map<string, Unique<Shader>> shaders;
  static Map<string, Unique<Texture2D>> textures;
  static Map<string, Unique<CubeMap>> cubemaps;
  static Map<string, Unique<Model>> models;
  static Map<string, Unique<VertexArray>> vaos;
  static Map<string, Unique<VertexBuffer>> vbos;
  static Map<string, Unique<UniformBuffer>> ubos;
  static Map<string, FramebufferData> framebuffers;

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

  static List<float> skyboxVertices = {
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

  static uint32 instanceCount = 400'000;

  static void CreateFramebuffer(const string &name, uint32 width, uint32 height)
  {
    GLuint framebuffer;
    glCreateFramebuffers(1, &framebuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Create a color attachment texture
    GLuint textureColorbuffer;
    glCreateTextures(GL_TEXTURE_2D, 1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTextureParameteri(textureColorbuffer, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(textureColorbuffer, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE
           && "framebuffer is incomplete");

    framebuffers[name] = {framebuffer, textureColorbuffer, rbo, width, height};

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    }

    // Textures
    {
      using namespace IO;
      Path base = Path("data/assets");
    }

    // Cubemaps
    {
      using namespace IO;
      Path base = Path("data/assets/skyboxes/sky");

      cubemaps["sky"] =
        CreateUnique<CubeMap>(base / Path("left.jpg"), base / Path("right.jpg"), base / Path("top.jpg"),
                              base / Path("bottom.jpg"), base / Path("front.jpg"), base / Path("back.jpg"));
    }

    // Models
    {
      using namespace IO;

      Path base = Path("data/assets/models");
      models["rock"] = CreateUnique<Model>(base / Path("rock/rock.obj"));
      models["planet"] = CreateUnique<Model>(base / Path("planet/planet.obj"));
    }

    // Buffers
    {
      vaos["skybox"] = CreateUnique<VertexArray>();
      vbos["skybox"] = CreateUnique<VertexBuffer>(skyboxVertices);
      vaos["skybox"]->AddVertexBuffer(vbos["skybox"].get(), {{VertexAttributeType::Float, 3}});

      ubos["matrices"] = CreateUnique<UniformBuffer>(2 * sizeof(Mat4));
      ubos["matrices"]->Bind(0);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    // glEnable(GL_FRAMEBUFFER_SRGB);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    List<Mat4> modelMatrices(instanceCount);
    srand((uint)Platform::GetTime()); // initialize random seed
    float radius = 150.0f;
    float offset = 25.f;
    for (uint i = 0; i < instanceCount; i++)
    {
      Mat4 model = Identity<Mat4>();
      // 1. translation: displace along circle with 'radius' in range [-offset, offset]
      float angle = (float)i / (float)instanceCount * 360.0f;
      float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
      float x = sin(angle) * radius + displacement;
      displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
      float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
      displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
      float z = cos(angle) * radius + displacement;
      model = Translate(model, Vec3(x, y, z));

      // 2. scale: scale between 0.05 and 0.25f
      float scale = (rand() % 20) / 100.0f + 0.05f;
      model = Scale(model, Vec3(scale));

      // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
      float rotAngle = (float)(rand() % 360);
      model = Rotate(model, rotAngle, Vec3(0.4f, 0.6f, 0.8f));

      // 4. now add to list of matrices
      modelMatrices[i] = model;
    }
    vbos["model-matrices"] = CreateUnique<VertexBuffer>(modelMatrices);
    vbos["model-matrices"]->Bind();
    models["rock"]->ApplyVertexLayout({{VertexAttributeType::Float, 4, VertexInputRate::PerInstance},
                                       {VertexAttributeType::Float, 4, VertexInputRate::PerInstance},
                                       {VertexAttributeType::Float, 4, VertexInputRate::PerInstance},
                                       {VertexAttributeType::Float, 4, VertexInputRate::PerInstance}});
  }

  void Context::Render(ICamera &camera) noexcept
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto view = camera.ViewMatrix();
    auto projection = camera.ProjectionMatrix();

    ubos.at("matrices")->Update(ByteUtils::AsBytesView(view));
    ubos.at("matrices")->Update(ByteUtils::AsBytesView(projection), sizeof(Mat4));

    {
      auto &shader = shaders.at("instanced-model");
      shader->Bind();
      models.at("rock")->DrawInstanced(*shader, instanceCount);
    }
  }

  void Context::Present() noexcept
  {
    _platformImpl->Present();
  }

  void Context::Resize(uint32 width, uint32 height) noexcept
  {
    glViewport(0, 0, width, height);
  }
}
