#include "Krystal.Booey/Html/Elements/HTMLTitleElement.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/Dom/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/Dom/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Html/CustomElement/CustomElementReactionQueue.hpp"

namespace krys::boo
{
  HTMLTitleElement::HTMLTitleElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Title)
  {
  }

#pragma region HTMLTitleElement

  DOMString HTMLTitleElement::Text() const noexcept
  {
    return dom::TextAlgorithms::ChildTextContent(*this);
  }

  ExceptionOr<void> HTMLTitleElement::Text(DOMString value) noexcept
  {
    html::CEReactionsScope ceReactions;
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

  void HTMLTitleElement::OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
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
