#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineItemsBuilder.hpp"
#include "Krystal.Booey/Core/TextSpacing.hpp"
#include "Krystal.Booey/Core/UnicodeBidi.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineSoftLineBreakItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineTextItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextBreakingPositionCache.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include <unicode/ubidi.h>
#include <unicode/utf16.h>

namespace krys::boo::layout
{
  namespace
  {
    struct WhitespaceContent
    {
      size_t Length {0};
      bool IsWordSeparator {true};
    };

    // The white space characters are ASCII, so this walks bytes.
    KRYS_NODISCARD Maybe<WhitespaceContent>
      MoveToNextNonWhitespacePosition(utf8_stringview characters, size_t startPosition, bool preserveNewline,
                                      bool preserveTab, bool stopAtWordSeparatorBoundary) noexcept
    {
      auto hasWordSeparatorCharacter = false;
      auto isWordSeparatorCharacter = false;
      auto isWhitespaceCharacter = [&](char8 character)
      {
        // white space processing in CSS affects only the document white space characters: spaces (U+0020),
        // tabs (U+0009), and segment breaks.
        auto isTreatedAsSpaceCharacter = character == u8' ' || (character == u8'\n' && !preserveNewline)
                                         || (character == u8'\t' && !preserveTab);
        isWordSeparatorCharacter = isTreatedAsSpaceCharacter;
        hasWordSeparatorCharacter = hasWordSeparatorCharacter || isWordSeparatorCharacter;
        return isTreatedAsSpaceCharacter || character == u8'\t';
      };
      auto nextNonWhiteSpacePosition = startPosition;
      while (nextNonWhiteSpacePosition < characters.size()
             && isWhitespaceCharacter(characters[nextNonWhiteSpacePosition]))
      {
        if (stopAtWordSeparatorBoundary && hasWordSeparatorCharacter && !isWordSeparatorCharacter)
          KRYS_UNLIKELY
        {
          break;
        }
        ++nextNonWhiteSpacePosition;
      }
      if (nextNonWhiteSpacePosition == startPosition)
      {
        return null;
      }
      return WhitespaceContent {nextNonWhiteSpacePosition - startPosition, hasWordSeparatorCharacter};
    }

    KRYS_NODISCARD uint32 MoveToNextBreakablePosition(
      uint32 startPosition, CachedLineBreakIteratorFactory &lineBreakIteratorFactory,
      const RenderStyle &style) noexcept
    {
      auto textLength = static_cast<uint32>(lineBreakIteratorFactory.StringView().length());
      auto startPositionForNextBreakablePosition = startPosition;
      while (startPositionForNextBreakablePosition < textLength)
      {
        auto nextBreakablePosition = TextUtil::FindNextBreakablePosition(
          lineBreakIteratorFactory, startPositionForNextBreakablePosition, style);
        // Oftentimes the next breakable position comes back as the start position (most notably hyphens).
        if (nextBreakablePosition != startPosition)
        {
          return nextBreakablePosition - startPosition;
        }
        auto text = lineBreakIteratorFactory.StringView();
        startPositionForNextBreakablePosition +=
          text::DecodeUTF8(Span<const char8>(text.data() + startPositionForNextBreakablePosition,
                                             text.size() - startPositionForNextBreakablePosition))
            .Length;
      }
      return textLength - startPosition;
    }

    /// @brief Whether the text before @p endPosition ends with a soft hyphen (two bytes in UTF-8).
    KRYS_NODISCARD bool EndsWithSoftHyphen(utf8_stringview text, size_t endPosition) noexcept
    {
      static_assert(text::SoftHyphen == 0x00AD);
      return endPosition >= 2 && text[endPosition - 2] == char8 {0xC2}
             && text[endPosition - 1] == char8 {0xAD};
    }

    KRYS_NODISCARD char32 CodePointAt(utf8_stringview text, size_t position) noexcept
    {
      return text::DecodeUTF8(Span<const char8>(text.data() + position, text.size() - position)).CodePoint;
    }

    KRYS_NODISCARD inline bool IsTextOrLineBreak(const Box &layoutBox) noexcept
    {
      return layoutBox.IsInFlow()
             && (layoutBox.IsInlineTextBox()
                 || (layoutBox.IsLineBreakBox() && !layoutBox.IsWordBreakOpportunity()));
    }

    KRYS_NODISCARD bool RequiresVisualReordering(const Box &layoutBox) noexcept
    {
      if (auto *inlineTextBox = DynamicDowncast<InlineTextBox>(layoutBox))
      {
        return inlineTextBox->HasStrongDirectionalityContent();
      }
      if (layoutBox.IsInlineBox() && layoutBox.IsInFlow())
      {
        auto &style = layoutBox.Style();
        return style.GetWritingMode().IsBidiRTL()
               || (style.RtlOrdering() == Order::Logical && style.GetUnicodeBidi() != UnicodeBidi::Normal);
      }
      return false;
    }

    void ReplaceNonPreservedNewLineAndTabCharactersAndAppend(const InlineTextBox &inlineTextBox,
                                                             StringBuilder &paragraphContentBuilder) noexcept
    {
      // ubidi prefers non-preserved new lines/tabs as space characters.
      krys_debug_assert(!TextUtil::ShouldPreserveNewline(inlineTextBox));
      auto textContent = utf8_stringview {inlineTextBox.Content()};
      auto contentLength = textContent.length();
      size_t nonReplacedContentStartPosition = 0;
      // New lines and tabs are ASCII, so they can't be part of a longer UTF-8 sequence.
      for (size_t position = 0; position < contentLength; ++position)
      {
        auto isNewLineOrTabCharacter = textContent[position] == u8'\n' || textContent[position] == u8'\t';
        if (!isNewLineOrTabCharacter)
        {
          continue;
        }

        if (nonReplacedContentStartPosition < position)
        {
          paragraphContentBuilder.AppendSubstring(textContent, nonReplacedContentStartPosition,
                                                  position - nonReplacedContentStartPosition);
        }
        paragraphContentBuilder.Append(text::Space);
        nonReplacedContentStartPosition = position + 1;
      }
      if (nonReplacedContentStartPosition < contentLength)
      {
        paragraphContentBuilder.AppendSubstring(textContent, nonReplacedContentStartPosition,
                                                contentLength - nonReplacedContentStartPosition);
      }
    }

    struct BidiContext
    {
      boo::UnicodeBidi UnicodeBidi;
      bool IsLeftToRightDirection {false};
      bool IsBlockLevel {false};
    };
    using BidiContextStack = List<BidiContext>;

