/*
 Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
 */

#include "Krystal.UI/Compositing/TextureMapperTiledBackingStore.hpp"
#include "Krystal.Booey/Graphics/Images/Image.hpp"
#include "Krystal.Booey/Graphics/Images/ImageObserver.hpp"
#include "Krystal.Booey/Graphics/Layers/GraphicsLayer.hpp"
#include "Krystal.UI/Compositing/TextureMapper.hpp"
#include <algorithm>

namespace krys::ui
{
  void TextureMapperTiledBackingStore::SetContentsToImage(boo::Image *image) noexcept
  {
    _image = ShareRefPtr(image);
  }

  void TextureMapperTiledBackingStore::UpdateContentsFromImageIfNeeded(TextureMapper &textureMapper) noexcept
  {
    if (!_image)
    {
      return;
    }

    UpdateContents(textureMapper, _image.get(), _image->Size(), EnclosingRectI(_image->Rect()));

    if (auto observer = _image->GetImageObserver())
    {
      observer->DidDraw(*_image);
    }
    _image = nullptr;
  }

  TransformationMatrix
    TextureMapperTiledBackingStore::AdjustedTransformForRect(const RectF &targetRect) noexcept
  {
    return TransformationMatrix::RectToRect(Rect(), targetRect);
  }

  void TextureMapperTiledBackingStore::PaintToTextureMapper(TextureMapper &textureMapper,
                                                            const RectF &targetRect,
                                                            const TransformationMatrix &transform,
                                                            float opacity) noexcept
  {
    UpdateContentsFromImageIfNeeded(textureMapper);
    TransformationMatrix adjustedTransform = transform * AdjustedTransformForRect(targetRect);
    for (auto &tile : _tiles)
    {
      tile.Paint(textureMapper, adjustedTransform, opacity, AllTileEdgesExposed(Rect(), tile.Rect()));
    }
  }

  void TextureMapperTiledBackingStore::DrawBorder(TextureMapper &textureMapper, const boo::Color &borderColor,
                                                  float borderWidth, const RectF &targetRect,
                                                  const TransformationMatrix &transform) noexcept
  {
    TransformationMatrix adjustedTransform = transform * AdjustedTransformForRect(targetRect);
    for (auto &tile : _tiles)
    {
      textureMapper.DrawBorder(borderColor, borderWidth, tile.Rect(), adjustedTransform);
    }
  }

  void TextureMapperTiledBackingStore::DrawRepaintCounter(TextureMapper &textureMapper, int32 repaintCount,
                                                          const boo::Color &borderColor,
                                                          const RectF &targetRect,
                                                          const TransformationMatrix &transform) noexcept
  {
    TransformationMatrix adjustedTransform = transform * AdjustedTransformForRect(targetRect);
    for (auto &tile : _tiles)
    {
      textureMapper.DrawNumber(repaintCount, borderColor, tile.Rect().Location(), adjustedTransform);
    }
  }

  void TextureMapperTiledBackingStore::UpdateContentsScale(float scale) noexcept
  {
    if (_contentsScale == scale)
    {
      return;
    }

    _isScaleDirty = true;
    _contentsScale = scale;
  }

  void TextureMapperTiledBackingStore::CreateOrDestroyTilesIfNeeded(const SizeF &size, const SizeI &tileSize,
                                                                    bool hasAlpha) noexcept
  {
    if (size == _size && !_isScaleDirty)
    {
      return;
    }

    _size = size;
    _isScaleDirty = false;

    SizeF scaledSize(_size);
    if (!_image)
    {
      scaledSize.Scale(_contentsScale);
    }

    List<RectF> tileRectsToAdd;
    List<int32> tileIndicesToRemove;
    static const size_t TileEraseThreshold = 6;

    // This method recycles tiles. We check which tiles we need to add, which to remove, and use as many
    // removable tiles as replacement for new tiles when possible.
    for (float y = 0; y < scaledSize.Height(); y += static_cast<float>(tileSize.Height()))
    {
      for (float x = 0; x < scaledSize.Width(); x += static_cast<float>(tileSize.Width()))
      {
        RectF tileRect(x, y, static_cast<float>(tileSize.Width()), static_cast<float>(tileSize.Height()));
        tileRect.Intersect(Rect());
        tileRectsToAdd.push_back(tileRect);
      }
    }

    // Check which tiles need to be removed, and which already exist.
    for (int32 i = static_cast<int32>(_tiles.size()) - 1; i >= 0; --i)
    {
      RectF oldTile = _tiles[static_cast<size_t>(i)].Rect();
      bool existsAlready = false;

      for (int32 j = static_cast<int32>(tileRectsToAdd.size()) - 1; j >= 0; --j)
      {
        RectF newTile = tileRectsToAdd[static_cast<size_t>(j)];
        if (oldTile != newTile)
        {
          continue;
        }

        // A tile that we want to add already exists, no need to add or remove it.
        existsAlready = true;
        tileRectsToAdd.erase(tileRectsToAdd.begin() + j);
        break;
      }

      // This tile is not needed.
      if (!existsAlready)
      {
        tileIndicesToRemove.push_back(i);
      }
    }

    // Recycle removable tiles to be used for newly requested tiles.
    for (auto &rect : tileRectsToAdd)
    {
      if (!tileIndicesToRemove.empty())
      {
        // We recycle an existing tile for usage with a new tile rect.
        TextureMapperTile &tile = _tiles[static_cast<size_t>(tileIndicesToRemove.back())];
        tileIndicesToRemove.pop_back();
        tile.SetRect(rect);

        if (auto texture = tile.Texture())
        {
          EnumSet<BitmapTexture::Flags> flags;
          if (hasAlpha)
          {
            flags.Add(BitmapTexture::Flags::SupportsAlpha);
          }
          texture->Reset(EnclosingRectI(tile.Rect()).Size(), flags);
        }
        continue;
      }

      _tiles.emplace_back(rect);
    }

    // Remove unnecessary tiles, if they weren't recycled. We use a threshold to make sure we don't
    // create/destroy tiles too eagerly.
    for (auto &index : tileIndicesToRemove)
    {
      if (_tiles.size() <= TileEraseThreshold)
      {
        break;
      }
      _tiles.erase(_tiles.begin() + index);
    }
  }

