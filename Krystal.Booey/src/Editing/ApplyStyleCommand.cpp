#include "Krystal.Booey/Editing/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Constants/Attributes.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/EditingStyle.hpp"
#include "Krystal.Booey/Editing/HTMLInterchange.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/Editing/VisibleSelection.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLFontElement.hpp"
#include "Krystal.Booey/HTML/HTMLSpanElement.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Extractor.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include <algorithm>
#include <utility>

namespace krys::boo
{
  using css::PropertyId;
  using css::ValueId;
  using dom::TreeQueries;

  namespace
  {
    KRYS_NODISCARD bool
      HasNoAttributeOrOnlyStyleAttribute(const dom::StyledElement &element,
                                         ShouldStyleAttributeBeEmpty shouldStyleAttributeBeEmpty) noexcept
    {
      if (!element.HasAttributes())
      {
        return true;
      }

      uint32 matchedAttributes = 0;
      if (element.GetAttribute(dom::DOMStringAtom(Attributes::Class)) == dom::DOMString(AppleStyleSpanClass))
      {
        matchedAttributes++;
      }
      if (element.HasAttribute(dom::DOMStringAtom(Attributes::Style))
          && (shouldStyleAttributeBeEmpty == AllowNonEmptyStyleAttribute || !element.InlineStyle()
              || element.InlineStyle()->empty()))
      {
        matchedAttributes++;
      }

      auto attributeCount = static_cast<uint32>(element.AttributeList().size());
      krys_debug_assert(matchedAttributes <= attributeCount);
      return matchedAttributes == attributeCount;
    }

    KRYS_NODISCARD bool IsSpanWithoutAttributesOrUnstyledStyleSpan(const dom::Element &element) noexcept
    {
      return IsHTMLElementOfType(&element, html::TagName::span)
             && HasNoAttributeOrOnlyStyleAttribute(Downcast<dom::StyledElement>(element),
                                                   StyleAttributeShouldBeEmpty);
    }

    KRYS_NODISCARD Ref<dom::Element> CreateFontElement(dom::Document &document) noexcept
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

    KRYS_NODISCARD ValueId ComputedUnicodeBidi(dom::Node *node) noexcept
    {
      auto value = style::Extractor(node).PropertyValue(PropertyId::UnicodeBidi);
      return value ? css::LonghandValueId(PropertyId::UnicodeBidi, *value) : ValueId::Invalid;
    }

    KRYS_NODISCARD dom::ContainerNode *DummySpanAncestorForNode(dom::Node *node) noexcept
    {
      RefPtr<dom::Node> currentNode = ShareRefPtr(node);
      while (currentNode)
      {
        auto *element = DynamicDowncast<dom::Element>(*currentNode);
        if (element && IsStyleSpanOrSpanWithOnlyStyleAttribute(*element))
        {
          break;
        }
        currentNode = ShareRefPtr(currentNode->ParentNode());
      }
      return currentNode ? currentNode->ParentNode() : nullptr;
    }

    KRYS_NODISCARD RefPtr<dom::Node> HighestEmbeddingAncestor(dom::Node *startNode,
                                                              dom::Node *enclosingNode) noexcept
    {
      for (RefPtr<dom::Node> currentNode = ShareRefPtr(startNode);
           currentNode && currentNode != enclosingNode; currentNode = ShareRefPtr(currentNode->ParentNode()))
      {
        if (Is<html::HTMLElement>(*currentNode) && ComputedUnicodeBidi(currentNode.get()) == ValueId::Embed)
        {
          return currentNode;
        }
      }

      return nullptr;
    }

    KRYS_NODISCARD bool ContainsNonEditableRegion(dom::Node &node) noexcept
    {
      if (!node.HasEditableStyle())
      {
        return true;
      }

      RefPtr<dom::Node> sibling = ShareRefPtr(TreeQueries::NextSkippingChildren(node));
      for (RefPtr<dom::Node> descendant = ShareRefPtr(node.FirstChild()); descendant && descendant != sibling;
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
      RefPtr<dom::Node> Start;
      RefPtr<dom::Node> End;
      RefPtr<dom::Node> PastEndNode;
      dom::Position PositionForStyleComputation;
      RefPtr<dom::Node> DummyElement;
      StyleChange Change;

      InlineRunToApplyStyle(dom::Node *start, dom::Node *end, dom::Node *pastEndNode) noexcept
          : Start(ShareRefPtr(start)), End(ShareRefPtr(end)), PastEndNode(ShareRefPtr(pastEndNode))
      {
        krys_debug_assert(start->ParentNode() == end->ParentNode());
      }

      KRYS_NODISCARD bool StartAndEndAreStillInDocument() const noexcept
      {
        return Start && End && Start->IsConnected() && End->IsConnected();
      }
    };

    KRYS_NODISCARD dom::DOMString JoinWithSpace(const dom::DOMString &a, const dom::DOMString &b) noexcept
    {
      if (a.empty())
      {
        return b;
      }
      if (b.empty())
      {
        return a;
      }
      dom::DOMString result;
      result.reserve(a.size() + 1 + b.size());
      result.append(a);
      result.push_back(u8' ');
      result.append(b);
      return result;
    }
  }

  bool IsLegacyAppleStyleSpan(const dom::Node *node) noexcept
  {
    return IsHTMLElementOfType(node, html::TagName::span)
           && Downcast<dom::Element>(*node).GetAttribute(dom::DOMStringAtom(Attributes::Class))
                == dom::DOMString(AppleStyleSpanClass);
  }

  bool IsStyleSpanOrSpanWithOnlyStyleAttribute(const dom::Element &element) noexcept
  {
    return IsHTMLElementOfType(&element, html::TagName::span)
           && HasNoAttributeOrOnlyStyleAttribute(Downcast<dom::StyledElement>(element),
                                                 AllowNonEmptyStyleAttribute);
  }

  bool IsEmptyFontTag(const dom::Element *element,
                      ShouldStyleAttributeBeEmpty shouldStyleAttributeBeEmpty) noexcept
  {
    return IsHTMLElementOfType(element, html::TagName::font)
           && HasNoAttributeOrOnlyStyleAttribute(Downcast<dom::StyledElement>(*element),
                                                 shouldStyleAttributeBeEmpty);
  }

  Ref<dom::Element> CreateStyleSpanElement(dom::Document &document) noexcept
  {
    return CreateHTMLElement(document, u8"span");
  }

  ApplyStyleCommand::ApplyStyleCommand(dom::Document &document, const EditingStyle *style, EditAction action,
                                       ApplyStylePropertyLevel level) noexcept
      : CompositeEditCommand(document, action), _style(style->Copy()), _propertyLevel(level),
        _start(EndingSelection().Start().Downstream()), _end(EndingSelection().End().Upstream()),
        _useEndingSelection(true), _removeOnly(false)
  {
  }

