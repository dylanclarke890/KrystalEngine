#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/Graphics/FontFeatureValues.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo
{
  FeaturesMap ComputeFeatureSettingsFromVariants(const FontVariantSettings &variantSettings,
                                                 RefPtr<FontFeatureValues> fontFeatureValues) noexcept
  {
    FeaturesMap features;
    auto set = [&](const FontTag &tag, int value)
    {
      features.insert_or_assign(tag, value);
    };

    switch (variantSettings.CommonLigatures)
    {
      case FontVariantLigatures::Normal: break;
      case FontVariantLigatures::Yes:
      {
        set(FontFeatureTag("liga"), 1);
        set(FontFeatureTag("clig"), 1);
        break;
      }
      case FontVariantLigatures::No:
      {
        set(FontFeatureTag("liga"), 0);
        set(FontFeatureTag("clig"), 0);
        break;
      }
    }

    switch (variantSettings.DiscretionaryLigatures)
    {
      case FontVariantLigatures::Normal: break;
      case FontVariantLigatures::Yes:    set(FontFeatureTag("dlig"), 1); break;
      case FontVariantLigatures::No:     set(FontFeatureTag("dlig"), 0); break;
    }

    switch (variantSettings.HistoricalLigatures)
    {
      case FontVariantLigatures::Normal: break;
      case FontVariantLigatures::Yes:    set(FontFeatureTag("hlig"), 1); break;
      case FontVariantLigatures::No:     set(FontFeatureTag("hlig"), 0); break;
    }

    switch (variantSettings.ContextualAlternates)
    {
      case FontVariantLigatures::Normal: break;
      case FontVariantLigatures::Yes:    set(FontFeatureTag("calt"), 1); break;
      case FontVariantLigatures::No:     set(FontFeatureTag("calt"), 0); break;
    }

    switch (variantSettings.Position)
    {
      case FontVariantPosition::Normal:      break;
      case FontVariantPosition::Subscript:   set(FontFeatureTag("subs"), 1); break;
      case FontVariantPosition::Superscript: set(FontFeatureTag("sups"), 1); break;
    }

    switch (variantSettings.Caps)
    {
      case FontVariantCaps::Normal:    break;
      case FontVariantCaps::AllSmall:  set(FontFeatureTag("c2sc"), 1); KRYS_FALLTHROUGH;
      case FontVariantCaps::Small:     set(FontFeatureTag("smcp"), 1); break;
      case FontVariantCaps::AllPetite: set(FontFeatureTag("c2pc"), 1); KRYS_FALLTHROUGH;
      case FontVariantCaps::Petite:    set(FontFeatureTag("pcap"), 1); break;
      case FontVariantCaps::Unicase:   set(FontFeatureTag("unic"), 1); break;
      case FontVariantCaps::Titling:   set(FontFeatureTag("titl"), 1); break;
    }

    switch (variantSettings.NumericFigure)
    {
      case FontVariantNumericFigure::Normal:          break;
      case FontVariantNumericFigure::LiningNumbers:   set(FontFeatureTag("lnum"), 1); break;
      case FontVariantNumericFigure::OldStyleNumbers: set(FontFeatureTag("onum"), 1); break;
    }

    switch (variantSettings.NumericSpacing)
    {
      case FontVariantNumericSpacing::Normal:              break;
      case FontVariantNumericSpacing::ProportionalNumbers: set(FontFeatureTag("pnum"), 1); break;
      case FontVariantNumericSpacing::TabularNumbers:      set(FontFeatureTag("tnum"), 1); break;
    }

    switch (variantSettings.NumericFraction)
    {
      case FontVariantNumericFraction::Normal:            break;
      case FontVariantNumericFraction::DiagonalFractions: set(FontFeatureTag("frac"), 1); break;
      case FontVariantNumericFraction::StackedFractions:  set(FontFeatureTag("afrc"), 1); break;
    }

    switch (variantSettings.NumericOrdinal)
    {
      case FontVariantNumericOrdinal::Normal: break;
      case FontVariantNumericOrdinal::Yes:    set(FontFeatureTag("ordn"), 1); break;
    }

    switch (variantSettings.NumericSlashedZero)
    {
      case FontVariantNumericSlashedZero::Normal: break;
      case FontVariantNumericSlashedZero::Yes:    set(FontFeatureTag("zero"), 1); break;
    }

    if (!variantSettings.Alternates.IsNormal())
    {
      const auto &values = variantSettings.Alternates.GetValues();
      if (values.HistoricalForms)
      {
        set(FontFeatureTag("hist"), 1);
      }

      if (fontFeatureValues)
      {
        auto lookupTags = [](const css::CSSOMString &name,
                             const FontFeatureValues::Tags &tags) -> Span<const uint32>
        {
          auto found = tags.find(name);
          if (found == tags.end())
          {
            return {};
          }

          return found->second;
        };

        auto addFeatureTagWithValue = [&](const Maybe<css::CSSOMString> &name,
                                          const FontFeatureValues::Tags &tags, const FontTag &codename)
        {
          if (!name)
          {
            return;
          }

          for (uint32 value : lookupTags(*name, tags))
          {
            if (value < 1 || value > 99)
            {
              continue;
            }

            set(codename, static_cast<int>(value));
          }
        };

        // https://www.w3.org/TR/css-fonts-4/#multi-value-features
        auto makeTag = [](uint32 value, Array<char, 2> codename)
        {
          char rightDigit = static_cast<char>('0' + (value % 10));
          char leftDigit = static_cast<char>('0' + (value / 10));
          return FontTag {codename[0], codename[1], leftDigit, rightDigit};
        };

        // For styleset and character-variant, the tag name itself is the actual conveyor of information.
        auto addFeatureTags = [&](const List<css::CSSOMString> &names, const FontFeatureValues::Tags &tags,
                                  Array<char, 2> codename)
        {
          for (const auto &name : names)
          {
            for (uint32 value : lookupTags(name, tags))
            {
              if (value < 1 || value > 99)
              {
                continue;
              }

              set(makeTag(value, codename), 1);
            }
          }
        };

        addFeatureTags(values.Styleset, fontFeatureValues->Styleset(), {'s', 's'});
        addFeatureTags(values.CharacterVariant, fontFeatureValues->CharacterVariant(), {'c', 'v'});
        addFeatureTagWithValue(values.Stylistic, fontFeatureValues->Stylistic(), FontFeatureTag("salt"));

        // "swash" feature sets 2 flags.
        addFeatureTagWithValue(values.Swash, fontFeatureValues->Swash(), FontFeatureTag("swsh"));
        addFeatureTagWithValue(values.Swash, fontFeatureValues->Swash(), FontFeatureTag("cswh"));

        addFeatureTagWithValue(values.Ornaments, fontFeatureValues->Ornaments(), FontFeatureTag("ornm"));
        addFeatureTagWithValue(values.Annotation, fontFeatureValues->Annotation(), FontFeatureTag("nalt"));
      }
    }

    switch (variantSettings.EastAsianVariant)
    {
      case FontVariantEastAsianVariant::Normal:      break;
      case FontVariantEastAsianVariant::Jis78:       set(FontFeatureTag("jp78"), 1); break;
      case FontVariantEastAsianVariant::Jis83:       set(FontFeatureTag("jp83"), 1); break;
      case FontVariantEastAsianVariant::Jis90:       set(FontFeatureTag("jp90"), 1); break;
      case FontVariantEastAsianVariant::Jis04:       set(FontFeatureTag("jp04"), 1); break;
      case FontVariantEastAsianVariant::Simplified:  set(FontFeatureTag("smpl"), 1); break;
      case FontVariantEastAsianVariant::Traditional: set(FontFeatureTag("trad"), 1); break;
    }

    switch (variantSettings.EastAsianWidth)
    {
      case FontVariantEastAsianWidth::Normal:       break;
      case FontVariantEastAsianWidth::Full:         set(FontFeatureTag("fwid"), 1); break;
      case FontVariantEastAsianWidth::Proportional: set(FontFeatureTag("pwid"), 1); break;
    }

    switch (variantSettings.EastAsianRuby)
    {
      case FontVariantEastAsianRuby::Normal: break;
      case FontVariantEastAsianRuby::Yes:    set(FontFeatureTag("ruby"), 1); break;
    }

    return features;
  }
}

namespace krys
{
  size_t
    DefaultHash<boo::FontVariantAlternatesValues>::Hash(const boo::FontVariantAlternatesValues &key) noexcept
  {
    auto hashString = [](const Maybe<boo::css::CSSOMString> &string) -> size_t
    {
      return string ? krys::Hash::Combine(true, *string) : krys::Hash::Combine(false);
    };
    auto hashList = [](const List<boo::css::CSSOMString> &list) -> size_t
    {
      size_t seed = list.size();
      for (const auto &string : list)
      {
        seed = krys::Hash::Combine(seed, string);
      }
      return seed;
    };
    return krys::Hash::Combine(key.HistoricalForms, hashString(key.Stylistic), hashList(key.Styleset),
                               hashList(key.CharacterVariant), hashString(key.Swash),
                               hashString(key.Ornaments), hashString(key.Annotation));
  }

  size_t DefaultHash<boo::FontVariantAlternates>::Hash(const boo::FontVariantAlternates &key) noexcept
  {
    if (key.IsNormal())
    {
      return krys::Hash::Combine(false);
    }
    return krys::Hash::Combine(true, key.GetValues());
  }
}
