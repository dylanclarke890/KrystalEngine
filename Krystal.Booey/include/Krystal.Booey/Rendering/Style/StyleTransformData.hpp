/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Graham Dennis (graham.dennis@gmail.com)
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

#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Transform.hpp"
#include "Krystal.Booey/Style/Values/Transforms/TransformOrigin.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleTransformData : public RefCounted<StyleTransformData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleTransformData> Create() noexcept
    {
      return AdoptRef(*new StyleTransformData);
    }
    KRYS_NODISCARD Ref<StyleTransformData> Copy() const noexcept;

    bool operator==(const StyleTransformData &) const noexcept;

    KRYS_NODISCARD bool HasTransform() const noexcept
    {
      return !Transform.IsNone();
    }

    style::Transform Transform;
    style::TransformOrigin Origin;
    unsigned TransformBox : 3;

  private:
    StyleTransformData() noexcept;
    StyleTransformData(const StyleTransformData &) noexcept;
  };
}
