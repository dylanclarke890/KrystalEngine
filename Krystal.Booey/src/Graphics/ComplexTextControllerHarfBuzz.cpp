#include "Krystal.Booey/Graphics/ComplexTextController.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/FontTaggedSettings.hpp"
#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Text/HarfBuzz.hpp"
#include <hb.h>
#include <unicode/uscript.h>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD float HarfBuzzPositionToFloat(hb_position_t value) noexcept
    {
      return static_cast<float>(value) / (1 << 16);
    }

    KRYS_NODISCARD Maybe<UScriptCode> CharacterScript(char32 character) noexcept
    {
      UErrorCode errorCode = U_ZERO_ERROR;
      UScriptCode script = uscript_getScript(static_cast<UChar32>(character), &errorCode);
      if (U_FAILURE(errorCode))
      {
        return null;
      }
      return script;
    }

    KRYS_NODISCARD hb_script_t HarfBuzzScript(UScriptCode script) noexcept
    {
      if (script == USCRIPT_INVALID_CODE)
      {
        return HB_SCRIPT_INVALID;
      }
      return hb_script_from_string(uscript_getShortName(script), -1);
    }

    struct HBRun
    {
      uint32 StartIndex;
      uint32 EndIndex;
      UScriptCode Script;
    };

    KRYS_NODISCARD Maybe<HBRun> FindNextRun(utf8_stringview characters, uint32 offset) noexcept
    {
      auto decode = [&](uint32 index)
      {
        return text::DecodeUTF8(Span<const char8> {characters}.subspan(index));
      };

      if (offset >= characters.length())
      {
        return null;
      }

      auto first = decode(offset);
      auto currentScript = CharacterScript(first.CodePoint);
      if (!currentScript)
      {
        return null;
      }

      uint32 startIndex = offset;
      uint32 index = offset + first.Length;
      for (; index < characters.length();)
      {
        auto decoded = decode(index);
        auto character = decoded.CodePoint;
        if (FontCascade::TreatAsZeroWidthSpace(character))
        {
          index += decoded.Length;
          continue;
        }

        auto nextScript = CharacterScript(character);
        if (!nextScript)
        {
          return null;
        }

        // §5.1 Handling Characters with the Common Script Property.
        // Programs must resolve any of the special Script property values, such as Common,
        // based on the context of the surrounding characters. A simple heuristic uses the
        // script of the preceding character, which works well in many cases.
        // http://www.unicode.org/reports/tr24/#Common.
        //
        // FIXME: cover all other cases mentioned in the spec (ie. brackets or quotation marks).
        //
        // If next script is inherited or common, keep using the current script.
        if (*nextScript == USCRIPT_INHERITED || *nextScript == USCRIPT_COMMON)
        {
          index += decoded.Length;
          continue;
        }
        // If current script is inherited or common, set the next script as current.
        if (*currentScript == USCRIPT_INHERITED || *currentScript == USCRIPT_COMMON)
        {
          currentScript = nextScript;
          index += decoded.Length;
          continue;
        }

        if (*currentScript != *nextScript
            && !uscript_hasScript(static_cast<UChar32>(character), *currentScript))
        {
          return HBRun {startIndex, index, *currentScript};
        }
        index += decoded.Length;
      }

      return HBRun {startIndex, index, *currentScript};
    }

    /// @brief The OpenType features to shape with @p fontDescription.
    KRYS_NODISCARD SmallList<hb_feature_t, 4> ComputeFeatures(const FontDescription &fontDescription) noexcept
    {
      FeaturesMap featuresToBeApplied;
      auto set = [&](const char (&tag)[5], int value)
      {
        featuresToBeApplied[FontFeatureTag(tag)] = value;
      };

      // 7.2. Feature precedence
      // https://www.w3.org/TR/css-fonts-3/#feature-precedence

      // 1. Font features enabled by default, including features required for a given script.

      // FIXME: optical sizing.

      // 2. If the font is defined via an @font-face rule, the font features implied by the
      //    font-feature-settings descriptor in the @font-face rule.
      // FIXME: No @font-face.

      // 3. Font features implied by the value of the ‘font-variant’ property, the related ‘font-variant’
      //    subproperties and any other CSS property that uses OpenType features.
      for (auto &[tag, value] :
           ComputeFeatureSettingsFromVariants(fontDescription.VariantSettings(), nullptr))
      {
        featuresToBeApplied[tag] = value;
      }

      // 4. Feature settings determined by properties other than ‘font-variant’ or ‘font-feature-settings’.
      bool optimizeSpeed = fontDescription.GetTextRenderingMode() == TextRenderingMode::OptimizeSpeed;
      bool shouldDisableLigaturesForSpacing = fontDescription.ShouldDisableLigaturesForSpacing();

      // clig and liga are on by default in HarfBuzz.
      auto commonLigatures = fontDescription.VariantCommonLigatures();
      if (shouldDisableLigaturesForSpacing
          || (commonLigatures == FontVariantLigatures::No
              || (commonLigatures == FontVariantLigatures::Normal && optimizeSpeed)))
      {
        set("liga", 0);
        set("clig", 0);
      }

      // dlig is off by default in HarfBuzz.
      auto discretionaryLigatures = fontDescription.VariantDiscretionaryLigatures();
      if (!shouldDisableLigaturesForSpacing && discretionaryLigatures == FontVariantLigatures::Yes)
      {
        set("dlig", 1);
      }

      // hlig is off by default in HarfBuzz.
      auto historicalLigatures = fontDescription.VariantHistoricalLigatures();
      if (!shouldDisableLigaturesForSpacing && historicalLigatures == FontVariantLigatures::Yes)
      {
        set("hlig", 1);
      }

      // calt is on by default in HarfBuzz.
      auto contextualAlternates = fontDescription.VariantContextualAlternates();
      if (shouldDisableLigaturesForSpacing
          || (contextualAlternates == FontVariantLigatures::No
              || (contextualAlternates == FontVariantLigatures::Normal && optimizeSpeed)))
      {
        set("calt", 0);
      }

      switch (fontDescription.WidthVariant())
      {
        case FontWidthVariant::RegularWidth: break;
        case FontWidthVariant::HalfWidth:    set("hwid", 1); break;
        case FontWidthVariant::ThirdWidth:   set("twid", 1); break;
        case FontWidthVariant::QuarterWidth: set("qwid", 1); break;
      }

      switch (fontDescription.VariantEastAsianVariant())
      {
        case FontVariantEastAsianVariant::Normal:      break;
        case FontVariantEastAsianVariant::Jis78:       set("jp78", 1); break;
        case FontVariantEastAsianVariant::Jis83:       set("jp83", 1); break;
        case FontVariantEastAsianVariant::Jis90:       set("jp90", 1); break;
        case FontVariantEastAsianVariant::Jis04:       set("jp04", 1); break;
        case FontVariantEastAsianVariant::Simplified:  set("smpl", 1); break;
        case FontVariantEastAsianVariant::Traditional: set("trad", 1); break;
      }

      switch (fontDescription.VariantEastAsianWidth())
      {
        case FontVariantEastAsianWidth::Normal:       break;
        case FontVariantEastAsianWidth::Full:         set("fwid", 1); break;
        case FontVariantEastAsianWidth::Proportional: set("pwid", 1); break;
      }

      switch (fontDescription.VariantEastAsianRuby())
      {
        case FontVariantEastAsianRuby::Normal: break;
        case FontVariantEastAsianRuby::Yes:    set("ruby", 1); break;
      }

      switch (fontDescription.VariantNumericFigure())
      {
        case FontVariantNumericFigure::Normal:          break;
        case FontVariantNumericFigure::LiningNumbers:   set("lnum", 1); break;
        case FontVariantNumericFigure::OldStyleNumbers: set("onum", 1); break;
      }

      switch (fontDescription.VariantNumericSpacing())
      {
        case FontVariantNumericSpacing::Normal:              break;
        case FontVariantNumericSpacing::ProportionalNumbers: set("pnum", 1); break;
        case FontVariantNumericSpacing::TabularNumbers:      set("tnum", 1); break;
      }

      switch (fontDescription.VariantNumericFraction())
      {
        case FontVariantNumericFraction::Normal:            break;
        case FontVariantNumericFraction::DiagonalFractions: set("frac", 1); break;
        case FontVariantNumericFraction::StackedFractions:  set("afrc", 1); break;
      }

      if (fontDescription.VariantNumericOrdinal() == FontVariantNumericOrdinal::Yes)
      {
        set("ordn", 1);
      }

      if (fontDescription.VariantNumericSlashedZero() == FontVariantNumericSlashedZero::Yes)
      {
        set("zero", 1);
      }

      // 5. Font features implied by the value of ‘font-feature-settings’ property.
      for (auto &newFeature : fontDescription.FeatureSettings())
      {
        featuresToBeApplied[newFeature.Tag()] = newFeature.Value();
      }

      SmallList<hb_feature_t, 4> features;
      features.reserve(featuresToBeApplied.size());
      for (const auto &[key, value] : featuresToBeApplied)
      {
        features.push_back({HB_TAG(key[0], key[1], key[2], key[3]), static_cast<uint32_t>(value), 0,
                            static_cast<unsigned>(-1)});
      }
      return features;
    }
  }

  ComplexTextController::ComplexTextRun::ComplexTextRun(hb_buffer_t *buffer, const Font &font,
                                                        utf8_stringview characters, uint32 stringLocation,
                                                        uint32 indexBegin, uint32 indexEnd) noexcept
      : _initialAdvance(0, 0), _font(ShareRef(font)), _characters(characters), _indexBegin(indexBegin),
        _indexEnd(indexEnd), _glyphCount(hb_buffer_get_length(buffer)), _stringLocation(stringLocation),
        _isLTR(HB_DIRECTION_IS_FORWARD(hb_buffer_get_direction(buffer))),
        _textAutospaceSize(TextAutospace::TextAutospaceSize(font))
  {
    if (!_glyphCount)
    {
      return;
    }

    _glyphs.resize(_glyphCount);
    _baseAdvances.resize(_glyphCount);
    _glyphOrigins.resize(_glyphCount);
    _coreTextIndices.resize(_glyphCount);

    hb_glyph_info_t *glyphInfos = hb_buffer_get_glyph_infos(buffer, nullptr);
    hb_glyph_position_t *glyphPositions = hb_buffer_get_glyph_positions(buffer, nullptr);

    // HarfBuzz returns the shaping result in visual order. We don't need to flip for RTL.
    for (uint32 i = 0; i < _glyphCount; ++i)
    {
      _coreTextIndices[i] = glyphInfos[i].cluster;

      auto glyph = static_cast<Glyph>(glyphInfos[i].codepoint);
      if (_font->IsZeroWidthSpaceGlyph(glyph) || !_font->PlatformData().Size())
      {
        _glyphs[i] = glyph;
        _baseAdvances[i] = {};
        _glyphOrigins[i] = {};
        continue;
      }

      float offsetX = HarfBuzzPositionToFloat(glyphPositions[i].x_offset);
      float offsetY = HarfBuzzPositionToFloat(glyphPositions[i].y_offset);
      float advanceX = HarfBuzzPositionToFloat(glyphPositions[i].x_advance);
      float advanceY = HarfBuzzPositionToFloat(glyphPositions[i].y_advance);

      _glyphs[i] = glyph;
      _baseAdvances[i] = {advanceX, advanceY};
      _glyphOrigins[i] = {offsetX, offsetY};
    }
    _initialAdvance = FloatSize(_glyphOrigins[0].X(), _glyphOrigins[0].Y());
  }

  void ComplexTextController::CollectComplexTextRunsForCharacters(utf8_stringview characters,
                                                                  uint32 stringLocation,
                                                                  const Font *font) noexcept
  {
    krys_debug_assert(!characters.empty());
    auto length = static_cast<uint32>(characters.length());
    if (!font)
    {
      // Create a run of missing glyphs from the primary font.
      _complexTextRuns.push_back(ComplexTextRun::Create(*_fontCascade.PrimaryFont(), characters,
                                                        stringLocation, 0, length, _run.Ltr()));
      return;
    }

    auto *hbFont = font->HarfBuzzFont();
    if (!hbFont)
    {
      // FIXME: A face HarfBuzz can't read is drawn with missing glyphs.
      _complexTextRuns.push_back(
        ComplexTextRun::Create(*font, characters, stringLocation, 0, length, _run.Ltr()));
      return;
    }

    auto features = ComputeFeatures(_fontCascade.GetFontDescription());
    // Kerning is not handled as font features, so only in case it's explicitly disabled
    // we need to include kern feature.
    if (!_fontCascade.EnableKerning())
    {
      features.insert(features.begin(),
                      hb_feature_t {HB_TAG('k', 'e', 'r', 'n'), 0, 0, static_cast<unsigned>(-1)});
    }

    static thread_local text::HbUniquePtr<hb_buffer_t> buffer(hb_buffer_create());

    // The computed "locale" equals the "lang" attribute. The latter must be a valid BCP 47 language tag.
    // This is exactly what hb_language_from_string() expects, so we can pass directly.
    auto locale = _fontCascade.GetFontDescription().ComputedLocale().View();
    auto language = hb_language_from_string(reinterpret_cast<const char *>(locale.data()),
                                            static_cast<int>(locale.length()));

    auto shape = [&](const HBRun &run)
    {
      hb_buffer_set_language(buffer.get(), language);
      hb_buffer_set_script(buffer.get(), HarfBuzzScript(run.Script));
      if (!_mayUseNaturalWritingDirection || _run.DirectionalOverride())
      {
        hb_buffer_set_direction(buffer.get(), _run.Rtl() ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
      }
      else
      {
        hb_buffer_set_direction(buffer.get(), hb_script_get_horizontal_direction(HarfBuzzScript(run.Script)));
      }

      hb_buffer_add_utf8(buffer.get(), reinterpret_cast<const char *>(characters.data()),
                         static_cast<int>(length), run.StartIndex,
                         static_cast<int>(run.EndIndex - run.StartIndex));

      hb_shape(hbFont, buffer.get(), features.empty() ? nullptr : features.data(),
               static_cast<unsigned>(features.size()));
      _complexTextRuns.push_back(ComplexTextRun::Create(buffer.get(), *font, characters, stringLocation,
                                                        run.StartIndex, run.EndIndex));
      hb_buffer_reset(buffer.get());
    };

    SmallList<HBRun, 1> runList;
    uint32 offset = 0;
    while (offset < length)
    {
      auto run = FindNextRun(characters, offset);
      if (!run)
      {
        break;
      }
      offset = run->EndIndex;
      if (_run.Ltr())
      {
        shape(*run);
      }
      else
      {
        runList.push_back(*run);
      }
    }

    for (auto it = runList.rbegin(); it != runList.rend(); ++it)
    {
      shape(*it);
    }
  }
}