  ApplyStyleCommand::ApplyStyleCommand(dom::Document &document, const EditingStyle *style,
                                       const dom::Position &start, const dom::Position &end,
                                       EditAction action, ApplyStylePropertyLevel level) noexcept
      : CompositeEditCommand(document, action), _style(style->Copy()), _propertyLevel(level), _start(start),
        _end(end), _useEndingSelection(false), _removeOnly(false)
  {
  }

  ApplyStyleCommand::ApplyStyleCommand(dom::Element &element, bool removeOnly, EditAction action) noexcept
      : CompositeEditCommand(element.NodeDocument(), action), _style(EditingStyle::Create()),
        _start(EndingSelection().Start().Downstream()), _end(EndingSelection().End().Upstream()),
        _useEndingSelection(true), _styledInlineElement(ShareRef(element)), _removeOnly(removeOnly)
  {
  }

  ApplyStyleCommand::ApplyStyleCommand(dom::Document &document, const EditingStyle *style,
                                       IsInlineElementToRemoveFunction isInlineElementToRemove,
                                       EditAction action) noexcept
      : CompositeEditCommand(document, action), _style(style->Copy()),
        _start(EndingSelection().Start().Downstream()), _end(EndingSelection().End().Upstream()),
        _useEndingSelection(true), _removeOnly(true),
        _isInlineElementToRemoveFunction(isInlineElementToRemove)
  {
  }

  Ref<ApplyStyleCommand> ApplyStyleCommand::Create(dom::Document &document, const EditingStyle *style,
                                                   EditAction action, ApplyStylePropertyLevel level) noexcept
  {
    return CreateRef<ApplyStyleCommand>(document, style, action, level);
  }

  Ref<ApplyStyleCommand> ApplyStyleCommand::Create(dom::Document &document, const EditingStyle *style,
                                                   const dom::Position &start, const dom::Position &end,
                                                   EditAction action, ApplyStylePropertyLevel level) noexcept
  {
    return CreateRef<ApplyStyleCommand>(document, style, start, end, action, level);
  }

  Ref<ApplyStyleCommand> ApplyStyleCommand::Create(dom::Element &element, bool removeOnly,
                                                   EditAction action) noexcept
  {
    return CreateRef<ApplyStyleCommand>(element, removeOnly, action);
  }

  Ref<ApplyStyleCommand>
    ApplyStyleCommand::Create(dom::Document &document, const EditingStyle *style,
                              IsInlineElementToRemoveFunction isInlineElementToRemoveFunction,
                              EditAction action) noexcept
  {
    return CreateRef<ApplyStyleCommand>(document, style, isInlineElementToRemoveFunction, action);
  }

  void ApplyStyleCommand::UpdateStartEnd(const dom::Position &newStart, const dom::Position &newEnd) noexcept
  {
    krys_debug_assert(!std::is_gt(dom::TreeOrder<dom::TreeType::ShadowIncludingTree>(newStart, newEnd)));

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

  dom::Position ApplyStyleCommand::StartPosition() noexcept
  {
    if (_useEndingSelection)
    {
      return EndingSelection().Start();
    }

    return _start;
  }

  dom::Position ApplyStyleCommand::EndPosition() noexcept
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
          ApplyRelativeFontStyleChange(_style.get());
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
    RefPtr<dom::ContainerNode> scopeRoot = HighestEditableRoot(visibleStart.DeepEquivalent());
    if (!scopeRoot)
    {
      return;
    }

    auto scope = dom::MakeRangeSelectingNodeContents(*scopeRoot);
    auto range = *MakeSimpleRange(VisiblePositionRange {visibleStart, visibleEnd});
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
        RefPtr<dom::Node> block =
          EnclosingBlock(ShareRefPtr(paragraphStart.DeepEquivalent().DeprecatedNode()));
        if (!_removeOnly)
        {
          RefPtr<dom::Node> newBlock =
            MoveParagraphContentsToNewBlockIfNecessary(paragraphStart.DeepEquivalent());
          if (newBlock)
          {
            block = newBlock;
          }
        }
        krys_debug_assert(!block || Is<dom::Element>(*block));
        if (RefPtr<html::HTMLElement> htmlBlock =
              ShareRefPtr(DynamicDowncast<html::HTMLElement>(block.get())))
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
      dom::MakeDeprecatedLegacyPosition(ResolveCharacterLocation(scope, startIndex, behaviors));
    auto endPosition =
      dom::MakeDeprecatedLegacyPosition(ResolveCharacterLocation(scope, endIndex, behaviors));
    UpdateStartEnd(startPosition, endPosition);
  }

