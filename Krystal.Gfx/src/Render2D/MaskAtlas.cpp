#include "Krystal.Gfx/Render2D/MaskAtlas.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Gfx/Render2D/CoverageBlur.hpp"
#include <algorithm>
#include <bit>
#include <cmath>

namespace krys::gfx
{
  namespace
  {
    /// @brief Shelf heights are rounded up to this, so masks of similar heights share shelves.
    constexpr uint32 ShelfGranularity = 8u;

    /// @brief Bounds of every point of the path once transformed. Curves stay within their control points,
    /// so this contains the path; it's cheap enough to check a mask's size before rasterising it.
    KRYS_NODISCARD Maybe<Rect2D> ControlBounds(const Path2D &path, const Transform2D &transform) noexcept
    {
      const auto points = path.Points();
      if (points.empty())
      {
        return {};
      }
      const auto first = transform.Apply(points[0]);
      float left = first.x;
      float top = first.y;
      float right = first.x;
      float bottom = first.y;
      for (const auto &point : points)
      {
        const auto mapped = transform.Apply(point);
        left = std::min(left, mapped.x);
        top = std::min(top, mapped.y);
        right = std::max(right, mapped.x);
        bottom = std::max(bottom, mapped.y);
      }
      if (!std::isfinite(left) || !std::isfinite(top) || !std::isfinite(right) || !std::isfinite(bottom))
      {
        return {};
      }
      return Rect2D {left, top, right - left, bottom - top};
    }

    KRYS_NODISCARD PixelRect2D Offset(PixelRect2D rect, int32 x, int32 y) noexcept
    {
      rect.X += x;
      rect.Y += y;
      return rect;
    }
  }

  size_t MaskAtlas::KeyHash::operator()(const Key &key) const noexcept
  {
    size_t seed = static_cast<size_t>(key.Rule);
    auto mix = [&](uint32 value)
    {
      seed ^= value + 0x9e'37'79'b9u + (seed << 6u) + (seed >> 2u);
    };
    for (const auto verb : key.Path.Verbs())
    {
      mix(static_cast<uint32>(verb));
    }
    for (const auto &point : key.Path.Points())
    {
      mix(std::bit_cast<uint32>(point.x));
      mix(std::bit_cast<uint32>(point.y));
    }
    for (const auto value : key.Linear)
    {
      mix(std::bit_cast<uint32>(value));
    }
    for (const auto value : key.Fraction)
    {
      mix(std::bit_cast<uint32>(value));
    }
    mix(std::bit_cast<uint32>(key.Blur));
    return seed;
  }

  MaskAtlas::MaskAtlas(Device &device, const MaskAtlasDesc &desc) noexcept : _device(&device), _desc(desc)
  {
  }

  MaskAtlas::~MaskAtlas() noexcept
  {
    for (const auto &[key, entry] : _entries)
    {
      if (entry.OwnTexture.IsValid())
      {
        _device->Destroy(entry.OwnTexture);
      }
    }
    for (const auto &retired : _retired)
    {
      _device->Destroy(retired.Texture);
    }
    for (const auto &page : _pages)
    {
      _device->Destroy(page.Texture);
    }
  }

