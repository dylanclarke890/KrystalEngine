#include "Krystal.Booey/Editing/InsertParagraphSeparatorCommand.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/EditingStyle.hpp"
#include "Krystal.Booey/Editing/InsertLineBreakCommand.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"

namespace krys::boo
{
  using dom::TreeQueries;

  namespace
  {
    KRYS_NODISCARD bool IsHeading(const dom::Node *node) noexcept
    {
      return IsHTMLElementOfType(node, html::TagName::h1) || IsHTMLElementOfType(node, html::TagName::h2)
             || IsHTMLElementOfType(node, html::TagName::h3) || IsHTMLElementOfType(node, html::TagName::h4)
             || IsHTMLElementOfType(node, html::TagName::h5);
    }

    // When inserting a new line, we want to avoid nesting empty divs if we can.  Otherwise, when
    // pasting, it's easy to have each new line be a div deeper than the previous.  E.g., in the case
    // below, we want to insert at ^ instead of |.
    // <div>foo<div>bar</div>|</div>^
    KRYS_NODISCARD RefPtr<dom::Element>
      HighestVisuallyEquivalentDivBelowRoot(dom::Element *startBlock) noexcept
    {
      auto *currentBlock = startBlock;
      // We don't want to return a root node (if it happens to be a div, e.g., in a document fragment) because
      // there are no siblings for us to append to.
      while (!currentBlock->NextSibling()
             && IsHTMLElementOfType(currentBlock->ParentElement(), html::TagName::div)
             && currentBlock->ParentElement()->ParentElement())
      {
        if (currentBlock->ParentElement()->HasAttributes())
        {
          break;
        }
        currentBlock = currentBlock->ParentElement();
      }
      return ShareRefPtr(currentBlock);
    }

    KRYS_NODISCARD bool IsPhrasingContent(const dom::Node &node) noexcept
    {
      auto *element = DynamicDowncast<dom::Element>(node);
      if (!element)
      {
        return false;
      }

      const auto &name = element->TagQualifiedName();
      switch (name.Namespace())
      {
        case html::Namespace::MathML: return name.TagName() == html::TagName::math;
        case html::Namespace::SVG:    return name.TagName() == html::TagName::svg;
        case html::Namespace::HTML:   break;
        default:                      return false;
      }

      switch (name.TagName())
      {
        case html::TagName::a:
        case html::TagName::abbr:
        case html::TagName::area:
        case html::TagName::audio:
        case html::TagName::b:
        case html::TagName::bdi:
        case html::TagName::bdo:
        case html::TagName::br:
        case html::TagName::button:
        case html::TagName::canvas:
        case html::TagName::cite:
        case html::TagName::code:
        case html::TagName::data:
        case html::TagName::datalist:
        case html::TagName::del:
        case html::TagName::dfn:
        case html::TagName::em:
        case html::TagName::embed:
        case html::TagName::i:
        case html::TagName::iframe:
        case html::TagName::img:
        case html::TagName::input:
        case html::TagName::ins:
        case html::TagName::kbd:
        case html::TagName::label:
        case html::TagName::link:
        case html::TagName::map:
        case html::TagName::mark:
        case html::TagName::meta:
        case html::TagName::meter:
        case html::TagName::noscript:
        case html::TagName::object:
        case html::TagName::output:
        case html::TagName::picture:
        case html::TagName::progress:
        case html::TagName::q:
        case html::TagName::ruby:
        case html::TagName::s:
        case html::TagName::samp:
        case html::TagName::script:
        case html::TagName::select:
        case html::TagName::slot:
        case html::TagName::small:
        case html::TagName::span:
        case html::TagName::strong:
        case html::TagName::sub:
        case html::TagName::sup:
        case html::TagName::template_:
        case html::TagName::textarea:
        case html::TagName::time:
        case html::TagName::u:
        case html::TagName::var:
        case html::TagName::video:
        case html::TagName::wbr:       return true;
        default:                       break;
      }
      return false;
    }

