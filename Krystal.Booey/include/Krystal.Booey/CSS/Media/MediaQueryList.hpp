#pragma once

#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryMatcher.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  namespace mq
  {
    class MediaQueryEvaluator;
  }

  /// @brief The object `matchMedia()` returns: the current result of a media query list, and `change` events
  /// whenever it changes.
  /// @see https://drafts.csswg.org/cssom-view/#the-mediaquerylist-interface
  class MediaQueryList final : public dom::EventTarget
  {
  private:
    RefPtr<MediaQueryMatcher> _matcher;
    const mq::MediaQueryList _mediaQueries;
    // Indicates if the query has been evaluated after the last style selector change.
    uint32 _evaluationRound;
    // Used to know if the query has changed in the last style selector change.
    uint32 _changeRound;
    bool _matches;
    bool _needsNotification {false};

    MediaQueryList(MediaQueryMatcher &matcher, mq::MediaQueryList &&mediaQueries, bool matches) noexcept;

  public:
    ~MediaQueryList() noexcept override;

    KRYS_NODISCARD static Ref<MediaQueryList>
      Create(MediaQueryMatcher &matcher, mq::MediaQueryList &&mediaQueries, bool matches) noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-mediaquerylist-media
    KRYS_NODISCARD CSSOMString Media() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-mediaquerylist-matches
    KRYS_NODISCARD bool Matches() noexcept;

    /// @note Legacy.
    /// @see https://drafts.csswg.org/cssom-view/#dom-mediaquerylist-addlistener
    void AddListener(RefPtr<dom::EventListenerCallback> &&listener) noexcept;

    /// @note Legacy.
    /// @see https://drafts.csswg.org/cssom-view/#dom-mediaquerylist-removelistener
    void RemoveListener(RefPtr<dom::EventListenerCallback> &&listener) noexcept;

    void Evaluate(mq::MediaQueryEvaluator &evaluator, MediaQueryMatcher::EventMode eventMode) noexcept;

    void DetachFromMatcher() noexcept;

  private:
    void SetMatches(bool newValue) noexcept;
  };
}
