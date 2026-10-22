#include "Krystal.Booey/Editing/MergeIdenticalElementsCommand.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"

namespace krys::boo
{
  Ref<MergeIdenticalElementsCommand> MergeIdenticalElementsCommand::Create(dom::Element &element1,
                                                                           dom::Element &element2) noexcept
  {
    return CreateRef<MergeIdenticalElementsCommand>(element1, element2);
  }

  MergeIdenticalElementsCommand::MergeIdenticalElementsCommand(dom::Element &first,
                                                               dom::Element &second) noexcept
      : SimpleEditCommand(first.NodeDocument()), _element1(ShareRef(first)), _element2(ShareRef(second))
  {
    krys_debug_assert(_element1->NextSibling() == _element2.get());
  }

  void MergeIdenticalElementsCommand::DoApply() noexcept
  {
    if (_element1->NextSibling() != _element2.get() || !_element1->HasEditableStyle()
        || !_element2->HasEditableStyle())
    {
      return;
    }

    _atChild = ShareRefPtr(_element2->FirstChild());

    List<Ref<dom::Node>> children;
    for (auto *child = _element1->FirstChild(); child; child = child->NextSibling())
    {
      children.push_back(ShareRef(*child));
    }

    for (auto &child : children)
    {
      (void)_element2->InsertBefore(*child, _atChild.get());
    }

    (void)dom::mixins::ChildNode::Remove(*_element1);
  }

  void MergeIdenticalElementsCommand::DoUnapply() noexcept
  {
    RefPtr<dom::Node> atChild = krys::move(_atChild);

    RefPtr<dom::ContainerNode> parent = ShareRefPtr(_element2->ParentNode());
    if (!parent || !parent->HasEditableStyle())
    {
      return;
    }

    if (parent->InsertBefore(*_element1, _element2.get()).HasException())
    {
      return;
    }

    List<Ref<dom::Node>> children;
    for (auto *child = _element2->FirstChild(); child && child != atChild.get(); child = child->NextSibling())
    {
      children.push_back(ShareRef(*child));
    }

    for (auto &child : children)
    {
      (void)_element1->AppendChild(*child);
    }
  }
}
