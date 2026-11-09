/*
 * Copyright (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Style/StyleTransformData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  StyleTransformData::StyleTransformData() noexcept
      : Transform(style::ComputedStyle::InitialTransform()),
        Origin({style::ComputedStyle::InitialTransformOriginX(),
                style::ComputedStyle::InitialTransformOriginY(),
                style::ComputedStyle::InitialTransformOriginZ()}),
        TransformBox(static_cast<unsigned>(style::ComputedStyle::InitialTransformBox()))
  {
  }

  inline StyleTransformData::StyleTransformData(const StyleTransformData &other) noexcept
      : RefCounted<StyleTransformData>(), Transform(other.Transform), Origin(other.Origin),
        TransformBox(other.TransformBox)
  {
  }

  Ref<StyleTransformData> StyleTransformData::Copy() const noexcept
  {
    return AdoptRef(*new StyleTransformData(*this));
  }

  bool StyleTransformData::operator==(const StyleTransformData &other) const noexcept
  {
    return Origin == other.Origin && TransformBox == other.TransformBox && Transform == other.Transform;
  }
}
