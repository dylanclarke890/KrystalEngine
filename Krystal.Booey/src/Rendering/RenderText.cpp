#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Core/TextBreakIterator.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
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
      // FIXME: The known-to-have-no-overflow-and-no-fallback-fonts state, once there's text measurement.
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

    // FIXME: The line layout's style and the accessibility cache, once there are any.

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
    SetTextInternal(krys::move(newContent), force);
    // FIXME: Invalidate the line layout on content changes, once there's line layout.
  }

  void RenderText::SetTextWithOffset(utf8_string newText, uint32) noexcept
  {
    if (_text == newText)
    {
      return;
    }

    SetTextInternal(krys::move(newText), false);
    // FIXME: Invalidate the line layout on content changes, once there's line layout.
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
}
