#include "Krystal.Booey/Style/Values/Masking/MaskMode.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"

namespace krys::boo::style
{
  MaskMode CSSValueConversion<MaskMode>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return MaskMode::MatchSource;
    }

    switch (primitiveValue->ValueId())
    {
      case css::ValueId::Alpha:       return MaskMode::Alpha;
      case css::ValueId::Luminance:   return MaskMode::Luminance;
      case css::ValueId::MatchSource: return MaskMode::MatchSource;
      case css::ValueId::Auto: // -webkit-mask-source-type
        return MaskMode::MatchSource;
      default: state.SetCurrentPropertyInvalidAtComputedValueTime(); return MaskMode::MatchSource;
    }
  }
}
