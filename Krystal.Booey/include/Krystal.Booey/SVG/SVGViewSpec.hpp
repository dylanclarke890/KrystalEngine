/*
 * Copyright (C) 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2018-2019 Apple Inc. All rights reserved.
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
 */

#pragma once

#include "Krystal.Booey/SVG/SVGFitToViewBox.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::svg
{
  class SVGElement;
  class SVGTransformList;

  /// @brief The view an `svgView(...)` fragment identifier gives an `<svg>`: its view box, aspect ratio,
  /// transform and target.
  /// @see https://svgwg.org/svg2-draft/linking.html#SVGFragmentIdentifiers
  class SVGViewSpec final : public RefCounted<SVGViewSpec>, public SVGFitToViewBox
  {
  private:
    WeakPtr<SVGElement> _contextElement;
    dom::DOMString _viewTargetString;
    Ref<SVGTransformList> _transform;

  public:
    KRYS_NODISCARD static Ref<SVGViewSpec> Create(SVGElement &contextElement) noexcept
    {
      return AdoptRef(*new SVGViewSpec(contextElement));
    }

    bool ParseViewSpec(dom::DOMStringView) noexcept;
    void Reset() noexcept;
    void ResetContextElement() noexcept
    {
      _contextElement = nullptr;
    }

    KRYS_NODISCARD RefPtr<SVGElement> ViewTarget() const noexcept;
    KRYS_NODISCARD const dom::DOMString &ViewTargetString() const noexcept
    {
      return _viewTargetString;
    }

    KRYS_NODISCARD dom::DOMString TransformString() const noexcept;
    KRYS_NODISCARD Ref<SVGTransformList> &Transform() noexcept
    {
      return _transform;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGViewSpec, SVGFitToViewBox>;

  private:
    explicit SVGViewSpec(SVGElement &) noexcept;
  };
}
