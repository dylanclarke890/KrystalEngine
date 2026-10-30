#include "Krystal.Booey/Rendering/RenderQuote.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/RenderTextFragment.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"
#include "Krystal.Booey/Style/Values/Content/Quotes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include <array>
#include <compare>
#include <string>
#include <string_view>

namespace krys::boo
{
  namespace
  {
    // These strings need to be compared according to "Extended Filtering", as in Section 3.3.2 in RFC4647.
    // https://tools.ietf.org/html/rfc4647#page-10
    //
    // The "checkFurther" field is needed in one specific situation.
    // In the quoteTable below, there are lines like:
    // { "de"   , 0x201e, 0x201c, 0x201a, 0x2018 },
    // { "de-ch", 0x00ab, 0x00bb, 0x2039, 0x203a },
    // Let's say the binary search arbitrarily decided to test our key against the upper line "de" first.
    // If the key we're testing against is "de-ch", then we should report "greater than",
    // so the binary search will keep searching and eventually find the "de-ch" line.
    // However, if the key we're testing against is "de-de", then we should report "equal to",
    // because these are the quotes we should use for all "de" except for "de-ch".
    struct QuotesForLanguage
    {
      std::string_view Language;
      uint8 CheckFurther {0};
      char32 Open1 {0};
      char32 Close1 {0};
      char32 Open2 {0};
      char32 Close2 {0};
    };

