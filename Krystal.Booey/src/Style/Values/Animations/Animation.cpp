/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
