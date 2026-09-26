#pragma once

#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryMatcher.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"

namespace krys::boo::css
{
  namespace mq
  {
    class MediaQueryEvaluator;
  }

  // MediaQueryList interface is specified at
  // https://drafts.csswg.org/cssom-view/#the-mediaquerylist-interface The objects of this class are returned
  // by window.matchMedia. They may be used to retrieve the current value of the given media query and to
  // add/remove listeners that will be called whenever the value of the query changes.

  class MediaQueryList final : public RefCounted<MediaQueryList>, public dom::EventTarget
  {
  private:
    RefPtr<MediaQueryMatcher> _matcher;
    const mq::MediaQueryList _mediaQueries;
    const mq::MediaQueryDynamicDependency _dynamicDependencies;
    // Indicates if the query has been evaluated after the last style selector change.
    size_t _evaluationRound;
    // Used to know if the query has changed in the last style selector change.
    size_t _changeRound;
    bool _matches;
    bool _hasChangeEventListener {false};
    bool _needsNotification {false};

    MediaQueryList(dom::Document &, MediaQueryMatcher &matcher, mq::MediaQueryList &&queries,
                   bool matches) noexcept;

  public:
    ~MediaQueryList() noexcept;

    KRYS_NODISCARD static Ref<MediaQueryList> Create(dom::Document &document, MediaQueryMatcher &matcher,
                                                     mq::MediaQueryList &&queries, bool matches) noexcept;

    KRYS_NODISCARD CSSOMString Media() const noexcept;

    KRYS_NODISCARD bool Matches() noexcept;

    void AddListener(RefPtr<dom::EventListener> &&listener) noexcept;

    void RemoveListener(RefPtr<dom::EventListener> &&listener) noexcept;

    void Evaluate(mq::MediaQueryEvaluator &evaluator, MediaQueryMatcher::EventMode eventMode) noexcept;

    void DetachFromMatcher() noexcept;

  private:
    void SetMatches(bool matches) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::MediaQueryList)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::EventTarget &target) noexcept
  {
    // TODO(implement): type checking for MediaQueryList against EventTarget.
    return false;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
