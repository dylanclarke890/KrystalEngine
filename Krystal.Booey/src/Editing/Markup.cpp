#include "Krystal.Booey/Editing/Markup.hpp"
#include "Krystal.Booey/Constants/Attributes.hpp"
#include "Krystal.Booey/Core/UnicodeHelpers.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/CompositeEditCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/EditingStyle.hpp"
#include "Krystal.Booey/Editing/MarkupAccumulator.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/Editing/VisibleSelection.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include <algorithm>
#include <ranges>

namespace krys::boo
{
  using css::PropertyId;
  using css::ValueId;
  using dom::TreeQueries;

  namespace
  {
    KRYS_NODISCARD bool HasTagName(const dom::Node &node, html::TagName tagName) noexcept
    {
      return IsHTMLElementOfType(&node, tagName);
    }

    // Composed tree traversal, ignoring user agent shadow trees.

    KRYS_NODISCARD dom::Node *FirstChildInComposedTree(dom::Node &node) noexcept
    {
      if (auto *element = DynamicDowncast<dom::Element>(node))
      {
        if (auto *shadowRoot = TreeQueries::GetShadowRoot(*element);
            shadowRoot && shadowRoot->Mode() != dom::ShadowRootMode::UserAgent)
        {
          return shadowRoot->FirstChild();
        }
      }
      if (auto *slot = DynamicDowncast<html::HTMLSlotElement>(node))
      {
        if (auto *assignedNodes = slot->AssignedNodes(); assignedNodes && !assignedNodes->empty())
        {
          return assignedNodes->front().get();
        }
      }
      return node.FirstChild();
    }

    KRYS_NODISCARD dom::Node *NextSiblingInComposedTree(dom::Node &node) noexcept
    {
      if (auto *slot = dom::SlotAlgorithms::FindSlot(node); slot && !slot->IsInUserAgentShadowTree())
      {
        if (auto *assignedNodes = slot->AssignedNodes())
        {
          auto found =
            std::ranges::find_if(*assignedNodes, [&](const auto &item) { return item.get() == &node; });
          if (found != assignedNodes->end() && found + 1 != assignedNodes->end())
          {
            return (found + 1)->get();
          }
          return nullptr;
        }
      }
      return node.NextSibling();
    }

