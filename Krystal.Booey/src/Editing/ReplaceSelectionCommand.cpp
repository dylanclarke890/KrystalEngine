#include "Krystal.Booey/Editing/ReplaceSelectionCommand.hpp"
#include "Krystal.Booey/Constants/Attributes.hpp"
#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/BeforeTextInsertedEvent.hpp"
#include "Krystal.Booey/DOM/DataTransfer.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/BreakBlockquoteCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/EditingStyle.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/HTMLInterchange.hpp"
#include "Krystal.Booey/Editing/Markup.hpp"
#include "Krystal.Booey/Editing/SimplifyMarkupCommand.hpp"
#include "Krystal.Booey/Editing/SmartReplace.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"

namespace krys::boo
{
  using dom::TreeQueries;

  namespace
  {
    KRYS_NODISCARD bool HasTagName(const dom::Node *node, html::TagName tagName) noexcept
    {
      return IsHTMLElementOfType(node, tagName);
    }

    KRYS_NODISCARD bool HasClass(const dom::Node &node, utf8_stringview className) noexcept
    {
      auto *element = DynamicDowncast<dom::Element>(node);
      return element
             && element->GetAttribute(dom::DOMStringAtom(Attributes::Class)) == dom::DOMString(className);
    }

    KRYS_NODISCARD bool IsInterchangeNewlineNode(const dom::Node &node) noexcept
    {
      return HasTagName(&node, html::TagName::br) && HasClass(node, AppleInterchangeNewline);
    }

    KRYS_NODISCARD bool IsInterchangeConvertedSpaceSpan(const dom::Node &node) noexcept
    {
      return Is<html::HTMLElement>(node) && HasClass(node, AppleConvertedSpace);
    }

    KRYS_NODISCARD bool IsScriptElement(const dom::Element &element) noexcept
    {
      const auto &name = element.TagQualifiedName();
      return name.TagName() == html::TagName::script
             && (name.Namespace() == html::Namespace::HTML || name.Namespace() == html::Namespace::SVG);
    }

    KRYS_NODISCARD dom::Position PositionAvoidingPrecedingNodes(dom::Position position) noexcept
    {
      krys_debug_assert(position.IsNotNull());

      // If we're already on a break, it's probably a placeholder and we shouldn't change our position.
      if (EditingIgnoresContent(*position.DeprecatedNode()))
      {
        return position;
      }

      // We also stop when changing block flow elements because even though the visual position is the
      // same.  E.g.,
      //   <div>foo^</div>^
      // The two positions above are the same visual position, but we want to stay in the same block.
      auto enclosingBlockNode = EnclosingBlock(ShareRefPtr(position.GetContainerNode()));
      for (dom::Position nextPosition = position; nextPosition.GetContainerNode() != enclosingBlockNode;
           position = nextPosition)
      {
        if (LineBreakExistsAtPosition(position))
        {
          break;
        }

        if (position.GetContainerNode()->NonShadowBoundaryParentNode())
        {
          nextPosition = dom::PositionInParentAfterNode(position.GetContainerNode());
        }

        if (nextPosition == position)
        {
          break;
        }
        if (EnclosingBlock(ShareRefPtr(nextPosition.GetContainerNode())) != enclosingBlockNode)
        {
          break;
        }
        if (VisiblePosition(position) != VisiblePosition(nextPosition))
        {
          break;
        }
      }
      return position;
    }

    KRYS_NODISCARD bool HasMatchingQuoteLevel(VisiblePosition endOfExistingContent,
                                              VisiblePosition endOfInsertedContent) noexcept
    {
      dom::Position existing = endOfExistingContent.DeepEquivalent();
      dom::Position inserted = endOfInsertedContent.DeepEquivalent();
      bool isInsideMailBlockquote =
        EnclosingNodeOfType(inserted, IsMailBlockquote, EditingBoundaryCrossingRule::CanCrossEditingBoundary)
        != nullptr;
      return isInsideMailBlockquote
             && (NumEnclosingMailBlockquotes(existing) == NumEnclosingMailBlockquotes(inserted));
    }

    KRYS_NODISCARD bool IsMailPasteAsQuotationNode(const dom::Node &node) noexcept
    {
      return HasTagName(&node, html::TagName::blockquote) && HasClass(node, ApplePasteAsQuotation);
    }

    KRYS_NODISCARD bool IsHeaderElement(const dom::Node &a) noexcept
    {
      return HasTagName(&a, html::TagName::h1) || HasTagName(&a, html::TagName::h2)
             || HasTagName(&a, html::TagName::h3) || HasTagName(&a, html::TagName::h4)
             || HasTagName(&a, html::TagName::h5) || HasTagName(&a, html::TagName::h6);
    }

    KRYS_NODISCARD bool HaveSameTagName(dom::Node &a, dom::Node *b) noexcept
    {
      auto *elementA = DynamicDowncast<dom::Element>(a);
      if (!elementA)
      {
        return false;
      }
      auto *elementB = DynamicDowncast<dom::Element>(b);
      return elementB && elementA->TagName() == elementB->TagName();
    }

    KRYS_NODISCARD bool IsProhibitedParagraphChild(const dom::Element &element) noexcept
    {
      // https://dvcs.w3.org/hg/editing/raw-file/57abe6d3cb60/editing.html#prohibited-paragraph-child
      const auto &name = element.TagQualifiedName();
      if (name.Namespace() != html::Namespace::HTML)
      {
        return false;
      }
      switch (name.TagName())
      {
        case html::TagName::address:
        case html::TagName::article:
        case html::TagName::aside:
        case html::TagName::blockquote:
        case html::TagName::caption:
        case html::TagName::center:
        case html::TagName::col:
        case html::TagName::colgroup:
        case html::TagName::dd:
        case html::TagName::details:
        case html::TagName::dir:
        case html::TagName::div:
        case html::TagName::dl:
        case html::TagName::dt:
        case html::TagName::fieldset:
        case html::TagName::figcaption:
        case html::TagName::figure:
        case html::TagName::footer:
        case html::TagName::form:
        case html::TagName::h1:
        case html::TagName::h2:
        case html::TagName::h3:
        case html::TagName::h4:
        case html::TagName::h5:
        case html::TagName::h6:
        case html::TagName::header:
        case html::TagName::hgroup:
        case html::TagName::hr:
        case html::TagName::li:
        case html::TagName::listing:
        case html::TagName::main: // Missing in the specification.
        case html::TagName::menu:
        case html::TagName::nav:
        case html::TagName::ol:
        case html::TagName::p:
        case html::TagName::plaintext:
        case html::TagName::pre:
        case html::TagName::section:
        case html::TagName::summary:
        case html::TagName::table:
        case html::TagName::tbody:
        case html::TagName::td:
        case html::TagName::tfoot:
        case html::TagName::th:
        case html::TagName::thead:
        case html::TagName::tr:
        case html::TagName::ul:
        case html::TagName::xmp:        return true;
        default:                        break;
      }
      return false;
    }

    KRYS_NODISCARD bool HasRenderedText(const dom::Text &text) noexcept
    {
      return text.Renderer() && text.Renderer()->HasRenderedText();
    }

    KRYS_NODISCARD RefPtr<dom::Node> EnclosingInline(dom::Node *node) noexcept
    {
      auto *currentNode = node;
      while (auto *parent = currentNode->ParentNode())
      {
        if (IsBlockFlowElement(*parent) || HasTagName(parent, html::TagName::body))
        {
          return ShareRefPtr(currentNode);
        }
        // Stop if any previous sibling is a block.
        for (auto *sibling = currentNode->PreviousSibling(); sibling; sibling = sibling->PreviousSibling())
        {
          if (IsBlockFlowElement(*sibling))
          {
            return ShareRefPtr(currentNode);
          }
        }
        currentNode = parent;
      }
      return ShareRefPtr(currentNode);
    }

    KRYS_NODISCARD bool IsInlineNodeWithStyle(const dom::Node &node) noexcept
    {
      // We don't want to skip over any block elements.
      if (IsBlock(node))
      {
        return false;
      }

      auto *element = DynamicDowncast<html::HTMLElement>(node);
      if (!element)
      {
        return false;
      }

      // We can skip over elements whose class attribute is
      // one of our internal classes.
      auto classAttribute =
        element->GetAttribute(dom::DOMStringAtom(Attributes::Class)).value_or(dom::DOMString());
      utf8_stringview classAttributeValue = classAttribute;
      if (classAttributeValue == AppleTabSpanClass || classAttributeValue == AppleConvertedSpace
          || classAttributeValue == ApplePasteAsQuotation)
      {
        return true;
      }

      return EditingStyle::ElementIsStyledSpanOrHTMLEquivalent(*element);
    }

    KRYS_NODISCARD RefPtr<dom::Node>
      NodeToSplitToAvoidPastingIntoInlineNodesWithStyle(const dom::Position &insertionPos) noexcept
    {
      auto containingBlock = EnclosingBlock(ShareRefPtr(insertionPos.GetContainerNode()));
      return HighestEnclosingNodeOfType(insertionPos, IsInlineNodeWithStyle,
                                        EditingBoundaryCrossingRule::CannotCrossEditingBoundary,
                                        containingBlock.get());
    }

    KRYS_NODISCARD bool HasBlankLineBetweenParagraphs(dom::Position &position) noexcept
    {
      bool reachedBoundaryStart = false;
      bool reachedBoundaryEnd = false;
      VisiblePosition visiblePosition(position);
      VisiblePosition previousPosition = visiblePosition.Previous(
        EditingBoundaryCrossingRule::CannotCrossEditingBoundary, &reachedBoundaryStart);
      VisiblePosition nextPosition =
        visiblePosition.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary, &reachedBoundaryStart);
      bool hasLineBeforePosition = IsEndOfLine(previousPosition);

      return !reachedBoundaryStart && !reachedBoundaryEnd && IsBlankParagraph(visiblePosition)
             && hasLineBeforePosition && IsStartOfLine(nextPosition);
    }

    KRYS_NODISCARD bool
      IsCharacterSmartReplaceExemptConsideringNonBreakingSpace(char32 character,
                                                               bool previousCharacter) noexcept
    {
      return IsCharacterSmartReplaceExempt(character == 0xA0 ? U' ' : character, previousCharacter);
    }

    KRYS_NODISCARD RefPtr<dom::Element> SingleChildList(dom::Element &element) noexcept
    {
      auto *child = element.FirstChild();
      if (!child || child->NextSibling())
      {
        return nullptr;
      }

      return IsListHTMLElement(child) ? ShareRefPtr(&Downcast<dom::Element>(*child)) : nullptr;
    }

    KRYS_NODISCARD Ref<dom::Element> DeepestSingleChildList(dom::Element &topLevelList) noexcept
    {
      Ref<dom::Element> list = ShareRef(topLevelList);
      while (auto childList = SingleChildList(*list))
      {
        list = Ref<dom::Element>(childList);
      }
      return list;
    }

