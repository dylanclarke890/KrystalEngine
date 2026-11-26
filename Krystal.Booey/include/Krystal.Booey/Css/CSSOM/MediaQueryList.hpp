#pragma once

#include "Krystal.Booey/Css/Queries/MediaQuery.hpp"
#include "Krystal.Booey/Css/Queries/MediaQueryMatcher.hpp"
#include "Krystal.Booey/Css/Types/CSSOMString.hpp"
#include "Krystal.Booey/Dom/Events/EventListener.hpp"
#include "Krystal.Booey/Dom/Events/EventTarget.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::css::mq
{
  class MediaQueryEvaluator;
}

namespace krys::boo
{
  /// @brief What `matchMedia()` returns: whether a media query list matches, and a `change` event whenever
  /// that changes. The document's media query matcher evaluates its lists together, a round at a time.
  /// @see https://drafts.csswg.org/cssom-view/#the-mediaquerylist-interface
  class MediaQueryList final : public EventTarget
  {
  private:
    // The matcher, until its document goes.
    RefPtr<css::MediaQueryMatcher> _matcher;
    const css::mq::MediaQueryList _mediaQueries;
    bool _matches;
    // The matcher's round in which the queries were last evaluated, and the round in which they last changed
    // whether they match.
    uint32 _evaluatedRound;
    uint32 _changedRound;
    // Whether a change awaits a `change` event.

    MediaQueryList(css::MediaQueryMatcher &matcher, css::mq::MediaQueryList &&mediaQueries,
                   bool matches) noexcept;

  public:
    ~MediaQueryList() noexcept override;

    KRYS_NODISCARD static Ref<MediaQueryList>
      Create(css::MediaQueryMatcher &matcher, css::mq::MediaQueryList &&mediaQueries, bool matches) noexcept;

#pragma region MediaQueryList - https://drafts.csswg.org/cssom-view/#the-mediaquerylist-interface

    /// @brief The serialisation of the media queries.
    /// @see https://drafts.csswg.org/cssom-view/#dom-mediaquerylist-media
    KRYS_NODISCARD CSSOMString Media() const noexcept;

    /// @brief Whether the media queries match, evaluated again if the environment may have changed since.
    /// @see https://drafts.csswg.org/cssom-view/#dom-mediaquerylist-matches
    KRYS_NODISCARD bool Matches() noexcept;

    /// @brief The legacy way to add a `change` listener.
    /// @see https://drafts.csswg.org/cssom-view/#dom-mediaquerylist-addlistener
    void AddListener(RefPtr<EventListenerCallback> &&callback) noexcept;

    /// @brief The legacy way to remove a `change` listener.
    /// @see https://drafts.csswg.org/cssom-view/#dom-mediaquerylist-removelistener
    void RemoveListener(RefPtr<EventListenerCallback> &&callback) noexcept;

#pragma endregion

    /// @brief Evaluates the queries with @p evaluator if they haven't been this round, and fires a `change`
    /// event for a change if @p eventMode says to dispatch now; otherwise the event waits for a later round.
    /// @see https://drafts.csswg.org/cssom-view/#evaluate-media-queries-and-report-changes
    void Evaluate(css::mq::MediaQueryEvaluator &evaluator) noexcept;

    /// @brief Stops using the matcher, whose document is going; the list keeps the result it last had.
    void DetachFromMatcher() noexcept;

  private:
    void SetMatches(bool matches) noexcept;
  };
}
