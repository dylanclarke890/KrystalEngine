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
#include "Krystal.Gfx/Material.hpp"
#include "Krystal.Maths/Convert.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Transform.hpp"
#include "Krystal.Maths/Vector.hpp"
#include "Krystal.Platform/Platform.hpp"

namespace
{
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

  static unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
  };

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

  using namespace Krys;
  using namespace Krys::Gfx;
  using namespace Krys::Gfx::OpenGL;

  static Map<string, Unique<OpenGLShader>> shaders;
  static Map<string, Unique<OpenGLTexture2D>> textures;
  static Map<string, Unique<OpenGLCubeMap>> cubemaps;

  static GLuint objectVAO;
  static GLuint objectVBO;
  static GLuint objectEBO;

  static GLuint skyboxVAO;
  static GLuint skyboxVBO;
  static GLuint skyboxTexture;
  static bool skyboxSet = false;

  static GLuint lightVAO;
  static GLuint lightVBO;

  static Maths::Vec3 pointLightPositions[] = {Maths::Vec3(0.7f, 0.2f, 2.0f), Maths::Vec3(2.3f, -3.3f, -4.0f),
                                              Maths::Vec3(-4.0f, 2.0f, -12.0f),
                                              Maths::Vec3(0.0f, 0.0f, -3.0f)};
  static Maths::Vec3 pointLightColors[] = {Maths::Vec3(1.0f, 0.6f, 0.0f), Maths::Vec3(1.0f, 0.0f, 0.0f),
                                           Maths::Vec3(1.0f, 1.0f, 0.0f), Maths::Vec3(0.2f, 0.2f, 1.0f)};
  static Maths::Vec3 lightDirection(-0.2f, -1.0f, -0.3f);

  static FlatColourMaterial cubeMaterial(Colour(1.0f, 0.5f, 0.31f), Colour(1.0f, 0.5f, 0.31f),
                                         Colour(0.5f, 0.5f, 0.5f), 32.0f);

  static OpenGLModel *backpack;

  static void SetFlatColourMaterial(OpenGLShader &shader, FlatColourMaterial &material)
  {
    shader.SetUniform("material.ambient", Colour::ToVec3(material.Ambient));
    shader.SetUniform("material.diffuse", Colour::ToVec3(material.Diffuse));
    shader.SetUniform("material.specular", Colour::ToVec3(material.Specular));
    shader.SetUniform("material.shininess", material.Shininess);
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

      // base = Path("data/assets/skyboxes/sky");
      // cubemaps["skybox"] =
      //   CreateUnique<OpenGLCubeMap>(base / Path("left.jpg"), base / Path("right.jpg"), base /
      //   Path("top.jpg"),
      //               base / Path("bottom.jpg"), base / Path("front.jpg"), base / Path("back.jpg"));
    }

    // Models
    {
      using namespace IO;
      Path base = Path("data/assets/models");
      backpack = new OpenGLModel(base / Path("backpack/backpack.obj"));
    }

    glCreateVertexArrays(1, &objectVAO);
    glBindVertexArray(objectVAO);

    glCreateBuffers(1, &objectVBO);
    glBindBuffer(GL_ARRAY_BUFFER, objectVBO);
    glNamedBufferData(objectVBO, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glCreateBuffers(1, &objectEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objectEBO);
    glNamedBufferData(objectEBO, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glCreateVertexArrays(1, &skyboxVAO);
    glBindVertexArray(skyboxVAO);

    glCreateBuffers(1, &skyboxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glNamedBufferData(skyboxVBO, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glCreateVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glCreateBuffers(1, &lightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glNamedBufferData(lightVBO, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);
  }

  void OpenGLContext::Render(ICamera &camera) noexcept
  {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto view = camera.ViewMatrix();
    auto projection = camera.ProjectionMatrix();

    // const float radius = 1.1f;
    // float lightX = std::sin((float)Platform::GetTime()) * radius;
    // float lightZ = std::cos((float)Platform::GetTime()) * radius;
    // lightPos = Maths::Vec3(lightX, 0.5f, lightZ);

    Maths::Vec3 diffuseColor = {0.5f, 0.5f, 0.5f};
    Maths::Vec3 ambientColor = {0.2f, 0.2f, 0.2f};
    Maths::Vec3 specularColor = {1.0f, 1.0f, 1.0f};

    // Draw cubes
    {
      auto &lightingShader = *shaders.at("phong-material");
      lightingShader.Bind();
      lightingShader.SetUniform("time", (float)Platform::GetTime());
      lightingShader.SetUniform("view", view);
      lightingShader.SetUniform("projection", projection);
      lightingShader.SetUniform("viewPos", camera.Position());

      // Lights
      {
        lightingShader.SetUniform("directionalLight.direction", lightDirection);
        lightingShader.SetUniform("directionalLight.diffuse", diffuseColor);
        lightingShader.SetUniform("directionalLight.ambient", ambientColor);
        lightingShader.SetUniform("directionalLight.specular", specularColor);

        for (unsigned int i = 0; i < 4; i++)
        {
          string index = std::to_string(i);
          lightingShader.SetUniform("pointLights[" + index + "].position", pointLightPositions[i]);
          lightingShader.SetUniform("pointLights[" + index + "].diffuse", pointLightColors[i]);
          lightingShader.SetUniform("pointLights[" + index + "].ambient", ambientColor);
          lightingShader.SetUniform("pointLights[" + index + "].specular", specularColor);
          lightingShader.SetUniform("pointLights[" + index + "].constant", 1.0f);
          lightingShader.SetUniform("pointLights[" + index + "].linear", 0.09f);
          lightingShader.SetUniform("pointLights[" + index + "].quadratic", 0.032f);
        }

        lightingShader.SetUniform("spotLight.position", camera.Position());
        lightingShader.SetUniform("spotLight.direction", camera.Forward());
        lightingShader.SetUniform("spotLight.diffuse", diffuseColor);
        lightingShader.SetUniform("spotLight.ambient", ambientColor);
        lightingShader.SetUniform("spotLight.specular", specularColor);
        lightingShader.SetUniform("spotLight.constant", 1.0f);
        lightingShader.SetUniform("spotLight.linear", 0.09f);
        lightingShader.SetUniform("spotLight.quadratic", 0.032f);
        lightingShader.SetUniform("spotLight.cutOff", std::cos(Maths::Radians(12.5f)));
        lightingShader.SetUniform("spotLight.outerCutOff", std::cos(Maths::Radians(15.0f)));
      }

      // Material
      {
        lightingShader.SetUniform("material.ambient", 0);
        lightingShader.SetUniform("material.specular", 1);
        lightingShader.SetUniform("material.emission", 2);
        lightingShader.SetUniform("material.shininess", cubeMaterial.Shininess);

        textures.at("container-diffuse")->Bind(0);
        textures.at("container-specular")->Bind(1);
        textures.at("container-emission")->Bind(2);
      }

      glBindVertexArray(objectVAO);
      Maths::Vec3 cubePositions[] = {Maths::Vec3(0.0f, 0.0f, 0.0f),    Maths::Vec3(2.0f, 5.0f, -15.0f),
                                     Maths::Vec3(-1.5f, -2.2f, -2.5f), Maths::Vec3(-3.8f, -2.0f, -12.3f),
                                     Maths::Vec3(2.4f, -0.4f, -3.5f),  Maths::Vec3(-1.7f, 3.0f, -7.5f),
                                     Maths::Vec3(1.3f, -2.0f, -2.5f),  Maths::Vec3(1.5f, 2.0f, -2.5f),
                                     Maths::Vec3(1.5f, 0.2f, -1.5f),   Maths::Vec3(-1.3f, 1.0f, -1.5f)};
      for (unsigned int i = 0; i < 10; i++)
      {
        Maths::Mat4 model = Maths::Identity<Maths::Mat4>();
        model = Maths::Translate(model, cubePositions[i]);
        float angle = 20.0f * i;
        model = Maths::Rotate(model, Maths::Radians(angle), {1.0f, 0.3f, 0.5f});

        lightingShader.SetUniform("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
      }

      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // Draw lights
    {
      auto &lightSourceShader = *shaders.at("light-source");
      lightSourceShader.Bind();
      lightSourceShader.SetUniform("view", view);
      lightSourceShader.SetUniform("projection", projection);

      glBindVertexArray(lightVAO);
      for (unsigned int i = 0; i < 4; i++)
      {
        Maths::Mat4 model = Maths::Identity<Maths::Mat4>();
        model = Maths::Translate(model, pointLightPositions[i]);
        model = Maths::Scale(model, Maths::Vec3(0.2f)); // a smaller cube

        lightSourceShader.SetUniform("lightColor", pointLightColors[i]);
        lightSourceShader.SetUniform("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
      }
    }

    {
      auto &backpackShader = *shaders.at("backpack");
      backpackShader.Bind();
      backpackShader.SetUniform("view", view);
      backpackShader.SetUniform("projection", projection);

      Maths::Mat4 model = Maths::Identity<Maths::Mat4>();
      model = Maths::Translate(
        model, Maths::Vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
      model = Maths::Scale(
        model, Maths::Vec3(0.2f, 0.2f, 0.2f)); // it's a bit too big for our scene, so scale it down

      backpackShader.SetUniform("model", model);
      backpack->Draw(backpackShader);
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