    KRYS_NODISCARD dom::Element *EnclosingLinkEventParentOrSelf(dom::Node &node) noexcept
    {
      for (auto *current = &node; current; current = current->ParentInComposedTree())
      {
        auto *element = DynamicDowncast<dom::Element>(*current);
        if (element && css::IsLink(*element))
        {
          return element;
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD bool FullySelectsEnclosingLink(const VisibleSelection &selection) noexcept
    {
      auto start = selection.Start();
      auto end = selection.End();
      auto *ancestor = dom::CommonInclusiveAncestor(start, end);
      if (!ancestor)
      {
        return false;
      }

      auto *link = EnclosingLinkEventParentOrSelf(*ancestor);
      if (!link)
      {
        return false;
      }

      return dom::PositionBeforeNode(link).Downstream().Equals(start)
             && dom::PositionAfterNode(link).Upstream().Equals(end);
    }
  }

  /// @brief The fragment being inserted, with the nodes it shouldn't bring removed and its interchange
  /// newlines noted and removed. A text control (or an editing host that isn't richly editable) gets the
  /// fragment's text, which it may change (`BeforeTextInsertedEvent`).
  // FIXME: Rendering the fragment in a staging document to take its text as it's rendered and drop its
  // unrendered nodes, with a page for sanitizing web content; the text is the fragment's text nodes, with a
  // line break for each `<br>` and between blocks, which is what pasted plain text makes.
  class ReplacementFragment
  {
    KRYS_NON_COPYABLE(ReplacementFragment);

  private:
    RefPtr<dom::DocumentFragment> _fragment;
    bool _hasInterchangeNewlineAtStart {false};
    bool _hasInterchangeNewlineAtEnd {false};

    /// @brief The text of @p fragment: its text nodes, with a line break for each `<br>` and before each
    /// paragraph but the first.
    KRYS_NODISCARD static utf8_string FragmentText(dom::DocumentFragment &fragment) noexcept;

  public:
    ReplacementFragment(RefPtr<dom::DocumentFragment> &&inputFragment,
                        const VisibleSelection &selection) noexcept;

    KRYS_NODISCARD dom::DocumentFragment *Fragment() noexcept
    {
      return _fragment.get();
    }

    KRYS_NODISCARD dom::Node *FirstChild() const noexcept
    {
      return _fragment ? _fragment->FirstChild() : nullptr;
    }
    KRYS_NODISCARD dom::Node *LastChild() const noexcept
    {
      return _fragment ? _fragment->LastChild() : nullptr;
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return (!_fragment || !_fragment->FirstChild()) && !_hasInterchangeNewlineAtStart
             && !_hasInterchangeNewlineAtEnd;
    }

    KRYS_NODISCARD bool HasInterchangeNewlineAtStart() const noexcept
    {
      return _hasInterchangeNewlineAtStart;
    }
    KRYS_NODISCARD bool HasInterchangeNewlineAtEnd() const noexcept
    {
      return _hasInterchangeNewlineAtEnd;
    }

    void RemoveNode(dom::Node &node) noexcept;
    void RemoveNodePreservingChildren(dom::Node &node) noexcept;

  private:
    void RemoveContentsWithSideEffects() noexcept;
    void RemoveInterchangeNodes(dom::Node *container) noexcept;

    void InsertNodeBefore(dom::Node &node, dom::Node &refNode) noexcept;
  };

  ReplacementFragment::ReplacementFragment(RefPtr<dom::DocumentFragment> &&inputFragment,
                                           const VisibleSelection &selection) noexcept
      : _fragment(krys::move(inputFragment))
  {
    RefPtr<dom::DocumentFragment> fragment = _fragment;
    if (!fragment)
    {
      return;
    }
    if (!fragment->FirstChild())
    {
      return;
    }

    RemoveContentsWithSideEffects();

    RefPtr<dom::Element> editableRoot = ShareRefPtr(selection.RootEditableElement());
    krys_debug_assert(editableRoot);
    if (!editableRoot)
    {
      return;
    }

    auto *shadowHost = dom::TreeQueries::ShadowHost(*editableRoot);
    if (!(shadowHost && shadowHost->Renderer() && shadowHost->Renderer()->IsRenderTextControl())
        && editableRoot->HasRichlyEditableStyle())
    {
      RemoveInterchangeNodes(fragment.get());
      return;
    }

    RemoveInterchangeNodes(fragment.get());
    auto text = FragmentText(*fragment);

    // Give the root a chance to change the text.
    auto event = dom::BeforeTextInsertedEvent::Create(text);
    (void)dom::EventDispatcher::DispatchToTarget(*event, editableRoot.get());
    if (text != event->Text() || !editableRoot->HasRichlyEditableStyle())
    {
      auto range = selection.ToNormalizedRange();
      if (!range)
      {
        return;
      }

      _fragment = CreateFragmentFromText(*range, event->Text());
      if (!_fragment->FirstChild())
      {
        return;
      }

      RemoveInterchangeNodes(_fragment.get());
    }
  }

  utf8_string ReplacementFragment::FragmentText(dom::DocumentFragment &fragment) noexcept
  {
    utf8_string text;
    for (auto *node = fragment.FirstChild(); node; node = dom::TreeQueries::Next(*node, &fragment))
    {
      if (auto *textNode = DynamicDowncast<dom::Text>(*node))
      {
        text.append(textNode->Data());
      }
      else if (HasTagName(node, html::TagName::br))
      {
        text.push_back(u8'\n');
      }
      else if (node->ParentNode() == &fragment && Is<dom::Element>(*node) && !text.empty()
               && text.back() != u8'\n')
      {
        text.push_back(u8'\n');
      }
    }
    return text;
  }

  ReplaceSelectionCommand::~ReplaceSelectionCommand() noexcept = default;

  // FIXME: Leaving out event handler attributes and javascript: URLs (isEventHandlerAttribute,
  // attributeContainsJavaScriptURL), with scripting.
  void ReplacementFragment::RemoveContentsWithSideEffects() noexcept
  {
    List<Ref<dom::Element>> elementsToRemove;

    auto *node = _fragment->FirstChild();
    while (node)
    {
      auto *element = DynamicDowncast<dom::Element>(*node);
      if (element
          && (IsScriptElement(*element)
              || (HasTagName(element, html::TagName::style) && !HasClass(*element, WebKitMSOListQuirksStyle))
              || HasTagName(element, html::TagName::base) || HasTagName(element, html::TagName::link)
              || HasTagName(element, html::TagName::meta) || HasTagName(element, html::TagName::title)))
      {
        elementsToRemove.push_back(ShareRef(*element));
        node = TreeQueries::NextSkippingChildren(*node, _fragment.get());
        continue;
      }
      node = TreeQueries::Next(*node, _fragment.get());
    }

    for (auto &element : elementsToRemove)
    {
      RemoveNode(*element);
    }
  }

  void ReplacementFragment::RemoveNodePreservingChildren(dom::Node &node) noexcept
  {
    Ref<dom::Node> protectedNode = ShareRef(node);
    while (auto *n = node.FirstChild())
    {
      Ref<dom::Node> child = ShareRef(*n);
      RemoveNode(*child);
      InsertNodeBefore(*child, node);
    }
    RemoveNode(node);
  }

  void ReplacementFragment::RemoveNode(dom::Node &node) noexcept
  {
    if (auto *parent = node.NonShadowBoundaryParentNode())
    {
      (void)parent->RemoveChild(node);
    }
  }

  void ReplacementFragment::InsertNodeBefore(dom::Node &node, dom::Node &refNode) noexcept
  {
    if (auto *parent = refNode.NonShadowBoundaryParentNode())
    {
      (void)parent->InsertBefore(node, &refNode);
    }
  }

  void ReplacementFragment::RemoveInterchangeNodes(dom::Node *container) noexcept
  {
    _hasInterchangeNewlineAtStart = false;
    _hasInterchangeNewlineAtEnd = false;

    // Interchange newlines at the "start" of the incoming fragment must be
    // either the first node in the fragment or the first leaf in the fragment.
    RefPtr<dom::Node> node = ShareRefPtr(container->FirstChild());
    while (node)
    {
      if (IsInterchangeNewlineNode(*node))
      {
        _hasInterchangeNewlineAtStart = true;
        RemoveNode(*node);
        break;
      }
      node = ShareRefPtr(node->FirstChild());
    }
    if (!container->HasChildNodes())
    {
      return;
    }
    // Interchange newlines at the "end" of the incoming fragment must be
    // either the last node in the fragment or the last leaf in the fragment.
    node = ShareRefPtr(container->LastChild());
    while (node)
    {
      if (IsInterchangeNewlineNode(*node))
      {
        _hasInterchangeNewlineAtEnd = true;
        RemoveNode(*node);
        break;
      }
      node = ShareRefPtr(node->LastChild());
    }

    node = ShareRefPtr(container->FirstChild());
    while (node)
    {
      RefPtr<dom::Node> next = ShareRefPtr(TreeQueries::Next(*node));
      if (IsInterchangeConvertedSpaceSpan(*node))
      {
        next = ShareRefPtr(TreeQueries::NextSkippingChildren(*node));
        RemoveNodePreservingChildren(*node);
      }
      node = krys::move(next);
    }
  }

  dom::Node *ReplaceSelectionCommand::InsertedNodes::LastLeafInserted() const noexcept
  {
    krys_debug_assert(_lastNodeInserted);
    return TreeQueries::DeepLastChild(*_lastNodeInserted);
  }

  dom::Node *ReplaceSelectionCommand::InsertedNodes::PastLastLeaf() const noexcept
  {
    krys_debug_assert(_lastNodeInserted);
    return TreeQueries::Next(*TreeQueries::DeepLastChild(*_lastNodeInserted));
  }

  void ReplaceSelectionCommand::InsertedNodes::RespondToNodeInsertion(dom::Node *node) noexcept
  {
    if (!node)
    {
      return;
    }

    if (!_firstNodeInserted)
    {
      _firstNodeInserted = ShareRefPtr(node);
    }

    _lastNodeInserted = ShareRefPtr(node);
  }

  void ReplaceSelectionCommand::InsertedNodes::WillRemoveNodePreservingChildren(dom::Node *node) noexcept
  {
    if (_firstNodeInserted == node)
    {
      _firstNodeInserted = ShareRefPtr(TreeQueries::Next(*node));
    }
    if (_lastNodeInserted == node)
    {
      _lastNodeInserted =
        ShareRefPtr(node->LastChild() ? node->LastChild() : TreeQueries::NextSkippingChildren(*node));
      if (!_lastNodeInserted && _firstNodeInserted)
      {
        // If the last inserted node is at the end of the document and doesn't have any children, look
        // backwards for the previous node as the last inserted node, clamping to the first inserted node if
        // needed to ensure that the document position of the last inserted node is not behind the first
        // inserted node.
        RefPtr<dom::Node> previousNode = ShareRefPtr(TreeQueries::PreviousSkippingChildren(*node));
        krys_debug_assert(previousNode);
        _lastNodeInserted = HasFlag(_firstNodeInserted->CompareDocumentPosition(*previousNode),
                                    dom::DocumentPosition::DOCUMENT_POSITION_FOLLOWING)
                              ? previousNode
                              : _firstNodeInserted;
      }
    }
  }

  void ReplaceSelectionCommand::InsertedNodes::WillRemovePossibleAncestorNode(dom::Node *node) noexcept
  {
    bool containsFirstNode = node->Contains(_firstNodeInserted.get());
    bool containsLastNode = node->Contains(_lastNodeInserted.get());
    if (containsFirstNode && containsLastNode)
    {
      _firstNodeInserted = nullptr;
      _lastNodeInserted = nullptr;
      return;
    }

    if (containsLastNode)
    {
      _lastNodeInserted = ShareRefPtr(TreeQueries::PreviousSkippingChildren(*node));
    }
    else if (containsFirstNode)
    {
      _firstNodeInserted = ShareRefPtr(TreeQueries::NextSkippingChildren(*node));
    }

    if (!_lastNodeInserted)
    {
      _lastNodeInserted = _firstNodeInserted;
    }
    else if (!_firstNodeInserted)
    {
      _firstNodeInserted = _lastNodeInserted;
    }
    else if (_firstNodeInserted->IsDescendantOf(_lastNodeInserted.get()))
    {
      std::swap(_firstNodeInserted, _lastNodeInserted);
    }
  }

  void ReplaceSelectionCommand::InsertedNodes::WillRemoveNode(dom::Node *node) noexcept
  {
    krys_debug_assert(!_firstNodeInserted || !_firstNodeInserted->IsDescendantOf(node));
    krys_debug_assert(!_lastNodeInserted || !_lastNodeInserted->IsDescendantOf(node));

    if (_firstNodeInserted == node && _lastNodeInserted == node)
    {
      _firstNodeInserted = nullptr;
      _lastNodeInserted = nullptr;
    }
    else if (_firstNodeInserted == node)
    {
      _firstNodeInserted = ShareRefPtr(TreeQueries::NextSkippingChildren(*_firstNodeInserted));
    }
    else if (_lastNodeInserted == node)
    {
      _lastNodeInserted = ShareRefPtr(TreeQueries::PreviousSkippingChildren(*_lastNodeInserted));
      if (!_lastNodeInserted)
      {
        _lastNodeInserted = _firstNodeInserted;
      }
    }
  }

  void ReplaceSelectionCommand::InsertedNodes::DidReplaceNode(dom::Node *node, dom::Node *newNode) noexcept
  {
    if (_firstNodeInserted == node)
    {
      _firstNodeInserted = ShareRefPtr(newNode);
    }
    if (_lastNodeInserted == node)
    {
      _lastNodeInserted = ShareRefPtr(newNode);
    }
  }

  ReplaceSelectionCommand::ReplaceSelectionCommand(dom::Document &document,
                                                   RefPtr<dom::DocumentFragment> &&fragment,
                                                   EnumSet<CommandOption> options,
                                                   EditAction editAction) noexcept
      : CompositeEditCommand(document, editAction),
        _selectReplacement(options.Contains(CommandOption::SelectReplacement)),
        _smartReplace(options.Contains(CommandOption::SmartReplace)),
        _matchStyle(options.Contains(CommandOption::MatchStyle)), _documentFragment(krys::move(fragment)),
        _preventNesting(options.Contains(CommandOption::PreventNesting)),
        _movingParagraph(options.Contains(CommandOption::MovingParagraph)),
        _sanitizeFragment(options.Contains(CommandOption::SanitizeFragment)),
        _ignoreMailBlockquote(options.Contains(CommandOption::IgnoreMailBlockquote))
  {
  }

  Ref<ReplaceSelectionCommand> ReplaceSelectionCommand::Create(dom::Document &document,
                                                               RefPtr<dom::DocumentFragment> &&fragment,
                                                               EnumSet<CommandOption> options,
                                                               EditAction editingAction) noexcept
  {
    return CreateRef<ReplaceSelectionCommand>(document, krys::move(fragment), options, editingAction);
  }

  bool ReplaceSelectionCommand::ShouldMergeStart(bool selectionStartWasStartOfParagraph,
                                                 bool fragmentHasInterchangeNewlineAtStart,
                                                 bool selectionStartWasInsideMailBlockquote) noexcept
  {
    if (_movingParagraph)
    {
      return false;
    }

    VisiblePosition startOfInsertedContent(PositionAtStartOfInsertedContent());
    VisiblePosition prev =
      startOfInsertedContent.Previous(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
    if (prev.IsNull())
    {
      return false;
    }

    // When we have matching quote levels, its ok to merge more frequently.
    // For a successful merge, we still need to make sure that the inserted content starts with the beginning
    // of a paragraph. And we should only merge here if the selection start was inside a mail blockquote. This
    // prevents against removing a blockquote from newly pasted quoted content that was pasted into an
    // unquoted position.  If that unquoted position happens to be right after another blockquote, we don't
    // want to merge and risk stripping a valid block (and newline) from the pasted content.
    if (IsStartOfParagraph(startOfInsertedContent) && selectionStartWasInsideMailBlockquote
        && HasMatchingQuoteLevel(prev, PositionAtEndOfInsertedContent()))
    {
      return true;
    }

    return !selectionStartWasStartOfParagraph && !fragmentHasInterchangeNewlineAtStart
           && IsStartOfParagraph(startOfInsertedContent)
           && !HasTagName(startOfInsertedContent.DeepEquivalent().DeprecatedNode(), html::TagName::br)
           && ShouldMerge(startOfInsertedContent, prev);
  }

  bool ReplaceSelectionCommand::ShouldMergeEnd(bool selectionEndWasEndOfParagraph) noexcept
  {
    VisiblePosition endOfInsertedContent(PositionAtEndOfInsertedContent());
    VisiblePosition next = endOfInsertedContent.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
    if (next.IsNull())
    {
      return false;
    }

    return !selectionEndWasEndOfParagraph && IsEndOfParagraph(endOfInsertedContent)
           && !HasTagName(endOfInsertedContent.DeepEquivalent().DeprecatedNode(), html::TagName::br)
           && ShouldMerge(endOfInsertedContent, next);
  }

  bool ReplaceSelectionCommand::ShouldMerge(const VisiblePosition &source,
                                            const VisiblePosition &destination) noexcept
  {
    if (source.IsNull() || destination.IsNull())
    {
      return false;
    }

    RefPtr<dom::Node> sourceNode = ShareRefPtr(source.DeepEquivalent().DeprecatedNode());
    RefPtr<dom::Node> destinationNode = ShareRefPtr(destination.DeepEquivalent().DeprecatedNode());
    auto sourceBlock = EnclosingBlock(sourceNode);
    auto destinationBlock = EnclosingBlock(destinationNode);
    return !EnclosingNodeOfType(source.DeepEquivalent(), &IsMailPasteAsQuotationNode) && sourceBlock
           && (!HasTagName(sourceBlock.get(), html::TagName::blockquote) || IsMailBlockquote(*sourceBlock))
           && EnclosingListChild(sourceBlock.get()) == EnclosingListChild(destinationNode.get())
           && EnclosingTableCell(source.DeepEquivalent()) == EnclosingTableCell(destination.DeepEquivalent())
           && (!IsHeaderElement(*sourceBlock) || HaveSameTagName(*sourceBlock, destinationBlock.get()))
           // Don't merge to or from a position before or after a block because it would
           // be a no-op and cause infinite recursion.
           && !IsBlock(*sourceNode) && !IsBlock(*destinationNode);
  }

  // Style rules that match just inserted elements could change their appearance, like
  // a div inserted into a document with div { display:inline; }.
  void ReplaceSelectionCommand::RemoveRedundantStylesAndKeepStyleSpanInline(
    InsertedNodes &insertedNodes) noexcept
  {
    RefPtr<dom::Node> pastEndNode = ShareRefPtr(insertedNodes.PastLastLeaf());
    RefPtr<dom::Node> next;
    for (RefPtr<dom::Node> node = ShareRefPtr(insertedNodes.FirstNodeInserted()); node && node != pastEndNode;
         node = next)
    {
      // FIXME: <rdar://problem/5371536> Style rules that match pasted content can change it's appearance

      next = ShareRefPtr(TreeQueries::Next(*node));
      RefPtr<dom::StyledElement> element = ShareRefPtr(DynamicDowncast<dom::StyledElement>(*node));
      if (!element)
      {
        continue;
      }

      RefPtr<const css::StyleProperties> inlineStyle = ShareRefPtr(element->InlineStyle());
      auto newInlineStyle = EditingStyle::Create(inlineStyle.get());
      if (inlineStyle)
      {
        if (RefPtr<html::HTMLElement> htmlElement = ShareRefPtr(DynamicDowncast<html::HTMLElement>(*element)))
        {
          List<dom::DOMStringAtom> attributes;

          if (newInlineStyle->ConflictsWithImplicitStyleOfElement(*htmlElement))
          {
            // e.g. <b style="font-weight: normal;"> is converted to <span style="font-weight: normal;">
            node = ReplaceElementWithSpanPreservingChildrenAndAttributes(*htmlElement);
            element = ShareRefPtr(&Downcast<dom::StyledElement>(*node));
            insertedNodes.DidReplaceNode(htmlElement.get(), node.get());
          }
          else if (newInlineStyle->ExtractConflictingImplicitStyleOfAttributes(
                     *htmlElement, EditingStyle::ShouldPreserveWritingDirection::Yes, nullptr, attributes,
                     EditingStyle::ShouldExtractMatchingStyle::No))
          {
            // e.g. <font size="3" style="font-size: 20px;"> is converted to <font style="font-size: 20px;">
            for (auto &attribute : attributes)
            {
              RemoveNodeAttribute(*element, attribute);
            }
          }
        }

        RefPtr<dom::ContainerNode> context = ShareRefPtr(element->ParentNode());

        // If Mail wraps the fragment with a Paste as Quotation blockquote, or if you're pasting into a quoted
        // region, styles from blockquoteNode are allowed to override those from the source document, see
        // <rdar://problem/4930986> and <rdar://problem/5089327>.
        auto hasBlockquoteNode = [&]() -> bool
        {
          if (!context)
          {
            return false;
          }
          if (IsMailPasteAsQuotationNode(*context))
          {
            return true;
          }
          return EnclosingNodeOfType(dom::FirstPositionInNode(context.get()), IsMailBlockquote,
                                     EditingBoundaryCrossingRule::CanCrossEditingBoundary)
                 != nullptr;
        };
        if (hasBlockquoteNode())
        {
          newInlineStyle->RemoveStyleFromRulesAndContext(*element,
                                                         TreeQueries::DocumentElement(GetDocument()));
        }

        newInlineStyle->RemoveStyleFromRulesAndContext(*element, context.get());
      }

      if (!inlineStyle || newInlineStyle->IsEmpty())
      {
        if (IsStyleSpanOrSpanWithOnlyStyleAttribute(*element)
            || IsEmptyFontTag(element.get(), AllowNonEmptyStyleAttribute))
        {
          insertedNodes.WillRemoveNodePreservingChildren(element.get());
          RemoveNodePreservingChildren(*element);
          continue;
        }
        RemoveNodeAttribute(*element, dom::DOMStringAtom(Attributes::Style));
      }
      // FIXME: It seems wrong to rely only on the difference of properties set, and not on which properties
      // are set.
      else if (newInlineStyle->Style()->PropertyCount() != inlineStyle->PropertyCount())
      {
        SetNodeAttribute(*element, dom::DOMStringAtom(Attributes::Style),
                         newInlineStyle->Style()->AsText(css::DefaultSerialisationContext()));
      }

      // FIXME: Tolerate differences in id, class, and style attributes.
      if (element->ParentNode() && IsNonTableCellHTMLBlockElement(element.get())
          && ElementIfEquivalent(*element, *element->ParentNode())
          && VisiblePosition(dom::FirstPositionInNode(element->ParentNode()))
               == VisiblePosition(dom::FirstPositionInNode(element.get()))
          && VisiblePosition(dom::LastPositionInNode(element->ParentNode()))
               == VisiblePosition(dom::LastPositionInNode(element.get())))
      {
        insertedNodes.WillRemoveNodePreservingChildren(element.get());
        RemoveNodePreservingChildren(*element);
        continue;
      }

      if (element->ParentNode() && element->ParentNode()->HasRichlyEditableStyle())
      {
        RemoveNodeAttribute(*element, dom::DOMStringAtom(u8"contenteditable"));
      }

      // WebKit used to not add display: inline and float: none on copy.
      // Keep this code around for backward compatibility
      if (IsLegacyAppleStyleSpan(element.get()))
      {
        if (!element->FirstChild())
        {
          insertedNodes.WillRemoveNodePreservingChildren(element.get());
          RemoveNodePreservingChildren(*element);
          continue;
        }
        // There are other styles that style rules can give to style spans,
        // but these are the two important ones because they'll prevent
        // inserted content from appearing in the right paragraph.
        // FIXME: Hyatt is concerned that selectively using display:inline will give inconsistent
        // results. We already know one issue because td elements ignore their display property
        // in quirks mode (which Mail.app is always in). We should look for an alternative.

        // Mutate using the CSSOM wrapper so we get the same event behavior as a script.
        if (IsBlock(*element))
        {
          (void)element->Style().SetPropertyInternal(css::PropertyId::Display, u8"inline",
                                                     css::IsImportant(false));
        }
        if (element->Renderer() && element->Renderer()->Style().IsFloating())
        {
          (void)element->Style().SetPropertyInternal(css::PropertyId::Float, u8"none",
                                                     css::IsImportant(false));
        }
      }
    }
  }

  void ReplaceSelectionCommand::MakeInsertedContentRoundTrippableWithHTMLTreeBuilder(
    InsertedNodes &insertedNodes) noexcept
  {
    RefPtr<dom::Node> pastEndNode = ShareRefPtr(insertedNodes.PastLastLeaf());
    RefPtr<dom::Node> next;
    for (RefPtr<dom::Node> node = ShareRefPtr(insertedNodes.FirstNodeInserted()); node && node != pastEndNode;
         node = next)
    {
      next = ShareRefPtr(TreeQueries::Next(*node));

      RefPtr<html::HTMLElement> element = ShareRefPtr(DynamicDowncast<html::HTMLElement>(*node));
      if (!element)
      {
        continue;
      }

      if (!node->IsConnected())
      {
        continue;
      }

      if (IsProhibitedParagraphChild(*element))
      {
        if (auto *paragraphElement =
              EnclosingElementWithTag(dom::PositionInParentBeforeNode(node.get()), html::TagName::p))
        {
          auto *parent = paragraphElement->ParentNode();
          if (parent && parent->HasEditableStyle())
          {
            MoveNodeOutOfAncestor(*node, *paragraphElement, insertedNodes);
            if (!node->IsConnected())
            {
              continue;
            }
          }
        }
      }

      if (IsHeaderElement(*node))
      {
        if (auto headerElement =
              HighestEnclosingNodeOfType(dom::PositionInParentBeforeNode(node.get()), IsHeaderElement))
        {
          if (headerElement->ParentNode() && headerElement->ParentNode()->IsContentRichlyEditable())
          {
            MoveNodeOutOfAncestor(*node, *headerElement, insertedNodes);
          }
          else
          {
            auto newSpanElement = ReplaceElementWithSpanPreservingChildrenAndAttributes(*element);
            insertedNodes.DidReplaceNode(node.get(), newSpanElement.get());
          }
        }
      }
    }
  }

  void ReplaceSelectionCommand::MoveNodeOutOfAncestor(dom::Node &node, dom::Node &ancestor,
                                                      InsertedNodes &insertedNodes) noexcept
  {
    Ref<dom::Node> protectedNode = ShareRef(node);
    Ref<dom::Node> protectedAncestor = ShareRef(ancestor);

    if (!protectedAncestor->ParentNode()->HasEditableStyle())
    {
      return;
    }

    VisiblePosition positionAtEndOfNode = LastPositionInOrAfterNode(&node);
    VisiblePosition lastPositionInParagraph = dom::LastPositionInNode(&ancestor);
    if (positionAtEndOfNode == lastPositionInParagraph)
    {
      RemoveNode(node);
      if (!ancestor.IsConnected())
      {
        return;
      }
      if (ancestor.NextSibling())
      {
        (void)InsertNodeBefore(*protectedNode, *ancestor.NextSibling());
      }
      else
      {
        AppendNode(*protectedNode, *ancestor.ParentNode());
      }
    }
    else
    {
      RefPtr<dom::Node> nodeToSplitTo = SplitTreeToNode(node, ancestor, true);
      RemoveNode(node);
      if (nodeToSplitTo)
      {
        (void)InsertNodeBefore(*protectedNode, *nodeToSplitTo);
      }
    }

    (void)GetDocument().UpdateLayoutIgnorePendingStylesheets();

    bool safeToRemoveAncestor = true;
    for (auto *child = ancestor.FirstChild(); child; child = child->NextSibling())
    {
      if (auto *text = DynamicDowncast<dom::Text>(*child); text && HasRenderedText(*text))
      {
        safeToRemoveAncestor = false;
        break;
      }

      if (Is<dom::Element>(*child))
      {
        safeToRemoveAncestor = false;
        break;
      }
    }

    if (safeToRemoveAncestor)
    {
      insertedNodes.WillRemoveNode(&ancestor);
      RemoveNode(ancestor);
    }
  }

  void ReplaceSelectionCommand::RemoveUnrenderedTextNodesAtEnds(InsertedNodes &insertedNodes) noexcept
  {
    (void)GetDocument().UpdateLayoutIgnorePendingStylesheets();

    RefPtr<dom::Node> lastLeafInserted = ShareRefPtr(insertedNodes.LastLeafInserted());
    if (auto *text = DynamicDowncast<dom::Text>(lastLeafInserted.get());
        text && !HasRenderedText(*text)
        && !EnclosingElementWithTag(FirstPositionInOrBeforeNode(lastLeafInserted.get()),
                                    html::TagName::select)
        && !EnclosingElementWithTag(FirstPositionInOrBeforeNode(lastLeafInserted.get()),
                                    html::TagName::script))
    {
      insertedNodes.WillRemoveNode(lastLeafInserted.get());
      RemoveNode(*lastLeafInserted);
    }

    (void)GetDocument().UpdateLayoutIgnorePendingStylesheets();

    // We don't have to make sure that firstNodeInserted isn't inside a select or script element
    // because it is a top level node in the fragment and the user can't insert into those elements.
    RefPtr<dom::Node> firstNodeInserted = ShareRefPtr(insertedNodes.FirstNodeInserted());
    if (auto *text = DynamicDowncast<dom::Text>(firstNodeInserted.get()); text && !HasRenderedText(*text))
    {
      insertedNodes.WillRemoveNode(firstNodeInserted.get());
      RemoveNode(*firstNodeInserted);
    }
  }

  VisiblePosition ReplaceSelectionCommand::PositionAtEndOfInsertedContent() const noexcept
  {
    // FIXME: Why is this hack here?  What's special about <select> tags?
    auto *enclosingSelect = EnclosingElementWithTag(_endOfInsertedContent, html::TagName::select);
    return enclosingSelect ? VisiblePosition(LastPositionInOrAfterNode(enclosingSelect))
                           : VisiblePosition(_endOfInsertedContent);
  }

  VisiblePosition ReplaceSelectionCommand::PositionAtStartOfInsertedContent() const noexcept
  {
    return _startOfInsertedContent;
  }

  namespace
  {
    // Remove style spans before insertion if they are unnecessary.  It's faster because we'll
    // avoid doing a layout.
    bool HandleStyleSpansBeforeInsertion(ReplacementFragment &fragment,
                                         const dom::Position &insertionPos) noexcept
    {
      RefPtr<dom::Node> topNode = ShareRefPtr(fragment.FirstChild());
      if (!topNode)
      {
        return false;
      }

      // Handling the case where we are doing Paste as Quotation or pasting into quoted content is more
      // complicated (see handleStyleSpans) and doesn't receive the optimization.
      if (IsMailPasteAsQuotationNode(*topNode)
          || EnclosingNodeOfType(FirstPositionInOrBeforeNode(topNode.get()), IsMailBlockquote,
                                 EditingBoundaryCrossingRule::CanCrossEditingBoundary))
      {
        return false;
      }

      // Either there are no style spans in the fragment or a WebKit client has added content to the fragment
      // before inserting it.  Look for and handle style spans after insertion.
      if (!IsLegacyAppleStyleSpan(topNode.get()))
      {
        return false;
      }

      auto &wrappingStyleSpan = Downcast<dom::Element>(*topNode);
      auto styleAtInsertionPos = EditingStyle::Create(insertionPos.ParentAnchoredEquivalent());
      auto styleText = styleAtInsertionPos->Style()->AsText(css::DefaultSerialisationContext());

      // FIXME: This string comparison is a naive way of comparing two styles.
      // We should be taking the diff and check that the diff is empty.
      if (styleText
          != wrappingStyleSpan.GetAttribute(dom::DOMStringAtom(Attributes::Style)).value_or(dom::DOMString()))
      {
        return false;
      }

      fragment.RemoveNodePreservingChildren(wrappingStyleSpan);
      return true;
    }
  }

  // At copy time, WebKit wraps copied content in a span that contains the source document's
  // default styles.  If the copied Range inherits any other styles from its ancestors, we put
  // those styles on a second span.
  // This function removes redundant styles from those spans, and removes the spans if all their
  // styles are redundant.
  // We should remove the Apple-style-span class when we're done, see <rdar://problem/5685600>.
  // We should remove styles from spans that are overridden by all of their children, either here
  // or at copy time.
  void ReplaceSelectionCommand::HandleStyleSpans(InsertedNodes &insertedNodes) noexcept
  {
    RefPtr<html::HTMLElement> wrappingStyleSpan;
    // The style span that contains the source document's default style should be at
    // the top of the fragment, but Mail sometimes adds a wrapper (for Paste As Quotation),
    // so search for the top level style span instead of assuming it's at the top.
    for (auto *node = insertedNodes.FirstNodeInserted(); node; node = TreeQueries::Next(*node))
    {
      if (IsLegacyAppleStyleSpan(node))
      {
        wrappingStyleSpan = ShareRefPtr(&Downcast<html::HTMLElement>(*node));
        break;
      }
    }

    // There might not be any style spans if we're pasting from another application or if
    // we are here because of a document.execCommand("InsertHTML", ...) call.
    if (!wrappingStyleSpan)
    {
      return;
    }

    auto style = EditingStyle::Create(wrappingStyleSpan->InlineStyle());
    RefPtr<dom::Node> context = ShareRefPtr<dom::Node>(wrappingStyleSpan->ParentNode());

    // If Mail wraps the fragment with a Paste as Quotation blockquote, or if you're pasting into a quoted
    // region, styles from blockquoteNode are allowed to override those from the source document, see
    // <rdar://problem/4930986> and <rdar://problem/5089327>.
    RefPtr<dom::Node> blockquoteNode;
    if (context && IsMailPasteAsQuotationNode(*context))
    {
      blockquoteNode = context;
    }
    else
    {
      blockquoteNode = EnclosingNodeOfType(dom::FirstPositionInNode(context.get()), IsMailBlockquote,
                                           EditingBoundaryCrossingRule::CanCrossEditingBoundary);
    }
    if (blockquoteNode)
    {
      context = ShareRefPtr<dom::Node>(TreeQueries::DocumentElement(GetDocument()));
    }

    // This operation requires that only editing styles to be removed from sourceDocumentStyle.
    style->PrepareToApplyAt(dom::FirstPositionInNode(context.get()));

    // Remove block properties in the span's style. This prevents properties that probably have no effect
    // currently from affecting blocks later if the style is cloned for a new block element during a future
    // editing operation.
    // FIXME: They *can* have an effect currently if blocks beneath the style span aren't individually marked
    // with block styles by the editing engine used to style them.  WebKit doesn't do this, but others might.
    style->RemoveBlockProperties();

    if (style->IsEmpty() || !wrappingStyleSpan->FirstChild())
    {
      insertedNodes.WillRemoveNodePreservingChildren(wrappingStyleSpan.get());
      RemoveNodePreservingChildren(*wrappingStyleSpan);
    }
    else
    {
      SetNodeAttribute(*wrappingStyleSpan, dom::DOMStringAtom(Attributes::Style),
                       style->Style()->AsText(css::DefaultSerialisationContext()));
    }
  }

  void ReplaceSelectionCommand::MergeEndIfNeeded() noexcept
  {
    if (!_shouldMergeEnd)
    {
      return;
    }

    VisiblePosition startOfInsertedContent(PositionAtStartOfInsertedContent());
    VisiblePosition endOfInsertedContent(PositionAtEndOfInsertedContent());

    // Bail to avoid infinite recursion.
    if (_movingParagraph)
    {
      krys_debug_assert(false);
      return;
    }

    krys_debug_assert(startOfInsertedContent.IsNull() == endOfInsertedContent.IsNull());
    if (startOfInsertedContent.IsNull() || endOfInsertedContent.IsNull())
    {
      return;
    }

    // Merging two paragraphs will destroy the moved one's block styles.  Always move the end of inserted
    // forward to preserve the block style of the paragraph already in the document, unless the paragraph to
    // move would include the what was the start of the selection that was pasted into, so that we preserve
    // that paragraph's block styles.
    bool mergeForward = !(InSameParagraph(startOfInsertedContent, endOfInsertedContent)
                          && !IsStartOfParagraph(startOfInsertedContent));

    VisiblePosition destination = mergeForward ? endOfInsertedContent.Next() : endOfInsertedContent;
    VisiblePosition startOfParagraphToMove =
      mergeForward ? StartOfParagraph(endOfInsertedContent) : endOfInsertedContent.Next();

    // Merging forward could result in deleting the destination anchor node.
    // To avoid this, we add a placeholder node before the start of the paragraph.
    if (EndOfParagraph(startOfParagraphToMove) == destination)
    {
      auto placeholder = CreateHTMLElement(GetDocument(), u8"br");
      (void)InsertNodeBefore(*placeholder, *startOfParagraphToMove.DeepEquivalent().DeprecatedNode());
      destination = VisiblePosition(dom::PositionBeforeNode(placeholder.get()));
    }

    MoveParagraph(startOfParagraphToMove, EndOfParagraph(startOfParagraphToMove), destination);

    // Merging forward will remove m_endOfInsertedContent from the document.
    if (mergeForward)
    {
      if (_startOfInsertedContent.IsOrphan())
      {
        _startOfInsertedContent = EndingSelection().VisibleStart().DeepEquivalent();
      }
      _endOfInsertedContent = EndingSelection().VisibleEnd().DeepEquivalent();
      // If we merged text nodes, m_endOfInsertedContent could be null. If this is the case, we use
      // m_startOfInsertedContent.
      if (_endOfInsertedContent.IsNull())
      {
        _endOfInsertedContent = _startOfInsertedContent;
      }
    }
  }

  bool ReplaceSelectionCommand::WillApplyCommand() noexcept
  {
    Ref<dom::DocumentFragment> documentFragment = ShareRef(*_documentFragment);
    _documentFragmentPlainText = documentFragment->TextContent().value_or(dom::DOMString());
    _documentFragmentHTMLMarkup = SerializeFragment(*documentFragment, SerializedNodes::SubtreeIncludingNode);
    (void)EnsureReplacementFragment();
    return CompositeEditCommand::WillApplyCommand();
  }

  // FIXME: Matching the text direction of each pasted paragraph to its content when bidi content aware
  // pasting is enabled (updateDirectionForStartOfInsertedContentIfNeeded), with the bidiContentAwarePaste
  // setting.
  void ReplaceSelectionCommand::DoApply() noexcept
  {
    VisibleSelection selection = EndingSelection();
    krys_debug_assert(selection.IsCaretOrRange());
    krys_debug_assert(selection.Start().DeprecatedNode());
    if (selection.IsNoneOrOrphaned() || !selection.Start().DeprecatedNode() || !selection.IsContentEditable())
    {
      return;
    }

    // In plain text only regions, we create style-less fragments, so the inserted content will automatically
    // match the style of the surrounding area and so we can avoid unnecessary work below for m_matchStyle.
    if (!selection.IsContentRichlyEditable())
    {
      _matchStyle = false;
    }

    ReplacementFragment &fragment = *EnsureReplacementFragment();
    if (PerformTrivialReplace(fragment))
    {
      return;
    }

    // We can skip matching the style if the selection is plain text.
    if ((selection.Start().DeprecatedNode()->Renderer()
         && selection.Start().DeprecatedNode()->Renderer()->Style().UsedUserModify()
              == UserModify::ReadWritePlaintextOnly)
        && (selection.End().DeprecatedNode()->Renderer()
            && selection.End().DeprecatedNode()->Renderer()->Style().UsedUserModify()
                 == UserModify::ReadWritePlaintextOnly))
    {
      _matchStyle = false;
    }

    if (_matchStyle)
    {
      _insertionStyle = EditingStyle::Create(selection.Start());
      _insertionStyle->MergeTypingStyle(GetDocument());
    }

    VisiblePosition visibleStart = selection.VisibleStart();
    VisiblePosition visibleEnd = selection.VisibleEnd();

    bool selectionEndWasEndOfParagraph = IsEndOfParagraph(visibleEnd);
    bool selectionStartWasStartOfParagraph = IsStartOfParagraph(visibleStart);

    auto startBlock = EnclosingBlock(ShareRefPtr(visibleStart.DeepEquivalent().DeprecatedNode()));

    dom::Position insertionPos = selection.Start();
    bool shouldHandleMailBlockquote =
      EnclosingNodeOfType(insertionPos, IsMailBlockquote,
                          EditingBoundaryCrossingRule::CanCrossEditingBoundary)
      && !_ignoreMailBlockquote;
    bool selectionIsPlainText = !selection.IsContentRichlyEditable();
    RefPtr<dom::Element> currentRoot = ShareRefPtr(selection.RootEditableElement());

    if ((selectionStartWasStartOfParagraph && selectionEndWasEndOfParagraph && !shouldHandleMailBlockquote)
        || startBlock == currentRoot || (startBlock && IsListItem(*startBlock)) || selectionIsPlainText)
    {
      _preventNesting = false;
    }

    if (selection.IsRange())
    {
      // When the end of the selection being pasted into is at the end of a paragraph, and that selection
      // spans multiple blocks, not merging may leave an empty line.
      // When the start of the selection being pasted into is at the start of a block, not merging
      // will leave hanging block(s).
      // Merge blocks if the start of the selection was in a Mail blockquote, since we handle
      // that case specially to prevent nesting.
      bool mergeBlocksAfterDelete =
        shouldHandleMailBlockquote || IsEndOfParagraph(visibleEnd) || IsStartOfBlock(visibleStart);
      // FIXME: We should only expand to include fully selected special elements if we are copying a
      // selection and pasting it on top of itself.
      // FIXME: capturing the content of this delete would allow a replace accessibility notification instead
      // of a simple insert
      DeleteSelection(false, mergeBlocksAfterDelete, true, false, true);
      visibleStart = EndingSelection().VisibleStart();
      if (fragment.HasInterchangeNewlineAtStart())
      {
        if (IsEndOfParagraph(visibleStart) && !IsStartOfParagraph(visibleStart))
        {
          if (!IsEndOfEditableOrNonEditableContent(visibleStart))
          {
            SetEndingSelection(VisibleSelection(visibleStart.Next()));
          }
        }
        else
        {
          InsertParagraphSeparator();
        }
      }
      insertionPos = EndingSelection().Start();
    }
    else
    {
      krys_debug_assert(selection.IsCaret());
      if (fragment.HasInterchangeNewlineAtStart())
      {
        VisiblePosition next = visibleStart.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
        if (IsEndOfParagraph(visibleStart) && !IsStartOfParagraph(visibleStart) && next.IsNotNull())
        {
          SetEndingSelection(VisibleSelection(next));
        }
        else
        {
          InsertParagraphSeparator();
          visibleStart = EndingSelection().VisibleStart();
        }
      }
      // We split the current paragraph in two to avoid nesting the blocks from the fragment inside the
      // current block. For example paste <div>foo</div><div>bar</div><div>baz</div> into <div>x^x</div>,
      // where ^ is the caret. As long as the  div styles are the same, visually you'd expect:
      // <div>xbar</div><div>bar</div><div>bazx</div>, not <div>xbar<div>bar</div><div>bazx</div></div>. Don't
      // do this if the selection started in a Mail blockquote.
      if (_preventNesting && !shouldHandleMailBlockquote && !IsEndOfParagraph(visibleStart)
          && !IsStartOfParagraph(visibleStart))
      {
        InsertParagraphSeparator();
        SetEndingSelection(VisibleSelection(EndingSelection().VisibleStart().Previous()));
      }
      insertionPos = EndingSelection().Start();
    }

    // We don't want any of the pasted content to end up nested in a Mail blockquote, so first break
    // out of any surrounding Mail blockquotes. Unless we're inserting in a table, in which case
    // breaking the blockquote will prevent the content from actually being inserted in the table.
    if (shouldHandleMailBlockquote && _preventNesting
        && !(EnclosingNodeOfType(insertionPos, &IsTableStructureNode)))
    {
      ApplyCommandToComposite(BreakBlockquoteCommand::Create(GetDocument()));
      // This will leave a br between the split.
      if (RefPtr<dom::Node> br = ShareRefPtr(EndingSelection().Start().DeprecatedNode()))
      {
        krys_debug_assert(HasTagName(br.get(), html::TagName::br));
        insertionPos = dom::PositionInParentBeforeNode(br.get());
        RemoveNode(*br);
      }
    }

    // Inserting content could cause whitespace to collapse, e.g. inserting <div>foo</div> into hello^ world.
    PrepareWhitespaceAtPositionForSplit(insertionPos);

    // If the downstream node has been removed there's no point in continuing.
    if (!insertionPos.Downstream().DeprecatedNode())
    {
      return;
    }

    // NOTE: This would be an incorrect usage of downstream() if downstream() were changed to mean the last
    // position after p that maps to the same visible position as p (since in the case where a br is at the
    // end of a block and collapsed away, there are positions after the br which map to the same visible
    // position as [br, 0]).
    RefPtr<dom::Node> endBR = HasTagName(insertionPos.Downstream().DeprecatedNode(), html::TagName::br)
                                ? ShareRefPtr(insertionPos.Downstream().DeprecatedNode())
                                : nullptr;
    VisiblePosition originalVisPosBeforeEndBR;
    if (endBR)
    {
      originalVisPosBeforeEndBR = VisiblePosition(dom::PositionBeforeNode(endBR.get())).Previous();
    }

    RefPtr<dom::Node> insertionBlock = EnclosingBlock(ShareRefPtr(insertionPos.DeprecatedNode()));

    // Adjust insertionPos to prevent nesting.
    // If the start was in a Mail blockquote, we will have already handled adjusting insertionPos above.
    if (_preventNesting && insertionBlock && insertionBlock != currentRoot && !IsTableCell(*insertionBlock)
        && !shouldHandleMailBlockquote)
    {
      VisiblePosition visibleInsertionPos(insertionPos);
      if (IsEndOfBlock(visibleInsertionPos)
          && !(IsStartOfBlock(visibleInsertionPos) && fragment.HasInterchangeNewlineAtEnd()))
      {
        insertionPos = dom::PositionInParentAfterNode(insertionBlock.get());
      }
      else if (IsStartOfBlock(visibleInsertionPos))
      {
        insertionPos = dom::PositionInParentBeforeNode(insertionBlock.get());
      }
    }

    // Paste at start or end of link goes outside of link.
    insertionPos = PositionAvoidingSpecialElementBoundary(insertionPos);

    // FIXME: Can this wait until after the operation has been performed?  There doesn't seem to be
    // any work performed after this that queries or uses the typing style.
    GetDocument().Selection().ClearTypingStyle();

    // We don't want the destination to end up inside nodes that weren't selected.  To avoid that, we move the
    // position forward without changing the visible position so we're still at the same visible location, but
    // outside of preceding tags.
    insertionPos = PositionAvoidingPrecedingNodes(insertionPos);

    // Paste into run of tabs splits the tab span.
    insertionPos = PositionOutsideTabSpan(insertionPos);

    bool hasBlankLinesBetweenParagraphs = HasBlankLineBetweenParagraphs(insertionPos);
    bool handledStyleSpans = HandleStyleSpansBeforeInsertion(fragment, insertionPos);

    // We're finished if there is nothing to add.
    if (fragment.IsEmpty() || !fragment.FirstChild())
    {
      return;
    }

    // If we are not trying to match the destination style we prefer a position
    // that is outside inline elements that provide style.
    // This way we can produce a less verbose markup.
    // We can skip this optimization for fragments not wrapped in one of
    // our style spans and for positions inside list items
    // since insertAsListItems already does the right thing.
    if (!_matchStyle && !EnclosingList(insertionPos.GetContainerNode()))
    {
      if (auto *containerNode = insertionPos.GetContainerNode())
      {
        if (Is<dom::Text>(*containerNode) && insertionPos.OffsetInContainerNode()
            && !insertionPos.AtLastEditingPositionForNode())
        {
          SplitTextNode(*insertionPos.ContainerText(),
                        static_cast<uint32>(insertionPos.OffsetInContainerNode()));
          insertionPos = dom::FirstPositionInNode(insertionPos.GetContainerNode());
        }
      }

      if (RefPtr<dom::Node> nodeToSplitTo = NodeToSplitToAvoidPastingIntoInlineNodesWithStyle(insertionPos))
      {
        if (nodeToSplitTo->ParentNode() && insertionPos.GetContainerNode() != nodeToSplitTo->ParentNode())
        {
          RefPtr<dom::Node> splitStart = ShareRefPtr(insertionPos.ComputeNodeAfterPosition());
          if (!splitStart)
          {
            splitStart = ShareRefPtr(insertionPos.GetContainerNode());
          }
          krys_debug_assert(splitStart);
          nodeToSplitTo = SplitTreeToNode(*splitStart, *nodeToSplitTo->ParentNode());
          insertionPos = dom::PositionInParentBeforeNode(nodeToSplitTo.get());
        }
      }
    }

    // FIXME: When pasting rich content we're often prevented from heading down the fast path by style spans.
    // Try again here if they've been removed.

    // 1) Insert the content.
    // 2) Remove redundant styles and style tags, this inner <b> for example: <b>foo <b>bar</b> baz</b>.
    // 3) Merge the start of the added content with the content before the position being pasted into.
    // 4) Do one of the following: a) expand the last br if the fragment ends with one and it collapsed,
    // b) merge the last paragraph of the incoming fragment with the paragraph that contained the
    // end of the selection that was pasted into, or c) handle an interchange newline at the end of the
    // incoming fragment.
    // 5) Add spaces for smart replace.
    // 6) Select the replacement if requested, and match style if requested.

    InsertedNodes insertedNodes;
    RefPtr<dom::Node> refNode = ShareRefPtr(fragment.FirstChild());
    RefPtr<dom::Node> node = ShareRefPtr(refNode->NextSibling());

    if (refNode)
    {
      fragment.RemoveNode(*refNode);
    }

    auto blockStart = EnclosingBlock(ShareRefPtr(insertionPos.DeprecatedNode()));
    bool isInsertingIntoList =
      (IsListHTMLElement(refNode.get())
       || (IsLegacyAppleStyleSpan(refNode.get()) && IsListHTMLElement(refNode->FirstChild())))
      && blockStart && blockStart->Renderer()->IsRenderListItem()
      && blockStart->ParentNode()->HasEditableStyle();
    if (isInsertingIntoList)
    {
      refNode =
        InsertAsListItems(Downcast<dom::Element>(*refNode), blockStart.get(), insertionPos, insertedNodes);
    }
    else if (IsEditablePosition(insertionPos))
    {
      InsertNodeAt(*refNode, insertionPos);
      insertedNodes.RespondToNodeInsertion(refNode.get());
    }

    // Mutation events (bug 22634) may have already removed the inserted content
    if (!refNode->IsConnected())
    {
      return;
    }

    bool plainTextFragment = IsPlainTextMarkup(refNode.get());

    while (node)
    {
      RefPtr<dom::Node> next = ShareRefPtr(node->NextSibling());
      fragment.RemoveNode(*node);
      InsertNodeAfter(*node, *refNode);
      insertedNodes.RespondToNodeInsertion(node.get());

      // Mutation events (bug 22634) may have already removed the inserted content
      if (!node->IsConnected())
      {
        return;
      }

      refNode = node;
      if (node && plainTextFragment)
      {
        plainTextFragment = IsPlainTextMarkup(node.get());
      }
      node = next;
    }

    if (insertedNodes.IsEmpty())
    {
      return;
    }

    // removeUnrenderedTextNodesAtEnds can trigger layout, so for performance, only do it when the presence of
    // richly-editable text requires us to.
    if (IsRichlyEditablePosition(insertionPos))
    {
      RemoveUnrenderedTextNodesAtEnds(insertedNodes);
    }

    if (!handledStyleSpans)
    {
      HandleStyleSpans(insertedNodes);
    }

    // Mutation events (bug 20161) may have already removed the inserted content
    if (insertedNodes.IsEmpty())
    {
      return;
    }
    if (!insertedNodes.FirstNodeInserted()->IsConnected())
    {
      return;
    }

    VisiblePosition startOfInsertedContent = FirstPositionInOrBeforeNode(insertedNodes.FirstNodeInserted());

    // We inserted before the insertionBlock to prevent nesting, and the content before the insertionBlock
    // wasn't in its own block and didn't have a br after it, so the inserted content ended up in the same
    // paragraph.
    if (!startOfInsertedContent.IsNull() && insertionBlock
        && insertionPos.DeprecatedNode() == insertionBlock->ParentNode()
        && static_cast<uint32>(insertionPos.DeprecatedEditingOffset()) < insertionBlock->ComputeNodeIndex()
        && !IsStartOfParagraph(startOfInsertedContent))
    {
      InsertNodeAt(*CreateHTMLElement(GetDocument(), u8"br"), startOfInsertedContent.DeepEquivalent());
    }

    if (endBR
        && (plainTextFragment
            || (ShouldRemoveEndBR(endBR.get(), originalVisPosBeforeEndBR)
                && !(fragment.HasInterchangeNewlineAtEnd() && selectionIsPlainText))))
    {
      RefPtr<dom::ContainerNode> parent = ShareRefPtr(endBR->ParentNode());
      insertedNodes.WillRemoveNode(endBR.get());
      RemoveNode(*endBR);
      (void)GetDocument().UpdateLayoutIgnorePendingStylesheets();
      if (auto nodeToRemove = HighestNodeToRemoveInPruning(parent.get()))
      {
        insertedNodes.WillRemovePossibleAncestorNode(nodeToRemove.get());
        RemoveNode(*nodeToRemove);
      }
    }

    if (insertedNodes.IsEmpty())
    {
      return;
    }

    MakeInsertedContentRoundTrippableWithHTMLTreeBuilder(insertedNodes);
    if (insertedNodes.IsEmpty())
    {
      return;
    }
    if (!insertedNodes.FirstNodeInserted()->IsConnected())
    {
      return;
    }

    RemoveRedundantStylesAndKeepStyleSpanInline(insertedNodes);
    if (insertedNodes.IsEmpty())
    {
      return;
    }

    if (_sanitizeFragment)
    {
      ApplyCommandToComposite(SimplifyMarkupCommand::Create(GetDocument(), insertedNodes.FirstNodeInserted(),
                                                            insertedNodes.PastLastLeaf()));
    }

    // Setup m_startOfInsertedContent and m_endOfInsertedContent. This should be the last two lines of code
    // that access insertedNodes.
    _startOfInsertedContent = FirstPositionInOrBeforeNode(insertedNodes.FirstNodeInserted());
    _endOfInsertedContent = LastPositionInOrAfterNode(insertedNodes.LastLeafInserted());

    // Determine whether or not we should merge the end of inserted content with what's after it before we do
    // the start merge so that the start merge doesn't effect our decision.
    _shouldMergeEnd = ShouldMergeEnd(selectionEndWasEndOfParagraph);

    if (ShouldMergeStart(selectionStartWasStartOfParagraph, fragment.HasInterchangeNewlineAtStart(),
                         shouldHandleMailBlockquote))
    {
      VisiblePosition startOfParagraphToMove = PositionAtStartOfInsertedContent();
      VisiblePosition destination = startOfParagraphToMove.Previous();
      // We need to handle the case where we need to merge the end
      // but our destination node is inside an inline that is the last in the block.
      // We insert a placeholder before the newly inserted content to avoid being merged into the inline.
      RefPtr<dom::Node> destinationNode = ShareRefPtr(destination.DeepEquivalent().DeprecatedNode());
      if (_shouldMergeEnd && EnclosingInline(destinationNode.get()) != destinationNode
          && EnclosingInline(destinationNode.get())->NextSibling())
      {
        (void)InsertNodeBefore(*CreateHTMLElement(GetDocument(), u8"br"), *refNode);
      }

      // Merging the first paragraph of inserted content with the content that came
      // before the selection that was pasted into would also move content after
      // the selection that was pasted into if: only one paragraph was being pasted,
      // and it was not wrapped in a block, the selection that was pasted into ended
      // at the end of a block and the next paragraph didn't start at the start of a block.
      // Insert a line break just after the inserted content to separate it from what
      // comes after and prevent that from happening.
      VisiblePosition endOfInsertedContent = PositionAtEndOfInsertedContent();
      if (StartOfParagraph(endOfInsertedContent) == startOfParagraphToMove)
      {
        InsertNodeAt(*CreateHTMLElement(GetDocument(), u8"br"), endOfInsertedContent.DeepEquivalent());
        // Mutation events (bug 22634) triggered by inserting the <br> might have removed the content we're
        // about to move
        if (!startOfParagraphToMove.DeepEquivalent().AnchorNode()->IsConnected())
        {
          return;
        }
      }

      // FIXME: Maintain positions for the start and end of inserted content instead of keeping nodes.  The
      // nodes are only ever used to create positions where inserted content starts/ends.
      MoveParagraph(startOfParagraphToMove, EndOfParagraph(startOfParagraphToMove), destination);
      _startOfInsertedContent = EndingSelection().VisibleStart().DeepEquivalent().Downstream();
      if (_endOfInsertedContent.IsOrphan())
      {
        _endOfInsertedContent = EndingSelection().VisibleEnd().DeepEquivalent().Upstream();
      }
    }

    dom::Position lastPositionToSelect;
    if (fragment.HasInterchangeNewlineAtEnd())
    {
      VisiblePosition endOfInsertedContent = PositionAtEndOfInsertedContent();
      VisiblePosition next =
        endOfInsertedContent.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);

      if (selectionEndWasEndOfParagraph || !IsEndOfParagraph(endOfInsertedContent) || next.IsNull())
      {
        if (!IsStartOfParagraph(endOfInsertedContent))
        {
          SetEndingSelection(VisibleSelection(endOfInsertedContent));
          auto enclosingNode =
            EnclosingBlock(ShareRefPtr(endOfInsertedContent.DeepEquivalent().DeprecatedNode()));
          if (enclosingNode && IsListItem(*enclosingNode))
          {
            auto newListItem = CreateHTMLElement(GetDocument(), u8"li");
            InsertNodeAfter(*newListItem, *enclosingNode);
            SetEndingSelection(
              VisibleSelection(VisiblePosition(dom::FirstPositionInNode(newListItem.get()))));
          }
          else
          {
            // Use a default paragraph element (a plain div) for the empty paragraph, using the last paragraph
            // block's style seems to annoy users.
            InsertParagraphSeparator(
              true, !shouldHandleMailBlockquote
                      && HighestEnclosingNodeOfType(endOfInsertedContent.DeepEquivalent(), IsMailBlockquote,
                                                    EditingBoundaryCrossingRule::CannotCrossEditingBoundary,
                                                    insertedNodes.FirstNodeInserted()->ParentNode()));
          }

          // Select up to the paragraph separator that was added.
          lastPositionToSelect = EndingSelection().VisibleStart().DeepEquivalent();
          UpdateNodesInserted(lastPositionToSelect.DeprecatedNode());
        }
      }
      else
      {
        // Select up to the beginning of the next paragraph.
        lastPositionToSelect = next.DeepEquivalent().Downstream();
      }
    }
    else
    {
      MergeEndIfNeeded();
    }

    if (auto mailBlockquote = EnclosingNodeOfType(PositionAtStartOfInsertedContent().DeepEquivalent(),
                                                  IsMailPasteAsQuotationNode))
    {
      RemoveNodeAttribute(Downcast<dom::Element>(*mailBlockquote), dom::DOMStringAtom(Attributes::Class));
    }

    if (ShouldPerformSmartReplace())
    {
      AddSpacesForSmartReplace();
    }

    if (!isInsertingIntoList && hasBlankLinesBetweenParagraphs && ShouldPerformSmartParagraphReplace())
    {
      AddNewLinesForSmartReplace();
    }

    // If we are dealing with a fragment created from plain text
    // no style matching is necessary.
    if (plainTextFragment)
    {
      _matchStyle = false;
    }

    CompleteHTMLReplacement(lastPositionToSelect);
  }

  dom::DOMString ReplaceSelectionCommand::InputEventData() const noexcept
  {
    if (IsEditingTextAreaOrTextInput())
    {
      return _documentFragment->TextContent().value_or(dom::DOMString());
    }

    return CompositeEditCommand::InputEventData();
  }

  RefPtr<dom::DataTransfer> ReplaceSelectionCommand::InputEventDataTransfer() const noexcept
  {
    if (IsEditingTextAreaOrTextInput())
    {
      return CompositeEditCommand::InputEventDataTransfer();
    }

    return dom::DataTransfer::CreateForInputEvent(_documentFragmentPlainText, _documentFragmentHTMLMarkup);
  }

  bool ReplaceSelectionCommand::ShouldRemoveEndBR(dom::Node *endBR,
                                                  const VisiblePosition &originalVisPosBeforeEndBR) noexcept
  {
    if (!endBR || !endBR->IsConnected())
    {
      return false;
    }

    VisiblePosition visiblePos(dom::PositionBeforeNode(endBR));

    // Don't remove the br if nothing was inserted.
    if (visiblePos.Previous() == originalVisPosBeforeEndBR)
    {
      return false;
    }

    // Remove the br if it is collapsed away and so is unnecessary.
    if (!GetDocument().InNoQuirksMode() && IsEndOfBlock(visiblePos) && !IsStartOfParagraph(visiblePos))
    {
      return true;
    }

    // A br that was originally holding a line open should be displaced by inserted content or turned into a
    // line break. A br that was originally acting as a line break should still be acting as a line break, not
    // as a placeholder.
    return IsStartOfParagraph(visiblePos) && IsEndOfParagraph(visiblePos);
  }

  bool ReplaceSelectionCommand::ShouldPerformSmartReplace() const noexcept
  {
    if (!_smartReplace)
    {
      return false;
    }

    auto *textControl = html::EnclosingTextFormControl(PositionAtStartOfInsertedContent().DeepEquivalent());
    if (auto *input = DynamicDowncast<html::HTMLInputElement>(textControl); input && input->IsPasswordField())
    {
      return false; // Disable smart replace for password fields.
    }

    return true;
  }

  bool ReplaceSelectionCommand::ShouldPerformSmartParagraphReplace() const noexcept
  {
    if (!_smartReplace)
    {
      return false;
    }

    if (!GetDocument().GetEditor().Behavior().ShouldSmartInsertDeleteParagraphs())
    {
      return false;
    }

    return true;
  }

  void ReplaceSelectionCommand::AddNewLinesForSmartReplace() noexcept
  {
    VisiblePosition startOfInsertedContent = PositionAtStartOfInsertedContent();
    VisiblePosition endOfInsertedContent = PositionAtEndOfInsertedContent();

    bool isPastedContentEntireParagraphs =
      IsStartOfParagraph(startOfInsertedContent) && IsEndOfParagraph(endOfInsertedContent);

    // If we aren't pasting a paragraph, no need to attempt to insert newlines.
    if (!isPastedContentEntireParagraphs)
    {
      return;
    }

    bool reachedBoundaryStart = false;
    bool reachedBoundaryEnd = false;
    VisiblePosition positionBeforeStart = startOfInsertedContent.Previous(
      EditingBoundaryCrossingRule::CannotCrossEditingBoundary, &reachedBoundaryStart);
    VisiblePosition positionAfterEnd =
      endOfInsertedContent.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary, &reachedBoundaryEnd);

    if (!reachedBoundaryStart && !reachedBoundaryEnd)
    {
      if (!IsBlankParagraph(positionBeforeStart) && !IsBlankParagraph(startOfInsertedContent)
          && IsEndOfLine(positionBeforeStart) && !IsEndOfEditableOrNonEditableContent(positionAfterEnd)
          && !IsEndOfEditableOrNonEditableContent(endOfInsertedContent))
      {
        SetEndingSelection(VisibleSelection(startOfInsertedContent));
        InsertParagraphSeparator();
        auto newStart = EndingSelection().VisibleStart().Previous(
          EditingBoundaryCrossingRule::CannotCrossEditingBoundary, &reachedBoundaryStart);
        if (!reachedBoundaryStart)
        {
          _startOfInsertedContent = newStart.DeepEquivalent();
        }
      }
    }

    reachedBoundaryStart = false;
    reachedBoundaryEnd = false;
    positionAfterEnd =
      endOfInsertedContent.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary, &reachedBoundaryEnd);
    positionBeforeStart = startOfInsertedContent.Previous(
      EditingBoundaryCrossingRule::CannotCrossEditingBoundary, &reachedBoundaryStart);

    if (!reachedBoundaryEnd && !reachedBoundaryStart)
    {
      if (!IsBlankParagraph(positionAfterEnd) && !IsBlankParagraph(endOfInsertedContent)
          && IsStartOfLine(positionAfterEnd) && !IsEndOfLine(positionAfterEnd)
          && !IsEndOfEditableOrNonEditableContent(positionAfterEnd))
      {
        SetEndingSelection(VisibleSelection(endOfInsertedContent));
        InsertParagraphSeparator();
        _endOfInsertedContent = EndingSelection().Start();
      }
    }
  }

