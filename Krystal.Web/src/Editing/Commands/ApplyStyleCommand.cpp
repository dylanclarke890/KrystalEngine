/*
 * Copyright (C) 2005-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Web/Editing/Commands/ApplyStyleCommand.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Web/Css/Properties/MutableStyleProperties.hpp"
#include "Krystal.Web/Css/Values/PrimitiveValue.hpp"
#include "Krystal.Web/Css/Values/SerialisationContext.hpp"
#include "Krystal.Web/Dom/Internals/TreeTraversal.hpp"
#include "Krystal.Web/Dom/Nodes/ContainerNode.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Dom/Ranges/SimpleRange.hpp"
#include "Krystal.Web/Dom/Trees/TreeQueries.hpp"
#include "Krystal.Web/Editing/Editing.hpp"
#include "Krystal.Web/Editing/EditingStyle.hpp"
#include "Krystal.Web/Editing/Markup/HTMLInterchange.hpp"
#include "Krystal.Web/Editing/Selection/VisibleSelection.hpp"
#include "Krystal.Web/Editing/Selection/VisibleUnits.hpp"
#include "Krystal.Web/Editing/TextIterator.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLFontElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLSpanElement.hpp"
#include "Krystal.Web/Rendering/RenderLineBreak.hpp"
#include "Krystal.Web/Rendering/RenderText.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Web/Style/Extractor/Extractor.hpp"
#include <algorithm>
#include <utility>

namespace krys::web
{
  using css::PropertyId;
  using css::ValueId;
  using dom::TreeQueries;
  using dom::TreeTraversal;

  namespace
  {
    KRYS_NODISCARD bool
      HasNoAttributeOrOnlyStyleAttribute(const StyledElement &element,
                                         ShouldStyleAttributeBeEmpty shouldStyleAttributeBeEmpty) noexcept
    {
      if (!element.HasAttributes())
      {
        return true;
      }

      uint32 matchedAttributes = 0;
      if (element.HasAttribute(u8"style")
          && (shouldStyleAttributeBeEmpty == AllowNonEmptyStyleAttribute || !element.InlineStyle()
              || element.InlineStyle()->empty()))
      {
        matchedAttributes++;
      }

      auto attributeCount = static_cast<uint32>(element.AttributeList().size());
      krys_debug_assert(matchedAttributes <= attributeCount);
      return matchedAttributes == attributeCount;
    }

    KRYS_NODISCARD bool IsSpanWithoutAttributesOrUnstyledStyleSpan(const Element &element) noexcept
    {
      return IsHTMLElementOfType(&element, html::TagName::span)
             && HasNoAttributeOrOnlyStyleAttribute(Downcast<StyledElement>(element),
                                                   StyleAttributeShouldBeEmpty);
    }

    KRYS_NODISCARD Ref<Element> CreateFontElement(Document &document) noexcept
    {
      return CreateHTMLElement(document, u8"font");
    }

    KRYS_NODISCARD Ref<css::MutableStyleProperties>
      CopyStyleOrCreateEmpty(const css::StyleProperties *style) noexcept
    {
      if (!style)
      {
        return css::MutableStyleProperties::Create();
      }
      return style->MutableCopy();
    }

    KRYS_NODISCARD ValueId ComputedUnicodeBidi(web::Node *node) noexcept
    {
      auto value = style::Extractor(node).PropertyValue(PropertyId::UnicodeBidi);
      return value ? css::LonghandValueId(PropertyId::UnicodeBidi, *value) : ValueId::Invalid;
    }

    KRYS_NODISCARD ContainerNode *DummySpanAncestorForNode(web::Node *node) noexcept
    {
      RefPtr<web::Node> currentNode = ShareRefPtr(node);
      while (currentNode)
      {
        auto *element = DynamicDowncast<Element>(*currentNode);
        if (element && IsStyleSpanOrSpanWithOnlyStyleAttribute(*element))
        {
          break;
        }
        currentNode = ShareRefPtr(currentNode->ParentNode());
      }
      return currentNode ? currentNode->ParentNode() : nullptr;
    }

    KRYS_NODISCARD RefPtr<web::Node> HighestEmbeddingAncestor(web::Node *startNode,
                                                              web::Node *enclosingNode) noexcept
    {
      for (RefPtr<web::Node> currentNode = ShareRefPtr(startNode);
           currentNode && currentNode != enclosingNode; currentNode = ShareRefPtr(currentNode->ParentNode()))
      {
        if (Is<HTMLElement>(*currentNode) && ComputedUnicodeBidi(currentNode.get()) == ValueId::Embed)
        {
          return currentNode;
        }
      }

      return nullptr;
    }

    KRYS_NODISCARD bool ContainsNonEditableRegion(web::Node &node) noexcept
    {
      if (!node.HasEditableStyle())
      {
        return true;
      }

      RefPtr<web::Node> sibling = ShareRefPtr(TreeTraversal::NextSkippingChildren(node));
      for (RefPtr<web::Node> descendant = ShareRefPtr(node.FirstChild()); descendant && descendant != sibling;
           descendant = ShareRefPtr(TreeQueries::Next(*descendant)))
      {
        if (!descendant->HasEditableStyle())
        {
          return true;
        }
      }

      return false;
    }

    struct InlineRunToApplyStyle
    {
      RefPtr<web::Node> Start;
      RefPtr<web::Node> End;
      RefPtr<web::Node> PastEndNode;
      Position PositionForStyleComputation;
      RefPtr<web::Node> DummyElement;
      StyleChange Change;

      InlineRunToApplyStyle(web::Node *start, web::Node *end, web::Node *pastEndNode) noexcept
          : Start(ShareRefPtr(start)), End(ShareRefPtr(end)), PastEndNode(ShareRefPtr(pastEndNode))
      {
        krys_debug_assert(start->ParentNode() == end->ParentNode());
      }

      KRYS_NODISCARD bool StartAndEndAreStillInDocument() const noexcept
      {
        return Start && End && Start->IsConnected() && End->IsConnected();
      }
    };

    KRYS_NODISCARD DOMString JoinWithSpace(const DOMString &a, const DOMString &b) noexcept
    {
      if (a.empty())
      {
        return b;
      }
      if (b.empty())
      {
        return a;
      }
      DOMString result;
      result.reserve(a.size() + 1 + b.size());
      result.append(a);
      result.push_back(u8' ');
      result.append(b);
      return result;
    }
  }

  bool IsStyleSpanOrSpanWithOnlyStyleAttribute(const Element &element) noexcept
  {
    return IsHTMLElementOfType(&element, html::TagName::span)
           && HasNoAttributeOrOnlyStyleAttribute(Downcast<StyledElement>(element),
                                                 AllowNonEmptyStyleAttribute);
  }

  bool IsEmptyFontTag(const Element *element,
                      ShouldStyleAttributeBeEmpty shouldStyleAttributeBeEmpty) noexcept
  {
    return IsHTMLElementOfType(element, html::TagName::font)
           && HasNoAttributeOrOnlyStyleAttribute(Downcast<StyledElement>(*element),
                                                 shouldStyleAttributeBeEmpty);
  }

  Ref<Element> CreateStyleSpanElement(Document &document) noexcept
  {
    return CreateHTMLElement(document, u8"span");
  }

  ApplyStyleCommand::ApplyStyleCommand(Document &document, const EditingStyle *style, EditAction action,
                                       ApplyStylePropertyLevel level) noexcept
      : CompositeEditCommand(document, action), _style(style->Copy()), _propertyLevel(level),
        _start(EndingSelection().Start().Downstream()), _end(EndingSelection().End().Upstream()),
        _useEndingSelection(true), _removeOnly(false)
  {
  }

  ApplyStyleCommand::ApplyStyleCommand(Document &document, const EditingStyle *style, Position start,
                                       Position end, EditAction action,
                                       ApplyStylePropertyLevel level) noexcept
      : CompositeEditCommand(document, action), _style(style->Copy()), _propertyLevel(level),
        _start(krys::move(start)), _end(krys::move(end)), _useEndingSelection(false), _removeOnly(false)
  {
  }

  ApplyStyleCommand::ApplyStyleCommand(Element &element, bool removeOnly, EditAction action) noexcept
      : CompositeEditCommand(element.NodeDocument(), action), _style(EditingStyle::Create()),
        _start(EndingSelection().Start().Downstream()), _end(EndingSelection().End().Upstream()),
        _useEndingSelection(true), _styledInlineElement(ShareRef(element)), _removeOnly(removeOnly)
  {
  }

  ApplyStyleCommand::ApplyStyleCommand(Document &document, const EditingStyle *style,
                                       IsInlineElementToRemoveFunction isInlineElementToRemove,
                                       EditAction action) noexcept
      : CompositeEditCommand(document, action), _style(style->Copy()),
        _start(EndingSelection().Start().Downstream()), _end(EndingSelection().End().Upstream()),
        _useEndingSelection(true), _removeOnly(true),
        _isInlineElementToRemoveFunction(isInlineElementToRemove)
  {
  }

  Ref<ApplyStyleCommand> ApplyStyleCommand::Create(Document &document, const EditingStyle *style,
                                                   EditAction action, ApplyStylePropertyLevel level) noexcept
  {
    return CreateRef<ApplyStyleCommand>(document, style, action, level);
  }

  Ref<ApplyStyleCommand> ApplyStyleCommand::Create(Document &document, const EditingStyle *style,
                                                   const Position &start, const Position &end,
                                                   EditAction action, ApplyStylePropertyLevel level) noexcept
  {
    return CreateRef<ApplyStyleCommand>(document, style, start, end, action, level);
  }

  Ref<ApplyStyleCommand> ApplyStyleCommand::Create(Element &element, bool removeOnly,
                                                   EditAction action) noexcept
  {
    return CreateRef<ApplyStyleCommand>(element, removeOnly, action);
  }

  Ref<ApplyStyleCommand>
    ApplyStyleCommand::Create(Document &document, const EditingStyle *style,
                              IsInlineElementToRemoveFunction isInlineElementToRemoveFunction,
                              EditAction action) noexcept
  {
    return CreateRef<ApplyStyleCommand>(document, style, isInlineElementToRemoveFunction, action);
  }

  void ApplyStyleCommand::UpdateStartEnd(const Position &newStart, const Position &newEnd) noexcept
  {
    krys_debug_assert(!std::is_gt(TreeOrder<dom::TreeType::ShadowIncludingTree>(newStart, newEnd)));

    if (!_useEndingSelection && (newStart != _start || newEnd != _end))
    {
      _useEndingSelection = true;
    }

    bool wasBaseFirst =
      StartingSelection().IsBaseFirst() || StartingSelection().GetDirectionality() != Directionality::Strong;
    SetEndingSelection(VisibleSelection(VisiblePosition(wasBaseFirst ? newStart : newEnd),
                                        VisiblePosition(wasBaseFirst ? newEnd : newStart),
                                        EndingSelection().GetDirectionality()));
    _start = newStart;
    _end = newEnd;
  }

  Position ApplyStyleCommand::StartPosition() noexcept
  {
    if (_useEndingSelection)
    {
      return EndingSelection().Start();
    }

    return _start;
  }

  Position ApplyStyleCommand::EndPosition() noexcept
  {
    if (_useEndingSelection)
    {
      return EndingSelection().End();
    }

    return _end;
  }

  void ApplyStyleCommand::DoApply() noexcept
  {
    switch (_propertyLevel)
    {
      case ApplyStylePropertyLevel::Default:
      {
        // Apply the block-centric properties of the style.
        auto blockStyle = _style->ExtractAndRemoveBlockProperties();
        if (!blockStyle->IsEmpty())
        {
          ApplyBlockStyle(*blockStyle);
        }
        // Apply any remaining styles to the inline elements.
        if (!_style->IsEmpty() || _styledInlineElement || _isInlineElementToRemoveFunction)
        {
          ApplyInlineStyle(*_style);
        }
        break;
      }
      case ApplyStylePropertyLevel::ForceBlock:
        // Force all properties to be applied as block styles.
        ApplyBlockStyle(*_style);
        break;
    }
  }

  void ApplyStyleCommand::ApplyBlockStyle(EditingStyle &style) noexcept
  {
    // Update document layout once before removing styles so that we avoid the expense of
    // updating before each and every call to check a computed style.
    GetDocument().UpdateLayoutIgnorePendingStylesheets();

    auto start = StartPosition();
    auto end = EndPosition();
    if (end < start)
    {
      std::swap(start, end);
    }

    VisiblePosition visibleStart(start);
    VisiblePosition visibleEnd(end);

    if (visibleStart.IsNull() || visibleStart.IsOrphan() || visibleEnd.IsNull() || visibleEnd.IsOrphan())
    {
      return;
    }

    // Save and restore the selection endpoints using their indices in the editable root, since
    // addBlockStyleIfNeeded may moveParagraphs, which can remove these endpoints.
    // Calculate start and end indices from the start of the tree that they're in.
    RefPtr<ContainerNode> scopeRoot = HighestEditableRoot(visibleStart.DeepEquivalent());
    if (!scopeRoot)
    {
      return;
    }

    auto scope = dom::CreateRangeSelectingNodeContents(*scopeRoot);
    auto range = *CreateSimpleRange(VisiblePositionRange {visibleStart, visibleEnd});
    constexpr TextIteratorBehaviors behaviors {
      TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions};
    auto startIndex = CharacterCount({scope.Start, range.Start}, behaviors);
    auto endIndex = CharacterCount({scope.Start, range.End}, behaviors);

    VisiblePosition paragraphStart(StartOfParagraph(visibleStart));
    VisiblePosition nextParagraphStart(EndOfParagraph(paragraphStart).Next());
    if (visibleEnd != visibleStart && IsStartOfParagraph(visibleEnd))
    {
      visibleEnd = visibleEnd.Previous(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
    }
    VisiblePosition beyondEnd(EndOfParagraph(visibleEnd).Next());
    while (paragraphStart.IsNotNull() && paragraphStart != beyondEnd)
    {
      StyleChange styleChange(&style, paragraphStart.DeepEquivalent());
      if (styleChange.CssStyle() || _removeOnly)
      {
        RefPtr<web::Node> block =
          EnclosingBlock(ShareRefPtr(paragraphStart.DeepEquivalent().DeprecatedNode()));
        if (!_removeOnly)
        {
          RefPtr<web::Node> newBlock =
            MoveParagraphContentsToNewBlockIfNecessary(paragraphStart.DeepEquivalent());
          if (newBlock)
          {
            block = newBlock;
          }
        }
        krys_debug_assert(!block || Is<Element>(*block));
        if (RefPtr<HTMLElement> htmlBlock = DynamicDowncast<HTMLElement>(block))
        {
          RemoveCSSStyle(style, *htmlBlock);
          if (!_removeOnly)
          {
            AddBlockStyle(styleChange, *htmlBlock);
          }
        }

        if (nextParagraphStart.IsOrphan())
        {
          nextParagraphStart = EndOfParagraph(paragraphStart).Next();
        }
      }

      paragraphStart = nextParagraphStart;
      nextParagraphStart = EndOfParagraph(paragraphStart).Next();
    }

    auto startPosition =
      CreateDeprecatedLegacyPosition(ResolveCharacterLocation(scope, startIndex, behaviors));
    auto endPosition = CreateDeprecatedLegacyPosition(ResolveCharacterLocation(scope, endIndex, behaviors));
    UpdateStartEnd(startPosition, endPosition);
  }

  void ApplyStyleCommand::CleanupUnstyledStyleSpans(ContainerNode *dummySpanAncestor) noexcept
  {
    if (!dummySpanAncestor)
    {
      return;
    }

    // Dummy spans are created when text node is split, so that style information
    // can be propagated, which can result in more splitting. If a dummy span gets
    // cloned/split, the new node is always a sibling of it. Therefore, we scan
    // all the children of the dummy's parent

    List<Ref<Element>> toRemove;
    for (auto *child = dummySpanAncestor->FirstChild(); child; child = child->NextSibling())
    {
      auto *element = DynamicDowncast<Element>(*child);
      if (element && IsSpanWithoutAttributesOrUnstyledStyleSpan(*element))
      {
        toRemove.push_back(ShareRef(*element));
      }
    }

    for (auto &element : toRemove)
    {
      RemoveNodePreservingChildren(*element);
    }
  }

  RefPtr<HTMLElement>
    ApplyStyleCommand::SplitAncestorsWithUnicodeBidi(web::Node *node, bool before,
                                                     WritingDirection allowedDirection) noexcept
  {
    // We are allowed to leave the highest ancestor with unicode-bidi unsplit if it is unicode-bidi: embed and
    // direction: allowedDirection. In that case, we return the unsplit ancestor. Otherwise, we return 0.
    RefPtr<Element> block = EnclosingBlock(ShareRefPtr(node));
    if (!block || node == block)
    {
      return nullptr;
    }

    RefPtr<web::Node> highestAncestorWithUnicodeBidi;
    RefPtr<web::Node> nextHighestAncestorWithUnicodeBidi;
    ValueId highestAncestorUnicodeBidi = ValueId::Invalid;
    for (RefPtr<web::Node> ancestor = ShareRefPtr(node->ParentNode()); ancestor != block;
         ancestor = ShareRefPtr(ancestor->ParentNode()))
    {
      auto unicodeBidi = ComputedUnicodeBidi(ancestor.get());
      if (unicodeBidi != ValueId::Invalid && unicodeBidi != ValueId::Normal)
      {
        highestAncestorUnicodeBidi = unicodeBidi;
        nextHighestAncestorWithUnicodeBidi = highestAncestorWithUnicodeBidi;
        highestAncestorWithUnicodeBidi = ancestor;
      }
    }

    if (!highestAncestorWithUnicodeBidi)
    {
      return nullptr;
    }

    RefPtr<HTMLElement> unsplitAncestor;

    if (allowedDirection != WritingDirection::Natural && highestAncestorUnicodeBidi != ValueId::BidiOverride)
    {
      if (RefPtr<HTMLElement> highestAncestorElementWithUnicodeBidi =
            DynamicDowncast<HTMLElement>(highestAncestorWithUnicodeBidi))
      {
        auto highestAncestorDirection = EditingStyle::Create(highestAncestorElementWithUnicodeBidi.get(),
                                                             EditingStyle::PropertiesToInclude::AllProperties)
                                          ->TextDirection();
        if (highestAncestorDirection && *highestAncestorDirection == allowedDirection)
        {
          if (!nextHighestAncestorWithUnicodeBidi)
          {
            return highestAncestorElementWithUnicodeBidi;
          }

          unsplitAncestor = krys::move(highestAncestorElementWithUnicodeBidi);
          highestAncestorWithUnicodeBidi = nextHighestAncestorWithUnicodeBidi;
        }
      }
    }

    // Split every ancestor through highest ancestor with embedding.
    RefPtr<web::Node> currentNode = ShareRefPtr(node);
    while (currentNode)
    {
      RefPtr<Element> parent = ShareRefPtr(&Downcast<Element>(*currentNode->ParentNode()));
      if (before ? currentNode->PreviousSibling() : currentNode->NextSibling())
      {
        SplitElement(*parent, before ? *currentNode : *currentNode->NextSibling());
      }
      if (highestAncestorWithUnicodeBidi == parent)
      {
        break;
      }
      currentNode = parent;
    }

    return unsplitAncestor;
  }

  void ApplyStyleCommand::RemoveEmbeddingUpToEnclosingBlock(web::Node *node,
                                                            web::Node *unsplitAncestor) noexcept
  {
    RefPtr<Element> block = EnclosingBlock(ShareRefPtr(node));
    if (!block || node == block)
    {
      return;
    }

    RefPtr<web::Node> parent;
    for (RefPtr<web::Node> ancestor = ShareRefPtr(node->ParentNode());
         ancestor != block && ancestor != unsplitAncestor; ancestor = parent)
    {
      parent = ShareRefPtr(ancestor->ParentNode());
      RefPtr<StyledElement> element = DynamicDowncast<StyledElement>(ancestor);
      if (!element)
      {
        continue;
      }

      auto unicodeBidi = ComputedUnicodeBidi(element.get());
      if (unicodeBidi == ValueId::Invalid || unicodeBidi == ValueId::Normal)
      {
        continue;
      }

      // FIXME: This code should really consider the mapped attribute 'dir', the inline style declaration,
      // and all matching style rules in order to determine how to best set the unicode-bidi property to
      // 'normal'. For now, it assumes that if the 'dir' attribute is present, then removing it will suffice,
      // and otherwise it sets the property in the inline style declaration.
      if (element->HasAttribute(u8"dir"))
      {
        // FIXME: If this is a BDO element, we should probably just remove it if it has no
        // other attributes, like we (should) do with B and I elements.
        RemoveNodeAttribute(*element, u8"dir");
      }
      else
      {
        auto inlineStyle = CopyStyleOrCreateEmpty(element->InlineStyle());
        (void)inlineStyle->SetProperty(PropertyId::UnicodeBidi, ValueId::Normal);
        (void)inlineStyle->RemoveProperty(PropertyId::Direction);
        SetNodeAttribute(*element, u8"style", inlineStyle->AsText(css::DefaultSerialisationContext()));
        if (IsSpanWithoutAttributesOrUnstyledStyleSpan(*element))
        {
          RemoveNodePreservingChildren(*element);
        }
      }
    }
  }

  void ApplyStyleCommand::ApplyInlineStyle(EditingStyle &style) noexcept
  {
    RefPtr<ContainerNode> startDummySpanAncestor;
    RefPtr<ContainerNode> endDummySpanAncestor;

    // update document layout once before removing styles
    // so that we avoid the expense of updating before each and every call
    // to check a computed style
    GetDocument().UpdateLayoutIgnorePendingStylesheets();

    // adjust to the positions we want to use for applying style
    auto start = StartPosition();
    auto end = EndPosition();
    if (end < start)
    {
      std::swap(start, end);
    }

    // split the start node and containing element if the selection starts inside of it
    bool splitStart = IsValidCaretPositionInTextNode(start);
    if (splitStart)
    {
      if (ShouldSplitTextElement(start.DeprecatedNode()->ParentElement(), style))
      {
        SplitTextElementAtStart(start, end);
      }
      else
      {
        SplitTextAtStart(start, end);
      }
      start = StartPosition();
      end = EndPosition();
      if (start.IsNull() || end.IsNull())
      {
        return;
      }
      startDummySpanAncestor = ShareRefPtr(DummySpanAncestorForNode(start.DeprecatedNode()));
    }

    // split the end node and containing element if the selection ends inside of it
    bool splitEnd = IsValidCaretPositionInTextNode(end);
    if (splitEnd)
    {
      if (ShouldSplitTextElement(end.DeprecatedNode()->ParentElement(), style))
      {
        SplitTextElementAtEnd(start, end);
      }
      else
      {
        SplitTextAtEnd(start, end);
      }
      start = StartPosition();
      end = EndPosition();
      if (start.IsNull() || end.IsNull())
      {
        return;
      }
      endDummySpanAncestor = ShareRefPtr(DummySpanAncestorForNode(end.DeprecatedNode()));
    }

    if (start.IsNull() || start.IsOrphan() || end.IsNull() || end.IsOrphan())
    {
      return;
    }

    // Remove style from the selection.
    // Use the upstream position of the start for removing style.
    // This will ensure we remove all traces of the relevant styles from the selection
    // and prevent us from adding redundant ones, as described in:
    // <rdar://problem/3724344> Bolding and unbolding creates extraneous tags
    Position removeStart = start.Upstream();
    auto textDirection = style.TextDirection();
    RefPtr<EditingStyle> styleWithoutEmbedding;
    RefPtr<EditingStyle> embeddingStyle;
    if (textDirection)
    {
      // Leave alone an ancestor that provides the desired single level embedding, if there is one.
      auto startUnsplitAncestor = SplitAncestorsWithUnicodeBidi(start.DeprecatedNode(), true, *textDirection);
      auto endUnsplitAncestor = SplitAncestorsWithUnicodeBidi(end.DeprecatedNode(), false, *textDirection);
      RemoveEmbeddingUpToEnclosingBlock(start.DeprecatedNode(), startUnsplitAncestor.get());
      RemoveEmbeddingUpToEnclosingBlock(end.DeprecatedNode(), endUnsplitAncestor.get());

      // Avoid removing the dir attribute and the unicode-bidi and direction properties from the unsplit
      // ancestors.
      Position embeddingRemoveStart = removeStart;
      if (startUnsplitAncestor && NodeFullySelected(*startUnsplitAncestor, removeStart, end))
      {
        embeddingRemoveStart = PositionInParentAfterNode(startUnsplitAncestor.get());
      }

      Position embeddingRemoveEnd = end;
      if (endUnsplitAncestor && NodeFullySelected(*endUnsplitAncestor, removeStart, end))
      {
        embeddingRemoveEnd = PositionInParentBeforeNode(endUnsplitAncestor.get()).Downstream();
      }

      if (embeddingRemoveEnd != removeStart || embeddingRemoveEnd != end)
      {
        styleWithoutEmbedding = style.Copy();
        embeddingStyle = styleWithoutEmbedding->ExtractAndRemoveTextDirection();

        if (embeddingRemoveStart <= embeddingRemoveEnd)
        {
          RemoveInlineStyle(*embeddingStyle, embeddingRemoveStart, embeddingRemoveEnd);
        }
      }
    }

    RemoveInlineStyle(styleWithoutEmbedding ? *styleWithoutEmbedding : style, removeStart, end);
    start = StartPosition();
    end = EndPosition();
    if (start.IsNull() || start.IsOrphan() || end.IsNull() || end.IsOrphan())
    {
      return;
    }

    if (splitStart && MergeStartWithPreviousIfIdentical(start, end))
    {
      start = StartPosition();
      end = EndPosition();
    }

    if (start.IsNull() || end.IsNull())
    {
      return;
    }

    if (splitEnd)
    {
      MergeEndWithNextIfIdentical(start, end);
      start = StartPosition();
      end = EndPosition();
    }

    if (start.IsNull() || end.IsNull())
    {
      return;
    }

    // update document layout once before running the rest of the function
    // so that we avoid the expense of updating before each and every call
    // to check a computed style
    GetDocument().UpdateLayoutIgnorePendingStylesheets();

    RefPtr<EditingStyle> styleToApply = ShareRefPtr(&style);
    if (textDirection)
    {
      // Avoid applying the unicode-bidi and direction properties beneath ancestors that already have them.
      RefPtr<web::Node> startNode = ShareRefPtr(start.DeprecatedNode());
      auto embeddingStartNode = HighestEmbeddingAncestor(startNode.get(), EnclosingBlock(startNode).get());
      RefPtr<web::Node> endNode = ShareRefPtr(end.DeprecatedNode());
      auto embeddingEndNode = HighestEmbeddingAncestor(endNode.get(), EnclosingBlock(endNode).get());

      if (embeddingStartNode || embeddingEndNode)
      {
        Position embeddingApplyStart =
          embeddingStartNode ? PositionInParentAfterNode(embeddingStartNode.get()) : start;
        Position embeddingApplyEnd =
          embeddingEndNode ? PositionInParentBeforeNode(embeddingEndNode.get()) : end;
        krys_debug_assert(embeddingApplyStart.IsNotNull() && embeddingApplyEnd.IsNotNull());

        if (!embeddingStyle)
        {
          styleWithoutEmbedding = style.Copy();
          embeddingStyle = styleWithoutEmbedding->ExtractAndRemoveTextDirection();
        }
        FixRangeAndApplyInlineStyle(*embeddingStyle, embeddingApplyStart, embeddingApplyEnd);

        styleToApply = styleWithoutEmbedding;
      }
    }

    FixRangeAndApplyInlineStyle(*styleToApply, start, end);

    // Remove dummy style spans created by splitting text elements.
    CleanupUnstyledStyleSpans(startDummySpanAncestor.get());
    if (endDummySpanAncestor != startDummySpanAncestor)
    {
      CleanupUnstyledStyleSpans(endDummySpanAncestor.get());
    }
  }

  void ApplyStyleCommand::FixRangeAndApplyInlineStyle(EditingStyle &style, const Position &start,
                                                      const Position &end) noexcept
  {
    RefPtr<web::Node> startNode = ShareRefPtr(start.DeprecatedNode());

    if (start.DeprecatedEditingOffset() >= CaretMaxOffset(*startNode))
    {
      startNode = ShareRefPtr(TreeQueries::Next(*startNode));
      if (!startNode || end < FirstPositionInOrBeforeNode(startNode.get()))
      {
        return;
      }
    }

    RefPtr<web::Node> pastEndNode = ShareRefPtr(end.DeprecatedNode());
    if (end.DeprecatedEditingOffset() >= CaretMaxOffset(*pastEndNode))
    {
      pastEndNode = ShareRefPtr(TreeTraversal::NextSkippingChildren(*pastEndNode));
    }

    // FIXME: Callers should perform this operation on a Range that includes the br
    // if they want style applied to the empty line.
    // FIXME: Should this be using startNode instead of start.deprecatedNode()?
    if (start == end && IsHTMLElementOfType(start.DeprecatedNode(), html::TagName::br))
    {
      pastEndNode = ShareRefPtr(TreeQueries::Next(*start.DeprecatedNode()));
    }

    // Start from the highest fully selected ancestor so that we can modify the fully selected node.
    // e.g. When applying font-size: large on <font color="blue">hello</font>, we need to include the font
    // element in our run to generate <font color="blue" size="4">hello</font> instead of <font
    // color="blue"><font size="4">hello</font></font>
    auto range = *CreateSimpleRange(PositionRange {start, end});
    RefPtr<Element> editableRoot = ShareRefPtr(startNode->RootEditableElement());
    if (startNode != editableRoot)
    {
      while (editableRoot && startNode->ParentNode() != editableRoot
             && IsNodeVisiblyContainedWithin(*startNode->ParentNode(), range))
      {
        startNode = ShareRefPtr(startNode->ParentNode());
      }
    }

    ApplyInlineStyleToNodeRange(style, *startNode, pastEndNode.get());
  }

  void ApplyStyleCommand::ApplyInlineStyleToNodeRange(EditingStyle &style, web::Node &startNode,
                                                      web::Node *pastEndNode) noexcept
  {
    if (_removeOnly)
    {
      return;
    }

    GetDocument().UpdateLayoutIgnorePendingStylesheets();

    List<InlineRunToApplyStyle> runs;
    RefPtr<web::Node> node = ShareRefPtr(&startNode);
    for (RefPtr<web::Node> next; node && node != pastEndNode; node = next)
    {
      next = ShareRefPtr(TreeQueries::Next(*node));

      if (!node->Renderer() || !node->HasEditableStyle())
      {
        continue;
      }

      if (!node->HasRichlyEditableStyle())
      {
        if (auto *element = DynamicDowncast<HTMLElement>(*node))
        {
          // This is a plaintext-only region. Only proceed if it's fully selected.
          // pastEndNode is the node after the last fully selected node, so if it's inside node then
          // node isn't fully selected.
          if (pastEndNode && pastEndNode->IsDescendantOf(*node))
          {
            break;
          }
          // Add to this element's inline style and skip over its contents.
          auto inlineStyle = CopyStyleOrCreateEmpty(element->InlineStyle());
          if (auto *otherStyle = style.Style())
          {
            (void)inlineStyle->MergeAndOverrideOnConflict(*otherStyle);
          }
          SetNodeAttribute(*element, u8"style", inlineStyle->AsText(css::DefaultSerialisationContext()));
          next = ShareRefPtr(TreeTraversal::NextSkippingChildren(*element));
          continue;
        }
      }

      if (IsBlock(*node))
      {
        continue;
      }

      if (node->HasChildNodes())
      {
        if (node->Contains(pastEndNode) || ContainsNonEditableRegion(*node)
            || !node->ParentNode()->HasEditableStyle())
        {
          continue;
        }
        if (EditingIgnoresContent(*node))
        {
          next = ShareRefPtr(TreeTraversal::NextSkippingChildren(*node));
          continue;
        }
      }

      auto runStart = node;
      auto runEnd = node;
      RefPtr<web::Node> sibling = ShareRefPtr(node->NextSibling());
      while (sibling && sibling != pastEndNode && !sibling->Contains(pastEndNode)
             && (!IsBlock(*sibling) || IsHTMLElementOfType(sibling.get(), html::TagName::br))
             && !ContainsNonEditableRegion(*sibling))
      {
        runEnd = sibling;
        sibling = ShareRefPtr(runEnd->NextSibling());
      }
      next = ShareRefPtr(TreeTraversal::NextSkippingChildren(*runEnd));

      RefPtr<web::Node> runPastEndNode = ShareRefPtr(TreeTraversal::NextSkippingChildren(*runEnd));
      if (!ShouldApplyInlineStyleToRun(style, runStart.get(), runPastEndNode.get()))
      {
        continue;
      }

      runs.emplace_back(runStart.get(), runEnd.get(), runPastEndNode.get());
    }

    for (auto &run : runs)
    {
      RemoveConflictingInlineStyleFromRun(style, run.Start, run.End, run.PastEndNode.get());
      if (run.StartAndEndAreStillInDocument())
      {
        run.PositionForStyleComputation = PositionToComputeInlineStyleChange(*run.Start, run.DummyElement);
      }
    }

    GetDocument().UpdateLayoutIgnorePendingStylesheets();

    for (auto &run : runs)
    {
      run.Change = StyleChange(&style, run.PositionForStyleComputation);
    }

    for (auto &run : runs)
    {
      if (run.DummyElement)
      {
        RemoveNode(*run.DummyElement);
      }
      if (run.StartAndEndAreStillInDocument())
      {
        ApplyInlineStyleChange(*run.Start, *run.End, run.Change, AddStyledElement::Yes);
      }
    }
  }

  bool ApplyStyleCommand::IsStyledInlineElementToRemove(Element *element) const noexcept
  {
    return (_styledInlineElement && element->LocalName() == _styledInlineElement->LocalName()
            && element->NamespaceURI() == _styledInlineElement->NamespaceURI())
           || (_isInlineElementToRemoveFunction && _isInlineElementToRemoveFunction(element));
  }

  bool ApplyStyleCommand::ShouldApplyInlineStyleToRun(EditingStyle &style, web::Node *runStart,
                                                      web::Node *pastEndNode) noexcept
  {
    krys_debug_assert(runStart);

    auto *styledInlineHTMLElement = DynamicDowncast<HTMLElement>(_styledInlineElement.get());
    for (RefPtr<web::Node> node = ShareRefPtr(runStart); node && node != pastEndNode;
         node = ShareRefPtr(TreeQueries::Next(*node)))
    {
      if (node->HasChildNodes())
      {
        continue;
      }
      // We don't consider _isInlineElementToRemoveFunction here because we never apply style when
      // _isInlineElementToRemoveFunction is specified
      if (!style.StyleIsPresentInComputedStyleOfNode(*node))
      {
        return true;
      }
      if (_styledInlineElement
          && (!styledInlineHTMLElement
              || !EnclosingElementWithTag(PositionBeforeNode(node.get()),
                                          styledInlineHTMLElement->TagQualifiedName().TagName())))
      {
        return true;
      }
    }
    return false;
  }

  void ApplyStyleCommand::RemoveConflictingInlineStyleFromRun(EditingStyle &style,
                                                              RefPtr<web::Node> &runStart,
                                                              RefPtr<web::Node> &runEnd,
                                                              web::Node *pastEndNode) noexcept
  {
    krys_debug_assert(runStart && runEnd);
    RefPtr<web::Node> next = runStart;
    for (RefPtr<web::Node> node = next; node && node->IsConnected() && node != pastEndNode; node = next)
    {
      if (EditingIgnoresContent(*node))
      {
        krys_debug_assert(!node->Contains(pastEndNode));
        next = ShareRefPtr(TreeTraversal::NextSkippingChildren(*node));
      }
      else
      {
        next = ShareRefPtr(TreeQueries::Next(*node));
      }

      auto *htmlElement = DynamicDowncast<HTMLElement>(*node);
      if (!htmlElement)
      {
        continue;
      }

      RefPtr<web::Node> previousSibling = ShareRefPtr(node->PreviousSibling());
      RefPtr<web::Node> nextSibling = ShareRefPtr(node->NextSibling());
      RefPtr<ContainerNode> parent = ShareRefPtr(node->ParentNode());
      RemoveInlineStyleFromElement(style, *htmlElement, InlineStyleRemovalMode::Always);
      if (!node->IsConnected())
      {
        // FIXME: We might need to update the start and the end of current selection here but need a test.
        if (runStart == node)
        {
          runStart = ShareRefPtr(previousSibling ? previousSibling->NextSibling() : parent->FirstChild());
        }
        if (runEnd == node)
        {
          runEnd = ShareRefPtr(nextSibling ? nextSibling->PreviousSibling() : parent->LastChild());
        }
      }
    }
  }

  bool ApplyStyleCommand::RemoveInlineStyleFromElement(EditingStyle &style, HTMLElement &element,
                                                       InlineStyleRemovalMode mode,
                                                       EditingStyle *extractedStyle) noexcept
  {
    if (!element.ParentNode() || !IsEditableNode(*element.ParentNode()))
    {
      return false;
    }

    if (IsStyledInlineElementToRemove(&element))
    {
      if (mode == InlineStyleRemovalMode::None)
      {
        return true;
      }
      if (extractedStyle)
      {
        extractedStyle->MergeInlineStyleOfElement(element,
                                                  EditingStyle::CSSPropertyOverrideMode::OverrideValues);
      }
      RemoveNodePreservingChildren(element);
      return true;
    }

    bool removed = false;
    if (RemoveImplicitlyStyledElement(style, element, mode, extractedStyle))
    {
      removed = true;
    }

    if (!element.IsConnected())
    {
      return removed;
    }

    // If the node was converted to a span, the span may still contain relevant
    // styles which must be removed (e.g. <b style='font-weight: bold'>)
    if (RemoveCSSStyle(style, element, mode, extractedStyle))
    {
      removed = true;
    }

    return removed;
  }

  void ApplyStyleCommand::ReplaceWithSpanOrRemoveIfWithoutAttributes(HTMLElement &element) noexcept
  {
    if (HasNoAttributeOrOnlyStyleAttribute(element, StyleAttributeShouldBeEmpty))
    {
      RemoveNodePreservingChildren(element);
    }
    else
    {
      KRYS_MAYBE_UNUSED auto newSpanElement = ReplaceElementWithSpanPreservingChildrenAndAttributes(element);
      krys_debug_assert(newSpanElement && newSpanElement->IsConnected());
    }
  }

  bool ApplyStyleCommand::RemoveImplicitlyStyledElement(EditingStyle &style, HTMLElement &element,
                                                        InlineStyleRemovalMode mode,
                                                        EditingStyle *extractedStyle) noexcept
  {
    if (mode == InlineStyleRemovalMode::None)
    {
      krys_debug_assert(!extractedStyle);
      return style.ConflictsWithImplicitStyleOfElement(element)
             || style.ConflictsWithImplicitStyleOfAttributes(element);
    }

    krys_debug_assert(mode == InlineStyleRemovalMode::IfNeeded || mode == InlineStyleRemovalMode::Always);
    auto shouldExtractMatchingStyle = mode == InlineStyleRemovalMode::Always
                                        ? EditingStyle::ShouldExtractMatchingStyle::Yes
                                        : EditingStyle::ShouldExtractMatchingStyle::No;
    if (style.ConflictsWithImplicitStyleOfElement(element, extractedStyle, shouldExtractMatchingStyle))
    {
      ReplaceWithSpanOrRemoveIfWithoutAttributes(element);
      return true;
    }

    // unicode-bidi and direction are pushed down separately so don't push down with other styles
    List<DOMStringAtom> attributes;
    if (!style.ExtractConflictingImplicitStyleOfAttributes(
          element,
          extractedStyle ? EditingStyle::ShouldPreserveWritingDirection::Yes
                         : EditingStyle::ShouldPreserveWritingDirection::No,
          extractedStyle, attributes, shouldExtractMatchingStyle))
    {
      return false;
    }

    for (auto &attribute : attributes)
    {
      RemoveNodeAttribute(element, attribute);
    }

    if (IsEmptyFontTag(&element) || IsSpanWithoutAttributesOrUnstyledStyleSpan(element))
    {
      RemoveNodePreservingChildren(element);
    }

    return true;
  }

  bool ApplyStyleCommand::RemoveCSSStyle(EditingStyle &style, HTMLElement &element,
                                         InlineStyleRemovalMode mode, EditingStyle *extractedStyle) noexcept
  {
    if (mode == InlineStyleRemovalMode::None)
    {
      return style.ConflictsWithInlineStyleOfElement(element);
    }

    RefPtr<css::MutableStyleProperties> newInlineStyle;
    if (!style.ConflictsWithInlineStyleOfElement(element, newInlineStyle, extractedStyle))
    {
      return false;
    }

    if (newInlineStyle->empty())
    {
      RemoveNodeAttribute(element, u8"style");
    }
    else
    {
      SetNodeAttribute(element, u8"style", newInlineStyle->AsText(css::DefaultSerialisationContext()));
    }

    if (IsSpanWithoutAttributesOrUnstyledStyleSpan(element))
    {
      RemoveNodePreservingChildren(element);
    }

    return true;
  }

  RefPtr<HTMLElement> ApplyStyleCommand::HighestAncestorWithConflictingInlineStyle(EditingStyle &style,
                                                                                   web::Node *node) noexcept
  {
    if (!node)
    {
      return nullptr;
    }

    RefPtr<HTMLElement> result;
    RefPtr<Element> unsplittableElement = UnsplittableElementForPosition(FirstPositionInOrBeforeNode(node));

    for (RefPtr<web::Node> ancestor = ShareRefPtr(node); ancestor;
         ancestor = ShareRefPtr(ancestor->ParentNode()))
    {
      auto *htmlAncestor = DynamicDowncast<HTMLElement>(*ancestor);
      if (htmlAncestor && ShouldRemoveInlineStyleFromElement(style, *htmlAncestor))
      {
        result = ShareRefPtr(htmlAncestor);
      }
      // Should stop at the editable root (cannot cross editing boundary) and
      // also stop at the unsplittable element to be consistent with other UAs
      if (ancestor == unsplittableElement)
      {
        break;
      }
    }

    return result;
  }

  void ApplyStyleCommand::ApplyInlineStyleToPushDown(web::Node &node, EditingStyle *style) noexcept
  {
    node.NodeDocument().UpdateStyleIfNeeded();

    if (!style || style->IsEmpty() || !node.Renderer() || IsHTMLElementOfType(&node, html::TagName::iframe))
    {
      return;
    }

    RefPtr<EditingStyle> newInlineStyle = ShareRefPtr(style);
    if (auto *htmlElement = DynamicDowncast<HTMLElement>(node); htmlElement && htmlElement->InlineStyle())
    {
      newInlineStyle = style->Copy();
      newInlineStyle->MergeInlineStyleOfElement(*htmlElement,
                                                EditingStyle::CSSPropertyOverrideMode::OverrideValues);
    }

    // Since addInlineStyleIfNeeded can't add styles to block-flow render objects, add style attribute
    // instead.
    // FIXME: applyInlineStyleToRange should be used here instead.
    if (node.Renderer()->IsRenderBlockFlow() || node.HasChildNodes())
    {
      if (auto *htmlElement = DynamicDowncast<HTMLElement>(node))
      {
        SetNodeAttribute(*htmlElement, u8"style",
                         newInlineStyle->Style()->AsText(css::DefaultSerialisationContext()));
        return;
      }
    }

    if (auto *textRenderer = DynamicDowncast<RenderText>(*node.Renderer());
        textRenderer && textRenderer->ContainsOnlyCollapsibleWhitespace())
    {
      return;
    }
    if (auto *lineBreak = DynamicDowncast<RenderLineBreak>(*node.Renderer());
        lineBreak && !lineBreak->Style().PreserveNewline())
    {
      return;
    }

    // We can't wrap node with the styled element here because new styled element will never be removed if we
    // did. If we modified the child pointer in pushDownInlineStyleAroundNode to point to new style element
    // then we fall into an infinite loop where we keep removing and adding styled element wrapping node.
    AddInlineStyleIfNeeded(newInlineStyle.get(), node, node, AddStyledElement::No);
  }

  void ApplyStyleCommand::PushDownInlineStyleAroundNode(EditingStyle &style, web::Node *targetNode) noexcept
  {
    auto highestAncestor = HighestAncestorWithConflictingInlineStyle(style, targetNode);
    if (!highestAncestor)
    {
      return;
    }

    // The outer loop is traversing the tree vertically from highestAncestor to targetNode
    RefPtr<web::Node> current = highestAncestor;
    // Along the way, styled elements that contain targetNode are removed and accumulated into
    // elementsToPushDown. Each child of the removed element, exclusing ancestors of targetNode, is then
    // wrapped by clones of elements in elementsToPushDown.
    List<Ref<Element>> elementsToPushDown;
    while (current && current != targetNode && current->Contains(targetNode))
    {
      List<Ref<web::Node>> currentChildren;
      for (auto *child = current->FirstChild(); child; child = child->NextSibling())
      {
        currentChildren.push_back(ShareRef(*child));
      }

      RefPtr<StyledElement> styledElement;
      if (auto *currentElement = DynamicDowncast<StyledElement>(*current);
          currentElement && IsStyledInlineElementToRemove(currentElement))
      {
        styledElement = ShareRefPtr(currentElement);
        elementsToPushDown.emplace_back(ShareRef(*currentElement));
      }

      auto styleToPushDown = EditingStyle::Create();
      if (auto *htmlElement = DynamicDowncast<HTMLElement>(*current))
      {
        RemoveInlineStyleFromElement(style, *htmlElement, InlineStyleRemovalMode::IfNeeded,
                                     styleToPushDown.get());
      }

      // The inner loop will go through children on each level
      // FIXME: we should aggregate inline child elements together so that we don't wrap each child
      // separately.
      for (auto &child : currentChildren)
      {
        if (!child->ParentNode())
        {
          continue;
        }
        if (!child->Contains(targetNode) && !elementsToPushDown.empty())
        {
          for (auto &element : elementsToPushDown)
          {
            auto wrapper = CloneElementWithoutChildren(*element);
            wrapper->RemoveAttribute(u8"style");
            SurroundNodeRangeWithElement(*child, *child, krys::move(wrapper));
          }
        }

        // Apply style to all nodes containing targetNode and their siblings but NOT to targetNode
        // But if we've removed styledElement then always apply the style.
        if (child.get() != targetNode || styledElement)
        {
          ApplyInlineStyleToPushDown(*child, styleToPushDown.get());
        }

        // We found the next node for the outer loop (contains targetNode)
        // When reached targetNode, stop the outer loop upon the completion of the current inner loop
        if (child.get() == targetNode || child->Contains(targetNode))
        {
          current = child;
        }
      }
    }
  }

  void ApplyStyleCommand::RemoveInlineStyle(EditingStyle &style, const Position &start,
                                            const Position &end) noexcept
  {
    krys_debug_assert(start.IsNotNull());
    krys_debug_assert(end.IsNotNull());
    krys_debug_assert(start.AnchorNode()->IsConnected());
    krys_debug_assert(end.AnchorNode()->IsConnected());
    krys_debug_assert(std::is_lteq(TreeOrder<dom::TreeType::ShadowIncludingTree>(start, end)));
    // FIXME: We should assert that start/end are not in the middle of a text node.

    Position pushDownStart = start.Downstream();
    // If the pushDownStart is at the end of a text node, then this node is not fully selected.
    // Move it to the next deep quivalent position to avoid removing the style from this node.
    // e.g. if pushDownStart was at Position("hello", 5) in <b>hello<div>world</div></b>, we want
    // Position("world", 0) instead.
    if (auto *text = DynamicDowncast<Text>(pushDownStart.GetContainerNode());
        text && static_cast<size_t>(pushDownStart.ComputeOffsetInContainerNode()) == text->Length())
    {
      pushDownStart = NextVisuallyDistinctCandidate(pushDownStart);
    }
    // If pushDownEnd is at the start of a text node, then this node is not fully selected.
    // Move it to the previous deep equivalent position to avoid removing the style from this node.
    Position pushDownEnd = end.Upstream();
    if (Is<Text>(pushDownEnd.GetContainerNode()) && !pushDownEnd.ComputeOffsetInContainerNode())
    {
      pushDownEnd = PreviousVisuallyDistinctCandidate(pushDownEnd);
    }

    PushDownInlineStyleAroundNode(style, pushDownStart.DeprecatedNode());
    PushDownInlineStyleAroundNode(style, pushDownEnd.DeprecatedNode());

    // The s and e variables store the positions used to set the ending selection after style removal
    // takes place. This will help callers to recognize when either the start node or the end node
    // are removed from the document during the work of this function.
    // If pushDownInlineStyleAroundNode has pruned start.deprecatedNode() or end.deprecatedNode(),
    // use pushDownStart or pushDownEnd instead, which pushDownInlineStyleAroundNode won't prune.
    Position s = start.IsNull() || start.IsOrphan() ? pushDownStart : start;
    Position e = end.IsNull() || end.IsOrphan() ? pushDownEnd : end;

    RefPtr<web::Node> node = ShareRefPtr(start.DeprecatedNode());
    while (node)
    {
      RefPtr<web::Node> next;
      if (EditingIgnoresContent(*node))
      {
        krys_debug_assert(node == end.DeprecatedNode() || !node->Contains(end.DeprecatedNode()));
        next = ShareRefPtr(TreeTraversal::NextSkippingChildren(*node));
      }
      else
      {
        next = ShareRefPtr(TreeQueries::Next(*node));
      }

      if (RefPtr<HTMLElement> element = DynamicDowncast<HTMLElement>(node);
          element && NodeFullySelected(*element, start, end))
      {
        RefPtr<web::Node> prev = ShareRefPtr(TreeTraversal::PreviousPostOrder(*element));
        RefPtr<web::Node> nextAfterElement = ShareRefPtr(TreeQueries::Next(*element));
        RefPtr<EditingStyle> styleToPushDown;
        RefPtr<web::Node> childNode;
        if (IsStyledInlineElementToRemove(element.get()))
        {
          styleToPushDown = EditingStyle::Create();
          childNode = ShareRefPtr(element->FirstChild());
        }

        RemoveInlineStyleFromElement(style, *element, InlineStyleRemovalMode::IfNeeded,
                                     styleToPushDown.get());
        if (!element->IsConnected())
        {
          if (s.DeprecatedNode() == element.get())
          {
            // Since elem must have been fully selected, and it is at the start
            // of the selection, it is clear we can set the new s offset to 0.
            krys_debug_assert(s.GetAnchorType() == Position::AnchorType::PositionIsBeforeAnchor
                              || s.GetAnchorType() == Position::AnchorType::PositionIsBeforeChildren
                              || s.OffsetInContainerNode() <= 0);
            s = FirstPositionInOrBeforeNode(nextAfterElement.get());
          }
          if (e.DeprecatedNode() == element.get())
          {
            // Since elem must have been fully selected, and it is at the end
            // of the selection, it is clear we can set the new e offset to
            // the max range offset of prev.
            e = LastPositionInOrAfterNode(prev.get());
          }
        }

        if (styleToPushDown)
        {
          for (; childNode; childNode = ShareRefPtr(childNode->NextSibling()))
          {
            ApplyInlineStyleToPushDown(*childNode, styleToPushDown.get());
          }
        }
      }
      if (node == end.DeprecatedNode())
      {
        break;
      }
      node = next;
    }

    UpdateStartEnd(s, e);
  }

  bool ApplyStyleCommand::NodeFullySelected(Element &element, const Position &start,
                                            const Position &end) noexcept
  {
    // The tree may have changed and Position::upstream() relies on an up-to-date layout.
    element.NodeDocument().UpdateLayoutIgnorePendingStylesheets();
    return FirstPositionInOrBeforeNode(&element) >= start
           && LastPositionInOrAfterNode(&element).Upstream() <= end;
  }

  void ApplyStyleCommand::SplitTextAtStart(const Position &start, const Position &end) noexcept
  {
    krys_debug_assert(Is<Text>(start.GetContainerNode()));

    Position newEnd;
    if (end.GetAnchorType() == Position::AnchorType::PositionIsOffsetInAnchor
        && start.GetContainerNode() == end.GetContainerNode())
    {
      newEnd = Position(ShareRefPtr(end.ContainerText()),
                        static_cast<uint32>(end.OffsetInContainerNode() - start.OffsetInContainerNode()));
    }
    else
    {
      newEnd = end;
    }

    RefPtr<Text> text = ShareRefPtr(start.ContainerText());
    SplitTextNode(*text, static_cast<uint32>(start.OffsetInContainerNode()));
    UpdateStartEnd(FirstPositionInNode(text.get()), newEnd);
  }

  void ApplyStyleCommand::SplitTextAtEnd(const Position &start, const Position &end) noexcept
  {
    krys_debug_assert(Is<Text>(end.GetContainerNode()));

    bool shouldUpdateStart = start.GetAnchorType() == Position::AnchorType::PositionIsOffsetInAnchor
                             && start.GetContainerNode() == end.GetContainerNode();
    Ref<Text> text = ShareRef(Downcast<Text>(*end.DeprecatedNode()));
    SplitTextNode(*text, static_cast<uint32>(end.OffsetInContainerNode()));

    RefPtr<Text> prevNode = ShareRefPtr(DynamicDowncast<Text>(text->PreviousSibling()));
    if (!prevNode)
    {
      return;
    }

    Position newStart =
      shouldUpdateStart ? Position(RefPtr<Text>(prevNode), static_cast<uint32>(start.OffsetInContainerNode()))
                        : start;
    UpdateStartEnd(newStart, LastPositionInNode(prevNode.get()));
  }

  void ApplyStyleCommand::SplitTextElementAtStart(const Position &start, const Position &end) noexcept
  {
    krys_debug_assert(Is<Text>(start.GetContainerNode()));

    Position newEnd;
    if (start.GetContainerNode() == end.GetContainerNode())
    {
      newEnd = Position(ShareRefPtr(end.ContainerText()),
                        static_cast<uint32>(end.OffsetInContainerNode() - start.OffsetInContainerNode()));
    }
    else
    {
      newEnd = end;
    }

    RefPtr<web::Node> container = ShareRefPtr(start.GetContainerNode());
    SplitTextNodeContainingElement(*start.ContainerText(),
                                   static_cast<uint32>(start.OffsetInContainerNode()));
    UpdateStartEnd(PositionBeforeNode(container.get()), newEnd);
  }

  void ApplyStyleCommand::SplitTextElementAtEnd(const Position &start, const Position &end) noexcept
  {
    krys_debug_assert(Is<Text>(end.GetContainerNode()));

    bool shouldUpdateStart = start.GetContainerNode() == end.GetContainerNode();
    SplitTextNodeContainingElement(*end.ContainerText(), static_cast<uint32>(end.OffsetInContainerNode()));

    RefPtr<web::Node> parentElement = ShareRefPtr(end.GetContainerNode()->ParentNode());
    if (!parentElement || !parentElement->PreviousSibling())
    {
      return;
    }
    RefPtr<Text> firstTextNode =
      ShareRefPtr(DynamicDowncast<Text>(parentElement->PreviousSibling()->LastChild()));
    if (!firstTextNode)
    {
      return;
    }

    Position newStart = shouldUpdateStart ? Position(RefPtr<Text>(firstTextNode),
                                                     static_cast<uint32>(start.OffsetInContainerNode()))
                                          : start;
    UpdateStartEnd(newStart, PositionAfterNode(firstTextNode.get()));
  }

  bool ApplyStyleCommand::ShouldSplitTextElement(Element *element, EditingStyle &style) noexcept
  {
    auto *htmlElement = DynamicDowncast<HTMLElement>(element);
    return htmlElement && ShouldRemoveInlineStyleFromElement(style, *htmlElement);
  }

  bool ApplyStyleCommand::IsValidCaretPositionInTextNode(const Position &position) noexcept
  {
    krys_debug_assert(position.IsNotNull());

    auto *node = position.GetContainerNode();
    if (position.GetAnchorType() != Position::AnchorType::PositionIsOffsetInAnchor || !Is<Text>(node))
    {
      return false;
    }
    int32 offsetInText = position.OffsetInContainerNode();
    return offsetInText > CaretMinOffset(*node) && offsetInText < CaretMaxOffset(*node);
  }

  bool ApplyStyleCommand::MergeStartWithPreviousIfIdentical(const Position &start,
                                                            const Position &end) noexcept
  {
    RefPtr<web::Node> startNode = ShareRefPtr(start.GetContainerNode());
    if (start.ComputeOffsetInContainerNode())
    {
      return false;
    }

    if (IsAtomicNode(startNode.get()))
    {
      // note: prior siblings could be unrendered elements. it's silly to miss the
      // merge opportunity just for that.
      if (startNode->PreviousSibling())
      {
        return false;
      }

      startNode = ShareRefPtr(startNode->ParentNode());
    }

    RefPtr<Element> element = DynamicDowncast<Element>(startNode);
    if (!element)
    {
      return false;
    }

    RefPtr<web::Node> previousSibling = ShareRefPtr(startNode->PreviousSibling());
    if (!previousSibling)
    {
      return false;
    }
    RefPtr<Element> previousElement = ShareRefPtr(ElementIfEquivalent(*element, *previousSibling));
    if (!previousElement)
    {
      return false;
    }

    RefPtr<web::Node> startChild = ShareRefPtr(element->FirstChild());
    krys_debug_assert(startChild);
    MergeIdenticalElements(*previousElement, *element);

    // FIXME: Inconsistent that we use computeOffsetInContainerNode for start, but deprecatedEditingOffset for
    // end.
    uint32 startOffset = startChild->ComputeNodeIndex();
    uint32 endOffset = static_cast<uint32>(end.DeprecatedEditingOffset())
                       + (startNode == end.DeprecatedNode() ? startOffset : 0);
    UpdateStartEnd(
      Position(RefPtr<web::Node>(startNode), startOffset, Position::AnchorType::PositionIsOffsetInAnchor),
      Position(ShareRefPtr(end.DeprecatedNode()), endOffset, Position::AnchorType::PositionIsOffsetInAnchor));
    return true;
  }

  bool ApplyStyleCommand::MergeEndWithNextIfIdentical(const Position &start, const Position &end) noexcept
  {
    RefPtr<web::Node> endNode = ShareRefPtr(end.GetContainerNode());

    if (IsAtomicNode(endNode.get()))
    {
      int32 endOffset = end.ComputeOffsetInContainerNode();
      if (OffsetIsBeforeLastNodeOffset(static_cast<uint32>(endOffset), endNode.get())
          || end.DeprecatedNode()->NextSibling())
      {
        return false;
      }

      endNode = ShareRefPtr(end.DeprecatedNode()->ParentNode());
    }

    if (IsHTMLElementOfType(endNode.get(), html::TagName::br))
    {
      return false;
    }

    RefPtr<Element> element = DynamicDowncast<Element>(endNode);
    if (!element)
    {
      return false;
    }

    RefPtr<web::Node> nextSibling = ShareRefPtr(endNode->NextSibling());
    if (!nextSibling)
    {
      return false;
    }
    RefPtr<Element> nextElement = ShareRefPtr(ElementIfEquivalent(*element, *nextSibling));
    if (!nextElement)
    {
      return false;
    }

    RefPtr<web::Node> nextChild = ShareRefPtr(nextElement->FirstChild());

    MergeIdenticalElements(*element, *nextElement);

    bool shouldUpdateStart = start.GetContainerNode() == endNode;
    uint32 endOffset = nextChild ? nextChild->ComputeNodeIndex() : nextElement->CountChildNodes();
    UpdateStartEnd(shouldUpdateStart ? Position(RefPtr<web::Node>(nextElement),
                                                static_cast<uint32>(start.OffsetInContainerNode()),
                                                Position::AnchorType::PositionIsOffsetInAnchor)
                                     : start,
                   Position(nextElement, endOffset, Position::AnchorType::PositionIsOffsetInAnchor));
    return true;
  }

  bool ApplyStyleCommand::SurroundNodeRangeWithElement(web::Node &startNode, web::Node &endNode,
                                                       Ref<Element> &&elementToInsert) noexcept
  {
    Ref<web::Node> protectedStartNode = ShareRef(startNode);
    Ref<Element> element = krys::move(elementToInsert);

    if (!InsertNodeBefore(*element, startNode) || !element->IsContentRichlyEditable())
    {
      RemoveNode(*element);
      return false;
    }

    RefPtr<web::Node> node = ShareRefPtr(&startNode);
    while (node)
    {
      RefPtr<web::Node> next = ShareRefPtr(node->NextSibling());
      if (IsEditableNode(*node))
      {
        RemoveNode(*node);
        AppendNode(*node, *element);
      }
      if (node == &endNode)
      {
        break;
      }
      node = next;
    }

    RefPtr<web::Node> nextSibling = ShareRefPtr(element->NextSibling());
    RefPtr<web::Node> previousSibling = ShareRefPtr(element->PreviousSibling());

    if (nextSibling && nextSibling->HasEditableStyle())
    {
      if (RefPtr<Element> nextElement = ShareRefPtr(ElementIfEquivalent(*element, *nextSibling)))
      {
        MergeIdenticalElements(*element, *nextElement);
      }
    }

    if (auto *previousSiblingElement = DynamicDowncast<Element>(previousSibling.get());
        previousSiblingElement && previousSiblingElement->HasEditableStyle())
    {
      RefPtr<web::Node> mergedElementNode = ShareRefPtr(previousSibling->NextSibling());
      krys_debug_assert(mergedElementNode);
      if (mergedElementNode->HasEditableStyle())
      {
        if (RefPtr<Element> mergedElement =
              ShareRefPtr(ElementIfEquivalent(*previousSiblingElement, *mergedElementNode)))
        {
          MergeIdenticalElements(*previousSiblingElement, *mergedElement);
        }
      }
    }

    // FIXME: We should probably call updateStartEnd if the start or end was in the node
    // range so that the endingSelection() is canonicalized.  See the comments at the end of
    // VisibleSelection::validate().
    return true;
  }

  void ApplyStyleCommand::AddBlockStyle(const StyleChange &styleChange, HTMLElement &block) noexcept
  {
    // Do not check for legacy styles here. Those styles, like <B> and <I>, only apply for inline content.
    krys_debug_assert(styleChange.CssStyle());
    SetNodeAttribute(block, u8"style",
                     JoinWithSpace(styleChange.CssStyle()->AsText(css::DefaultSerialisationContext()),
                                   block.GetAttribute(u8"style").value_or(DOMString())));
  }

  void ApplyStyleCommand::AddInlineStyleIfNeeded(EditingStyle *style, web::Node &start, web::Node &end,
                                                 AddStyledElement addStyledElement) noexcept
  {
    if (!start.IsConnected() || !end.IsConnected())
    {
      return;
    }

    Ref<web::Node> protectedStart = ShareRef(start);
    RefPtr<web::Node> dummyElement;
    StyleChange styleChange(style, PositionToComputeInlineStyleChange(start, dummyElement));

    if (dummyElement)
    {
      RemoveNode(*dummyElement);
    }

    ApplyInlineStyleChange(start, end, styleChange, addStyledElement);
  }

  Position ApplyStyleCommand::PositionToComputeInlineStyleChange(web::Node &startNode,
                                                                 RefPtr<web::Node> &dummyElement) noexcept
  {
    // It's okay to obtain the style at the startNode because we've removed all relevant styles from the
    // current run.
    if (!Is<Element>(startNode))
    {
      dummyElement = CreateStyleSpanElement(GetDocument());
      InsertNodeAt(*dummyElement, PositionBeforeNode(&startNode));
      return FirstPositionInOrBeforeNode(dummyElement.get());
    }

    return FirstPositionInOrBeforeNode(&startNode);
  }

  void ApplyStyleCommand::ApplyInlineStyleChange(web::Node &passedStart, web::Node &passedEnd,
                                                 StyleChange &styleChange,
                                                 AddStyledElement addStyledElement) noexcept
  {
    RefPtr<web::Node> startNode = ShareRefPtr(&passedStart);
    RefPtr<web::Node> endNode = ShareRefPtr(&passedEnd);
    krys_debug_assert(startNode->IsConnected());
    krys_debug_assert(endNode->IsConnected());

    // Find appropriate font and span elements top-down.
    RefPtr<HTMLFontElement> fontContainer;
    RefPtr<HTMLElement> styleContainer;
    while (startNode == endNode)
    {
      if (auto *container = DynamicDowncast<HTMLElement>(*startNode))
      {
        if (auto *fontElement = DynamicDowncast<HTMLFontElement>(*container))
        {
          fontContainer = ShareRefPtr(fontElement);
        }
        if (Is<HTMLSpanElement>(*container)
            || (!Is<HTMLSpanElement>(styleContainer.get()) && container->HasChildNodes()))
        {
          styleContainer = ShareRefPtr(container);
        }
        if (!CanHaveChildrenForEditing(*startNode))
        {
          break;
        }
      }
      RefPtr<web::Node> startNodeFirstChild = ShareRefPtr(startNode->FirstChild());
      if (!startNodeFirstChild)
      {
        break;
      }
      endNode = ShareRefPtr(startNode->LastChild());
      startNode = startNodeFirstChild;
    }

    const DOMStringAtom colorAttr(u8"color");
    const DOMStringAtom faceAttr(u8"face");
    const DOMStringAtom sizeAttr(u8"size");

    // Font tags need to go outside of CSS so that CSS font sizes override leagcy font sizes.
    if (styleChange.ApplyFontColor() || styleChange.ApplyFontFace() || styleChange.ApplyFontSize())
    {
      if (fontContainer)
      {
        if (styleChange.ApplyFontColor())
        {
          SetNodeAttribute(*fontContainer, colorAttr, styleChange.FontColor());
        }
        if (styleChange.ApplyFontFace())
        {
          SetNodeAttribute(*fontContainer, faceAttr, styleChange.FontFace());
        }
        if (styleChange.ApplyFontSize())
        {
          SetNodeAttribute(*fontContainer, sizeAttr, styleChange.FontSize());
        }
      }
      else
      {
        auto fontElement = CreateFontElement(GetDocument());
        if (styleChange.ApplyFontColor())
        {
          (void)fontElement->SetAttribute(colorAttr, styleChange.FontColor());
        }
        if (styleChange.ApplyFontFace())
        {
          (void)fontElement->SetAttribute(faceAttr, styleChange.FontFace());
        }
        if (styleChange.ApplyFontSize())
        {
          (void)fontElement->SetAttribute(sizeAttr, styleChange.FontSize());
        }
        SurroundNodeRangeWithElement(*startNode, *endNode, krys::move(fontElement));
      }
    }

    if (const auto *styleToMerge = styleChange.CssStyle())
    {
      if (styleContainer)
      {
        if (const auto *existingStyle = styleContainer->InlineStyle())
        {
          auto inlineStyle = EditingStyle::Create(existingStyle);
          inlineStyle->OverrideWithStyle(*styleToMerge);
          SetNodeAttribute(*styleContainer, u8"style",
                           inlineStyle->Style()->AsText(css::DefaultSerialisationContext()));
        }
        else
        {
          SetNodeAttribute(*styleContainer, u8"style",
                           styleToMerge->AsText(css::DefaultSerialisationContext()));
        }
      }
      else
      {
        auto styleElement = CreateStyleSpanElement(GetDocument());
        (void)styleElement->SetAttribute(u8"style", styleToMerge->AsText(css::DefaultSerialisationContext()));
        SurroundNodeRangeWithElement(*startNode, *endNode, krys::move(styleElement));
      }
    }

    if (styleChange.ApplyBold())
    {
      SurroundNodeRangeWithElement(*startNode, *endNode, CreateHTMLElement(GetDocument(), u8"b"));
    }

    if (styleChange.ApplyItalic())
    {
      SurroundNodeRangeWithElement(*startNode, *endNode, CreateHTMLElement(GetDocument(), u8"i"));
    }

    if (styleChange.ApplyUnderline())
    {
      SurroundNodeRangeWithElement(*startNode, *endNode, CreateHTMLElement(GetDocument(), u8"u"));
    }

    if (styleChange.ApplyLineThrough())
    {
      SurroundNodeRangeWithElement(*startNode, *endNode, CreateHTMLElement(GetDocument(), u8"strike"));
    }

    if (styleChange.ApplySubscript())
    {
      SurroundNodeRangeWithElement(*startNode, *endNode, CreateHTMLElement(GetDocument(), u8"sub"));
    }
    else if (styleChange.ApplySuperscript())
    {
      SurroundNodeRangeWithElement(*startNode, *endNode, CreateHTMLElement(GetDocument(), u8"sup"));
    }

    if (_styledInlineElement && addStyledElement == AddStyledElement::Yes)
    {
      SurroundNodeRangeWithElement(*startNode, *endNode, CloneElementWithoutChildren(*_styledInlineElement));
    }
  }
}
