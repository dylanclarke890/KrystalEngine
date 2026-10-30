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