  void TextureMapperTiledBackingStore::UpdateContents(TextureMapper &textureMapper, boo::Image *image,
                                                      const SizeF &totalSize, const RectI &dirtyRect) noexcept
  {
    CreateOrDestroyTilesIfNeeded(totalSize, textureMapper.MaxTextureSize(),
                                 !image->CurrentFrameKnownToBeOpaque());
    for (auto &tile : _tiles)
    {
      tile.UpdateContents(textureMapper, *image, dirtyRect);
    }
  }

  void TextureMapperTiledBackingStore::UpdateContents(TextureMapper &textureMapper,
                                                      const boo::GraphicsLayer &sourceLayer,
                                                      const SizeF &totalSize, const RectI &dirtyRect,
                                                      const RectI &visibleRect,
                                                      const SizeI &viewportSize) noexcept
  {
    CreateOrDestroyCoveringTiles(totalSize, visibleRect, viewportSize,
                                 textureMapper.MaxTextureSize().Width());

    for (auto &tile : _tiles)
    {
      auto tileRect = EnclosingRectI(tile.Rect());
      if (tileRect.Intersects(dirtyRect))
      {
        tile.AddDirtyRect(Intersection(tileRect, dirtyRect));
      }
    }

    for (auto &tile : _tiles)
    {
      if (!tile.DirtyRect().IsEmpty())
      {
        tile.UpdateContents(textureMapper, sourceLayer, tile.DirtyRect(), _contentsScale);
      }
    }
  }

  void TextureMapperTiledBackingStore::CreateOrDestroyCoveringTiles(const SizeF &size,
                                                                    const RectI &unscaledVisibleRect,
                                                                    const SizeI &unscaledViewportSize,
                                                                    int32 maxTextureSize) noexcept
  {
    bool contentsScaleChanged = _isScaleDirty;
    _isScaleDirty = false;
    _size = size;

    auto scaled = [&](const RectI &rect)
    {
      RectF scaledRect(rect);
      scaledRect.Scale(_contentsScale);
      return EnclosingRectI(scaledRect);
    };

    auto contentsRect = EnclosingRectI(Rect());
    _visibleRect = scaled(unscaledVisibleRect);
    if (contentsRect.IsEmpty())
    {
      _tiles.clear();
      _coverRect = {};
      _keepRect = {};
      return;
    }

    auto viewportSize = unscaledViewportSize.IsEmpty() ? _visibleRect.Size() : unscaledViewportSize;
    SizeF scaledViewportSize(viewportSize);
    scaledViewportSize.Scale(_contentsScale);
    auto tileSize = ComputeTileSize(SizeI(scaledViewportSize), maxTextureSize, contentsRect);
    if (tileSize != _tileSize || contentsScaleChanged)
    {
      _tiles.clear();
      _tileSize = tileSize;
    }

    // Cover and keep rects use the visible rect rather than its intersection with the contents: the visible
    // rect needn't intersect them, and tiles in the cover rect around it may.
    auto [coverRect, keepRect] = ComputeCoverAndKeepRect(contentsRect);
    _coverRect = coverRect;
    _keepRect = keepRect;

    // Drop tiles outside the new keepRect, and resize tiles at the edge in case the contents size has
    // changed.
    std::erase_if(_tiles,
                  [&](TextureMapperTile &tile)
                  {
                    auto rect = EnclosingRectI(tile.Rect());
                    if (!rect.Intersects(_keepRect))
                    {
                      return true;
                    }

                    auto expectedTileRect =
                      TileRectForPosition(TilePositionForPoint(rect.Location()), contentsRect);
                    if (expectedTileRect.IsEmpty())
                    {
                      return true;
                    }

                    if (expectedTileRect != rect)
                    {
                      tile.SetRect(RectF(expectedTileRect));
                      if (auto texture = tile.Texture())
                      {
                        texture->Reset(expectedTileRect.Size(), {BitmapTexture::Flags::SupportsAlpha});
                      }
                      tile.AddDirtyRect(expectedTileRect);
                    }
                    return false;
                  });

    if (_coverRect.IsEmpty())
    {
      return;
    }

    // Cover the cover rect: a tile wherever there isn't one, painted whole.
    auto topLeft = TilePositionForPoint(_coverRect.MinXMinYCorner());
    auto innerBottomRight = TilePositionForPoint(_coverRect.MaxXMaxYCorner() - SizeI(1, 1));
    for (int32 y = topLeft.Y(); y <= innerBottomRight.Y(); ++y)
    {
      for (int32 x = topLeft.X(); x <= innerBottomRight.X(); ++x)
      {
        auto rect = TileRectForPosition({x, y}, contentsRect);
        if (rect.IsEmpty())
        {
          continue;
        }
        bool exists = std::any_of(_tiles.begin(), _tiles.end(), [&](const TextureMapperTile &tile)
                                  { return EnclosingRectI(tile.Rect()) == rect; });
        if (exists)
        {
          continue;
        }
        TextureMapperTile tile(RectF {rect});
        tile.AddDirtyRect(rect);
        _tiles.push_back(krys::move(tile));
      }
    }
  }

