/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesCanonicalization.hpp"

namespace krys::boo::css
{
  struct AngleValidator
  {
    KRYS_NODISCARD constexpr static Maybe<AngleUnit> Validate(UnitType unitType, PropertyParserState &,
                                                              PropertyParserOptions) noexcept
    {
      return UnitTraits<AngleUnit>::Validate(unitType);
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(AngleRaw<R, V> raw, PropertyParserOptions) noexcept
    {
      return IsValidDimensionValue(raw,
                                   [&]
                                   {
                                     auto canonicalValue = Canonicalize(raw);
                                     return canonicalValue >= raw.Range.Min
                                            && canonicalValue <= raw.Range.Max;
                                   });
    }

    KRYS_NODISCARD static bool ShouldAcceptUnitlessValue(double value, PropertyParserState &state,
                                                         PropertyParserOptions options) noexcept
    {
      if (!value && options.UnitlessZeroAngle)
      {
        return true;
      }

      auto mode = options.OverrideParserMode.value_or(state.Context.Mode);
      if (IsUnitlessValueParsingForcedForMode(mode))
      {
        return true;
      }

      return mode == ParserMode::HTMLQuirks && Property::AcceptsQuirkyAngle(state.CurrentProperty);
    }
  };

  template <Range R, typename V>
  struct ConsumerDefinition<Angle<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<Angle<R, V>>;
    using DimensionToken = DimensionConsumer<Angle<R, V>, AngleValidator>;
    using NumberToken = NumberConsumerForUnitlessValues<Angle<R, V>, AngleValidator, AngleUnit::Deg>;
  };
}