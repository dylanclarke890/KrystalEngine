#include "Krystal.Booey/DOM/VisitedLinkState.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::dom
{
  namespace
  {
    KRYS_NODISCARD Maybe<DOMString> LinkAttribute(const Element &element) noexcept
    {
      if (!css::IsLink(element))
      {
        return {};
      }
      if (Is<html::HTMLElement>(element))
      {
        return element.GetAttribute(DOMStringAtom(u8"href"));
      }
      if (Is<svg::SVGElement>(element))
      {
        if (auto href = element.GetAttribute(DOMStringAtom(u8"href")))
        {
          return href;
        }
        return element.GetAttributeNS(DOMStringAtom(infra::Namespaces::XLink), DOMStringAtom(u8"href"));
      }
      return {};
    }
  }

  VisitedLinkState::VisitedLinkState(Document &document) noexcept
      : _document(WeakPtr<Document>::WithRef(document))
  {
  }

  InsideLink VisitedLinkState::DetermineLinkState(const Element &element) const noexcept
  {
    if (!css::IsLink(element))
    {
      return InsideLink::NotInside;
    }
    return DetermineLinkStateSlowCase(element);
  }

  InsideLink VisitedLinkState::DetermineLinkStateSlowCase(const Element &element) const noexcept
  {
    krys_debug_assert(css::IsLink(element));

    auto attribute = LinkAttribute(element);
    if (!attribute)
    {
      return InsideLink::NotInside;
    }

    // An empty href refers to the document itself which is always visited. It is useful to check this
    // explicitly so that visited links can be tested in platform independent manner, without explicit support
    // in the test harness.
    if (attribute->empty())
    {
      return InsideLink::InsideVisited;
    }

    // FIXME: Without a page there is no visited link store to ask.
    return InsideLink::InsideUnvisited;
  }
}
