#include "Krystal.Booey/CSS/Media/MediaList.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSImportRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSMediaRule.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  MediaList::MediaList(CSSStyleSheet *parentSheet) noexcept
      : _parentStyleSheet(WeakPtr<CSSStyleSheet>::WithRef(parentSheet))
  {
  }

  MediaList::MediaList(CSSRule *parentRule) noexcept : _parentRule(WeakPtr<CSSRule>::WithRef(parentRule))
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

  void MediaList::DetachFromParent() noexcept
  {
    _detachedMediaQueries = MediaQueries();
    _parentStyleSheet = nullptr;
    _parentRule = nullptr;
  }

  uint32 MediaList::Length() const noexcept
  {
    return static_cast<uint32>(MediaQueries().size());
  }

  const mq::MediaQueryList &MediaList::MediaQueries() const noexcept
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

  void MediaList::SetMediaQueries(mq::MediaQueryList &&queries) noexcept
  {
    if (RefPtr parentStyleSheet = _parentStyleSheet.lock())
    {
      parentStyleSheet->SetMediaQueries(krys::move(queries));
      parentStyleSheet->DidMutate();
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(_parentRule.get());
    if (auto *rule = DynamicDowncast<CSSImportRule>(_parentRule.get()))
    {
      rule->SetMediaQueries(krys::move(queries));
    }
    if (auto *rule = DynamicDowncast<CSSMediaRule>(_parentRule.get()))
    {
      rule->SetMediaQueries(krys::move(queries));
    }
  }

  CSSOMString MediaList::MediaText() const noexcept
  {
    StringBuilder builder;
    mq::Serialise(builder, MediaQueries());
    return krys::move(builder).ToString();
  }

  void MediaList::SetMediaText(const CSSOMString &value) noexcept
  {
    SetMediaQueries(mq::MediaQueryParser::Parse(value, StrictParserContext()));
  }

  CSSRule *MediaList::ParentRule() const noexcept
  {
    return _parentRule.get();
  }

  CSSStyleSheet *MediaList::ParentStyleSheet() const noexcept
  {
    return _parentStyleSheet.get();
  }

  CSSOMString MediaList::Item(uint32 index) const noexcept
  {
    const auto &queries = MediaQueries();
    if (index < queries.size())
    {
      StringBuilder builder;
      mq::Serialise(builder, queries[index]);
      return krys::move(builder).ToString();
    }
    return {};
  }

  dom::ExceptionOr<void> MediaList::DeleteMedium(const CSSOMString &value) noexcept
  {
    auto valueToRemove = text::ToASCIILower(CSSOMStringView(value));
    auto queries = MediaQueries();
    for (uint32 i = 0; i < queries.size(); ++i)
    {
      if (Item(i) == valueToRemove)
      {
        queries.erase(queries.begin() + i);
        SetMediaQueries(krys::move(queries));
        return {};
      }
    }
    return dom::ExceptionCode::NotFoundError;
  }

  void MediaList::AppendMedium(const CSSOMString &value) noexcept
  {
    if (value.empty())
    {
      return;
    }

    auto newQuery = mq::MediaQueryParser::Parse(value, StrictParserContext());
    auto queries = MediaQueries();
    queries.insert(queries.end(), newQuery.begin(), newQuery.end());
    SetMediaQueries(krys::move(queries));
  }
}