    // Table of quotes from http://www.whatwg.org/specs/web-apps/current-work/multipage/rendering.html#quotes
    // FIXME: This table is out-of-date.
    constexpr auto quoteTable = std::to_array<QuotesForLanguage>({
      {"af", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"agq", 0, 0x201e, 0x201d, 0x201a, 0x2019},
      {"ak", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"am", 0, 0x00ab, 0x00bb, 0x2039, 0x203a},
      {"ar", 0, 0x201d, 0x201c, 0x2019, 0x2018},       {"asa", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"az-cyrl", 0, 0x00ab, 0x00bb, 0x2039, 0x203a},  {"bas", 0, 0x00ab, 0x00bb, 0x201e, 0x201c},
      {"bem", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"bez", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"bg", 0, 0x201e, 0x201c, 0x201a, 0x2018},       {"bm", 0, 0x00ab, 0x00bb, 0x201c, 0x201d},
      {"bn", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"br", 0, 0x00ab, 0x00bb, 0x2039, 0x203a},
      {"brx", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"bs-cyrl", 0, 0x201e, 0x201c, 0x201a, 0x2018},
      {"ca", 0, 0x201c, 0x201d, 0x00ab, 0x00bb},       {"cgg", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"chr", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"cs", 0, 0x201e, 0x201c, 0x201a, 0x2018},
      {"da", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"dav", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"de", 1, 0x201e, 0x201c, 0x201a, 0x2018},       {"de-ch", 0, 0x00ab, 0x00bb, 0x2039, 0x203a},
      {"dje", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"dua", 0, 0x00ab, 0x00bb, 0x2018, 0x2019},
      {"dyo", 0, 0x00ab, 0x00bb, 0x201c, 0x201d},      {"dz", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"ebu", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"ee", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"el", 0, 0x00ab, 0x00bb, 0x201c, 0x201d},       {"en", 1, 0x201c, 0x201d, 0x2018, 0x2019},
      {"en-gb", 0, 0x201c, 0x201d, 0x2018, 0x2019},    {"es", 0, 0x201c, 0x201d, 0x00ab, 0x00bb},
      {"et", 0, 0x201e, 0x201c, 0x201a, 0x2018},       {"eu", 0, 0x201c, 0x201d, 0x00ab, 0x00bb},
      {"ewo", 0, 0x00ab, 0x00bb, 0x201c, 0x201d},      {"fa", 0, 0x00ab, 0x00bb, 0x2039, 0x203a},
      {"ff", 0, 0x201e, 0x201d, 0x201a, 0x2019},       {"fi", 0, 0x201d, 0x201d, 0x2019, 0x2019},
      {"fr", 2, 0x00ab, 0x00bb, 0x00ab, 0x00bb},       {"fr-ca", 0, 0x00ab, 0x00bb, 0x2039, 0x203a},
      {"fr-ch", 0, 0x00ab, 0x00bb, 0x2039, 0x203a},    {"gsw", 0, 0x00ab, 0x00bb, 0x2039, 0x203a},
      {"gu", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"guz", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"ha", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"he", 0, 0x0022, 0x0022, 0x0027, 0x0027},
      {"hi", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"hr", 0, 0x201e, 0x201c, 0x201a, 0x2018},
      {"hu", 0, 0x201e, 0x201d, 0x00bb, 0x00ab},       {"id", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"ig", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"it", 0, 0x00ab, 0x00bb, 0x201c, 0x201d},
      {"ja", 0, 0x300c, 0x300d, 0x300e, 0x300f},       {"jgo", 0, 0x00ab, 0x00bb, 0x2039, 0x203a},
      {"jmc", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"kab", 0, 0x00ab, 0x00bb, 0x201c, 0x201d},
      {"kam", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"kde", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"kea", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"khq", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"ki", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"kkj", 0, 0x00ab, 0x00bb, 0x2039, 0x203a},
      {"kln", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"km", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"kn", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"ko", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"ksb", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"ksf", 0, 0x00ab, 0x00bb, 0x2018, 0x2019},
      {"lag", 0, 0x201d, 0x201d, 0x2019, 0x2019},      {"lg", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"ln", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"lo", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"lt", 0, 0x201e, 0x201c, 0x201e, 0x201c},       {"lu", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"luo", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"luy", 0, 0x201e, 0x201c, 0x201a, 0x2018},
      {"lv", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"mas", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"mer", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"mfe", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"mg", 0, 0x00ab, 0x00bb, 0x201c, 0x201d},       {"mgo", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"mk", 0, 0x201e, 0x201c, 0x201a, 0x2018},       {"ml", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"mr", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"ms", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"mua", 0, 0x00ab, 0x00bb, 0x201c, 0x201d},      {"my", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"naq", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"nb", 0, 0x00ab, 0x00bb, 0x2018, 0x2019},
      {"nd", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"nl", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"nmg", 0, 0x201e, 0x201d, 0x00ab, 0x00bb},      {"nn", 0, 0x00ab, 0x00bb, 0x2018, 0x2019},
      {"nnh", 0, 0x00ab, 0x00bb, 0x201c, 0x201d},      {"nus", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"nyn", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"pl", 0, 0x201e, 0x201d, 0x00ab, 0x00bb},
      {"pt", 1, 0x201c, 0x201d, 0x2018, 0x2019},       {"pt-pt", 0, 0x00ab, 0x00bb, 0x201c, 0x201d},
      {"rn", 0, 0x201d, 0x201d, 0x2019, 0x2019},       {"ro", 0, 0x201e, 0x201d, 0x00ab, 0x00bb},
      {"rof", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"ru", 0, 0x00ab, 0x00bb, 0x201e, 0x201c},
      {"rw", 0, 0x00ab, 0x00bb, 0x2018, 0x2019},       {"rwk", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"saq", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"sbp", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"seh", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"ses", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"sg", 0, 0x00ab, 0x00bb, 0x201c, 0x201d},       {"shi", 1, 0x00ab, 0x00bb, 0x201e, 0x201d},
      {"shi-tfng", 0, 0x00ab, 0x00bb, 0x201e, 0x201d}, {"si", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"sk", 0, 0x201e, 0x201c, 0x201a, 0x2018},       {"sl", 0, 0x201e, 0x201c, 0x201a, 0x2018},
      {"sn", 0, 0x201d, 0x201d, 0x2019, 0x2019},       {"so", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"sq", 0, 0x201e, 0x201c, 0x201a, 0x2018},       {"sr", 1, 0x201e, 0x201c, 0x201a, 0x2018},
      {"sr-latn", 0, 0x201e, 0x201c, 0x201a, 0x2018},  {"sv", 0, 0x201d, 0x201d, 0x2019, 0x2019},
      {"sw", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"swc", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"ta", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"te", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"teo", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"th", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"ti-er", 0, 0x2018, 0x2019, 0x201c, 0x201d},    {"to", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"tr", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"twq", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"tzm", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"uk", 0, 0x00ab, 0x00bb, 0x201e, 0x201c},
      {"ur", 0, 0x201d, 0x201c, 0x2019, 0x2018},       {"vai", 1, 0x201c, 0x201d, 0x2018, 0x2019},
      {"vai-latn", 0, 0x201c, 0x201d, 0x2018, 0x2019}, {"vi", 0, 0x201c, 0x201d, 0x2018, 0x2019},
      {"vun", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"xh", 0, 0x2018, 0x2019, 0x201c, 0x201d},
      {"xog", 0, 0x201c, 0x201d, 0x2018, 0x2019},      {"yav", 0, 0x00ab, 0x00bb, 0x00ab, 0x00bb},
      {"yo", 0, 0x201c, 0x201d, 0x2018, 0x2019},       {"zh", 1, 0x201c, 0x201d, 0x2018, 0x2019},
      {"zh-hant", 0, 0x300c, 0x300d, 0x300e, 0x300f},  {"zu", 0, 0x201c, 0x201d, 0x2018, 0x2019},
    });

    struct SubtagComparison
    {
      size_t KeyLength;
      size_t KeyContinue;
      size_t RangeLength;
      size_t RangeContinue;
      std::strong_ordering Comparison {std::strong_ordering::equal};
    };

