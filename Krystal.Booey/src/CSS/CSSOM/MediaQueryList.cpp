#include "Krystal.Booey/CSS/CSSOM/MediaQueryList.hpp"
#include "Krystal.Booey/CSS/CSSOM/MediaQueryListEvent.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"

namespace krys::boo
{
  MediaQueryList::MediaQueryList(css::MediaQueryMatcher &matcher, css::mq::MediaQueryList &&mediaQueries,
                                 bool matches) noexcept
      : _matcher(ShareRefPtr(&matcher)), _mediaQueries(krys::move(mediaQueries)), _matches(matches),
        _evaluatedRound(matcher.EvaluationRound()),
        // Any round but this one: the result hasn't changed yet.
        _changedRound(_evaluatedRound - 1)
  {
    matcher.AddMediaQueryList(*this);
  }

  MediaQueryList::~MediaQueryList() noexcept
  {
    if (_matcher)
    {
      _matcher->RemoveMediaQueryList(*this);
    }
  }

  Ref<MediaQueryList> MediaQueryList::Create(css::MediaQueryMatcher &matcher,
                                             css::mq::MediaQueryList &&mediaQueries, bool matches) noexcept
  {
    return AdoptRef(*new MediaQueryList(matcher, krys::move(mediaQueries), matches));
  }

  CSSOMString MediaQueryList::Media() const noexcept
  {
    StringBuilder builder;
    css::mq::Serialise(builder, _mediaQueries);
    return krys::move(builder).ToString();
  }

  bool MediaQueryList::Matches() noexcept
  {
    if (_matcher && _evaluatedRound != _matcher->EvaluationRound())
    {
      SetMatches(_matcher->Evaluate(_mediaQueries));
    }
    return _matches;
  }

  void MediaQueryList::AddListener(RefPtr<EventListenerCallback> &&callback) noexcept
  {
    // 1. Add an event listener with this and an event listener whose type is change, and callback is
    //    callback. (Adding one does nothing if its callback is null.)
    if (callback)
    {
      AddEventListener(event_names::Change, Ref<EventListenerCallback>(krys::move(callback)), false);
    }
  }

  void MediaQueryList::RemoveListener(RefPtr<EventListenerCallback> &&callback) noexcept
  {
    // 1. If this's event listener list contains an event listener whose type is change, callback is callback,
    //    and capture is false, then remove an event listener with this and that event listener.
    if (callback)
    {
      RemoveEventListener(event_names::Change, Ref<EventListenerCallback>(krys::move(callback)), false);
    }
  }

  void MediaQueryList::Evaluate(css::mq::MediaQueryEvaluator &evaluator) noexcept
  {
    krys_assert(_matcher);
    if (_evaluatedRound != _matcher->EvaluationRound())
    {
      SetMatches(evaluator.Evaluate(_mediaQueries));
    }
    if (_changedRound != _matcher->EvaluationRound())
    {
      return;
    }

    // 1a. If target's matches state has changed since the last time these steps were run, fire an event named
    //     change at target using MediaQueryListEvent, with its isTrusted attribute initialized to true, its
    //     media attribute initialized to target's media, and its matches attribute initialized to target's
    //     matches state.
    MediaQueryListEventInit init;
    init.Media = Media();
    init.Matches = _matches;
    dom::EventDispatcher::FireEvent<MediaQueryListEvent>(event_names::Change, *this, init);
  }

  void MediaQueryList::DetachFromMatcher() noexcept
  {
    _matcher = nullptr;
  }

  void MediaQueryList::SetMatches(bool matches) noexcept
  {
    krys_debug_assert(_matcher);
    _evaluatedRound = _matcher->EvaluationRound();
    if (matches != _matches)
    {
      _matches = matches;
      _changedRound = _evaluatedRound;
    }
  }
}
