#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Gfx/Device.hpp"
#include "Krystal.Gfx/Handles.hpp"
#include "Krystal.Gfx/Render2D/AtlasPages.hpp"
#include "Krystal.Gfx/Render2D/Path2D.hpp"
#include "Krystal.Gfx/Render2D/PathRasteriser.hpp"
#include "Krystal.Gfx/Render2D/Rect2D.hpp"
#include "Krystal.Gfx/Render2D/Transform2D.hpp"

namespace krys::gfx
{
  struct MaskAtlasDesc
  {
    /// @brief The width and height of each atlas page, in pixels.
    uint32 PageSize {1'024u};
    /// @brief The most pages the atlas allocates; past that, it evicts the least recently used page.
    uint32 MaxPages {4u};
    /// @brief Masks wider or taller than this get a texture of their own instead of atlas space.
    uint32 MaxPackedSize {256u};
  };

  /// @brief Where a mask's coverage is, and which target pixels it covers.
  struct MaskLocation
  {
    /// @brief The R8 texture holding the coverage.
    TextureHandle Texture {};
    /// @brief The mask's texels in the texture.
    PixelRect2D Texels {};
    /// @brief The target pixels the mask covers; empty when the path covers none.
    PixelRect2D Bounds {};
  };

  struct MaskAtlasStats
  {
    /// @brief Masks rasterised because they weren't cached.
    uint32 Rasterised {0u};
    /// @brief Masks found in the cache.
    uint32 Hits {0u};
    /// @brief Pages cleared to make room.
    uint32 PageEvictions {0u};
  };

  /// @brief Rasterises paths into coverage masks and caches them, packed into R8 texture pages in shelves.
  ///
  /// A mask is cached under its path, fill rule, the transform's linear part and the fractional part of its
  /// translation, so it's reused exactly as rasterised: moving a path by whole pixels reuses its mask, and
  /// any other change rasterises a new one. Blurred masks, for shadows, are cached under their blur too.
  /// Masks aren't clipped; the clip is applied when they're drawn.
  ///
  /// Masks are packed into R8 pages (`AtlasPages`), which reclaim space a page at a time. Masks larger than
  /// `MaxPackedSize`, or that find no page, get textures of their own, destroyed once a frame passes without
  /// them. Masks too large for a
  /// texture are rasterised within the clip only, and not kept past the frame.
  class MaskAtlas
  {
    KRYS_NON_COPY_MOVABLE(MaskAtlas);

    /// @brief An entry's page when the mask isn't on one.
    constexpr static uint32 NoPage = ~0u;

    struct Key
    {
      Path2D Path;
      Array<float, 4> Linear;
      Array<float, 2> Fraction;
      FillRule Rule;
      /// @brief The blur's standard deviation, in pixels; 0 for none.
      float Blur;

      KRYS_NODISCARD bool operator==(const Key &) const noexcept = default;
    };

    struct KeyHash
    {
      KRYS_NODISCARD size_t operator()(const Key &key) const noexcept;
    };

    struct Entry
    {
      /// @brief The page holding the mask, or `NoPage`.
      uint32 Page {NoPage};
      /// @brief The mask's own texture, when it isn't on a page; invalid for masks that cover nothing.
      TextureHandle OwnTexture {};
      PixelRect2D Texels {};
      /// @brief The pixels the mask covers, before the whole-pixel part of the translation.
      PixelRect2D Bounds {};
      uint64 LastUsedFrame {0u};
    };

    struct RetiredTexture
    {
      TextureHandle Texture {};
      /// @brief The frame that last drew from it.
      uint64 Frame {0u};
    };

  private:
    Device *_device;
    MaskAtlasDesc _desc;
    PathRasteriser _rasteriser {};
    AtlasPages _pages;
    HashMap<Key, Entry, KeyHash> _entries {};
    List<RetiredTexture> _retired {};
    MaskAtlasStats _stats {};
    uint64 _collectedFrame {0u};

  public:
    MaskAtlas(Device &device, const MaskAtlasDesc &desc) noexcept;
    ~MaskAtlas() noexcept;

    /// @brief The mask of `path`, mapped into target pixels by `transform` and blurred by a Gaussian with
    /// standard deviation `blur` pixels (for shadows), rasterising it if it isn't cached. `clip` only limits
    /// masks too large to cache.
    KRYS_NODISCARD MaskLocation GetMask(const Path2D &path, const Transform2D &transform, FillRule rule,
                                        const PixelRect2D &clip, float blur = 0.f) noexcept;
    /// @brief Uploads a mask made elsewhere into a texture of its own, kept for this frame only.
    KRYS_NODISCARD MaskLocation UploadTransient(const CoverageMask &mask) noexcept;

    KRYS_NODISCARD const MaskAtlasStats &Stats() const noexcept
    {
      return _stats;
    }

    void ResetStats() noexcept
    {
      _stats = {};
    }

    KRYS_NODISCARD uint32 PageCount() const noexcept
    {
      return _pages.PageCount();
    }

  private:
    /// @brief Once per frame: destroys the textures no frame still needs.
    void Collect() noexcept;
    /// @brief Finds page space, forgetting the masks of any page cleared to make room.
    KRYS_NODISCARD Maybe<AtlasSlot> Allocate(uint32 width, uint32 height) noexcept;
    KRYS_NODISCARD TextureHandle CreateMaskTexture(uint32 width, uint32 height,
                                                   utf8_stringview name) noexcept;
    void Upload(TextureHandle texture, const PixelRect2D &texels, const CoverageMask &mask) noexcept;
  };
}
