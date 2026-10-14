#include "Krystal.Booey/CSS/Media/MediaQueryList.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Media/MediaQueryListEvent.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css
{
  MediaQueryList::MediaQueryList(MediaQueryMatcher &matcher, mq::MediaQueryList &&mediaQueries,
                                 bool matches) noexcept
      : _matcher(ShareRefPtr(&matcher)), _mediaQueries(krys::move(mediaQueries)),
        _evaluationRound(matcher.EvaluationRound()),
        _changeRound(_evaluationRound - 1), // Any value that is not the same as _evaluationRound would do.
        _matches(matches)
  {
    matcher.AddMediaQueryList(*this);
  }

  Ref<MediaQueryList> MediaQueryList::Create(MediaQueryMatcher &matcher, mq::MediaQueryList &&mediaQueries,
                                             bool matches) noexcept
  {
    return AdoptRef(*new MediaQueryList(matcher, krys::move(mediaQueries), matches));
  }

  MediaQueryList::~MediaQueryList() noexcept
  {
    if (_matcher)
    {
      _matcher->RemoveMediaQueryList(*this);
    }
  }

  void MediaQueryList::DetachFromMatcher() noexcept
  {
    _matcher = nullptr;
  }

  CSSOMString MediaQueryList::Media() const noexcept
  {
    CSSOMString builder;
    mq::Serialise(builder, _mediaQueries);
    return builder;
  }

  void MediaQueryList::AddListener(RefPtr<dom::EventListenerCallback> &&listener) noexcept
  {
    if (!listener)
    {
      return;
    }

    Ref<dom::EventListenerCallback> callback = krys::move(listener);
    AddEventListener(CSSOMStringAtom(EventNames::Change), krys::move(callback), false);
  }

  void MediaQueryList::RemoveListener(RefPtr<dom::EventListenerCallback> &&listener) noexcept
  {
    if (!listener)
    {
      return;
    }

    Ref<dom::EventListenerCallback> callback = krys::move(listener);
    RemoveEventListener(CSSOMStringAtom(EventNames::Change), krys::move(callback), false);
  }

  void MediaQueryList::Evaluate(mq::MediaQueryEvaluator &evaluator,
                                MediaQueryMatcher::EventMode eventMode) noexcept
  {
    krys_assert(_matcher);
    if (_evaluationRound != _matcher->EvaluationRound())
    {
      SetMatches(evaluator.Evaluate(_mediaQueries));
    }

    _needsNotification = _changeRound == _matcher->EvaluationRound() || _needsNotification;
    if (!_needsNotification || eventMode == MediaQueryMatcher::EventMode::Schedule)
    {
      return;
    }
    krys_debug_assert(eventMode == MediaQueryMatcher::EventMode::DispatchNow);

    // https://drafts.csswg.org/cssom-view/#evaluate-media-queries-and-report-changes
    dom::EventDispatcher::FireEvent<MediaQueryListEvent>(CSSOMStringAtom(EventNames::Change), *this,
                                                         [&](MediaQueryListEvent &event)
                                                         {
                                                           event._media = Media();
                                                           event._matches = Matches();
                                                         });
    _needsNotification = false;
  }

  void MediaQueryList::SetMatches(bool newValue) noexcept
  {
    krys_debug_assert(_matcher);
    _evaluationRound = _matcher->EvaluationRound();

    if (newValue == _matches)
    {
      return;
    }

    _matches = newValue;
    _changeRound = _evaluationRound;
  }

  bool MediaQueryList::Matches() noexcept
  {
    if (!_matcher)
    {
      return _matches;
    }

    // FIXME: Re-evaluate viewport-dependent queries after updating the owner document's layout once documents
    // can be nested in frames.

    if (_evaluationRound != _matcher->EvaluationRound())
    {
      SetMatches(_matcher->Evaluate(_mediaQueries));
    }

    return _matches;
  }
}
