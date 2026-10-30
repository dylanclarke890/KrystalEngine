#include "Krystal.Booey/Editing/BreakBlockquoteCommand.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Rendering/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include <string>

namespace krys::boo
{
  using dom::TreeQueries;

  BreakBlockquoteCommand::BreakBlockquoteCommand(dom::Document &document) noexcept
      : CompositeEditCommand(document)
  {
  }

  Ref<BreakBlockquoteCommand> BreakBlockquoteCommand::Create(dom::Document &document) noexcept
  {
    return CreateRef<BreakBlockquoteCommand>(document);
  }

  void BreakBlockquoteCommand::DoApply() noexcept
  {
    if (EndingSelection().IsNone())
    {
      return;
    }

    // Delete the current selection.
    if (EndingSelection().IsRange())
    {
      DeleteSelection(false, false);
    }

    // This is a scenario that should never happen, but we want to
    // make sure we don't dereference a null pointer below.

    krys_debug_assert(!EndingSelection().IsNone());

    if (EndingSelection().IsNone())
    {
      return;
    }

    VisiblePosition visiblePos = EndingSelection().VisibleStart();

    // pos is a position equivalent to the caret.  We use downstream() so that pos will
    // be in the first node that we need to move (there are a few exceptions to this, see below).
    dom::Position pos = EndingSelection().Start().Downstream();

    // Find the top-most blockquote from the start.
    auto topBlockquoteNode = HighestEnclosingNodeOfType(pos, IsMailBlockquote);
    RefPtr<dom::Element> topBlockquote = ShareRefPtr(DynamicDowncast<dom::Element>(topBlockquoteNode.get()));
    if (!topBlockquote || !topBlockquote->ParentNode())
    {
      return;
    }

    Ref<dom::Element> breakNode = [&]() -> Ref<dom::Element>
    {
      auto lineBreak = CreateHTMLElement(GetDocument(), u8"br");
      auto *containerNode = pos.GetContainerNode();
      if (!containerNode || !containerNode->GetRenderStyle())
      {
        return lineBreak;
      }

      auto *parentStyle = topBlockquote->ParentNode()->GetRenderStyle();
      if (!parentStyle)
      {
        return lineBreak;
      }

      if (parentStyle->GetWritingMode().BidiDirection()
          == containerNode->GetRenderStyle()->GetWritingMode().BidiDirection())
      {
        return lineBreak;
      }

      auto container = CreateHTMLElement(GetDocument(), u8"div");
      (void)container->SetAttribute(dom::DOMStringAtom(u8"dir"), dom::DOMString(u8"auto"));
      (void)container->AppendChild(*lineBreak);
      return container;
    }();

    bool isLastVisPosInNode = IsLastVisiblePositionInNode(visiblePos, topBlockquote.get());

    // If the position is at the beginning of the top quoted content, we don't need to break the quote.
    // Instead, insert the break before the blockquote, unless the position is as the end of the quoted
    // content.
    if (IsFirstVisiblePositionInNode(visiblePos, topBlockquote.get()) && !isLastVisPosInNode)
    {
      (void)InsertNodeBefore(*breakNode, *topBlockquote);
      SetEndingSelection(VisibleSelection(dom::PositionBeforeNode(breakNode.get()), Affinity::Downstream,
                                          EndingSelection().GetDirectionality()));
      RebalanceWhitespace();
      return;
    }

    // Insert a break after the top blockquote.
    InsertNodeAfter(*breakNode, *topBlockquote);

    // If we're inserting the break at the end of the quoted content, we don't need to break the quote.
    if (isLastVisPosInNode)
    {
      SetEndingSelection(VisibleSelection(dom::PositionBeforeNode(breakNode.get()), Affinity::Downstream,
                                          EndingSelection().GetDirectionality()));
      RebalanceWhitespace();
      return;
    }

    // Don't move a line break just after the caret.  Doing so would create an extra, empty paragraph
    // in the new blockquote.
    if (LineBreakExistsAtVisiblePosition(visiblePos))
    {
      pos = pos.Next();
    }

    // Adjust the position so we don't split at the beginning of a quote.
    while (
      IsFirstVisiblePositionInNode(VisiblePosition(pos), EnclosingNodeOfType(pos, IsMailBlockquote).get()))
    {
      pos = pos.Previous();
    }

    // startNode is the first node that we need to move to the new blockquote.
    RefPtr<dom::Node> startNode = ShareRefPtr(pos.DeprecatedNode());
    krys_debug_assert(startNode);
    // Split at pos if in the middle of a text node.
    if (auto *textNode = DynamicDowncast<dom::Text>(*startNode))
    {
      if (static_cast<size_t>(pos.DeprecatedEditingOffset()) >= textNode->Length())
      {
        if (auto *nextNode = TreeQueries::Next(*startNode))
        {
          startNode = ShareRefPtr(nextNode);
        }
      }
      else if (pos.DeprecatedEditingOffset() > 0)
      {
        SplitTextNode(*textNode, static_cast<uint32>(pos.DeprecatedEditingOffset()));
      }
    }
    else if (pos.DeprecatedEditingOffset() > 0)
    {
      auto *child = startNode->FirstChild();
      for (int32 i = 0; child && i < pos.DeprecatedEditingOffset(); ++i)
      {
        child = child->NextSibling();
      }
      if (child)
      {
        startNode = ShareRefPtr(child);
      }
      else if (auto *next = TreeQueries::Next(*startNode))
      {
        startNode = ShareRefPtr(next);
      }
    }

    // If there's nothing inside topBlockquote to move, we're finished.
    if (!startNode->IsDescendantOf(*topBlockquote))
    {
      SetEndingSelection(VisibleSelection(VisiblePosition(FirstPositionInOrBeforeNode(startNode.get())),
                                          EndingSelection().GetDirectionality()));
      return;
    }

    // Build up list of ancestors in between the start node and the top blockquote.
    List<RefPtr<dom::Element>> ancestors;
    for (auto *node = startNode->ParentElement(); node && node != topBlockquote; node = node->ParentElement())
    {
      ancestors.push_back(ShareRefPtr(node));
    }

    // Insert a clone of the top blockquote after the break.
    auto clonedBlockquote = CloneElementWithoutChildren(*topBlockquote);
    InsertNodeAfter(*clonedBlockquote, *breakNode);

    // Clone startNode's ancestors into the cloned blockquote.
    // On exiting this loop, clonedAncestor is the lowest ancestor
    // that was cloned (i.e. the clone of either ancestors.last()
    // or clonedBlockquote if ancestors is empty).
    RefPtr<dom::Element> clonedAncestor = clonedBlockquote;
    for (size_t i = ancestors.size(); i != 0; --i)
    {
      auto clonedChild = CloneElementWithoutChildren(*ancestors[i - 1]);
      // Preserve list item numbering in cloned lists.
      if (IsHTMLElementOfType(clonedChild.get(), html::TagName::ol))
      {
        dom::Node *listChildNode = i > 1 ? static_cast<dom::Node *>(ancestors[i - 2].get()) : startNode.get();
        // The first child of the cloned list might not be a list item element,
        // find the first one so that we know where to start numbering.
        while (listChildNode && !IsHTMLElementOfType(listChildNode, html::TagName::li))
        {
          listChildNode = listChildNode->NextSibling();
        }
        if (listChildNode)
        {
          if (auto *listItemRenderer = DynamicDowncast<RenderListItem>(listChildNode->Renderer()))
          {
            auto value = std::to_string(listItemRenderer->Value());
            SetNodeAttribute(*clonedChild, dom::DOMStringAtom(u8"start"),
                             dom::DOMString(reinterpret_cast<const char8_t *>(value.data()), value.size()));
          }
        }
      }

      AppendNode(*clonedChild, *clonedAncestor);
      clonedAncestor = krys::move(clonedChild);
    }

    MoveRemainingSiblingsToNewParent(startNode.get(), nullptr, *clonedAncestor);

    if (!ancestors.empty())
    {
      // Split the tree up the ancestor chain until the topBlockquote
      // Throughout this loop, clonedParent is the clone of ancestor's parent.
      // This is so we can clone ancestor's siblings and place the clones
      // into the clone corresponding to the ancestor's parent.
      RefPtr<dom::Element> ancestor;
      RefPtr<dom::Element> clonedParent;
      for (ancestor = ancestors.front(), clonedParent = ShareRefPtr(clonedAncestor->ParentElement());
           ancestor && ancestor != topBlockquote; ancestor = ShareRefPtr(ancestor->ParentElement()),
          clonedParent = ShareRefPtr(clonedParent->ParentElement()))
      {
        if (!clonedParent)
        {
          break;
        }
        MoveRemainingSiblingsToNewParent(ancestor->NextSibling(), nullptr, *clonedParent);
      }

      // If the startNode's original parent is now empty, remove it
      RefPtr<dom::Element> originalParent = ancestors.front();
      if (!originalParent->HasChildNodes())
      {
        RemoveNode(*originalParent);
      }
    }

    // Make sure the cloned block quote renders.
    (void)AddBlockPlaceholderIfNeeded(clonedBlockquote.get());

    // Put the selection right before br or at the first position in div.
    auto beforeBROrFirstPositionInDiv = IsAtomicNode(breakNode.get())
                                          ? dom::PositionBeforeNode(breakNode.get())
                                          : dom::FirstPositionInNode(breakNode.get());
    SetEndingSelection(VisibleSelection(beforeBROrFirstPositionInDiv, Affinity::Downstream,
                                        EndingSelection().GetDirectionality()));
    RebalanceWhitespace();
  }
}
