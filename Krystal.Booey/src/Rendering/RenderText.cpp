#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Core/Hyphenation.hpp"
#include "Krystal.Booey/Core/TextBreakIterator.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/BreakablePositions.hpp"
#include "Krystal.Booey/Rendering/Line/WordTrailingSpace.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Text/TabSize.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include <algorithm>
#include <array>
#include <bitset>
#include <string>
#include <unicode/ubrk.h>
#include <unicode/ucasemap.h>
#include <unicode/uchar.h>
#include <unicode/ustring.h>

namespace krys::boo
{
  namespace
  {
    /// @brief The byte offset where the code point at @p offset ends.
    KRYS_NODISCARD size_t CodePointEnd(utf8_stringview text, size_t offset) noexcept
    {
      return offset + text::DecodeUTF8(Span<const char8>(text.data() + offset, text.size() - offset)).Length;
    }

    /// @brief Appends @p codePoint, title-cased, to @p output.
    void AppendTitleCase(char32 codePoint, StringBuilder &output) noexcept
    {
      std::array<UChar, 2> source {};
      int32_t sourceLength = 0;
      U16_APPEND_UNSAFE(source.data(), sourceLength, codePoint);

      std::array<UChar, 8> destination {};
      UErrorCode status = U_ZERO_ERROR;
      auto length = u_strToTitle(destination.data(), static_cast<int32_t>(destination.size()), source.data(),
                                 sourceLength, nullptr, "", &status);
      if (U_FAILURE(status))
      {
        output.Append(codePoint);
        return;
      }
      for (int32_t i = 0; i < length;)
      {
        UChar32 titleCodePoint;
        U16_NEXT(destination.data(), i, length, titleCodePoint);
        output.Append(static_cast<char32>(titleCodePoint));
      }
    }

    // Capitalizes the code point at startOffset into output, and returns the number of bytes of textContent
    // it consumed (none if it's left as it is).
    KRYS_NODISCARD size_t CapitalizeCharacter(utf8_stringview textContent, size_t startOffset,
                                              StringBuilder &output) noexcept
    {
      if (startOffset >= textContent.size())
      {
        krys_debug_assert(false);
        return 0;
      }

      auto end = CodePointEnd(textContent, startOffset);
      auto character =
        text::DecodeUTF8(Span<const char8>(textContent.data() + startOffset, end - startOffset)).CodePoint;
      if (end - startOffset == 1)
      {
        if ((character >= 'A' && character <= 'Z') || character == ' ')
        {
          return 0;
        }
        if (character <= 'z')
        {
          output.Append(static_cast<char32>(u_totitle(static_cast<UChar32>(character))));
          return 1;
        }
      }

      AppendTitleCase(character, output);
      return end - startOffset;
    }

    KRYS_NODISCARD utf8_string ConvertCase(utf8_stringview text, const css::CSSOMStringAtom &locale,
                                           bool toUpper) noexcept
    {
      std::string localeName;
      for (auto character : locale.View())
      {
        localeName.push_back(static_cast<char>(character));
      }

      UErrorCode status = U_ZERO_ERROR;
      auto *caseMap = ucasemap_open(localeName.c_str(), 0, &status);
      if (U_FAILURE(status))
      {
        return utf8_string(text);
      }

      auto convert = [&](char *destination, int32_t capacity)
      {
        status = U_ZERO_ERROR;
        auto *source = reinterpret_cast<const char *>(text.data());
        auto sourceLength = static_cast<int32_t>(text.size());
        return toUpper ? ucasemap_utf8ToUpper(caseMap, destination, capacity, source, sourceLength, &status)
                       : ucasemap_utf8ToLower(caseMap, destination, capacity, source, sourceLength, &status);
      };

      auto length = convert(nullptr, 0);
      utf8_string result(static_cast<size_t>(length), u8'\0');
      if (status == U_BUFFER_OVERFLOW_ERROR || U_SUCCESS(status))
      {
        (void)convert(reinterpret_cast<char *>(result.data()), length);
      }
      ucasemap_close(caseMap);
      if (U_FAILURE(status))
      {
        return utf8_string(text);
      }
      return result;
    }

    // https://www.w3.org/TR/css-text-3/#small-kana
    KRYS_NODISCARD utf8_string ConvertToFullSizeKana(utf8_stringview string) noexcept
    {
      static constexpr auto kanasMap = std::to_array<std::pair<char32, char32>>({
        {0x3041, 0x3042},  {0x3043, 0x3044},  {0x3045, 0x3046},  {0x3047, 0x3048},  {0x3049, 0x304A},
        {0x3063, 0x3064},  {0x3083, 0x3084},  {0x3085, 0x3086},  {0x3087, 0x3088},  {0x308E, 0x308F},
        {0x3095, 0x304B},  {0x3096, 0x3051},  {0x30A1, 0x30A2},  {0x30A3, 0x30A4},  {0x30A5, 0x30A6},
        {0x30A7, 0x30A8},  {0x30A9, 0x30AA},  {0x30C3, 0x30C4},  {0x30E3, 0x30E4},  {0x30E5, 0x30E6},
        {0x30E7, 0x30E8},  {0x30EE, 0x30EF},  {0x30F5, 0x30AB},  {0x30F6, 0x30B1},  {0x31F0, 0x30AF},
        {0x31F1, 0x30B7},  {0x31F2, 0x30B9},  {0x31F3, 0x30C8},  {0x31F4, 0x30CC},  {0x31F5, 0x30CF},
        {0x31F6, 0x30D2},  {0x31F7, 0x30D5},  {0x31F8, 0x30D8},  {0x31F9, 0x30DB},  {0x31FA, 0x30E0},
        {0x31FB, 0x30E9},  {0x31FC, 0x30EA},  {0x31FD, 0x30EB},  {0x31FE, 0x30EC},  {0x31FF, 0x30ED},
        {0xFF67, 0xFF71},  {0xFF68, 0xFF72},  {0xFF69, 0xFF73},  {0xFF6A, 0xFF74},  {0xFF6B, 0xFF75},
        {0xFF6C, 0xFF94},  {0xFF6D, 0xFF95},  {0xFF6E, 0xFF96},  {0xFF6F, 0xFF82},  {0x1B132, 0x3053},
        {0x1B150, 0x3090}, {0x1B151, 0x3091}, {0x1B152, 0x3092}, {0x1B155, 0x30B3}, {0x1B164, 0x30F0},
        {0x1B165, 0x30F1}, {0x1B166, 0x30F2}, {0x1B167, 0x30F3},
      });

      auto find = [](char32 character) -> const std::pair<char32, char32> *
      {
        auto it = std::ranges::lower_bound(kanasMap, character, {}, &std::pair<char32, char32>::first);
        return it != kanasMap.end() && it->first == character ? &*it : nullptr;
      };

      bool needsConversion = false;
      for (text::CodePointIterator it(string); !it.AtEnd(); ++it)
      {
        if (find(*it))
        {
          needsConversion = true;
          break;
        }
      }
      if (!needsConversion)
      {
        return utf8_string(string);
      }

      StringBuilder result;
      for (text::CodePointIterator it(string); !it.AtEnd(); ++it)
      {
        if (auto *found = find(*it))
        {
          result.Append(found->second);
        }
        else
        {
          result.Append(*it);
        }
      }
      return krys::move(result).ToString();
    }

