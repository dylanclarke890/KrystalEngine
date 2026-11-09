/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace krys::boo::css
{
  struct LengthPercentageValidator
  {
    KRYS_NODISCARD constexpr static Maybe<LengthPercentageUnit>
      Validate(UnitType unitType, PropertyParserState &state, PropertyParserOptions options) noexcept
    {
      // NOTE: Percentages are handled explicitly by the PercentageValidator, so this only
      // needs to be concerned with the Length units.
      if (auto result = LengthValidator::Validate(unitType, state, options))
      {
        return static_cast<LengthPercentageUnit>(*result);
      }

      return null;
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(LengthPercentageRaw<R, V> raw, PropertyParserOptions) noexcept
    {
      // Values other than 0 and +/-∞ are not supported for <length-percentage> numeric ranges currently.
      return IsValidNonCanonicalizableDimensionValue(raw);
    }

    KRYS_NODISCARD static bool ShouldAcceptUnitlessValue(double value, PropertyParserState &state,
                                                         PropertyParserOptions options) noexcept
    {
      return LengthValidator::ShouldAcceptUnitlessValue(value, state, options);
    }
  };

  template <Range R, typename V>
  struct ConsumerDefinition<LengthPercentage<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<LengthPercentage<R, V>>;
    using DimensionToken = DimensionConsumer<LengthPercentage<R, V>, LengthPercentageValidator>;
    using PercentageToken =
      PercentageConsumer<LengthPercentage<R, V>, LengthPercentageValidator, LengthPercentageUnit::Percentage>;
    using NumberToken = NumberConsumerForUnitlessValues<LengthPercentage<R, V>, LengthPercentageValidator,
                                                        LengthPercentageUnit::Px>;
  };
}