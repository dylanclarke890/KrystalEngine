#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Gfx/CommandBuffer.hpp"
#include "Krystal.Gfx/Device.hpp"
#include "Krystal.Gfx/Formats.hpp"
#include "Krystal.Gfx/Handles.hpp"
#include "Krystal.Gfx/Render2D/FilterGraph2D.hpp"

namespace krys::gfx
{
  /// @brief Evaluates filter graphs on the GPU: each node is a pass (a blur or morphology two) into a texture
  /// of its own.
  ///
  /// `Render` records a graph's passes into a command buffer at once; while a `Renderer2D` frame is open on
  /// it, they come before the frame's passes, so the frame can draw the result. Results stay valid until the
  /// frame after next: textures aren't reused within a frame.
  class FilterRenderer2D
  {
    KRYS_NON_COPY_MOVABLE(FilterRenderer2D);

    struct PooledTexture
    {
      TextureHandle Texture {};
      uint32 Width {0u};
      uint32 Height {0u};
      uint64 LastUsedFrame {0u};
    };

    struct TurbulenceTable
    {
      int32 Seed {0};
      TextureHandle Texture {};
      uint64 LastUsedFrame {0u};
    };

  public:
    /// @brief The node textures' format.
    constexpr static TextureFormat Format = TextureFormat::RGBA8Unorm;

  private:
    Device *_device;
    ShaderHandle _vertexShader {};
    ShaderHandle _fragmentShader {};
    PipelineHandle _pipeline {};
    SamplerHandle _sampler {};
    /// @brief Bound for inputs a node doesn't have.
    TextureHandle _transparentTexture {};
    List<PooledTexture> _textures {};
    List<TurbulenceTable> _turbulenceTables {};

  public:
    KRYS_NODISCARD static Expected<UniquePtr<FilterRenderer2D>> Create(Device &device) noexcept;

    /// @brief Use `Create`.
    explicit FilterRenderer2D(Device &device) noexcept;
    ~FilterRenderer2D() noexcept;

    /// @brief Records `graph`'s passes into `commands`; nothing when the graph has no nodes or its result is
    /// empty.
    KRYS_NODISCARD Maybe<FilterResult2D> Render(CommandBuffer &commands, const FilterGraph2D &graph) noexcept;

  private:
    KRYS_NODISCARD Expected<void> Initialise() noexcept;
    /// @brief A texture at least `width` by `height` that this frame hasn't used.
    KRYS_NODISCARD TextureHandle AcquireTexture(uint32 width, uint32 height) noexcept;
    /// @brief The lattice and gradients of the turbulence function for `seed` (after the seed's clamping).
    KRYS_NODISCARD TextureHandle TurbulenceTableFor(int32 seed) noexcept;
  };
}