    KRYS_NODISCARD SubtagComparison SubtagCompare(std::string_view key, std::string_view range) noexcept
    {
      SubtagComparison result;

      result.KeyLength = key.size();
      result.KeyContinue = result.KeyLength;
      if (auto hyphenIndex = key.find('-'); hyphenIndex != std::string_view::npos)
      {
        result.KeyLength = hyphenIndex;
        result.KeyContinue = result.KeyLength + 1;
      }

      result.RangeLength = range.size();
      result.RangeContinue = result.RangeLength;
      if (auto hyphenIndex = range.find('-'); hyphenIndex != std::string_view::npos)
      {
        result.RangeLength = hyphenIndex;
        result.RangeContinue = result.RangeLength + 1;
      }

      if (result.KeyLength == result.RangeLength)
      {
        result.Comparison = key.substr(0, result.KeyLength) <=> range.substr(0, result.KeyLength);
      }
      else
      {
        result.Comparison = key <=> range;
      }

      return result;
    }

    KRYS_NODISCARD std::strong_ordering
      QuoteTableLanguageComparisonFunction(std::string_view key, Span<const QuotesForLanguage> range) noexcept
    {
      // These strings need to be compared according to "Extended Filtering", as in Section 3.3.2 in RFC4647.
      // https://tools.ietf.org/html/rfc4647#page-10
      //
      // We can exploit a few things here to improve perf:
      // 1. The first subtag must be matched exactly
      // 2. All the ranges have either 1 or 2 subtags
      // 3. None of the subtags in any of the ranges are wildcards
      //
      // Also, see the comment just above the QuotesForLanguage struct.

      auto firstSubtagComparison = SubtagCompare(key, range[0].Language);

      if (firstSubtagComparison.KeyLength != firstSubtagComparison.RangeLength)
      {
        return firstSubtagComparison.Comparison;
      }

      if (std::is_neq(firstSubtagComparison.Comparison))
      {
        return firstSubtagComparison.Comparison;
      }

      for (auto &checkFurtherRange : range.subspan(1))
      {
        if (std::is_eq(QuoteTableLanguageComparisonFunction(
              key, Span<const QuotesForLanguage>(&checkFurtherRange, 1))))
        {
          // Tell the binary search to check later in the array of ranges, to eventually find the match we
          // just found here.
          return std::strong_ordering::greater;
        }
      }

      auto rangeRest =
        range[0].Language.substr(std::min(firstSubtagComparison.RangeContinue, range[0].Language.size()));
      for (size_t keyOffset = firstSubtagComparison.KeyContinue;;)
      {
        auto nextSubtagComparison = SubtagCompare(key.substr(std::min(keyOffset, key.size())), rangeRest);

        if (!nextSubtagComparison.RangeLength)
        {
          // E.g. The key is "zh-Hans" and the range is "zh".
          return std::strong_ordering::equal;
        }

        if (!nextSubtagComparison.KeyLength)
        {
          // E.g. the key is "zh" and the range is "zh-Hant".
          return nextSubtagComparison.Comparison;
        }

        if (nextSubtagComparison.KeyLength == 1)
        {
          // E.g. the key is "zh-x-Hant" and the range is "zh-Hant".
          // We want to try to find the range "zh", so tell the binary search to check earlier in the array of
          // ranges.
          return std::strong_ordering::less;
        }

        if (nextSubtagComparison.KeyLength == nextSubtagComparison.RangeLength
            && std::is_eq(nextSubtagComparison.Comparison))
        {
          // E.g. the key is "de-Latn-ch" and the range is "de-ch".
          return std::strong_ordering::equal;
        }

        keyOffset += nextSubtagComparison.KeyContinue;
      }
    }

    KRYS_NODISCARD const QuotesForLanguage *BinaryFindQuotes(std::string_view key,
                                                             Span<const QuotesForLanguage> subrange) noexcept
    {
      if (subrange.empty())
      {
        return nullptr;
      }

      auto &middle = subrange[subrange.size() / 2];
      auto middleIndex = static_cast<size_t>(&middle - quoteTable.data());
      auto comparison = QuoteTableLanguageComparisonFunction(
        key, Span<const QuotesForLanguage>(quoteTable).subspan(middleIndex, 1 + middle.CheckFurther));
      if (std::is_eq(comparison))
      {
        return &middle;
      }
      if (std::is_lt(comparison))
      {
        return BinaryFindQuotes(key, subrange.first(subrange.size() / 2));
      }
      return BinaryFindQuotes(key, subrange.subspan(subrange.size() / 2 + 1));
    }