    KRYS_NODISCARD dom::Node *NextSkippingChildrenInComposedTree(dom::Node &node) noexcept
    {
      if (auto *sibling = NextSiblingInComposedTree(node))
      {
        return sibling;
      }
      for (auto *ancestor = node.ParentInComposedTree(); ancestor;
           ancestor = ancestor->ParentInComposedTree())
      {
        if (auto *sibling = NextSiblingInComposedTree(*ancestor))
        {
          return sibling;
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD utf8_string DirectionAttributeAndValue(TextDirection direction) noexcept
    {
      return direction == TextDirection::LTR ? utf8_string(u8"dir=\"ltr\"") : utf8_string(u8"dir=\"rtl\"");
    }

    KRYS_NODISCARD bool PropertyMissingOrEqualToNone(const css::StyleProperties *style,
                                                     PropertyId propertyID) noexcept
    {
      if (!style)
      {
        return false;
      }
      return style->PropertyAsValueId(propertyID).value_or(ValueId::None) == ValueId::None;
    }

    enum class MSOListMode : bool
    {
      Preserve,
      DoNotPreserve
    };

    /// @brief Serializes a range with the styles that give it its appearance inlined.
    class StyledMarkupAccumulator final : public MarkupAccumulator
    {
    public:
      enum RangeFullySelectsNode : bool
      {
        DoesFullySelectNode,
        DoesNotFullySelectNode
      };

    private:
      enum class SpanReplacementType : uint8
      {
        None,
        Slot,
      };

      enum class NodeTraversalMode : bool
      {
        EmitString,
        DoNotEmitString
      };

      dom::Position _start;
      dom::Position _end;
      List<utf8_string> _reversedPrecedingMarkup;
      const AnnotateForInterchange _annotate;
      RefPtr<dom::Node> _highestNodeToBeSerialized;
      RefPtr<EditingStyle> _wrappingStyle;
      bool _useComposedTree;
      bool _ignoresUserSelectNone;
      bool _needsPositionStyleConversion;
      StandardFontFamilySerializationMode _standardFontFamilySerializationMode;
      bool _shouldPreserveMSOList;
      bool _needRelativeStyleWrapper {false};
      bool _needClearingDiv {false};
      bool _inMSOList {false};
      bool _preserveDirectionForInlineText {false};
      bool _hasAppendedAnyText {false};

    public:
      StyledMarkupAccumulator(const dom::Position &start, const dom::Position &end,
                              List<Ref<dom::Node>> *nodes, ResolveURLs resolveURLs,
                              SerializeComposedTree serializeComposedTree,
                              IgnoreUserSelectNone ignoreUserSelectNone, AnnotateForInterchange annotate,
                              StandardFontFamilySerializationMode standardFontFamilySerializationMode,
                              MSOListMode msoListMode, bool needsPositionStyleConversion,
                              PreserveDirectionForInlineText preserveDirectionForInlineText,
                              dom::Node *highestNodeToBeSerialized = nullptr) noexcept
          : MarkupAccumulator(nodes, resolveURLs, GetSerializationSyntax(*start.GetDocument())),
            _start(start), _end(end), _annotate(annotate),
            _highestNodeToBeSerialized(ShareRefPtr(highestNodeToBeSerialized)),
            _useComposedTree(serializeComposedTree == SerializeComposedTree::Yes),
            _ignoresUserSelectNone(ignoreUserSelectNone == IgnoreUserSelectNone::Yes),
            _needsPositionStyleConversion(needsPositionStyleConversion),
            _standardFontFamilySerializationMode(standardFontFamilySerializationMode),
            _shouldPreserveMSOList(msoListMode == MSOListMode::Preserve),
            _preserveDirectionForInlineText(preserveDirectionForInlineText
                                            == PreserveDirectionForInlineText::Yes)
      {
      }

      RefPtr<dom::Node> SerializeNodes(const dom::Position &start, const dom::Position &end) noexcept;
      void WrapWithNode(dom::Node &node, bool convertBlocksToInlines = false,
                        RangeFullySelectsNode rangeFullySelectsNode = DoesFullySelectNode) noexcept;
      void WrapWithStyleNode(css::StyleProperties *style, bool isBlock = false) noexcept;
      KRYS_NODISCARD utf8_string TakeResults() noexcept;

      KRYS_NODISCARD bool NeedRelativeStyleWrapper() const noexcept
      {
        return _needRelativeStyleWrapper;
      }
      KRYS_NODISCARD bool NeedClearingDiv() const noexcept
      {
        return _needClearingDiv;
      }

      using MarkupAccumulator::Append;

      KRYS_NODISCARD dom::ContainerNode *ParentNode(dom::Node &node) noexcept
      {
        if (_useComposedTree) KRYS_UNLIKELY
        {
          return node.ParentInComposedTree();
        }
        return node.ParentOrShadowHostNode();
      }

      // FIXME: The `meta charset` for clients that read the markup back from the pasteboard as UTF-8, with
      // the pasteboard's HTML.
      void PrependHeadIfNecessary(const dom::Element *baseElement) noexcept
      {
        bool shouldAppendMetaCharset = false;
        if (!shouldAppendMetaCharset && !baseElement)
        {
          return;
        }

        _reversedPrecedingMarkup.push_back(u8"</head>");
        if (baseElement)
        {
          utf8_string markupForBase;
          AppendStartTag(markupForBase, *baseElement, false, DoesNotFullySelectNode);
          _reversedPrecedingMarkup.push_back(krys::move(markupForBase));
        }
        if (shouldAppendMetaCharset)
        {
          _reversedPrecedingMarkup.push_back(u8"<meta charset=\"UTF-8\">");
        }
        _reversedPrecedingMarkup.push_back(u8"<head>");
      }

      void WrapInBodyTagWithDirectionAttributeIfNecessary(const VisiblePosition &start,
                                                          const VisiblePosition &end) noexcept
      {
        if (!InSameParagraph(start, end))
        {
          return;
        }

        if (_highestNodeToBeSerialized && HasTagName(*_highestNodeToBeSerialized, html::TagName::body))
        {
          return;
        }

        auto block = EnclosingBlock(ShareRefPtr(start.DeepEquivalent().GetContainerNode()));
        if (!block || block != EnclosingBlock(ShareRefPtr(end.DeepEquivalent().GetContainerNode())))
        {
          return;
        }

        auto *renderer = block->Renderer();
        if (!renderer)
        {
          return;
        }

        utf8_string bodyTag(u8"<body ");
        bodyTag.append(DirectionAttributeAndValue(renderer->GetWritingMode().BidiDirection()));
        bodyTag.push_back(u8'>');
        _reversedPrecedingMarkup.push_back(krys::move(bodyTag));
        Append(u8"</body>");
      }

    private:
      KRYS_NODISCARD bool ContainsOnlyASCII() const noexcept;
      void AppendStyleNodeOpenTag(utf8_string &out, css::StyleProperties *style, bool isBlock = false,
                                  Maybe<TextDirection> directionToAppend = null) noexcept;
      KRYS_NODISCARD static utf8_stringview StyleNodeCloseTag(bool isBlock = false) noexcept
      {
        return isBlock ? u8"</div>" : u8"</span>";
      }

      KRYS_NODISCARD utf8_string RenderedTextRespectingRange(const dom::Text &text) noexcept;
      KRYS_NODISCARD utf8_string TextContentRespectingRange(const dom::Text &text) noexcept;

      KRYS_NODISCARD bool ShouldPreserveMSOListStyleForElement(const dom::Element &element) noexcept;

      KRYS_NODISCARD SpanReplacementType SpanReplacementForElement(const dom::Element &element) noexcept;

      void AppendStartTag(utf8_string &out, const dom::Element &element, bool addDisplayInline,
                          RangeFullySelectsNode rangeFullySelectsNode) noexcept;
      void AppendEndTag(utf8_string &out, const dom::Element &element) noexcept override;
      void AppendCustomAttributes(utf8_string &out, const dom::Element &element,
                                  Namespaces *namespaces) noexcept override;

      void AppendText(utf8_string &out, const dom::Text &text) noexcept override;
      void AppendStartTag(utf8_string &out, const dom::Element &element, Namespaces *) noexcept override
      {
        AppendStartTag(out, element, false, DoesFullySelectNode);
      }

      KRYS_NODISCARD dom::Node *FirstChild(dom::Node &node) noexcept
      {
        if (_useComposedTree) KRYS_UNLIKELY
        {
          return FirstChildInComposedTree(node);
        }
        return node.FirstChild();
      }

      KRYS_NODISCARD dom::Node *NextSibling(dom::Node &node) noexcept
      {
        if (_useComposedTree) KRYS_UNLIKELY
        {
          return NextSiblingInComposedTree(node);
        }
        return node.NextSibling();
      }

      KRYS_NODISCARD dom::Node *NextSkippingChildren(dom::Node &node) noexcept
      {
        if (_useComposedTree) KRYS_UNLIKELY
        {
          return NextSkippingChildrenInComposedTree(node);
        }
        return TreeQueries::NextSkippingChildren(node);
      }

      KRYS_NODISCARD bool HasChildNodes(dom::Node &node) noexcept
      {
        if (_useComposedTree) KRYS_UNLIKELY
        {
          return FirstChildInComposedTree(node);
        }
        return node.HasChildNodes();
      }

      KRYS_NODISCARD bool IsDescendantOf(dom::Node &node, dom::Node &possibleAncestor) noexcept
      {
        if (_useComposedTree) KRYS_UNLIKELY
        {
          return TreeQueries::IsShadowIncludingDescendant(node, possibleAncestor);
        }
        return node.IsDescendantOf(&possibleAncestor);
      }

      RefPtr<dom::Node> TraverseNodesForSerialization(dom::Node &startNode, dom::Node *pastEnd,
                                                      NodeTraversalMode traversalMode) noexcept;

      bool AppendNodeToPreserveMSOList(dom::Node &node) noexcept;

      KRYS_NODISCARD bool ShouldAnnotate() const noexcept
      {
        return _annotate == AnnotateForInterchange::Yes;
      }

      KRYS_NODISCARD bool ShouldApplyWrappingStyle(const dom::Node &node) const noexcept
      {
        return _highestNodeToBeSerialized && _highestNodeToBeSerialized->ParentNode() == node.ParentNode()
               && _wrappingStyle && _wrappingStyle->Style();
      }
    };

    void StyledMarkupAccumulator::WrapWithNode(dom::Node &node, bool convertBlocksToInlines,
                                               RangeFullySelectsNode rangeFullySelectsNode) noexcept
    {
      utf8_string markup;
      if (auto *element = DynamicDowncast<dom::Element>(node))
      {
        AppendStartTag(markup, *element, convertBlocksToInlines && IsBlock(node), rangeFullySelectsNode);
      }
      else
      {
        AppendNonElementNode(markup, node, nullptr);
      }
      _reversedPrecedingMarkup.push_back(krys::move(markup));
      EndAppendingNode(node);
      if (_nodes)
      {
        _nodes->push_back(ShareRef(node));
      }
    }

    void StyledMarkupAccumulator::WrapWithStyleNode(css::StyleProperties *style, bool isBlock) noexcept
    {
      utf8_string openTag;
      AppendStyleNodeOpenTag(openTag, style, isBlock);
      _reversedPrecedingMarkup.push_back(krys::move(openTag));
      Append(StyleNodeCloseTag(isBlock));
    }

    void StyledMarkupAccumulator::AppendStyleNodeOpenTag(utf8_string &out, css::StyleProperties *style,
                                                         bool isBlock,
                                                         Maybe<TextDirection> directionToAppend) noexcept
    {
      // With AnnotateForInterchange::Yes, wrappingStyleForSerialization should have removed
      // -webkit-text-decorations-in-effect
      krys_debug_assert(!ShouldAnnotate()
                        || PropertyMissingOrEqualToNone(style, PropertyId::WebkitTextDecorationsInEffect));
      out.push_back(u8'<');
      out.append(isBlock ? u8"div" : u8"span");
      out.push_back(u8' ');
      if (directionToAppend)
      {
        out.append(DirectionAttributeAndValue(*directionToAppend));
        out.push_back(u8' ');
      }
      out.append(u8"style=\"");

      AppendAttributeValue(out, style->AsText(css::DefaultSerialisationContext()));
      out.append(u8"\">");
    }

    bool StyledMarkupAccumulator::ContainsOnlyASCII() const noexcept
    {
      for (const auto &preceding : _reversedPrecedingMarkup)
      {
        if (!std::ranges::all_of(preceding, [](char8_t c) { return static_cast<uint8>(c) < 0x80; }))
        {
          return false;
        }
      }
      return MarkupAccumulator::ContainsOnlyASCII();
    }

    utf8_string StyledMarkupAccumulator::TakeResults() noexcept
    {
      size_t length = Length();
      for (const auto &string : _reversedPrecedingMarkup)
      {
        length += string.size();
      }
      utf8_string result;
      result.reserve(length);
      for (const auto &string : _reversedPrecedingMarkup | std::views::reverse)
      {
        result.append(string);
      }
      result.append(TakeMarkup());
      // Remove '\0' characters because they are not visibly rendered to the user.
      std::erase(result, u8'\0');
      return result;
    }

    void StyledMarkupAccumulator::AppendText(utf8_string &out, const dom::Text &text) noexcept
    {
      const bool parentIsTextarea = IsHTMLElementOfType(text.ParentElement(), html::TagName::textarea);
      const bool wrappingSpan = ShouldApplyWrappingStyle(text) && !parentIsTextarea;
      if (wrappingSpan)
      {
        auto wrappingStyle = _wrappingStyle->Copy();
        // FIXME: <rdar://problem/5371536> Style rules that match pasted content can change it's appearance
        // Make sure spans are inline style in paste side e.g. span { display: block }.
        wrappingStyle->ForceDisplayInline();
        // FIXME: Should this be included in forceDisplayInline?
        (void)wrappingStyle->Style()->SetProperty(PropertyId::Float, ValueId::None);

        AppendStyleNodeOpenTag(out, wrappingStyle->Style(), false,
                               [&]() -> Maybe<TextDirection>
                               {
                                 if (_hasAppendedAnyText)
                                 {
                                   return null;
                                 }

                                 if (!_preserveDirectionForInlineText)
                                 {
                                   return null;
                                 }

                                 auto directionFromText = BaseTextDirection(text.WholeText());
                                 if (!directionFromText)
                                 {
                                   return null;
                                 }

                                 auto enclosingBlockDirection = DirectionOfEnclosingBlock(
                                   dom::Position(ShareRefPtr(const_cast<dom::Text *>(&text)), 0));
                                 if (enclosingBlockDirection == directionFromText)
                                 {
                                   return null;
                                 }

                                 return enclosingBlockDirection;
                               }());
      }

      if (!ShouldAnnotate() || parentIsTextarea)
      {
        auto content = TextContentRespectingRange(text);
        AppendCharactersReplacingEntities(out, content, EntityMaskForText(text));
      }
      else
      {
        const bool useRenderedText = !EnclosingElementWithTag(
          dom::FirstPositionInNode(const_cast<dom::Text *>(&text)), html::TagName::select);
        utf8_string content =
          useRenderedText ? RenderedTextRespectingRange(text) : TextContentRespectingRange(text);
        utf8_string buffer;
        AppendCharactersReplacingEntities(buffer, content, EntityMaskInPCDATA);
        out.append(ConvertHTMLTextToInterchangeFormat(buffer, &text));
      }

      if (wrappingSpan)
      {
        out.append(StyleNodeCloseTag());
      }

      _hasAppendedAnyText = true;
    }

    utf8_string StyledMarkupAccumulator::RenderedTextRespectingRange(const dom::Text &text) noexcept
    {
      TextIteratorBehaviors behaviors;
      auto *mutableText = const_cast<dom::Text *>(&text);
      dom::Position start =
        &text == _start.GetContainerNode() ? _start : dom::FirstPositionInNode(mutableText);
      dom::Position end;
      if (&text == _end.GetContainerNode())
      {
        end = _end;
      }
      else
      {
        end = dom::LastPositionInNode(mutableText);
        if (!_end.IsNull())
        {
          behaviors.Add(TextIteratorBehavior::BehavesAsIfNodesFollowing);
        }
      }
      if (_ignoresUserSelectNone)
      {
        behaviors.Add(TextIteratorBehavior::IgnoresUserSelectNone);
      }

      auto range = dom::MakeSimpleRange(dom::PositionRange {start, end});
      return range ? PlainText(*range, behaviors) : utf8_string();
    }

    utf8_string StyledMarkupAccumulator::TextContentRespectingRange(const dom::Text &text) noexcept
    {
      if (_start.IsNull() && _end.IsNull())
      {
        return text.Data();
      }

      size_t start = 0;
      size_t end = utf8_string::npos;
      if (&text == _start.GetContainerNode())
      {
        start = static_cast<size_t>(_start.OffsetInContainerNode());
      }
      if (&text == _end.GetContainerNode())
      {
        end = static_cast<size_t>(_end.OffsetInContainerNode());
      }
      krys_debug_assert(start <= end);
      return utf8_string(utf8_stringview(text.Data()).substr(start, end - start));
    }

    // FIXME: The attachment identifiers of attachment, image and source elements, with attachments.
    void StyledMarkupAccumulator::AppendCustomAttributes(utf8_string &, const dom::Element &,
                                                         Namespaces *) noexcept
    {
    }

    bool StyledMarkupAccumulator::ShouldPreserveMSOListStyleForElement(const dom::Element &element) noexcept
    {
      if (_inMSOList)
      {
        return true;
      }
      if (_shouldPreserveMSOList)
      {
        auto style = element.GetAttribute(dom::DOMStringAtom(Attributes::Style)).value_or(dom::DOMString());
        utf8_stringview view(style);
        return view.starts_with(u8"mso-list:") || view.find(u8";mso-list:") != utf8_stringview::npos
               || view.find(u8"\nmso-list:") != utf8_stringview::npos;
      }
      return false;
    }

    // FIXME: Data detector elements, with data detection.
    StyledMarkupAccumulator::SpanReplacementType
      StyledMarkupAccumulator::SpanReplacementForElement(const dom::Element &element) noexcept
    {
      if (Is<html::HTMLSlotElement>(element))
      {
        return SpanReplacementType::Slot;
      }

      return SpanReplacementType::None;
    }

    // FIXME: Leaving out event handler attributes and javascript: URLs (isEventHandlerAttribute,
    // attributeContainsJavaScriptURL), with scripting.
    void StyledMarkupAccumulator::AppendStartTag(utf8_string &out, const dom::Element &element,
                                                 bool addDisplayInline,
                                                 RangeFullySelectsNode rangeFullySelectsNode) noexcept
    {
      auto replacementType = SpanReplacementForElement(element);
      if (replacementType != SpanReplacementType::None) KRYS_UNLIKELY
      {
        out.append(u8"<span");
      }
      else
      {
        AppendOpenTag(out, element, nullptr);
      }

      AppendCustomAttributes(out, element, nullptr);

      const bool shouldAnnotateOrForceInline =
        Is<html::HTMLElement>(element) && (ShouldAnnotate() || addDisplayInline);
      bool shouldOverrideStyleAttr = (shouldAnnotateOrForceInline || ShouldApplyWrappingStyle(element)
                                      || replacementType != SpanReplacementType::None)
                                     && !ShouldPreserveMSOListStyleForElement(element);
      for (const auto &attribute : element.AttributeList())
      {
        // We'll handle the style attribute separately, below.
        if (attribute->NamespaceURI().View().empty()
            && attribute->LocalName() == dom::DOMStringAtom(Attributes::Style) && shouldOverrideStyleAttr)
        {
          continue;
        }
        (void)AppendAttribute(out, element, *attribute, nullptr);
      }

      if (shouldOverrideStyleAttr)
      {
        RefPtr<EditingStyle> newInlineStyle;
        auto &mutableElement = const_cast<dom::Element &>(element);

        if (ShouldApplyWrappingStyle(element))
        {
          newInlineStyle = _wrappingStyle->Copy();
          newInlineStyle->RemovePropertiesInElementDefaultStyle(mutableElement);
          newInlineStyle->RemoveStyleConflictingWithStyleOfNode(mutableElement);
        }
        else
        {
          newInlineStyle = EditingStyle::Create();
        }

        if (replacementType == SpanReplacementType::Slot)
        {
          newInlineStyle->AddDisplayContents();
        }

        if (auto *styledElement = DynamicDowncast<dom::StyledElement>(element);
            styledElement && styledElement->InlineStyle())
        {
          newInlineStyle->OverrideWithStyle(*styledElement->InlineStyle());
        }

        if (shouldAnnotateOrForceInline)
        {
          if (ShouldAnnotate())
          {
            newInlineStyle->MergeStyleFromRulesForSerialization(Downcast<html::HTMLElement>(mutableElement),
                                                                _standardFontFamilySerializationMode);
          }

          if (addDisplayInline)
          {
            newInlineStyle->ForceDisplayInline();
          }

          if (_needsPositionStyleConversion)
          {
            _needRelativeStyleWrapper |= newInlineStyle->ConvertPositionStyle();
            _needClearingDiv |= newInlineStyle->IsFloating();
          }

          // If the node is not fully selected by the range, then we don't want to keep styles that affect its
          // relationship to the nodes around it only the ones that affect it and the nodes within it.
          if (rangeFullySelectsNode == DoesNotFullySelectNode && newInlineStyle->Style())
          {
            (void)newInlineStyle->Style()->RemoveProperty(PropertyId::Float);
          }
        }

        if (!newInlineStyle->IsEmpty())
        {
          out.append(u8" style=\"");
          AppendAttributeValue(out, newInlineStyle->Style()->AsText(css::DefaultSerialisationContext()));
          out.push_back(u8'"');
        }
      }

      AppendCloseTag(out, element);
    }

    void StyledMarkupAccumulator::AppendEndTag(utf8_string &out, const dom::Element &element) noexcept
    {
      if (SpanReplacementForElement(element) != SpanReplacementType::None) KRYS_UNLIKELY
      {
        out.append(u8"</span>");
      }
      else
      {
        MarkupAccumulator::AppendEndTag(out, element);
      }
    }

    RefPtr<dom::Node> StyledMarkupAccumulator::SerializeNodes(const dom::Position &start,
                                                              const dom::Position &end) noexcept
    {
      krys_debug_assert(start <= end);
      RefPtr<dom::Node> startNode = start.FirstNode();
      if (!startNode)
      {
        return nullptr;
      }
      RefPtr<dom::Node> pastEnd = ShareRefPtr(end.ComputeNodeAfterPosition());
      if (!pastEnd && end.GetContainerNode())
      {
        pastEnd = ShareRefPtr(NextSkippingChildren(*end.GetContainerNode()));
      }

      if (!_highestNodeToBeSerialized)
      {
        _highestNodeToBeSerialized =
          TraverseNodesForSerialization(*startNode, pastEnd.get(), NodeTraversalMode::DoNotEmitString);
      }

      if (_highestNodeToBeSerialized && _highestNodeToBeSerialized->ParentNode())
      {
        _wrappingStyle = EditingStyle::WrappingStyleForSerialization(
          *_highestNodeToBeSerialized->ParentNode(), ShouldAnnotate(), _standardFontFamilySerializationMode);
      }

      return TraverseNodesForSerialization(*startNode, pastEnd.get(), NodeTraversalMode::EmitString);
    }

    RefPtr<dom::Node>
      StyledMarkupAccumulator::TraverseNodesForSerialization(dom::Node &startNode, dom::Node *pastEnd,
                                                             NodeTraversalMode traversalMode) noexcept
    {
      const bool shouldEmit = traversalMode == NodeTraversalMode::EmitString;
      UserSelectNoneStateCache userSelectNoneStateCache(_useComposedTree ? dom::TreeType::ComposedTree
                                                                         : dom::TreeType::Tree);

      _inMSOList = false;

      uint32 depth = 0;
      auto enterNode = [&](dom::Node &node)
      {
        if (_shouldPreserveMSOList) KRYS_UNLIKELY
        {
          if (shouldEmit)
          {
            if (AppendNodeToPreserveMSOList(node))
            {
              return false;
            }
          }
        }

        auto *element = DynamicDowncast<dom::Element>(node);
        bool isDisplayContents = element && element->HasDisplayContents();
        if (!node.Renderer() && !isDisplayContents
            && !EnclosingElementWithTag(FirstPositionInOrBeforeNode(&node), html::TagName::select))
        {
          return false;
        }

        if (node.Renderer() && node.Renderer()->IsSkippedContent())
        {
          return false;
        }

        if (_ignoresUserSelectNone && userSelectNoneStateCache.NodeOnlyContainsUserSelectNone(node))
        {
          return false;
        }

        ++depth;
        if (shouldEmit)
        {
          StartAppendingNode(node);
        }

        return true;
      };

      RefPtr<dom::Node> lastClosed;
      auto exitNode = [&](dom::Node &node)
      {
        bool closing = depth;
        if (depth)
        {
          --depth;
        }
        if (shouldEmit)
        {
          if (closing)
          {
            EndAppendingNode(node);
          }
          else
          {
            WrapWithNode(node);
          }
        }
        lastClosed = ShareRefPtr(&node);
      };

      RefPtr<dom::Node> lastNode;
      RefPtr<dom::Node> next;
      for (RefPtr<dom::Node> n = ShareRefPtr(&startNode); n != pastEnd; lastNode = n, n = next)
      {
        List<RefPtr<dom::Node>> exitedAncestors;
        next = nullptr;

        auto advanceToAncestorSibling = [&]()
        {
          if (auto *sibling = NextSibling(*n))
          {
            next = ShareRefPtr(sibling);
            return;
          }
          for (auto *ancestor = ParentNode(*n); ancestor; ancestor = ParentNode(*ancestor))
          {
            exitedAncestors.push_back(ShareRefPtr<dom::Node>(ancestor));
            if (auto *sibling = NextSibling(*ancestor))
            {
              next = ShareRefPtr(sibling);
              return;
            }
          }
        };

        if (auto *child = FirstChild(*n))
        {
          next = ShareRefPtr(child);
        }
        else
        {
          advanceToAncestorSibling();
        }

        krys_debug_assert(next || !pastEnd || TreeQueries::IsShadowIncludingInclusiveAncestor(*n, *pastEnd));

        if (IsBlock(*n) && CanHaveChildrenForEditing(*n) && next == pastEnd)
        {
          // Don't write out empty block containers that aren't fully selected.
          continue;
        }

        bool didEnterNode = false;
        if (!enterNode(*n))
        {
          exitedAncestors.clear();
          advanceToAncestorSibling();
        }
        else if (!HasChildNodes(*n))
        {
          exitNode(*n);
        }
        else
        {
          didEnterNode = true;
        }

        bool aboutToGoPastEnd = pastEnd && !didEnterNode && (!next || IsDescendantOf(*pastEnd, *n));
        if (aboutToGoPastEnd)
        {
          next = ShareRefPtr(pastEnd);
        }

        for (auto &ancestor : exitedAncestors)
        {
          if (!depth && next == pastEnd)
          {
            break;
          }
          exitNode(*ancestor);
        }
      }

      krys_debug_assert(lastNode || !depth);
      if (depth)
      {
        for (auto *ancestor = ParentNode(pastEnd ? *pastEnd : *lastNode); ancestor && depth;
             ancestor = ParentNode(*ancestor))
        {
          exitNode(*ancestor);
        }
      }

      return lastClosed;
    }

    bool StyledMarkupAccumulator::AppendNodeToPreserveMSOList(dom::Node &node) noexcept
    {
      if (auto *commentNode = DynamicDowncast<dom::Comment>(node))
      {
        if (!_inMSOList && commentNode->Data() == u8"[if !supportLists]")
        {
          _inMSOList = true;
        }
        else if (_inMSOList && commentNode->Data() == u8"[endif]")
        {
          _inMSOList = false;
        }
        else
        {
          return false;
        }
        StartAppendingNode(*commentNode);
        return true;
      }
      if (HasTagName(node, html::TagName::style))
      {
        auto *textChild = DynamicDowncast<dom::Text>(node.FirstChild());
        if (!textChild)
        {
          return false;
        }

        utf8_stringview styleContent = textChild->Data();

        const auto msoStyleDefinitionsStart = styleContent.find(u8"/* Style Definitions */");
        const auto msoListDefinitionsStart = styleContent.find(u8"/* List Definitions */");
        const auto lastListItem = styleContent.rfind(u8"\n@list");
        if (msoListDefinitionsStart == utf8_stringview::npos || lastListItem == utf8_stringview::npos)
        {
          return false;
        }
        const auto start = msoStyleDefinitionsStart != utf8_stringview::npos
                               && msoStyleDefinitionsStart < msoListDefinitionsStart
                             ? msoStyleDefinitionsStart
                             : msoListDefinitionsStart;

        const auto msoListDefinitionsEnd = styleContent.find(u8";}\n", lastListItem);
        if (msoListDefinitionsEnd == utf8_stringview::npos || start >= msoListDefinitionsEnd)
        {
          return false;
        }

        Append(u8"<head><style class=\"", WebKitMSOListQuirksStyle, u8"\">\n<!--\n",
               styleContent.substr(start, msoListDefinitionsEnd - start + 3), u8"\n-->\n</style></head>");

        return true;
      }
      return false;
    }

    KRYS_NODISCARD dom::Node *
      AncestorToRetainStructureAndAppearanceForBlock(dom::Node *commonAncestorBlock) noexcept
    {
      if (!commonAncestorBlock)
      {
        return nullptr;
      }

      if (HasTagName(*commonAncestorBlock, html::TagName::tbody)
          || HasTagName(*commonAncestorBlock, html::TagName::tr))
      {
        dom::ContainerNode *table = commonAncestorBlock->ParentNode();
        while (table && !HasTagName(*table, html::TagName::table))
        {
          table = table->ParentNode();
        }

        return table;
      }

      if (IsNonTableCellHTMLBlockElement(commonAncestorBlock))
      {
        return commonAncestorBlock;
      }

      return nullptr;
    }

    KRYS_NODISCARD dom::Node *AncestorToRetainStructureAndAppearance(dom::Node *commonAncestor) noexcept
    {
      return AncestorToRetainStructureAndAppearanceForBlock(
        EnclosingBlock(ShareRefPtr(commonAncestor)).get());
    }

    KRYS_NODISCARD bool NeedInterchangeNewlineAfter(const VisiblePosition &v) noexcept
    {
      VisiblePosition next = v.Next();
      auto *upstreamNode = next.DeepEquivalent().Upstream().DeprecatedNode();
      auto *downstreamNode = v.DeepEquivalent().Downstream().DeprecatedNode();
      // Add an interchange newline if a paragraph break is selected and a br won't already be added to the
      // markup to represent it.
      return IsEndOfParagraph(v) && IsStartOfParagraph(next)
             && !(HasTagName(*upstreamNode, html::TagName::br) && upstreamNode == downstreamNode);
    }

    KRYS_NODISCARD RefPtr<EditingStyle> StyleFromMatchedRulesAndInlineDecl(dom::Node &node) noexcept
    {
      auto *element = DynamicDowncast<html::HTMLElement>(node);
      if (!element)
      {
        return nullptr;
      }

      auto style = EditingStyle::Create(element->InlineStyle());
      style->MergeStyleFromRules(*element);
      return style;
    }

    KRYS_NODISCARD bool IsElementPresentational(const dom::Node &node) noexcept
    {
      return HasTagName(node, html::TagName::u) || HasTagName(node, html::TagName::s)
             || HasTagName(node, html::TagName::strike) || HasTagName(node, html::TagName::i)
             || HasTagName(node, html::TagName::em) || HasTagName(node, html::TagName::b)
             || HasTagName(node, html::TagName::strong);
    }

    KRYS_NODISCARD RefPtr<dom::Node> HighestAncestorToWrapMarkup(const dom::Position &start,
                                                                 const dom::Position &end,
                                                                 dom::Node &commonAncestor,
                                                                 AnnotateForInterchange annotate) noexcept
    {
      RefPtr<dom::Node> specialCommonAncestor;
      if (annotate == AnnotateForInterchange::Yes)
      {
        // Include ancestors that aren't completely inside the range but are required to retain
        // the structure and appearance of the copied markup.
        specialCommonAncestor = ShareRefPtr(AncestorToRetainStructureAndAppearance(&commonAncestor));

        if (auto parentListNode = EnclosingNodeOfType(start, IsListItem))
        {
          if (!EditingIgnoresContent(*parentListNode)
              && VisibleSelection::SelectionFromContentsOfNode(parentListNode.get())
                   == VisibleSelection(start, end))
          {
            specialCommonAncestor = ShareRefPtr<dom::Node>(parentListNode->ParentNode());
            while (specialCommonAncestor && !IsListHTMLElement(specialCommonAncestor.get()))
            {
              specialCommonAncestor = ShareRefPtr<dom::Node>(specialCommonAncestor->ParentNode());
            }
          }
        }

        // Retain the Mail quote level by including all ancestor mail block quotes.
        if (auto highestMailBlockquote = HighestEnclosingNodeOfType(
              start, IsMailBlockquote, EditingBoundaryCrossingRule::CanCrossEditingBoundary))
        {
          specialCommonAncestor = krys::move(highestMailBlockquote);
        }
      }

      RefPtr<dom::Node> checkAncestor =
        specialCommonAncestor ? specialCommonAncestor : ShareRefPtr(&commonAncestor);
      if (checkAncestor->Renderer() && checkAncestor->Renderer()->ContainingBlock())
      {
        auto newSpecialCommonAncestor =
          HighestEnclosingNodeOfType(dom::FirstPositionInNode(checkAncestor.get()), &IsElementPresentational,
                                     EditingBoundaryCrossingRule::CanCrossEditingBoundary,
                                     checkAncestor->Renderer()->ContainingBlock()->GetElement());
        if (newSpecialCommonAncestor)
        {
          specialCommonAncestor = krys::move(newSpecialCommonAncestor);
        }
      }

      // If a single tab is selected, commonAncestor will be a text node inside a tab span.
      // If two or more tabs are selected, commonAncestor will be the tab span.
      // In either case, if there is a specialCommonAncestor already, it will necessarily be above
      // any tab span that needs to be included.
      if (!specialCommonAncestor && ParentTabSpanNode(&commonAncestor))
      {
        specialCommonAncestor = ShareRefPtr<dom::Node>(commonAncestor.ParentNode());
      }
      if (!specialCommonAncestor && TabSpanNode(&commonAncestor))
      {
        specialCommonAncestor = ShareRefPtr(&commonAncestor);
      }

      if (auto *enclosingAnchor = EnclosingElementWithTag(
            dom::FirstPositionInNode(specialCommonAncestor ? specialCommonAncestor.get() : &commonAncestor),
            html::TagName::a))
      {
        specialCommonAncestor = ShareRefPtr<dom::Node>(enclosingAnchor);
      }

      if (auto *enclosingPicture = EnclosingElementWithTag(
            dom::FirstPositionInNode(specialCommonAncestor ? specialCommonAncestor.get() : &commonAncestor),
            html::TagName::picture))
      {
        specialCommonAncestor = ShareRefPtr<dom::Node>(enclosingPicture);
      }

      return specialCommonAncestor;
    }

    KRYS_NODISCARD utf8_string InterchangeNewlineString() noexcept
    {
      utf8_string string(u8"<br class=\"");
      string.append(AppleInterchangeNewline);
      string.append(u8"\">");
      return string;
    }

    // FIXME: The document's first `base` element (firstBaseElement), with base URLs.
    KRYS_NODISCARD utf8_string SerializePreservingVisualAppearanceInternal(
      const dom::Position &start, const dom::Position &end, List<Ref<dom::Node>> *nodes,
      ResolveURLs resolveURLs, SerializeComposedTree serializeComposedTree,
      IgnoreUserSelectNone ignoreUserSelectNone, AnnotateForInterchange annotate,
      ConvertBlocksToInlines convertBlocksToInlines,
      StandardFontFamilySerializationMode standardFontFamilySerializationMode, MSOListMode msoListMode,
      PreserveBaseElement preserveBaseElement,
      PreserveDirectionForInlineText preserveDirectionForInlineText) noexcept
    {
      auto interchangeNewlineString = InterchangeNewlineString();

      if (!(start < end))
      {
        return {};
      }

      RefPtr<dom::Node> commonAncestor = ShareRefPtr(dom::CommonInclusiveAncestor(start, end));
      if (!commonAncestor)
      {
        return {};
      }

      Ref<dom::Document> document = ShareRef(*start.GetDocument());
      (void)document->UpdateLayoutIgnorePendingStylesheets();

      VisiblePosition visibleStart {start};
      VisiblePosition visibleEnd {end};

      RefPtr<dom::Element> body = ShareRefPtr(
        EnclosingElementWithTag(dom::FirstPositionInNode(commonAncestor.get()), html::TagName::body));
      RefPtr<dom::Element> fullySelectedRoot;
      // FIXME: Do this for all fully selected blocks, not just the body.
      if (body && VisiblePosition(dom::FirstPositionInNode(body.get())) == visibleStart
          && VisiblePosition(dom::LastPositionInNode(body.get())) == visibleEnd)
      {
        fullySelectedRoot = body;
      }
      bool needsPositionStyleConversion =
        body && fullySelectedRoot == body && document->GetSettings().ShouldConvertPositionStyleOnCopy;

      auto specialCommonAncestor = HighestAncestorToWrapMarkup(start, end, *commonAncestor, annotate);

      StyledMarkupAccumulator accumulator(start, end, nodes, resolveURLs, serializeComposedTree,
                                          ignoreUserSelectNone, annotate, standardFontFamilySerializationMode,
                                          msoListMode, needsPositionStyleConversion,
                                          preserveDirectionForInlineText, specialCommonAncestor.get());

      dom::Position adjustedStart = start;

      if (auto *pictureElement = EnclosingElementWithTag(adjustedStart, html::TagName::picture))
      {
        adjustedStart = dom::FirstPositionInNode(pictureElement);
      }

      if (annotate == AnnotateForInterchange::Yes && NeedInterchangeNewlineAfter(visibleStart))
      {
        if (visibleStart == visibleEnd.Previous())
        {
          return interchangeNewlineString;
        }

        accumulator.Append(interchangeNewlineString);
        adjustedStart = visibleStart.Next().DeepEquivalent();

        if (!(adjustedStart < end))
        {
          return interchangeNewlineString;
        }
      }

      auto lastClosed = accumulator.SerializeNodes(adjustedStart, end);

      if (specialCommonAncestor && lastClosed)
      {
        // Also include all of the ancestors of lastClosed up to this special ancestor.
        for (RefPtr<dom::Node> ancestor = ShareRefPtr<dom::Node>(accumulator.ParentNode(*lastClosed));
             ancestor; ancestor = ShareRefPtr<dom::Node>(accumulator.ParentNode(*ancestor)))
        {
          if (fullySelectedRoot && ancestor == fullySelectedRoot
              && convertBlocksToInlines == ConvertBlocksToInlines::No)
          {
            auto fullySelectedRootStyle = StyleFromMatchedRulesAndInlineDecl(*fullySelectedRoot);

            // Bring the background attribute over, but not as an attribute because a background attribute on
            // a div appears to have no effect.
            dom::DOMStringAtom backgroundAttr(u8"background");
            if ((!fullySelectedRootStyle || !fullySelectedRootStyle->Style()
                 || !fullySelectedRootStyle->Style()->GetPropertyCSSValue(PropertyId::BackgroundImage))
                && fullySelectedRoot->HasAttribute(backgroundAttr))
            {
              utf8_string backgroundImage(u8"url('");
              backgroundImage.append(
                fullySelectedRoot->GetAttribute(backgroundAttr).value_or(dom::DOMString()));
              backgroundImage.append(u8"')");
              (void)fullySelectedRootStyle->Style()->SetProperty(PropertyId::BackgroundImage,
                                                                 backgroundImage);
            }

            if (fullySelectedRootStyle->Style())
            {
              // Reset the CSS properties to avoid an assertion error in addStyleMarkup().
              // This assertion is caused at least when we select all text of a <body> element whose
              // 'text-decoration' property is "inherit", and copy it.
              auto *style = fullySelectedRootStyle->Style();
              if (!PropertyMissingOrEqualToNone(style, PropertyId::TextDecorationLine))
              {
                (void)style->SetProperty(PropertyId::TextDecorationLine, ValueId::None);
                (void)style->SetProperty(PropertyId::TextDecorationThickness, ValueId::Auto);
                (void)style->SetProperty(PropertyId::TextDecorationStyle, ValueId::Solid);
                (void)style->SetProperty(PropertyId::TextDecorationColor, ValueId::Currentcolor);
              }
              if (!PropertyMissingOrEqualToNone(style, PropertyId::WebkitTextDecorationsInEffect))
              {
                (void)style->SetProperty(PropertyId::WebkitTextDecorationsInEffect, ValueId::None);
              }
              accumulator.WrapWithStyleNode(style, true);
            }
          }
          else
          {
            // Since this node and all the other ancestors are not in the selection we want to set
            // RangeFullySelectsNode to DoesNotFullySelectNode so that styles that affect the exterior of the
            // node are not included.
            accumulator.WrapWithNode(*ancestor, convertBlocksToInlines == ConvertBlocksToInlines::Yes,
                                     StyledMarkupAccumulator::DoesNotFullySelectNode);
          }
          if (nodes)
          {
            nodes->push_back(Ref<dom::Node>(ancestor));
          }

          if (ancestor == specialCommonAncestor)
          {
            break;
          }
        }
      }

      if (accumulator.NeedRelativeStyleWrapper() && needsPositionStyleConversion)
      {
        if (accumulator.NeedClearingDiv())
        {
          accumulator.Append(u8"<div style=\"clear: both;\"></div>");
        }
        auto positionRelativeStyle = StyleFromMatchedRulesAndInlineDecl(*body);
        (void)positionRelativeStyle->Style()->SetProperty(PropertyId::Position, ValueId::Relative);
        accumulator.WrapWithStyleNode(positionRelativeStyle->Style(), true);
      }

      // FIXME: The interchange newline should be placed in the block that it's in, not after all of the
      // content, unconditionally.
      if (annotate == AnnotateForInterchange::Yes && NeedInterchangeNewlineAfter(visibleEnd.Previous()))
      {
        accumulator.Append(interchangeNewlineString);
      }

      if (preserveDirectionForInlineText == PreserveDirectionForInlineText::Yes)
      {
        accumulator.WrapInBodyTagWithDirectionAttributeIfNecessary(visibleStart, visibleEnd);
      }

      (void)preserveBaseElement;
      accumulator.PrependHeadIfNecessary(nullptr);

      return accumulator.TakeResults();
    }

    void FillContainerFromString(dom::ContainerNode &paragraph, utf8_stringview string) noexcept
    {
      auto &document = paragraph.NodeDocument();

      if (string.empty())
      {
        (void)paragraph.AppendChild(*CreateBlockPlaceholderElement(document));
        return;
      }

      krys_debug_assert(string.find(u8'\n') == utf8_stringview::npos);

      List<utf8_stringview> tabList;
      for (auto part : std::views::split(string, u8'\t'))
      {
        tabList.emplace_back(part.begin(), part.end());
      }
      utf8_string tabText;
      bool first = true;
      size_t numEntries = tabList.size();
      for (size_t i = 0; i < numEntries; ++i)
      {
        auto s = tabList[i];

        // append the non-tab textual part
        if (!s.empty())
        {
          if (!tabText.empty())
          {
            (void)paragraph.AppendChild(*CreateTabSpanElement(document, utf8_string(tabText)));
            tabText.clear();
          }
          auto textNode =
            document.CreateTextNode(StringWithRebalancedWhitespace(s, first, i + 1 == numEntries));
          (void)paragraph.AppendChild(*textNode);
        }

        // there is a tab after every entry, except the last entry
        // (if the last character is a tab, the list gets an extra empty entry)
        if (i + 1 != numEntries)
        {
          tabText.push_back(u8'\t');
        }
        else if (!tabText.empty())
        {
          (void)paragraph.AppendChild(*CreateTabSpanElement(document, utf8_string(tabText)));
        }

        first = false;
      }
    }

    KRYS_NODISCARD bool ContextPreservesNewline(const dom::SimpleRange &context) noexcept
    {
      auto *container =
        VisiblePosition(dom::MakeDeprecatedLegacyPosition(context.Start)).DeepEquivalent().GetContainerNode();
      return container && container->Renderer() && container->Renderer()->Style().PreserveNewline();
    }

    List<Ref<dom::Element>> CollectElementsToRemoveFromFragment(dom::ContainerNode &container) noexcept
    {
      List<Ref<dom::Element>> toRemove;
      for (auto *child = container.FirstChild(); child; child = child->NextSibling())
      {
        auto *element = DynamicDowncast<html::HTMLElement>(*child);
        if (!element)
        {
          continue;
        }
        if (HasTagName(*element, html::TagName::html))
        {
          toRemove.push_back(ShareRef<dom::Element>(*element));
          auto nested = CollectElementsToRemoveFromFragment(*element);
          (void)nested;
          continue;
        }
        if (HasTagName(*element, html::TagName::head) || HasTagName(*element, html::TagName::body))
        {
          toRemove.push_back(ShareRef<dom::Element>(*element));
        }
      }
      return toRemove;
    }

    void RemoveElementFromFragmentPreservingChildren(dom::DocumentFragment &fragment,
                                                     dom::Element &element) noexcept
    {
      RefPtr<dom::Node> nextChild;
      for (RefPtr<dom::Node> child = ShareRefPtr(element.FirstChild()); child; child = nextChild)
      {
        nextChild = ShareRefPtr(child->NextSibling());
        (void)element.RemoveChild(*child);
        (void)fragment.InsertBefore(*child, &element);
      }
      (void)fragment.RemoveChild(element);
    }

    KRYS_NODISCARD Ref<dom::DocumentFragment> ParseHTMLIntoFragment(dom::Document &document,
                                                                    utf8_stringview markup,
                                                                    dom::Element &contextElement) noexcept
    {
      auto fragment = document.CreateDocumentFragment();
      auto nodes = html::HTMLDocumentParser::ParseFragment(contextElement, utf8_string(markup));
      for (auto &node : nodes)
      {
        (void)fragment->AppendChild(*node);
      }
      return fragment;
    }
  }

  UserSelectNoneStateCache::UserSelectNoneStateCache(dom::TreeType treeType) noexcept
      : _useComposedTree(treeType == dom::TreeType::ComposedTree)
  {
    krys_debug_assert(treeType == dom::TreeType::Tree || treeType == dom::TreeType::ComposedTree);
  }

  dom::ContainerNode *UserSelectNoneStateCache::ParentNode(dom::Node &node) noexcept
  {
    if (_useComposedTree)
    {
      return node.ParentInComposedTree();
    }
    return node.ParentOrShadowHostNode();
  }

  dom::Node *UserSelectNoneStateCache::FirstChild(dom::Node &node) noexcept
  {
    if (_useComposedTree)
    {
      return FirstChildInComposedTree(node);
    }
    return node.FirstChild();
  }

  dom::Node *UserSelectNoneStateCache::NextSibling(dom::Node &node) noexcept
  {
    if (_useComposedTree)
    {
      return NextSiblingInComposedTree(node);
    }
    return node.NextSibling();
  }

  UserSelectNoneStateCache::State UserSelectNoneStateCache::ComputeState(dom::Node &targetNode) noexcept
  {
    if (auto it = _cache.find(&targetNode); it != _cache.end())
    {
      return it->second;
    }
    if (!dom::Position::NodeIsUserSelectNone(&targetNode))
    {
      return State::NotUserSelectNone;
    }
    auto state = State::OnlyUserSelectNone;
    dom::Node *currentNode = &targetNode;
    bool foundMixed = false;
    while (currentNode)
    {
      if (!dom::Position::NodeIsUserSelectNone(currentNode))
      {
        state = State::Mixed;
        // Only traverse upward once any mixed content is found
        // since inner element may only contain user-select: none but we won't be able to tell apart.
        foundMixed = true;
      }
      if (auto *child = FirstChild(*currentNode); child && !foundMixed)
      {
        currentNode = child;
      }
      else if (currentNode == &targetNode)
      {
        break;
      }
      else if (auto *sibling = NextSibling(*currentNode); sibling && !foundMixed)
      {
        currentNode = sibling;
      }
      else
      {
        dom::Node *ancestor;
        for (ancestor = ParentNode(*currentNode); ancestor; ancestor = ParentNode(*ancestor))
        {
          _cache[ancestor] = state;
          if (ancestor == &targetNode)
          {
            currentNode = nullptr;
            break;
          }
          if (auto *ancestorSibling = NextSibling(*ancestor); ancestorSibling && !foundMixed)
          {
            currentNode = ancestorSibling;
            break;
          }
        }
        if (!ancestor)
        {
          currentNode = nullptr;
        }
      }
    }
    return state;
  }

  utf8_string SerializePreservingVisualAppearance(const dom::SimpleRange &range, List<Ref<dom::Node>> *nodes,
                                                  AnnotateForInterchange annotate,
                                                  ConvertBlocksToInlines convertBlocksToInlines,
                                                  ResolveURLs resolveURLs) noexcept
  {
    return SerializePreservingVisualAppearanceInternal(
      dom::MakeDeprecatedLegacyPosition(range.Start), dom::MakeDeprecatedLegacyPosition(range.End), nodes,
      resolveURLs, SerializeComposedTree::No, IgnoreUserSelectNone::No, annotate, convertBlocksToInlines,
      StandardFontFamilySerializationMode::Keep, MSOListMode::DoNotPreserve, PreserveBaseElement::No,
      PreserveDirectionForInlineText::No);
  }

  utf8_string SerializePreservingVisualAppearance(
    const VisibleSelection &selection, ResolveURLs resolveURLs, SerializeComposedTree serializeComposedTree,
    IgnoreUserSelectNone ignoreUserSelectNone, PreserveBaseElement preserveBaseElement,
    PreserveDirectionForInlineText preserveDirectionForInlineText, List<Ref<dom::Node>> *nodes) noexcept
  {
    return SerializePreservingVisualAppearanceInternal(
      selection.Start(), selection.End(), nodes, resolveURLs, serializeComposedTree, ignoreUserSelectNone,
      AnnotateForInterchange::Yes, ConvertBlocksToInlines::No, StandardFontFamilySerializationMode::Keep,
      MSOListMode::DoNotPreserve, preserveBaseElement, preserveDirectionForInlineText);
  }

  // FIXME: Restoring attachment elements' files (restoreAttachmentElementsInFragment), with attachments; and
  // completing relative URLs against baseURL (completeURLs), with URL attributes.
  Ref<dom::DocumentFragment> CreateFragmentFromMarkup(dom::Document &document, utf8_stringview markup,
                                                      utf8_stringview baseURL) noexcept
  {
    (void)baseURL;
    // We use a fake body element here to trick the HTML parser into using the InBody insertion mode.
    auto fakeBody = CreateHTMLElement(document, u8"body");
    return ParseHTMLIntoFragment(document, markup, *fakeBody);
  }

  utf8_string SerializeFragment(const dom::Node &node, SerializedNodes root, List<Ref<dom::Node>> *nodes,
                                ResolveURLs resolveURLs, Maybe<SerializationSyntax> serializationSyntax,
                                SerializeShadowRoots serializeShadowRoots,
                                List<Ref<dom::ShadowRoot>> &&explicitShadowRoots) noexcept
  {
    if (!serializationSyntax)
    {
      serializationSyntax = MarkupAccumulator::GetSerializationSyntax(node.NodeDocument());
    }

    MarkupAccumulator accumulator(nodes, resolveURLs, *serializationSyntax, serializeShadowRoots,
                                  krys::move(explicitShadowRoots));
    return accumulator.SerializeNodes(const_cast<dom::Node &>(node), root);
  }

  bool IsPlainTextMarkup(dom::Node *node) noexcept
  {
    krys_debug_assert(node);
    if (!HasTagName(*node, html::TagName::div) || Downcast<dom::Element>(*node).HasAttributes())
    {
      return false;
    }

    auto *firstChild = node->FirstChild();
    if (!firstChild)
    {
      return false;
    }

    auto *secondChild = firstChild->NextSibling();
    if (!secondChild)
    {
      return Is<dom::Text>(*firstChild) || firstChild->FirstChild();
    }

    if (secondChild->NextSibling())
    {
      return false;
    }

    return ParentTabSpanNode(firstChild->FirstChild()) && Is<dom::Text>(*secondChild);
  }

  Ref<dom::DocumentFragment> CreateFragmentFromText(const dom::SimpleRange &context,
                                                    utf8_stringview text) noexcept
  {
    auto &document = context.Start.Container->NodeDocument();
    auto fragment = document.CreateDocumentFragment();

    if (text.empty())
    {
      return fragment;
    }

    utf8_string string = infra::StringAlgorithms::NormalizeNewlines(text);

    auto createHTMLBRElement = [&document]()
    {
      auto element = CreateHTMLElement(document, u8"br");
      (void)element->SetAttribute(dom::DOMStringAtom(Attributes::Class),
                                  dom::DOMString(AppleInterchangeNewline));
      return element;
    };

    if (ContextPreservesNewline(context))
    {
      bool endsWithNewLine = string.ends_with(u8'\n');
      (void)fragment->AppendChild(*document.CreateTextNode(krys::move(string)));
      if (endsWithNewLine)
      {
        (void)fragment->AppendChild(*createHTMLBRElement());
      }
      return fragment;
    }

    // A string with no newlines gets added inline, rather than being put into a paragraph.
    if (string.find(u8'\n') == utf8_string::npos)
    {
      FillContainerFromString(*fragment, string);
      return fragment;
    }

    if (string.size() == 1 && string[0] == u8'\n')
    {
      // This is a single newline char, thus just create one HTMLBRElement.
      (void)fragment->AppendChild(*createHTMLBRElement());
      return fragment;
    }

    // Break string into paragraphs. Extra line breaks turn into empty paragraphs.
    auto start = dom::MakeDeprecatedLegacyPosition(context.Start);
    auto block = EnclosingBlock(start.FirstNode());
    bool useClonesOfEnclosingBlock = block && !HasTagName(*block, html::TagName::body)
                                     && !HasTagName(*block, html::TagName::html)
                                     // Avoid using table as paragraphs due to its special treatment in
                                     // Position::upstream/downstream.
                                     && !IsRenderedTable(block.get())
                                     && block != EditableRootForPosition(start);
    bool useLineBreak = html::EnclosingTextFormControl(start);

    List<utf8_stringview> list;
    for (auto part : std::views::split(utf8_stringview(string), u8'\n'))
    {
      list.emplace_back(part.begin(), part.end());
    }
    size_t numLines = list.size();
    for (size_t i = 0; i < numLines; ++i)
    {
      auto s = list[i];

      RefPtr<dom::Element> element;
      if (s.empty() && i + 1 == numLines)
      {
        // For last line, use the "magic BR" rather than a P.
        element = createHTMLBRElement();
      }
      else if (useLineBreak)
      {
        element = CreateHTMLElement(document, u8"br");
        FillContainerFromString(*fragment, s);
      }
      else
      {
        if (useClonesOfEnclosingBlock)
        {
          element = CloneElementWithoutChildren(*block);
        }
        else
        {
          element = CreateDefaultParagraphElement(document);
        }
        FillContainerFromString(*element, s);
      }
      (void)fragment->AppendChild(*element);
    }
    return fragment;
  }

  utf8_string DocumentTypeString(const dom::Document &document) noexcept
  {
    auto documentType = document.DocType();
    if (!documentType)
    {
      return {};
    }
    return SerializeFragment(*documentType, SerializedNodes::SubtreeIncludingNode);
  }

  utf8_string URLToMarkup(utf8_stringview url, utf8_stringview title) noexcept
  {
    utf8_string markup(u8"<a href=\"");
    markup.append(url);
    markup.append(u8"\">");
    MarkupAccumulator::AppendCharactersReplacingEntities(markup, title, EntityMaskInPCDATA);
    markup.append(u8"</a>");
    return markup;
  }

  // FIXME: Parsing XML for documents that aren't HTML (parseXML), with XML documents; and the template
  // document for contexts that are templates.
  dom::ExceptionOr<Ref<dom::DocumentFragment>> CreateContextualFragment(dom::Element &element,
                                                                        utf8_stringview markup) noexcept
  {
    auto fragment = ParseHTMLIntoFragment(element.NodeDocument(), markup, element);

    // We need to pop <html> and <body> elements and remove <head> to
    // accommodate folks passing complete HTML documents to make the
    // child of an element.
    auto toRemove = CollectElementsToRemoveFromFragment(*fragment);
    for (auto &elementToRemove : toRemove)
    {
      RemoveElementFromFragmentPreservingChildren(*fragment, *elementToRemove);
    }

    return fragment;
  }

  // FIXME: Setting a lone text child's data instead of replacing it when script can't observe the difference
  // (canUseSetDataOptimization), with mutation observers' child list scope.
  dom::ExceptionOr<void> ReplaceChildrenWithFragment(dom::ContainerNode &container,
                                                     Ref<dom::DocumentFragment> &&fragment) noexcept
  {
    Ref<dom::ContainerNode> containerNode = ShareRef(container);

    while (auto *child = containerNode->FirstChild())
    {
      if (auto result = containerNode->RemoveChild(*child); result.HasException())
      {
        return result.ReleaseException();
      }
    }

    if (!fragment->FirstChild())
    {
      return {};
    }

    if (auto result = containerNode->AppendChild(*fragment); result.HasException())
    {
      return result.ReleaseException();
    }
    return {};
  }
}
