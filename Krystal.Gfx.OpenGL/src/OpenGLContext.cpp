#include "Krystal.Gfx.OpenGL/OpenGLContext.hpp"

#include "Krystal.Core/Core.hpp"
#include "Krystal.Core/Detection.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.IO/Streams/NativeFileStream.hpp"
#include "Krystal.IO/Streams/StreamUtils.hpp"

#ifdef KRYS_PLATFORM_WINDOWS
  #include "Krystal.Gfx.OpenGL/Win32/GLContextPlatformImpl.hpp"
  #undef CreateWindow
  #undef LoadImage
  #undef min
  #undef max
#else
  #error "Unsupported platform for OpenGL context creation."
#endif

#include "Krystal.Gfx.OpenGL/Hooks/gl.hpp"
#include "Krystal.Gfx.OpenGL/OpenGLModel.hpp"
#include "Krystal.Gfx.OpenGL/OpenGLShader.hpp"
#include "Krystal.Gfx.OpenGL/OpenGLTexture.hpp"
#include "Krystal.Gfx.OpenGL/Utils.hpp"
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

  static Map<string, Unique<OpenGLShader>> shaders;
  static Map<string, Unique<OpenGLTexture2D>> textures;
  static Map<string, Unique<OpenGLModel>> models;
  static Map<string, GLuint> vaos;
  static Map<string, GLuint> vbos;

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