  // FIXME: An editing text node (createEditingTextNode, which echoes passwords), with password fields.
  void ReplaceSelectionCommand::AddSpacesForSmartReplace() noexcept
  {
    VisiblePosition startOfInsertedContent = PositionAtStartOfInsertedContent();
    VisiblePosition endOfInsertedContent = PositionAtEndOfInsertedContent();

    dom::Position endUpstream = endOfInsertedContent.DeepEquivalent().Upstream();
    RefPtr<dom::Node> endNode = ShareRefPtr(endUpstream.ComputeNodeBeforePosition());
    auto *endTextNode = DynamicDowncast<dom::Text>(endNode.get());
    auto endOffset = endTextNode ? static_cast<uint32>(endTextNode->Length()) : 0u;
    if (endUpstream.GetAnchorType() == dom::Position::AnchorType::PositionIsOffsetInAnchor)
    {
      endNode = ShareRefPtr(endUpstream.GetContainerNode());
      endOffset = static_cast<uint32>(endUpstream.OffsetInContainerNode());
    }

    bool needsTrailingSpace =
      !IsEndOfParagraph(endOfInsertedContent) && !IsStartOfParagraph(endOfInsertedContent)
      && !IsCharacterSmartReplaceExemptConsideringNonBreakingSpace(endOfInsertedContent.CharacterAfter(),
                                                                   false);
    if (needsTrailingSpace && endNode)
    {
      bool collapseWhiteSpace = !endNode->Renderer() || endNode->Renderer()->Style().CollapseWhiteSpace();
      if (auto *text = DynamicDowncast<dom::Text>(*endNode))
      {
        InsertTextIntoNode(*text, endOffset,
                           collapseWhiteSpace ? NonBreakingSpaceString() : utf8_stringview(u8" "));
        if (_endOfInsertedContent.GetContainerNode() == endNode)
        {
          _endOfInsertedContent.MoveToOffset(
            static_cast<uint32>(_endOfInsertedContent.OffsetInContainerNode() + 1));
        }
      }
      else
      {
        auto node = GetDocument().CreateTextNode(collapseWhiteSpace ? utf8_string(NonBreakingSpaceString())
                                                                    : utf8_string(u8" "));
        InsertNodeAfter(*node, *endNode);
        UpdateNodesInserted(node.get());
      }
    }

    (void)GetDocument().UpdateLayout();

    dom::Position startDownstream = startOfInsertedContent.DeepEquivalent().Downstream();
    RefPtr<dom::Node> startNode = ShareRefPtr(startDownstream.ComputeNodeAfterPosition());
    uint32 startOffset = 0;
    if (startDownstream.GetAnchorType() == dom::Position::AnchorType::PositionIsOffsetInAnchor)
    {
      startNode = ShareRefPtr(startDownstream.GetContainerNode());
      startOffset = static_cast<uint32>(startDownstream.OffsetInContainerNode());
    }

    bool needsLeadingSpace = !IsStartOfParagraph(startOfInsertedContent)
                             && !IsEndOfParagraph(startOfInsertedContent)
                             && !IsCharacterSmartReplaceExemptConsideringNonBreakingSpace(
                               startOfInsertedContent.Previous().CharacterAfter(), true);
    if (needsLeadingSpace && startNode)
    {
      bool collapseWhiteSpace = !startNode->Renderer() || startNode->Renderer()->Style().CollapseWhiteSpace();
      if (auto *text = DynamicDowncast<dom::Text>(*startNode))
      {
        InsertTextIntoNode(*text, startOffset,
                           collapseWhiteSpace ? NonBreakingSpaceString() : utf8_stringview(u8" "));
        if (_endOfInsertedContent.GetContainerNode() == startNode
            && _endOfInsertedContent.OffsetInContainerNode())
        {
          _endOfInsertedContent.MoveToOffset(
            static_cast<uint32>(_endOfInsertedContent.OffsetInContainerNode() + 1));
        }
      }
      else
      {
        auto node = GetDocument().CreateTextNode(collapseWhiteSpace ? utf8_string(NonBreakingSpaceString())
                                                                    : utf8_string(u8" "));
        // Don't updateNodesInserted. Doing so would set m_endOfInsertedContent to be the node containing the
        // leading space, but m_endOfInsertedContent is supposed to mark the end of pasted content.
        (void)InsertNodeBefore(*node, *startNode);
        _startOfInsertedContent = dom::FirstPositionInNode(node.get());
      }
    }
  }

