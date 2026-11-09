/*
 * Copyright (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2014 Apple Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "Krystal.Booey/Style/Values/Masking/MaskLayer.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"

namespace krys::boo::style
{
  bool MaskLayer::HasOpaqueImage(const RenderElement &renderer) const noexcept
  {
    auto image = _image.TryStyleImage();
    if (!image)
    {
      return false;
    }

    if (GetComposite() == CompositeOperator::Clear || GetComposite() == CompositeOperator::Copy)
    {
      return true;
    }

    return GetBlendMode() == BlendMode::Normal && GetComposite() == CompositeOperator::SourceOver
           && image->KnownToBeOpaque(renderer);
  }

  MaskLayer::MaskLayer() noexcept
      : _image(MaskLayer::InitialImage()), _positionX(MaskLayer::InitialPositionX()),
        _positionY(MaskLayer::InitialPositionY()), _size(MaskLayer::InitialSize()),
        _repeat(MaskLayer::InitialRepeat()), _clip(static_cast<uint8>(MaskLayer::InitialClip())),
        _origin(static_cast<uint8>(MaskLayer::InitialOrigin())),
        _composite(static_cast<uint8>(MaskLayer::InitialComposite())),
        _maskMode(static_cast<uint8>(MaskLayer::InitialMaskMode())),
        _clipMax(static_cast<uint8>(MaskLayer::InitialClip()))
  {
  }

  MaskLayer::MaskLayer(ImageOrNone &&image) noexcept : MaskLayer {}
  {
    SetImage(krys::move(image));
  }

  MaskLayer::MaskLayer(css::keywords::None keyword) noexcept : MaskLayer {ImageOrNone {keyword}}
  {
  }

  MaskLayer::MaskLayer(RefPtr<StyleImage> &&image) noexcept : MaskLayer {ImageOrNone {krys::move(image)}}
  {
  }

  bool MaskLayer::operator==(const MaskLayer &other) const noexcept
  {
    // NOTE: Default operator== is not used due to exclusion of _clipMax.
    return AllOfCoordinatedValueListProperties<MaskLayer>(
      [this, &other]<auto propertyID>()
      {
        using PropertyAccessor = CoordinatedValueListPropertyConstAccessor<propertyID>;
        return PropertyAccessor {*this} == PropertyAccessor {other};
      });
  }

  // MARK: - Blending

  bool Blending<MaskLayer>::CanBlend(const MaskLayer &a, const MaskLayer &b) noexcept
  {
    return a.GetSize().HasSameType(b.GetSize());
  }
}
