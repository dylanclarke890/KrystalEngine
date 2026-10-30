#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Gfx/Device.hpp"
#include "Krystal.Gfx/Formats.hpp"
#include "Krystal.Gfx/Handles.hpp"
#include "Krystal.Gfx/Render2D/Rect2D.hpp"

namespace krys::gfx
{
  struct AtlasPagesDesc
  {
    TextureFormat Format {TextureFormat::R8Unorm};
    /// @brief The width and height of each page, in texels.
    uint32 PageSize {1'024u};
    /// @brief The most pages there are; past that, the least recently used page is cleared.
    uint32 MaxPages {4u};
    utf8_stringview DebugName {u8"Render2D.AtlasPage"};
  };

  /// @brief Space on an atlas page.
  struct AtlasSlot
  {
    uint32 Page {0u};
    PixelRect2D Texels {};
  };

  /// @brief Texture pages that rects are packed into in shelves, shared by the mask and glyph atlases.
  ///
  /// Rects go on the lowest shelf they fit, so tall shelves aren't spent on short rects; shelves are rounded
  /// up to 8 texels so similar heights share them. Space is reclaimed a page at a time: with every page
  /// allocated and full, the least recently used page that the current frame hasn't drawn from is cleared,
  /// since overwriting what unsubmitted draws read would change them.
  class AtlasPages
  {
    KRYS_NON_COPY_MOVABLE(AtlasPages);

    struct Shelf
    {
      uint32 Y {0u};
      uint32 Height {0u};
      uint32 NextX {0u};
    };

    struct Page
    {
      TextureHandle Texture {};
      List<Shelf> Shelves {};
      uint32 NextShelfY {0u};
      uint64 LastUsedFrame {0u};
    };

  private:
    Device *_device;
    AtlasPagesDesc _desc;
    List<Page> _pages {};
    uint32 _evictions {0u};

  public:
    AtlasPages(Device &device, const AtlasPagesDesc &desc) noexcept;
    ~AtlasPages() noexcept;

    /// @brief Space for a `width` by `height` rect, on a page with room, a new page, or the least recently
    /// used page cleared; `evicted` is called with a cleared page's index first, so what was on it can be
    /// forgotten. Nothing when the rect is larger than a page, or every page was drawn from this frame.
    KRYS_NODISCARD Maybe<AtlasSlot> Allocate(uint32 width, uint32 height,
                                             const Func<void(uint32)> &evicted) noexcept;
    /// @brief Marks a page as drawn from this frame, so it isn't cleared before the frame is submitted.
    void MarkUsed(uint32 page) noexcept;
    /// @brief Writes texels into a slot, rows from the top, tightly packed.
    void Write(const AtlasSlot &slot, Span<const byte> texels) noexcept;

    KRYS_NODISCARD TextureHandle Texture(uint32 page) const noexcept
    {
      return _pages[page].Texture;
    }

    KRYS_NODISCARD uint32 PageCount() const noexcept
    {
      return static_cast<uint32>(_pages.size());
    }

    /// @brief How many pages have been cleared to make room.
    KRYS_NODISCARD uint32 Evictions() const noexcept
    {
      return _evictions;
    }

    KRYS_NODISCARD const AtlasPagesDesc &Desc() const noexcept
    {
      return _desc;
    }

  private:
    KRYS_NODISCARD Maybe<PixelRect2D> AllocateInPage(Page &page, uint32 width, uint32 height) const noexcept;
  };
}