    enum class EnterExitType : uint8
    {
      EnteringBlock,
      ExitingBlock,
      EnteringInlineBox,
      ExitingInlineBox
    };
    inline void HandleEnterExitBidiContext(StringBuilder &paragraphContentBuilder, UnicodeBidi unicodeBidi,
                                           bool isLTR, EnterExitType enterExitType,
                                           BidiContextStack &bidiContextStack) noexcept
    {
      if (enterExitType == EnterExitType::ExitingInlineBox && bidiContextStack.size() == 1)
      {
        // Refuse to pop the initial block entry off of the stack. It indicates unbalanced InlineBoxStart/End
        // pairs.
        krys_debug_assert(false);
        return;
      }

      auto isEnteringBidi =
        enterExitType == EnterExitType::EnteringBlock || enterExitType == EnterExitType::EnteringInlineBox;
      switch (unicodeBidi)
      {
        case UnicodeBidi::Normal:
          // The box does not open an additional level of embedding with respect to the bidirectional
          // algorithm. For inline boxes, implicit reordering works across box boundaries.
          break;
        case UnicodeBidi::Embed:
          // Isolate and embed values are enforced by default and redundant on the block level boxes.
          if (enterExitType == EnterExitType::EnteringBlock)
          {
            break;
          }
          paragraphContentBuilder.Append(isEnteringBidi
                                           ? (isLTR ? text::LeftToRightEmbed : text::RightToLeftEmbed)
                                           : text::PopDirectionalFormatting);
          break;
        case UnicodeBidi::Override:
          paragraphContentBuilder.Append(isEnteringBidi
                                           ? (isLTR ? text::LeftToRightOverride : text::RightToLeftOverride)
                                           : text::PopDirectionalFormatting);
          break;
        case UnicodeBidi::Isolate:
          // Isolate and embed values are enforced by default and redundant on the block level boxes.
          if (enterExitType == EnterExitType::EnteringBlock)
          {
            break;
          }
          paragraphContentBuilder.Append(isEnteringBidi
                                           ? (isLTR ? text::LeftToRightIsolate : text::RightToLeftIsolate)
                                           : text::PopDirectionalIsolate);
          break;
        case UnicodeBidi::Plaintext:
          paragraphContentBuilder.Append(isEnteringBidi ? text::FirstStrongIsolate
                                                        : text::PopDirectionalIsolate);
          break;
        case UnicodeBidi::IsolateOverride:
          if (isEnteringBidi)
          {
            paragraphContentBuilder.Append(text::FirstStrongIsolate);
            paragraphContentBuilder.Append(isLTR ? text::LeftToRightOverride : text::RightToLeftOverride);
          }
          else
          {
            paragraphContentBuilder.Append(text::PopDirectionalFormatting);
            paragraphContentBuilder.Append(text::PopDirectionalIsolate);
          }
          break;
        default: krys_debug_assert(false);
      }

      if (isEnteringBidi)
      {
        bidiContextStack.push_back({unicodeBidi, isLTR, enterExitType == EnterExitType::EnteringBlock});
      }
      else
      {
        bidiContextStack.pop_back();
      }
    }

    inline void UnwindBidiContextStack(StringBuilder &paragraphContentBuilder,
                                       BidiContextStack &bidiContextStack,
                                       const BidiContextStack &copyOfBidiStack,
                                       size_t &blockLevelBidiContextIndex) noexcept
    {
      if (bidiContextStack.empty())
      {
        krys_debug_assert(false);
        return;
      }
      // Unwind all the way up to the block entry.
      size_t unwindingIndex = bidiContextStack.size() - 1;
      while (unwindingIndex && !copyOfBidiStack[unwindingIndex].IsBlockLevel)
      {
        HandleEnterExitBidiContext(paragraphContentBuilder, copyOfBidiStack[unwindingIndex].UnicodeBidi,
                                   copyOfBidiStack[unwindingIndex].IsLeftToRightDirection,
                                   EnterExitType::ExitingInlineBox, bidiContextStack);
        --unwindingIndex;
      }
      blockLevelBidiContextIndex = unwindingIndex;
      // and unwind the block entries as well.
      do
      {
        krys_debug_assert(copyOfBidiStack[unwindingIndex].IsBlockLevel);
        HandleEnterExitBidiContext(paragraphContentBuilder, copyOfBidiStack[unwindingIndex].UnicodeBidi,
                                   copyOfBidiStack[unwindingIndex].IsLeftToRightDirection,
                                   EnterExitType::ExitingBlock, bidiContextStack);
      } while (unwindingIndex--);
    }

    inline void RewindBidiContextStack(StringBuilder &paragraphContentBuilder,
                                       BidiContextStack &bidiContextStack,
                                       const BidiContextStack &copyOfBidiStack,
                                       size_t blockLevelBidiContextIndex) noexcept
    {
      if (copyOfBidiStack.empty())
      {
        krys_debug_assert(false);
        return;
      }

      for (size_t blockLevelIndex = 0; blockLevelIndex <= blockLevelBidiContextIndex; ++blockLevelIndex)
      {
        HandleEnterExitBidiContext(paragraphContentBuilder, copyOfBidiStack[blockLevelIndex].UnicodeBidi,
                                   copyOfBidiStack[blockLevelIndex].IsLeftToRightDirection,
                                   EnterExitType::EnteringBlock, bidiContextStack);
      }

      for (size_t inlineLevelIndex = blockLevelBidiContextIndex + 1;
           inlineLevelIndex < copyOfBidiStack.size(); ++inlineLevelIndex)
      {
        HandleEnterExitBidiContext(paragraphContentBuilder, copyOfBidiStack[inlineLevelIndex].UnicodeBidi,
                                   copyOfBidiStack[inlineLevelIndex].IsLeftToRightDirection,
                                   EnterExitType::EnteringInlineBox, bidiContextStack);
      }
    }

    using InlineItemOffsetList = List<Maybe<size_t>>;
    inline void HandleBidiParagraphStart(StringBuilder &paragraphContentBuilder,
                                         InlineItemOffsetList &inlineItemOffsetList,
                                         BidiContextStack &bidiContextStack) noexcept
    {
      // Bidi handling requires us to close all the nested bidi contexts at the end of the line triggered by
      // forced line breaks and re-open it for the content on the next line (i.e. paragraph handling).
      auto copyOfBidiStack = bidiContextStack;

      size_t blockLevelBidiContextIndex = 0;
      UnwindBidiContextStack(paragraphContentBuilder, bidiContextStack, copyOfBidiStack,
                             blockLevelBidiContextIndex);

      inlineItemOffsetList.push_back(paragraphContentBuilder.Length());
      paragraphContentBuilder.Append(text::NewlineCharacter);

      RewindBidiContextStack(paragraphContentBuilder, bidiContextStack, copyOfBidiStack,
                             blockLevelBidiContextIndex);
    }

