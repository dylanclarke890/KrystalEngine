#include "Krystal.Booey/Editing/Commands/SplitTextNodeCommand.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/Editing/Commands/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"

namespace krys::boo
{
  Ref<SplitTextNodeCommand> SplitTextNodeCommand::Create(dom::Text &node, uint32 offset) noexcept
  {
    return CreateRef<SplitTextNodeCommand>(node, offset);
  }

  SplitTextNodeCommand::SplitTextNodeCommand(dom::Text &text, uint32 offset) noexcept
      : SimpleEditCommand(text.NodeDocument()), _text2(ShareRef(text)), _offset(offset)
  {
    // NOTE: Various callers rely on the fact that the original node becomes
    // the second node (i.e. the new node is inserted before the existing one).
    // That is not a fundamental dependency (i.e. it could be re-coded), but
    // rather is based on how this code happens to work.
    krys_debug_assert(_text2->Length() > 0);
    krys_debug_assert(_offset > 0);
    krys_debug_assert(_offset < _text2->Length());
  }

  // (There are no document markers to copy; see DIFFERENCES.md.)
  void SplitTextNodeCommand::DoApply() noexcept
  {
    auto *parent = _text2->ParentNode();
    if (!parent || !parent->HasEditableStyle())
    {
      return;
    }

    auto result = _text2->SubstringData(0, _offset);
    if (result.HasException())
    {
      return;
    }
    auto prefixText = result.ReleaseValue();
    if (prefixText.empty())
    {
      return;
    }

    _text1 = GetDocument().CreateTextNode(krys::move(prefixText));
    krys_debug_assert(_text1);

    InsertText1AndTrimText2();
  }

  void SplitTextNodeCommand::DoUnapply() noexcept
  {
    RefPtr<dom::Text> text1 = _text1;
    if (!text1 || !text1->HasEditableStyle())
    {
      return;
    }

    krys_debug_assert(&text1->NodeDocument() == &GetDocument());

    utf8_string prefixText = text1->Data();

    (void)_text2->InsertData(0, prefixText);

    (void)dom::mixins::ChildNode::Remove(*text1);
  }

  void SplitTextNodeCommand::DoReapply() noexcept
  {
    if (!_text1)
    {
      return;
    }

    auto *parent = _text2->ParentNode();
    if (!parent || !parent->HasEditableStyle())
    {
      return;
    }

    InsertText1AndTrimText2();
  }

  void SplitTextNodeCommand::InsertText1AndTrimText2() noexcept
  {
    if (_text2->ParentNode()->InsertBefore(*_text1, _text2.get()).HasException())
    {
      return;
    }
    (void)_text2->DeleteData(0, _offset);
  }
}
