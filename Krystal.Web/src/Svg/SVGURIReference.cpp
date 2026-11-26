/*
 * Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
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

#include "Krystal.Web/Svg/SVGURIReference.hpp"
#include "Krystal.Net/Url.hpp"
#include "Krystal.Web/Dom/Mixins/DocumentOrShadowRoot.hpp"
#include "Krystal.Web/Dom/Mixins/NonElementParentNode.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Traversal/TreeTraversal.hpp"
#include "Krystal.Web/Style/Values/Primitives/Url.hpp"
#include "Krystal.Web/Style/Values/SVG/SVGMarkerResource.hpp"
#include "Krystal.Web/Svg/Elements/SVGElement.hpp"
#include "Krystal.Web/Svg/Elements/SVGUseElement.hpp"
#include "Krystal.Web/Svg/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Web/Svg/SVGNames.hpp"

namespace krys::web
{
  SVGURIReference::SVGURIReference(SVGElement *contextElement) noexcept
      : _href(SVGAnimatedString::Create(contextElement))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGURIReference::_href>(SVGNames::HrefAttr());
      PropertyRegistry::RegisterProperty<&SVGURIReference::_href>(svg::XLinkNames::HrefAttr());
    }
  }

  bool SVGURIReference::IsKnownAttribute(const QualifiedName &attributeName) noexcept
  {
    return PropertyRegistry::IsKnownAttribute(attributeName);
  }

  SVGElement &SVGURIReference::ContextElement() const noexcept
  {
    return *_href->ContextElement();
  }

  void SVGURIReference::ParseAttribute(const QualifiedName &name, Maybe<DOMStringView> value) noexcept
  {
    if (name.Matches(SVGNames::HrefAttr()))
    {
      if (value)
      {
        _href->SetBaseValInternal(DOMString {*value});
      }
      else
      {
        const auto &xlinkHref = svg::XLinkNames::HrefAttr();
        _href->SetBaseValInternal(ContextElement()
                                    .GetAttributeNS(xlinkHref.NamespaceURI(), xlinkHref.LocalName())
                                    .value_or(DOMString {}));
      }
    }
    else if (name.Matches(svg::XLinkNames::HrefAttr())
             && !ContextElement().HasAttributeNS(null, SVGNames::HrefAttr().LocalName()))
    {
      _href->SetBaseValInternal(DOMString {value.value_or(DOMStringView {})});
    }
  }

  DOMStringAtom SVGURIReference::FragmentIdentifierFromIRIString(DOMStringView url,
                                                                 const Document &document) noexcept
  {
    size_t start = url.find(u8'#');
    if (start == DOMStringView::npos)
    {
      return DOMStringAtom::Empty();
    }

    if (!start)
    {
      return DOMStringAtom(url.substr(1));
    }

    auto base = document.CompleteUrl(url.substr(0, start), document.BaseUrl());
    auto fragmentIdentifier = url.substr(start);
    auto urlWithFragment = document.CompleteUrl(fragmentIdentifier, base);
    if (net::EqualIgnoringFragmentIdentifier(urlWithFragment, document.GetUrl()))
    {
      return DOMStringAtom(fragmentIdentifier.substr(1));
    }

    // The url doesn't have any fragment identifier.
    return DOMStringAtom::Empty();
  }

  DOMStringAtom SVGURIReference::FragmentIdentifierFromIRIString(const style::Url &url,
                                                                 const Document &document) noexcept
  {
    return FragmentIdentifierFromIRIString(url.Resolved.String(), document);
  }

  DOMStringAtom
    SVGURIReference::FragmentIdentifierFromIRIString(const style::SVGMarkerResource &markerResource,
                                                     const Document &document) noexcept
  {
    if (auto url = markerResource.TryUrl())
    {
      return FragmentIdentifierFromIRIString(*url, document);
    }
    return DOMStringAtom::Empty();
  }

  bool SVGURIReference::IsExternalURIReference(DOMStringView uri, const Document &document) noexcept
  {
    // Fragment-only URIs are always internal
    if (uri.starts_with(u8'#'))
    {
      return false;
    }

    // If the URI matches our documents URL, we're dealing with a local reference.
    auto url = document.CompleteUrl(uri);
    krys_assert(!url.ProtocolIsData());
    return !net::EqualIgnoringFragmentIdentifier(url, document.GetUrl());
  }

  auto SVGURIReference::TargetElementFromIRIString(DOMStringView iri, const DocumentOrShadowRoot &treeScope,
                                                   Document *externalDocument) noexcept -> TargetElementResult
  {
    // If there's no fragment identifier contained within the IRI string, we can't lookup an element.
    size_t startOfFragmentIdentifier = iri.find(u8'#');
    if (startOfFragmentIdentifier == DOMStringView::npos)
    {
      return {};
    }

    // Exclude the '#' character when determining the fragmentIdentifier.
    auto id = DOMStringAtom(iri.substr(startOfFragmentIdentifier + 1));
    if (id.View().empty())
    {
      return {};
    }

    auto &document = treeScope.RootNode().NodeDocument();
    auto url = document.CompleteUrl(iri);
    if (externalDocument)
    {
      // Enforce that the referenced url matches the url of the document that we've loaded for it!
      krys_assert(net::EqualIgnoringFragmentIdentifier(url, externalDocument->GetUrl()));
      return {externalDocument->GetElementById(id.View()), krys::move(id)};
    }

    if (url.ProtocolIsData())
    {
      // FIXME: We need to load the data url in a Document to be able to get the target element.
      if (!net::EqualIgnoringFragmentIdentifier(url, document.GetUrl()))
      {
        return {nullptr, krys::move(id)};
      }
    }

    // Exit early if the referenced url is external, and we have no externalDocument given.
    if (IsExternalURIReference(iri, document))
    {
      return {nullptr, krys::move(id)};
    }

    auto *shadowHost = dom::TreeTraversal::ShadowHost(treeScope.RootNode());
    if (auto *svgHost = DynamicDowncast<SVGElement>(shadowHost); svgHost && Is<SVGUseElement>(*svgHost))
    {
      return {
        dom::mixins::NonElementParentNode::GetElementById(shadowHost->TreeScope().RootNode(), id.View()),
        krys::move(id)};
    }

    return {dom::mixins::NonElementParentNode::GetElementById(treeScope.RootNode(), id.View()),
            krys::move(id)};
  }

  auto SVGURIReference::TargetElementFromIRIString(const style::Url &iri,
                                                   const DocumentOrShadowRoot &treeScope,
                                                   Document *externalDocument) noexcept -> TargetElementResult
  {
    return TargetElementFromIRIString(iri.Resolved.String(), treeScope, externalDocument);
  }

  bool SVGURIReference::HaveLoadedRequiredResources() const noexcept
  {
    if (Href().empty())
    {
      return true;
    }
    auto &document = ContextElement().NodeDocument();
    if (document.CompleteUrl(Href()).ProtocolIsData())
    {
      return true;
    }
    if (!IsExternalURIReference(Href(), document))
    {
      return true;
    }
    return ErrorOccurred() || HaveFiredLoadEvent();
  }

  void SVGURIReference::DispatchLoadEvent() noexcept
  {
    if (HaveFiredLoadEvent())
    {
      return;
    }

    // Dispatch the load event
    SetHaveFiredLoadEvent(true);
    krys_assert(ContextElement().HaveLoadedRequiredResources());

    ContextElement().SendLoadEventIfPossible();
  }
}
