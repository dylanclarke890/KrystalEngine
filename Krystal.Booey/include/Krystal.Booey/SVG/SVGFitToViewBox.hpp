/*
 * Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007, 2010 Rob Buis <buis@kde.org>
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

#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"
#include "Krystal.Booey/SVG/SVGPreserveAspectRatio.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys
{
  class AffineTransform;
}

namespace krys::boo
{
  template <typename TOwner, typename... TBaseTypes>
  class SVGPropertyOwnerRegistry;

  /// @brief The `viewBox` and `preserveAspectRatio` of the elements that establish a viewport.
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGFitToViewBox
  class SVGFitToViewBox
  {
    KRYS_NON_COPYABLE(SVGFitToViewBox);

  private:
    Ref<SVGAnimatedRect> _viewBox;
    Ref<SVGAnimatedPreserveAspectRatio> _preserveAspectRatio;
    bool _isViewBoxValid {false};

  public:
    virtual ~SVGFitToViewBox() noexcept = default;

    KRYS_NODISCARD static AffineTransform ViewBoxToViewTransform(const RectF &viewBoxRect,
                                                                 const SVGPreserveAspectRatioValue &,
                                                                 float viewWidth, float viewHeight) noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGFitToViewBox>;

    KRYS_NODISCARD const RectF &ViewBox() const noexcept
    {
      return _viewBox->CurrentValue();
    }

    KRYS_NODISCARD const SVGPreserveAspectRatioValue &PreserveAspectRatio() const noexcept
    {
      return _preserveAspectRatio->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedRect &ViewBoxAnimated() noexcept
    {
      return *_viewBox;
    }

    KRYS_NODISCARD SVGAnimatedPreserveAspectRatio &PreserveAspectRatioAnimated() noexcept
    {
      return *_preserveAspectRatio;
    }

    void SetViewBox(const RectF &viewBox) noexcept;

    void ResetViewBox() noexcept;

    void SetPreserveAspectRatio(const SVGPreserveAspectRatioValue &preserveAspectRatio) noexcept
    {
      _preserveAspectRatio->SetBaseValInternal(preserveAspectRatio);
    }

    void ResetPreserveAspectRatio() noexcept
    {
      _preserveAspectRatio->SetBaseValInternal({});
    }

    KRYS_NODISCARD DOMString ViewBoxString() const noexcept;

    KRYS_NODISCARD DOMString PreserveAspectRatioString() const noexcept
    {
      return PreserveAspectRatio().ValueAsString();
    }

    KRYS_NODISCARD bool HasValidViewBox() const noexcept
    {
      return _isViewBoxValid;
    }

    KRYS_NODISCARD bool HasEmptyViewBox() const noexcept
    {
      return _isViewBoxValid && ViewBox().IsEmpty();
    }

  protected:
    explicit SVGFitToViewBox(SVGElement *contextElement,
                             SVGPropertyAccess access = SVGPropertyAccess::ReadWrite) noexcept;

    KRYS_NODISCARD static bool IsKnownAttribute(const QualifiedName &attributeName) noexcept;

    void Reset() noexcept;

    bool ParseAttribute(const QualifiedName &name, Maybe<DOMStringView> value) noexcept;

    KRYS_NODISCARD Maybe<RectF> ParseViewBox(DOMStringView value) noexcept;

    KRYS_NODISCARD static Maybe<RectF> ParseViewBox(DOMStringView &buffer, bool validate) noexcept;
  };
}
