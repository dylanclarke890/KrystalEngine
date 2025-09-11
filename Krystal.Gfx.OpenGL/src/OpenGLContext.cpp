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

  struct FramebufferData
  {
    GLuint FBO;
    GLuint Texture;
    GLuint RBO;
    uint32 Width;
    uint32 Height;
  };

  static Map<string, Unique<OpenGLShader>> shaders;
  static Map<string, Unique<OpenGLTexture2D>> textures;
  static Map<string, Unique<OpenGLCubeMap>> cubemaps;
  static Map<string, Unique<OpenGLModel>> models;
  static Map<string, GLuint> vaos;
  static Map<string, GLuint> vbos;
  static Map<string, GLuint> ubos;
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

#pragma endregion

  static float cubeVertices[] = {
    // Back face
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // Bottom-left
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  // bottom-right
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // bottom-left
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // top-left
    // Front face
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // bottom-right
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // top-right
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // top-right
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,  // top-left
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left
    // Left face
    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // top-right
    -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  // top-left
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  // bottom-right
    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // top-right
                                     // Right face
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    // top-left
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // bottom-right
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // bottom-right
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    // top-left
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f,   // bottom-left
    // Bottom face
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f,  // top-left
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,   // bottom-left
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,   // bottom-left
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  // bottom-right
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right
    // Top face
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // bottom-right
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  // top-right
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // bottom-right
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f   // bottom-left
  };

  static float planeVertices[] = {
    // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as
    // texture wrapping mode). this will cause the floor texture to repeat)
    5.0f, -0.5f, 5.0f, 2.0f, 0.0f, -5.0f, -0.5f, 5.0f,  0.0f, 0.0f, -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,

    5.0f, -0.5f, 5.0f, 2.0f, 0.0f, -5.0f, -0.5f, -5.0f, 0.0f, 2.0f, 5.0f,  -0.5f, -5.0f, 2.0f, 2.0f};

  static float transparentVertices[] = {
    // positions         // texture Coords
    0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, -0.5f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.5f,  0.0f, 1.0f, 1.0f};

  static VertexBufferLayout depthTestLayout({
    {VertexAttributeType::Float, 3}, // Position
    {VertexAttributeType::Float, 2}  // Texture Coordinates
  });

  static VertexBufferLayout reflectiveCubeLayout({
    {VertexAttributeType::Float, 3}, // Position
    {VertexAttributeType::Float, 3}  // Normal
  });

  static VertexBufferLayout positionOnlyLayout({
    {VertexAttributeType::Float, 3} // Position
  });

  static float vertices[] = {
    -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,
    0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
    -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,

    -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,

    -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, -1.0f, 0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
    -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,

    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 1.0f,  0.0f,  0.0f,
    0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
    0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,
    0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,

    -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f};

  static float quadVertices[] = {
    // positions   // texCoords
    -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

    -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};

  static float skyboxVertices[] = {
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

  static void CreateUniformBuffer(const string &name, size_t size, uint binding) noexcept
  {
    GLuint ubo;
    glCreateBuffers(1, &ubo);
    ubos[name] = ubo;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glNamedBufferData(ubo, size, NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
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
      : _windowHandle(windowHandle), _platformImpl(CreateUnique<GLContextPlatformImpl>(windowHandle)),
        _width(width), _height(height)
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
      shaders["framebuffer"] =
        CreateUnique<OpenGLShader>(base / Path("framebuffer.vert"), base / Path("framebuffer.frag"));
      shaders["reflection"] =
        CreateUnique<OpenGLShader>(base / Path("reflection.vert"), base / Path("reflection.frag"));
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
      textures["grass"] = CreateUnique<OpenGLTexture2D>(base / Path("grass.png"));
      textures["window"] = CreateUnique<OpenGLTexture2D>(base / Path("blending_transparent_window.png"));

      textures["marble"] = CreateUnique<OpenGLTexture2D>(base / Path("marble.jpg"));
      textures["marble"]->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
      textures["marble"]->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    {
      using namespace IO;

      Path base = Path("data/assets/skyboxes/sky");
      cubemaps["sky"] = CreateUnique<OpenGLCubeMap>(base / Path("left.jpg"), base / Path("right.jpg"),
                                                    base / Path("top.jpg"), base / Path("bottom.jpg"),
                                                    base / Path("front.jpg"), base / Path("back.jpg"));
    }

    CreateVertexArray("cube", vertices, std::size(vertices), reflectiveCubeLayout);
    CreateVertexArray("skybox", skyboxVertices, std::size(skyboxVertices), positionOnlyLayout);

    CreateUniformBuffer("Matrices", 2 * sizeof(Maths::Mat4), 0);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  }

  void OpenGLContext::Render(ICamera &camera) noexcept
  {
    auto view = camera.ViewMatrix();
    auto projection = camera.ProjectionMatrix();
    Maths::Mat4 model;

    glBindBuffer(GL_UNIFORM_BUFFER, ubos.at("Matrices"));
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Maths::Mat4), &view[0][0]);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Maths::Mat4), sizeof(Maths::Mat4), &projection[0][0]);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto &shader = *shaders.at("reflection");
    shader.Bind();
    shader.SetUniform("skybox", 0);
    shader.SetUniform("cameraPos", camera.Position());

    glBindVertexArray(vaos.at("cube"));
    cubemaps.at("sky")->Bind(0);
    {
      model = Maths::Identity<Maths::Mat4>();
      model = Maths::Translate(model, {-1.0f, 0.0f, -1.0f});
      shader.SetUniform("model", model);
      Utils::DrawTriangles(36);
    }

    // draw skybox last
    glBindVertexArray(vaos.at("skybox"));
    {
      glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when values are equal to depth
                              // buffer's content
      auto &skyboxShader = *shaders.at("skybox");
      skyboxShader.Bind();
      skyboxShader.SetUniform("skybox", 0);

      // skybox cube
      Utils::DrawTriangles(36);
      glBindVertexArray(0);
      glDepthFunc(GL_LESS); // set depth function back to default
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
