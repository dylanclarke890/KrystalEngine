#include "Krystal.Booey/CSS/Queries/MediaQueryMatcher.hpp"
#include "Krystal.Booey/CSS/Media/MediaQueryList.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/ResolvedStyle.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Core/Assert.hpp"
#include <algorithm>
#include <utility>

namespace krys::boo::css
{
  MediaQueryMatcher::MediaQueryMatcher(dom::Document &document) noexcept
      : _document(WeakPtr<dom::Document>::WithRef(document))
  {
  }

  MediaQueryMatcher::~MediaQueryMatcher() noexcept = default;

  Ref<MediaQueryMatcher> MediaQueryMatcher::Create(dom::Document &document) noexcept
  {
    return AdoptRef(*new MediaQueryMatcher(document));
  }

  void MediaQueryMatcher::DocumentDestroyed() noexcept
  {
    _document = nullptr;
    auto mediaQueryLists = std::exchange(_mediaQueryLists, {});
    for (auto &mediaQueryList : mediaQueryLists)
    {
      if (mediaQueryList)
      {
        mediaQueryList->DetachFromMatcher();
      }
    }
  }

  CSSOMStringAtom MediaQueryMatcher::MediaType() const noexcept
  {
    if (!_document || !_document->Frame() || !_document->Frame()->View())
    {
      return CSSOMStringAtom::Null();
    }

    return _document->Frame()->View()->MediaType();
  }

  UniquePtr<RenderStyle> MediaQueryMatcher::DocumentElementUserAgentStyle() const noexcept
  {
    if (!_document || !_document->Frame())
    {
      return nullptr;
    }

    if (!_document->DocumentElement())
    {
      return nullptr;
    }

    return _document->StyleScope()
      .GetResolver()
      .StyleForElement(*_document->DocumentElement(), {_document->InitialContainingBlockStyle()},
                       RuleMatchingBehavior::MatchOnlyUserAgentRules)
      .Style;
  }

  bool MediaQueryMatcher::Evaluate(const mq::MediaQueryList &queries) const noexcept
  {
    auto style = DocumentElementUserAgentStyle();
    if (!style)
    {
      return false;
    }
    return mq::MediaQueryEvaluator {MediaType(), *_document, style.get()}.Evaluate(queries);
  }

  void MediaQueryMatcher::AddMediaQueryList(MediaQueryList &list) noexcept
  {
    krys_debug_assert(std::ranges::none_of(_mediaQueryLists, [&](const WeakPtr<MediaQueryList> &existing)
                                           { return existing.get() == &list; }));
    _mediaQueryLists.push_back(WeakPtr<MediaQueryList>::WithRef(list));
  }

  void MediaQueryMatcher::RemoveMediaQueryList(MediaQueryList &list) noexcept
  {
    auto it = std::ranges::find_if(_mediaQueryLists, [&](const WeakPtr<MediaQueryList> &existing)
                                   { return existing.get() == &list; });
    if (it != _mediaQueryLists.end())
    {
      _mediaQueryLists.erase(it);
    }
  }

  RefPtr<MediaQueryList> MediaQueryMatcher::MatchMedia(CSSOMStringView query) noexcept
  {
    if (!_document)
    {
      return nullptr;
    }

    auto queries = mq::MediaQueryParser::Parse(query, _document->CssParserContext());
    bool matches = Evaluate(queries);
    return MediaQueryList::Create(*this, krys::move(queries), matches);
  }

  void MediaQueryMatcher::EvaluateAll(EventMode eventMode) noexcept
  {
    krys_debug_assert(_document);

    ++_evaluationRound;

    auto style = DocumentElementUserAgentStyle();
    if (!style)
    {
      return;
    }

    mq::MediaQueryEvaluator evaluator {MediaType(), *_document, style.get()};

    auto mediaQueryLists = _mediaQueryLists;
    for (auto &list : mediaQueryLists)
    {
      if (auto protectedList = ShareRefPtr(list.get()))
      {
        protectedList->Evaluate(evaluator, eventMode);
        if (!_document)
        {
          break;
        }
      }
    }
  }
}