  MaskLocation MaskAtlas::GetMask(const Path2D &path, const Transform2D &transform, FillRule rule,
                                  const PixelRect2D &clip, float blur) noexcept
  {
    Collect();
    blur = blur > 0.f && std::isfinite(blur) ? blur : 0.f;
    const uint64 frame = _device->FrameNumber();

    // The whole-pixel part of the translation moves the mask; the rest shapes it.
    const float wholeX = std::floor(transform.E);
    const float wholeY = std::floor(transform.F);
    if (!std::isfinite(wholeX) || !std::isfinite(wholeY))
    {
      return {};
    }
    const auto offsetX = static_cast<int32>(wholeX);
    const auto offsetY = static_cast<int32>(wholeY);
    const Transform2D shape {.A = transform.A,
                             .B = transform.B,
                             .C = transform.C,
                             .D = transform.D,
                             .E = transform.E - wholeX,
                             .F = transform.F - wholeY};

    Key key {.Path = path,
             .Linear = {shape.A, shape.B, shape.C, shape.D},
             .Fraction = {shape.E, shape.F},
             .Rule = rule,
             .Blur = blur};
    if (auto found = _entries.find(key); found != _entries.end())
    {
      auto &entry = found->second;
      entry.LastUsedFrame = frame;
      if (entry.Page != NoPage)
      {
        _pages[entry.Page].LastUsedFrame = frame;
      }
      ++_stats.Hits;
      const auto texture = entry.Page != NoPage ? _pages[entry.Page].Texture : entry.OwnTexture;
      return {.Texture = texture, .Texels = entry.Texels, .Bounds = Offset(entry.Bounds, offsetX, offsetY)};
    }

    const auto bounds = ControlBounds(path, shape);
    if (!bounds)
    {
      return {};
    }

    // Masks too large for a texture are rasterised within the clip (and as far around it as a blur reaches
    // into it) and kept for this frame only.
    const auto extent = detail::BlurExtent(blur);
    const auto largest = static_cast<float>(std::min(_device->Limits().MaxTextureSize, 4'096u));
    const float spread = 2.f + 2.f * static_cast<float>(extent);
    if (bounds->Width + spread > largest || bounds->Height + spread > largest)
    {
      auto localClip =
        Offset(clip, -offsetX - static_cast<int32>(extent), -offsetY - static_cast<int32>(extent));
      localClip.Width += 2u * extent;
      localClip.Height += 2u * extent;
      auto mask = _rasteriser.Rasterise(path, shape, rule, localClip);
      ++_stats.Rasterised;
      if (extent > 0u)
      {
        mask = detail::BlurCoverage(mask, blur);
      }
      mask.Bounds = Offset(mask.Bounds, offsetX, offsetY);
      return UploadTransient(mask);
    }

    constexpr PixelRect2D everywhere {
      .X = -(1 << 24), .Y = -(1 << 24), .Width = 1u << 25, .Height = 1u << 25};
    auto mask = _rasteriser.Rasterise(path, shape, rule, everywhere);
    ++_stats.Rasterised;
    if (extent > 0u)
    {
      mask = detail::BlurCoverage(mask, blur);
    }

    Entry entry {.Bounds = mask.Bounds, .LastUsedFrame = frame};
    if (!mask.Bounds.IsEmpty())
    {
      const uint32 width = mask.Bounds.Width;
      const uint32 height = mask.Bounds.Height;
      Maybe<Entry> packed {};
      if (width <= _desc.MaxPackedSize && height <= _desc.MaxPackedSize)
      {
        packed = Allocate(width, height);
      }

      if (packed)
      {
        entry.Page = packed->Page;
        entry.Texels = packed->Texels;
        _pages[entry.Page].LastUsedFrame = frame;
        Upload(_pages[entry.Page].Texture, entry.Texels, mask);
      }
      else
      {
        entry.OwnTexture = CreateMaskTexture(width, height, u8"Render2D.Mask");
        if (!entry.OwnTexture.IsValid())
        {
          return {};
        }
        entry.Texels = {.X = 0, .Y = 0, .Width = width, .Height = height};
        Upload(entry.OwnTexture, entry.Texels, mask);
      }
    }

    const auto texture = entry.Page != NoPage ? _pages[entry.Page].Texture : entry.OwnTexture;
    const MaskLocation location {
      .Texture = texture, .Texels = entry.Texels, .Bounds = Offset(entry.Bounds, offsetX, offsetY)};
    _entries.emplace(krys::move(key), entry);
    return location;
  }

  MaskLocation MaskAtlas::UploadTransient(const CoverageMask &mask) noexcept
  {
    Collect();
    if (mask.Bounds.IsEmpty())
    {
      return {};
    }
    const auto texture =
      CreateMaskTexture(mask.Bounds.Width, mask.Bounds.Height, u8"Render2D.Mask.Transient");
    if (!texture.IsValid())
    {
      return {};
    }
    const PixelRect2D texels {.X = 0, .Y = 0, .Width = mask.Bounds.Width, .Height = mask.Bounds.Height};
    Upload(texture, texels, mask);
    _retired.push_back({.Texture = texture, .Frame = _device->FrameNumber()});
    return {.Texture = texture, .Texels = texels, .Bounds = mask.Bounds};
  }

  void MaskAtlas::Collect() noexcept
  {
    const uint64 frame = _device->FrameNumber();
    if (frame == _collectedFrame)
    {
      return;
    }
    _collectedFrame = frame;

    // Earlier frames' commands are submitted by now, so textures they last drew from can go.
    std::erase_if(_retired,
                  [&](const RetiredTexture &retired)
                  {
                    if (retired.Frame < frame)
                    {
                      _device->Destroy(retired.Texture);
                      return true;
                    }
                    return false;
                  });

    // Masks with textures of their own (and masks that cover nothing) go once a whole frame passes without
    // them.
    std::erase_if(_entries,
                  [&](const auto &item)
                  {
                    const auto &entry = item.second;
                    if (entry.Page != NoPage || entry.LastUsedFrame + 1u >= frame)
                    {
                      return false;
                    }
                    if (entry.OwnTexture.IsValid())
                    {
                      _device->Destroy(entry.OwnTexture);
                    }
                    return true;
                  });
  }

  Maybe<MaskAtlas::Entry> MaskAtlas::Allocate(uint32 width, uint32 height) noexcept
  {
    for (uint32 index = 0u; index < _pages.size(); ++index)
    {
      if (auto texels = AllocateInPage(_pages[index], width, height))
      {
        return Entry {.Page = index, .Texels = *texels};
      }
    }

    if (_pages.size() < _desc.MaxPages)
    {
      const auto texture = CreateMaskTexture(_desc.PageSize, _desc.PageSize, u8"Render2D.MaskPage");
      if (texture.IsValid())
      {
        _pages.push_back({.Texture = texture});
        if (auto texels = AllocateInPage(_pages.back(), width, height))
        {
          return Entry {.Page = static_cast<uint32>(_pages.size() - 1u), .Texels = *texels};
        }
      }
    }

    // Clear the least recently used page, unless the current frame has drawn from every page: overwriting
    // texels its unsubmitted draws read would change what they draw.
    const uint64 frame = _device->FrameNumber();
    uint32 oldest = NoPage;
    for (uint32 index = 0u; index < _pages.size(); ++index)
    {
      if (_pages[index].LastUsedFrame < frame
          && (oldest == NoPage || _pages[index].LastUsedFrame < _pages[oldest].LastUsedFrame))
      {
        oldest = index;
      }
    }
    if (oldest == NoPage)
    {
      return {};
    }

    EvictPage(oldest);
    if (auto texels = AllocateInPage(_pages[oldest], width, height))
    {
      return Entry {.Page = oldest, .Texels = *texels};
    }
    return {};
  }

  Maybe<PixelRect2D> MaskAtlas::AllocateInPage(Page &page, uint32 width, uint32 height) const noexcept
  {
    const uint32 size = _desc.PageSize;
    if (width > size || height > size)
    {
      return {};
    }

    // The lowest shelf that fits, so tall shelves aren't spent on short masks.
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

  void MaskAtlas::EvictPage(uint32 index) noexcept
  {
    std::erase_if(_entries, [&](const auto &item) { return item.second.Page == index; });
    auto &page = _pages[index];
    page.Shelves.clear();
    page.NextShelfY = 0u;
    ++_stats.PageEvictions;
  }

  TextureHandle MaskAtlas::CreateMaskTexture(uint32 width, uint32 height, utf8_stringview name) noexcept
  {
    auto texture = _device->CreateTexture(
      {.Format = TextureFormat::R8Unorm, .Width = width, .Height = height, .DebugName = name});
    krys_debug_assert_msg(texture.has_value(), "Couldn't create a mask texture.");
    return texture ? *texture : TextureHandle {};
  }

  void MaskAtlas::Upload(TextureHandle texture, const PixelRect2D &texels, const CoverageMask &mask) noexcept
  {
    _device->WriteTexture(
      texture,
      {.X = static_cast<uint32>(texels.X),
       .Y = static_cast<uint32>(texels.Y),
       .Width = texels.Width,
       .Height = texels.Height},
      Span<const byte>(reinterpret_cast<const byte *>(mask.Coverage.data()), mask.Coverage.size()));
  }
}