    KRYS_NODISCARD bool IsEditableRootPhrasingContent(const dom::Position &position) noexcept
    {
      auto editableRoot = HighestEditableRoot(position);
      if (!editableRoot)
      {
        return false;
      }
      return EnclosingNodeOfType(FirstPositionInOrBeforeNode(editableRoot.get()), IsPhrasingContent)
             != nullptr;
    }
  }

  InsertParagraphSeparatorCommand::InsertParagraphSeparatorCommand(dom::Document &document,
                                                                   bool mustUseDefaultParagraphElement,
                                                                   bool pasteBlockquoteIntoUnquotedArea,
                                                                   EditAction editingAction) noexcept
      : CompositeEditCommand(document, editingAction),
        _mustUseDefaultParagraphElement(mustUseDefaultParagraphElement),
        _pasteBlockquoteIntoUnquotedArea(pasteBlockquoteIntoUnquotedArea)
  {
  }

  InsertParagraphSeparatorCommand::~InsertParagraphSeparatorCommand() noexcept = default;

  Ref<InsertParagraphSeparatorCommand>
    InsertParagraphSeparatorCommand::Create(dom::Document &document, bool useDefaultParagraphElement,
                                            bool pasteBlockquoteIntoUnquotedArea,
                                            EditAction editingAction) noexcept
  {
    return CreateRef<InsertParagraphSeparatorCommand>(document, useDefaultParagraphElement,
                                                      pasteBlockquoteIntoUnquotedArea, editingAction);
  }

  bool InsertParagraphSeparatorCommand::PreservesTypingStyle() const noexcept
  {
    return true;
  }

  void InsertParagraphSeparatorCommand::CalculateStyleBeforeInsertion(const dom::Position &position) noexcept
  {
    // It is only important to set a style to apply later if we're at the boundaries of
    // a paragraph. Otherwise, content that is moved as part of the work of the command
    // will lend their styles to the new paragraph without any extra work needed.
    auto visiblePosition = VisiblePosition {position};
    if (!IsStartOfParagraph(visiblePosition) && !IsEndOfParagraph(visiblePosition))
    {
      return;
    }

    _style = EditingStyle::Create(position, EditingStyle::PropertiesToInclude::EditingPropertiesInEffect);
    _style->MergeTypingStyle(*position.GetDocument());
  }

  void InsertParagraphSeparatorCommand::ApplyStyleAfterInsertion(dom::Node *originalEnclosingBlock) noexcept
  {
    // Not only do we break out of header tags, but we also do not preserve the typing style,
    // in order to match other browsers.
    if (IsHeading(originalEnclosingBlock))
    {
      return;
    }

    RefPtr<EditingStyle> style = _style;
    if (!style)
    {
      return;
    }

    style->PrepareToApplyAt(EndingSelection().Start());
    if (!style->IsEmpty())
    {
      ApplyStyle(style.get());
    }
  }

  bool InsertParagraphSeparatorCommand::ShouldUseDefaultParagraphElement(
    dom::Node *enclosingBlock) const noexcept
  {
    if (_mustUseDefaultParagraphElement)
    {
      return true;
    }

    // Assumes that if there was a range selection, it was already deleted.
    if (!IsEndOfBlock(EndingSelection().VisibleStart()))
    {
      return false;
    }

    return IsHeading(enclosingBlock);
  }

  void InsertParagraphSeparatorCommand::GetAncestorsInsideBlock(
    const dom::Node *insertionNode, dom::Element *outerBlock, List<RefPtr<dom::Element>> &ancestors) noexcept
  {
    ancestors.clear();

    // Build up list of ancestors elements between the insertion node and the outer block.
    if (insertionNode != outerBlock)
    {
      for (auto *element = insertionNode->ParentElement(); element && element != outerBlock;
           element = element->ParentElement())
      {
        ancestors.push_back(ShareRefPtr(element));
      }
    }
  }

