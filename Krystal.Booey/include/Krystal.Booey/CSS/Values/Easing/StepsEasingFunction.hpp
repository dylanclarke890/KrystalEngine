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

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  // <steps-easing-function> = steps( <integer>, <steps-easing-function-position>? )
  // <steps-easing-function-position> = jump-start | jump-end | jump-none | jump-both | start | end
  //
  // with range constraints, this is:
  //
  // <steps-easing-function> = steps( <integer [1,∞]>, jump-start )
  //                         | steps( <integer [1,∞]>, jump-end )
  //                         | steps( <integer [1,∞]>, jump-both )
  //                         | steps( <integer [1,∞]>, start )
  //                         | steps( <integer [1,∞]>, end )
  //                         | steps( <integer [2,∞]>, jump-none )
  // https://drafts.csswg.org/css-easing-2/#funcdef-steps
  struct StepsEasingParameters
  {
    enum class ShouldSerialiseKeyword : bool
    {
      No,
      Yes
    };

    template <typename T, typename TKeyword, auto TShouldSerialiseKeyword = ShouldSerialiseKeyword::Yes>
    struct Kind
    {
      constexpr static TKeyword Keyword = TKeyword {};

      T Steps;

      KRYS_NODISCARD constexpr bool operator==(const Kind &) const noexcept = default;
    };

    using JumpStart = Kind<Integer<Range {1, Range::Inf}>, keywords::JumpStart>;
    using JumpEnd = Kind<Integer<Range {1, Range::Inf}>, keywords::JumpEnd, ShouldSerialiseKeyword::No>;
    using JumpBoth = Kind<Integer<Range {1, Range::Inf}>, keywords::JumpBoth>;
    using Start = Kind<Integer<Range {1, Range::Inf}>, keywords::Start>;
    using End = Kind<Integer<Range {1, Range::Inf}>, keywords::End, ShouldSerialiseKeyword::No>;
    using JumpNone = Kind<Integer<Range {2, Range::Inf}>, keywords::JumpNone>;

    Variant<JumpStart, JumpEnd, JumpBoth, Start, End, JumpNone> Value;

    KRYS_NODISCARD constexpr bool operator==(const StepsEasingParameters &) const noexcept = default;
  };

  using StepsEasingFunction = FunctionNotation<ValueId::Steps, StepsEasingParameters>;

  DEFINE_TYPE_WRAPPER_GET(StepsEasingParameters, Value)

  template <size_t I, typename T, typename TKeyword, auto TShouldSerialiseKeyword>
  KRYS_NODISCARD constexpr const auto &
    get(const StepsEasingParameters::Kind<T, TKeyword, TShouldSerialiseKeyword> &value) noexcept
  {
    return value.Steps;
  }

  template <typename T, typename TKeyword, auto TShouldSerialiseKeyword>
  struct Serialiser<StepsEasingParameters::Kind<T, TKeyword, TShouldSerialiseKeyword>>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const StepsEasingParameters::Kind<T, TKeyword, TShouldSerialiseKeyword> &value) noexcept
    {
      Serialise(builder, context, value.Steps);
      if constexpr (TShouldSerialiseKeyword == StepsEasingParameters::ShouldSerialiseKeyword::Yes)
      {
        builder.Append(u8", ");
        Serialise(builder, context, value.Keyword);
      }
    }
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::StepsEasingParameters, 1)

namespace std
{
  template <typename T, typename TKeyword, auto TShouldSerialiseKeyword>
  class tuple_size<krys::boo::css::StepsEasingParameters::Kind<T, TKeyword, TShouldSerialiseKeyword>>
      : public std::integral_constant<size_t, 1>
  {
  };

  template <size_t I, typename T, typename TKeyword, auto TShouldSerialiseKeyword>
  class tuple_element<I, krys::boo::css::StepsEasingParameters::Kind<T, TKeyword, TShouldSerialiseKeyword>>
  {
  public:
    using type = T;
  };
}

template <typename T, typename TKeyword, auto TShouldSerialiseKeyword>
constexpr bool krys::boo::css::TreatAsTupleLike<
  krys::boo::css::StepsEasingParameters::Kind<T, TKeyword, TShouldSerialiseKeyword>> = true;
