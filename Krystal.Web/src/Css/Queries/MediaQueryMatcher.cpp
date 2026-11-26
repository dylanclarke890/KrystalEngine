#include "Krystal.Web/Css/Queries/MediaQueryMatcher.hpp"
#include "Krystal.Web/Css/CSSOM/MediaQueryList.hpp"
#include "Krystal.Web/Css/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Web/Css/Queries/MediaQueryParser.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Web/Style/ResolvedStyle.hpp"
#include "Krystal.Web/Style/Resolver.hpp"
#include "Krystal.Web/Style/Scope/Scope.hpp"
#include <algorithm>
#include <utility>

namespace krys::web::css
{
  MediaQueryMatcher::MediaQueryMatcher(Document &document) noexcept : _document(CreateWeakPtr(&document))
  {
  }

  MediaQueryMatcher::~MediaQueryMatcher() noexcept = default;

  Ref<MediaQueryMatcher> MediaQueryMatcher::Create(Document &document) noexcept
  {
    return AdoptRef(*new MediaQueryMatcher(document));
  }

  void MediaQueryMatcher::DocumentDestroyed() noexcept
  {
    _document = nullptr;
    for (auto &list : std::exchange(_mediaQueryLists, {}))
    {
      if (list)
      {
        list->DetachFromMatcher();
      }
    }
  }

  void MediaQueryMatcher::AddMediaQueryList(MediaQueryList &list) noexcept
  {
    krys_debug_assert(
      std::ranges::none_of(_mediaQueryLists, [&](const auto &added) { return added.get() == &list; }));
    _mediaQueryLists.push_back(CreateWeakPtr(&list));
  }

  void MediaQueryMatcher::RemoveMediaQueryList(MediaQueryList &list) noexcept
  {
    auto found =
      std::ranges::find_if(_mediaQueryLists, [&](const auto &added) { return added.get() == &list; });
    if (found != _mediaQueryLists.end())
    {
      _mediaQueryLists.erase(found);
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

  void MediaQueryMatcher::EvaluateAll() noexcept
  {
    krys_debug_assert(_document);
    ++_evaluationRound;

    auto style = DocumentElementUserAgentStyle();
    if (!style)
    {
      return;
    }
    mq::MediaQueryEvaluator evaluator {MediaType(), *_document, style.get()};

    // A change listener can add or remove lists, or destroy the document.
    auto lists = _mediaQueryLists;
    for (auto &weakList : lists)
    {
      if (auto list = ShareRefPtr(weakList.get()))
      {
        list->Evaluate(evaluator);
        if (!_document)
        {
          return;
        }
      }
    }
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

  CSSOMStringAtom MediaQueryMatcher::MediaType() const noexcept
  {
    if (!_document || !_document->Frame() || !_document->Frame()->View())
    {
      return null;
    }
    return _document->Frame()->View()->MediaType();
  }

  UniquePtr<RenderStyle> MediaQueryMatcher::DocumentElementUserAgentStyle() const noexcept
  {
    if (!_document || !_document->Frame() || !_document->DocumentElement())
    {
      return nullptr;
    }
    return _document->StyleScope()
      .GetResolver()
      .StyleForElement(*_document->DocumentElement(), {_document->InitialContainingBlockStyle()},
                       RuleMatchingBehavior::MatchOnlyUserAgentRules)
      .Style;
  }
}
