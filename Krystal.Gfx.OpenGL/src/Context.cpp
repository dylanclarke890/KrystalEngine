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

  struct ShadowMapData
  {
    GLuint FBO {};
    GLuint Texture {};
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

  static Vec3 lightPos(0.5f, 1.0f, 0.3f);

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

    shadowMaps[name] = {depthMapFBO, depthMap, width, height};

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
    shadowMaps[name] = {depthMapFBO, depthCubemap, width, height};
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
    }

    // Textures
    {
      using namespace IO;
      Path base = Path("data/assets");

      textures["wood"] = CreateUnique<Texture2D>(base / Path("wood.png"));
      textures.at("wood")->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
      textures.at("wood")->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

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

    // Uniform buffers
    {
      ubos["matrices"] = CreateUnique<UniformBuffer>(2 * sizeof(Mat4));
      ubos.at("matrices")->Bind(0);
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    shaders.at("parallax-mapping")->SetUniform("diffuseTexture", 0);
    shaders.at("parallax-mapping")->SetUniform("normalMap", 1);
    shaders.at("parallax-mapping")->SetUniform("depthMap", 2);
  }

  void Context::Render(ICamera &camera) noexcept
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto view = camera.ViewMatrix();
    auto projection = camera.ProjectionMatrix();

    ubos.at("matrices")->Update(view);
    ubos.at("matrices")->Update(projection, sizeof(Mat4));

    {
      auto &shader = shaders.at("parallax-mapping");
      shader->Bind();

      // render normal-mapped quad
      Mat4 model = Identity<Mat4>();
      //model = Rotate(model, Radians((float)Platform::GetTime() * -10.0f), Normalize(Vec3(1.0, 0.0, 1.0)));
      shader->SetUniform("model", model);
      shader->SetUniform("viewPos", camera.Position());
      shader->SetUniform("lightPos", lightPos);
      shader->SetUniform("heightScale", 0.1f);
      textures.at("brickwall-diffuse")->Bind(0);
      textures.at("brickwall-normal")->Bind(1);
      textures.at("brickwall-displacement")->Bind(2);
      vaos.at("quad")->Bind();
      Utils::Draw(GL_TRIANGLES, 6);
    }

    // Light source
    {
      auto &shader = shaders.at("light-source");
      shader->Bind();
      Mat4 model = Identity<Mat4>();
      model = Translate(model, lightPos);
      model = Scale(model, Vec3(0.2f));
      shader->SetUniform("model", model);
      shader->SetUniform("lightColor", (Colour {1.0f, 1.0f, 1.0f}).ToVec3());
      vaos.at("cube")->Bind();
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
