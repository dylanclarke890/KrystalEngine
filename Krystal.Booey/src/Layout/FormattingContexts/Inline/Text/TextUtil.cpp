#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include <unicode/uchar.h>

namespace krys::boo::layout
{
  namespace
  {
    KRYS_NODISCARD bool MayBeBidiRTL(char32 ch) noexcept
    {
      if (ch < 0x0590)
      {
        return false;
      }
      // General Punctuation such as curly quotes.
      if (ch >= 0x2010 && ch <= 0x2029)
      {
        return false;
      }
      // CJK etc., up to Surrogate Pairs.
      if (ch >= 0x206A && ch <= 0xD7FF)
      {
        return false;
      }
      // Common in CJK.
      if (ch >= 0xFF00 && ch <= 0xFFFF)
      {
        return false;
      }
      return true;
    }
  }

  bool TextUtil::IsStrongDirectionalityCharacter(char32 character) noexcept
  {
    if (!MayBeBidiRTL(character))
    {
      return false;
    }

    auto bidiCategory = u_charDirection(static_cast<UChar32>(character));
    return bidiCategory == U_RIGHT_TO_LEFT || bidiCategory == U_RIGHT_TO_LEFT_ARABIC
           || bidiCategory == U_RIGHT_TO_LEFT_EMBEDDING || bidiCategory == U_RIGHT_TO_LEFT_OVERRIDE
           || bidiCategory == U_LEFT_TO_RIGHT_EMBEDDING || bidiCategory == U_LEFT_TO_RIGHT_OVERRIDE
           || bidiCategory == U_POP_DIRECTIONAL_FORMAT;
  }

  bool TextUtil::ContainsStrongDirectionalityText(utf8_stringview text) noexcept
  {
    for (auto it = text::CodePointIterator(text); !it.AtEnd(); ++it)
    {
      if (IsStrongDirectionalityCharacter(*it))
      {
        return true;
      }
    }

    return false;
  }

  bool TextUtil::CanUseSimplifiedTextMeasuring(utf8_stringview textContent, const FontCascade &fontCascade,
                                               bool whitespaceIsCollapsed,
                                               const RenderStyle *firstLineStyle) noexcept
  {
    krys_debug_assert(FontCascade::CharacterRangeCodePath(textContent) == FontCascade::CodePath::Simple);
    // FIXME: All these checks should be more fine-grained at the inline item level.
    if (fontCascade.WordSpacing() || fontCascade.LetterSpacing())
    {
      return false;
    }

    auto fontVariantCaps = fontCascade.GetFontDescription().VariantCaps();
    if (fontVariantCaps == FontVariantCaps::Small || fontVariantCaps == FontVariantCaps::AllSmall
        || fontVariantCaps == FontVariantCaps::Petite || fontVariantCaps == FontVariantCaps::AllPetite)
    {
      return false;
    }

    // Additional check on the font codepath.
    auto run = TextRun(textContent);
    run.SetCharacterScanForCodePath(false);
    if (fontCascade.GetCodePath(run) != FontCascade::CodePath::Simple)
    {
      return false;
    }

    if (firstLineStyle && fontCascade != firstLineStyle->GetFontCascade())
    {
      return false;
    }

    Ref<const Font> primaryFont = fontCascade.PrimaryFont();
    if (primaryFont->SyntheticBoldOffset())
    {
      return false;
    }

    for (auto it = text::CodePointIterator(textContent); !it.AtEnd(); ++it)
    {
      if (!fontCascade.CanUseSimplifiedTextMeasuring(*it, AutoVariant, whitespaceIsCollapsed, *primaryFont))
      {
        return false;
      }
    }
    return true;
  }

  bool TextUtil::HasPositionDependentContentWidth(utf8_stringview textContent) noexcept
  {
    return textContent.find(u8'\t') != utf8_stringview::npos;
  }
}
