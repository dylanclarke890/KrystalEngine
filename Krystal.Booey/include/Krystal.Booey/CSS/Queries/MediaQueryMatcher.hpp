#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <memory>

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::css::mq
{
  struct MediaQuery;
  using MediaQueryList = SmallList<MediaQuery>;

}
namespace krys::boo::css
{
  class MediaQueryList;

  // MediaQueryMatcher class is responsible for evaluating the queries whenever it
  // is needed and dispatch "change" event on MediaQueryLists if the corresponding
  // query has changed. MediaQueryLists are invoked in the order in which they were added.

  class MediaQueryMatcher final : public RefCounted<MediaQueryMatcher>
  {
  private:
    WeakPtr<dom::Document> _document;
    SmallList<WeakPtr<MediaQueryList>> _mediaQueryLists;

    // This value is incremented at style selector changes.
    // It is used to avoid evaluating queries more then once and to make sure
    // that a media query result change is notified exactly once.
    size_t _evaluationRound {1};

    explicit MediaQueryMatcher(dom::Document &document) noexcept;

  public:
    ~MediaQueryMatcher() noexcept;

    KRYS_NODISCARD static Ref<MediaQueryMatcher> Create(dom::Document &document) noexcept
    {
      return AdoptRef(*new MediaQueryMatcher(document));
    }

    void DocumentDestroyed() noexcept;
    void AddMediaQueryList(MediaQueryList &queryList) noexcept;
    void RemoveMediaQueryList(MediaQueryList &queryList) noexcept;

    RefPtr<MediaQueryList> MatchMedia(const CSSOMString &mediaQuery) noexcept;

    KRYS_NODISCARD size_t EvaluationRound() const noexcept
    {
      return _evaluationRound;
    }

    enum class EventMode : uint8
    {
      Schedule,
      DispatchNow
    };

    KRYS_NODISCARD void EvaluateAll(EventMode mode) noexcept;

    KRYS_NODISCARD bool Evaluate(const mq::MediaQueryList &queryList) noexcept;

    KRYS_NODISCARD CSSOMStringAtom MediaType() const noexcept;

  private:
    KRYS_NODISCARD UniquePtr<RenderStyle> DocumentElementUserAgentStyle() const noexcept;
  };
}