    KRYS_NODISCARD bool IsInlineFlowOrEmptyText(const RenderObject &renderer) noexcept
    {
      if (Is<RenderInline>(renderer))
      {
        return true;
      }
      auto *textRenderer = DynamicDowncast<RenderText>(renderer);
      return textRenderer && textRenderer->Text().empty();
    }
  }

  utf8_string Capitalize(utf8_stringview string, utf8_stringview previousCharacter) noexcept
  {
    // Replace NO BREAK SPACE with a normal spaces since ICU does not treat it as a word separator. It takes
    // two bytes in UTF-8, so it becomes two spaces, which keeps the offsets.
    auto replaceNoBreakSpaces = [](utf8_stringview text, utf8_string &output)
    {
      for (size_t i = 0; i < text.size(); ++i)
      {
        if (text[i] == 0xC2 && i + 1 < text.size() && text[i + 1] == 0xA0)
        {
          output += u8"  ";
          ++i;
          continue;
        }
        output.push_back(text[i]);
      }
    };
    utf8_string stringWithPrevious;
    replaceNoBreakSpaces(previousCharacter, stringWithPrevious);
    replaceNoBreakSpaces(string, stringWithPrevious);
    auto previousCharacterLength = static_cast<int32_t>(previousCharacter.size());

    auto *breakIterator = WordBreakIterator(stringWithPrevious);
    if (!breakIterator)
    {
      return utf8_string(string);
    }

    StringBuilder result;

    int32_t startOfWord = ubrk_first(breakIterator);
    for (int32_t endOfWord = ubrk_next(breakIterator); endOfWord != UBRK_DONE;
         startOfWord = endOfWord, endOfWord = ubrk_next(breakIterator))
    {
      // Do not try to titlecase the previous content.
      if (startOfWord >= previousCharacterLength)
      {
        auto capitalizedContentLength =
          CapitalizeCharacter(string, static_cast<size_t>(startOfWord - previousCharacterLength), result);
        auto from = static_cast<size_t>(startOfWord - previousCharacterLength) + capitalizedContentLength;
        auto to = static_cast<size_t>(endOfWord - previousCharacterLength);
        if (from < to)
        {
          result.Append(string.substr(from, to - from));
        }
      }
      else
      {
        // This is previous and continous non-titlecased current content. Only append current content.
        if (endOfWord > previousCharacterLength)
        {
          result.Append(string.substr(0, static_cast<size_t>(endOfWord - previousCharacterLength)));
        }
      }
    }
    return krys::move(result).ToString();
  }

  utf8_string ApplyTextTransform(const RenderStyle &style, utf8_string text,
                                 utf8_stringview previousCharacter) noexcept
  {
    auto transform = style.GetTextTransform();

    if (transform.IsNone())
    {
      return text;
    }

    // https://w3c.github.io/csswg-drafts/css-text/#text-transform-order
    auto modified = krys::move(text);
    if (transform.Contains(style::TextTransformValue::Capitalize))
    {
      modified = Capitalize(modified, previousCharacter); // FIXME: Need to take locale into account.
    }
    else if (transform.Contains(style::TextTransformValue::Uppercase))
    {
      modified = ConvertCase(modified, style.ComputedLocale().GetPlatform(), true);
    }
    else if (transform.Contains(style::TextTransformValue::Lowercase))
    {
      modified = ConvertCase(modified, style.ComputedLocale().GetPlatform(), false);
    }

    // FIXME: full-width (transformToFullWidth), which upstream only implements on Cocoa platforms.

    if (transform.Contains(style::TextTransformValue::FullSizeKana))
    {
      modified = ConvertToFullSizeKana(modified);
    }

    // FIXME: math-auto (the italic mathematical variant of single code points), once there are MathML
    // variants.

    return modified;
  }

  RenderText::RenderText(Type type, dom::Node &node, utf8_string text) noexcept
      : RenderObject(type, node, TypeFlag::IsText, {}), _text(krys::move(text)),
        _containsOnlyASCII(text::IsASCII(utf8_stringview {_text}))
  {
    ComputeFontCodePath();
    krys_debug_assert(IsRenderText());
  }

  RenderText::RenderText(Type type, dom::Text &textNode, utf8_string text) noexcept
      : RenderText(type, static_cast<dom::Node &>(textNode), krys::move(text))
  {
  }

  RenderText::RenderText(Type type, dom::Document &document, utf8_string text) noexcept
      : RenderText(type, static_cast<dom::Node &>(document), krys::move(text))
  {
  }

  RenderText::~RenderText() noexcept = default;

  utf8_stringview RenderText::RenderName() const noexcept
  {
    return u8"RenderText";
  }

  layout::InlineTextBox *RenderText::LayoutBox() noexcept
  {
    return Downcast<layout::InlineTextBox>(RenderObject::LayoutBox());
  }

  const layout::InlineTextBox *RenderText::LayoutBox() const noexcept
  {
    return Downcast<layout::InlineTextBox>(RenderObject::LayoutBox());
  }