    KRYS_NODISCARD const QuotesForLanguage *QuotesForLanguageTag(utf8_stringview language) noexcept
    {
      if (language.empty())
      {
        return nullptr;
      }

      std::string languageKeyBuffer;
      languageKeyBuffer.reserve(language.size());
      for (auto character : language)
      {
        auto lower = character >= u8'A' && character <= u8'Z'
                       ? static_cast<char8>(character + (u8'a' - u8'A'))
                       : character;
        if (!((lower >= u8'a' && lower <= u8'z') || lower == u8'-'))
        {
          return nullptr;
        }
        languageKeyBuffer.push_back(static_cast<char>(lower));
      }

      return BinaryFindQuotes(languageKeyBuffer, quoteTable);
    }

    KRYS_NODISCARD utf8_string StringForQuoteCharacter(char32 character) noexcept
    {
      krys_debug_assert(character);
      StringBuilder builder;
      builder.Append(character);
      return krys::move(builder).ToString();
    }
  }

  RenderQuote::RenderQuote(dom::Document &document, RenderStyle &&style, QuoteType quote) noexcept
      : RenderInline(Type::Quote, document, krys::move(style)), _type(quote)
  {
    krys_debug_assert(IsRenderQuote());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderQuote::~RenderQuote() noexcept = default;

  void RenderQuote::InsertedIntoTree() noexcept
  {
    RenderInline::InsertedIntoTree();
    View().SetHasQuotesNeedingUpdate(true);
  }

  void RenderQuote::WillBeRemovedFromTree() noexcept
  {
    View().SetHasQuotesNeedingUpdate(true);
    RenderInline::WillBeRemovedFromTree();
  }

  void RenderQuote::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderInline::StyleDidChange(diff, oldStyle);
    if (diff >= style::DifferenceResult::Layout)
    {
      _needsTextUpdate = true;
      View().SetHasQuotesNeedingUpdate(true);
    }
  }

  void RenderQuote::UpdateTextRenderer(RenderTreeBuilder &builder) noexcept
  {
    krys_debug_assert(GetDocument().InRenderTreeUpdate());
    auto text = ComputeText();
    if (_text == text)
    {
      return;
    }
    _text = text;
    if (auto *renderText = DynamicDowncast<RenderTextFragment>(LastChild()))
    {
      renderText->SetContentString(_text);
      return;
    }
    builder.Attach(*this, CreateRenderer<RenderTextFragment>(GetDocument(), _text));
  }

  utf8_string RenderQuote::ComputeText() const noexcept
  {
    if (_depth < 0)
    {
      return {};
    }
    bool isOpenQuote = false;
    switch (_type)
    {
      case QuoteType::NoOpenQuote:
      case QuoteType::NoCloseQuote: return {};
      case QuoteType::OpenQuote:    isOpenQuote = true; [[fallthrough]];
      case QuoteType::CloseQuote:
      {
        const auto &quotes = Style().GetQuotes();
        if (!quotes.IsAuto())
        {
          return isOpenQuote ? quotes.OpenQuote(static_cast<unsigned>(_depth))
                             : quotes.CloseQuote(static_cast<unsigned>(_depth));
        }
        if (const auto *quotesForLanguage =
              QuotesForLanguageTag(Style().ComputedLocale().GetPlatform().View()))
        {
          return StringForQuoteCharacter(
            isOpenQuote ? (_depth ? quotesForLanguage->Open2 : quotesForLanguage->Open1)
                        : (_depth ? quotesForLanguage->Close2 : quotesForLanguage->Close1));
        }
        // FIXME: Should the default be the quotes for "en" rather than straight quotes?
        // (According to https://html.spec.whatwg.org/multipage/rendering.html#quotes, the answer is "yes".)
        return _depth ? StringForQuoteCharacter(text::Apostrophe)
                      : StringForQuoteCharacter(text::QuotationMark);
      }
    }
    krys_debug_assert(false);
    return {};
  }

  bool RenderQuote::IsOpen() const noexcept
  {
    switch (_type)
    {
      case QuoteType::OpenQuote:
      case QuoteType::NoOpenQuote:  return true;
      case QuoteType::CloseQuote:
      case QuoteType::NoCloseQuote: return false;
    }
    krys_debug_assert(false);
    return false;
  }

  void RenderQuote::UpdateRenderer(RenderTreeBuilder &builder, RenderQuote *previousQuote) noexcept
  {
    krys_debug_assert(GetDocument().InRenderTreeUpdate());
    int32 depth = -1;
    if (previousQuote)
    {
      depth = previousQuote->_depth;
      if (previousQuote->IsOpen())
      {
        ++depth;
      }
    }

    if (!IsOpen())
    {
      --depth;
    }
    else if (depth < 0)
    {
      depth = 0;
    }

    if (_depth == depth && !_needsTextUpdate)
    {
      return;
    }

    _depth = depth;
    _needsTextUpdate = false;
    UpdateTextRenderer(builder);
  }
}
