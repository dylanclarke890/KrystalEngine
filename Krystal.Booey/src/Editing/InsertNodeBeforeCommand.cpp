#include "Krystal.Booey/Editing/InsertNodeBeforeCommand.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"

namespace krys::boo
{
  Ref<InsertNodeBeforeCommand>
    InsertNodeBeforeCommand::Create(dom::Node &childToInsert, dom::Node &childToInsertBefore,
                                    ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable,
                                    EditAction editingAction) noexcept
  {
    return CreateRef<InsertNodeBeforeCommand>(childToInsert, childToInsertBefore,
                                              shouldAssumeContentIsAlwaysEditable, editingAction);
  }

  InsertNodeBeforeCommand::InsertNodeBeforeCommand(
    dom::Node &insertChild, dom::Node &refChild,
    ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable,
    EditAction editingAction) noexcept
      : SimpleEditCommand(refChild.NodeDocument(), editingAction), _insertChild(ShareRef(insertChild)),
        _refChild(ShareRef(refChild)),
        _shouldAssumeContentIsAlwaysEditable(shouldAssumeContentIsAlwaysEditable)
  {
    krys_debug_assert(!_insertChild->ParentNode());
    krys_debug_assert(_refChild->ParentNode());
    krys_debug_assert(_refChild->ParentNode()->HasEditableStyle() || !_refChild->ParentNode()->Renderer());
  }

  void InsertNodeBeforeCommand::DoApply() noexcept
  {
    RefPtr<dom::ContainerNode> parent = ShareRefPtr(_refChild->ParentNode());
    if (!parent
        || (_shouldAssumeContentIsAlwaysEditable == DoNotAssumeContentIsAlwaysEditable
            && !IsEditableNode(*parent)))
    {
      return;
    }
    krys_debug_assert(IsEditableNode(*parent));

    (void)parent->InsertBefore(*_insertChild, _refChild.get());
  }

  void InsertNodeBeforeCommand::DoUnapply() noexcept
  {
    if (!IsEditableNode(*_insertChild))
    {
      return;
    }

    (void)dom::mixins::ChildNode::Remove(*_insertChild);
  }
}
