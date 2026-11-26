/*
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2008, 2009, 2010, 2011 Google Inc. All rights reserved.
 * Copyright (C) 2011 Igalia S.L.
 * Copyright (C) 2011 Motorola Mobility. All rights reserved.
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

#include "Krystal.Booey/Editing/Markup/Markup.hpp"
#include "Krystal.Booey/Css/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/Css/Values/SerialisationContext.hpp"
#include "Krystal.Booey/Dom/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/Dom/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/Dom/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/Dom/Nodes/Attr.hpp"
#include "Krystal.Booey/Dom/Nodes/Comment.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Dom/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/Dom/Nodes/DocumentType.hpp"
#include "Krystal.Booey/Dom/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Dom/Nodes/StyledElement.hpp"
#include "Krystal.Booey/Dom/Nodes/Text.hpp"
#include "Krystal.Booey/Dom/Ranges/SimpleRange.hpp"
#include "Krystal.Booey/Editing/Commands/CompositeEditCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/EditingStyle.hpp"
#include "Krystal.Booey/Editing/Markup/MarkupAccumulator.hpp"
#include "Krystal.Booey/Editing/Position.hpp"
#include "Krystal.Booey/Editing/Selection/VisibleSelection.hpp"
#include "Krystal.Booey/Editing/Selection/VisibleUnits.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/Html/Elements/HTMLBodyElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/Html/Parser/DocumentParser.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Loader/DocumentWriter.hpp"
#include "Krystal.Booey/Page/Frame.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Xml/XMLDocumentParser.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Text/UnicodeHelpers.hpp"
#include <algorithm>
#include <ranges>

namespace krys::boo
{
  using css::PropertyId;
  using css::ValueId;
  using dom::TreeQueries;

  namespace
  {
    KRYS_NODISCARD bool HasTagName(const Node &node, html::TagName tagName) noexcept
    {
      return IsHTMLElementOfType(&node, tagName);
    }

    // Composed tree traversal, ignoring user agent shadow trees.

    KRYS_NODISCARD Node *FirstChildInComposedTree(Node &node) noexcept
    {
      if (auto *element = DynamicDowncast<Element>(node))
      {
        if (auto *shadowRoot = TreeQueries::GetShadowRoot(*element);
            shadowRoot && shadowRoot->Mode() != dom::ShadowRootMode::UserAgent)
        {
          return shadowRoot->FirstChild();
        }
      }
      if (auto *slot = DynamicDowncast<HTMLSlotElement>(node))
      {
        if (auto *assignedNodes = slot->AssignedNodes(); assignedNodes && !assignedNodes->empty())
        {
          return assignedNodes->front().get();
        }
      }
      return node.FirstChild();
    }

    KRYS_NODISCARD Node *NextSiblingInComposedTree(Node &node) noexcept
    {
      auto *slot = dom::SlotAlgorithms::IsSlottable(node) ? dom::SlotAlgorithms::FindSlot(node) : nullptr;
      if (slot && !slot->IsInUserAgentShadowTree())
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

    KRYS_NODISCARD Node *NextSkippingChildrenInComposedTree(Node &node) noexcept
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

    KRYS_NODISCARD utf8_string DirectionAttributeAndValue(text::TextDirection direction) noexcept
    {
      return direction == text::TextDirection::LTR ? utf8_string(u8"dir=\"ltr\"")
                                                   : utf8_string(u8"dir=\"rtl\"");
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
      DoNotPreserve,
    };

    /// @brief Serializes a range with the styles that give it its appearance inlined.
    class StyledMarkupAccumulator final : public MarkupAccumulator
    {
    public:
      enum RangeFullySelectsNode : bool
      {
        DoesFullySelectNode,
        DoesNotFullySelectNode,
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
        DoNotEmitString,
      };

      Position _start;
      Position _end;
      List<utf8_string> _reversedPrecedingMarkup;
      const AnnotateForInterchange _annotate;
      RefPtr<Node> _highestNodeToBeSerialized;
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
      StyledMarkupAccumulator(const Position &start, Position end, List<Ref<Node>> *nodes,
                              ResolveUrls resolveUrls, SerializeComposedTree serializeComposedTree,
                              IgnoreUserSelectNone ignoreUserSelectNone, AnnotateForInterchange annotate,
                              StandardFontFamilySerializationMode standardFontFamilySerializationMode,
                              MSOListMode msoListMode, bool needsPositionStyleConversion,
                              PreserveDirectionForInlineText preserveDirectionForInlineText,
                              Node *highestNodeToBeSerialized = nullptr) noexcept
          : MarkupAccumulator(nodes, resolveUrls, GetSerializationSyntax(*start.GetDocument())),
            _start(start), _end(krys::move(end)), _annotate(annotate),
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

      RefPtr<Node> SerializeNodes(const Position &start, const Position &end) noexcept;

      void WrapWithNode(Node &node, bool convertBlocksToInlines = false,
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

      KRYS_NODISCARD ContainerNode *ParentNode(Node &node) const noexcept
      {
        if (_useComposedTree) KRYS_UNLIKELY
        {
          return node.ParentInComposedTree();
        }
        return node.ParentOrShadowHostNode();
      }

      // FIXME: The `meta charset` for clients that read the markup back from the pasteboard as UTF-8, with
      // the pasteboard's HTML.
      void PrependHeadIfNecessary(const Element *baseElement) noexcept
      {
        bool shouldAppendMetaCharset = false;
        if (!shouldAppendMetaCharset && !baseElement)
        {
          return;
        }

        _reversedPrecedingMarkup.emplace_back(u8"</head>");
        if (baseElement)
        {
          utf8_string markupForBase;
          AppendStartTag(markupForBase, *baseElement, false, DoesNotFullySelectNode);
          _reversedPrecedingMarkup.push_back(krys::move(markupForBase));
        }
        if (shouldAppendMetaCharset)
        {
          _reversedPrecedingMarkup.emplace_back(u8"<meta charset=\"UTF-8\">");
        }
        _reversedPrecedingMarkup.emplace_back(u8"<head>");
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
                                  Maybe<text::TextDirection> directionToAppend = null) noexcept;

      KRYS_NODISCARD static utf8_stringview StyleNodeCloseTag(bool isBlock = false) noexcept
      {
        return isBlock ? u8"</div>" : u8"</span>";
      }

      KRYS_NODISCARD utf8_string RenderedTextRespectingRange(const Text &text) noexcept;

      KRYS_NODISCARD utf8_string TextContentRespectingRange(const Text &text) noexcept;

      KRYS_NODISCARD bool ShouldPreserveMSOListStyleForElement(const Element &element) const noexcept;

      KRYS_NODISCARD static SpanReplacementType SpanReplacementForElement(const Element &element) noexcept;

      void AppendStartTag(utf8_string &out, const Element &element, bool addDisplayInline,
                          RangeFullySelectsNode rangeFullySelectsNode) noexcept;

      void AppendEndTag(utf8_string &out, const Element &element) noexcept override;

      void AppendCustomAttributes(utf8_string &out, const Element &element,
                                  Namespaces *namespaces) noexcept override;

      void AppendText(utf8_string &out, const Text &text) noexcept override;

      void AppendStartTag(utf8_string &out, const Element &element, Namespaces *) noexcept override
      {
        AppendStartTag(out, element, false, DoesFullySelectNode);
      }

      KRYS_NODISCARD Node *FirstChild(Node &node) const noexcept
      {
        if (_useComposedTree) KRYS_UNLIKELY
        {
          return FirstChildInComposedTree(node);
        }
        return node.FirstChild();
      }

      KRYS_NODISCARD Node *NextSibling(Node &node) const noexcept
      {
        if (_useComposedTree) KRYS_UNLIKELY
        {
          return NextSiblingInComposedTree(node);
        }
        return node.NextSibling();
      }

      KRYS_NODISCARD Node *NextSkippingChildren(Node &node) const noexcept
      {
        if (_useComposedTree) KRYS_UNLIKELY
        {
          return NextSkippingChildrenInComposedTree(node);
        }
        return TreeQueries::NextSkippingChildren(node);
      }

      KRYS_NODISCARD bool HasChildNodes(Node &node) const noexcept
      {
        if (_useComposedTree) KRYS_UNLIKELY
        {
          return FirstChildInComposedTree(node);
        }
        return node.HasChildNodes();
      }

      KRYS_NODISCARD bool IsDescendantOf(Node &node, Node &possibleAncestor) const noexcept
      {
        if (_useComposedTree) KRYS_UNLIKELY
        {
          return TreeQueries::IsShadowIncludingDescendant(node, possibleAncestor);
        }
        return node.IsDescendantOf(&possibleAncestor);
      }

      RefPtr<Node> TraverseNodesForSerialization(Node &startNode, Node *pastEnd,
                                                 NodeTraversalMode traversalMode) noexcept;

      bool AppendNodeToPreserveMSOList(Node &node) noexcept;

      KRYS_NODISCARD bool ShouldAnnotate() const noexcept
      {
        return _annotate == AnnotateForInterchange::Yes;
      }

      KRYS_NODISCARD bool ShouldApplyWrappingStyle(const Node &node) const noexcept
      {
        return _highestNodeToBeSerialized && _highestNodeToBeSerialized->ParentNode() == node.ParentNode()
               && _wrappingStyle && _wrappingStyle->Style();
      }
    };

    void StyledMarkupAccumulator::WrapWithNode(Node &node, bool convertBlocksToInlines,
                                               RangeFullySelectsNode rangeFullySelectsNode) noexcept
    {
      utf8_string markup;
      if (auto *element = DynamicDowncast<Element>(node))
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

    void
      StyledMarkupAccumulator::AppendStyleNodeOpenTag(utf8_string &out, css::StyleProperties *style,
                                                      bool isBlock,
                                                      Maybe<text::TextDirection> directionToAppend) noexcept
    {
      // With AnnotateForInterchange::Yes, wrappingStyleForSerialization should have removed
      // -webkit-text-decorations-in-effect
      krys_debug_assert(!ShouldAnnotate()
                        || PropertyMissingOrEqualToNone(style, PropertyId::InternalTextDecorationsInEffect));
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

    void StyledMarkupAccumulator::AppendText(utf8_string &out, const Text &text) noexcept
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
                               [&]() -> Maybe<text::TextDirection>
                               {
                                 if (_hasAppendedAnyText)
                                 {
                                   return null;
                                 }

                                 if (!_preserveDirectionForInlineText)
                                 {
                                   return null;
                                 }

                                 auto directionFromText = text::BaseTextDirection(text.WholeText());
                                 if (!directionFromText)
                                 {
                                   return null;
                                 }

                                 auto enclosingBlockDirection = DirectionOfEnclosingBlock(
                                   Position(ShareRefPtr(const_cast<Text *>(&text)), 0));
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
        const bool useRenderedText =
          !EnclosingElementWithTag(FirstPositionInNode(const_cast<Text *>(&text)), html::TagName::select);
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

    utf8_string StyledMarkupAccumulator::RenderedTextRespectingRange(const Text &text) noexcept
    {
      TextIteratorBehaviors behaviors;
      auto *mutableText = const_cast<Text *>(&text);
      Position start = &text == _start.GetContainerNode() ? _start : FirstPositionInNode(mutableText);
      Position end;
      if (&text == _end.GetContainerNode())
      {
        end = _end;
      }
      else
      {
        end = LastPositionInNode(mutableText);
        if (!_end.IsNull())
        {
          behaviors.Add(TextIteratorBehavior::BehavesAsIfNodesFollowing);
        }
      }
      if (_ignoresUserSelectNone)
      {
        behaviors.Add(TextIteratorBehavior::IgnoresUserSelectNone);
      }

      auto range = CreateSimpleRange(PositionRange {start, end});
      return range ? PlainText(*range, behaviors) : utf8_string();
    }

    utf8_string StyledMarkupAccumulator::TextContentRespectingRange(const Text &text) noexcept
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

    // (No attachment identifiers: `<attachment>` is WebKit's, not a standard element.)
    void StyledMarkupAccumulator::AppendCustomAttributes(utf8_string &, const Element &,
                                                         Namespaces *) noexcept
    {
    }

    bool StyledMarkupAccumulator::ShouldPreserveMSOListStyleForElement(const Element &element) const noexcept
    {
      if (_inMSOList)
      {
        return true;
      }
      if (_shouldPreserveMSOList)
      {
        auto style = element.GetAttribute(u8"style").value_or(DOMString());
        utf8_stringview view(style);
        return view.starts_with(u8"mso-list:") || view.contains(u8";mso-list:")
               || view.contains(u8"\nmso-list:");
      }
      return false;
    }

    // FIXME: Data detector elements, with data detection.
    StyledMarkupAccumulator::SpanReplacementType
      StyledMarkupAccumulator::SpanReplacementForElement(const Element &element) noexcept
    {
      if (Is<HTMLSlotElement>(element))
      {
        return SpanReplacementType::Slot;
      }

      return SpanReplacementType::None;
    }

    void StyledMarkupAccumulator::AppendStartTag(utf8_string &out, const Element &element,
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
        Is<HTMLElement>(element) && (ShouldAnnotate() || addDisplayInline);
      bool shouldOverrideStyleAttr = (shouldAnnotateOrForceInline || ShouldApplyWrappingStyle(element)
                                      || replacementType != SpanReplacementType::None)
                                     && !ShouldPreserveMSOListStyleForElement(element);
      for (const auto &attribute : element.AttributeList())
      {
        // We'll handle the style attribute separately, below.
        if (attribute->NamespaceURI().View().empty() && attribute->LocalName() == u8"style"
            && shouldOverrideStyleAttr)
        {
          continue;
        }
        (void)AppendAttribute(out, element, *attribute, nullptr);
      }

      if (shouldOverrideStyleAttr)
      {
        RefPtr<EditingStyle> newInlineStyle;
        auto &mutableElement = const_cast<Element &>(element);

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

        if (auto *styledElement = DynamicDowncast<StyledElement>(element);
            styledElement && styledElement->InlineStyle())
        {
          newInlineStyle->OverrideWithStyle(*styledElement->InlineStyle());
        }

        if (shouldAnnotateOrForceInline)
        {
          if (ShouldAnnotate())
          {
            newInlineStyle->MergeStyleFromRulesForSerialization(Downcast<HTMLElement>(mutableElement),
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

    void StyledMarkupAccumulator::AppendEndTag(utf8_string &out, const Element &element) noexcept
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

    RefPtr<Node> StyledMarkupAccumulator::SerializeNodes(const Position &start, const Position &end) noexcept
    {
      krys_debug_assert(start <= end);
      RefPtr<Node> startNode = start.FirstNode();
      if (!startNode)
      {
        return nullptr;
      }
      RefPtr<Node> pastEnd = ShareRefPtr(end.ComputeNodeAfterPosition());
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

    RefPtr<Node>
      StyledMarkupAccumulator::TraverseNodesForSerialization(Node &startNode, Node *pastEnd,
                                                             NodeTraversalMode traversalMode) noexcept
    {
      const bool shouldEmit = traversalMode == NodeTraversalMode::EmitString;
      UserSelectNoneStateCache userSelectNoneStateCache(_useComposedTree ? dom::TreeType::ComposedTree
                                                                         : dom::TreeType::Tree);

      _inMSOList = false;

      uint32 depth = 0;
      auto enterNode = [&](Node &node)
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

        auto *element = DynamicDowncast<Element>(node);
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

      RefPtr<Node> lastClosed;
      auto exitNode = [&](Node &node)
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

      RefPtr<Node> lastNode;
      RefPtr<Node> next;
      for (RefPtr<Node> n = ShareRefPtr(&startNode); n != pastEnd; lastNode = n, n = next)
      {
        List<RefPtr<Node>> exitedAncestors;
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
            exitedAncestors.push_back(ShareRefPtr<Node>(ancestor));
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

    bool StyledMarkupAccumulator::AppendNodeToPreserveMSOList(Node &node) noexcept
    {
      if (auto *commentNode = DynamicDowncast<Comment>(node))
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
        auto *textChild = DynamicDowncast<Text>(node.FirstChild());
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

    KRYS_NODISCARD Node *AncestorToRetainStructureAndAppearanceForBlock(Node *commonAncestorBlock) noexcept
    {
      if (!commonAncestorBlock)
      {
        return nullptr;
      }

      if (HasTagName(*commonAncestorBlock, html::TagName::tbody)
          || HasTagName(*commonAncestorBlock, html::TagName::tr))
      {
        ContainerNode *table = commonAncestorBlock->ParentNode();
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

    KRYS_NODISCARD Node *AncestorToRetainStructureAndAppearance(Node *commonAncestor) noexcept
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

    KRYS_NODISCARD RefPtr<EditingStyle> StyleFromMatchedRulesAndInlineDecl(Node &node) noexcept
    {
      auto *element = DynamicDowncast<HTMLElement>(node);
      if (!element)
      {
        return nullptr;
      }

      auto style = EditingStyle::Create(element->InlineStyle());
      style->MergeStyleFromRules(*element);
      return style;
    }

    KRYS_NODISCARD bool IsElementPresentational(const Node &node) noexcept
    {
      return HasTagName(node, html::TagName::u) || HasTagName(node, html::TagName::s)
             || HasTagName(node, html::TagName::strike) || HasTagName(node, html::TagName::i)
             || HasTagName(node, html::TagName::em) || HasTagName(node, html::TagName::b)
             || HasTagName(node, html::TagName::strong);
    }

    KRYS_NODISCARD RefPtr<Node> HighestAncestorToWrapMarkup(const Position &start, const Position &end,
                                                            Node &commonAncestor,
                                                            AnnotateForInterchange annotate) noexcept
    {
      RefPtr<Node> specialCommonAncestor;
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
            specialCommonAncestor = ShareRefPtr<Node>(parentListNode->ParentNode());
            while (specialCommonAncestor && !IsListHTMLElement(specialCommonAncestor.get()))
            {
              specialCommonAncestor = ShareRefPtr<Node>(specialCommonAncestor->ParentNode());
            }
          }
        }
      }

      RefPtr<Node> checkAncestor =
        specialCommonAncestor ? specialCommonAncestor : ShareRefPtr(&commonAncestor);
      if (checkAncestor->Renderer() && checkAncestor->Renderer()->ContainingBlock())
      {
        auto newSpecialCommonAncestor =
          HighestEnclosingNodeOfType(FirstPositionInNode(checkAncestor.get()), &IsElementPresentational,
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
        specialCommonAncestor = ShareRefPtr<Node>(commonAncestor.ParentNode());
      }
      if (!specialCommonAncestor && TabSpanNode(&commonAncestor))
      {
        specialCommonAncestor = ShareRefPtr(&commonAncestor);
      }

      if (auto *enclosingAnchor = EnclosingElementWithTag(
            FirstPositionInNode(specialCommonAncestor ? specialCommonAncestor.get() : &commonAncestor),
            html::TagName::a))
      {
        specialCommonAncestor = ShareRefPtr<Node>(enclosingAnchor);
      }

      if (auto *enclosingPicture = EnclosingElementWithTag(
            FirstPositionInNode(specialCommonAncestor ? specialCommonAncestor.get() : &commonAncestor),
            html::TagName::picture))
      {
        specialCommonAncestor = ShareRefPtr<Node>(enclosingPicture);
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
      const Position &start, const Position &end, List<Ref<Node>> *nodes, ResolveUrls resolveUrls,
      SerializeComposedTree serializeComposedTree, IgnoreUserSelectNone ignoreUserSelectNone,
      AnnotateForInterchange annotate, ConvertBlocksToInlines convertBlocksToInlines,
      StandardFontFamilySerializationMode standardFontFamilySerializationMode, MSOListMode msoListMode,
      PreserveBaseElement preserveBaseElement,
      PreserveDirectionForInlineText preserveDirectionForInlineText) noexcept
    {
      auto interchangeNewlineString = InterchangeNewlineString();

      if (!(start < end))
      {
        return {};
      }

      RefPtr<Node> commonAncestor = ShareRefPtr(CommonInclusiveAncestor(start, end));
      if (!commonAncestor)
      {
        return {};
      }

      Ref<Document> document = ShareRef(*start.GetDocument());
      (void)document->UpdateLayoutIgnorePendingStylesheets();

      VisiblePosition visibleStart {start};
      VisiblePosition visibleEnd {end};

      RefPtr<Element> body =
        ShareRefPtr(EnclosingElementWithTag(FirstPositionInNode(commonAncestor.get()), html::TagName::body));
      RefPtr<Element> fullySelectedRoot;
      // FIXME: Do this for all fully selected blocks, not just the body.
      if (body && VisiblePosition(FirstPositionInNode(body.get())) == visibleStart
          && VisiblePosition(LastPositionInNode(body.get())) == visibleEnd)
      {
        fullySelectedRoot = body;
      }
      bool needsPositionStyleConversion =
        body && fullySelectedRoot == body && document->GetSettings().ShouldConvertPositionStyleOnCopy;

      auto specialCommonAncestor = HighestAncestorToWrapMarkup(start, end, *commonAncestor, annotate);

      StyledMarkupAccumulator accumulator(start, end, nodes, resolveUrls, serializeComposedTree,
                                          ignoreUserSelectNone, annotate, standardFontFamilySerializationMode,
                                          msoListMode, needsPositionStyleConversion,
                                          preserveDirectionForInlineText, specialCommonAncestor.get());

      Position adjustedStart = start;

      if (auto *pictureElement = EnclosingElementWithTag(adjustedStart, html::TagName::picture))
      {
        adjustedStart = FirstPositionInNode(pictureElement);
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
        for (RefPtr<Node> ancestor = ShareRefPtr<Node>(accumulator.ParentNode(*lastClosed)); ancestor;
             ancestor = ShareRefPtr<Node>(accumulator.ParentNode(*ancestor)))
        {
          if (fullySelectedRoot && ancestor == fullySelectedRoot
              && convertBlocksToInlines == ConvertBlocksToInlines::No)
          {
            auto fullySelectedRootStyle = StyleFromMatchedRulesAndInlineDecl(*fullySelectedRoot);

            // Bring the background attribute over, but not as an attribute because a background attribute on
            // a div appears to have no effect.
            DOMStringAtom backgroundAttr(u8"background");
            if ((!fullySelectedRootStyle || !fullySelectedRootStyle->Style()
                 || !fullySelectedRootStyle->Style()->GetPropertyCSSValue(PropertyId::BackgroundImage))
                && fullySelectedRoot->HasAttribute(backgroundAttr))
            {
              utf8_string backgroundImage(u8"url('");
              backgroundImage.append(fullySelectedRoot->GetAttribute(backgroundAttr).value_or(DOMString()));
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
              if (!PropertyMissingOrEqualToNone(style, PropertyId::InternalTextDecorationsInEffect))
              {
                (void)style->SetProperty(PropertyId::InternalTextDecorationsInEffect, ValueId::None);
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
            nodes->emplace_back(ancestor);
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

    void FillContainerFromString(ContainerNode &paragraph, utf8_stringview string) noexcept
    {
      auto &document = paragraph.NodeDocument();

      if (string.empty())
      {
        (void)paragraph.AppendChild(*CreateBlockPlaceholderElement(document));
        return;
      }

      krys_debug_assert(!string.contains(u8'\n'));

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
            (void)paragraph.AppendChild(*CreateTabSpanElement(document, tabText));
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
          (void)paragraph.AppendChild(*CreateTabSpanElement(document, tabText));
        }

        first = false;
      }
    }

    KRYS_NODISCARD bool ContextPreservesNewline(const dom::SimpleRange &context) noexcept
    {
      auto *container =
        VisiblePosition(CreateDeprecatedLegacyPosition(context.Start)).DeepEquivalent().GetContainerNode();
      return container && container->Renderer() && container->Renderer()->Style().PreserveNewline();
    }

    List<Ref<Element>> CollectElementsToRemoveFromFragment(ContainerNode &container) noexcept
    {
      List<Ref<Element>> toRemove;
      for (auto *child = container.FirstChild(); child; child = child->NextSibling())
      {
        auto *element = DynamicDowncast<HTMLElement>(*child);
        if (!element)
        {
          continue;
        }
        if (HasTagName(*element, html::TagName::html))
        {
          toRemove.push_back(ShareRef<Element>(*element));
          auto nested = CollectElementsToRemoveFromFragment(*element);
          (void)nested;
          continue;
        }
        if (HasTagName(*element, html::TagName::head) || HasTagName(*element, html::TagName::body))
        {
          toRemove.push_back(ShareRef<Element>(*element));
        }
      }
      return toRemove;
    }

    void RemoveElementFromFragmentPreservingChildren(DocumentFragment &fragment, Element &element) noexcept
    {
      RefPtr<Node> nextChild;
      for (RefPtr<Node> child = ShareRefPtr(element.FirstChild()); child; child = nextChild)
      {
        nextChild = ShareRefPtr(child->NextSibling());
        (void)element.RemoveChild(*child);
        (void)fragment.InsertBefore(*child, &element);
      }
      (void)fragment.RemoveChild(element);
    }

    KRYS_NODISCARD Ref<DocumentFragment> ParseHTMLIntoFragment(utf8_stringview markup,
                                                               Element &contextElement) noexcept
    {
      return html::DocumentParser::ParseFragment(contextElement, utf8_string(markup));
    }

    // FIXME: The template document for contexts that are templates.
    KRYS_NODISCARD ExceptionOr<Ref<DocumentFragment>> CreateFragmentForMarkup(Element &contextElement,
                                                                              utf8_stringview markup) noexcept
    {
      auto &document = contextElement.NodeDocument();
      if (Is<HTMLDocument>(document))
      {
        return ParseHTMLIntoFragment(markup, contextElement);
      }

      Ref<DocumentFragment> fragment = document.CreateDocumentFragment();
      if (!xml::XMLDocumentParser::ParseDocumentFragment(markup, *fragment, &contextElement))
      {
        return Exception {ExceptionCode::SyntaxError};
      }
      return fragment;
    }
  }

  UserSelectNoneStateCache::UserSelectNoneStateCache(dom::TreeType treeType) noexcept
      : _useComposedTree(treeType == dom::TreeType::ComposedTree)
  {
    krys_debug_assert(treeType == dom::TreeType::Tree || treeType == dom::TreeType::ComposedTree);
  }

  ContainerNode *UserSelectNoneStateCache::ParentNode(Node &node) const noexcept
  {
    if (_useComposedTree)
    {
      return node.ParentInComposedTree();
    }
    return node.ParentOrShadowHostNode();
  }

  Node *UserSelectNoneStateCache::FirstChild(Node &node) const noexcept
  {
    if (_useComposedTree)
    {
      return FirstChildInComposedTree(node);
    }
    return node.FirstChild();
  }

  Node *UserSelectNoneStateCache::NextSibling(Node &node) const noexcept
  {
    if (_useComposedTree)
    {
      return NextSiblingInComposedTree(node);
    }
    return node.NextSibling();
  }

  UserSelectNoneStateCache::State UserSelectNoneStateCache::ComputeState(Node &targetNode) noexcept
  {
    if (auto it = _cache.find(&targetNode); it != _cache.end())
    {
      return it->second;
    }
    if (!Position::NodeIsUserSelectNone(&targetNode))
    {
      return State::NotUserSelectNone;
    }
    auto state = State::OnlyUserSelectNone;
    Node *currentNode = &targetNode;
    bool foundMixed = false;
    while (currentNode)
    {
      if (!Position::NodeIsUserSelectNone(currentNode))
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
        Node *ancestor;
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

  utf8_string SerializePreservingVisualAppearance(const dom::SimpleRange &range, List<Ref<Node>> *nodes,
                                                  AnnotateForInterchange annotate,
                                                  ConvertBlocksToInlines convertBlocksToInlines,
                                                  ResolveUrls resolveUrls) noexcept
  {
    return SerializePreservingVisualAppearanceInternal(
      CreateDeprecatedLegacyPosition(range.Start), CreateDeprecatedLegacyPosition(range.End), nodes,
      resolveUrls, SerializeComposedTree::No, IgnoreUserSelectNone::No, annotate, convertBlocksToInlines,
      StandardFontFamilySerializationMode::Keep, MSOListMode::DoNotPreserve, PreserveBaseElement::No,
      PreserveDirectionForInlineText::No);
  }

  utf8_string SerializePreservingVisualAppearance(
    const VisibleSelection &selection, ResolveUrls resolveUrls, SerializeComposedTree serializeComposedTree,
    IgnoreUserSelectNone ignoreUserSelectNone, PreserveBaseElement preserveBaseElement,
    PreserveDirectionForInlineText preserveDirectionForInlineText, List<Ref<Node>> *nodes) noexcept
  {
    return SerializePreservingVisualAppearanceInternal(
      selection.Start(), selection.End(), nodes, resolveUrls, serializeComposedTree, ignoreUserSelectNone,
      AnnotateForInterchange::Yes, ConvertBlocksToInlines::No, StandardFontFamilySerializationMode::Keep,
      MSOListMode::DoNotPreserve, preserveBaseElement, preserveDirectionForInlineText);
  }

  // FIXME: The destination page's dark appearance (setUseColorAppearance), with dark appearance.
  Ref<Frame> CreateFrameForSanitizingWebContent(Document *) noexcept
  {
    Ref<Frame> frame = Frame::Create();
    frame->CreateView(SizeI {800, 600});

    constexpr static utf8_stringview markup = u8"<!DOCTYPE html><html><head><meta name='color-scheme' "
                                              u8"content='light dark'/></head><body></body></html>";
    DocumentWriter writer;
    writer.SetFrame(*frame);
    writer.SetMimeType(u8"text/html");
    if (writer.Begin({}))
    {
      writer.AddData(Span<const uint8>(reinterpret_cast<const uint8 *>(markup.data()), markup.size()));
      writer.End();
      krys_assert(frame->Document() && frame->Document()->Body());
    }

    return frame;
  }

  Maybe<utf8_string> SanitizeMarkup(utf8_stringview rawHTML, Document *destinationDocument,
                                    MSOListQuirks msoListQuirks,
                                    const Func<void(DocumentFragment &)> &fragmentSanitizer) noexcept
  {
    Ref<Frame> frame = CreateFrameForSanitizingWebContent(destinationDocument);
    RefPtr<boo::Document> stagingDocument = ShareRefPtr(frame->Document());
    if (!stagingDocument)
    {
      return null;
    }

    auto fragment = CreateFragmentFromMarkup(*stagingDocument, rawHTML, {});

    if (fragmentSanitizer)
    {
      fragmentSanitizer(*fragment);
    }

    auto markup =
      SanitizedMarkupForFragmentInDocument(krys::move(fragment), *stagingDocument, msoListQuirks, rawHTML);
    frame->SetDocument(nullptr);
    return markup;
  }

  namespace
  {
    KRYS_NODISCARD bool ShouldPreserveMSOLists(utf8_stringview markup) noexcept
    {
      if (!markup.starts_with(u8"<html xmlns:"))
      {
        return false;
      }
      auto tagClose = markup.find(u8'>');
      if (tagClose == utf8_stringview::npos)
      {
        return false;
      }
      auto tag = markup.substr(0, tagClose);
      return tag.contains(u8"xmlns:o=\"urn:schemas-microsoft-com:office:office\"")
             && tag.contains(u8"xmlns:w=\"urn:schemas-microsoft-com:office:word\"");
    }
  }

  utf8_string SanitizedMarkupForFragmentInDocument(const Ref<DocumentFragment> &fragment, Document &document,
                                                   MSOListQuirks msoListQuirks,
                                                   utf8_stringview originalMarkup) noexcept
  {
    MSOListMode msoListMode =
      msoListQuirks == MSOListQuirks::CheckIfNeeded && ShouldPreserveMSOLists(originalMarkup)
        ? MSOListMode::Preserve
        : MSOListMode::DoNotPreserve;

    RefPtr<Element> bodyElement = document.Body();
    krys_debug_assert(bodyElement);
    (void)bodyElement->AppendChild(*fragment);

    // SerializeComposedTree::No because there can't be a shadow tree in the pasted fragment.
    auto result = SerializePreservingVisualAppearanceInternal(
      FirstPositionInNode(bodyElement.get()), LastPositionInNode(bodyElement.get()), nullptr,
      ResolveUrls::YesExcludingUrlsForPrivacy, SerializeComposedTree::No, IgnoreUserSelectNone::No,
      AnnotateForInterchange::Yes, ConvertBlocksToInlines::No, StandardFontFamilySerializationMode::Strip,
      msoListMode, PreserveBaseElement::No, PreserveDirectionForInlineText::No);

    if (msoListMode != MSOListMode::Preserve)
    {
      return result;
    }

    return u8"<html xmlns:o=\"urn:schemas-microsoft-com:office:office\"\n"
           u8"xmlns:w=\"urn:schemas-microsoft-com:office:word\"\n"
           u8"xmlns:m=\"http://schemas.microsoft.com/office/2004/12/omml\"\n"
           u8"xmlns=\"http://www.w3.org/TR/REC-html40\">"
           + result + u8"</html>";
  }

  // FIXME: Completing relative URLs against baseURL (completeURLs), with URL attributes. (No attachment
  // elements' files to restore: `<attachment>` is WebKit's, not a standard element.)
  Ref<DocumentFragment> CreateFragmentFromMarkup(Document &document, utf8_stringview markup,
                                                 utf8_stringview baseUrl) noexcept
  {
    (void)baseUrl;
    // We use a fake body element here to trick the HTML parser into using the InBody insertion mode.
    auto fakeBody = CreateHTMLElement(document, u8"body");
    return ParseHTMLIntoFragment(markup, *fakeBody);
  }

  utf8_string SerializeFragment(const Node &node, SerializedNodes root, List<Ref<Node>> *nodes,
                                ResolveUrls resolveUrls, Maybe<SerializationSyntax> serializationSyntax,
                                SerializeShadowRoots serializeShadowRoots) noexcept
  {
    return SerializeFragment(node, root, nodes, resolveUrls, serializationSyntax, serializeShadowRoots, {});
  }

  utf8_string SerializeFragment(const Node &node, SerializedNodes root, List<Ref<Node>> *nodes,
                                ResolveUrls resolveUrls, Maybe<SerializationSyntax> serializationSyntax,
                                SerializeShadowRoots serializeShadowRoots,
                                List<Ref<ShadowRoot>> &&explicitShadowRoots) noexcept
  {
    if (!serializationSyntax)
    {
      serializationSyntax = MarkupAccumulator::GetSerializationSyntax(node.NodeDocument());
    }

    MarkupAccumulator accumulator(nodes, resolveUrls, *serializationSyntax, serializeShadowRoots,
                                  krys::move(explicitShadowRoots));
    return accumulator.SerializeNodes(const_cast<Node &>(node), root);
  }

  ExceptionOr<utf8_string> SerializeFragmentRequiringWellFormed(const Node &node,
                                                                SerializedNodes root) noexcept
  {
    // 1. Let context document be node's node document.
    // 2. If context document is an HTML document, then return the result of HTML fragment serialization
    //    algorithm with node, false, and « ».
    // 3. Return the XML serialization of node given require well-formed.
    MarkupAccumulator accumulator(nullptr, ResolveUrls::NoExcludingUrlsForPrivacy,
                                  MarkupAccumulator::GetSerializationSyntax(node.NodeDocument()),
                                  SerializeShadowRoots::Explicit, {}, RequireWellFormed::Yes);
    auto markup = accumulator.SerializeNodes(const_cast<Node &>(node), root);
    if (!accumulator.IsWellFormed())
    {
      return Exception {ExceptionCode::InvalidStateError};
    }
    return markup;
  }

  bool IsPlainTextMarkup(Node *node) noexcept
  {
    krys_debug_assert(node);
    if (!HasTagName(*node, html::TagName::div) || Downcast<Element>(*node).HasAttributes())
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
      return Is<Text>(*firstChild) || firstChild->FirstChild();
    }

    if (secondChild->NextSibling())
    {
      return false;
    }

    return ParentTabSpanNode(firstChild->FirstChild()) && Is<Text>(*secondChild);
  }

  Ref<DocumentFragment> CreateFragmentFromText(const dom::SimpleRange &context, utf8_stringview text) noexcept
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
      (void)element->SetAttribute(u8"class", DOMString(AppleInterchangeNewline));
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
    if (!string.contains(u8'\n'))
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
    auto start = CreateDeprecatedLegacyPosition(context.Start);
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

      RefPtr<Element> element;
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

  utf8_string DocumentTypeString(const Document &document) noexcept
  {
    auto documentType = document.DocType();
    if (!documentType)
    {
      return {};
    }
    return SerializeFragment(*documentType, SerializedNodes::SubtreeIncludingNode);
  }

  utf8_string UrlToMarkup(utf8_stringview url, utf8_stringview title) noexcept
  {
    utf8_string markup(u8"<a href=\"");
    markup.append(url);
    markup.append(u8"\">");
    MarkupAccumulator::AppendCharactersReplacingEntities(markup, title, EntityMaskInPCDATA);
    markup.append(u8"</a>");
    return markup;
  }

  ExceptionOr<Ref<DocumentFragment>> CreateFragmentForInnerOuterHTML(Element &contextElement,
                                                                     utf8_stringview markup) noexcept
  {
    return CreateFragmentForMarkup(contextElement, markup);
  }

  ExceptionOr<Ref<DocumentFragment>> CreateContextualFragment(Element &element,
                                                              utf8_stringview markup) noexcept
  {
    auto result = CreateFragmentForMarkup(element, markup);
    if (result.HasException())
    {
      return result.ReleaseException();
    }
    Ref<DocumentFragment> fragment = result.ReleaseValue();

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
  ExceptionOr<void> ReplaceChildrenWithFragment(ContainerNode &container,
                                                const Ref<DocumentFragment> &fragment) noexcept
  {
    Ref<ContainerNode> containerNode = ShareRef(container);

    // Replacing them all at once queues a single mutation record.
    return dom::MutationAlgorithms::ReplaceAll(fragment.get(), *containerNode);
  }
}
