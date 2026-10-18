#include "Krystal.Gfx/OpenGL/GLDevice.hpp"
#include "Krystal.Core/Commands/CommandListReader.hpp"
#include "Krystal.Core/Log/ILogger.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Gfx/OpenGL/GLMappings.hpp"
#include <cstring>
#include <format>

namespace krys::gfx::opengl
{
  namespace
  {
    void APIENTRY DebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei,
                                       const GLchar *message, const void *) noexcept
    {
      auto *logger = log::GetGlobalLogger();
      if (logger == nullptr)
      {
        return;
      }

      auto sourceName = [&]
      {
        switch (source)
        {
          case GL_DEBUG_SOURCE_API:             return "API";
          case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "window system";
          case GL_DEBUG_SOURCE_SHADER_COMPILER: return "shader compiler";
          case GL_DEBUG_SOURCE_THIRD_PARTY:     return "third party";
          case GL_DEBUG_SOURCE_APPLICATION:     return "application";
          default:                              return "other";
        }
      }();

      auto typeName = [&]
      {
        switch (type)
        {
          case GL_DEBUG_TYPE_ERROR:               return "error";
          case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "deprecated behaviour";
          case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "undefined behaviour";
          case GL_DEBUG_TYPE_PORTABILITY:         return "portability";
          case GL_DEBUG_TYPE_PERFORMANCE:         return "performance";
          default:                                return "other";
        }
      }();

      auto text = std::format("OpenGL {} ({}, {}): {}", id, sourceName, typeName, message);
      switch (severity)
      {
        case GL_DEBUG_SEVERITY_HIGH:   logger->Error(text); break;
        case GL_DEBUG_SEVERITY_MEDIUM: logger->Warn(text); break;
        case GL_DEBUG_SEVERITY_LOW:    logger->Info(text); break;
        default:                       break;
      }
    }

    void Label(GLenum identifier, GLuint id, utf8_stringview name) noexcept
    {
      if (!name.empty())
      {
        glObjectLabel(identifier, id, static_cast<GLsizei>(name.size()),
                      reinterpret_cast<const GLchar *>(name.data()));
      }
    }

    KRYS_NODISCARD string ToString(utf8_stringview text) noexcept
    {
      return string(reinterpret_cast<const char *>(text.data()), text.size());
    }

    KRYS_NODISCARD constexpr uint64 AlignUp(uint64 value, uint64 alignment) noexcept
    {
      return (value + alignment - 1u) & ~(alignment - 1u);
    }

    KRYS_NODISCARD constexpr uint32 MipSize(uint32 size, uint32 mipLevel) noexcept
    {
      uint32 result = size >> mipLevel;
      return result == 0u ? 1u : result;
    }

    KRYS_NODISCARD constexpr bool IsLayered(TextureDimension dimension) noexcept
    {
      return dimension == TextureDimension::Texture2DArray || dimension == TextureDimension::Texture3D
             || dimension == TextureDimension::Cube;
    }

    KRYS_NODISCARD GLenum DepthStencilAttachmentPoint(TextureFormat format) noexcept
    {
      return HasStencil(format) ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
    }

    /// @brief Copies the source into a null-terminated string, checking it starts with a `#version` line.
    KRYS_NODISCARD Expected<string> PrepareSource(utf8_stringview source) noexcept
    {
      string text = ToString(source);
      if (!text.starts_with("#version"))
      {
        return Unexpected<string>("The shader source must start with a #version line.");
      }
      return text;
    }

    KRYS_NODISCARD string ShaderLog(GLuint shader) noexcept
    {
      GLint length = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
      string log(static_cast<size_t>(length > 0 ? length : 0), '\0');
      if (length > 0)
      {
        glGetShaderInfoLog(shader, length, nullptr, log.data());
        log.resize(std::strlen(log.c_str()));
      }
      return log;
    }

    KRYS_NODISCARD string ProgramLog(GLuint program) noexcept
    {
      GLint length = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
      string log(static_cast<size_t>(length > 0 ? length : 0), '\0');
      if (length > 0)
      {
        glGetProgramInfoLog(program, length, nullptr, log.data());
        log.resize(std::strlen(log.c_str()));
      }
      return log;
    }

