#include "Krystal.Booey/CSS/CSSOM/MediaList.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSImportRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSMediaRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    /// @see https://drafts.csswg.org/cssom/#parse-a-media-query-list
    KRYS_NODISCARD css::mq::MediaQueryList ParseMediaQueryList(const CSSOMString &text) noexcept
    {
      return css::mq::MediaQueryParser::Parse(text, css::StrictParserContext());
    }

    /// @brief The single query @p text is, or null if it's none or several.
    /// @see https://drafts.csswg.org/cssom/#parse-a-media-query
    KRYS_NODISCARD Maybe<css::mq::MediaQuery> ParseMediaQuery(const CSSOMString &text) noexcept
    {
      auto queries = ParseMediaQueryList(text);
      if (queries.size() != 1)
      {
        return null;
      }
      return krys::move(queries.front());
    }

    /// @see https://drafts.csswg.org/cssom/#serialize-a-media-query
    KRYS_NODISCARD CSSOMString SerialiseMediaQuery(const css::mq::MediaQuery &query) noexcept
    {
      StringBuilder builder;
      css::mq::Serialise(builder, query);
      return krys::move(builder).ToString();
    }

    /// @brief Whether @p a and @p b serialise the same.
    /// @see https://drafts.csswg.org/cssom/#compare-media-queries
    KRYS_NODISCARD bool AreSameMediaQuery(const css::mq::MediaQuery &a, const css::mq::MediaQuery &b) noexcept
    {
      return SerialiseMediaQuery(a) == SerialiseMediaQuery(b);
    }
  }

  MediaList::MediaList(CSSStyleSheet *parentSheet) noexcept : _parentStyleSheet(CreateWeakPtr(parentSheet))
  {
  }

  MediaList::MediaList(CSSRule *parentRule) noexcept : _parentRule(CreateWeakPtr(parentRule))
  {
  }

  MediaList::~MediaList() noexcept = default;

  Ref<MediaList> MediaList::Create(CSSStyleSheet *parentSheet) noexcept
  {
    return AdoptRef(*new MediaList(parentSheet));
  }

  Ref<MediaList> MediaList::Create(CSSRule *parentRule) noexcept
  {
    return AdoptRef(*new MediaList(parentRule));
  }

  CSSOMString MediaList::MediaText() const noexcept
  {
    // https://drafts.csswg.org/cssom/#serialize-a-media-query-list
    StringBuilder builder;
    css::mq::Serialise(builder, MediaQueries());
    return krys::move(builder).ToString();
  }

  void MediaList::SetMediaText(const CSSOMString &text) noexcept
  {
    // 1. Empty the collection of media queries.
    // 2. If the given value is the empty string, then return.
    // 3. Append all the media queries as a result of parsing the given value to the collection of media
    //    queries.
    SetMediaQueries(text.empty() ? css::mq::MediaQueryList {} : ParseMediaQueryList(text));
  }

  CSSOMString MediaList::Item(uint32 index) const noexcept
  {
    const auto &queries = MediaQueries();
    return index < queries.size() ? SerialiseMediaQuery(queries[index]) : CSSOMString {};
  }

  uint32 MediaList::Length() const noexcept
  {
    return static_cast<uint32>(MediaQueries().size());
  }

  void MediaList::AppendMedium(const CSSOMString &medium) noexcept
  {
    // 1. Let m be the result of parsing the given value.
    auto m = ParseMediaQuery(medium);

    // 2. If m is null, then return.
    if (!m)
    {
      return;
    }

    // 3. If comparing m with any of the media queries in the collection of media queries returns true, then
    //    return.
    auto queries = MediaQueries();
    if (std::ranges::any_of(queries, [&](const auto &query) { return AreSameMediaQuery(*m, query); }))
    {
      return;
    }

    // 4. Append m to the collection of media queries.
    queries.push_back(krys::move(*m));
    SetMediaQueries(krys::move(queries));
  }

  ExceptionOr<void> MediaList::DeleteMedium(const CSSOMString &medium) noexcept
  {
    // 1. Let m be the result of parsing the given value.
    auto m = ParseMediaQuery(medium);

    // 2. If m is null, then return.
    if (!m)
    {
      return {};
    }

    // 3. Remove any media query from the collection of media queries for which comparing the media query with
    //    m returns true. If nothing was removed, then throw a NotFoundError exception.
    auto queries = MediaQueries();
    auto removed = std::erase_if(queries, [&](const auto &query) { return AreSameMediaQuery(query, *m); });
    if (!removed)
    {
      return ExceptionCode::NotFoundError;
    }
    SetMediaQueries(krys::move(queries));
    return {};
  }

  CSSRule *MediaList::ParentRule() const noexcept
  {
    return _parentRule.get();
  }

  CSSStyleSheet *MediaList::ParentStyleSheet() const noexcept
  {
    return _parentStyleSheet.get();
  }

  const css::mq::MediaQueryList &MediaList::MediaQueries() const noexcept
  {
    if (_detachedMediaQueries)
    {
      return *_detachedMediaQueries;
    }
    if (auto *rule = DynamicDowncast<CSSImportRule>(_parentRule.get()))
    {
      return rule->MediaQueries();
    }
    if (auto *rule = DynamicDowncast<CSSMediaRule>(_parentRule.get()))
    {
      return rule->MediaQueries();
    }
    return _parentStyleSheet->MediaQueries();
  }

  void MediaList::DetachFromParent() noexcept
  {
    _detachedMediaQueries = MediaQueries();
    _parentStyleSheet = nullptr;
    _parentRule = nullptr;
  }

  void MediaList::SetMediaQueries(css::mq::MediaQueryList queries) noexcept
  {
    if (_detachedMediaQueries)
    {
      _detachedMediaQueries = krys::move(queries);
      return;
    }
    if (RefPtr sheet = _parentStyleSheet.lock())
    {
      sheet->SetMediaQueries(krys::move(queries));
      sheet->DidMutate();
      return;
    }

    // A rule's queries are part of its sheet's contents, so changing them is a change to the sheet's rules.
    CSSStyleSheet::RuleMutationScope mutationScope(_parentRule.get());
    if (auto *importRule = DynamicDowncast<CSSImportRule>(_parentRule.get()))
    {
      importRule->SetMediaQueries(krys::move(queries));
    }
    else if (auto *mediaRule = DynamicDowncast<CSSMediaRule>(_parentRule.get()))
    {
      mediaRule->SetMediaQueries(krys::move(queries));
    }
  }
}