#pragma endregion

  static float vertices[] = {
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

  static Vec3 cubePositions[] = {{0.0f, 0.0f, 0.0f},     {2.0f, 5.0f, -15.0f}, {-1.5f, -2.2f, -2.5f},
                                 {-3.8f, -2.0f, -12.3f}, {2.4f, -0.4f, -3.5f}, {-1.7f, 3.0f, -7.5f},
                                 {1.3f, -2.0f, -2.5f},   {1.5f, 2.0f, -2.5f},  {1.5f, 0.2f, -1.5f},
                                 {-1.3f, 1.0f, -1.5f}};

  static void CreateVertexArray(const string &name, const float *vertices, size_t vertexCount,
                                const VertexBufferLayout &layout) noexcept
  {
    GLuint vao;
    glCreateVertexArrays(1, &vao);
    vaos[name] = vao;

    glBindVertexArray(vao);

    GLuint vbo;
    glCreateBuffers(1, &vbo);
    vbos[name] = vbo;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glNamedBufferData(vbo, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW);
    Utils::ApplyVertexBufferLayout(layout);
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
    // Shaders
    {
      using namespace IO;

      Path base = Path("data/shaders/opengl");
      shaders["skybox"] = CreateUnique<OpenGLShader>(base / Path("skybox.vert"), base / Path("skybox.frag"));
      shaders["light-source"] =
        CreateUnique<OpenGLShader>(base / Path("lightsource.vert"), base / Path("lightsource.frag"));
      shaders["lighting"] =
        CreateUnique<OpenGLShader>(base / Path("basic.vert"), base / Path("lighting.frag"));
      shaders["flat-colour-phong-material"] =
        CreateUnique<OpenGLShader>(base / Path("basic.vert"), base / Path("flat-colour-phong-material.frag"));
      shaders["phong-material"] =
        CreateUnique<OpenGLShader>(base / Path("basic.vert"), base / Path("phong-material.frag"));
      shaders["backpack"] =
        CreateUnique<OpenGLShader>(base / Path("backpack.vert"), base / Path("backpack.frag"));
    }

    // Textures
    {
      using namespace IO;

      Path base = Path("data/assets");
      textures["wall"] = CreateUnique<OpenGLTexture2D>(base / Path("wall.jpg"));
      textures["container"] = CreateUnique<OpenGLTexture2D>(base / Path("container.jpg"));
      textures["awesomeface"] = CreateUnique<OpenGLTexture2D>(base / Path("awesomeface.png"));
      textures["container-diffuse"] = CreateUnique<OpenGLTexture2D>(base / Path("container-diffuse.png"));
      textures["container-specular"] = CreateUnique<OpenGLTexture2D>(base / Path("container-specular.png"));
      textures["container-emission"] = CreateUnique<OpenGLTexture2D>(base / Path("container-emission.png"));
    }

    // Models
    {
      using namespace IO;
      Path base = Path("data/assets/models");
      models["backpack"] = CreateUnique<OpenGLModel>(base / Path("backpack/backpack.obj"));
    }

    CreateVertexArray("object", vertices, std::size(vertices), VertexLayouts::Basic);
    CreateVertexArray("light-source", vertices, std::size(vertices), VertexLayouts::Basic);

    glEnable(GL_DEPTH_TEST);
  }

  void OpenGLContext::Render(ICamera &camera) noexcept
  {
    auto view = camera.ViewMatrix();
    auto projection = camera.ProjectionMatrix();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto &lightingShader = *shaders.at("phong-material");
    lightingShader.Bind();
    lightingShader.SetUniform("time", (float)Platform::GetTime());
    lightingShader.SetUniform("view", view);
    lightingShader.SetUniform("projection", projection);
    lightingShader.SetUniform("viewPos", camera.Position());

    // Light uniforms
    {
      Utils::SetDirectionalLightUniforms(lightingShader, directionalLight);

      for (uint i = 0; i < 4; i++)
      {
        Utils::SetPointLightUniforms(lightingShader, pointLights[i],
                                     "pointLights[" + std::to_string(i) + "]");
      }

      spotLight.Position = camera.Position();
      spotLight.Direction = camera.Forward();
      Utils::SetSpotLightUniforms(lightingShader, spotLight);
    }

    // Cubes
    {
      // Material uniforms
      {
        lightingShader.SetUniform("material.diffuse", 0);
        textures.at("container-diffuse")->Bind(0);

        lightingShader.SetUniform("material.specular", 1);
        textures.at("container-specular")->Bind(1);

        lightingShader.SetUniform("material.emission", 2);
        textures.at("container-emission")->Bind(2);

        lightingShader.SetUniform("material.shininess", 32.f);
      }

      glBindVertexArray(vaos.at("object"));
      for (uint i = 0; i < 10; i++)
      {
        Maths::Mat4 model = Maths::Identity<Maths::Mat4>();
        model = Maths::Translate(model, cubePositions[i]);
        float angle = 20.0f * i;
        model = Maths::Rotate(model, Maths::Radians(angle), {1.0f, 0.3f, 0.5f});

        lightingShader.SetUniform("model", model);
        Utils::DrawTriangles(36);
      }
    }

    // Draw lights
    {
      auto &lightSourceShader = *shaders.at("light-source");
      lightSourceShader.Bind();
      lightSourceShader.SetUniform("view", view);
      lightSourceShader.SetUniform("projection", projection);

      glBindVertexArray(vaos.at("light-source"));
      for (uint i = 0; i < 4; i++)
      {
        Maths::Mat4 model = Maths::Identity<Maths::Mat4>();
        model = Maths::Translate(model, pointLights[i].Position);
        model = Maths::Scale(model, Maths::Vec3(0.2f)); // a smaller cube

        lightSourceShader.SetUniform("lightColor", pointLights[i].Colour.Diffuse.ToVec3());
        lightSourceShader.SetUniform("model", model);
        Utils::DrawTriangles(36);
      }
    }

    {
      using namespace Maths;
      auto &backpackShader = *shaders.at("backpack");
      backpackShader.Bind();
      backpackShader.SetUniform("view", view);
      backpackShader.SetUniform("projection", projection);

      Mat4 model = Identity<Mat4>();
      model = Translate(model, {0.0f, -1.75f, 0.0f}); // translate it down so it's at the center of the scene
      model = Scale(model, {0.2f, 0.2f, 0.2f});       // it's a bit too big for our scene, so scale it down
      backpackShader.SetUniform("model", model);

      models.at("backpack")->Draw(backpackShader);
    }
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
