/*
 * Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007, 2010 Rob Buis <buis@kde.org>
 * Copyright (C) 2018 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Svg/SVGFitToViewBox.hpp"
#include "Krystal.Booey/Svg/Elements/SVGElement.hpp"
#include "Krystal.Booey/Svg/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/Svg/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/Svg/SVGNames.hpp"
#include "Krystal.Booey/Svg/SVGRect.hpp"
#include "Krystal.Core/Geometry/AffineTransform.hpp"

namespace krys::boo
{
  SVGFitToViewBox::SVGFitToViewBox(SVGElement *contextElement, SVGPropertyAccess access) noexcept
      : _viewBox(SVGAnimatedRect::Create(contextElement, access)),
        _preserveAspectRatio(SVGAnimatedPreserveAspectRatio::Create(contextElement, access))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFitToViewBox::_viewBox>(SVGNames::ViewBoxAttr());
      PropertyRegistry::RegisterProperty<&SVGFitToViewBox::_preserveAspectRatio>(
        SVGNames::PreserveAspectRatioAttr());
    }
  }

  bool SVGFitToViewBox::IsKnownAttribute(const QualifiedName &attributeName) noexcept
  {
    return PropertyRegistry::IsKnownAttribute(attributeName);
  }

  void SVGFitToViewBox::SetViewBox(const RectF &viewBox) noexcept
  {
    _viewBox->SetBaseValInternal(viewBox);
    _isViewBoxValid = true;
  }

  void SVGFitToViewBox::ResetViewBox() noexcept
  {
    _viewBox->SetBaseValInternal({});
    _isViewBoxValid = false;
  }

  DOMString SVGFitToViewBox::ViewBoxString() const noexcept
  {
    return SVGPropertyTraits<RectF>::ToString(ViewBox());
  }

  void SVGFitToViewBox::Reset() noexcept
  {
    ResetViewBox();
    ResetPreserveAspectRatio();
  }

  bool SVGFitToViewBox::ParseAttribute(const QualifiedName &name, Maybe<DOMStringView> value) noexcept
  {
    if (name == SVGNames::ViewBoxAttr())
    {
      if (value)
      {
        if (auto result = ParseViewBox(*value))
        {
          SetViewBox(*result);
          return true;
        }
      }
      ResetViewBox();
      return true;
    }

    if (name == SVGNames::PreserveAspectRatioAttr())
    {
      SetPreserveAspectRatio(SVGPreserveAspectRatioValue {value.value_or(DOMStringView {})});
      return true;
    }

    return false;
  }

  Maybe<RectF> SVGFitToViewBox::ParseViewBox(DOMStringView value) noexcept
  {
    return ParseViewBox(value, true);
  }

  Maybe<RectF> SVGFitToViewBox::ParseViewBox(DOMStringView &buffer, bool validate) noexcept
  {
    svg::SkipOptionalSVGSpaces(buffer);

    auto x = svg::ParseNumber(buffer);
    auto y = svg::ParseNumber(buffer);
    auto width = svg::ParseNumber(buffer);
    auto height = svg::ParseNumber(buffer, svg::SuffixSkippingPolicy::DontSkip);

    if (validate)
    {
      if (!x || !y || !width || !height)
      {
        return {};
      }

      // Check that width is positive.
      if (*width < 0.0)
      {
        return {};
      }

      // Check that height is positive.
      if (*height < 0.0)
      {
        return {};
      }

      // Nothing should come after the last, fourth number.
      svg::SkipOptionalSVGSpaces(buffer);
      if (!buffer.empty())
      {
        return {};
      }
    }

    return RectF {x.value_or(0), y.value_or(0), width.value_or(0), height.value_or(0)};
  }

  AffineTransform
    SVGFitToViewBox::ViewBoxToViewTransform(const RectF &viewBoxRect,
                                            const SVGPreserveAspectRatioValue &preserveAspectRatio,
                                            float viewWidth, float viewHeight) noexcept
  {
    if (!viewBoxRect.Width() || !viewBoxRect.Height() || !viewWidth || !viewHeight)
    {
      return AffineTransform();
    }

    return preserveAspectRatio.GetCTM(viewBoxRect.X(), viewBoxRect.Y(), viewBoxRect.Width(),
                                      viewBoxRect.Height(), viewWidth, viewHeight);
  }
}
