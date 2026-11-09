/*
 * Copyright (C) 2004, 2005, 2008, 2009 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
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
#include "Krystal.Booey/SVG/Properties/SVGAnimatedString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
  class DocumentOrShadowRoot;
  class Element;
}

namespace krys::boo::style
{
  struct SVGMarkerResource;
  struct URL;
}

namespace krys::boo::svg
{
  class SVGElement;

  template <typename TOwner, typename... TBaseTypes>
  class SVGPropertyOwnerRegistry;

  /// @brief The `SVGURIReference` mixin: the `href` of the elements that refer to a resource.
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGURIReference
  class SVGURIReference
  {
    KRYS_NON_COPYABLE(SVGURIReference);

  private:
    Ref<SVGAnimatedString> _href;

  public:
    virtual ~SVGURIReference() noexcept = default;

    void ParseAttribute(const dom::QualifiedName &name, Maybe<dom::DOMStringView> value) noexcept;

    KRYS_NODISCARD static dom::DOMStringAtom
      FragmentIdentifierFromIRIString(dom::DOMStringView url, const dom::Document &document) noexcept;
    KRYS_NODISCARD static dom::DOMStringAtom
      FragmentIdentifierFromIRIString(const style::URL &url, const dom::Document &document) noexcept;
    KRYS_NODISCARD static dom::DOMStringAtom
      FragmentIdentifierFromIRIString(const style::SVGMarkerResource &markerResource,
                                      const dom::Document &document) noexcept;

    struct TargetElementResult
    {
      RefPtr<dom::Element> Element;
      dom::DOMStringAtom Identifier {dom::DOMStringAtom::Null()};
    };

    KRYS_NODISCARD static TargetElementResult
      TargetElementFromIRIString(dom::DOMStringView iri, const dom::DocumentOrShadowRoot &treeScope,
                                 RefPtr<dom::Document> externalDocument = nullptr) noexcept;
    KRYS_NODISCARD static TargetElementResult
      TargetElementFromIRIString(const style::URL &iri, const dom::DocumentOrShadowRoot &treeScope,
                                 RefPtr<dom::Document> externalDocument = nullptr) noexcept;

    KRYS_NODISCARD static bool IsExternalURIReference(dom::DOMStringView uri,
                                                      const dom::Document &document) noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGURIReference>;

    KRYS_NODISCARD const dom::DOMString &Href() const noexcept
    {
      return _href->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedString &HrefAnimated() noexcept
    {
      return *_href;
    }

  protected:
    explicit SVGURIReference(SVGElement *contextElement) noexcept;

    KRYS_NODISCARD static bool IsKnownAttribute(const dom::QualifiedName &attributeName) noexcept;

    KRYS_NODISCARD virtual bool HaveFiredLoadEvent() const noexcept
    {
      return false;
    }

    virtual void SetHaveFiredLoadEvent(bool) noexcept
    {
    }

    KRYS_NODISCARD virtual bool ErrorOccurred() const noexcept
    {
      return false;
    }

    virtual void SetErrorOccurred(bool) noexcept
    {
    }

    KRYS_NODISCARD bool HaveLoadedRequiredResources() const noexcept;
    void DispatchLoadEvent() noexcept;

  private:
    KRYS_NODISCARD SVGElement &ContextElement() const noexcept;
  };
}