  void ApplyStyleCommand::ApplyRelativeFontStyleChange(EditingStyle *style) noexcept
  {
    static constexpr float MinimumFontSize = 0.1f;

    if (!style || !style->HasFontSizeDelta())
    {
      return;
    }

    auto start = StartPosition();
    auto end = EndPosition();
    if (end < start)
    {
      std::swap(start, end);
    }

    if (start.IsNull() || end.IsNull()
        || &TreeQueries::Root(*start.DeprecatedNode()) != &TreeQueries::Root(*end.DeprecatedNode()))
    {
      return;
    }

    // Join up any adjacent text nodes.
    if (Is<dom::Text>(start.DeprecatedNode()))
    {
      JoinChildTextNodes(start.DeprecatedNode()->ParentNode(), start, end);
      start = StartPosition();
      end = EndPosition();
    }

    if (start.IsNull() || end.IsNull())
    {
      return;
    }

    if (Is<dom::Text>(*end.DeprecatedNode())
        && start.DeprecatedNode()->ParentNode() != end.DeprecatedNode()->ParentNode())
    {
      JoinChildTextNodes(end.DeprecatedNode()->ParentNode(), start, end);
      start = StartPosition();
      end = EndPosition();
    }

    if (start.IsNull() || end.IsNull())
    {
      return;
    }

    // Split the start text nodes if needed to apply style.
    if (IsValidCaretPositionInTextNode(start))
    {
      SplitTextAtStart(start, end);
      start = StartPosition();
      end = EndPosition();
    }

    if (start.IsNull() || end.IsNull())
    {
      return;
    }

    if (IsValidCaretPositionInTextNode(end))
    {
      SplitTextAtEnd(start, end);
      start = StartPosition();
      end = EndPosition();
    }

    if (start.IsNull() || end.IsNull())
    {
      return;
    }

    // Calculate loop end point.
    // If the end node is before the start node (can only happen if the end node is
    // an ancestor of the start node), we gather nodes up to the next sibling of the end node.
    RefPtr<dom::Node> beyondEnd;
    krys_debug_assert(start.DeprecatedNode());
    krys_debug_assert(end.DeprecatedNode());
    if (end.DeprecatedNode()->Contains(start.DeprecatedNode()))
    {
      beyondEnd = ShareRefPtr(TreeQueries::NextSkippingChildren(*end.DeprecatedNode()));
    }
    else
    {
      beyondEnd = ShareRefPtr(TreeQueries::Next(*end.DeprecatedNode()));
    }

    start = start.Upstream(); // Move upstream to ensure we do not add redundant spans.
    RefPtr<dom::Node> startNode = ShareRefPtr(start.DeprecatedNode());

    if (!startNode)
    {
      return;
    }

    // Ensure the startNode is not at or past the beyondEnd when node traversal
    // is performed in the following loops below.
    if (beyondEnd && std::is_gteq(dom::TreeOrder(*startNode, *beyondEnd)))
    {
      return;
    }

    if (Is<dom::Text>(*startNode) && start.DeprecatedEditingOffset() >= CaretMaxOffset(*startNode))
    {
      // Move out of text node if range does not include its characters.
      startNode = ShareRefPtr(TreeQueries::Next(*startNode));
      if (!startNode)
      {
        return;
      }
    }

    // Store away font size before making any changes to the document.
    // This ensures that changes to one node won't effect another.
    HashMap<const dom::Node *, float> startingFontSizes;
    List<Ref<dom::Node>> protectedNodes;
    for (auto node = startNode; node != beyondEnd; node = ShareRefPtr(TreeQueries::Next(*node)))
    {
      krys_assert(node);
      startingFontSizes[node.get()] = ComputedFontSize(node.get());
      protectedNodes.push_back(Ref<dom::Node>(node));
    }

    // These spans were added by us. If empty after font size changes, they can be removed.
    List<Ref<dom::Element>> unstyledSpans;

    RefPtr<dom::Node> lastStyledNode;
    bool reachedEnd = false;
    for (auto node = startNode; node != beyondEnd && !reachedEnd;
         node = ShareRefPtr(TreeQueries::Next(*node)))
    {
      krys_assert(node);

      RefPtr<html::HTMLElement> element;
      if (auto *htmlElement = DynamicDowncast<html::HTMLElement>(*node))
      {
        // Only work on fully selected nodes.
        if (!NodeFullySelected(*htmlElement, start, end))
        {
          if (!node->IsConnected())
          {
            break;
          }
          continue;
        }
        element = ShareRefPtr(htmlElement);
      }
      else if (Is<dom::Text>(*node) && node->Renderer() && lastStyledNode != node->ParentNode())
      {
        // Last styled node was not parent node of this text node, but we wish to style this
        // text node. To make this possible, add a style span to surround this text node.
        auto span = CreateStyleSpanElement(GetDocument());
        if (!SurroundNodeRangeWithElement(*node, *node, Ref<dom::Element>(span)))
        {
          continue;
        }
        reachedEnd = node->IsDescendantOf(beyondEnd.get()) || !node->ParentElement();
        element = ShareRefPtr(&Downcast<html::HTMLElement>(*span));
      }
      else
      {
        // Only handle HTML elements and text nodes.
        continue;
      }
      lastStyledNode = node;

      auto inlineStyle = CopyStyleOrCreateEmpty(element->InlineStyle());
      float currentFontSize = ComputedFontSize(node.get());
      auto startingFontSize = startingFontSizes.find(node.get());
      float desiredFontSize = std::max(
        MinimumFontSize, (startingFontSize != startingFontSizes.end() ? startingFontSize->second : 0.0f)
                           + style->FontSizeDelta());
      if (inlineStyle->GetPropertyCSSValue(PropertyId::FontSize))
      {
        (void)element->RemoveInlineStyleProperty(PropertyId::FontSize);
        currentFontSize = ComputedFontSize(node.get());
      }
      if (currentFontSize != desiredFontSize)
      {
        inlineStyle->SetProperty(PropertyId::FontSize,
                                 css::PrimitiveValue::Create(desiredFontSize, css::UnitType::px));
        SetNodeAttribute(*element, dom::DOMStringAtom(Attributes::Style),
                         inlineStyle->AsText(css::DefaultSerialisationContext()));
      }
      if (inlineStyle->empty())
      {
        RemoveNodeAttribute(*element, dom::DOMStringAtom(Attributes::Style));
        if (IsSpanWithoutAttributesOrUnstyledStyleSpan(*element))
        {
          unstyledSpans.push_back(Ref<dom::Element>(element));
        }
      }
    }

    for (auto &unstyledSpan : unstyledSpans)
    {
      RemoveNodePreservingChildren(*unstyledSpan);
    }
  }