  // FIXME: The accessibility selection of the inserted text (m_visibleSelectionForInsertedText), with
  // accessibility.
  void ReplaceSelectionCommand::CompleteHTMLReplacement(const dom::Position &lastPositionToSelect) noexcept
  {
    dom::Position start = PositionAtStartOfInsertedContent().DeepEquivalent();
    dom::Position end = PositionAtEndOfInsertedContent().DeepEquivalent();

    // Mutation events may have deleted start or end
    if (start.IsNotNull() && !start.IsOrphan() && end.IsNotNull() && !end.IsOrphan())
    {
      // FIXME (11475): Remove this and require that the creator of the fragment to use nbsps.
      // A rebalanced run can change length (a no-break space is two bytes), so the positions move across it.
      if (auto rebalance = RebalanceWhitespaceAt(start))
      {
        start = rebalance->Adjust(start);
        end = rebalance->Adjust(end);
      }
      if (auto rebalance = RebalanceWhitespaceAt(end))
      {
        start = rebalance->Adjust(start);
        end = rebalance->Adjust(end);
      }

      if (_matchStyle)
      {
        krys_debug_assert(_insertionStyle);
        ApplyStyle(_insertionStyle.get(), start, end);
        // applyStyle may clone content to new block wrappers and make anchor nodes orphan.
        if (start.IsOrphan() || end.IsOrphan())
        {
          start = EndingSelection().Start();
          end = EndingSelection().End();
          _startOfInsertedContent = start;
          _endOfInsertedContent = end;
        }
      }

      if (lastPositionToSelect.IsNotNull())
      {
        end = lastPositionToSelect;
      }

      MergeTextNodesAroundPosition(start, end);
      MergeTextNodesAroundPosition(end, start);
    }
    else if (lastPositionToSelect.IsNotNull())
    {
      start = end = lastPositionToSelect;
    }
    else
    {
      return;
    }

    if (_selectReplacement)
    {
      SetEndingSelection(VisibleSelection(start, end, VisiblePosition::DefaultAffinity,
                                          EndingSelection().GetDirectionality()));
    }
    else
    {
      SetEndingSelection(
        VisibleSelection(end, VisiblePosition::DefaultAffinity, EndingSelection().GetDirectionality()));
    }
  }