    void SetEnabled(GLenum capability, bool enabled) noexcept
    {
      if (enabled)
      {
        glEnable(capability);
      }
      else
      {
        glDisable(capability);
      }
    }
  }

  // MARK: - Lifetime

  GLDevice::GLDevice(const DeviceDesc &desc) noexcept : _desc(desc)
  {
  }

  GLDevice::~GLDevice() noexcept
  {
    if (_context == nullptr)
    {
      return;
    }

    _context->MakeCurrent({});
    WaitIdle();
    ReportLeaks();
    DestroyAll();
  }

  Expected<void> GLDevice::Initialise() noexcept
  {
    auto context = GLContext::Create(_desc.EnableValidation);
    if (!context)
    {
      return Unexpected<string>(context.error());
    }
    _context = krys::move(*context);

    if (_desc.EnableValidation)
    {
      glEnable(GL_DEBUG_OUTPUT);
      glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
      glDebugMessageCallback(DebugMessageCallback, nullptr);
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    // Uploads and readbacks are tightly packed.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    QueryLimits();
    glCreateFramebuffers(2, _blitFramebuffers.data());
    return CreateTransientRing();
  }

  void GLDevice::QueryLimits() noexcept
  {
    GLint value = 0;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &value);
    _limits.UniformBufferOffsetAlignment = static_cast<uint32>(value);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
    _limits.MaxTextureSize = static_cast<uint32>(value);
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &value);
    _limits.MaxColourAttachments =
      static_cast<uint32>(value) < MaxColourAttachments ? static_cast<uint32>(value) : MaxColourAttachments;
    if (IsGLExtensionSupported("GL_ARB_texture_filter_anisotropic")
        || IsGLExtensionSupported("GL_EXT_texture_filter_anisotropic"))
    {
      GLfloat anisotropy = 1.f;
      glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &anisotropy);
      _limits.MaxSamplerAnisotropy = anisotropy;
    }
  }

  Expected<void> GLDevice::CreateTransientRing() noexcept
  {
    const uint64 size = _desc.TransientMemoryPerFrame * FramesInFlight;
    GLuint id = 0u;
    glCreateBuffers(1, &id);
    constexpr GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    glNamedBufferStorage(id, static_cast<GLsizeiptr>(size), nullptr, flags);
    _transientMemory =
      static_cast<byte *>(glMapNamedBufferRange(id, 0, static_cast<GLsizeiptr>(size), flags));
    if (_transientMemory == nullptr)
    {
      glDeleteBuffers(1, &id);
      return Unexpected<string>("Failed to map the transient memory ring.");
    }

    Label(GL_BUFFER, id, u8"TransientRing");
    _transientBuffer = _buffers.Add({.Id = id, .Size = size, .Internal = true, .DebugName = "TransientRing"});
    return {};
  }

  void GLDevice::ReportLeaks() noexcept
  {
    auto report = [](const char *kind, const string &name)
    {
      KRYS_LOG_WARN("Graphics device destroyed with a live {}: '{}'", kind, name);
    };

    _buffers.ForEach(
      [&](BufferHandle, const GLBuffer &buffer)
      {
        if (!buffer.Internal)
        {
          report("buffer", buffer.DebugName);
        }
      });
    _textures.ForEach([&](TextureHandle, const GLTexture &texture) { report("texture", texture.DebugName); });
    _shaders.ForEach([&](ShaderHandle, const GLShader &shader) { report("shader", shader.DebugName); });
    _pipelines.ForEach([&](PipelineHandle, const GLPipeline &pipeline)
                       { report("pipeline", pipeline.DebugName); });
    _swapchains.ForEach([&](SwapchainHandle, const GLSwapchain &) { report("swapchain", "window"); });
  }

  void GLDevice::DestroyAll() noexcept
  {
    for (auto &[key, framebuffer] : _framebuffers)
    {
      glDeleteFramebuffers(1, &framebuffer);
    }
    _framebuffers.clear();
    glDeleteFramebuffers(2, _blitFramebuffers.data());

    for (auto &fence : _frameFences)
    {
      if (fence != nullptr)
      {
        glDeleteSync(fence);
        fence = nullptr;
      }
    }

    if (auto *ring = _buffers.TryGet(_transientBuffer))
    {
      glUnmapNamedBuffer(ring->Id);
    }

    _pipelines.ForEach(
      [](PipelineHandle, GLPipeline &pipeline)
      {
        glDeleteProgram(pipeline.Program);
        glDeleteVertexArrays(1, &pipeline.VertexArray);
      });
    _shaders.ForEach([](ShaderHandle, GLShader &shader) { glDeleteShader(shader.Id); });
    _samplers.ForEach([](SamplerHandle, GLSampler &sampler) { glDeleteSamplers(1, &sampler.Id); });
    _textures.ForEach([](TextureHandle, GLTexture &texture) { glDeleteTextures(1, &texture.Id); });
    _buffers.ForEach([](BufferHandle, GLBuffer &buffer) { glDeleteBuffers(1, &buffer.Id); });
    _swapchains.ForEach([&](SwapchainHandle, GLSwapchain &swapchain)
                        { _context->DetachWindow(swapchain.Window, swapchain.Surface); });

    _pipelines.Clear();
    _shaders.Clear();
    _samplers.Clear();
    _samplerCache.clear();
    _textures.Clear();
    _buffers.Clear();
    _swapchains.Clear();
  }

  // MARK: - Creation

  Expected<BufferHandle> GLDevice::CreateBuffer(const BufferDesc &desc) noexcept
  {
    if (desc.Size == 0u)
    {
      return Unexpected<string>("A buffer can't be empty.");
    }
    if (desc.InitialData.size() > desc.Size)
    {
      return Unexpected<string>("The initial data is larger than the buffer.");
    }

    GLuint id = 0u;
    glCreateBuffers(1, &id);
    const bool fullData = desc.InitialData.size() == desc.Size;
    glNamedBufferStorage(id, static_cast<GLsizeiptr>(desc.Size), fullData ? desc.InitialData.data() : nullptr,
                         GL_DYNAMIC_STORAGE_BIT);
    if (!fullData && !desc.InitialData.empty())
    {
      glNamedBufferSubData(id, 0, static_cast<GLsizeiptr>(desc.InitialData.size()), desc.InitialData.data());
    }
    Label(GL_BUFFER, id, desc.DebugName);
    return _buffers.Add(
      {.Id = id, .Size = desc.Size, .Internal = false, .DebugName = ToString(desc.DebugName)});
  }

  Expected<TextureHandle> GLDevice::CreateTexture(const TextureDesc &desc) noexcept
  {
    if (desc.Format == TextureFormat::Undefined)
    {
      return Unexpected<string>("A texture needs a format.");
    }
    if (desc.Width == 0u || desc.Height == 0u || desc.Depth == 0u || desc.Layers == 0u)
    {
      return Unexpected<string>("A texture can't be empty.");
    }
    if (desc.Width > _limits.MaxTextureSize || desc.Height > _limits.MaxTextureSize)
    {
      return Unexpected<string>("The texture is larger than the device supports.");
    }
    if (desc.MipLevels == 0u || desc.MipLevels > FullMipChainLength(desc.Width, desc.Height))
    {
      return Unexpected<string>("The texture has more mip levels than its size allows.");
    }
    if (desc.Dimension == TextureDimension::Cube && (desc.Width != desc.Height || desc.Layers != 6u))
    {
      return Unexpected<string>("A cube texture must be square with 6 layers.");
    }
    if (desc.Samples > 1u && (desc.Dimension != TextureDimension::Texture2D || desc.MipLevels != 1u))
    {
      return Unexpected<string>("Only 2D textures without mip levels can be multisampled.");
    }

    const GLenum target = MapTextureTarget(desc.Dimension, desc.Samples);
    const GLenum internalFormat = MapInternalFormat(desc.Format);
    const auto width = static_cast<GLsizei>(desc.Width);
    const auto height = static_cast<GLsizei>(desc.Height);
    const auto mips = static_cast<GLsizei>(desc.MipLevels);

    GLuint id = 0u;
    glCreateTextures(target, 1, &id);
    switch (desc.Dimension)
    {
      case TextureDimension::Texture1D: glTextureStorage1D(id, mips, internalFormat, width); break;
      case TextureDimension::Texture2D:
        if (desc.Samples > 1u)
        {
          glTextureStorage2DMultisample(id, static_cast<GLsizei>(desc.Samples), internalFormat, width, height,
                                        GL_TRUE);
        }
        else
        {
          glTextureStorage2D(id, mips, internalFormat, width, height);
        }
        break;
      case TextureDimension::Texture2DArray:
        glTextureStorage3D(id, mips, internalFormat, width, height, static_cast<GLsizei>(desc.Layers));
        break;
      case TextureDimension::Texture3D:
        glTextureStorage3D(id, mips, internalFormat, width, height, static_cast<GLsizei>(desc.Depth));
        break;
      case TextureDimension::Cube: glTextureStorage2D(id, mips, internalFormat, width, height); break;
    }
    Label(GL_TEXTURE, id, desc.DebugName);

    GLTexture texture {.Id = id, .Target = target, .Desc = desc, .DebugName = ToString(desc.DebugName)};
    texture.Desc.DebugName = {};
    return _textures.Add(krys::move(texture));
  }

  Expected<SamplerHandle> GLDevice::CreateSampler(const SamplerDesc &desc) noexcept
  {
    if (auto it = _samplerCache.find(desc); it != _samplerCache.end())
    {
      ++_samplers.Get(it->second).References;
      return it->second;
    }

    GLuint id = 0u;
    glCreateSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER,
                        static_cast<GLint>(MapMinFilter(desc.MinFilter, desc.MipmapMode)));
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(MapMagFilter(desc.MagFilter)));
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, static_cast<GLint>(MapAddressMode(desc.AddressU)));
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, static_cast<GLint>(MapAddressMode(desc.AddressV)));
    glSamplerParameteri(id, GL_TEXTURE_WRAP_R, static_cast<GLint>(MapAddressMode(desc.AddressW)));
    glSamplerParameterf(id, GL_TEXTURE_MIN_LOD, desc.MinLod);
    glSamplerParameterf(id, GL_TEXTURE_MAX_LOD,
                        desc.MipmapMode == SamplerMipmapMode::None ? 0.25f : desc.MaxLod);
    glSamplerParameterf(id, GL_TEXTURE_LOD_BIAS, desc.LodBias);

    const GLfloat borders[3][4] = {{0.f, 0.f, 0.f, 0.f}, {0.f, 0.f, 0.f, 1.f}, {1.f, 1.f, 1.f, 1.f}};
    glSamplerParameterfv(id, GL_TEXTURE_BORDER_COLOR, borders[static_cast<size_t>(desc.BorderColour)]);

    if (desc.MaxAnisotropy > 1.f && _limits.MaxSamplerAnisotropy > 1.f)
    {
      glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY,
                          desc.MaxAnisotropy < _limits.MaxSamplerAnisotropy ? desc.MaxAnisotropy
                                                                            : _limits.MaxSamplerAnisotropy);
    }

    if (desc.CompareEnabled)
    {
      glSamplerParameteri(id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
      glSamplerParameteri(id, GL_TEXTURE_COMPARE_FUNC, static_cast<GLint>(MapCompareOp(desc.Compare)));
    }

    auto handle = _samplers.Add({.Id = id, .Desc = desc, .References = 1u});
    _samplerCache.emplace(desc, handle);
    return handle;
  }

  Expected<ShaderHandle> GLDevice::CreateShader(const ShaderDesc &desc) noexcept
  {
    auto source = PrepareSource(desc.Source);
    if (!source)
    {
      return Unexpected<string>(std::format("{}: {}", ToString(desc.DebugName), source.error()));
    }

    GLuint id = glCreateShader(desc.Stage == ShaderStage::Vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
    const GLchar *text = source->c_str();
    glShaderSource(id, 1, &text, nullptr);
    glCompileShader(id);

    GLint compiled = GL_FALSE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE)
    {
      auto log = ShaderLog(id);
      glDeleteShader(id);
      return Unexpected<string>(std::format("{}: {}", ToString(desc.DebugName), log));
    }

    Label(GL_SHADER, id, desc.DebugName);
    return _shaders.Add({.Id = id, .Stage = desc.Stage, .DebugName = ToString(desc.DebugName)});
  }

  Expected<PipelineHandle> GLDevice::CreatePipeline(const PipelineDesc &desc) noexcept
  {
    const auto *vertexShader = _shaders.TryGet(desc.VertexShader);
    const auto *fragmentShader = _shaders.TryGet(desc.FragmentShader);
    if (vertexShader == nullptr || vertexShader->Stage != ShaderStage::Vertex)
    {
      return Unexpected<string>("The pipeline needs a vertex shader.");
    }
    if (fragmentShader == nullptr || fragmentShader->Stage != ShaderStage::Fragment)
    {
      return Unexpected<string>("The pipeline needs a fragment shader.");
    }
    if (desc.ColourFormats.size() > _limits.MaxColourAttachments)
    {
      return Unexpected<string>("The pipeline has more colour targets than the device supports.");
    }
    if (desc.VertexBuffers.size() > 8u)
    {
      return Unexpected<string>("The pipeline has more than 8 vertex buffers.");
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader->Id);
    glAttachShader(program, fragmentShader->Id);
    glLinkProgram(program);
    glDetachShader(program, vertexShader->Id);
    glDetachShader(program, fragmentShader->Id);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE)
    {
      auto log = ProgramLog(program);
      glDeleteProgram(program);
      return Unexpected<string>(std::format("{}: {}", ToString(desc.DebugName), log));
    }

    GLuint vertexArray = 0u;
    glCreateVertexArrays(1, &vertexArray);
    GLPipeline pipeline {
      .Program = program, .VertexArray = vertexArray, .Topology = MapTopology(desc.Topology)};
    for (uint32 slot = 0u; slot < desc.VertexBuffers.size(); ++slot)
    {
      const auto &layout = desc.VertexBuffers[slot];
      for (const auto &attribute : layout.Attributes)
      {
        const auto format = MapVertexFormat(attribute.Format);
        glEnableVertexArrayAttrib(vertexArray, attribute.Location);
        if (format.Integer)
        {
          glVertexArrayAttribIFormat(vertexArray, attribute.Location, format.Components, format.Type,
                                     attribute.Offset);
        }
        else
        {
          glVertexArrayAttribFormat(vertexArray, attribute.Location, format.Components, format.Type,
                                    format.Normalised, attribute.Offset);
        }
        glVertexArrayAttribBinding(vertexArray, attribute.Location, slot);
      }
      glVertexArrayBindingDivisor(vertexArray, slot,
                                  layout.InputRate == VertexInputRate::PerInstance ? 1u : 0u);
      pipeline.Strides.push_back(layout.Stride);
      pipeline.AppliedVertexBuffers.push_back({});
    }

    pipeline.Blend = desc.Blend;
    pipeline.DepthStencil = desc.DepthStencil;
    pipeline.Raster = desc.Raster;
    pipeline.DebugName = ToString(desc.DebugName);
    Label(GL_PROGRAM, program, desc.DebugName);
    Label(GL_VERTEX_ARRAY, vertexArray, desc.DebugName);
    return _pipelines.Add(krys::move(pipeline));
  }

  Expected<SwapchainHandle> GLDevice::CreateSwapchain(const SwapchainDesc &desc) noexcept
  {
    if (desc.Format != TextureFormat::RGBA8Unorm && desc.Format != TextureFormat::RGBA8Srgb)
    {
      return Unexpected<string>("A swapchain's format is RGBA8Unorm or RGBA8Srgb.");
    }
    if (desc.Format == TextureFormat::RGBA8Srgb && !_context->SupportsSrgb())
    {
      return Unexpected<string>("The system can't present sRGB swapchains.");
    }

    auto surface = _context->AttachWindow(desc.Window);
    if (!surface)
    {
      return Unexpected<string>(surface.error());
    }
    return _swapchains.Add({.Window = desc.Window,
                            .Surface = *surface,
                            .Width = desc.Width,
                            .Height = desc.Height,
                            .Format = desc.Format,
                            .VSync = desc.VSync});
  }

  // MARK: - Destruction

  void GLDevice::Destroy(BufferHandle buffer) noexcept
  {
    const auto *existing = _buffers.TryGet(buffer);
    krys_debug_assert_msg(existing != nullptr, "Destroying a stale or invalid buffer.");
    if (existing == nullptr || existing->Internal)
    {
      return;
    }

    // A new buffer can reuse the name; forget the old one in the pipelines' vertex arrays.
    const GLuint id = existing->Id;
    _pipelines.ForEach(
      [id](PipelineHandle, GLPipeline &pipeline)
      {
        for (auto &binding : pipeline.AppliedVertexBuffers)
        {
          if (binding.Buffer == id)
          {
            binding = {};
          }
        }
        if (pipeline.AppliedIndexBuffer == id)
        {
          pipeline.AppliedIndexBuffer = 0u;
        }
      });

    auto removed = _buffers.Remove(buffer);
    glDeleteBuffers(1, &removed->Id);
  }

  void GLDevice::Destroy(TextureHandle texture) noexcept
  {
    auto removed = _textures.Remove(texture);
    krys_debug_assert_msg(removed.has_value(), "Destroying a stale or invalid texture.");
    if (removed)
    {
      ForgetFramebuffersUsing(removed->Id);
      glDeleteTextures(1, &removed->Id);
    }
  }

  void GLDevice::Destroy(SamplerHandle sampler) noexcept
  {
    auto *existing = _samplers.TryGet(sampler);
    krys_debug_assert_msg(existing != nullptr, "Destroying a stale or invalid sampler.");
    if (existing == nullptr || --existing->References > 0u)
    {
      return;
    }

    _samplerCache.erase(existing->Desc);
    auto removed = _samplers.Remove(sampler);
    glDeleteSamplers(1, &removed->Id);
  }

  void GLDevice::Destroy(ShaderHandle shader) noexcept
  {
    auto removed = _shaders.Remove(shader);
    krys_debug_assert_msg(removed.has_value(), "Destroying a stale or invalid shader.");
    if (removed)
    {
      glDeleteShader(removed->Id);
    }
  }

  void GLDevice::Destroy(PipelineHandle pipeline) noexcept
  {
    auto removed = _pipelines.Remove(pipeline);
    krys_debug_assert_msg(removed.has_value(), "Destroying a stale or invalid pipeline.");
    if (removed)
    {
      glDeleteProgram(removed->Program);
      glDeleteVertexArrays(1, &removed->VertexArray);
    }
  }

  void GLDevice::Destroy(SwapchainHandle swapchain) noexcept
  {
    auto removed = _swapchains.Remove(swapchain);
    krys_debug_assert_msg(removed.has_value(), "Destroying a stale or invalid swapchain.");
    if (removed)
    {
      _context->DetachWindow(removed->Window, removed->Surface);
    }
  }

  void GLDevice::ForgetFramebuffersUsing(GLuint texture) noexcept
  {
    for (auto it = _framebuffers.begin(); it != _framebuffers.end();)
    {
      const auto &key = it->first;
      bool uses = false;
      for (uint32 i = 0u; i < key.Textures.size(); ++i)
      {
        uses = uses || key.Textures[i] == texture;
      }

      if (uses)
      {
        glDeleteFramebuffers(1, &it->second);
        it = _framebuffers.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

  // MARK: - Swapchains

  void GLDevice::ResizeSwapchain(SwapchainHandle swapchain, uint32 width, uint32 height) noexcept
  {
    if (auto *existing = _swapchains.TryGet(swapchain))
    {
      existing->Width = width;
      existing->Height = height;
    }
  }

  void GLDevice::SetVSync(SwapchainHandle swapchain, bool enabled) noexcept
  {
    if (auto *existing = _swapchains.TryGet(swapchain))
    {
      existing->VSync = enabled;
    }
  }

  // MARK: - Data

  void GLDevice::WriteBuffer(BufferHandle buffer, uint64 offset, Span<const byte> data) noexcept
  {
    const auto *existing = _buffers.TryGet(buffer);
    krys_debug_assert_msg(existing != nullptr, "Writing a stale or invalid buffer.");
    if (existing == nullptr || data.empty())
    {
      return;
    }
    krys_assert_msg(offset + data.size() <= existing->Size, "Write past the end of the buffer.");
    glNamedBufferSubData(existing->Id, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(data.size()),
                         data.data());
  }

  void GLDevice::WriteTexture(TextureHandle texture, const TextureRegion &region,
                              Span<const byte> data) noexcept
  {
    const auto *existing = _textures.TryGet(texture);
    krys_debug_assert_msg(existing != nullptr, "Writing a stale or invalid texture.");
    if (existing == nullptr)
    {
      return;
    }
    krys_assert_msg(existing->Desc.Samples == 1u, "Multisampled textures can't be written.");
    krys_assert_msg(data.size() >= uint64(region.Width) * region.Height * region.Depth
                                     * BytesPerTexel(existing->Desc.Format),
                    "Not enough data for the texture region.");

    const auto transfer = MapTransferFormat(existing->Desc.Format);
    const auto mip = static_cast<GLint>(region.MipLevel);
    switch (existing->Desc.Dimension)
    {
      case TextureDimension::Texture1D:
        glTextureSubImage1D(existing->Id, mip, static_cast<GLint>(region.X),
                            static_cast<GLsizei>(region.Width), transfer.Format, transfer.Type, data.data());
        break;
      case TextureDimension::Texture2D:
        glTextureSubImage2D(existing->Id, mip, static_cast<GLint>(region.X), static_cast<GLint>(region.Y),
                            static_cast<GLsizei>(region.Width), static_cast<GLsizei>(region.Height),
                            transfer.Format, transfer.Type, data.data());
        break;
      case TextureDimension::Texture2DArray:
      case TextureDimension::Texture3D:
      case TextureDimension::Cube:
        glTextureSubImage3D(existing->Id, mip, static_cast<GLint>(region.X), static_cast<GLint>(region.Y),
                            static_cast<GLint>(region.Z), static_cast<GLsizei>(region.Width),
                            static_cast<GLsizei>(region.Height), static_cast<GLsizei>(region.Depth),
                            transfer.Format, transfer.Type, data.data());
        break;
    }
  }

  void GLDevice::ReadTexture(TextureHandle texture, const TextureRegion &region, Span<byte> out) noexcept
  {
    const auto *existing = _textures.TryGet(texture);
    krys_debug_assert_msg(existing != nullptr, "Reading a stale or invalid texture.");
    if (existing == nullptr)
    {
      return;
    }
    krys_assert_msg(existing->Desc.Samples == 1u, "Multisampled textures can't be read.");

    const auto transfer = MapTransferFormat(existing->Desc.Format);
    const bool is1D = existing->Desc.Dimension == TextureDimension::Texture1D;
    glGetTextureSubImage(existing->Id, static_cast<GLint>(region.MipLevel), static_cast<GLint>(region.X),
                         is1D ? 0 : static_cast<GLint>(region.Y), static_cast<GLint>(region.Z),
                         static_cast<GLsizei>(region.Width), is1D ? 1 : static_cast<GLsizei>(region.Height),
                         static_cast<GLsizei>(region.Depth), transfer.Format, transfer.Type,
                         static_cast<GLsizei>(out.size()), out.data());
  }

  void GLDevice::GenerateMipmaps(TextureHandle texture) noexcept
  {
    if (const auto *existing = _textures.TryGet(texture))
    {
      glGenerateTextureMipmap(existing->Id);
    }
  }

  TransientAllocation GLDevice::AllocateTransient(uint64 size, uint32 alignment) noexcept
  {
    krys_debug_assert_msg(alignment > 0u && (alignment & (alignment - 1u)) == 0u,
                          "The alignment must be a power of two.");
    const uint64 offset = AlignUp(_transientOffset, alignment);
    if (offset + size > _desc.TransientMemoryPerFrame)
    {
      krys_debug_assert_msg(false, "The frame's transient memory is used up.");
      return {};
    }

    _transientOffset = offset + size;
    const uint64 base = uint64(_frameIndex) * _desc.TransientMemoryPerFrame + offset;
    return {.Buffer = _transientBuffer, .Offset = base, .Data = Span<byte>(_transientMemory + base, size)};
  }

  // MARK: - Frames

  void GLDevice::BeginFrame() noexcept
  {
    krys_debug_assert_msg(!_inFrame, "BeginFrame called twice.");
    if (GLsync &fence = _frameFences[_frameIndex]; fence != nullptr)
    {
      while (glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1'000'000'000ull) == GL_TIMEOUT_EXPIRED)
      {
      }
      glDeleteSync(fence);
      fence = nullptr;
    }
    _transientOffset = 0u;
    _commandBuffer.Reset();
    _inFrame = true;
    ++_frameNumber;
  }

  CommandBuffer &GLDevice::GetCommandBuffer() noexcept
  {
    return _commandBuffer;
  }

  void GLDevice::Present(SwapchainHandle swapchain) noexcept
  {
    auto *existing = _swapchains.TryGet(swapchain);
    krys_debug_assert_msg(existing != nullptr, "Presenting a stale or invalid swapchain.");
    if (existing != nullptr)
    {
      // Drivers may keep the interval per window, so each swapchain tracks what it last set.
      int interval = existing->VSync ? 1 : 0;
      if (interval != existing->SwapInterval)
      {
        _context->SetSwapInterval(existing->Surface, interval);
        existing->SwapInterval = interval;
      }
      _context->SwapBuffers(existing->Surface);
    }
  }

  void GLDevice::EndFrame() noexcept
  {
    krys_debug_assert_msg(_inFrame, "EndFrame without BeginFrame.");
    _frameFences[_frameIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    _frameIndex = (_frameIndex + 1u) % FramesInFlight;
    _inFrame = false;
  }

  void GLDevice::WaitIdle() noexcept
  {
    glFinish();
  }

  uint64 GLDevice::FrameNumber() const noexcept
  {
    return _frameNumber;
  }

  const DeviceLimits &GLDevice::Limits() const noexcept
  {
    return _limits;
  }

  Maybe<TextureDesc> GLDevice::GetTextureDesc(TextureHandle texture) const noexcept
  {
    if (const auto *existing = _textures.TryGet(texture))
    {
      return existing->Desc;
    }
    return {};
  }

  // MARK: - Replay

  size_t GLDevice::FramebufferKeyHash::operator()(const FramebufferKey &key) const noexcept
  {
    size_t seed = key.ColourCount;
    for (uint32 i = 0u; i < key.Textures.size(); ++i)
    {
      size_t value = (size_t(key.Textures[i]) << 32u) ^ (size_t(key.MipLevels[i]) << 16u) ^ key.Layers[i];
      seed ^= value + 0x9e'37'79'b9'7f'4a'7c'55 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }

  GLuint GLDevice::GetFramebuffer(const FramebufferKey &key) noexcept
  {
    if (auto it = _framebuffers.find(key); it != _framebuffers.end())
    {
      return it->second;
    }

    GLuint framebuffer = 0u;
    glCreateFramebuffers(1, &framebuffer);

    auto attach = [&](GLenum attachment, uint32 index)
    {
      const GLuint texture = key.Textures[index];
      GLint target = 0;
      glGetTextureParameteriv(texture, GL_TEXTURE_TARGET, &target);
      const bool layered =
        target == GL_TEXTURE_2D_ARRAY || target == GL_TEXTURE_3D || target == GL_TEXTURE_CUBE_MAP;
      if (layered)
      {
        glNamedFramebufferTextureLayer(framebuffer, attachment, texture, key.MipLevels[index],
                                       key.Layers[index]);
      }
      else
      {
        glNamedFramebufferTexture(framebuffer, attachment, texture, key.MipLevels[index]);
      }
    };

    Array<GLenum, MaxColourAttachments> drawBuffers {};
    for (uint32 i = 0u; i < key.ColourCount; ++i)
    {
      attach(GL_COLOR_ATTACHMENT0 + i, i);
      drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }
    glNamedFramebufferDrawBuffers(framebuffer, static_cast<GLsizei>(key.ColourCount), drawBuffers.data());

    if (const GLuint depth = key.Textures[MaxColourAttachments]; depth != 0u)
    {
      GLint internalFormat = 0;
      glGetTextureLevelParameteriv(depth, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
      attach(internalFormat == GL_DEPTH24_STENCIL8 ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT,
             MaxColourAttachments);
    }

    KRYS_MAYBE_UNUSED const GLenum status = glCheckNamedFramebufferStatus(framebuffer, GL_FRAMEBUFFER);
    krys_assert_msg(status == GL_FRAMEBUFFER_COMPLETE, "The render pass's targets can't be combined.");
    _framebuffers.emplace(key, framebuffer);
    return framebuffer;
  }

  void GLDevice::Submit(CommandBuffer &commands) noexcept
  {
    krys_assert_msg(commands.IsComplete(), "A render pass or debug group was left open.");

    ReplayState replay {};
    CommandListReader reader(commands.Commands());
    while (reader.HasMore())
    {
      const CommandHeader header = reader.ReadHeader();
      switch (header.Type)
      {
        case commands::BeginRenderPass::Type:
          ExecuteBeginRenderPass(replay, reader.ReadCommand<commands::BeginRenderPass>());
          break;
        case commands::EndRenderPass::Type:
          static_cast<void>(reader.ReadCommand<commands::EndRenderPass>());
          ExecuteEndRenderPass(replay);
          break;
        case commands::SetViewport::Type:
        {
          const auto &command = reader.ReadCommand<commands::SetViewport>();
          const float y =
            replay.TargetIsSwapchain ? float(replay.TargetHeight) - (command.Y + command.Height) : command.Y;
          glViewportIndexedf(0, command.X, y, command.Width, command.Height);
          glDepthRangef(command.MinDepth, command.MaxDepth);
          break;
        }
        case commands::SetScissor::Type:
        {
          const auto &command = reader.ReadCommand<commands::SetScissor>();
          const int32 y = replay.TargetIsSwapchain
                            ? int32(replay.TargetHeight) - (command.Y + static_cast<int32>(command.Height))
                            : command.Y;
          glScissor(command.X, y, static_cast<GLsizei>(command.Width), static_cast<GLsizei>(command.Height));
          break;
        }
        case commands::SetBlendConstant::Type:
        {
          const auto &command = reader.ReadCommand<commands::SetBlendConstant>();
          glBlendColor(command.Constant[0], command.Constant[1], command.Constant[2], command.Constant[3]);
          break;
        }
        case commands::SetStencilReference::Type:
        {
          const auto &command = reader.ReadCommand<commands::SetStencilReference>();
          _state.StencilReference = command.Reference;
          if (replay.Pipeline != nullptr)
          {
            const auto &stencil = replay.Pipeline->DepthStencil;
            glStencilFuncSeparate(GL_FRONT, MapCompareOp(stencil.StencilFront.Compare),
                                  static_cast<GLint>(command.Reference), stencil.StencilReadMask);
            glStencilFuncSeparate(GL_BACK, MapCompareOp(stencil.StencilBack.Compare),
                                  static_cast<GLint>(command.Reference), stencil.StencilReadMask);
          }
          break;
        }
        case commands::BindPipeline::Type:
          ExecuteBindPipeline(replay, reader.ReadCommand<commands::BindPipeline>().Pipeline);
          break;
        case commands::BindVertexBuffer::Type:
        {
          const auto &command = reader.ReadCommand<commands::BindVertexBuffer>();
          const auto *buffer = _buffers.TryGet(command.Buffer);
          krys_debug_assert_msg(buffer != nullptr && command.Slot < replay.VertexBuffers.size(),
                                "Invalid vertex buffer binding.");
          if (buffer != nullptr && command.Slot < replay.VertexBuffers.size())
          {
            replay.VertexBuffers[command.Slot] = {buffer->Id, command.Offset};
          }
          break;
        }
        case commands::BindIndexBuffer::Type:
        {
          const auto &command = reader.ReadCommand<commands::BindIndexBuffer>();
          const auto *buffer = _buffers.TryGet(command.Buffer);
          krys_debug_assert_msg(buffer != nullptr, "Invalid index buffer.");
          replay.IndexBuffer = buffer != nullptr ? buffer->Id : 0u;
          replay.IndexOffset = command.Offset;
          replay.IndexType = command.IndexType;
          break;
        }
        case commands::BindUniformBuffer::Type:
        {
          const auto &command = reader.ReadCommand<commands::BindUniformBuffer>();
          const auto *buffer = _buffers.TryGet(command.Buffer);
          krys_debug_assert_msg(buffer != nullptr, "Invalid uniform buffer.");
          if (buffer != nullptr)
          {
            glBindBufferRange(GL_UNIFORM_BUFFER, command.Slot, buffer->Id,
                              static_cast<GLintptr>(command.Offset), static_cast<GLsizeiptr>(command.Size));
          }
          break;
        }
        case commands::BindTexture::Type:
        {
          const auto &command = reader.ReadCommand<commands::BindTexture>();
          const auto *texture = _textures.TryGet(command.Texture);
          const auto *sampler = _samplers.TryGet(command.Sampler);
          krys_debug_assert_msg(texture != nullptr && sampler != nullptr, "Invalid texture or sampler.");
          if (texture != nullptr && sampler != nullptr)
          {
            glBindTextureUnit(command.Slot, texture->Id);
            glBindSampler(command.Slot, sampler->Id);
          }
          break;
        }
        case commands::Draw::Type:
        {
          const auto &command = reader.ReadCommand<commands::Draw>();
          PrepareDraw(replay);
          if (replay.Pipeline != nullptr)
          {
            glDrawArraysInstancedBaseInstance(
              replay.Pipeline->Topology, static_cast<GLint>(command.FirstVertex),
              static_cast<GLsizei>(command.VertexCount), static_cast<GLsizei>(command.InstanceCount),
              command.FirstInstance);
          }
          break;
        }
        case commands::DrawIndexed::Type:
        {
          const auto &command = reader.ReadCommand<commands::DrawIndexed>();
          PrepareDraw(replay);
          if (replay.Pipeline != nullptr)
          {
            const uint64 offset =
              replay.IndexOffset + uint64(command.FirstIndex) * IndexSize(replay.IndexType);
            glDrawElementsInstancedBaseVertexBaseInstance(
              replay.Pipeline->Topology, static_cast<GLsizei>(command.IndexCount),
              replay.IndexType == IndexType::UInt16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
              reinterpret_cast<const void *>(static_cast<uintptr_t>(offset)),
              static_cast<GLsizei>(command.InstanceCount), command.VertexOffset, command.FirstInstance);
          }
          break;
        }
        case commands::CopyBufferToTexture::Type:
          ExecuteCopyBufferToTexture(reader.ReadCommand<commands::CopyBufferToTexture>());
          break;
        case commands::BlitTexture::Type:
          ExecuteBlitTexture(reader.ReadCommand<commands::BlitTexture>());
          break;
        case commands::PushDebugGroup::Type:
        {
          const auto &command = reader.ReadCommand<commands::PushDebugGroup>();
          glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0u, -1, command.Name.data());
          break;
        }
        case commands::PopDebugGroup::Type:
          static_cast<void>(reader.ReadCommand<commands::PopDebugGroup>());
          glPopDebugGroup();
          break;
        default:
          krys_assert_msg(false, "Unknown graphics command.");
          reader.SkipBytes(header.SizeInBytes);
          break;
      }
    }

    commands.Reset();
  }

  void GLDevice::ExecuteBeginRenderPass(ReplayState &replay,
                                        const commands::BeginRenderPass &command) noexcept
  {
    // Buffer bindings carry over between passes; a pass needs its own pipeline.
    replay.InRenderPass = true;
    replay.TargetIsSwapchain = false;
    replay.Pipeline = nullptr;
    replay.DiscardCount = 0u;

    uint32 width = 0u;
    uint32 height = 0u;
    const auto &first = command.ColourAttachments[0];
    if (command.ColourAttachmentCount > 0u && first.Swapchain.IsValid())
    {
      krys_assert_msg(command.ColourAttachmentCount == 1u && !command.DepthStencil.Texture.IsValid(),
                      "A swapchain is a render pass's only target.");
      const auto *swapchain = _swapchains.TryGet(first.Swapchain);
      krys_assert_msg(swapchain != nullptr, "Rendering to a stale or invalid swapchain.");
      if (swapchain == nullptr)
      {
        return;
      }

      _context->MakeCurrent(swapchain->Surface);
      glBindFramebuffer(GL_FRAMEBUFFER, 0u);
      // The window's origin is bottom-left; NDC y up already puts the top of the image at the top.
      glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
      SetEnabled(GL_FRAMEBUFFER_SRGB, IsSrgbFormat(swapchain->Format));
      replay.TargetIsSwapchain = true;
      replay.Framebuffer = 0u;
      width = swapchain->Width;
      height = swapchain->Height;
    }
    else
    {
      FramebufferKey key {};
      key.ColourCount = command.ColourAttachmentCount;
      auto describe = [&](uint32 index, TextureHandle handle, uint32 mip, uint32 layer)
      {
        const auto *texture = _textures.TryGet(handle);
        krys_assert_msg(texture != nullptr, "Rendering to a stale or invalid texture.");
        if (texture == nullptr)
        {
          return;
        }
        key.Textures[index] = texture->Id;
        key.MipLevels[index] = static_cast<uint16>(mip);
        key.Layers[index] = static_cast<uint16>(layer);
        if (width == 0u)
        {
          width = MipSize(texture->Desc.Width, mip);
          height = MipSize(texture->Desc.Height, mip);
        }
      };

      for (uint32 i = 0u; i < command.ColourAttachmentCount; ++i)
      {
        const auto &attachment = command.ColourAttachments[i];
        describe(i, attachment.Texture, attachment.MipLevel, attachment.Layer);
      }
      if (command.DepthStencil.Texture.IsValid())
      {
        describe(MaxColourAttachments, command.DepthStencil.Texture, 0u, 0u);
      }

      replay.Framebuffer = GetFramebuffer(key);
      glBindFramebuffer(GL_FRAMEBUFFER, replay.Framebuffer);
      // Texture rows start at the top of the image, so window y runs down; this flips the winding too.
      glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);
      glEnable(GL_FRAMEBUFFER_SRGB);
    }

    replay.TargetHeight = height;
    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    glDepthRangef(0.f, 1.f);
    glScissor(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

    // Clears obey the scissor test and write masks; open them up, and let the next pipeline reapply its
    // state.
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glStencilMask(0xFFu);
    _state.Valid = false;

    for (uint32 i = 0u; i < command.ColourAttachmentCount; ++i)
    {
      const auto &attachment = command.ColourAttachments[i];
      if (attachment.Load == AttachmentLoadOp::Clear)
      {
        const auto *texture = _textures.TryGet(attachment.Texture);
        if (texture != nullptr && IsIntegerFormat(texture->Desc.Format))
        {
          const GLuint values[4] = {GLuint(attachment.ClearColour[0]), GLuint(attachment.ClearColour[1]),
                                    GLuint(attachment.ClearColour[2]), GLuint(attachment.ClearColour[3])};
          glClearNamedFramebufferuiv(replay.Framebuffer, GL_COLOR, static_cast<GLint>(i), values);
        }
        else
        {
          glClearNamedFramebufferfv(replay.Framebuffer, GL_COLOR, static_cast<GLint>(i),
                                    attachment.ClearColour.data());
        }
      }
      if (attachment.Store == AttachmentStoreOp::DontCare && !replay.TargetIsSwapchain)
      {
        replay.Discards[replay.DiscardCount++] = GL_COLOR_ATTACHMENT0 + i;
      }
    }

    const auto &depthStencil = command.DepthStencil;
    if (const auto *texture = _textures.TryGet(depthStencil.Texture))
    {
      const bool stencil = HasStencil(texture->Desc.Format);
      const bool clearDepth = depthStencil.DepthLoad == AttachmentLoadOp::Clear;
      const bool clearStencil = stencil && depthStencil.StencilLoad == AttachmentLoadOp::Clear;
      if (clearDepth && clearStencil)
      {
        glClearNamedFramebufferfi(replay.Framebuffer, GL_DEPTH_STENCIL, 0, depthStencil.ClearDepth,
                                  depthStencil.ClearStencil);
      }
      else if (clearDepth)
      {
        glClearNamedFramebufferfv(replay.Framebuffer, GL_DEPTH, 0, &depthStencil.ClearDepth);
      }
      else if (clearStencil)
      {
        const GLint value = depthStencil.ClearStencil;
        glClearNamedFramebufferiv(replay.Framebuffer, GL_STENCIL, 0, &value);
      }

      const bool discardDepth = depthStencil.DepthStore == AttachmentStoreOp::DontCare;
      const bool discardStencil = !stencil || depthStencil.StencilStore == AttachmentStoreOp::DontCare;
      if (discardDepth && discardStencil)
      {
        replay.Discards[replay.DiscardCount++] = DepthStencilAttachmentPoint(texture->Desc.Format);
      }
      else if (discardDepth)
      {
        replay.Discards[replay.DiscardCount++] = GL_DEPTH_ATTACHMENT;
      }
      else if (stencil && depthStencil.StencilStore == AttachmentStoreOp::DontCare)
      {
        replay.Discards[replay.DiscardCount++] = GL_STENCIL_ATTACHMENT;
      }
    }
  }

  void GLDevice::ExecuteEndRenderPass(ReplayState &replay) noexcept
  {
    if (replay.DiscardCount > 0u)
    {
      glInvalidateNamedFramebufferData(replay.Framebuffer, static_cast<GLsizei>(replay.DiscardCount),
                                       replay.Discards.data());
    }
    replay.InRenderPass = false;
    replay.Pipeline = nullptr;
    replay.DiscardCount = 0u;
  }

  void GLDevice::ExecuteBindPipeline(ReplayState &replay, PipelineHandle handle) noexcept
  {
    auto *pipeline = _pipelines.TryGet(handle);
    krys_debug_assert_msg(pipeline != nullptr, "Binding a stale or invalid pipeline.");
    replay.Pipeline = pipeline;
    if (pipeline == nullptr)
    {
      return;
    }

    glUseProgram(pipeline->Program);
    glBindVertexArray(pipeline->VertexArray);
    ApplyPipelineState(*pipeline, !replay.TargetIsSwapchain);
  }

  void GLDevice::ApplyPipelineState(const GLPipeline &pipeline, bool flipFrontFace) noexcept
  {
    const bool all = !_state.Valid;

    if (all || pipeline.Blend != _state.Blend)
    {
      const auto &blend = pipeline.Blend;
      SetEnabled(GL_BLEND, blend.Enabled);
      glBlendFuncSeparate(MapBlendFactor(blend.SourceColour), MapBlendFactor(blend.DestinationColour),
                          MapBlendFactor(blend.SourceAlpha), MapBlendFactor(blend.DestinationAlpha));
      glBlendEquationSeparate(MapBlendOp(blend.ColourOp), MapBlendOp(blend.AlphaOp));
      glColorMask(
        HasFlag(blend.WriteMask, ColourWriteMask::Red), HasFlag(blend.WriteMask, ColourWriteMask::Green),
        HasFlag(blend.WriteMask, ColourWriteMask::Blue), HasFlag(blend.WriteMask, ColourWriteMask::Alpha));
      _state.Blend = blend;
    }

    if (all || pipeline.DepthStencil != _state.DepthStencil)
    {
      const auto &depthStencil = pipeline.DepthStencil;
      SetEnabled(GL_DEPTH_TEST, depthStencil.DepthTest);
      glDepthMask(depthStencil.DepthWrite ? GL_TRUE : GL_FALSE);
      glDepthFunc(MapCompareOp(depthStencil.DepthCompare));

      SetEnabled(GL_STENCIL_TEST, depthStencil.StencilTest);
      const auto reference = static_cast<GLint>(_state.StencilReference);
      for (auto [face, state] :
           {std::pair {GL_FRONT, depthStencil.StencilFront}, std::pair {GL_BACK, depthStencil.StencilBack}})
      {
        glStencilFuncSeparate(face, MapCompareOp(state.Compare), reference, depthStencil.StencilReadMask);
        glStencilOpSeparate(face, MapStencilOp(state.Fail), MapStencilOp(state.DepthFail),
                            MapStencilOp(state.Pass));
      }
      glStencilMask(depthStencil.StencilWriteMask);
      _state.DepthStencil = depthStencil;
    }

    if (all || pipeline.Raster != _state.Raster || flipFrontFace != _state.FrontFaceFlipped)
    {
      const auto &raster = pipeline.Raster;
      SetEnabled(GL_CULL_FACE, raster.Cull != CullMode::None);
      if (raster.Cull != CullMode::None)
      {
        glCullFace(raster.Cull == CullMode::Front ? GL_FRONT : GL_BACK);
      }
      const bool counterClockwise = (raster.Front == FrontFace::CounterClockwise) != flipFrontFace;
      glFrontFace(counterClockwise ? GL_CCW : GL_CW);
      glPolygonMode(GL_FRONT_AND_BACK, raster.Fill == FillMode::Wireframe ? GL_LINE : GL_FILL);
      SetEnabled(GL_SCISSOR_TEST, raster.ScissorTest);
      _state.Raster = raster;
      _state.FrontFaceFlipped = flipFrontFace;
    }

    _state.Valid = true;
  }

  void GLDevice::PrepareDraw(ReplayState &replay) noexcept
  {
    krys_debug_assert_msg(replay.Pipeline != nullptr, "Drawing without a pipeline.");
    if (replay.Pipeline == nullptr)
    {
      return;
    }

    auto &pipeline = *replay.Pipeline;
    for (uint32 slot = 0u; slot < pipeline.Strides.size(); ++slot)
    {
      const auto &wanted = replay.VertexBuffers[slot];
      if (pipeline.AppliedVertexBuffers[slot] != wanted)
      {
        glVertexArrayVertexBuffer(pipeline.VertexArray, slot, wanted.Buffer,
                                  static_cast<GLintptr>(wanted.Offset),
                                  static_cast<GLsizei>(pipeline.Strides[slot]));
        pipeline.AppliedVertexBuffers[slot] = wanted;
      }
    }

    if (pipeline.AppliedIndexBuffer != replay.IndexBuffer)
    {
      glVertexArrayElementBuffer(pipeline.VertexArray, replay.IndexBuffer);
      pipeline.AppliedIndexBuffer = replay.IndexBuffer;
    }
  }

  void GLDevice::ExecuteCopyBufferToTexture(const commands::CopyBufferToTexture &command) noexcept
  {
    const auto *buffer = _buffers.TryGet(command.Source);
    const auto *texture = _textures.TryGet(command.Destination);
    krys_debug_assert_msg(buffer != nullptr && texture != nullptr, "Invalid copy source or destination.");
    if (buffer == nullptr || texture == nullptr)
    {
      return;
    }

    const auto &region = command.Region;
    const auto transfer = MapTransferFormat(texture->Desc.Format);
    const auto *offset = reinterpret_cast<const void *>(static_cast<uintptr_t>(command.SourceOffset));
    const auto mip = static_cast<GLint>(region.MipLevel);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer->Id);
    switch (texture->Desc.Dimension)
    {
      case TextureDimension::Texture1D:
        glTextureSubImage1D(texture->Id, mip, static_cast<GLint>(region.X),
                            static_cast<GLsizei>(region.Width), transfer.Format, transfer.Type, offset);
        break;
      case TextureDimension::Texture2D:
        glTextureSubImage2D(texture->Id, mip, static_cast<GLint>(region.X), static_cast<GLint>(region.Y),
                            static_cast<GLsizei>(region.Width), static_cast<GLsizei>(region.Height),
                            transfer.Format, transfer.Type, offset);
        break;
      default:
        glTextureSubImage3D(texture->Id, mip, static_cast<GLint>(region.X), static_cast<GLint>(region.Y),
                            static_cast<GLint>(region.Z), static_cast<GLsizei>(region.Width),
                            static_cast<GLsizei>(region.Height), static_cast<GLsizei>(region.Depth),
                            transfer.Format, transfer.Type, offset);
        break;
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0u);
  }

  void GLDevice::ExecuteBlitTexture(const commands::BlitTexture &command) noexcept
  {
    const auto *source = _textures.TryGet(command.Source);
    const auto *destination = _textures.TryGet(command.Destination);
    krys_debug_assert_msg(source != nullptr && destination != nullptr, "Invalid blit source or destination.");
    if (source == nullptr || destination == nullptr)
    {
      return;
    }

    const bool depth = IsDepthFormat(source->Desc.Format);
    const GLenum attachment = depth ? DepthStencilAttachmentPoint(source->Desc.Format) : GL_COLOR_ATTACHMENT0;
    auto attach = [&](GLuint framebuffer, const GLTexture &texture, const TextureRegion &region)
    {
      if (IsLayered(texture.Desc.Dimension))
      {
        glNamedFramebufferTextureLayer(framebuffer, attachment, texture.Id,
                                       static_cast<GLint>(region.MipLevel), static_cast<GLint>(region.Z));
      }
      else
      {
        glNamedFramebufferTexture(framebuffer, attachment, texture.Id, static_cast<GLint>(region.MipLevel));
      }
    };
    attach(_blitFramebuffers[0], *source, command.SourceRegion);
    attach(_blitFramebuffers[1], *destination, command.DestinationRegion);
    if (!depth)
    {
      glNamedFramebufferReadBuffer(_blitFramebuffers[0], GL_COLOR_ATTACHMENT0);
      glNamedFramebufferDrawBuffer(_blitFramebuffers[1], GL_COLOR_ATTACHMENT0);
    }

    // Blits obey the scissor test.
    glDisable(GL_SCISSOR_TEST);
    _state.Valid = false;

    const auto &from = command.SourceRegion;
    const auto &to = command.DestinationRegion;
    glBlitNamedFramebuffer(
      _blitFramebuffers[0], _blitFramebuffers[1], static_cast<GLint>(from.X), static_cast<GLint>(from.Y),
      static_cast<GLint>(from.X + from.Width), static_cast<GLint>(from.Y + from.Height),
      static_cast<GLint>(to.X), static_cast<GLint>(to.Y), static_cast<GLint>(to.X + to.Width),
      static_cast<GLint>(to.Y + to.Height), depth ? GL_DEPTH_BUFFER_BIT : GL_COLOR_BUFFER_BIT,
      depth || command.Filter == SamplerFilter::Nearest ? GL_NEAREST : GL_LINEAR);
    glNamedFramebufferTexture(_blitFramebuffers[0], attachment, 0u, 0);
    glNamedFramebufferTexture(_blitFramebuffers[1], attachment, 0u, 0);
  }
}