  Ref<dom::Element>
    InsertParagraphSeparatorCommand::CloneHierarchyUnderNewBlock(const List<RefPtr<dom::Element>> &ancestors,
                                                                 Ref<dom::Element> &&blockToInsert) noexcept
  {
    // Make clones of ancestors in between the start node and the start block.
    Ref<dom::Element> parent = krys::move(blockToInsert);
    for (size_t i = ancestors.size(); i != 0; --i)
    {
      auto child = CloneElementWithoutChildren(*ancestors[i - 1]);
      // It should always be okay to remove id from the cloned elements, since the originals are not deleted.
      child->RemoveAttribute(dom::DOMStringAtom(u8"id"));
      AppendNode(*child, *parent);
      parent = krys::move(child);
    }

    return parent;
  }

  void InsertParagraphSeparatorCommand::DoApply() noexcept
  {
    if (EndingSelection().IsNoneOrOrphaned())
    {
      return;
    }

    auto insertionPosition = EndingSelection().Start();
    auto affinity = EndingSelection().GetAffinity();

    // Delete the current selection.
    if (EndingSelection().IsRange())
    {
      CalculateStyleBeforeInsertion(insertionPosition);
      DeleteSelection(false, true);
      insertionPosition = EndingSelection().Start();
      affinity = EndingSelection().GetAffinity();
    }

    Ref<dom::Document> document = ShareRef(GetDocument());
    // FIXME: The parentAnchoredEquivalent conversion needs to be moved into enclosingBlock.
    RefPtr<dom::Element> startBlock =
      EnclosingBlock(ShareRefPtr(insertionPosition.ParentAnchoredEquivalent().GetContainerNode()));
    dom::Position canonicalPos = VisiblePosition(insertionPosition).DeepEquivalent();
    if (!startBlock || !startBlock->NonShadowBoundaryParentNode()
        || IsEditableRootPhrasingContent(insertionPosition) || IsRenderedTable(startBlock.get())
        || IsTableCell(*startBlock)
        || IsHTMLElementOfType(startBlock.get(), html::TagName::form)
        // FIXME: If the node is hidden, we don't have a canonical position so we will do the wrong thing for
        // tables and <hr>. https://bugs.webkit.org/show_bug.cgi?id=40342
        || (!canonicalPos.IsNull() && canonicalPos.DeprecatedNode()->Renderer()
            && canonicalPos.DeprecatedNode()->Renderer()->IsRenderTable())
        || (!canonicalPos.IsNull() && IsHTMLElementOfType(canonicalPos.DeprecatedNode(), html::TagName::hr)))
    {
      ApplyCommandToComposite(InsertLineBreakCommand::Create(*document));
      return;
    }

    // Use the leftmost candidate.
    insertionPosition = insertionPosition.Upstream();
    if (!insertionPosition.IsCandidate())
    {
      insertionPosition = insertionPosition.Downstream();
    }

    // Adjust the insertion position after the delete
    insertionPosition = PositionAvoidingSpecialElementBoundary(insertionPosition);
    VisiblePosition visiblePos(insertionPosition, affinity);
    if (visiblePos.IsNull())
    {
      return;
    }

    if (!startBlock->Contains(visiblePos.DeepEquivalent().GetContainerNode()))
    {
      return;
    }

    CalculateStyleBeforeInsertion(insertionPosition);

    //---------------------------------------------------------------------
    // Handle special case of typing return on an empty list item
    if (BreakOutOfEmptyListItem())
    {
      return;
    }

    //---------------------------------------------------------------------
    // Prepare for more general cases.

    bool isFirstInBlock = IsStartOfBlock(visiblePos);
    bool isLastInBlock = IsEndOfBlock(visiblePos);
    bool nestNewBlock = false;

    // FIXME: <http://webkit.org/b/211864> If insertionPosition is not editable, we should compute a position
    // that is.
    if (!IsEditablePosition(insertionPosition))
    {
      return;
    }

    // Create block to be inserted.
    RefPtr<dom::Element> blockToInsert;
    if (startBlock->IsRootEditableElement())
    {
      blockToInsert = CreateDefaultParagraphElement(*document);
      nestNewBlock = true;
    }
    else if (ShouldUseDefaultParagraphElement(startBlock.get()))
    {
      blockToInsert = CreateDefaultParagraphElement(*document);
    }
    else
    {
      blockToInsert = CloneElementWithoutChildren(*startBlock);
    }

    //---------------------------------------------------------------------
    // Handle case when position is in the last visible position in its block,
    // including when the block is empty.
    if (isLastInBlock)
    {
      if (nestNewBlock)
      {
        if (isFirstInBlock && !LineBreakExistsAtVisiblePosition(visiblePos))
        {
          // The block is empty.  Create an empty block to
          // represent the paragraph that we're leaving.
          auto extraBlock = CreateDefaultParagraphElement(*document);
          AppendNode(*extraBlock, *startBlock);
          if (!AppendBlockPlaceholder(*extraBlock))
          {
            return;
          }
        }
        AppendNode(*blockToInsert, *startBlock);
      }
      else
      {
        // We can get here if we pasted a copied portion of a blockquote with a newline at the end and are
        // trying to paste it into an unquoted area. We then don't want the newline within the blockquote or
        // else it will also be quoted.
        if (_pasteBlockquoteIntoUnquotedArea)
        {
          if (auto highestBlockquote = HighestEnclosingNodeOfType(canonicalPos, &IsMailBlockquote))
          {
            startBlock = ShareRefPtr(&Downcast<dom::Element>(*highestBlockquote));
          }
        }

        // Most of the time we want to stay at the nesting level of the startBlock (e.g., when nesting within
        // lists).  However, for div nodes, this can result in nested div tags that are hard to break out of.
        RefPtr<dom::Element> siblingNode = startBlock;
        if (IsHTMLElementOfType(blockToInsert.get(), html::TagName::div))
        {
          siblingNode = HighestVisuallyEquivalentDivBelowRoot(startBlock.get());
        }
        InsertNodeAfter(*blockToInsert, *siblingNode);
      }

      // Recreate the same structure in the new paragraph.

      List<RefPtr<dom::Element>> ancestors;
      GetAncestorsInsideBlock(PositionOutsideTabSpan(insertionPosition).DeprecatedNode(), startBlock.get(),
                              ancestors);
      auto parent = CloneHierarchyUnderNewBlock(ancestors, Ref<dom::Element>(blockToInsert));

      if (!AppendBlockPlaceholder(*parent))
      {
        return;
      }

      SetEndingSelection(
        VisibleSelection(VisiblePosition(dom::FirstPositionInNode(parent.get()), Affinity::Downstream),
                         EndingSelection().GetDirectionality()));
      return;
    }

    //---------------------------------------------------------------------
    // Handle case when position is in the first visible position in its block, and
    // similar case where previous position is in another, presumeably nested, block.
    if (isFirstInBlock || !InSameBlock(visiblePos, visiblePos.Previous()))
    {
      RefPtr<dom::Node> refNode;

      insertionPosition = PositionOutsideTabSpan(insertionPosition);

      if (isFirstInBlock && !nestNewBlock)
      {
        refNode = startBlock;
      }
      else if (isFirstInBlock && nestNewBlock)
      {
        // startBlock should always have children, otherwise isLastInBlock would be true and it's handled
        // above.
        krys_debug_assert(startBlock->FirstChild());
        refNode = ShareRefPtr(startBlock->FirstChild());
      }
      else if (insertionPosition.GetContainerNode() == startBlock && nestNewBlock)
      {
        refNode = ShareRefPtr(TreeQueries::ChildAt(
          *startBlock, static_cast<size_t>(insertionPosition.ComputeOffsetInContainerNode())));
        krys_debug_assert(refNode); // must be true or we'd be in the end of block case
      }
      else
      {
        refNode = ShareRefPtr(insertionPosition.DeprecatedNode());
      }

      // find ending selection position easily before inserting the paragraph
      insertionPosition = insertionPosition.Downstream();

      (void)InsertNodeBefore(*blockToInsert, *refNode);

      // Recreate the same structure in the new paragraph.

      List<RefPtr<dom::Element>> ancestors;
      GetAncestorsInsideBlock(
        PositionAvoidingSpecialElementBoundary(PositionOutsideTabSpan(insertionPosition)).DeprecatedNode(),
        startBlock.get(), ancestors);

      auto parent = CloneHierarchyUnderNewBlock(ancestors, Ref<dom::Element>(blockToInsert));
      if (!AppendBlockPlaceholder(*parent))
      {
        return;
      }

      // In this case, we need to set the new ending selection.
      SetEndingSelection(VisibleSelection(VisiblePosition(insertionPosition, Affinity::Downstream),
                                          EndingSelection().GetDirectionality()));
      return;
    }

    //---------------------------------------------------------------------
    // Handle the (more complicated) general case,

    // All of the content in the current block after visiblePos is
    // about to be wrapped in a new paragraph element.  Add a br before
    // it if visiblePos is at the start of a paragraph so that the
    // content will move down a line.
    if (IsStartOfParagraph(visiblePos))
    {
      auto br = CreateHTMLElement(*document, u8"br");
      InsertNodeAt(*br, insertionPosition);
      if (!br->ParentNode())
      {
        return;
      }
      insertionPosition = dom::PositionInParentAfterNode(br.get());
      // If the insertion point is a break element, there is nothing else
      // we need to do.
      if (auto *renderer = visiblePos.DeepEquivalent().AnchorNode()->Renderer(); renderer && renderer->IsBR())
      {
        SetEndingSelection(VisibleSelection(VisiblePosition(insertionPosition, Affinity::Downstream),
                                            EndingSelection().GetDirectionality()));
        return;
      }
    }

    // Move downstream. Typing style code will take care of carrying along the
    // style of the upstream position.
    insertionPosition = insertionPosition.Downstream();

    // At this point, the insertionPosition's node could be a container, and we want to make sure we include
    // all of the correct nodes when building the ancestor list.  So this needs to be the deepest
    // representation of the position before we walk the DOM tree.
    insertionPosition = PositionOutsideTabSpan(VisiblePosition(insertionPosition).DeepEquivalent());
    if (insertionPosition.IsNull())
    {
      return;
    }

    // If the returned position lies either at the end or at the start of an element that is ignored by
    // editing we should move to its upstream or downstream position.
    if (EditingIgnoresContent(*insertionPosition.DeprecatedNode()))
    {
      if (insertionPosition.AtLastEditingPositionForNode())
      {
        insertionPosition = insertionPosition.Downstream();
      }
      else if (insertionPosition.AtFirstEditingPositionForNode())
      {
        insertionPosition = insertionPosition.Upstream();
      }
    }

    // Make sure we do not cause a rendered space to become unrendered.
    // FIXME: We need the affinity for pos, but pos.downstream() does not give it
    dom::Position leadingWhitespace =
      insertionPosition.LeadingWhitespacePosition(VisiblePosition::DefaultAffinity);
    // FIXME: leadingWhitespacePosition is returning the position before preserved newlines for positions
    // after the preserved newline, causing the newline to be turned into a nbsp.
    if (RefPtr<dom::Text> textNode =
          ShareRefPtr(DynamicDowncast<dom::Text>(leadingWhitespace.DeprecatedNode())))
    {
      krys_debug_assert(!textNode->Renderer() || textNode->Renderer()->Style().CollapseWhiteSpace());
      ReplaceTextInNodePreservingMarkers(*textNode,
                                         static_cast<uint32>(leadingWhitespace.DeprecatedEditingOffset()), 1,
                                         NonBreakingSpaceString());
    }

    // Split at pos if in the middle of a text node.
    dom::Position positionAfterSplit;
    if (RefPtr<dom::Text> textNode =
          ShareRefPtr(DynamicDowncast<dom::Text>(*insertionPosition.GetContainerNode()));
        textNode && insertionPosition.GetAnchorType() == dom::Position::AnchorType::PositionIsOffsetInAnchor)
    {
      bool atEnd = static_cast<size_t>(insertionPosition.OffsetInContainerNode()) >= textNode->Length();
      if (insertionPosition.DeprecatedEditingOffset() > 0 && !atEnd)
      {
        SplitTextNode(*textNode, static_cast<uint32>(insertionPosition.OffsetInContainerNode()));
        positionAfterSplit = dom::FirstPositionInNode(textNode.get());
        if (!textNode->PreviousSibling())
        {
          return; // Bail out if mutation events detachd the split text node.
        }
        insertionPosition.MoveToPosition(textNode->PreviousSibling(),
                                         static_cast<uint32>(insertionPosition.OffsetInContainerNode()));
        visiblePos = VisiblePosition(insertionPosition);
      }
    }

    // If we got detached due to mutation events, just bail out.
    if (!startBlock->ParentNode())
    {
      return;
    }

    // Put the added block in the tree.
    if (nestNewBlock)
    {
      AppendNode(*blockToInsert, *startBlock);
    }
    else
    {
      InsertNodeAfter(*blockToInsert, *startBlock);
    }

    (void)document->UpdateLayoutIgnorePendingStylesheets();

    // If the paragraph separator was inserted at the end of a paragraph, an empty line must be
    // created.  All of the nodes, starting at visiblePos, are about to be added to the new paragraph
    // element.  If the first node to be inserted won't be one that will hold an empty line open, add a br.
    if (IsEndOfParagraph(visiblePos) && !LineBreakExistsAtVisiblePosition(visiblePos))
    {
      AppendNode(*CreateHTMLElement(*document, u8"br"), *blockToInsert);
    }

    // Move the start node and the siblings of the start node.
    if (VisiblePosition(insertionPosition) != VisiblePosition(dom::PositionBeforeNode(blockToInsert.get())))
    {
      RefPtr<dom::Node> n;
      if (insertionPosition.GetContainerNode() == startBlock)
      {
        n = ShareRefPtr(insertionPosition.ComputeNodeAfterPosition());
      }
      else
      {
        RefPtr<dom::Node> splitTo = ShareRefPtr(insertionPosition.GetContainerNode());
        if (Is<dom::Text>(*splitTo) && insertionPosition.OffsetInContainerNode() >= CaretMaxOffset(*splitTo))
        {
          splitTo = ShareRefPtr(TreeQueries::Next(*splitTo, startBlock.get()));
        }
        if (splitTo)
        {
          (void)SplitTreeToNode(*splitTo, *startBlock);

          for (n = ShareRefPtr(startBlock->FirstChild()); n; n = ShareRefPtr(n->NextSibling()))
          {
            if (VisiblePosition(insertionPosition) <= VisiblePosition(dom::PositionBeforeNode(n.get())))
            {
              break;
            }
          }
        }
      }

      MoveRemainingSiblingsToNewParent(n.get(), blockToInsert.get(), *blockToInsert);
    }

    // Handle whitespace that occurs after the split
    if (positionAfterSplit.IsNotNull())
    {
      (void)document->UpdateLayoutIgnorePendingStylesheets();
      if (!positionAfterSplit.IsRenderedCharacter())
      {
        // Clear out all whitespace and insert one non-breaking space
        krys_debug_assert(!positionAfterSplit.GetContainerNode()->Renderer()
                          || positionAfterSplit.GetContainerNode()->Renderer()->Style().CollapseWhiteSpace());
        DeleteInsignificantTextDownstream(positionAfterSplit);
        if (Is<dom::Text>(*positionAfterSplit.DeprecatedNode()))
        {
          InsertTextIntoNode(Downcast<dom::Text>(*positionAfterSplit.GetContainerNode()), 0,
                             NonBreakingSpaceString());
        }
      }
    }

    SetEndingSelection(
      VisibleSelection(VisiblePosition(dom::FirstPositionInNode(blockToInsert.get()), Affinity::Downstream),
                       EndingSelection().GetDirectionality()));
    ApplyStyleAfterInsertion(startBlock.get());
  }
}