  void ReplaceSelectionCommand::MergeTextNodesAroundPosition(dom::Position &position,
                                                             dom::Position &positionOnlyToBeUpdated) noexcept
  {
    bool positionIsOffsetInAnchor =
      position.GetAnchorType() == dom::Position::AnchorType::PositionIsOffsetInAnchor;
    bool positionOnlyToBeUpdatedIsOffsetInAnchor =
      positionOnlyToBeUpdated.GetAnchorType() == dom::Position::AnchorType::PositionIsOffsetInAnchor;
    RefPtr<dom::Text> text;
    if (auto *container = DynamicDowncast<dom::Text>(position.GetContainerNode());
        positionIsOffsetInAnchor && container)
    {
      text = ShareRefPtr(container);
    }
    else
    {
      if (auto *before = DynamicDowncast<dom::Text>(position.ComputeNodeBeforePosition()))
      {
        text = ShareRefPtr(before);
      }
      else if (auto *after = DynamicDowncast<dom::Text>(position.ComputeNodeAfterPosition()))
      {
        text = ShareRefPtr(after);
      }
    }
    if (!text)
    {
      return;
    }

    if (RefPtr<dom::Text> previous = ShareRefPtr(DynamicDowncast<dom::Text>(text->PreviousSibling())))
    {
      InsertTextIntoNode(*text, 0, previous->Data());

      auto previousLength = static_cast<uint32>(previous->Length());
      if (positionIsOffsetInAnchor)
      {
        position.MoveToOffset(previousLength + static_cast<uint32>(position.OffsetInContainerNode()));
      }
      else
      {
        UpdatePositionForNodeRemoval(position, *previous);
      }

      if (positionOnlyToBeUpdatedIsOffsetInAnchor)
      {
        if (positionOnlyToBeUpdated.GetContainerNode() == text)
        {
          positionOnlyToBeUpdated.MoveToOffset(
            previousLength + static_cast<uint32>(positionOnlyToBeUpdated.OffsetInContainerNode()));
        }
        else if (positionOnlyToBeUpdated.GetContainerNode() == previous)
        {
          positionOnlyToBeUpdated.MoveToPosition(
            text.get(), static_cast<uint32>(positionOnlyToBeUpdated.OffsetInContainerNode()));
        }
      }
      else
      {
        UpdatePositionForNodeRemoval(positionOnlyToBeUpdated, *previous);
      }

      RemoveNode(*previous);
    }
    if (RefPtr<dom::Text> next = ShareRefPtr(DynamicDowncast<dom::Text>(text->NextSibling())))
    {
      auto originalLength = static_cast<uint32>(text->Length());
      InsertTextIntoNode(*text, originalLength, next->Data());

      if (!positionIsOffsetInAnchor)
      {
        UpdatePositionForNodeRemoval(position, *next);
      }

      if (positionOnlyToBeUpdatedIsOffsetInAnchor && positionOnlyToBeUpdated.GetContainerNode() == next)
      {
        positionOnlyToBeUpdated.MoveToPosition(
          text.get(), originalLength + static_cast<uint32>(positionOnlyToBeUpdated.OffsetInContainerNode()));
      }
      else
      {
        UpdatePositionForNodeRemoval(positionOnlyToBeUpdated, *next);
      }

      RemoveNode(*next);
    }
  }

