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

  static float cubeVertices[] = {
    // positions          // texture Coords
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
    0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

    -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f, -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,

    -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,

    0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
    0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

    -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f};

  static float planeVertices[] = {
    // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as
    // texture wrapping mode). this will cause the floor texture to repeat)
    5.0f, -0.5f, 5.0f, 2.0f, 0.0f, -5.0f, -0.5f, 5.0f,  0.0f, 0.0f, -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,

    5.0f, -0.5f, 5.0f, 2.0f, 0.0f, -5.0f, -0.5f, -5.0f, 0.0f, 2.0f, 5.0f,  -0.5f, -5.0f, 2.0f, 2.0f};

  static VertexBufferLayout depthTestLayout({
    {VertexAttributeType::Float, 3}, // Position
    {VertexAttributeType::Float, 2}  // Texture Coordinates
  });

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

  static void SetLightUniforms(OpenGLShader &shader, ICamera &camera) noexcept
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
      shaders["depth-testing"] =
        CreateUnique<OpenGLShader>(base / Path("depth-testing.vert"), base / Path("depth-testing.frag"));
      shaders["single-colour"] =
        CreateUnique<OpenGLShader>(base / Path("depth-testing.vert"), base / Path("single-colour.frag"));
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
      textures["metal"] = CreateUnique<OpenGLTexture2D>(base / Path("metal.png"));

      textures["marble"] = CreateUnique<OpenGLTexture2D>(base / Path("marble.jpg"));
      textures["marble"]->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
      textures["marble"]->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    CreateVertexArray("object", vertices, std::size(vertices), VertexLayouts::Basic);
    CreateVertexArray("light-source", vertices, std::size(vertices), VertexLayouts::Basic);
    CreateVertexArray("plane", planeVertices, std::size(planeVertices), depthTestLayout);
    CreateVertexArray("cube", cubeVertices, std::size(cubeVertices), depthTestLayout);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  }
  
  void OpenGLContext::Render(ICamera &camera) noexcept
  {
    auto view = camera.ViewMatrix();
    auto projection = camera.ProjectionMatrix();
    Maths::Mat4 model;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    auto &shader = *shaders.at("depth-testing");
    shader.Bind();

    shader.SetUniform("view", view);
    shader.SetUniform("projection", projection);
    shader.SetUniform("texture1", 0);

    glStencilMask(0x00);

    glBindVertexArray(vaos.at("plane"));
    textures.at("marble")->Bind(0);
    {
      model = Maths::Identity<Maths::Mat4>();
      shader.SetUniform("model", model);
      Utils::DrawTriangles(6);
    }

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    glBindVertexArray(vaos.at("cube"));
    textures.at("metal")->Bind(0);
    {
      model = Maths::Identity<Maths::Mat4>();
      model = Maths::Translate(model, {-1.0f, 0.0f, -1.0f});
      shader.SetUniform("model", model);
      Utils::DrawTriangles(36);

      model = Maths::Identity<Maths::Mat4>();
      model = Maths::Translate(model, {2.0f, 0.0f, 0.0f});
      shader.SetUniform("model", model);
      Utils::DrawTriangles(36);
    }

    auto &stencilShader = *shaders.at("single-colour");
    stencilShader.Bind();

    stencilShader.SetUniform("view", view);
    stencilShader.SetUniform("projection", projection);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);

    {
      model = Maths::Identity<Maths::Mat4>();
      model = Maths::Translate(model, {-1.0f, 0.0f, -1.0f});
      model = Maths::Scale(model, {1.1f, 1.1f, 1.1f});
      stencilShader.SetUniform("model", model);
      Utils::DrawTriangles(36);

      model = Maths::Identity<Maths::Mat4>();
      model = Maths::Translate(model, {2.0f, 0.0f, 0.0f});
      model = Maths::Scale(model, {1.1f, 1.1f, 1.1f});
      stencilShader.SetUniform("model", model);
      Utils::DrawTriangles(36);
    }

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glEnable(GL_DEPTH_TEST);
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
