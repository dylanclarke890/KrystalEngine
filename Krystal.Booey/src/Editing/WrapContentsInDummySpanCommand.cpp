#include "Krystal.Booey/Editing/WrapContentsInDummySpanCommand.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"

namespace krys::boo
{
  Ref<WrapContentsInDummySpanCommand> WrapContentsInDummySpanCommand::Create(dom::Element &element) noexcept
  {
    return CreateRef<WrapContentsInDummySpanCommand>(element);
  }

  WrapContentsInDummySpanCommand::WrapContentsInDummySpanCommand(dom::Element &element) noexcept
      : SimpleEditCommand(element.NodeDocument()), _element(ShareRef(element))
  {
  }

  void WrapContentsInDummySpanCommand::ExecuteApply() noexcept
  {
    List<Ref<dom::Node>> children;
    for (auto *child = _element->FirstChild(); child; child = child->NextSibling())
    {
      children.push_back(ShareRef(*child));
    }

    for (auto &child : children)
    {
      (void)_dummySpan->AppendChild(*child);
    }

    (void)_element->AppendChild(*_dummySpan);
  }

  void WrapContentsInDummySpanCommand::DoApply() noexcept
  {
    _dummySpan = CreateStyleSpanElement(GetDocument());

    ExecuteApply();
  }

  void WrapContentsInDummySpanCommand::DoUnapply() noexcept
  {
    if (!_dummySpan)
    {
      return;
    }

    if (!_element->HasEditableStyle())
    {
      return;
    }

    List<Ref<dom::Node>> children;
    for (auto *child = _dummySpan->FirstChild(); child; child = child->NextSibling())
    {
      children.push_back(ShareRef(*child));
    }

    for (auto &child : children)
    {
      (void)_element->AppendChild(*child);
    }

    (void)dom::mixins::ChildNode::Remove(*_dummySpan);
  }

  void WrapContentsInDummySpanCommand::DoReapply() noexcept
  {
    if (!_dummySpan || !_element->HasEditableStyle())
    {
      return;
    }

    ExecuteApply();
  }
}
