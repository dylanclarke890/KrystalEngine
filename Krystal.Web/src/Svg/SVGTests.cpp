/*
 * Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2015-2024 Apple Inc. All right reserved.
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

#include "Krystal.Web/Svg/SVGTests.hpp"
#include "Krystal.I18n/Locale.hpp"
#include "Krystal.Web/Infra/Namespaces.hpp"
#include "Krystal.Web/Svg/Elements/SVGElement.hpp"
#include "Krystal.Web/Svg/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Web/Svg/SVGNames.hpp"
#include "Krystal.Web/Svg/SVGStringList.hpp"

namespace krys::web
{
  SVGConditionalProcessingAttributes::SVGConditionalProcessingAttributes(SVGElement &contextElement) noexcept
      : _requiredExtensions(SVGStringList::Create(&contextElement)),
        _systemLanguage(SVGStringList::Create(&contextElement))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      SVGTests::PropertyRegistry::RegisterConditionalProcessingAttributeProperty<
        &SVGConditionalProcessingAttributes::_requiredExtensions>(SVGNames::RequiredExtensionsAttr());
      SVGTests::PropertyRegistry::RegisterConditionalProcessingAttributeProperty<
        &SVGConditionalProcessingAttributes::_systemLanguage>(SVGNames::SystemLanguageAttr());
    }
  }

  SVGTests::SVGTests(SVGElement *contextElement) noexcept : _contextElement(CreateWeakPtr(contextElement))
  {
  }

  bool SVGTests::HasExtension(DOMStringView extension) noexcept
  {
    // We recognize XHTML and MathML, as implemented in Gecko and suggested in the SVG Tiny
    // recommendation (http://www.w3.org/TR/SVG11/struct.html#RequiredExtensionsAttribute).
    if (extension == infra::namespaces::MathML)
    {
      return true;
    }
    return extension == infra::namespaces::HTML;
  }

  bool SVGTests::IsValid() const noexcept
  {
    auto *attributes = ConditionalProcessingAttributesIfExists();
    if (!attributes)
    {
      return true;
    }

    auto defaultLanguage = i18n::DefaultLanguage();
    auto genericDefaultLanguage = DOMStringView(defaultLanguage).substr(0, 2);
    for (auto &language : attributes->SystemLanguage().Items())
    {
      if (language != genericDefaultLanguage)
      {
        return false;
      }
    }
    for (auto &extension : attributes->RequiredExtensions().Items())
    {
      if (!HasExtension(extension))
      {
        return false;
      }
    }
    return true;
  }

  void SVGTests::ParseAttribute(const QualifiedName &name, Maybe<DOMStringView> value) noexcept
  {
    // NOTE: an attribute that's present but empty evaluates to false (its list holds an empty string); one
    // that's absent evaluates to true.
    auto reset = [&](SVGStringList &list)
    {
      if (value)
      {
        list.Reset(*value);
      }
      else
      {
        list.ClearItems();
      }
    };

    if (name == SVGNames::RequiredExtensionsAttr())
    {
      reset(RequiredExtensions());
    }
    else if (name == SVGNames::SystemLanguageAttr())
    {
      reset(SystemLanguage());
    }
  }

  void SVGTests::SVGAttributeChanged(const QualifiedName &attributeName) noexcept
  {
    if (!PropertyRegistry::IsKnownAttribute(attributeName))
    {
      return;
    }

    auto &contextElement = ContextElement();
    if (!contextElement.IsConnected())
    {
      return;
    }
    contextElement.InvalidateStyleAndRenderersForSubtree();
  }

  SVGElement &SVGTests::ContextElement() const noexcept
  {
    krys_assert(_contextElement);
    return *_contextElement;
  }

  SVGConditionalProcessingAttributes &SVGTests::ConditionalProcessingAttributes() noexcept
  {
    return ContextElement().ConditionalProcessingAttributes();
  }

  SVGConditionalProcessingAttributes *SVGTests::ConditionalProcessingAttributesIfExists() const noexcept
  {
    return ContextElement().ConditionalProcessingAttributesIfExists();
  }
}
