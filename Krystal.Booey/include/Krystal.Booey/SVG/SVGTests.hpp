/*
 * Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2010 Rob Buis <buis@kde.org>
 * Copyright (C) 2018-2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/SVG/SVGStringList.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::svg
{
  class SVGElement;

  template <typename TOwner, typename... TBaseTypes>
  class SVGPropertyOwnerRegistry;

  /// @brief An element's conditional processing attributes, which it keeps in its rare data.
  /// @see https://svgwg.org/svg2-draft/struct.html#ConditionalProcessingAttributes
  class SVGConditionalProcessingAttributes
  {
    KRYS_NON_COPYABLE(SVGConditionalProcessingAttributes);

  private:
    Ref<SVGStringList> _requiredExtensions;
    Ref<SVGStringList> _systemLanguage;

  public:
    explicit SVGConditionalProcessingAttributes(SVGElement &contextElement) noexcept;

    KRYS_NODISCARD SVGStringList &RequiredExtensions() noexcept
    {
      return *_requiredExtensions;
    }

    KRYS_NODISCARD SVGStringList &SystemLanguage() noexcept
    {
      return *_systemLanguage;
    }
  };

  /// @brief The `SVGTests` mixin: the conditional processing attributes of the elements they apply to.
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGTests
  class SVGTests
  {
    KRYS_NON_COPYABLE(SVGTests);

  private:
    WeakPtr<SVGElement> _contextElement;

  public:
    KRYS_NODISCARD static bool HasExtension(dom::DOMStringView extension) noexcept;
    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGTests>;

    KRYS_NODISCARD SVGConditionalProcessingAttributes &ConditionalProcessingAttributes() noexcept;
    KRYS_NODISCARD SVGConditionalProcessingAttributes *
      ConditionalProcessingAttributesIfExists() const noexcept;

    // These methods are called from DOM through the super classes.
    KRYS_NODISCARD SVGStringList &RequiredExtensions() noexcept
    {
      return ConditionalProcessingAttributes().RequiredExtensions();
    }

    KRYS_NODISCARD SVGStringList &SystemLanguage() noexcept
    {
      return ConditionalProcessingAttributes().SystemLanguage();
    }

  protected:
    explicit SVGTests(SVGElement *contextElement) noexcept;

    KRYS_NODISCARD bool IsValid() const noexcept;

    void ParseAttribute(const dom::QualifiedName &name, Maybe<dom::DOMStringView> value) noexcept;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept;

  private:
    KRYS_NODISCARD SVGElement &ContextElement() const noexcept;
  };
}