  void RenderText::InitiateFontLoadingByAccessingGlyphDataAndComputeCanUseSimplifiedTextMeasuring(
    utf8_stringview textContent) noexcept
  {
    const auto &style = Style();
    const auto &fontCascade = style.GetFontCascade();
    // See webkit.org/b/252668
    auto fontVariant = AutoVariant;
    _canUseSimplifiedTextMeasuring = CanUseSimpleFontCodePath();

    auto fontVariantCaps = fontCascade.GetFontDescription().VariantCaps();
    if (fontVariantCaps == FontVariantCaps::Small || fontVariantCaps == FontVariantCaps::AllSmall
        || fontVariantCaps == FontVariantCaps::Petite || fontVariantCaps == FontVariantCaps::AllPetite)
    {
      // This matches the behavior of ComplexTextController::collectComplexTextRuns(): that function doesn't
      // perform font fallback on the capitalized characters when small caps is enabled, so we shouldn't here
      // either.
      fontVariant = NormalVariant;
      _canUseSimplifiedTextMeasuring = false;
    }

    auto whitespaceIsCollapsed = style.CollapseWhiteSpace();
    Ref<const Font> primaryFont = fontCascade.PrimaryFont();
    _canUseSimplifiedTextMeasuring =
      *_canUseSimplifiedTextMeasuring && !fontCascade.WordSpacing() && !fontCascade.LetterSpacing()
      && !primaryFont->SyntheticBoldOffset()
      && (&FirstLineStyle() == &style || &fontCascade == &FirstLineStyle().GetFontCascade());

    if (*_canUseSimplifiedTextMeasuring)
    {
      // Additional check on the font codepath.
      auto run = TextRun(textContent);
      run.SetCharacterScanForCodePath(false);
      _canUseSimplifiedTextMeasuring = fontCascade.GetCodePath(run) == FontCascade::CodePath::Simple;
    }

    _hasPositionDependentContentWidth = false;
    _hasStrongDirectionalityContent = false;
    auto mayHaveStrongDirectionalityContent = !text::IsASCII(textContent);
    // FIXME: Pre-warm glyph loading in FontCascade with the most common range.
    std::bitset<256> hasSeen;
    for (auto it = text::CodePointIterator(textContent); !it.AtEnd(); ++it)
    {
      auto character = *it;
      if (character < 256)
      {
        if (hasSeen.test(character))
        {
          continue;
        }
        hasSeen.set(character);
      }

      _canUseSimplifiedTextMeasuring = *_canUseSimplifiedTextMeasuring
                                       && fontCascade.CanUseSimplifiedTextMeasuring(
                                         character, fontVariant, whitespaceIsCollapsed, *primaryFont);
      _hasPositionDependentContentWidth =
        *_hasPositionDependentContentWidth || character == text::TabCharacter;
      _hasStrongDirectionalityContent = *_hasStrongDirectionalityContent
                                        || (mayHaveStrongDirectionalityContent
                                            && layout::TextUtil::IsStrongDirectionalityCharacter(character));
    }
  }

  void RenderText::ComputeFontCodePath() noexcept
  {
    if (_containsOnlyASCII)
    {
      _fontCodePath = FontCascade::CodePath::Simple;
      return;
    }
    _fontCodePath = FontCascade::CharacterRangeCodePath(_text);
  }

  dom::Text *RenderText::TextNode() const noexcept
  {
    return Downcast<dom::Text>(GetNode());
  }

  bool RenderText::ComputeUseBackslashAsYenSymbol() const noexcept
  {
    // FIXME: Fonts that use the yen sign for backslash, and the document's encoding (a decoder whose
    // encoding's currency symbol for backslash isn't backslash), once there are any.
    return false;
  }

  void RenderText::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    // There is no need to ever schedule repaints from a style change of a text run, since
    // we already did this for the parent of the text run.
    // We do have to schedule layouts, though, since a style change can force us to
    // need to relayout.
    if (diff == style::DifferenceResult::Layout)
    {
      SetNeedsLayoutAndPreferredWidthsUpdate();
      _knownToHaveNoOverflowAndNoFallbackFonts = false;
    }

    const auto &newStyle = Style();
    if (!oldStyle)
    {
      InitiateFontLoadingByAccessingGlyphDataAndComputeCanUseSimplifiedTextMeasuring(_text);
    }
    if (oldStyle && !oldStyle->FontCascadeEqual(newStyle))
    {
      _canUseSimplifiedTextMeasuring = null;
    }

    bool needsResetText = false;
    if (!oldStyle)
    {
      _useBackslashAsYenSymbol = ComputeUseBackslashAsYenSymbol();
      needsResetText = _useBackslashAsYenSymbol;
    }

    auto oldTransform =
      oldStyle ? oldStyle->GetTextTransform() : style::TextTransform {css::keywords::None {}};
    auto oldSecurity = oldStyle ? oldStyle->GetTextSecurity() : TextSecurity::None;
    if (needsResetText || oldTransform != newStyle.GetTextTransform()
        || oldSecurity != newStyle.GetTextSecurity())
    {
      RenderText::SetText(OriginalText(), true);
    }

    // FIXME: First line change on the block comes in as equal on text.
    auto needsLayoutBoxStyleUpdate =
      LayoutBox() && (diff >= style::DifferenceResult::RecompositeLayer || (&Style() != &FirstLineStyle()));
    if (needsLayoutBoxStyleUpdate)
    {
      layout_integration::LineLayout::UpdateStyle(*this);
    }

    // FIXME: The accessibility cache, once there is one.

