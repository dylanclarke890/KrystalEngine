#include "Krystal.Booey/Style/Values/Animations/Animation.hpp"

namespace krys::boo::style
{
  Animation::Animation() noexcept : _data {AnimationData::Create()}
  {
  }

  Animation::Animation(SingleAnimationName &&name) noexcept : Animation {}
  {
    SetName(krys::move(name));
  }

  Animation::AnimationData::AnimationData() noexcept
      : _name {Animation::InitialName()}, _delay {Animation::InitialDelay()},
        _duration {Animation::InitialDuration()}, _iterationCount {Animation::InitialIterationCount()},
        _timeline {Animation::InitialTimeline()}, _timingFunction {Animation::InitialTimingFunction()},
        _defaultTimingFunctionForKeyframes {std::nullopt}, _rangeStart {Animation::InitialRangeStart()},
        _rangeEnd {Animation::InitialRangeEnd()},
        _direction {static_cast<uint8>(Animation::InitialDirection())},
        _fillMode {static_cast<uint8>(Animation::InitialFillMode())},
        _playState {static_cast<uint8>(Animation::InitialPlayState())},
        _compositeOperation {static_cast<uint8>(Animation::InitialCompositeOperation())}
  {
  }

  Animation::AnimationData::AnimationData(const AnimationData &other) noexcept
      : RefCounted<AnimationData>(), _name {other._name}, _delay {other._delay}, _duration {other._duration},
        _iterationCount {other._iterationCount}, _timeline {other._timeline},
        _timingFunction {other._timingFunction},
        _defaultTimingFunctionForKeyframes {other._defaultTimingFunctionForKeyframes},
        _rangeStart {other._rangeStart}, _rangeEnd {other._rangeEnd}, _direction {other._direction},
        _fillMode {other._fillMode}, _playState {other._playState},
        _compositeOperation {other._compositeOperation}, _nameState {other._nameState},
        _timelineState {other._timelineState}, _timingFunctionState {other._timingFunctionState},
        _rangeStartState {other._rangeStartState}, _rangeEndState {other._rangeEndState},
        _delayState {other._delayState}, _durationState {other._durationState},
        _iterationCountState {other._iterationCountState}, _directionState {other._directionState},
        _fillModeState {other._fillModeState}, _playStateState {other._playStateState},
        _compositeOperationState {other._compositeOperationState}
  {
  }

  bool Animation::AnimationData::operator==(const AnimationData &other) const noexcept
  {
    return _name == other._name && _delay == other._delay && _direction == other._direction
           && _duration == other._duration && _fillMode == other._fillMode
           && _iterationCount == other._iterationCount && _playState == other._playState
           && _timeline == other._timeline && _timingFunction == other._timingFunction
           && _compositeOperation == other._compositeOperation && _rangeStart == other._rangeStart
           && _rangeEnd == other._rangeEnd && _nameState == other._nameState
           && _delayState == other._delayState && _directionState == other._directionState
           && _durationState == other._durationState && _fillModeState == other._fillModeState
           && _playStateState == other._playStateState && _iterationCountState == other._iterationCountState
           && _timelineState == other._timelineState && _timingFunctionState == other._timingFunctionState
           && _compositeOperationState == other._compositeOperationState
           && _rangeStartState == other._rangeStartState && _rangeEndState == other._rangeEndState;
  }
}