    inline void BuildBidiParagraph(const RenderStyle &rootStyle, const InlineItemList &inlineItemList,
                                   StringBuilder &paragraphContentBuilder,
                                   InlineItemOffsetList &inlineItemOffsetList) noexcept
    {
      auto bidiContextStack = BidiContextStack {};
      HandleEnterExitBidiContext(paragraphContentBuilder, rootStyle.GetUnicodeBidi(),
                                 rootStyle.GetWritingMode().IsBidiLTR(), EnterExitType::EnteringBlock,
                                 bidiContextStack);
      if (rootStyle.RtlOrdering() != Order::Logical)
      {
        HandleEnterExitBidiContext(paragraphContentBuilder, UnicodeBidi::Override,
                                   rootStyle.GetWritingMode().IsBidiLTR(), EnterExitType::EnteringBlock,
                                   bidiContextStack);
      }

      const Box *lastInlineTextBox = nullptr;
      size_t inlineTextBoxOffset = 0;
      for (size_t index = 0; index < inlineItemList.size(); ++index)
      {
        auto &inlineItem = inlineItemList[index];
        auto &layoutBox = inlineItem.LayoutBox();

        if (inlineItem.IsText() || inlineItem.IsSoftLineBreak())
        {
          auto *inlineTextBox = DynamicDowncast<InlineTextBox>(layoutBox);
          auto mayAppendTextContentAsOneEntry =
            inlineTextBox && !TextUtil::ShouldPreserveNewline(*inlineTextBox);
          if (mayAppendTextContentAsOneEntry)
          {
            // Append the entire InlineTextBox content and keep track of individual inline item positions as
            // we process them.
            if (lastInlineTextBox != &layoutBox)
            {
              inlineTextBoxOffset = paragraphContentBuilder.Length();
              ReplaceNonPreservedNewLineAndTabCharactersAndAppend(*inlineTextBox, paragraphContentBuilder);
              lastInlineTextBox = &layoutBox;
            }
            auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem);
            inlineItemOffsetList.push_back(inlineTextBoxOffset
                                           + (inlineTextItem
                                                ? inlineTextItem->Start()
                                                : Downcast<InlineSoftLineBreakItem>(inlineItem).Position()));
          }
          else if (auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem))
          {
            inlineItemOffsetList.push_back(paragraphContentBuilder.Length());
            paragraphContentBuilder.Append(inlineTextItem->Content());
          }
          else if (Is<InlineSoftLineBreakItem>(inlineItem))
          {
            HandleBidiParagraphStart(paragraphContentBuilder, inlineItemOffsetList, bidiContextStack);
          }
          else
          {
            krys_debug_assert(false);
          }
        }
        else if (inlineItem.IsAtomicInlineBox())
        {
          inlineItemOffsetList.push_back(paragraphContentBuilder.Length());
          paragraphContentBuilder.Append(text::ObjectReplacementCharacter);
        }
        else if (inlineItem.IsInlineBoxStartOrEnd())
        {
          // https://drafts.csswg.org/css-writing-modes/#unicode-bidi
          auto &style = inlineItem.Style();
          auto initiatesControlCharacter =
            style.RtlOrdering() == Order::Logical && style.GetUnicodeBidi() != UnicodeBidi::Normal;
          if (!initiatesControlCharacter)
          {
            // Opaque items do not have position in the bidi paragraph. They inherit their bidi level from the
            // next inline item.
            inlineItemOffsetList.push_back({});
            continue;
          }
          inlineItemOffsetList.push_back(paragraphContentBuilder.Length());
          auto isEnteringBidi = inlineItem.IsInlineBoxStart();
          HandleEnterExitBidiContext(
            paragraphContentBuilder, style.GetUnicodeBidi(), style.GetWritingMode().IsBidiLTR(),
            isEnteringBidi ? EnterExitType::EnteringInlineBox : EnterExitType::ExitingInlineBox,
            bidiContextStack);
        }
        else if (inlineItem.IsHardLineBreak())
        {
          HandleBidiParagraphStart(paragraphContentBuilder, inlineItemOffsetList, bidiContextStack);
        }
        else if (inlineItem.IsWordBreakOpportunity())
        {
          // Soft wrap opportunity markers are opaque to bidi.
          inlineItemOffsetList.push_back({});
        }
        else if (inlineItem.IsFloat())
        {
          // Floats are not part of the inline content which make them opaque to bidi.
          inlineItemOffsetList.push_back({});
        }
        else if (inlineItem.IsOpaque())
        {
          if (inlineItem.LayoutBox().IsOutOfFlowPositioned())
          {
            // Out of flow boxes participate in inflow layout as if they were static positioned.
            inlineItemOffsetList.push_back(paragraphContentBuilder.Length());
            paragraphContentBuilder.Append(text::ObjectReplacementCharacter);
          }
          else
          {
            // truly opaque items are also opaque to bidi.
            inlineItemOffsetList.push_back({});
          }
        }
        else if (inlineItem.IsBlock())
        {
          HandleBidiParagraphStart(paragraphContentBuilder, inlineItemOffsetList, bidiContextStack);
        }
        else
        {
          krys_debug_assert(false);
        }
      }
    }

    KRYS_NODISCARD inline bool CanCacheMeasuredWidthOnInlineTextItem(const InlineTextBox &inlineTextBox,
                                                                     bool isWhitespace) noexcept
    {
      // Do not cache when:
      // 1. first-line style's unique font properties may produce non-matching width values.
      // 2. position dependent content is present (preserved tab character atm).
      if (&inlineTextBox.Style() != &inlineTextBox.FirstLineStyle()
          && !inlineTextBox.Style().FontCascadeEqual(inlineTextBox.FirstLineStyle()))
      {
        return false;
      }
      if (!isWhitespace || !TextUtil::ShouldPreserveSpacesAndTabs(inlineTextBox))
      {
        return true;
      }
      return !inlineTextBox.HasPositionDependentContentWidth();
    }

    void HandleTextSpacing(text_spacing::SpacingState &spacingState,
                           TrimmableTextSpacings &trimmableTextSpacings, const InlineTextItem &inlineTextItem,
                           size_t inlineItemIndex) noexcept
    {
      auto autospace = inlineTextItem.Style().GetTextAutospace();
      if (!autospace.IsNoAutospace())
      {
        // We need to store information about spacing added between inline text items since it needs to be
        // trimmed during line breaking if the consecutive items are placed on different lines
        auto content = inlineTextItem.Content();
        auto characterClass = text_spacing::GetCharacterClass(CodePointAt(content, 0));
        if (autospace.ShouldApplySpacing(spacingState.LastCharacterClassFromPreviousRun, characterClass))
        {
          trimmableTextSpacings.try_emplace(
            inlineItemIndex,
            TextAutospace::TextAutospaceSize(*inlineTextItem.Style().GetFontCascade().PrimaryFont()));
        }

        auto lastCharacterFromPreviousRun = TextUtil::LastBaseCharacterFromText(content);
        spacingState.LastCharacterClassFromPreviousRun =
          text_spacing::GetCharacterClass(lastCharacterFromPreviousRun);
      }
      else
      {
        spacingState.LastCharacterClassFromPreviousRun = text_spacing::CharacterClass::Undefined;
      }
    }

    /// @brief The UTF-16 form of UTF-8 text, for ICU's bidi algorithm, with the byte offset of each UTF-16
    /// offset (and of the end).
    struct UTF16Paragraph
    {
      List<UChar> Characters;
      List<size_t> ByteOffsets;

      explicit UTF16Paragraph(utf8_stringview text) noexcept
      {
        Characters.reserve(text.size());
        ByteOffsets.reserve(text.size() + 1);
        for (size_t position = 0; position < text.size();)
        {
          auto decoded = text::DecodeUTF8(Span<const char8>(text.data() + position, text.size() - position));
          if (decoded.CodePoint <= 0xFFFF)
          {
            Characters.push_back(static_cast<UChar>(decoded.CodePoint));
            ByteOffsets.push_back(position);
          }
          else
          {
            Characters.push_back(U16_LEAD(decoded.CodePoint));
            Characters.push_back(U16_TRAIL(decoded.CodePoint));
            ByteOffsets.push_back(position);
            ByteOffsets.push_back(position);
          }
          position += decoded.Length;
        }
        ByteOffsets.push_back(text.size());
      }
    };
  }

  InlineItemsBuilder::InlineItemsBuilder(InlineContentCache &inlineContentCache,
                                         const ElementBox &root) noexcept
      : _inlineContentCache(inlineContentCache), _root(root),
        _hasTextAutospace(!root.Style().GetTextAutospace().IsNoAutospace())
  {
  }

  void InlineItemsBuilder::Build(InlineItemPosition startPosition) noexcept
  {
    InlineItemList inlineItemList;
    CollectInlineItems(inlineItemList, startPosition);

    if (Root().GetWritingMode().IsBidiRTL() || ContentRequiresVisualReordering())
    {
      // FIXME: Add support for partial, yet paragraph level bidi content handling.
      BreakAndComputeBidiLevels(inlineItemList);
    }

    auto &inlineItemCache = GetInlineContentCache().GetInlineItems();
    auto contentAttributes = ComputeContentAttributesAndInlineTextItemWidths(inlineItemList, startPosition,
                                                                             inlineItemCache.Content());
    auto adjustInlineContentCacheWithNewInlineItems = [&]
    {
      krys_debug_assert(!startPosition || startPosition.Index < inlineItemCache.Content().size());
      auto isPopulatedFromCache = _textContentPopulatedFromCache && *_textContentPopulatedFromCache
                                    ? InlineContentCache::InlineItems::IsPopulatedFromCache::Yes
                                    : InlineContentCache::InlineItems::IsPopulatedFromCache::No;
      if (!startPosition || startPosition.Index >= inlineItemCache.Content().size())
      {
        return inlineItemCache.Set(krys::move(inlineItemList), contentAttributes, isPopulatedFromCache);
      }
      inlineItemCache.Replace(startPosition.Index, krys::move(inlineItemList), contentAttributes,
                              isPopulatedFromCache);
    };
    adjustInlineContentCacheWithNewInlineItems();

#if KRYS_DEBUG
    // Check if we've got matching inline box start/end pairs and unique inline level items (non-text,
    // non-inline box items).
    size_t inlineBoxStart = 0;
    size_t inlineBoxEnd = 0;
    auto inlineLevelItems = HashSet<const Box *> {};
    for (auto &inlineItem : GetInlineContentCache().GetInlineItems().Content())
    {
      if (inlineItem.IsInlineBoxStart())
      {
        ++inlineBoxStart;
      }
      else if (inlineItem.IsInlineBoxEnd())
      {
        ++inlineBoxEnd;
      }
      else
      {
        auto hasToBeUniqueLayoutBox =
          inlineItem.IsAtomicInlineBox() || inlineItem.IsFloat() || inlineItem.IsHardLineBreak();
        if (hasToBeUniqueLayoutBox)
        {
          krys_debug_assert(inlineLevelItems.insert(&inlineItem.LayoutBox()).second);
        }
      }
    }
    krys_debug_assert(inlineBoxStart == inlineBoxEnd);
#endif
  }

  void InlineItemsBuilder::ComputeInlineBoxBoundaryTextSpacings(const InlineItemList &inlineItemList) noexcept
  {
    krys_debug_assert(_hasTextAutospace);
    char32 lastCharacterFromPreviousRun = 0;
    size_t lastCharacterDepth = 0;
    size_t currentCharacterDepth = 0;
    InlineBoxBoundaryTextSpacings spacings;
    List<size_t> inlineBoxStartIndexesOnInlineItemsList;
    bool processInlineBoxBoundary = false;
    for (size_t inlineItemIndex = 0; inlineItemIndex < inlineItemList.size(); ++inlineItemIndex)
    {
      auto &inlineItem = inlineItemList[inlineItemIndex];
      if (inlineItem.IsInlineBoxStart())
      {
        inlineBoxStartIndexesOnInlineItemsList.push_back(inlineItemIndex);
        ++currentCharacterDepth;
        continue;
      }
      if (inlineItem.IsInlineBoxEnd())
      {
        if (!currentCharacterDepth--)
        {
          krys_debug_assert(false);
          // Skip unbalanced inline box start/end pairs.
          processInlineBoxBoundary = false;
          currentCharacterDepth = 0;
          continue;
        }
        processInlineBoxBoundary = true;
        continue;
      }
      auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem);
      if (!inlineTextItem)
      {
        continue;
      }

      auto content = inlineTextItem->Content();
      if (!processInlineBoxBoundary || !lastCharacterFromPreviousRun)
      {
        lastCharacterFromPreviousRun = TextUtil::LastBaseCharacterFromText(content);
        lastCharacterDepth = currentCharacterDepth;
        processInlineBoxBoundary = false;
        continue;
      }

      size_t boundaryDepth = std::min(currentCharacterDepth, lastCharacterDepth);
      size_t inlineBoxStartOnBoundaryIndex =
        inlineBoxStartIndexesOnInlineItemsList.size() - 1 - (currentCharacterDepth - boundaryDepth);
      size_t boundaryIndex = inlineBoxStartIndexesOnInlineItemsList[inlineBoxStartOnBoundaryIndex];
      const RenderStyle &boundaryOwnerStyle = inlineItemList[boundaryIndex].LayoutBox().Parent().Style();
      auto boundaryTextAutospace = boundaryOwnerStyle.GetTextAutospace();
      if (!boundaryTextAutospace.IsNoAutospace() && !content.empty()
          && boundaryTextAutospace.ShouldApplySpacing(CodePointAt(content, 0), lastCharacterFromPreviousRun))
      {
        spacings.try_emplace(boundaryIndex, TextAutospace::TextAutospaceSize(
                                              *boundaryOwnerStyle.GetFontCascade().PrimaryFont()));
      }

      lastCharacterFromPreviousRun = TextUtil::LastBaseCharacterFromText(content);
      lastCharacterDepth = currentCharacterDepth;
      processInlineBoxBoundary = false;
    }
    if (!spacings.empty())
    {
      GetInlineContentCache().SetInlineBoxBoundaryTextSpacings(krys::move(spacings));
    }
  }

  InlineItemsBuilder::LayoutQueue
    InlineItemsBuilder::TraverseUntilDamaged(const Box &firstDamagedLayoutBox) noexcept
  {
    LayoutQueue queue;

    auto appendAndCheckForDamage = [&](const Box &layoutBox)
    {
      queue.push_back(&layoutBox);
      _contentRequiresVisualReordering =
        _contentRequiresVisualReordering || RequiresVisualReordering(layoutBox);
      return &layoutBox == &firstDamagedLayoutBox;
    };

    if (appendAndCheckForDamage(*Root().FirstChild()))
    {
      return queue;
    }

    while (!queue.empty())
    {
      while (true)
      {
        auto *layoutBox = queue.back();
        auto isInlineBoxWithInlineContent = layoutBox->IsInlineBox() && !layoutBox->IsInlineTextBox()
                                            && !layoutBox->IsLineBreakBox()
                                            && !layoutBox->IsOutOfFlowPositioned();
        if (!isInlineBoxWithInlineContent)
        {
          break;
        }
        auto *firstChild = Downcast<ElementBox>(*layoutBox).FirstChild();
        if (!firstChild)
        {
          break;
        }
        if (appendAndCheckForDamage(*firstChild))
        {
          return queue;
        }
      }

      while (!queue.empty())
      {
        auto *last = queue.pop_back_val();
        if (auto *nextSibling = last->NextSibling())
        {
          if (appendAndCheckForDamage(*nextSibling))
          {
            return queue;
          }
          break;
        }
      }
    }
    // How did we miss the damaged box?
    krys_debug_assert(false);
    queue.push_back(Root().FirstChild());
    return queue;
  }

  InlineItemsBuilder::LayoutQueue
    InlineItemsBuilder::InitializeLayoutQueue(InlineItemPosition startPosition) noexcept
  {
    auto &root = Root();
    if (!root.FirstChild())
    {
      // There should always be at least one inflow child in this inline formatting context.
      krys_debug_assert(false);
      return {};
    }

    if (!startPosition)
    {
      return {root.FirstChild()};
    }

    // For partial layout we need to build the layout queue up to the point where the new content is in order
    // to be able to produce non-content type of trailing inline items.
    // e.g <div><span<span>text</span></span> produces
    // [inline box start][inline box start][text][inline box end][inline box end]
    // and inserting new content after text
    // <div><span><span>text more_text</span></span> should produce
    // [inline box start][inline box start][text][ ][more_text][inline box end][inline box end]
    // where we start processing the content at the new layout box and continue with whatever we have on the
    // stack (layout queue).
    auto &existingInlineItems = GetInlineContentCache().GetInlineItems().Content();
    if (startPosition.Index >= existingInlineItems.size())
    {
      krys_debug_assert(false);
      return {root.FirstChild()};
    }

    auto &firstDamagedLayoutBox = existingInlineItems[startPosition.Index].LayoutBox();
    return TraverseUntilDamaged(firstDamagedLayoutBox);
  }

  void InlineItemsBuilder::CollectInlineItems(InlineItemList &inlineItemList,
                                              InlineItemPosition startPosition) noexcept
  {
    // Traverse the tree and create inline items out of inline boxes and leaf nodes. This essentially turns
    // the tree inline structure into a flat one. <span>text<span></span><img></span> ->
    // [InlineBoxStart][InlineLevelBox][InlineBoxStart][InlineBoxEnd][InlineLevelBox][InlineBoxEnd]
    auto layoutQueue = InitializeLayoutQueue(startPosition);
    auto &inlineItemCache = GetInlineContentCache().GetInlineItems();

    auto partialContentOffset = [&](const InlineTextBox &inlineTextBox) -> Maybe<size_t>
    {
      if (!startPosition)
      {
        return {};
      }
      auto &currentInlineItems = inlineItemCache.Content();
      if (startPosition.Index >= currentInlineItems.size())
      {
        krys_debug_assert(false);
        return {};
      }
      auto &damagedInlineItem = currentInlineItems[startPosition.Index];
      if (&inlineTextBox != &damagedInlineItem.LayoutBox())
      {
        return {};
      }
      if (auto *inlineTextItem = DynamicDowncast<InlineTextItem>(damagedInlineItem))
      {
        return inlineTextItem->Start();
      }
      if (auto *inlineSoftLineBreakItem = DynamicDowncast<InlineSoftLineBreakItem>(damagedInlineItem))
      {
        return inlineSoftLineBreakItem->Position();
      }
      krys_debug_assert(false);
      return {};
    };

    while (!layoutQueue.empty())
    {
      while (true)
      {
        auto *layoutBox = layoutQueue.back();
        auto isInlineBoxWithInlineContent = layoutBox->IsInlineBox() && !layoutBox->IsInlineTextBox()
                                            && !layoutBox->IsLineBreakBox()
                                            && !layoutBox->IsOutOfFlowPositioned();
        if (!isInlineBoxWithInlineContent)
        {
          break;
        }
        // This is the start of an inline box (e.g. <span>).
        HandleInlineBoxStart(*layoutBox, inlineItemList);
        auto &inlineBox = Downcast<ElementBox>(*layoutBox);
        if (!inlineBox.HasChild())
        {
          break;
        }
        layoutQueue.push_back(inlineBox.FirstChild());
      }

      while (!layoutQueue.empty())
      {
        auto *layoutBox = layoutQueue.pop_back_val();
        if (layoutBox->IsOutOfFlowPositioned())
        {
          inlineItemList.push_back(InlineItem(*layoutBox, InlineItem::Type::Opaque));
        }
        else if (auto *inlineTextBox = DynamicDowncast<InlineTextBox>(*layoutBox))
        {
          HandleTextContent(*inlineTextBox, inlineItemList, partialContentOffset(*inlineTextBox));
        }
        else if (layoutBox->IsAtomicInlineBox() || layoutBox->IsLineBreakBox())
        {
          HandleInlineLevelBox(*layoutBox, inlineItemList);
        }
        else if (layoutBox->IsInlineBox())
        {
          HandleInlineBoxEnd(*layoutBox, inlineItemList);
        }
        else if (layoutBox->IsFloatingPositioned())
        {
          inlineItemList.push_back(InlineItem(*layoutBox, InlineItem::Type::Float));
        }
        else if (layoutBox->IsBlockLevelBox())
        {
          inlineItemList.push_back(InlineItem(*layoutBox, InlineItem::Type::Block));
        }
        else
        {
          krys_debug_assert(false);
        }

        if (auto *nextSibling = layoutBox->NextSibling())
        {
          layoutQueue.push_back(nextSibling);
          break;
        }
      }
    }
  }

  void InlineItemsBuilder::BreakAndComputeBidiLevels(InlineItemList &inlineItemList) noexcept
  {
    krys_debug_assert(!inlineItemList.empty());

    StringBuilder paragraphContentBuilder;
    InlineItemOffsetList inlineItemOffsets;
    inlineItemOffsets.reserve(inlineItemList.size());
    BuildBidiParagraph(Root().Style(), inlineItemList, paragraphContentBuilder, inlineItemOffsets);
    if (paragraphContentBuilder.IsEmpty())
    {
      // Style may trigger visual reordering even on a completely empty content.
      // e.g. <div><span style="direction:rtl"></span></div>
      // Let's not try to do bidi handling when there's no content to reorder.
      return;
    }
    auto mayNotUseBlockDirection = Root().Style().GetUnicodeBidi() == UnicodeBidi::Plaintext;
    if (!ContentRequiresVisualReordering() && mayNotUseBlockDirection
        && TextUtil::DirectionForTextContent(paragraphContentBuilder.View()) == TextDirection::LTR)
    {
      // UnicodeBidi::Plaintext makes directionality calculated without taking parent direction property into
      // account.
      return;
    }
    krys_debug_assert(inlineItemOffsets.size() == inlineItemList.size());
    // 1. Setup the bidi boundary loop by calling ubidi_setPara with the paragraph text.
    // 2. Call ubidi_getLogicalRun to advance to the next bidi boundary until we hit the end of the content.
    // 3. Set the computed bidi level on the associated inline items. Split them as needed.
    UBiDi *ubidi = ubidi_open();

    struct CloseUBiDiOnExit
    {
      UBiDi *UBiDi;
      ~CloseUBiDiOnExit() noexcept
      {
        ubidi_close(UBiDi);
      }
    } closeUBiDiOnExit {ubidi};

    UBiDiLevel rootBidiLevel = UBIDI_DEFAULT_LTR;
    bool useHeuristicBaseDirection = Root().Style().GetUnicodeBidi() == UnicodeBidi::Plaintext;
    if (!useHeuristicBaseDirection)
    {
      rootBidiLevel = static_cast<UBiDiLevel>(Root().GetWritingMode().IsBidiLTR() ? UBIDI_LTR : UBIDI_RTL);
    }

    // ICU's bidi algorithm takes UTF-16: the runs it finds are mapped back to byte offsets.
    auto bidiContent = UTF16Paragraph(paragraphContentBuilder.View());
    auto bidiContentLength = static_cast<int32_t>(bidiContent.Characters.size());
    UErrorCode error = U_ZERO_ERROR;
    krys_debug_assert(bidiContentLength);
    ubidi_setPara(ubidi, bidiContent.Characters.data(), bidiContentLength, rootBidiLevel, nullptr, &error);

    if (U_FAILURE(error))
    {
      krys_debug_assert(false);
      return;
    }

    size_t inlineItemIndex = 0;
    auto hasSeenOpaqueItem = false;
    for (int32_t currentPosition = 0; currentPosition < bidiContentLength;)
    {
      UBiDiLevel bidiLevel;
      int32_t endPosition = currentPosition;
      ubidi_getLogicalRun(ubidi, currentPosition, &endPosition, &bidiLevel);

      auto setBidiLevelOnRange = [&](size_t bidiEnd, UBiDiLevel bidiLevelForRange)
      {
        // We should always have inline item(s) associated with a bidi range.
        krys_debug_assert(inlineItemIndex < inlineItemOffsets.size());
        // Start of the range is always where we left off (bidi ranges do not have gaps).
        for (; inlineItemIndex < inlineItemOffsets.size(); ++inlineItemIndex)
        {
          auto offset = inlineItemOffsets[inlineItemIndex];
          auto &inlineItem = inlineItemList[inlineItemIndex];
          if (!offset)
          {
            // This is an opaque item. Let's post-process it.
            hasSeenOpaqueItem = true;
            inlineItem.SetBidiLevel(bidiLevelForRange);
            continue;
          }
          if (*offset >= bidiEnd)
          {
            // This inline item is outside of the bidi range.
            break;
          }
          inlineItem.SetBidiLevel(bidiLevelForRange);
          auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem);
          if (!inlineTextItem)
          {
            continue;
          }
          // Check if this text item is on bidi boundary and needs splitting.
          auto itemEndPosition = *offset + inlineTextItem->Length();
          if (itemEndPosition > bidiEnd)
          {
            auto rightSide = inlineTextItem->Split(bidiEnd - *offset);
            inlineItemList.insert(inlineItemList.begin() + static_cast<ptrdiff_t>(inlineItemIndex + 1),
                                  rightSide);
            // Right side is going to be processed at the next bidi range.
            inlineItemOffsets.insert(inlineItemOffsets.begin() + static_cast<ptrdiff_t>(inlineItemIndex + 1),
                                     bidiEnd);
            ++inlineItemIndex;
            break;
          }
        }
      };
      setBidiLevelOnRange(bidiContent.ByteOffsets[static_cast<size_t>(endPosition)], bidiLevel);
      currentPosition = endPosition;
    }

    auto setBidiLevelForOpaqueInlineItems = [&]
    {
      if (!hasSeenOpaqueItem)
      {
        return;
      }
      // Let's not confuse ubidi with non-content entries.
      // Opaque runs are excluded from the visual list (ie. only empty inline boxes should be kept around as
      // bidi content -to figure out their visual order).
      enum class InlineBoxHasContent : bool
      {
        No,
        Yes
      };
      List<InlineBoxHasContent> inlineBoxContentFlagStack;
      inlineBoxContentFlagStack.reserve(inlineItemList.size());
      for (auto index = inlineItemList.size(); index--;)
      {
        auto &inlineItem = inlineItemList[index];
        auto &style = inlineItem.Style();
        auto initiatesControlCharacter =
          style.RtlOrdering() == Order::Logical && style.GetUnicodeBidi() != UnicodeBidi::Normal;

        if (inlineItem.IsInlineBoxStart())
        {
          krys_debug_assert(!inlineBoxContentFlagStack.empty());
          auto hasContent = inlineBoxContentFlagStack.back();
          inlineBoxContentFlagStack.pop_back();
          if (hasContent == InlineBoxHasContent::Yes)
          {
            if (!initiatesControlCharacter)
            {
              inlineItemList[index].SetBidiLevel(InlineItem::OpaqueBidiLevel);
            }
          }
          continue;
        }
        if (inlineItem.IsInlineBoxEnd())
        {
          inlineBoxContentFlagStack.push_back(InlineBoxHasContent::No);
          if (!initiatesControlCharacter)
          {
            inlineItem.SetBidiLevel(InlineItem::OpaqueBidiLevel);
          }
          continue;
        }
        if (inlineItem.IsWordBreakOpportunity())
        {
          inlineItem.SetBidiLevel(InlineItem::OpaqueBidiLevel);
          continue;
        }

        auto isContentfulInlineItem = [&]
        {
          if (auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem))
          {
            return !inlineTextItem->IsWhitespace()
                   || TextUtil::ShouldPreserveSpacesAndTabs(inlineTextItem->LayoutBox());
          }
          return inlineItem.IsAtomicInlineBox() || inlineItem.IsLineBreak()
                 || (inlineItem.IsOpaque() && inlineItem.LayoutBox().IsOutOfFlowPositioned());
        };
        if (isContentfulInlineItem())
        {
          // Mark the inline box stack with "content yes", when we come across a content type of inline item
          // so that we can mark the inline box as opaque and let the content drive visual ordering.
          std::fill(inlineBoxContentFlagStack.begin(), inlineBoxContentFlagStack.end(),
                    InlineBoxHasContent::Yes);
        }
      }
    };
    setBidiLevelForOpaqueInlineItems();
  }

  InlineContentCache::InlineItems::ContentAttributes
    InlineItemsBuilder::ComputeContentAttributesAndInlineTextItemWidths(
      InlineItemList &inlineItemList, InlineItemPosition damagePosition,
      const InlineItemList &damagedItemList) noexcept
  {
    if (inlineItemList.empty() && !damagePosition)
    {
      return {};
    }

    bool isTextAndForcedLineBreakOnlyContent = true;
    size_t inlineBoxCount = 0;

    auto computeContentAttributesUpToDamage = [&]
    {
      if (!damagePosition)
      {
        return;
      }
      // ContentAttributes::requiresVisualReordering is not handled here as we don't support partial layout
      // with bidi content.
      krys_debug_assert(damagePosition.Index < damagedItemList.size());
      for (size_t index = 0; index < damagePosition.Index && index < damagedItemList.size(); ++index)
      {
        auto &inlineItem = damagedItemList[index];
        if (inlineItem.IsText())
        {
          continue;
        }

        if (inlineItem.IsInlineBoxStart())
        {
          ++inlineBoxCount;
          continue;
        }

        if (!inlineItem.IsInlineBoxEnd())
        {
          isTextAndForcedLineBreakOnlyContent =
            isTextAndForcedLineBreakOnlyContent && IsTextOrLineBreak(inlineItem.LayoutBox());
        }
      }
    };
    computeContentAttributesUpToDamage();

    if (_hasTextAutospace)
    {
      ComputeInlineBoxBoundaryTextSpacings(inlineItemList);
    }

    text_spacing::SpacingState spacingState;
    TrimmableTextSpacings trimmableTextSpacings;
    auto &inlineBoxBoundaryTextSpacings = GetInlineContentCache().GetInlineBoxBoundaryTextSpacings();
    for (size_t inlineItemIndex = 0; inlineItemIndex < inlineItemList.size(); ++inlineItemIndex)
    {
      auto extraInlineTextSpacing = 0.f;
      auto &inlineItem = inlineItemList[inlineItemIndex];

      if (auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem))
      {
        auto needsMeasuring = inlineTextItem->Length() && !inlineTextItem->IsZeroWidthSpaceSeparator()
                              && CanCacheMeasuredWidthOnInlineTextItem(inlineTextItem->GetInlineTextBox(),
                                                                       inlineTextItem->IsWhitespace());
        if (needsMeasuring)
        {
          auto start = inlineTextItem->Start();
          if (inlineItemIndex)
          {
            // Box boudary text spacing is potentially registered for inline box start items which appear
            // logically before an inline text item
            auto potentialInlineBoxStartIndex = inlineItemIndex - 1;
            if (auto inlineBoxBoundaryTextSpacing =
                  inlineBoxBoundaryTextSpacings.find(potentialInlineBoxStartIndex);
                inlineBoxBoundaryTextSpacing != inlineBoxBoundaryTextSpacings.end())
            {
              extraInlineTextSpacing = inlineBoxBoundaryTextSpacing->second;
            }
          }
          inlineTextItem->SetWidth(TextUtil::Width(*inlineTextItem, inlineTextItem->Style().GetFontCascade(),
                                                   start, start + inlineTextItem->Length(), {},
                                                   TextUtil::UseTrailingWhitespaceMeasuringOptimization::Yes,
                                                   spacingState)
                                   + extraInlineTextSpacing);
          HandleTextSpacing(spacingState, trimmableTextSpacings, *inlineTextItem, inlineItemIndex);
        }
        continue;
      }
      spacingState.LastCharacterClassFromPreviousRun = text_spacing::CharacterClass::Undefined;

      if (inlineItem.IsInlineBoxStart())
      {
        ++inlineBoxCount;
        continue;
      }

      if (!inlineItem.IsInlineBoxEnd())
      {
        isTextAndForcedLineBreakOnlyContent =
          isTextAndForcedLineBreakOnlyContent && IsTextOrLineBreak(inlineItem.LayoutBox());
      }
    }
    GetInlineContentCache().SetTrimmableTextSpacings(krys::move(trimmableTextSpacings));

    return {_contentRequiresVisualReordering, isTextAndForcedLineBreakOnlyContent, _hasTextAutospace,
            inlineBoxCount};
  }

  bool InlineItemsBuilder::BuildInlineItemListForTextFromBreakingPositionsCache(
    const InlineTextBox &inlineTextBox, InlineItemList &inlineItemList) noexcept
  {
    auto text = utf8_stringview {inlineTextBox.Content()};
    auto *breakingPositions = TextBreakingPositionCache::Singleton().Get(
      {inlineTextBox.Content(), TextBreakingPositionContext {inlineTextBox.Style()}});
    if (!breakingPositions)
    {
      return false;
    }

    auto shouldPreserveNewline = TextUtil::ShouldPreserveNewline(inlineTextBox);
    auto shouldPreserveSpacesAndTabs = TextUtil::ShouldPreserveSpacesAndTabs(inlineTextBox);

    auto initialSize = inlineItemList.size();
    auto contentLength = text.length();
    krys_debug_assert(contentLength);

    inlineItemList.reserve(inlineItemList.size() + breakingPositions->size());
    size_t previousPosition = 0;
    for (auto endPosition : *breakingPositions)
    {
      auto startPosition = std::exchange(previousPosition, endPosition);
      if (endPosition > contentLength || startPosition >= endPosition)
      {
        krys_debug_assert(false);
        // Revert.
        inlineItemList.erase(inlineItemList.begin() + static_cast<ptrdiff_t>(initialSize),
                             inlineItemList.end());
        return false;
      }

      auto character = text[startPosition];
      auto start = static_cast<uint32>(startPosition);
      auto length = static_cast<uint32>(endPosition - startPosition);
      if (character == u8'\n' && shouldPreserveNewline)
      {
        inlineItemList.push_back(InlineSoftLineBreakItem::CreateSoftLineBreakItem(inlineTextBox, start));
        continue;
      }

      auto isWhitespaceCharacter = character == u8' ' || character == u8'\n' || character == u8'\t';
      if (isWhitespaceCharacter)
      {
        auto isWordSeparator = character != u8'\t' || !shouldPreserveSpacesAndTabs;
        inlineItemList.push_back(InlineTextItem::CreateWhitespaceItem(
          inlineTextBox, start, length, UBIDI_DEFAULT_LTR, isWordSeparator, {}));
        continue;
      }

      krys_debug_assert(endPosition);
      auto hasTrailingSoftHyphen = EndsWithSoftHyphen(text, endPosition);
      inlineItemList.push_back(InlineTextItem::CreateNonWhitespaceItem(
        inlineTextBox, start, length, UBIDI_DEFAULT_LTR, hasTrailingSoftHyphen, {}));
    }
    return true;
  }

  void InlineItemsBuilder::HandleTextContent(const InlineTextBox &inlineTextBox,
                                             InlineItemList &inlineItemList,
                                             Maybe<size_t> partialContentOffset) noexcept
  {
    auto text = utf8_stringview {inlineTextBox.Content()};
    auto contentLength = text.length();
    if (!contentLength)
    {
      return inlineItemList.push_back(InlineTextItem::CreateEmptyItem(inlineTextBox));
    }

    _contentRequiresVisualReordering =
      _contentRequiresVisualReordering || RequiresVisualReordering(inlineTextBox);

    if (inlineTextBox.IsCombined())
    {
      return inlineItemList.push_back(InlineTextItem::CreateNonWhitespaceItem(
        inlineTextBox, {}, static_cast<uint32>(contentLength), UBIDI_DEFAULT_LTR, false, {}));
    }

    if (!partialContentOffset
        && BuildInlineItemListForTextFromBreakingPositionsCache(inlineTextBox, inlineItemList))
    {
      if (!_textContentPopulatedFromCache)
      {
        _textContentPopulatedFromCache = true;
      }
      return;
    }

    _textContentPopulatedFromCache = false;
    auto &style = inlineTextBox.Style();
    auto shouldPreserveSpacesAndTabs = TextUtil::ShouldPreserveSpacesAndTabs(inlineTextBox);
    auto shouldPreserveNewline = TextUtil::ShouldPreserveNewline(inlineTextBox);
    auto lineBreakIteratorFactory = CachedLineBreakIteratorFactory {
      text, style.ComputedLocale().GetPlatform(), TextUtil::LineBreakIteratorMode(style.GetLineBreak()),
      TextUtil::GetContentAnalysis(style.GetWordBreak())};
    auto currentPosition = partialContentOffset.value_or(0);
    krys_debug_assert(currentPosition <= contentLength);

    auto handleSegmentBreak = [&]
    {
      // Segment breaks with preserve new line style (white-space: pre, pre-wrap, break-spaces and pre-line)
      // compute to forced line break.
      auto isSegmentBreakCandidate = text[currentPosition] == u8'\n';
      if (!isSegmentBreakCandidate || !shouldPreserveNewline)
      {
        return false;
      }
      inlineItemList.push_back(InlineSoftLineBreakItem::CreateSoftLineBreakItem(
        inlineTextBox, static_cast<uint32>(currentPosition++)));
      return true;
    };
    auto handleWhitespace = [&]
    {
      auto stopAtWordSeparatorBoundary = shouldPreserveSpacesAndTabs && style.GetFontCascade().WordSpacing();
      auto whitespaceContent =
        MoveToNextNonWhitespacePosition(text, currentPosition, shouldPreserveNewline,
                                        shouldPreserveSpacesAndTabs, stopAtWordSeparatorBoundary);
      if (!whitespaceContent)
      {
        return false;
      }

      krys_debug_assert(whitespaceContent->Length);
      if (style.GetWhiteSpaceCollapse() == WhiteSpaceCollapse::BreakSpaces)
      {
        // https://www.w3.org/TR/css-text-3/#white-space-phase-1
        // For break-spaces, a soft wrap opportunity exists after every space and every tab.
        // FIXME: if this turns out to be a perf hit with too many individual whitespace inline items, we
        // should transition this logic to line breaking.
        for (size_t offset = 0; offset < whitespaceContent->Length; ++offset)
        {
          inlineItemList.push_back(InlineTextItem::CreateWhitespaceItem(
            inlineTextBox, static_cast<uint32>(currentPosition + offset), 1, UBIDI_DEFAULT_LTR,
            whitespaceContent->IsWordSeparator, {}));
        }
      }
      else
      {
        inlineItemList.push_back(InlineTextItem::CreateWhitespaceItem(
          inlineTextBox, static_cast<uint32>(currentPosition), static_cast<uint32>(whitespaceContent->Length),
          UBIDI_DEFAULT_LTR, whitespaceContent->IsWordSeparator, {}));
      }
      currentPosition += whitespaceContent->Length;
      return true;
    };
    auto handleNonWhitespace = [&]
    {
      auto startPosition = static_cast<uint32>(currentPosition);
      auto endPosition = startPosition;
      auto hasTrailingSoftHyphen = false;
      if (style.GetHyphens() == Hyphens::None)
      {
        // Let's merge candidate InlineTextItems separated by soft hyphen when the style says so.
        do
        {
          endPosition += MoveToNextBreakablePosition(endPosition, lineBreakIteratorFactory, style);
          krys_debug_assert(startPosition < endPosition);
        } while (endPosition < contentLength && EndsWithSoftHyphen(text, endPosition));
      }
      else
      {
        endPosition += MoveToNextBreakablePosition(startPosition, lineBreakIteratorFactory, style);
        krys_debug_assert(startPosition < endPosition);
        hasTrailingSoftHyphen = EndsWithSoftHyphen(text, endPosition);
      }
      krys_debug_assert(style.GetHyphens() != Hyphens::None || !hasTrailingSoftHyphen);
      inlineItemList.push_back(
        InlineTextItem::CreateNonWhitespaceItem(inlineTextBox, startPosition, endPosition - startPosition,
                                                UBIDI_DEFAULT_LTR, hasTrailingSoftHyphen, {}));
      currentPosition = endPosition;
      return true;
    };
    while (currentPosition < contentLength)
    {
      if (handleSegmentBreak())
      {
        continue;
      }
      if (handleWhitespace())
      {
        continue;
      }
      if (handleNonWhitespace())
      {
        continue;
      }
      // Unsupported content?
      krys_debug_assert(false);
    }
  }

  void InlineItemsBuilder::HandleInlineBoxStart(const Box &inlineBox, InlineItemList &inlineItemList) noexcept
  {
    inlineItemList.push_back(InlineItem(inlineBox, InlineItem::Type::InlineBoxStart));
    _contentRequiresVisualReordering |= RequiresVisualReordering(inlineBox);
    _hasTextAutospace |= !inlineBox.Style().GetTextAutospace().IsNoAutospace();
  }

  void InlineItemsBuilder::HandleInlineBoxEnd(const Box &inlineBox, InlineItemList &inlineItemList) noexcept
  {
    inlineItemList.push_back(InlineItem(inlineBox, InlineItem::Type::InlineBoxEnd));
    // Inline box end item itself can not trigger bidi content.
    krys_debug_assert(ContentRequiresVisualReordering() || inlineBox.GetWritingMode().IsBidiLTR()
                      || inlineBox.Style().RtlOrdering() == Order::Visual
                      || inlineBox.Style().GetUnicodeBidi() == UnicodeBidi::Normal);
  }

  void InlineItemsBuilder::HandleInlineLevelBox(const Box &layoutBox, InlineItemList &inlineItemList) noexcept
  {
    if (layoutBox.IsRubyAnnotationBox())
    {
      return inlineItemList.push_back(InlineItem(layoutBox, InlineItem::Type::Opaque));
    }

    if (layoutBox.IsAtomicInlineBox())
    {
      return inlineItemList.push_back(InlineItem(layoutBox, InlineItem::Type::AtomicInlineBox));
    }

    if (layoutBox.IsLineBreakBox())
    {
      return inlineItemList.push_back(InlineItem(layoutBox, layoutBox.IsWordBreakOpportunity()
                                                              ? InlineItem::Type::WordBreakOpportunity
                                                              : InlineItem::Type::HardLineBreak));
    }

    krys_debug_assert(false);
  }

  void InlineItemsBuilder::PopulateBreakingPositionCache(const InlineItemList &inlineItemList) noexcept
  {
    if (inlineItemList.size() < TextBreakingPositionCache::MinimumRequiredContentBreaks)
    {
      return;
    }

    auto inlineTextBoxContentSpan =
      [](const InlineItemList &inlineItemList, size_t index, const InlineTextBox *inlineTextBox)
    {
      size_t length = 0;
      for (auto i = index; i < inlineItemList.size(); ++i)
      {
        if (&inlineItemList[i].LayoutBox() != inlineTextBox)
        {
          break;
        }
        ++length;
      }
      return Span<const InlineItem>(inlineItemList.data() + index, length);
    };

    // Preserve breaking positions across content mutation.
    auto &breakingPositionCache = TextBreakingPositionCache::Singleton();
    size_t index = 0;
    while (index < inlineItemList.size())
    {
      auto *inlineTextBox = DynamicDowncast<InlineTextBox>(inlineItemList[index].LayoutBox());
      if (!inlineTextBox)
      {
        ++index;
        continue;
      }

      auto span = inlineTextBoxContentSpan(inlineItemList, index, inlineTextBox);
      if (span.size() < TextBreakingPositionCache::MinimumRequiredContentBreaks)
      {
        // Inline text box content's span is too short.
        index += span.size();
        continue;
      }

      auto isInlineTextBoxEligibleForBreakingPositionCache =
        inlineTextBox->Content().length()
        >= TextBreakingPositionCache::MinimumRequiredTextLengthForContentBreakCache;
      if (!isInlineTextBoxEligibleForBreakingPositionCache)
      {
        // Text is too short.
        index += span.size();
        continue;
      }

      auto key = TextBreakingPositionCache::Key {inlineTextBox->Content(),
                                                 TextBreakingPositionContext {inlineTextBox->Style()}};
      if (breakingPositionCache.Get(key))
      {
        // Cache is already populated.
        index += span.size();
        continue;
      }

      TextBreakingPositionCache::List breakingPositionList;
      breakingPositionList.reserve(span.size());
      for (auto &inlineItem : span)
      {
        if (auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem))
        {
          breakingPositionList.push_back(inlineTextItem->End());
        }
        else if (auto *softLineBreakItem = DynamicDowncast<InlineSoftLineBreakItem>(inlineItem))
        {
          breakingPositionList.push_back(softLineBreakItem->Position() + 1);
        }
        else
        {
          krys_debug_assert(false);
          breakingPositionList.clear();
          break;
        }
      }

      krys_debug_assert(!breakingPositionList.empty());
      if (breakingPositionList.size() >= TextBreakingPositionCache::MinimumRequiredContentBreaks)
      {
        breakingPositionCache.Set(key, krys::move(breakingPositionList));
      }
      index += span.size();
    }
  }
}