  // If the user is inserting a list into an existing list, instead of nesting the list,
  // we put the list items into the existing list.
  RefPtr<dom::Node> ReplaceSelectionCommand::InsertAsListItems(dom::Element &passedListElement,
                                                               dom::Node *insertionBlock,
                                                               const dom::Position &insertPos,
                                                               InsertedNodes &insertedNodes) noexcept
  {
    Ref<dom::Element> listElement = DeepestSingleChildList(passedListElement);

    bool isStart = IsStartOfParagraph(insertPos);
    bool isEnd = IsEndOfParagraph(insertPos);
    bool isMiddle = !isStart && !isEnd;
    RefPtr<dom::Node> lastNode = ShareRefPtr(insertionBlock);

    // If we're in the middle of a list item, we should split it into two separate
    // list items and insert these nodes between them.
    if (isMiddle)
    {
      int32 textNodeOffset = insertPos.OffsetInContainerNode();
      if (auto *text = DynamicDowncast<dom::Text>(*insertPos.DeprecatedNode()); text && textNodeOffset > 0)
      {
        SplitTextNode(*text, static_cast<uint32>(textNodeOffset));
      }
      (void)SplitTreeToNode(*insertPos.DeprecatedNode(), *lastNode, true);
    }

    while (auto *firstChild = listElement->FirstChild())
    {
      Ref<dom::Node> listItem = ShareRef(*firstChild);
      (void)listElement->RemoveChild(*listItem);
      if (isStart || isMiddle)
      {
        (void)InsertNodeBefore(*listItem, *lastNode);
        insertedNodes.RespondToNodeInsertion(listItem.get());
      }
      else if (isEnd)
      {
        InsertNodeAfter(*listItem, *lastNode);
        insertedNodes.RespondToNodeInsertion(listItem.get());
        lastNode = listItem;
      }
      else
      {
        krys_debug_assert(false);
      }
    }
    if ((isStart || isMiddle) && lastNode->PreviousSibling())
    {
      lastNode = ShareRefPtr(lastNode->PreviousSibling());
    }
    return lastNode;
  }