  SizeI TextureMapperTiledBackingStore::ComputeTileSize(const SizeI &viewportSize, int32 maxTextureSize,
                                                        const RectI &contentsRect) noexcept
  {
    // Tiles are painted by the GPU: tiles as wide as the viewport (or half of it), and a quarter of its
    // height.
    constexpr static int32 minGPUTileHeight = 256;

    auto gpuTileSize = [&](const SizeI &baseSize) -> SizeI
    {
      int32 width = baseSize.Width();
      int32 divisor = 4;
      if (contentsRect.Width() <= baseSize.Width() / 2)
      {
        divisor = 2;
      }
      if (contentsRect.Width() <= baseSize.Width() / 4)
      {
        divisor = 1;
      }
      int32 height = ((baseSize.Height() + divisor - 1) / divisor * divisor) / divisor;
      return {std::max(width, 1), std::max(height, minGPUTileHeight)};
    };

    SizeI baseSize = viewportSize;
    SizeI tileSize = gpuTileSize(baseSize);

    // Use half-width tiles when the contents width is greater than computed tile size.
    if (contentsRect.Width() > tileSize.Width())
    {
      baseSize.SetWidth((baseSize.Width() + 1) / 2);
      tileSize = gpuTileSize(baseSize);
    }

    tileSize.ClampToMaximumSize(contentsRect.Size());
    if (maxTextureSize)
    {
      tileSize.ClampToMaximumSize({maxTextureSize, maxTextureSize});
    }

    return tileSize;
  }

  Pair<RectI, RectI>
    TextureMapperTiledBackingStore::ComputeCoverAndKeepRect(const RectI &contentsRect) const noexcept
  {
    constexpr static float coverAreaMultiplier = 2.0f;

    RectI coverRect = _visibleRect;
    RectI keepRect = _visibleRect;

    // The initial cover area covers equally in each direction, according to the coverAreaMultiplier.
    coverRect.InflateX(
      static_cast<int32>(static_cast<float>(_visibleRect.Width()) * (coverAreaMultiplier - 1) / 2));
    coverRect.InflateY(
      static_cast<int32>(static_cast<float>(_visibleRect.Height()) * (coverAreaMultiplier - 1) / 2));
    keepRect = coverRect;

    // Try to create a cover rect of the same size as the candidate, but within content bounds.
    auto candidateSize = coverRect.Size();
    coverRect.Intersect(contentsRect);
    if (coverRect.Size() != candidateSize && !coverRect.IsEmpty())
    {
      int64 pixelsCovered = static_cast<int64>(candidateSize.Width()) * candidateSize.Height();
      if (coverRect.Width() < candidateSize.Width())
      {
        coverRect.InflateY(static_cast<int32>((pixelsCovered / coverRect.Width() - coverRect.Height()) / 2));
      }
      if (coverRect.Height() < candidateSize.Height())
      {
        coverRect.InflateX(static_cast<int32>((pixelsCovered / coverRect.Height() - coverRect.Width()) / 2));
      }
      coverRect.Intersect(contentsRect);
    }

    // The keep rect is an inflated version of the cover rect, inflated in tile dimensions.
    keepRect.Unite(coverRect);
    keepRect.InflateX(_tileSize.Width() / 2);
    keepRect.InflateY(_tileSize.Height() / 2);
    keepRect.Intersect(contentsRect);

    return {coverRect, keepRect};
  }

  RectI TextureMapperTiledBackingStore::TileRectForPosition(const PointI &position,
                                                            const RectI &contentsRect) const noexcept
  {
    RectI rect(position.X() * _tileSize.Width(), position.Y() * _tileSize.Height(), _tileSize.Width(),
               _tileSize.Height());
    rect.Intersect(contentsRect);
    return rect;
  }

  PointI TextureMapperTiledBackingStore::TilePositionForPoint(const PointI &point) const noexcept
  {
    int32 x = point.X() / _tileSize.Width();
    int32 y = point.Y() / _tileSize.Height();
    return {std::max(x, 0), std::max(y, 0)};
  }
}
