/*
 * Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2018-2024 Apple Inc. All rights reserved.
 * Copyright (C) 2018 Adobe Systems Incorporated. All rights reserved.
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

#include "Krystal.Booey/SVG/Elements/SVGGeometryElement.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGShape.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGPoint.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <charconv>

namespace krys::boo::svg
{
  namespace
  {
    constexpr EnumSet<dom::LayoutOptions> GeometryQueryLayoutOptions {
      dom::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
      dom::LayoutOptions::TreatContentVisibilityAutoAsVisible};

    // The number at the start of the string, after any whitespace, or 0 if there isn't one.
    KRYS_NODISCARD float ToFloat(dom::DOMStringView string) noexcept
    {
      while (!string.empty() && text::IsASCIIWhitespace(string.front()))
      {
        string.remove_prefix(1);
      }
      if (!string.empty() && string.front() == u8'+')
      {
        string.remove_prefix(1);
      }

      float value = 0;
      auto first = reinterpret_cast<const char *>(string.data());
      auto result = std::from_chars(first, first + string.size(), value);
      return result.ec == std::errc {} ? value : 0;
    }
  }

  SVGGeometryElement::SVGGeometryElement(dom::Document &document,
                                         UniquePtr<SVGPropertyRegistry> &&propertyRegistry,
                                         SVGElementInterface interface) noexcept
      : SVGGraphicsElement(document, krys::move(propertyRegistry), interface,
                           SVGElementFlags::IsSvgGeometryElement),
        _pathLength(SVGAnimatedNumber::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGGeometryElement::_pathLength>(SVGNames::PathLengthAttr());
    }
  }

  float SVGGeometryElement::GetTotalLength() const noexcept
  {
    (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets(GeometryQueryLayoutOptions, this);

    auto *renderer = Renderer();
    if (!renderer)
    {
      return 0;
    }

    if (auto *renderSVGShape = DynamicDowncast<LegacyRenderSVGShape>(renderer))
    {
      return renderSVGShape->GetTotalLength();
    }

    krys_debug_assert(false);
    return 0;
  }

  dom::ExceptionOr<Ref<SVGPoint>> SVGGeometryElement::GetPointAtLength(float distance) const noexcept
  {
    (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets(GeometryQueryLayoutOptions, this);

    auto *renderer = Renderer();
    // Spec: If current element is a non-rendered element, throw an InvalidStateError.
    if (!renderer)
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    // Spec: Clamp distance to [0, length].
    distance = std::clamp<float>(distance, 0, GetTotalLength());

    // Spec: Return a newly created, detached SVGPoint object.
    if (auto *renderSVGShape = DynamicDowncast<LegacyRenderSVGShape>(renderer))
    {
      return SVGPoint::Create(renderSVGShape->GetPointAtLength(distance));
    }

    krys_debug_assert(false);
    return dom::Exception {dom::ExceptionCode::InvalidStateError};
  }

  bool SVGGeometryElement::IsPointInFill(dom::DOMPointInit &&pointInit) noexcept
  {
    (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets(GeometryQueryLayoutOptions, this);

    auto *renderer = Renderer();
    if (!renderer)
    {
      return false;
    }

    FloatPoint point {static_cast<float>(pointInit.X), static_cast<float>(pointInit.Y)};
    if (auto *renderSVGShape = DynamicDowncast<LegacyRenderSVGShape>(renderer))
    {
      return renderSVGShape->IsPointInFill(point);
    }

    krys_debug_assert(false);
    return false;
  }

  bool SVGGeometryElement::IsPointInStroke(dom::DOMPointInit &&pointInit) noexcept
  {
    (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets(GeometryQueryLayoutOptions, this);

    auto *renderer = Renderer();
    if (!renderer)
    {
      return false;
    }

    FloatPoint point {static_cast<float>(pointInit.X), static_cast<float>(pointInit.Y)};
    if (auto *renderSVGShape = DynamicDowncast<LegacyRenderSVGShape>(renderer))
    {
      return renderSVGShape->IsPointInStroke(point);
    }

    krys_debug_assert(false);
    return false;
  }

  void SVGGeometryElement::AttributeChanged(const dom::QualifiedName &name,
                                            Maybe<dom::DOMStringView> oldValue,
                                            Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == SVGNames::PathLengthAttr())
    {
      _pathLength->SetBaseValInternal(ToFloat(value.value_or(dom::DOMStringView {})));
    }

    SVGGraphicsElement::AttributeChanged(name, oldValue, value);
  }

  void SVGGeometryElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      krys_debug_assert(attributeName == SVGNames::PathLengthAttr());
      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }

    SVGGraphicsElement::SVGAttributeChanged(attributeName);
  }
}
