#include "Krystal.Booey/SVG/SVGURIReference.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Mixins/NonElementParentNode.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Url.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGMarkerResource.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGUseElement.hpp"
#include "Krystal.Booey/URL/URL.hpp"

namespace krys::boo::svg
{
  SVGURIReference::SVGURIReference(SVGElement *contextElement) noexcept
      : _href(SVGAnimatedString::Create(contextElement))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGURIReference::_href>(SVGNames::HrefAttr());
      PropertyRegistry::RegisterProperty<&SVGURIReference::_href>(XLinkNames::HrefAttr());
    }
  }

  bool SVGURIReference::IsKnownAttribute(const dom::QualifiedName &attributeName) noexcept
  {
    return PropertyRegistry::IsKnownAttribute(attributeName);
  }

  SVGElement &SVGURIReference::ContextElement() const noexcept
  {
    return *_href->ContextElement();
  }

  void SVGURIReference::ParseAttribute(const dom::QualifiedName &name,
                                       Maybe<dom::DOMStringView> value) noexcept
  {
    if (name.Matches(SVGNames::HrefAttr()))
    {
      if (value)
      {
        _href->SetBaseValInternal(dom::DOMString {*value});
      }
      else
      {
        const auto &xlinkHref = XLinkNames::HrefAttr();
        _href->SetBaseValInternal(ContextElement()
                                    .GetAttributeNS(xlinkHref.NamespaceURI(), xlinkHref.LocalName())
                                    .value_or(dom::DOMString {}));
      }
    }
    else if (name.Matches(XLinkNames::HrefAttr())
             && !ContextElement().HasAttributeNS(dom::DOMStringAtom::Null(),
                                                 SVGNames::HrefAttr().LocalName()))
    {
      _href->SetBaseValInternal(dom::DOMString {value.value_or(dom::DOMStringView {})});
    }
  }

  dom::DOMStringAtom SVGURIReference::FragmentIdentifierFromIRIString(dom::DOMStringView url,
                                                                      const dom::Document &document) noexcept
  {
    size_t start = url.find(u8'#');
    if (start == dom::DOMStringView::npos)
    {
      return dom::DOMStringAtom::Empty();
    }

    if (!start)
    {
      return dom::DOMStringAtom(url.substr(1));
    }

    auto base = document.CompleteURL(url.substr(0, start), document.BaseURL());
    auto fragmentIdentifier = url.substr(start);
    auto urlWithFragment = document.CompleteURL(fragmentIdentifier, base);
    if (url::EqualIgnoringFragmentIdentifier(urlWithFragment, document.GetURL()))
    {
      return dom::DOMStringAtom(fragmentIdentifier.substr(1));
    }

    // The url doesn't have any fragment identifier.
    return dom::DOMStringAtom::Empty();
  }

  dom::DOMStringAtom SVGURIReference::FragmentIdentifierFromIRIString(const style::URL &url,
                                                                      const dom::Document &document) noexcept
  {
    return FragmentIdentifierFromIRIString(url.Resolved.String(), document);
  }

  dom::DOMStringAtom
    SVGURIReference::FragmentIdentifierFromIRIString(const style::SVGMarkerResource &markerResource,
                                                     const dom::Document &document) noexcept
  {
    if (auto url = markerResource.TryURL())
    {
      return FragmentIdentifierFromIRIString(*url, document);
    }
    return dom::DOMStringAtom::Empty();
  }

  bool SVGURIReference::IsExternalURIReference(dom::DOMStringView uri, const dom::Document &document) noexcept
  {
    // Fragment-only URIs are always internal
    if (uri.starts_with(u8'#'))
    {
      return false;
    }

    // If the URI matches our documents URL, we're dealing with a local reference.
    auto url = document.CompleteURL(uri);
    krys_assert(!url.ProtocolIsData());
    return !url::EqualIgnoringFragmentIdentifier(url, document.GetURL());
  }

  auto SVGURIReference::TargetElementFromIRIString(dom::DOMStringView iri,
                                                   const dom::DocumentOrShadowRoot &treeScope,
                                                   RefPtr<dom::Document> externalDocument) noexcept
    -> TargetElementResult
  {
    // If there's no fragment identifier contained within the IRI string, we can't lookup an element.
    size_t startOfFragmentIdentifier = iri.find(u8'#');
    if (startOfFragmentIdentifier == dom::DOMStringView::npos)
    {
      return {};
    }

    // Exclude the '#' character when determining the fragmentIdentifier.
    auto id = dom::DOMStringAtom(iri.substr(startOfFragmentIdentifier + 1));
    if (id.View().empty())
    {
      return {};
    }

    auto &document = treeScope.RootNode().NodeDocument();
    auto url = document.CompleteURL(iri);
    if (externalDocument)
    {
      // Enforce that the referenced url matches the url of the document that we've loaded for it!
      krys_assert(url::EqualIgnoringFragmentIdentifier(url, externalDocument->GetURL()));
      return {externalDocument->GetElementById(id.View()), krys::move(id)};
    }

    if (url.ProtocolIsData())
    {
      // FIXME: We need to load the data url in a Document to be able to get the target element.
      if (!url::EqualIgnoringFragmentIdentifier(url, document.GetURL()))
      {
        return {nullptr, krys::move(id)};
      }
    }

    // Exit early if the referenced url is external, and we have no externalDocument given.
    if (IsExternalURIReference(iri, document))
    {
      return {nullptr, krys::move(id)};
    }

    auto *shadowHost = dom::TreeQueries::ShadowHost(treeScope.RootNode());
    if (auto *svgHost = DynamicDowncast<SVGElement>(shadowHost); svgHost && Is<SVGUseElement>(*svgHost))
    {
      return {
        dom::mixins::NonElementParentNode::GetElementById(shadowHost->TreeScope().RootNode(), id.View()),
        krys::move(id)};
    }

    return {dom::mixins::NonElementParentNode::GetElementById(treeScope.RootNode(), id.View()),
            krys::move(id)};
  }

  auto SVGURIReference::TargetElementFromIRIString(const style::URL &iri,
                                                   const dom::DocumentOrShadowRoot &treeScope,
                                                   RefPtr<dom::Document> externalDocument) noexcept
    -> TargetElementResult
  {
    return TargetElementFromIRIString(iri.Resolved.String(), treeScope, krys::move(externalDocument));
  }

  bool SVGURIReference::HaveLoadedRequiredResources() const noexcept
  {
    if (Href().empty())
    {
      return true;
    }
    auto &document = ContextElement().NodeDocument();
    if (document.CompleteURL(Href()).ProtocolIsData())
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
