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

#include "Krystal.Booey/Style/Values/Transitions/Transition.hpp"

namespace krys::boo::style
{
  Transition::Transition() noexcept : _data {TransitionData::Create()}
  {
  }

  Transition::Transition(SingleTransitionProperty &&property) noexcept : Transition {}
  {
    SetProperty(krys::move(property));
  }

  Transition::TransitionData::TransitionData() noexcept
      : _property {Transition::InitialProperty()}, _delay {Transition::InitialDelay()},
        _duration {Transition::InitialDuration()}, _timingFunction {Transition::InitialTimingFunction()},
        _behavior {static_cast<uint8>(Transition::InitialBehavior())}
  {
  }

  Transition::TransitionData::TransitionData(const TransitionData &other) noexcept
      : RefCounted<TransitionData>(), _property {other._property}, _delay {other._delay},
        _duration {other._duration}, _timingFunction {other._timingFunction}, _behavior {other._behavior},
        _propertyState {other._propertyState}, _timingFunctionState {other._timingFunctionState},
        _delayState {other._delayState}, _durationState {other._durationState},
        _behaviorState {other._behaviorState}
  {
  }

  bool Transition::TransitionData::operator==(const TransitionData &other) const noexcept
  {
    return _property == other._property && _delay == other._delay && _duration == other._duration
           && _timingFunction == other._timingFunction && _behavior == other._behavior
           && _propertyState == other._propertyState && _delayState == other._delayState
           && _durationState == other._durationState && _timingFunctionState == other._timingFunctionState
           && _behaviorState == other._behaviorState;
  }
}
