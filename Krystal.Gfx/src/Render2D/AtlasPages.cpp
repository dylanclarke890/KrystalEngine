#include "Krystal.Gfx/Render2D/AtlasPages.hpp"
#include "Krystal.Core/Assert.hpp"
#include <algorithm>

namespace krys::gfx
{
  namespace
  {
    /// @brief Shelf heights are rounded up to this, so rects of similar heights share shelves.
    constexpr uint32 ShelfGranularity = 8u;
  }

  AtlasPages::AtlasPages(Device &device, const AtlasPagesDesc &desc) noexcept : _device(&device), _desc(desc)
  {
  }

  AtlasPages::~AtlasPages() noexcept
  {
    for (const auto &page : _pages)
    {
      _device->Destroy(page.Texture);
    }
  }

  Maybe<AtlasSlot> AtlasPages::Allocate(uint32 width, uint32 height,
                                        const Func<void(uint32)> &evicted) noexcept
  {
    if (width > _desc.PageSize || height > _desc.PageSize)
    {
      return {};
    }

    for (uint32 index = 0u; index < _pages.size(); ++index)
    {
      if (auto texels = AllocateInPage(_pages[index], width, height))
      {
        return AtlasSlot {.Page = index, .Texels = *texels};
      }
    }

    if (_pages.size() < _desc.MaxPages)
    {
      auto texture = _device->CreateTexture({.Format = _desc.Format,
                                             .Width = _desc.PageSize,
                                             .Height = _desc.PageSize,
                                             .DebugName = _desc.DebugName});
      krys_debug_assert_msg(texture.has_value(), "Couldn't create an atlas page.");
      if (texture)
      {
        _pages.push_back({.Texture = *texture});
        if (auto texels = AllocateInPage(_pages.back(), width, height))
        {
          return AtlasSlot {.Page = static_cast<uint32>(_pages.size() - 1u), .Texels = *texels};
        }
      }
    }

    // Clear the least recently used page, unless the current frame has drawn from every page: overwriting
    // texels its unsubmitted draws read would change what they draw.
    const uint64 frame = _device->FrameNumber();
    Maybe<uint32> oldest {};
    for (uint32 index = 0u; index < _pages.size(); ++index)
    {
      if (_pages[index].LastUsedFrame < frame
          && (!oldest || _pages[index].LastUsedFrame < _pages[*oldest].LastUsedFrame))
      {
        oldest = index;
      }
    }
    if (!oldest)
    {
      return {};
    }

    evicted(*oldest);
    auto &page = _pages[*oldest];
    page.Shelves.clear();
    page.NextShelfY = 0u;
    ++_evictions;
    if (auto texels = AllocateInPage(page, width, height))
    {
      return AtlasSlot {.Page = *oldest, .Texels = *texels};
    }
    return {};
  }

  void AtlasPages::MarkUsed(uint32 page) noexcept
  {
    _pages[page].LastUsedFrame = _device->FrameNumber();
  }

  void AtlasPages::Write(const AtlasSlot &slot, Span<const byte> texels) noexcept
  {
    _device->WriteTexture(_pages[slot.Page].Texture,
                          {.X = static_cast<uint32>(slot.Texels.X),
                           .Y = static_cast<uint32>(slot.Texels.Y),
                           .Width = slot.Texels.Width,
                           .Height = slot.Texels.Height},
                          texels);
  }

  Maybe<PixelRect2D> AtlasPages::AllocateInPage(Page &page, uint32 width, uint32 height) const noexcept
  {
    const uint32 size = _desc.PageSize;

    // The lowest shelf that fits, so tall shelves aren't spent on short rects.
    Shelf *best = nullptr;
    for (auto &shelf : page.Shelves)
    {
      if (shelf.Height >= height && shelf.NextX + width <= size
          && (best == nullptr || shelf.Height < best->Height))
      {
        best = &shelf;
      }
    }

    const uint32 shelfHeight =
      std::min((height + ShelfGranularity - 1u) / ShelfGranularity * ShelfGranularity, size);
    if (best == nullptr || best->Height > shelfHeight * 2u)
    {
      if (page.NextShelfY + shelfHeight <= size)
      {
        page.Shelves.push_back({.Y = page.NextShelfY, .Height = shelfHeight, .NextX = 0u});
        page.NextShelfY += shelfHeight;
        best = &page.Shelves.back();
      }
      else if (best == nullptr)
      {
        return {};
      }
    }

    const PixelRect2D texels {.X = static_cast<int32>(best->NextX),
                              .Y = static_cast<int32>(best->Y),
                              .Width = width,
                              .Height = height};
    best->NextX += width;
    return texels;
  }
}