    SetHorizontalWritingMode(newStyle.GetWritingMode().IsHorizontal());
  }

  void RenderText::WillBeDestroyed() noexcept
  {
    // FIXME: The secure text timer, once there's editing.

    SetInlineWrapperForDisplayContents(nullptr);

    RenderObject::WillBeDestroyed();
  }

  utf8_string RenderText::OriginalText() const noexcept
  {
    return _originalText ? *_originalText : _text;
  }

  bool RenderText::ContainsOnlyCollapsibleWhitespace() const noexcept
  {
    const auto &style = Style();
    return std::ranges::all_of(_text, [&](char8 character)
                               { return style.IsCollapsibleWhiteSpace(static_cast<char16_t>(character)); });
  }

  bool RenderText::ContainsOnlyCSSWhitespace(size_t from, size_t length) const noexcept
  {
    krys_debug_assert(from <= _text.size());
    krys_debug_assert(length <= _text.size());
    krys_debug_assert(from + length <= _text.size());
    return std::all_of(_text.begin() + from, _text.begin() + from + length, [](char8 character)
                       { return character == '\n' || character == ' ' || character == '\t'; });
  }

  utf8_string RenderText::PreviousCharacter() const noexcept
  {
    const RenderObject *previousText = this;
    while ((previousText = previousText->PreviousInPreOrder()))
    {
      if (!previousText->IsInFlow())
      {
        continue;
      }
      if (!IsInlineFlowOrEmptyText(*previousText))
      {
        break;
      }
    }
    auto *renderText = DynamicDowncast<RenderText>(previousText);
    if (!renderText)
    {
      return u8" ";
    }
    const auto &previousString = renderText->Text();
    auto start = previousString.size();
    while (start > 0)
    {
      --start;
      if ((previousString[start] & 0xC0) != 0x80)
      {
        break;
      }
    }
    return previousString.substr(start);
  }

  void RenderText::SetRenderedText(utf8_string newText) noexcept
  {
    auto originalText = OriginalText();

    _text = krys::move(newText);

    if (_useBackslashAsYenSymbol)
    {
      StringBuilder builder;
      for (auto character : _text)
      {
        if (character == u8'\\')
        {
          builder.Append(text::YenSign);
        }
        else
        {
          builder.Append(character);
        }
      }
      _text = krys::move(builder).ToString();
    }

    const auto &style = Style();
    if (!style.GetTextTransform().IsNone())
    {
      _text = ApplyTextTransform(style, krys::move(_text), PreviousCharacter());
    }

    // At rendering time, if certain fonts are used, these characters get swapped out with higher-quality PUA
    // characters. See RenderBlock::updateSecurityDiscCharacters().
    switch (style.GetTextSecurity())
    {
      case TextSecurity::None:   break;
      // We use the same characters here as for list markers.
      // See the listMarkerText function in RenderListMarker.cpp.
      case TextSecurity::Circle: SecureText(text::WhiteBullet); break;
      case TextSecurity::Disc:   SecureText(text::Bullet); break;
      case TextSecurity::Square: SecureText(text::BlackSquare); break;
    }

    _containsOnlyASCII = text::IsASCII(utf8_stringview {_text});
    ComputeFontCodePath();
    _canUseSimplifiedTextMeasuring = null;
    _hasPositionDependentContentWidth = null;
    _hasStrongDirectionalityContent = null;

    if (_text != originalText)
    {
      _originalText = krys::move(originalText);
    }
    else
    {
      _originalText = null;
    }
  }

  void RenderText::SecureText(char32 maskingCharacter) noexcept
  {
    // This hides the text by replacing all the characters with the masking character.
    // FIXME: Revealing the last typed character (the secure text timer), once there's editing.
    StringBuilder masked;
    for (text::CodePointIterator it(utf8_stringview {_text}); !it.AtEnd(); ++it)
    {
      masked.Append(maskingCharacter);
    }
    _text = krys::move(masked).ToString();
  }

  namespace
  {
    void InvalidateLineLayoutPathOnContentChangeIfNeeded(RenderText &renderer, Maybe<size_t> offset,
                                                         size_t oldLength) noexcept
    {
      auto *container = layout_integration::LineLayout::BlockContainer(renderer);
      if (!container)
      {
        return;
      }

      auto *inlineLayout = container->InlineLayout();
      if (!inlineLayout)
      {
        return;
      }

      if (layout_integration::LineLayout::ShouldInvalidateLineLayoutAfterContentChange(*container, renderer,
                                                                                       *inlineLayout))
      {
        container->InvalidateLineLayout(RenderBlockFlow::InvalidationReason::ContentChange);
        return;
      }
      if (!inlineLayout->UpdateTextContent(renderer, offset, oldLength))
      {
        container->InvalidateLineLayout(RenderBlockFlow::InvalidationReason::ContentChange);
      }
    }
  }

  void RenderText::SetTextInternal(utf8_string text, bool force) noexcept
  {
    if (!force && text == OriginalText())
    {
      return;
    }

    _text = text;
    _originalText = null;

    SetRenderedText(krys::move(text));

    SetNeedsLayoutAndPreferredWidthsUpdate();
    // FIXME: The accessibility cache, once there is one.
  }

  void RenderText::SetText(utf8_string newContent, bool force) noexcept
  {
    auto isDifferent = newContent != _text;
    SetTextInternal(krys::move(newContent), force);
    if (isDifferent || force)
    {
      InvalidateLineLayoutPathOnContentChangeIfNeeded(*this, {}, {});
    }
  }

  void RenderText::SetTextWithOffset(utf8_string newText, uint32 offset) noexcept
  {
    if (_text == newText)
    {
      return;
    }

    auto oldLength = _text.size();
    SetTextInternal(krys::move(newText), false);
    InvalidateLineLayoutPathOnContentChangeIfNeeded(*this, offset, oldLength);
  }

  utf8_string RenderText::TextWithoutConvertingBackslashToYenSymbol() const noexcept
  {
    if (!_useBackslashAsYenSymbol || Style().GetTextSecurity() != TextSecurity::None)
    {
      return _text;
    }

    if (Style().GetTextTransform().IsNone())
    {
      return OriginalText();
    }

    return ApplyTextTransform(Style(), OriginalText(), PreviousCharacter());
  }

  void RenderText::SetInlineWrapperForDisplayContents(RenderInline *wrapper) noexcept
  {
    _inlineWrapperForDisplayContents = wrapper;
  }

  namespace
  {
    struct DecodedCodePoint
    {
      char32 CodePoint {0};
      size_t Length {0};
    };

    KRYS_NODISCARD inline DecodedCodePoint DecodeCodePointAt(utf8_stringview text, size_t offset) noexcept
    {
      auto decoded = text::DecodeUTF8(Span<const char8>(text.data() + offset, text.size() - offset));
      return {decoded.CodePoint, static_cast<size_t>(decoded.Length)};
    }

    KRYS_NODISCARD inline char32 LastCodePoint(utf8_stringview text) noexcept
    {
      auto position = text.size() - 1;
      while (position && text::IsTrailingUTF8(text[position]))
      {
        --position;
      }
      return DecodeCodePointAt(text, position).CodePoint;
    }

    KRYS_NODISCARD inline bool IsHangablePunctuationAtLineStart(char32 character) noexcept
    {
      return U_GET_GC_MASK(static_cast<UChar32>(character)) & (U_GC_PS_MASK | U_GC_PI_MASK | U_GC_PF_MASK);
    }

    KRYS_NODISCARD inline bool IsHangablePunctuationAtLineEnd(char32 character) noexcept
    {
      return U_GET_GC_MASK(static_cast<UChar32>(character)) & (U_GC_PE_MASK | U_GC_PI_MASK | U_GC_PF_MASK);
    }

    // -webkit-nbsp-mode was dropped, so only a space is a space.
    KRYS_NODISCARD inline bool IsSpaceAccordingToStyle(char32 character) noexcept
    {
      return character == ' ';
    }

    KRYS_NODISCARD inline bool IsCollapsibleWhiteSpace(char32 character, const RenderStyle &style) noexcept
    {
      return character <= 0xFFFF && style.IsCollapsibleWhiteSpace(static_cast<char16_t>(character));
    }

    KRYS_NODISCARD inline float HyphenWidth(const RenderStyle &style, const FontCascade &font) noexcept
    {
      return font.Width(RenderBlock::ConstructTextRun(style.HyphenString().View(), style));
    }

    template <typename TMeasureTextCallback>
    KRYS_NODISCARD float
      MeasureTextConsideringPossibleTrailingSpace(bool currentCharacterIsSpace, size_t startIndex,
                                                  size_t wordLength, WordTrailingSpace &wordTrailingSpace,
                                                  HashSet<RefPtr<const Font>> &fallbackFonts,
                                                  TMeasureTextCallback &&callback) noexcept
    {
      Maybe<float> wordTrailingSpaceWidth;
      if (currentCharacterIsSpace)
      {
        wordTrailingSpaceWidth = wordTrailingSpace.Width(fallbackFonts);
      }
      if (wordTrailingSpaceWidth)
      {
        return callback(startIndex, wordLength + 1) - *wordTrailingSpaceWidth;
      }
      return callback(startIndex, wordLength);
    }
  }

  auto RenderText::LocalRectsForRepaint(RepaintOutlineBounds) const noexcept -> RepaintRects
  {
    LayoutRect overflowRect = LayoutRect(LinesBoundingBox());
    // FIXME: layoutDelta needs to be applied in parts before/after transforms and
    // repaint containers. https://bugs.webkit.org/show_bug.cgi?id=23308
    overflowRect.Move(View().FrameView().LayoutContext().LayoutDelta());

    return RepaintRects {overflowRect};
  }

  IntRect RenderText::LinesBoundingBox() const noexcept
  {
    auto firstTextBox = inline_iterator::LineLeftmostTextBoxFor(*this);
    if (!firstTextBox)
    {
      return {};
    }

    auto boundingBox = firstTextBox->VisualRectIgnoringBlockDirection();
    for (auto textBox = firstTextBox; ++textBox;)
    {
      boundingBox.UniteEvenIfEmpty(textBox->VisualRectIgnoringBlockDirection());
    }

    return EnclosingIntRect(boundingBox);
  }

  IntPoint RenderText::FirstRunLocation() const noexcept
  {
    auto first = inline_iterator::LineLeftmostTextBoxFor(*this);
    if (!first)
    {
      return {};
    }
    return IntPoint {first->VisualRectIgnoringBlockDirection().Location()};
  }

  float RenderText::WidthFromCache(const FontCascade &fontCascade, size_t start, size_t length, float xPos,
                                   HashSet<RefPtr<const Font>> *fallbackFonts, GlyphOverflow *glyphOverflow,
                                   const RenderStyle &style) const noexcept
  {
    // FIXME: The combined text's width (RenderCombineText::combinedTextWidth), once there's
    // text-combine-upright.
    auto run = RenderBlock::ConstructTextRun(*this, start, length, style);
    run.SetCharacterScanForCodePath(!CanUseSimpleFontCodePath());
    run.SetTabSize(!style.CollapseWhiteSpace(), style::ToPlatform<style::TabSize> {}(style.GetTabSize()));
    run.SetXPos(xPos);
    return fontCascade.Width(run, fallbackFonts, glyphOverflow);
  }

  float RenderText::WidthFromCacheConsideringPossibleTrailingSpace(
    const RenderStyle &style, const FontCascade &font, size_t startIndex, size_t wordLength, float xPos,
    bool currentCharacterIsSpace, WordTrailingSpace &wordTrailingSpace,
    HashSet<RefPtr<const Font>> &fallbackFonts, GlyphOverflow &glyphOverflow) const noexcept
  {
    return MeasureTextConsideringPossibleTrailingSpace(
      currentCharacterIsSpace, startIndex, wordLength, wordTrailingSpace, fallbackFonts,
      [&](size_t from, size_t length)
      { return WidthFromCache(font, from, length, xPos, &fallbackFonts, &glyphOverflow, style); });
  }

  float RenderText::HangablePunctuationStartWidth(size_t index) const noexcept
  {
    if (index >= _text.size())
    {
      return 0;
    }

    auto [character, characterLength] = DecodeCodePointAt(_text, index);
    if (!IsHangablePunctuationAtLineStart(character))
    {
      return 0;
    }

    auto &style = Style();
    return WidthFromCache(style.GetFontCascade(), index, characterLength, 0, nullptr, nullptr, style);
  }

  float RenderText::HangablePunctuationEndWidth(size_t index) const noexcept
  {
    if (index >= _text.size())
    {
      return 0;
    }

    auto [character, characterLength] = DecodeCodePointAt(_text, index);
    if (!IsHangablePunctuationAtLineEnd(character))
    {
      return 0;
    }

    auto &style = Style();
    return WidthFromCache(style.GetFontCascade(), index, characterLength, 0, nullptr, nullptr, style);
  }

  size_t RenderText::FirstCharacterIndexStrippingSpaces() const noexcept
  {
    if (!Style().CollapseWhiteSpace())
    {
      return 0;
    }

    size_t i = 0;
    for (auto length = _text.size(); i < length; ++i)
    {
      if (_text[i] != ' ' && (_text[i] != '\n' || Style().PreserveNewline()) && _text[i] != '\t')
      {
        break;
      }
    }
    return i;
  }

  size_t RenderText::LastCharacterIndexStrippingSpaces() const noexcept
  {
    if (_text.empty())
    {
      return 0;
    }

    if (!Style().CollapseWhiteSpace())
    {
      // The start of the last code point.
      auto position = _text.size() - 1;
      while (position && text::IsTrailingUTF8(_text[position]))
      {
        --position;
      }
      return position;
    }

    auto i = static_cast<ptrdiff_t>(_text.size()) - 1;
    for (; i >= 0; --i)
    {
      if (_text[i] != ' ' && (_text[i] != '\n' || Style().PreserveNewline()) && _text[i] != '\t')
      {
        break;
      }
    }
    // The start of that code point.
    while (i > 0 && text::IsTrailingUTF8(_text[i]))
    {
      --i;
    }
    return static_cast<size_t>(std::max<ptrdiff_t>(i, 0));
  }

  RenderText::Widths RenderText::TrimmedPreferredWidths(float leadWidth, bool &stripFrontSpaces) noexcept
  {
    auto &style = Style();
    bool collapseWhiteSpace = style.CollapseWhiteSpace();

    if (!collapseWhiteSpace)
    {
      stripFrontSpaces = false;
    }

    if (_hasTab || NeedsPreferredLogicalWidthsUpdate() || !_minWidth || !_maxWidth)
    {
      ComputePreferredLogicalWidths(leadWidth, !_minWidth || !_maxWidth);
    }

    Widths widths;

    widths.BeginWS = !stripFrontSpaces && _hasBeginWS;
    widths.EndWS = _hasEndWS;

    auto length = _text.size();

    if (!length
        || (stripFrontSpaces
            && std::ranges::all_of(_text,
                                   [](char8 character) { return text::IsASCIIWhitespace(character); })))
    {
      return widths;
    }

    widths.EndZeroSpace = LastCodePoint(_text) == text::ZeroWidthSpace;

    widths.Min = _minWidth.value_or(-1);
    widths.Max = _maxWidth.value_or(-1);

    widths.BeginMin = _beginMinWidth;
    widths.EndMin = _endMinWidth;

    widths.HasBreakableChar = _hasBreakableChar;
    widths.HasBreak = _hasBreak;
    widths.EndsWithBreak = _hasBreak && _text[length - 1] == '\n';

    if (_text[0] == ' ' || (_text[0] == '\n' && !style.PreserveNewline()) || _text[0] == '\t')
    {
      auto &font = style.GetFontCascade(); // FIXME: This ignores first-line.
      if (stripFrontSpaces)
      {
        widths.Max -= font.Width(RenderBlock::ConstructTextRun(u8" ", style));
      }
      else
      {
        widths.Max += font.WordSpacing();
      }
    }

    stripFrontSpaces = collapseWhiteSpace && _hasEndWS;

    if (!style.AutoWrap() || widths.Min > widths.Max)
    {
      widths.Min = widths.Max;
    }

    // Compute our max widths by scanning the string for newlines.
    if (widths.HasBreak)
    {
      auto &font = style.GetFontCascade(); // FIXME: This ignores first-line.
      bool firstLine = true;
      widths.BeginMax = widths.Max;
      widths.EndMax = widths.Max;
      for (size_t i = 0; i < length; i++)
      {
        size_t lineLength = 0;
        while (i + lineLength < length && _text[i + lineLength] != '\n')
        {
          lineLength++;
        }

        if (lineLength)
        {
          widths.EndMax =
            WidthFromCache(font, i, lineLength, leadWidth + widths.EndMax, nullptr, nullptr, style);
          if (firstLine)
          {
            firstLine = false;
            leadWidth = 0;
            widths.BeginMax = widths.EndMax;
          }
          i += lineLength;
        }
        else if (firstLine)
        {
          widths.BeginMax = 0;
          firstLine = false;
          leadWidth = 0;
        }

        if (i == length - 1)
        {
          // A <pre> run that ends with a newline, as in, e.g.,
          // <pre>Some text\n\n<span>More text</pre>
          widths.EndMax = 0;
        }
      }
    }

    return widths;
  }

  float RenderText::MinLogicalWidth() const noexcept
  {
    if (NeedsPreferredLogicalWidthsUpdate() || !_minWidth)
    {
      const_cast<RenderText &>(*this).ComputePreferredLogicalWidths(0, !NeedsPreferredLogicalWidthsUpdate());
    }

    return *_minWidth;
  }

  float RenderText::MaxLogicalWidth() const noexcept
  {
    if (NeedsPreferredLogicalWidthsUpdate() || !_maxWidth)
    {
      const_cast<RenderText &>(*this).ComputePreferredLogicalWidths(0, !NeedsPreferredLogicalWidthsUpdate());
    }

    return *_maxWidth;
  }

  void RenderText::ComputePreferredLogicalWidths(float leadWidth, bool forcedMinMaxWidthComputation) noexcept
  {
    HashSet<RefPtr<const Font>> fallbackFonts;
    GlyphOverflow glyphOverflow;
    ComputePreferredLogicalWidths(leadWidth, fallbackFonts, glyphOverflow, forcedMinMaxWidthComputation);
    if (fallbackFonts.empty() && glyphOverflow.IsEmpty())
    {
      _knownToHaveNoOverflowAndNoFallbackFonts = true;
    }
  }

  float RenderText::MaxWordFragmentWidth(const RenderStyle &style, const FontCascade &font,
                                         utf8_stringview word, size_t minimumPrefixLength,
                                         size_t minimumSuffixLength, bool currentCharacterIsSpace,
                                         size_t characterIndex, float xPos, float entireWordWidth,
                                         WordTrailingSpace &wordTrailingSpace,
                                         HashSet<RefPtr<const Font>> &fallbackFonts,
                                         GlyphOverflow &glyphOverflow) noexcept
  {
    size_t suffixStart = 0;
    if (word.size() <= minimumSuffixLength)
    {
      return entireWordWidth;
    }

    List<size_t> hyphenLocations;
    krys_debug_assert(word.size() >= minimumSuffixLength);
    auto hyphenLocation = word.size() - minimumSuffixLength;
    auto locale = style.ComputedLocale().GetPlatform();
    while ((hyphenLocation = LastHyphenLocation(word, hyphenLocation, locale))
           >= std::max<size_t>(minimumPrefixLength, 1))
    {
      hyphenLocations.push_back(hyphenLocation);
    }

    if (hyphenLocations.empty())
    {
      return entireWordWidth;
    }

    std::ranges::reverse(hyphenLocations);

    // Consider the word "ABC-DEF-GHI" (where the '-' characters are hyphenation opportunities). We want to
    // measure the width of "ABC-" and "DEF-", but not "GHI-". Instead, we should measure "GHI" the same way
    // we measure regular unhyphenated words (by using wordTrailingSpace). Therefore, this function is split
    // up into two parts - one that measures each prefix, and one that measures the single last suffix.

    // FIXME: Breaking the string at these places in the middle of words doesn't work with complex text.
    float minimumFragmentWidthToConsider = font.Size() * 5 / 4 + HyphenWidth(style, font);
    float maxFragmentWidth = 0;
    for (auto location : hyphenLocations)
    {
      auto fragmentLength = location - suffixStart;
      auto fragmentWithHyphen = utf8_string {word.substr(suffixStart, fragmentLength)};
      fragmentWithHyphen += style.HyphenString().View();

      auto run = RenderBlock::ConstructTextRun(fragmentWithHyphen, style);
      run.SetCharacterScanForCodePath(!CanUseSimpleFontCodePath());
      float fragmentWidth = font.Width(run, &fallbackFonts, &glyphOverflow);

      // Narrow prefixes are ignored.
      if (fragmentWidth <= minimumFragmentWidthToConsider)
      {
        continue;
      }

      suffixStart += fragmentLength;
      maxFragmentWidth = std::max(maxFragmentWidth, fragmentWidth);
    }

    if (!suffixStart)
    {
      // We didn't find any hyphenation opportunities that we're willing to actually use.
      // Therefore, the width of the maximum fragment is just ... the width of the entire word.
      return entireWordWidth;
    }

    auto suffixWidth = WidthFromCacheConsideringPossibleTrailingSpace(
      style, font, characterIndex + suffixStart, word.size() - suffixStart, xPos, currentCharacterIsSpace,
      wordTrailingSpace, fallbackFonts, glyphOverflow);
    return std::max(maxFragmentWidth, suffixWidth);
  }

  void RenderText::ComputePreferredLogicalWidths(float leadWidth, HashSet<RefPtr<const Font>> &fallbackFonts,
                                                 GlyphOverflow &glyphOverflow, bool) noexcept
  {
    _minWidth = 0.f;
    _beginMinWidth = 0;
    _endMinWidth = 0;
    _maxWidth = 0.f;

    float currMaxWidth = 0;
    _hasBreakableChar = false;
    _hasBreak = false;
    _hasTab = false;
    _hasBeginWS = false;
    _hasEndWS = false;

    auto &style = Style();
    auto &font = style.GetFontCascade(); // FIXME: This ignores first-line.
    float wordSpacing = font.WordSpacing();
    auto string = utf8_stringview {_text};
    auto length = string.size();
    auto iteratorMode = layout::TextUtil::LineBreakIteratorMode(style.GetLineBreak());
    auto contentAnalysis = layout::TextUtil::GetContentAnalysis(style.GetWordBreak());
    CachedLineBreakIteratorFactory lineBreakIteratorFactory(string, style.ComputedLocale().GetPlatform(),
                                                            iteratorMode, contentAnalysis);
    bool needsWordSpacing = false;
    bool ignoringSpaces = false;
    bool isSpace = false;
    bool firstWord = true;
    bool firstLine = true;
    Maybe<uint32> nextBreakable;
    size_t lastWordBoundary = 0;

    WordTrailingSpace wordTrailingSpace(style);
    // If automatic hyphenation is allowed, we keep track of the width of the widest word (or word
    // fragment) encountered so far, and only try hyphenating words that are wider.
    float maxWordWidth = std::numeric_limits<float>::max();
    size_t minimumPrefixLength = 0;
    size_t minimumSuffixLength = 0;
    if (style.GetHyphens() == Hyphens::Auto && CanHyphenate(style.ComputedLocale().GetPlatform()))
    {
      maxWordWidth = 0;

      // Map 'hyphenate-limit-{before,after}: auto;' to 2.
      auto before = style.HyphenateLimitBefore().TryValue();
      minimumPrefixLength = before ? static_cast<size_t>(before->Value) : 2;
      auto after = style.HyphenateLimitAfter().TryValue();
      minimumSuffixLength = after ? static_cast<size_t>(after->Value) : 2;
    }

    Maybe<LayoutUnit> firstGlyphLeftOverflow;

    // -webkit-nbsp-mode was dropped, so no-break spaces never break.
    bool breakNBSP = false;

    bool breakAnywhere = style.GetLineBreak() == LineBreak::Anywhere && style.AutoWrap();
    // Note the deliberate omission of word-wrap/overflow-wrap's break-word value from this breakAll check.
    // Those do not affect minimum preferred sizes. Note that break-word is a non-standard value for
    // word-break, but we support it as though it means break-all.
    bool breakAll =
      (style.GetWordBreak() == WordBreak::BreakAll || style.GetWordBreak() == WordBreak::BreakWord
       || style.GetOverflowWrap() == OverflowWrap::Anywhere)
      && style.AutoWrap();
    bool keepAllWords = style.GetWordBreak() == WordBreak::KeepAll;
    bool canUseLineBreakShortcut = iteratorMode == TextBreakLineBehavior::Default
                                   && contentAnalysis == TextBreakIterator::ContentAnalysis::Mechanical;

    size_t i = 0;
    while (i < length)
    {
      auto [c, characterLength] = DecodeCodePointAt(string, i);

      bool previousCharacterIsSpace = isSpace;

      bool isNewline = false;
      if (c == '\n')
      {
        if (style.PreserveNewline())
        {
          _hasBreak = true;
          isNewline = true;
          isSpace = false;
        }
        else
        {
          isSpace = true;
        }
      }
      else if (c == '\t')
      {
        if (!style.CollapseWhiteSpace())
        {
          _hasTab = true;
          isSpace = false;
        }
        else
        {
          isSpace = true;
        }
      }
      else
      {
        isSpace = c == ' ';
      }

      if ((isSpace || isNewline) && !i)
      {
        _hasBeginWS = true;
      }
      if ((isSpace || isNewline) && i + characterLength == length)
      {
        _hasEndWS = true;
      }

      ignoringSpaces |= style.CollapseWhiteSpace() && previousCharacterIsSpace && isSpace;
      ignoringSpaces &= isSpace;

      // Ignore spaces and soft hyphens
      if (ignoringSpaces)
      {
        krys_debug_assert(lastWordBoundary == i);
        lastWordBoundary += characterLength;
        i += characterLength;
        continue;
      }
      if (c == text::SoftHyphen && style.GetHyphens() != Hyphens::None)
      {
        krys_debug_assert(i >= lastWordBoundary);
        currMaxWidth += WidthFromCache(font, lastWordBoundary, i - lastWordBoundary, leadWidth + currMaxWidth,
                                       &fallbackFonts, &glyphOverflow, style);
        if (!firstGlyphLeftOverflow)
        {
          firstGlyphLeftOverflow = glyphOverflow.Left;
        }
        lastWordBoundary = i + characterLength;
        i += characterLength;
        continue;
      }

      bool hasBreak =
        breakAll
        || BreakablePositions::IsBreakable(lineBreakIteratorFactory, static_cast<uint32>(i), nextBreakable,
                                           breakNBSP, canUseLineBreakShortcut, keepAllWords, breakAnywhere);
      bool betweenWords = true;
      auto j = i;
      while (c != '\n' && !IsSpaceAccordingToStyle(c) && c != '\t' && c != text::ZeroWidthSpace
             && (c != text::SoftHyphen || style.GetHyphens() == Hyphens::None))
      {
        auto previousCharacter = c;
        j += DecodeCodePointAt(string, j).Length;
        if (j == length)
        {
          break;
        }
        c = DecodeCodePointAt(string, j).CodePoint;
        if (BreakablePositions::IsBreakable(lineBreakIteratorFactory, static_cast<uint32>(j), nextBreakable,
                                            breakNBSP, canUseLineBreakShortcut, keepAllWords, breakAnywhere)
            && previousCharacter != text::SoftHyphen)
        {
          break;
        }
        if (breakAll)
        {
          // FIXME: This code is ultra wrong.
          // The spec says "word-break: break-all: Any typographic letter units are treated as ID(“ideographic
          // characters”) for the purpose of line-breaking." The spec describes how a "typographic letter
          // unit" is a cluster, not a code point:
          // https://drafts.csswg.org/css-text-3/#typographic-character-unit
          betweenWords = false;
          break;
        }
      }

      auto wordLength = j - i;
      if (wordLength)
      {
        float currMinWidth = 0;
        bool isTrailingSpace = (j < length) && IsSpaceAccordingToStyle(c);
        float w = WidthFromCacheConsideringPossibleTrailingSpace(
          style, font, i, wordLength, leadWidth + currMaxWidth, isTrailingSpace, wordTrailingSpace,
          fallbackFonts, glyphOverflow);
        if (c == text::SoftHyphen && style.GetHyphens() != Hyphens::None)
        {
          currMinWidth = HyphenWidth(style, font);
        }

        if (w > maxWordWidth)
        {
          auto maxFragmentWidth = MaxWordFragmentWidth(
            style, font, string.substr(i, wordLength), minimumPrefixLength, minimumSuffixLength,
            isTrailingSpace, i, leadWidth + currMaxWidth, w, wordTrailingSpace, fallbackFonts, glyphOverflow);
          // This, when combined with "currMinWidth += w" below, has the effect of executing
          // "currMinWidth += maxFragmentWidth" instead.
          currMinWidth += maxFragmentWidth - w;
          maxWordWidth = std::max(maxWordWidth, maxFragmentWidth);
        }

        if (!firstGlyphLeftOverflow)
        {
          firstGlyphLeftOverflow = glyphOverflow.Left;
        }
        currMinWidth += w;
        if (betweenWords)
        {
          if (lastWordBoundary == i)
          {
            currMaxWidth += w;
          }
          else
          {
            krys_debug_assert(j >= lastWordBoundary);
            currMaxWidth += WidthFromCache(font, lastWordBoundary, j - lastWordBoundary,
                                           leadWidth + currMaxWidth, &fallbackFonts, &glyphOverflow, style);
          }
          lastWordBoundary = j;
        }

        bool isCollapsibleWhiteSpace = (j < length) && IsCollapsibleWhiteSpace(c, style);
        if (j < length && style.AutoWrap())
        {
          _hasBreakableChar = true;
        }

        // Add in wordSpacing to our currMaxWidth, but not if this is the last word on a line or the
        // last word in the run.
        if ((isTrailingSpace || isCollapsibleWhiteSpace) && !ContainsOnlyCSSWhitespace(j, length - j))
        {
          currMaxWidth += wordSpacing;
        }

        if (firstWord)
        {
          firstWord = false;
          // If the first character in the run is breakable, then we consider ourselves to have a beginning
          // minimum width of 0, since a break could occur right before our run starts, preventing us from
          // ever being appended to a previous text run when considering the total minimum width of the
          // containing block.
          if (hasBreak)
          {
            _hasBreakableChar = true;
          }
          _beginMinWidth = hasBreak ? 0 : currMinWidth;
        }
        _endMinWidth = currMinWidth;

        _minWidth = std::max(currMinWidth, *_minWidth);

        i = j;
      }
      else
      {
        // Nowrap can never be broken, so don't bother setting the
        // breakable character boolean. Pre can only be broken if we encounter a newline.
        if (style.AutoWrap() || isNewline)
        {
          _hasBreakableChar = true;
        }

        if (isNewline)
        {
          // Only set if preserveNewline was true and we saw a newline.
          if (firstLine)
          {
            firstLine = false;
            leadWidth = 0;
            if (!style.AutoWrap())
            {
              _beginMinWidth = currMaxWidth;
            }
          }

          if (currMaxWidth > *_maxWidth)
          {
            _maxWidth = currMaxWidth;
          }
          currMaxWidth = 0;
        }
        else
        {
          auto run = RenderBlock::ConstructTextRun(*this, i, characterLength, style);
          run.SetTabSize(!style.CollapseWhiteSpace(),
                         style::ToPlatform<style::TabSize> {}(style.GetTabSize()));
          run.SetXPos(leadWidth + currMaxWidth);

          currMaxWidth += font.Width(run, &fallbackFonts);
          glyphOverflow.Right = 0_lu;
          needsWordSpacing = isSpace && !previousCharacterIsSpace && i + characterLength == length;
        }
        krys_debug_assert(lastWordBoundary == i);
        lastWordBoundary += characterLength;
        i += characterLength;
      }
    }

    glyphOverflow.Left = firstGlyphLeftOverflow.value_or(glyphOverflow.Left);

    if ((needsWordSpacing && length > 1) || (ignoringSpaces && !firstWord))
    {
      currMaxWidth += wordSpacing;
    }

    _maxWidth = std::max(currMaxWidth, *_maxWidth);

    if (!style.AutoWrap())
    {
      _minWidth = _maxWidth;
    }

    if (style.GetWhiteSpaceCollapse() == WhiteSpaceCollapse::Preserve
        && style.GetTextWrapMode() == TextWrapMode::NoWrap)
    {
      if (firstLine)
      {
        _beginMinWidth = *_maxWidth;
      }
      _endMinWidth = currMaxWidth;
    }

    ClearNeedsPreferredWidthsUpdate();
  }

  float RenderText::Width(size_t from, size_t length, float xPos, bool firstLine,
                          HashSet<RefPtr<const Font>> *fallbackFonts,
                          GlyphOverflow *glyphOverflow) const noexcept
  {
    if (from >= _text.size())
    {
      return 0;
    }

    if (from + length > _text.size())
    {
      length = _text.size() - from;
    }

    auto &lineStyle = firstLine ? FirstLineStyle() : Style();
    return Width(from, length, lineStyle.GetFontCascade(), xPos, fallbackFonts, glyphOverflow);
  }

  float RenderText::Width(size_t from, size_t length, const FontCascade &fontCascade, float xPos,
                          HashSet<RefPtr<const Font>> *fallbackFonts,
                          GlyphOverflow *glyphOverflow) const noexcept
  {
    krys_debug_assert(from + length <= _text.size());
    if (_text.empty() || !length)
    {
      return 0.f;
    }

    auto &style = Style();
    // FIXME: The combined text's width (RenderCombineText::combinedTextWidth), once there's
    // text-combine-upright.

    if (length == 1 && _text[from] == ' ')
    {
      return fontCascade.WidthOfSpaceString();
    }

    float width = 0.f;
    if (&fontCascade == &style.GetFontCascade())
    {
      if (!style.PreserveNewline() && !from && length == _text.size()
          && (!glyphOverflow || !glyphOverflow->ComputeBounds))
      {
        if (fallbackFonts)
        {
          krys_debug_assert(glyphOverflow);
          if (NeedsPreferredLogicalWidthsUpdate() || !_knownToHaveNoOverflowAndNoFallbackFonts)
          {
            const_cast<RenderText &>(*this).ComputePreferredLogicalWidths(0, *fallbackFonts, *glyphOverflow);
            if (fallbackFonts->empty() && glyphOverflow->IsEmpty())
            {
              _knownToHaveNoOverflowAndNoFallbackFonts = true;
            }
          }
          // The rare case of when we switch between IFC and legacy preferred width computation.
          width = _maxWidth ? *_maxWidth : MaxLogicalWidth();
        }
        else
        {
          width = MaxLogicalWidth();
        }
      }
      else
      {
        width = WidthFromCache(fontCascade, from, length, xPos, fallbackFonts, glyphOverflow, style);
      }
    }
    else
    {
      auto run = RenderBlock::ConstructTextRun(*this, from, length, style);
      run.SetCharacterScanForCodePath(!CanUseSimpleFontCodePath());
      run.SetTabSize(!style.CollapseWhiteSpace(), style::ToPlatform<style::TabSize> {}(style.GetTabSize()));
      run.SetXPos(xPos);

      width = fontCascade.Width(run, fallbackFonts, glyphOverflow);
    }

    return std::max(width, 0.f);
  }

  void RenderText::BoundingRects(List<LayoutRect> &rects, const LayoutPoint &accumulatedOffset) const noexcept
  {
    for (auto &run : inline_iterator::TextBoxesFor(*this))
    {
      auto rect = LayoutRect {run.VisualRectIgnoringBlockDirection()};
      rect.MoveBy(accumulatedOffset);
      rects.push_back(rect);
    }
  }

  void RenderText::AbsoluteQuads(List<FloatQuad> &quads, bool *wasFixed) const noexcept
  {
    for (auto &textBox : inline_iterator::TextBoxesFor(*this))
    {
      // FIXME: SVG text boxes' transforms (calculateBoundariesIncludingSVGTransform), once there's SVG.
      quads.push_back(LocalToAbsoluteQuad(textBox.VisualRectIgnoringBlockDirection(),
                                          MapCoordinatesMode::UseTransforms, wasFixed));
    }
  }
}
