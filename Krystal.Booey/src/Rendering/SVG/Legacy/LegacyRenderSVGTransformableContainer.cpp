/*
 * Copyright (C) 2004, 2005 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2009-2016 Google, Inc.
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

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGTransformableContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/SVG/Elements/SVGGElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGGraphicsElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGUseElement.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"

namespace krys::boo
{
  LegacyRenderSVGTransformableContainer::LegacyRenderSVGTransformableContainer(
    svg::SVGGraphicsElement &element, RenderStyle &&style) noexcept
      : LegacyRenderSVGContainer(Type::LegacySVGTransformableContainer, element, krys::move(style))
  {
    krys_debug_assert(IsLegacyRenderSVGTransformableContainer());
  }

  LegacyRenderSVGTransformableContainer::~LegacyRenderSVGTransformableContainer() noexcept = default;

  bool LegacyRenderSVGTransformableContainer::CalculateLocalTransform() noexcept
  {
    auto &element = GraphicsElement();

    // If we're either the renderer for a <use> element, or for any <g> element inside the shadow
    // tree, that was created during the use/symbol/svg expansion in SVGUseElement. These containers
    // need to respect the translations induced by their corresponding use elements x/y attributes.
    RefPtr<svg::SVGUseElement> useElement = ShareRefPtr(DynamicDowncast<svg::SVGUseElement>(element));
    if (!useElement && element.IsInShadowTree() && Is<svg::SVGGElement>(element))
    {
      if (auto *correspondingElement = element.CorrespondingElement())
      {
        useElement = ShareRefPtr(DynamicDowncast<svg::SVGUseElement>(*correspondingElement));
      }
    }

    if (useElement)
    {
      svg::SVGLengthContext lengthContext(&element);
      FloatSize translation(useElement->X().Value(lengthContext), useElement->Y().Value(lengthContext));
      if (translation != _additionalTranslation)
      {
        _needsTransformUpdate = true;
      }
      _additionalTranslation = translation;
    }

    auto referenceBoxRect = TransformReferenceBoxRect();
    if (referenceBoxRect != _lastTransformReferenceBoxRect)
    {
      _lastTransformReferenceBoxRect = referenceBoxRect;
      _needsTransformUpdate = true;
    }

    _didTransformToRootUpdate = _needsTransformUpdate || SVGRenderSupport::TransformToRootChanged(Parent());
    if (!_needsTransformUpdate)
    {
      return false;
    }

    _localTransform = element.AnimatedLocalTransform();
    _localTransform.Translate(_additionalTranslation);
    _needsTransformUpdate = false;
    return true;
  }

  svg::SVGGraphicsElement &LegacyRenderSVGTransformableContainer::GraphicsElement() noexcept
  {
    return Downcast<svg::SVGGraphicsElement>(LegacyRenderSVGContainer::GetElement());
  }
}