  void ReplaceSelectionCommand::UpdateNodesInserted(dom::Node *node) noexcept
  {
    if (!node)
    {
      return;
    }

    if (_startOfInsertedContent.IsNull())
    {
      _startOfInsertedContent = FirstPositionInOrBeforeNode(node);
    }

    _endOfInsertedContent = LastPositionInOrAfterNode(TreeQueries::DeepLastChild(*node));
  }

  ReplacementFragment *ReplaceSelectionCommand::EnsureReplacementFragment() noexcept
  {
    if (!_replacementFragment)
    {
      _replacementFragment = CreateUnique<ReplacementFragment>(
        RefPtr<dom::DocumentFragment>(_documentFragment), EndingSelection());
    }
    return _replacementFragment.get();
  }

  // During simple pastes, where we're just pasting a text node into a run of text, we insert the text node
  // directly into the text node that holds the selection.  This is much faster than the generalized code in
  // ReplaceSelectionCommand, and works around <https://bugs.webkit.org/show_bug.cgi?id=6148> since we don't
  // split text nodes.
  bool ReplaceSelectionCommand::PerformTrivialReplace(const ReplacementFragment &fragment) noexcept
  {
    auto *textNode = DynamicDowncast<dom::Text>(fragment.FirstChild());
    if (!textNode || fragment.FirstChild() != fragment.LastChild())
    {
      return false;
    }

    // FIXME: Would be nice to handle smart replace in the fast path.
    if (_smartReplace || fragment.HasInterchangeNewlineAtStart() || fragment.HasInterchangeNewlineAtEnd())
    {
      return false;
    }

    // e.g. when "bar" is inserted after "foo" in <div><u>foo</u></div>, "bar" should not be underlined.
    if (NodeToSplitToAvoidPastingIntoInlineNodesWithStyle(EndingSelection().Start()))
    {
      return false;
    }

    if (FullySelectsEnclosingLink(EndingSelection()))
    {
      return false;
    }

    RefPtr<dom::Node> nodeAfterInsertionPos = ShareRefPtr(EndingSelection().End().Downstream().AnchorNode());
    // Our fragment creation code handles tabs, spaces, and newlines, so we don't have to worry about those
    // here.

    dom::Position start = EndingSelection().Start();
    dom::Position end = ReplaceSelectedTextInNode(textNode->Data());
    if (end.IsNull())
    {
      return false;
    }

    if (nodeAfterInsertionPos && nodeAfterInsertionPos->ParentNode()
        && HasTagName(nodeAfterInsertionPos.get(), html::TagName::br)
        && ShouldRemoveEndBR(nodeAfterInsertionPos.get(),
                             dom::PositionBeforeNode(nodeAfterInsertionPos.get())))
    {
      RemoveNodeAndPruneAncestors(*nodeAfterInsertionPos);
    }

    VisibleSelection selectionAfterReplace(_selectReplacement ? start : end, end);

    SetEndingSelection(selectionAfterReplace);

    return true;
  }

  Maybe<dom::SimpleRange> ReplaceSelectionCommand::InsertedContentRange() const noexcept
  {
    return dom::MakeSimpleRange(dom::PositionRange {_startOfInsertedContent, _endOfInsertedContent});
  }
}
