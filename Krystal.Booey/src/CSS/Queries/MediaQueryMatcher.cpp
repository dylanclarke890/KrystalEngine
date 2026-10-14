#include "Krystal.Booey/CSS/Queries/MediaQueryMatcher.hpp"
#include "Krystal.Booey/CSS/Media/MediaQueryList.hpp"
#include "Krystal.Booey/CSS/Queries/MediaEnvironment.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
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
    if (!_document)
    {
      return CSSOMStringAtom::Null();
    }
    auto *environment = mq::MediaEnvironmentFor(*_document);
    if (!environment)
    {
      return CSSOMStringAtom::Null();
    }
    return environment->MediaType;
  }

  UniquePtr<RenderStyle> MediaQueryMatcher::DocumentElementUserAgentStyle() const noexcept
  {
    if (!_document || !mq::MediaEnvironmentFor(*_document))
    {
      return nullptr;
    }

    if (!_document->DocumentElement())
    {
      return nullptr;
    }

    // FIXME: Resolve the document element's style from the user agent rules once there's a style resolver.
    // Media queries only read its font, which until then is the initial font.
    return CreateUnique<RenderStyle>();
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
