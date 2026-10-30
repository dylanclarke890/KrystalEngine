#include "Krystal.Booey/Editing/InsertLineBreakCommand.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/EditingStyle.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"

namespace krys::boo
{
  InsertLineBreakCommand::InsertLineBreakCommand(dom::Document &document) noexcept
      : CompositeEditCommand(document)
  {
  }

  Ref<InsertLineBreakCommand> InsertLineBreakCommand::Create(dom::Document &document) noexcept
  {
    return CreateRef<InsertLineBreakCommand>(document);
  }

  bool InsertLineBreakCommand::PreservesTypingStyle() const noexcept
  {
    return true;
  }

  // Whether we should insert a break element or a '\n'.
  bool InsertLineBreakCommand::ShouldUseBreakElement(const dom::Position &position) noexcept
  {
    // An editing position like [input, 0] actually refers to the position before
    // the input element, and in that case we need to check the input element's
    // parent's renderer.
    auto *node = position.ParentAnchoredEquivalent().DeprecatedNode();
    return node && node->Renderer() && !node->Renderer()->Style().PreserveNewline();
  }

  void InsertLineBreakCommand::DoApply() noexcept
  {
    DeleteSelection();
    VisibleSelection selection = EndingSelection();
    if (selection.IsNoneOrOrphaned())
    {
      return;
    }

    VisiblePosition caret(selection.VisibleStart());
    // FIXME: If the node is hidden, we should still be able to insert text.
    // For now, we return to avoid a crash.  https://bugs.webkit.org/show_bug.cgi?id=40342
    if (caret.IsNull())
    {
      return;
    }

    dom::Position position(caret.DeepEquivalent());

    position = PositionAvoidingSpecialElementBoundary(position);
    position = PositionOutsideTabSpan(position);

    if (!IsEditablePosition(position))
    {
      return;
    }

    Ref<dom::Document> document = ShareRef(GetDocument());
    RefPtr<dom::Node> nodeToInsert;
    if (ShouldUseBreakElement(position))
    {
      nodeToInsert = CreateHTMLElement(*document, u8"br");
    }
    else
    {
      nodeToInsert = document->CreateTextNode(u8"\n");
    }

    auto cloneWithoutChildren = [&](dom::Node &node) -> Ref<dom::Node>
    {
      auto clone = dom::NodeAlgorithms::CloneNode(node, &*document, false);
      krys_debug_assert(!clone.HasException());
      return clone.ReleaseValue();
    };

    // FIXME: Need to merge text nodes when inserting just after or before text.
    (void)document->UpdateLayoutIgnorePendingStylesheets();
    if (IsEndOfParagraph(caret) && !LineBreakExistsAtVisiblePosition(caret))
    {
      bool needExtraLineBreak = !IsHTMLElementOfType(position.DeprecatedNode(), html::TagName::hr)
                                && !IsHTMLElementOfType(position.DeprecatedNode(), html::TagName::table);

      InsertNodeAt(*nodeToInsert, position);

      if (needExtraLineBreak)
      {
        (void)InsertNodeBefore(*cloneWithoutChildren(*nodeToInsert), *nodeToInsert);
      }

      VisiblePosition endingPosition(dom::PositionBeforeNode(nodeToInsert.get()));
      SetEndingSelection(VisibleSelection(endingPosition, EndingSelection().GetDirectionality()));
    }
    else if (position.DeprecatedEditingOffset() <= CaretMinOffset(*position.DeprecatedNode()))
    {
      InsertNodeAt(*nodeToInsert, position);

      // Insert an extra br or '\n' if the just inserted one collapsed.
      if (!IsStartOfParagraph(dom::PositionBeforeNode(nodeToInsert.get())))
      {
        (void)InsertNodeBefore(*cloneWithoutChildren(*nodeToInsert), *nodeToInsert);
      }

      SetEndingSelection(VisibleSelection(dom::PositionInParentAfterNode(nodeToInsert.get()),
                                          Affinity::Downstream, EndingSelection().GetDirectionality()));
      // If we're inserting after all of the rendered text in a text node, or into a non-text node,
      // a simple insertion is sufficient.
    }
    else if (position.DeprecatedEditingOffset() >= CaretMaxOffset(*position.DeprecatedNode())
             || !Is<dom::Text>(*position.DeprecatedNode()))
    {
      InsertNodeAt(*nodeToInsert, position);
      SetEndingSelection(VisibleSelection(dom::PositionInParentAfterNode(nodeToInsert.get()),
                                          Affinity::Downstream, EndingSelection().GetDirectionality()));
    }
    else if (RefPtr<dom::Text> textNode = ShareRefPtr(DynamicDowncast<dom::Text>(*position.DeprecatedNode())))
    {
      // Split a text node
      SplitTextNode(*textNode, static_cast<uint32>(position.DeprecatedEditingOffset()));
      (void)InsertNodeBefore(*nodeToInsert, *textNode);
      dom::Position endingPosition = dom::FirstPositionInNode(textNode.get());

      // Handle whitespace that occurs after the split
      (void)document->UpdateLayoutIgnorePendingStylesheets();
      if (!endingPosition.IsRenderedCharacter())
      {
        dom::Position positionBeforeTextNode(dom::PositionInParentBeforeNode(textNode.get()));
        // Clear out all whitespace and insert one non-breaking space
        DeleteInsignificantTextDownstream(endingPosition);
        krys_debug_assert(!textNode->Renderer() || textNode->Renderer()->Style().CollapseWhiteSpace());
        // Deleting insignificant whitespace will remove textNode if it contains nothing but insignificant
        // whitespace.
        if (textNode->IsConnected())
        {
          InsertTextIntoNode(*textNode, 0, NonBreakingSpaceString());
        }
        else
        {
          auto nbspNode = document->CreateTextNode(utf8_string(NonBreakingSpaceString()));
          InsertNodeAt(*nbspNode, positionBeforeTextNode);
          endingPosition = dom::FirstPositionInNode(nbspNode.get());
        }
      }

      SetEndingSelection(
        VisibleSelection(endingPosition, Affinity::Downstream, EndingSelection().GetDirectionality()));
    }

    // Handle the case where there is a typing style.

    RefPtr<EditingStyle> typingStyle = ShareRefPtr(document->Selection().TypingStyle());

    if (typingStyle && !typingStyle->IsEmpty())
    {
      // Apply the typing style to the inserted line break, so that if the selection
      // leaves and then comes back, new input will have the right style.
      // FIXME: We shouldn't always apply the typing style to the line break here,
      // see <rdar://problem/5794462>.
      ApplyStyle(typingStyle.get(), FirstPositionInOrBeforeNode(nodeToInsert.get()),
                 LastPositionInOrAfterNode(nodeToInsert.get()));
      // Even though this applyStyle operates on a Range, it still sets an endingSelection().
      // It tries to set a VisibleSelection around the content it operated on. So, that VisibleSelection
      // will either (a) select the line break we inserted, or it will (b) be a caret just
      // before the line break (if the line break is at the end of a block it isn't selectable).
      // So, this next call sets the endingSelection() to a caret just after the line break
      // that we inserted, or just before it if it's at the end of a block.
      SetEndingSelection(VisibleSelection(EndingSelection().VisibleEnd()));
    }

    RebalanceWhitespace();
  }
}