  void ApplyStyleCommand::CleanupUnstyledAppleStyleSpans(dom::ContainerNode *dummySpanAncestor) noexcept
  {
    if (!dummySpanAncestor)
    {
      return;
    }

    // Dummy spans are created when text node is split, so that style information
    // can be propagated, which can result in more splitting. If a dummy span gets
    // cloned/split, the new node is always a sibling of it. Therefore, we scan
    // all the children of the dummy's parent

    List<Ref<dom::Element>> toRemove;
    for (auto *child = dummySpanAncestor->FirstChild(); child; child = child->NextSibling())
    {
      auto *element = DynamicDowncast<dom::Element>(*child);
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

  RefPtr<html::HTMLElement>
    ApplyStyleCommand::SplitAncestorsWithUnicodeBidi(dom::Node *node, bool before,
                                                     WritingDirection allowedDirection) noexcept
  {
    // We are allowed to leave the highest ancestor with unicode-bidi unsplit if it is unicode-bidi: embed and
    // direction: allowedDirection. In that case, we return the unsplit ancestor. Otherwise, we return 0.
    RefPtr<dom::Element> block = EnclosingBlock(ShareRefPtr(node));
    if (!block || node == block)
    {
      return nullptr;
    }

    RefPtr<dom::Node> highestAncestorWithUnicodeBidi;
    RefPtr<dom::Node> nextHighestAncestorWithUnicodeBidi;
    ValueId highestAncestorUnicodeBidi = ValueId::Invalid;
    for (RefPtr<dom::Node> ancestor = ShareRefPtr(node->ParentNode()); ancestor != block;
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

    RefPtr<html::HTMLElement> unsplitAncestor;

    if (allowedDirection != WritingDirection::Natural && highestAncestorUnicodeBidi != ValueId::BidiOverride)
    {
      if (RefPtr<html::HTMLElement> highestAncestorElementWithUnicodeBidi =
            ShareRefPtr(DynamicDowncast<html::HTMLElement>(*highestAncestorWithUnicodeBidi)))
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
    RefPtr<dom::Node> currentNode = ShareRefPtr(node);
    while (currentNode)
    {
      RefPtr<dom::Element> parent = ShareRefPtr(&Downcast<dom::Element>(*currentNode->ParentNode()));
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

  void ApplyStyleCommand::RemoveEmbeddingUpToEnclosingBlock(dom::Node *node,
                                                            dom::Node *unsplitAncestor) noexcept
  {
    RefPtr<dom::Element> block = EnclosingBlock(ShareRefPtr(node));
    if (!block || node == block)
    {
      return;
    }

    RefPtr<dom::Node> parent;
    for (RefPtr<dom::Node> ancestor = ShareRefPtr(node->ParentNode());
         ancestor != block && ancestor != unsplitAncestor; ancestor = parent)
    {
      parent = ShareRefPtr(ancestor->ParentNode());
      RefPtr<dom::StyledElement> element = ShareRefPtr(DynamicDowncast<dom::StyledElement>(*ancestor));
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
      if (element->HasAttribute(dom::DOMStringAtom(Attributes::Dir)))
      {
        // FIXME: If this is a BDO element, we should probably just remove it if it has no
        // other attributes, like we (should) do with B and I elements.
        RemoveNodeAttribute(*element, dom::DOMStringAtom(Attributes::Dir));
      }
      else
      {
        auto inlineStyle = CopyStyleOrCreateEmpty(element->InlineStyle());
        (void)inlineStyle->SetProperty(PropertyId::UnicodeBidi, ValueId::Normal);
        (void)inlineStyle->RemoveProperty(PropertyId::Direction);
        SetNodeAttribute(*element, dom::DOMStringAtom(Attributes::Style),
                         inlineStyle->AsText(css::DefaultSerialisationContext()));
        if (IsSpanWithoutAttributesOrUnstyledStyleSpan(*element))
        {
          RemoveNodePreservingChildren(*element);
        }
      }
    }
  }

  void ApplyStyleCommand::ApplyInlineStyle(EditingStyle &style) noexcept
  {
    RefPtr<dom::ContainerNode> startDummySpanAncestor;
    RefPtr<dom::ContainerNode> endDummySpanAncestor;

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
    dom::Position removeStart = start.Upstream();
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
      dom::Position embeddingRemoveStart = removeStart;
      if (startUnsplitAncestor && NodeFullySelected(*startUnsplitAncestor, removeStart, end))
      {
        embeddingRemoveStart = dom::PositionInParentAfterNode(startUnsplitAncestor.get());
      }

      dom::Position embeddingRemoveEnd = end;
      if (endUnsplitAncestor && NodeFullySelected(*endUnsplitAncestor, removeStart, end))
      {
        embeddingRemoveEnd = dom::PositionInParentBeforeNode(endUnsplitAncestor.get()).Downstream();
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
      RefPtr<dom::Node> startNode = ShareRefPtr(start.DeprecatedNode());
      auto embeddingStartNode = HighestEmbeddingAncestor(startNode.get(), EnclosingBlock(startNode).get());
      RefPtr<dom::Node> endNode = ShareRefPtr(end.DeprecatedNode());
      auto embeddingEndNode = HighestEmbeddingAncestor(endNode.get(), EnclosingBlock(endNode).get());

      if (embeddingStartNode || embeddingEndNode)
      {
        dom::Position embeddingApplyStart =
          embeddingStartNode ? dom::PositionInParentAfterNode(embeddingStartNode.get()) : start;
        dom::Position embeddingApplyEnd =
          embeddingEndNode ? dom::PositionInParentBeforeNode(embeddingEndNode.get()) : end;
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
    CleanupUnstyledAppleStyleSpans(startDummySpanAncestor.get());
    if (endDummySpanAncestor != startDummySpanAncestor)
    {
      CleanupUnstyledAppleStyleSpans(endDummySpanAncestor.get());
    }
  }

  void ApplyStyleCommand::FixRangeAndApplyInlineStyle(EditingStyle &style, const dom::Position &start,
                                                      const dom::Position &end) noexcept
  {
    RefPtr<dom::Node> startNode = ShareRefPtr(start.DeprecatedNode());

    if (start.DeprecatedEditingOffset() >= CaretMaxOffset(*startNode))
    {
      startNode = ShareRefPtr(TreeQueries::Next(*startNode));
      if (!startNode || end < FirstPositionInOrBeforeNode(startNode.get()))
      {
        return;
      }
    }

    RefPtr<dom::Node> pastEndNode = ShareRefPtr(end.DeprecatedNode());
    if (end.DeprecatedEditingOffset() >= CaretMaxOffset(*pastEndNode))
    {
      pastEndNode = ShareRefPtr(TreeQueries::NextSkippingChildren(*pastEndNode));
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
    auto range = *dom::MakeSimpleRange(dom::PositionRange {start, end});
    RefPtr<dom::Element> editableRoot = ShareRefPtr(startNode->RootEditableElement());
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

  void ApplyStyleCommand::ApplyInlineStyleToNodeRange(EditingStyle &style, dom::Node &startNode,
                                                      dom::Node *pastEndNode) noexcept
  {
    if (_removeOnly)
    {
      return;
    }

    GetDocument().UpdateLayoutIgnorePendingStylesheets();

    List<InlineRunToApplyStyle> runs;
    RefPtr<dom::Node> node = ShareRefPtr(&startNode);
    for (RefPtr<dom::Node> next; node && node != pastEndNode; node = next)
    {
      next = ShareRefPtr(TreeQueries::Next(*node));

      if (!node->Renderer() || !node->HasEditableStyle())
      {
        continue;
      }

      if (!node->HasRichlyEditableStyle())
      {
        if (auto *element = DynamicDowncast<html::HTMLElement>(*node))
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
          SetNodeAttribute(*element, dom::DOMStringAtom(Attributes::Style),
                           inlineStyle->AsText(css::DefaultSerialisationContext()));
          next = ShareRefPtr(TreeQueries::NextSkippingChildren(*element));
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
          next = ShareRefPtr(TreeQueries::NextSkippingChildren(*node));
          continue;
        }
      }

      auto runStart = node;
      auto runEnd = node;
      RefPtr<dom::Node> sibling = ShareRefPtr(node->NextSibling());
      while (sibling && sibling != pastEndNode && !sibling->Contains(pastEndNode)
             && (!IsBlock(*sibling) || IsHTMLElementOfType(sibling.get(), html::TagName::br))
             && !ContainsNonEditableRegion(*sibling))
      {
        runEnd = sibling;
        sibling = ShareRefPtr(runEnd->NextSibling());
      }
      next = ShareRefPtr(TreeQueries::NextSkippingChildren(*runEnd));

      RefPtr<dom::Node> runPastEndNode = ShareRefPtr(TreeQueries::NextSkippingChildren(*runEnd));
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

  bool ApplyStyleCommand::IsStyledInlineElementToRemove(dom::Element *element) const noexcept
  {
    return (_styledInlineElement && element->LocalName() == _styledInlineElement->LocalName()
            && element->NamespaceURI() == _styledInlineElement->NamespaceURI())
           || (_isInlineElementToRemoveFunction && _isInlineElementToRemoveFunction(element));
  }

  bool ApplyStyleCommand::ShouldApplyInlineStyleToRun(EditingStyle &style, dom::Node *runStart,
                                                      dom::Node *pastEndNode) noexcept
  {
    krys_debug_assert(runStart);

    auto *styledInlineHTMLElement = DynamicDowncast<html::HTMLElement>(_styledInlineElement.get());
    for (RefPtr<dom::Node> node = ShareRefPtr(runStart); node && node != pastEndNode;
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
              || !EnclosingElementWithTag(dom::PositionBeforeNode(node.get()),
                                          styledInlineHTMLElement->TagQualifiedName().TagName())))
      {
        return true;
      }
    }
    return false;
  }

  void ApplyStyleCommand::RemoveConflictingInlineStyleFromRun(EditingStyle &style,
                                                              RefPtr<dom::Node> &runStart,
                                                              RefPtr<dom::Node> &runEnd,
                                                              dom::Node *pastEndNode) noexcept
  {
    krys_debug_assert(runStart && runEnd);
    RefPtr<dom::Node> next = runStart;
    for (RefPtr<dom::Node> node = next; node && node->IsConnected() && node != pastEndNode; node = next)
    {
      if (EditingIgnoresContent(*node))
      {
        krys_debug_assert(!node->Contains(pastEndNode));
        next = ShareRefPtr(TreeQueries::NextSkippingChildren(*node));
      }
      else
      {
        next = ShareRefPtr(TreeQueries::Next(*node));
      }

      auto *htmlElement = DynamicDowncast<html::HTMLElement>(*node);
      if (!htmlElement)
      {
        continue;
      }

      RefPtr<dom::Node> previousSibling = ShareRefPtr(node->PreviousSibling());
      RefPtr<dom::Node> nextSibling = ShareRefPtr(node->NextSibling());
      RefPtr<dom::ContainerNode> parent = ShareRefPtr(node->ParentNode());
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

  bool ApplyStyleCommand::RemoveInlineStyleFromElement(EditingStyle &style, html::HTMLElement &element,
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

  void ApplyStyleCommand::ReplaceWithSpanOrRemoveIfWithoutAttributes(html::HTMLElement &element) noexcept
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

  bool ApplyStyleCommand::RemoveImplicitlyStyledElement(EditingStyle &style, html::HTMLElement &element,
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
    List<dom::DOMStringAtom> attributes;
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

  bool ApplyStyleCommand::RemoveCSSStyle(EditingStyle &style, html::HTMLElement &element,
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
      RemoveNodeAttribute(element, dom::DOMStringAtom(Attributes::Style));
    }
    else
    {
      SetNodeAttribute(element, dom::DOMStringAtom(Attributes::Style),
                       newInlineStyle->AsText(css::DefaultSerialisationContext()));
    }

    if (IsSpanWithoutAttributesOrUnstyledStyleSpan(element))
    {
      RemoveNodePreservingChildren(element);
    }

    return true;
  }

  RefPtr<html::HTMLElement>
    ApplyStyleCommand::HighestAncestorWithConflictingInlineStyle(EditingStyle &style,
                                                                 dom::Node *node) noexcept
  {
    if (!node)
    {
      return nullptr;
    }

    RefPtr<html::HTMLElement> result;
    RefPtr<dom::Element> unsplittableElement =
      UnsplittableElementForPosition(FirstPositionInOrBeforeNode(node));

    for (RefPtr<dom::Node> ancestor = ShareRefPtr(node); ancestor;
         ancestor = ShareRefPtr(ancestor->ParentNode()))
    {
      auto *htmlAncestor = DynamicDowncast<html::HTMLElement>(*ancestor);
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

  void ApplyStyleCommand::ApplyInlineStyleToPushDown(dom::Node &node, EditingStyle *style) noexcept
  {
    node.NodeDocument().UpdateStyleIfNeeded();

    if (!style || style->IsEmpty() || !node.Renderer() || IsHTMLElementOfType(&node, html::TagName::iframe))
    {
      return;
    }

    RefPtr<EditingStyle> newInlineStyle = ShareRefPtr(style);
    if (auto *htmlElement = DynamicDowncast<html::HTMLElement>(node);
        htmlElement && htmlElement->InlineStyle())
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
      if (auto *htmlElement = DynamicDowncast<html::HTMLElement>(node))
      {
        SetNodeAttribute(*htmlElement, dom::DOMStringAtom(Attributes::Style),
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

  void ApplyStyleCommand::PushDownInlineStyleAroundNode(EditingStyle &style, dom::Node *targetNode) noexcept
  {
    auto highestAncestor = HighestAncestorWithConflictingInlineStyle(style, targetNode);
    if (!highestAncestor)
    {
      return;
    }

    // The outer loop is traversing the tree vertically from highestAncestor to targetNode
    RefPtr<dom::Node> current = highestAncestor;
    // Along the way, styled elements that contain targetNode are removed and accumulated into
    // elementsToPushDown. Each child of the removed element, exclusing ancestors of targetNode, is then
    // wrapped by clones of elements in elementsToPushDown.
    List<Ref<dom::Element>> elementsToPushDown;
    while (current && current != targetNode && current->Contains(targetNode))
    {
      List<Ref<dom::Node>> currentChildren;
      for (auto *child = current->FirstChild(); child; child = child->NextSibling())
      {
        currentChildren.push_back(ShareRef(*child));
      }

      RefPtr<dom::StyledElement> styledElement;
      if (auto *currentElement = DynamicDowncast<dom::StyledElement>(*current);
          currentElement && IsStyledInlineElementToRemove(currentElement))
      {
        styledElement = ShareRefPtr(currentElement);
        elementsToPushDown.push_back(ShareRef(*currentElement));
      }

      auto styleToPushDown = EditingStyle::Create();
      if (auto *htmlElement = DynamicDowncast<html::HTMLElement>(*current))
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
            wrapper->RemoveAttribute(dom::DOMStringAtom(Attributes::Style));
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

  void ApplyStyleCommand::RemoveInlineStyle(EditingStyle &style, const dom::Position &start,
                                            const dom::Position &end) noexcept
  {
    krys_debug_assert(start.IsNotNull());
    krys_debug_assert(end.IsNotNull());
    krys_debug_assert(start.AnchorNode()->IsConnected());
    krys_debug_assert(end.AnchorNode()->IsConnected());
    krys_debug_assert(std::is_lteq(dom::TreeOrder<dom::TreeType::ShadowIncludingTree>(start, end)));
    // FIXME: We should assert that start/end are not in the middle of a text node.

    dom::Position pushDownStart = start.Downstream();
    // If the pushDownStart is at the end of a text node, then this node is not fully selected.
    // Move it to the next deep quivalent position to avoid removing the style from this node.
    // e.g. if pushDownStart was at Position("hello", 5) in <b>hello<div>world</div></b>, we want
    // Position("world", 0) instead.
    if (auto *text = DynamicDowncast<dom::Text>(pushDownStart.GetContainerNode());
        text && static_cast<size_t>(pushDownStart.ComputeOffsetInContainerNode()) == text->Length())
    {
      pushDownStart = NextVisuallyDistinctCandidate(pushDownStart);
    }
    // If pushDownEnd is at the start of a text node, then this node is not fully selected.
    // Move it to the previous deep equivalent position to avoid removing the style from this node.
    dom::Position pushDownEnd = end.Upstream();
    if (Is<dom::Text>(pushDownEnd.GetContainerNode()) && !pushDownEnd.ComputeOffsetInContainerNode())
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
    dom::Position s = start.IsNull() || start.IsOrphan() ? pushDownStart : start;
    dom::Position e = end.IsNull() || end.IsOrphan() ? pushDownEnd : end;

    RefPtr<dom::Node> node = ShareRefPtr(start.DeprecatedNode());
    while (node)
    {
      RefPtr<dom::Node> next;
      if (EditingIgnoresContent(*node))
      {
        krys_debug_assert(node == end.DeprecatedNode() || !node->Contains(end.DeprecatedNode()));
        next = ShareRefPtr(TreeQueries::NextSkippingChildren(*node));
      }
      else
      {
        next = ShareRefPtr(TreeQueries::Next(*node));
      }

      if (RefPtr<html::HTMLElement> element = ShareRefPtr(DynamicDowncast<html::HTMLElement>(*node));
          element && NodeFullySelected(*element, start, end))
      {
        RefPtr<dom::Node> prev = ShareRefPtr(TreeQueries::PreviousPostOrder(*element));
        RefPtr<dom::Node> nextAfterElement = ShareRefPtr(TreeQueries::Next(*element));
        RefPtr<EditingStyle> styleToPushDown;
        RefPtr<dom::Node> childNode;
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
            krys_debug_assert(s.GetAnchorType() == dom::Position::AnchorType::PositionIsBeforeAnchor
                              || s.GetAnchorType() == dom::Position::AnchorType::PositionIsBeforeChildren
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

  bool ApplyStyleCommand::NodeFullySelected(dom::Element &element, const dom::Position &start,
                                            const dom::Position &end) const noexcept
  {
    // The tree may have changed and Position::upstream() relies on an up-to-date layout.
    element.NodeDocument().UpdateLayoutIgnorePendingStylesheets();
    return FirstPositionInOrBeforeNode(&element) >= start
           && LastPositionInOrAfterNode(&element).Upstream() <= end;
  }

  void ApplyStyleCommand::SplitTextAtStart(const dom::Position &start, const dom::Position &end) noexcept
  {
    krys_debug_assert(Is<dom::Text>(start.GetContainerNode()));

    dom::Position newEnd;
    if (end.GetAnchorType() == dom::Position::AnchorType::PositionIsOffsetInAnchor
        && start.GetContainerNode() == end.GetContainerNode())
    {
      newEnd =
        dom::Position(ShareRefPtr(end.ContainerText()),
                      static_cast<uint32>(end.OffsetInContainerNode() - start.OffsetInContainerNode()));
    }
    else
    {
      newEnd = end;
    }

    RefPtr<dom::Text> text = ShareRefPtr(start.ContainerText());
    SplitTextNode(*text, static_cast<uint32>(start.OffsetInContainerNode()));
    UpdateStartEnd(dom::FirstPositionInNode(text.get()), newEnd);
  }

  void ApplyStyleCommand::SplitTextAtEnd(const dom::Position &start, const dom::Position &end) noexcept
  {
    krys_debug_assert(Is<dom::Text>(end.GetContainerNode()));

    bool shouldUpdateStart = start.GetAnchorType() == dom::Position::AnchorType::PositionIsOffsetInAnchor
                             && start.GetContainerNode() == end.GetContainerNode();
    Ref<dom::Text> text = ShareRef(Downcast<dom::Text>(*end.DeprecatedNode()));
    SplitTextNode(*text, static_cast<uint32>(end.OffsetInContainerNode()));

    RefPtr<dom::Text> prevNode = ShareRefPtr(DynamicDowncast<dom::Text>(text->PreviousSibling()));
    if (!prevNode)
    {
      return;
    }

    dom::Position newStart =
      shouldUpdateStart
        ? dom::Position(RefPtr<dom::Text>(prevNode), static_cast<uint32>(start.OffsetInContainerNode()))
        : start;
    UpdateStartEnd(newStart, dom::LastPositionInNode(prevNode.get()));
  }

  void ApplyStyleCommand::SplitTextElementAtStart(const dom::Position &start,
                                                  const dom::Position &end) noexcept
  {
    krys_debug_assert(Is<dom::Text>(start.GetContainerNode()));

    dom::Position newEnd;
    if (start.GetContainerNode() == end.GetContainerNode())
    {
      newEnd =
        dom::Position(ShareRefPtr(end.ContainerText()),
                      static_cast<uint32>(end.OffsetInContainerNode() - start.OffsetInContainerNode()));
    }
    else
    {
      newEnd = end;
    }

    RefPtr<dom::Node> container = ShareRefPtr(start.GetContainerNode());
    SplitTextNodeContainingElement(*start.ContainerText(),
                                   static_cast<uint32>(start.OffsetInContainerNode()));
    UpdateStartEnd(dom::PositionBeforeNode(container.get()), newEnd);
  }

  void ApplyStyleCommand::SplitTextElementAtEnd(const dom::Position &start, const dom::Position &end) noexcept
  {
    krys_debug_assert(Is<dom::Text>(end.GetContainerNode()));

    bool shouldUpdateStart = start.GetContainerNode() == end.GetContainerNode();
    SplitTextNodeContainingElement(*end.ContainerText(), static_cast<uint32>(end.OffsetInContainerNode()));

    RefPtr<dom::Node> parentElement = ShareRefPtr(end.GetContainerNode()->ParentNode());
    if (!parentElement || !parentElement->PreviousSibling())
    {
      return;
    }
    RefPtr<dom::Text> firstTextNode =
      ShareRefPtr(DynamicDowncast<dom::Text>(parentElement->PreviousSibling()->LastChild()));
    if (!firstTextNode)
    {
      return;
    }

    dom::Position newStart =
      shouldUpdateStart
        ? dom::Position(RefPtr<dom::Text>(firstTextNode), static_cast<uint32>(start.OffsetInContainerNode()))
        : start;
    UpdateStartEnd(newStart, dom::PositionAfterNode(firstTextNode.get()));
  }

  bool ApplyStyleCommand::ShouldSplitTextElement(dom::Element *element, EditingStyle &style) noexcept
  {
    auto *htmlElement = DynamicDowncast<html::HTMLElement>(element);
    return htmlElement && ShouldRemoveInlineStyleFromElement(style, *htmlElement);
  }

  bool ApplyStyleCommand::IsValidCaretPositionInTextNode(const dom::Position &position) noexcept
  {
    krys_debug_assert(position.IsNotNull());

    auto *node = position.GetContainerNode();
    if (position.GetAnchorType() != dom::Position::AnchorType::PositionIsOffsetInAnchor
        || !Is<dom::Text>(node))
    {
      return false;
    }
    int32 offsetInText = position.OffsetInContainerNode();
    return offsetInText > CaretMinOffset(*node) && offsetInText < CaretMaxOffset(*node);
  }

  bool ApplyStyleCommand::MergeStartWithPreviousIfIdentical(const dom::Position &start,
                                                            const dom::Position &end) noexcept
  {
    RefPtr<dom::Node> startNode = ShareRefPtr(start.GetContainerNode());
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

    RefPtr<dom::Element> element = ShareRefPtr(DynamicDowncast<dom::Element>(*startNode));
    if (!element)
    {
      return false;
    }

    RefPtr<dom::Node> previousSibling = ShareRefPtr(startNode->PreviousSibling());
    if (!previousSibling)
    {
      return false;
    }
    RefPtr<dom::Element> previousElement = ShareRefPtr(ElementIfEquivalent(*element, *previousSibling));
    if (!previousElement)
    {
      return false;
    }

    RefPtr<dom::Node> startChild = ShareRefPtr(element->FirstChild());
    krys_debug_assert(startChild);
    MergeIdenticalElements(*previousElement, *element);

    // FIXME: Inconsistent that we use computeOffsetInContainerNode for start, but deprecatedEditingOffset for
    // end.
    uint32 startOffset = startChild->ComputeNodeIndex();
    uint32 endOffset = static_cast<uint32>(end.DeprecatedEditingOffset())
                       + (startNode == end.DeprecatedNode() ? startOffset : 0);
    UpdateStartEnd(dom::Position(RefPtr<dom::Node>(startNode), startOffset,
                                 dom::Position::AnchorType::PositionIsOffsetInAnchor),
                   dom::Position(ShareRefPtr(end.DeprecatedNode()), endOffset,
                                 dom::Position::AnchorType::PositionIsOffsetInAnchor));
    return true;
  }

  bool ApplyStyleCommand::MergeEndWithNextIfIdentical(const dom::Position &start,
                                                      const dom::Position &end) noexcept
  {
    RefPtr<dom::Node> endNode = ShareRefPtr(end.GetContainerNode());

    if (IsAtomicNode(endNode.get()))
    {
      int32 endOffset = end.ComputeOffsetInContainerNode();
      if (dom::OffsetIsBeforeLastNodeOffset(static_cast<uint32>(endOffset), endNode.get())
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

    RefPtr<dom::Element> element = ShareRefPtr(DynamicDowncast<dom::Element>(*endNode));
    if (!element)
    {
      return false;
    }

    RefPtr<dom::Node> nextSibling = ShareRefPtr(endNode->NextSibling());
    if (!nextSibling)
    {
      return false;
    }
    RefPtr<dom::Element> nextElement = ShareRefPtr(ElementIfEquivalent(*element, *nextSibling));
    if (!nextElement)
    {
      return false;
    }

    RefPtr<dom::Node> nextChild = ShareRefPtr(nextElement->FirstChild());

    MergeIdenticalElements(*element, *nextElement);

    bool shouldUpdateStart = start.GetContainerNode() == endNode;
    uint32 endOffset = nextChild ? nextChild->ComputeNodeIndex() : nextElement->CountChildNodes();
    UpdateStartEnd(shouldUpdateStart ? dom::Position(RefPtr<dom::Node>(nextElement),
                                                     static_cast<uint32>(start.OffsetInContainerNode()),
                                                     dom::Position::AnchorType::PositionIsOffsetInAnchor)
                                     : start,
                   dom::Position(RefPtr<dom::Node>(nextElement), endOffset,
                                 dom::Position::AnchorType::PositionIsOffsetInAnchor));
    return true;
  }

  bool ApplyStyleCommand::SurroundNodeRangeWithElement(dom::Node &startNode, dom::Node &endNode,
                                                       Ref<dom::Element> &&elementToInsert) noexcept
  {
    Ref<dom::Node> protectedStartNode = ShareRef(startNode);
    Ref<dom::Element> element = krys::move(elementToInsert);

    if (!InsertNodeBefore(*element, startNode) || !element->IsContentRichlyEditable())
    {
      RemoveNode(*element);
      return false;
    }

    RefPtr<dom::Node> node = ShareRefPtr(&startNode);
    while (node)
    {
      RefPtr<dom::Node> next = ShareRefPtr(node->NextSibling());
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

    RefPtr<dom::Node> nextSibling = ShareRefPtr(element->NextSibling());
    RefPtr<dom::Node> previousSibling = ShareRefPtr(element->PreviousSibling());

    if (nextSibling && nextSibling->HasEditableStyle())
    {
      if (RefPtr<dom::Element> nextElement = ShareRefPtr(ElementIfEquivalent(*element, *nextSibling)))
      {
        MergeIdenticalElements(*element, *nextElement);
      }
    }

    if (auto *previousSiblingElement = DynamicDowncast<dom::Element>(previousSibling.get());
        previousSiblingElement && previousSiblingElement->HasEditableStyle())
    {
      RefPtr<dom::Node> mergedElementNode = ShareRefPtr(previousSibling->NextSibling());
      krys_debug_assert(mergedElementNode);
      if (mergedElementNode->HasEditableStyle())
      {
        if (RefPtr<dom::Element> mergedElement =
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

  void ApplyStyleCommand::AddBlockStyle(const StyleChange &styleChange, html::HTMLElement &block) noexcept
  {
    // Do not check for legacy styles here. Those styles, like <B> and <I>, only apply for inline content.
    krys_debug_assert(styleChange.CssStyle());
    SetNodeAttribute(
      block, dom::DOMStringAtom(Attributes::Style),
      JoinWithSpace(styleChange.CssStyle()->AsText(css::DefaultSerialisationContext()),
                    block.GetAttribute(dom::DOMStringAtom(Attributes::Style)).value_or(dom::DOMString())));
  }

  void ApplyStyleCommand::AddInlineStyleIfNeeded(EditingStyle *style, dom::Node &start, dom::Node &end,
                                                 AddStyledElement addStyledElement) noexcept
  {
    if (!start.IsConnected() || !end.IsConnected())
    {
      return;
    }

    Ref<dom::Node> protectedStart = ShareRef(start);
    RefPtr<dom::Node> dummyElement;
    StyleChange styleChange(style, PositionToComputeInlineStyleChange(start, dummyElement));

    if (dummyElement)
    {
      RemoveNode(*dummyElement);
    }

    ApplyInlineStyleChange(start, end, styleChange, addStyledElement);
  }

  dom::Position
    ApplyStyleCommand::PositionToComputeInlineStyleChange(dom::Node &startNode,
                                                          RefPtr<dom::Node> &dummyElement) noexcept
  {
    // It's okay to obtain the style at the startNode because we've removed all relevant styles from the
    // current run.
    if (!Is<dom::Element>(startNode))
    {
      dummyElement = CreateStyleSpanElement(GetDocument());
      InsertNodeAt(*dummyElement, dom::PositionBeforeNode(&startNode));
      return FirstPositionInOrBeforeNode(dummyElement.get());
    }

    return FirstPositionInOrBeforeNode(&startNode);
  }

  void ApplyStyleCommand::ApplyInlineStyleChange(dom::Node &passedStart, dom::Node &passedEnd,
                                                 StyleChange &styleChange,
                                                 AddStyledElement addStyledElement) noexcept
  {
    RefPtr<dom::Node> startNode = ShareRefPtr(&passedStart);
    RefPtr<dom::Node> endNode = ShareRefPtr(&passedEnd);
    krys_debug_assert(startNode->IsConnected());
    krys_debug_assert(endNode->IsConnected());

    // Find appropriate font and span elements top-down.
    RefPtr<html::HTMLFontElement> fontContainer;
    RefPtr<html::HTMLElement> styleContainer;
    while (startNode == endNode)
    {
      if (auto *container = DynamicDowncast<html::HTMLElement>(*startNode))
      {
        if (auto *fontElement = DynamicDowncast<html::HTMLFontElement>(*container))
        {
          fontContainer = ShareRefPtr(fontElement);
        }
        if (Is<html::HTMLSpanElement>(*container)
            || (!Is<html::HTMLSpanElement>(styleContainer.get()) && container->HasChildNodes()))
        {
          styleContainer = ShareRefPtr(container);
        }
        if (!CanHaveChildrenForEditing(*startNode))
        {
          break;
        }
      }
      RefPtr<dom::Node> startNodeFirstChild = ShareRefPtr(startNode->FirstChild());
      if (!startNodeFirstChild)
      {
        break;
      }
      endNode = ShareRefPtr(startNode->LastChild());
      startNode = startNodeFirstChild;
    }

    const dom::DOMStringAtom colorAttr(u8"color");
    const dom::DOMStringAtom faceAttr(u8"face");
    const dom::DOMStringAtom sizeAttr(u8"size");

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
          (void)fontElement->SetAttribute(colorAttr, dom::DOMString(styleChange.FontColor()));
        }
        if (styleChange.ApplyFontFace())
        {
          (void)fontElement->SetAttribute(faceAttr, dom::DOMString(styleChange.FontFace()));
        }
        if (styleChange.ApplyFontSize())
        {
          (void)fontElement->SetAttribute(sizeAttr, dom::DOMString(styleChange.FontSize()));
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
          SetNodeAttribute(*styleContainer, dom::DOMStringAtom(Attributes::Style),
                           inlineStyle->Style()->AsText(css::DefaultSerialisationContext()));
        }
        else
        {
          SetNodeAttribute(*styleContainer, dom::DOMStringAtom(Attributes::Style),
                           styleToMerge->AsText(css::DefaultSerialisationContext()));
        }
      }
      else
      {
        auto styleElement = CreateStyleSpanElement(GetDocument());
        (void)styleElement->SetAttribute(dom::DOMStringAtom(Attributes::Style),
                                         styleToMerge->AsText(css::DefaultSerialisationContext()));
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

  float ApplyStyleCommand::ComputedFontSize(dom::Node *node) noexcept
  {
    if (!node)
    {
      return 0;
    }

    auto value = style::Extractor(node).PropertyValue(PropertyId::FontSize);
    if (!value)
    {
      return 0;
    }
    return Downcast<css::PrimitiveValue>(*value).ResolveAsLengthDeprecated<float>();
  }

  void ApplyStyleCommand::JoinChildTextNodes(dom::Node *node, const dom::Position &start,
                                             const dom::Position &end) noexcept
  {
    if (!node)
    {
      return;
    }

    dom::Position newStart = start;
    dom::Position newEnd = end;

    List<Ref<dom::Text>> textNodes;
    for (auto *child = node->FirstChild(); child; child = child->NextSibling())
    {
      if (auto *textNode = DynamicDowncast<dom::Text>(*child))
      {
        textNodes.push_back(ShareRef(*textNode));
      }
    }

    for (auto &childText : textNodes)
    {
      RefPtr<dom::Text> next = ShareRefPtr(DynamicDowncast<dom::Text>(childText->NextSibling()));
      if (!next)
      {
        continue;
      }

      auto childLength = static_cast<uint32>(childText->Length());
      if (start.GetAnchorType() == dom::Position::AnchorType::PositionIsOffsetInAnchor
          && start.GetContainerNode() == next)
      {
        newStart = dom::Position(RefPtr<dom::Text>(ShareRefPtr(childText.get())),
                                 childLength + static_cast<uint32>(start.OffsetInContainerNode()));
      }
      if (end.GetAnchorType() == dom::Position::AnchorType::PositionIsOffsetInAnchor
          && end.GetContainerNode() == next)
      {
        newEnd = dom::Position(RefPtr<dom::Text>(ShareRefPtr(childText.get())),
                               childLength + static_cast<uint32>(end.OffsetInContainerNode()));
      }
      dom::DOMString textToMove = next->Data();
      InsertTextIntoNode(*childText, childLength, textToMove);
      RemoveNode(*next);
      // don't move child node pointer. it may want to merge with more text nodes.
    }

    UpdateStartEnd(newStart, newEnd);
  }
}
