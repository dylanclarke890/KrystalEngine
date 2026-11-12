#include "Krystal.Booey/HTML/Elements/HTMLTitleElement.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"

namespace krys::boo::html
{
  HTMLTitleElement::HTMLTitleElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Title)
  {
  }

#pragma region HTMLTitleElement

  dom::DOMString HTMLTitleElement::Text() const noexcept
  {
    return dom::TextAlgorithms::ChildTextContent(*this);
  }

  dom::ExceptionOr<void> HTMLTitleElement::Text(dom::DOMString value) noexcept
  {
    CEReactionsScope ceReactions;
    return dom::NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
  }

#pragma endregion

  void HTMLTitleElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();
    if (IsConnected())
    {
      TitleMayHaveChanged();
    }
  }

  void HTMLTitleElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
    if (oldAncestor.IsConnected())
    {
      TitleMayHaveChanged();
    }
  }

  void HTMLTitleElement::OnChildrenChanged() noexcept
  {
    HTMLElement::OnChildrenChanged();
    if (IsConnected())
    {
      TitleMayHaveChanged();
    }
  }

  void HTMLTitleElement::TitleMayHaveChanged() noexcept
  {
    auto &document = NodeDocument();
    if (auto *cache = document.ExistingAXObjectCache())
    {
      cache->OnTitleChange(document);
    }
  }
}
