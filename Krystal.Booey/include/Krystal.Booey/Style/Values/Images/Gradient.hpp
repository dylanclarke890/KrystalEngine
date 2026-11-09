/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Values/Images/Gradient.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Position.hpp"

namespace krys::boo
{
  class Gradient;

  namespace style
  {
    // MARK: - Common Types

    using Horizontal = css::Horizontal;
    using Vertical = css::Vertical;

    using RadialGradientExtent = css::RadialGradientExtent;

    // MARK: - Gradient Color Interpolation Definitions.

    using GradientColorInterpolationMethod = css::GradientColorInterpolationMethod;

    template <>
    inline constexpr bool TreatAsNonConverting<GradientColorInterpolationMethod> = true;

    // MARK: - Gradient Color Stop Definitions.

    template <typename Stop>
    using GradientColorStopList = css::CommaSeparatedList<Stop, 2>;

    template <typename C, typename P>
    struct GradientColorStop
    {
      using ColorType = C;
      using PositionType = P;
      using List = GradientColorStopList<GradientColorStop<C, P>>;

      ColorType Color;
      PositionType Position;

      KRYS_NODISCARD bool operator==(const GradientColorStop<C, P> &) const noexcept = default;
    };
    template <typename C, typename P>
    GradientColorStop(C color, P position) -> GradientColorStop<C, P>;

    template <size_t I, typename C, typename P>
    KRYS_NODISCARD const auto &get(const GradientColorStop<C, P> &stop) noexcept
    {
      if constexpr (!I)
      {
        return stop.Color;
      }
      else if constexpr (I == 1)
      {
        return stop.Position;
      }
    }

    using GradientAngularColorStopColor = Markable<Color>;
    using GradientAngularColorStopPosition = Maybe<AnglePercentage<>>;
    using GradientAngularColorStop =
      GradientColorStop<GradientAngularColorStopColor, GradientAngularColorStopPosition>;
    using GradientAngularColorStopList = GradientColorStopList<GradientAngularColorStop>;

    using GradientLinearColorStopColor = Markable<Color>;
    using GradientLinearColorStopPosition = Maybe<LengthPercentage<>>;
    using GradientLinearColorStop =
      GradientColorStop<GradientLinearColorStopColor, GradientLinearColorStopPosition>;
    using GradientLinearColorStopList = GradientColorStopList<GradientLinearColorStop>;

    template <>
    struct ToCSS<GradientAngularColorStop>
    {
      KRYS_NODISCARD css::GradientAngularColorStop operator()(const GradientAngularColorStop &,
                                                              const RenderStyle &) noexcept;
    };
    template <>
    struct ToCSS<GradientLinearColorStop>
    {
      KRYS_NODISCARD css::GradientLinearColorStop operator()(const GradientLinearColorStop &,
                                                             const RenderStyle &) noexcept;
    };

    template <>
    struct ToStyle<css::GradientAngularColorStop>
    {
      KRYS_NODISCARD GradientAngularColorStop operator()(const css::GradientAngularColorStop &,
                                                         const BuilderState &) noexcept;
    };
    template <>
    struct ToStyle<css::GradientLinearColorStop>
    {
      KRYS_NODISCARD GradientLinearColorStop operator()(const css::GradientLinearColorStop &,
                                                        const BuilderState &) noexcept;
    };

    // MARK: - LinearGradient

    struct LinearGradient
    {
      using GradientLineType =
        Variant<Angle<>, Horizontal, Vertical, css::SpaceSeparatedTuple<Horizontal, Vertical>>;

      GradientColorInterpolationMethod ColourInterpolationMethod;
      GradientLineType GradientLine;
      GradientLinearColorStopList Stops;

      KRYS_NODISCARD bool operator==(const LinearGradient &) const noexcept = default;
    };

    template <size_t I>
    KRYS_NODISCARD const auto &get(const LinearGradient &gradient) noexcept
    {
      if constexpr (!I)
      {
        return gradient.ColourInterpolationMethod;
      }
      else if constexpr (I == 1)
      {
        return gradient.GradientLine;
      }
      else if constexpr (I == 2)
      {
        return gradient.Stops;
      }
    }

    DEFINE_TYPE_MAPPING(css::LinearGradient, LinearGradient)

    // MARK: - RadialGradient

    struct RadialGradient
    {
      using Extent = css::RadialGradient::Extent;
      struct Ellipse
      {
        using SizeType = css::SpaceSeparatedArray<LengthPercentage<css::NonNegative>, 2>;
        Variant<SizeType, Extent> Size;
        Maybe<style::Position> Position;
        KRYS_NODISCARD bool operator==(const Ellipse &) const noexcept = default;
      };
      struct Circle
      {
        using Length = style::Length<css::NonNegative>;
        Variant<Length, Extent> Size;
        Maybe<style::Position> Position;
        KRYS_NODISCARD bool operator==(const Circle &) const noexcept = default;
      };
      using GradientBoxType = Variant<Ellipse, Circle>;

      GradientColorInterpolationMethod ColourInterpolationMethod;
      GradientBoxType GradientBox;
      GradientLinearColorStopList Stops;

      KRYS_NODISCARD bool operator==(const RadialGradient &) const noexcept = default;
    };

    template <size_t I>
    KRYS_NODISCARD const auto &get(const RadialGradient::Ellipse &ellipse) noexcept
    {
      if constexpr (!I)
      {
        return ellipse.Size;
      }
      else if constexpr (I == 1)
      {
        return ellipse.Position;
      }
    }

    template <size_t I>
    KRYS_NODISCARD const auto &get(const RadialGradient::Circle &circle) noexcept
    {
      if constexpr (!I)
      {
        return circle.Size;
      }
      else if constexpr (I == 1)
      {
        return circle.Position;
      }
    }

    template <size_t I>
    KRYS_NODISCARD const auto &get(const RadialGradient &gradient) noexcept
    {
      if constexpr (!I)
      {
        return gradient.ColourInterpolationMethod;
      }
      else if constexpr (I == 1)
      {
        return gradient.GradientBox;
      }
      else if constexpr (I == 2)
      {
        return gradient.Stops;
      }
    }

    DEFINE_TYPE_MAPPING(css::RadialGradient::Ellipse, RadialGradient::Ellipse)
    DEFINE_TYPE_MAPPING(css::RadialGradient::Circle, RadialGradient::Circle)
    DEFINE_TYPE_MAPPING(css::RadialGradient, RadialGradient)

    // MARK: - ConicGradient

    struct ConicGradient
    {
      struct GradientBoxType
      {
        Maybe<style::Angle<>> Angle;
        Maybe<style::Position> Position;

        KRYS_NODISCARD bool operator==(const GradientBoxType &) const noexcept = default;
      };

      GradientColorInterpolationMethod ColourInterpolationMethod;
      GradientBoxType GradientBox;
      GradientAngularColorStopList Stops;

      KRYS_NODISCARD bool operator==(const ConicGradient &) const noexcept = default;
    };

    template <size_t I>
    KRYS_NODISCARD const auto &get(const ConicGradient::GradientBoxType &gradientBox) noexcept
    {
      if constexpr (!I)
      {
        return gradientBox.Angle;
      }
      else if constexpr (I == 1)
      {
        return gradientBox.Position;
      }
    }

    template <size_t I>
    KRYS_NODISCARD const auto &get(const ConicGradient &gradient) noexcept
    {
      if constexpr (!I)
      {
        return gradient.ColourInterpolationMethod;
      }
      else if constexpr (I == 1)
      {
        return gradient.GradientBox;
      }
      else if constexpr (I == 2)
      {
        return gradient.Stops;
      }
    }

    DEFINE_TYPE_MAPPING(css::ConicGradient::GradientBoxType, ConicGradient::GradientBoxType)
    DEFINE_TYPE_MAPPING(css::ConicGradient, ConicGradient)

    // MARK: - Gradient (variant)

    using Gradient = Variant<
      // Linear
      css::FunctionNotation<css::ValueId::LinearGradient, LinearGradient>,
      css::FunctionNotation<css::ValueId::RepeatingLinearGradient, LinearGradient>,

      // Radial
      css::FunctionNotation<css::ValueId::RadialGradient, RadialGradient>,
      css::FunctionNotation<css::ValueId::RepeatingRadialGradient, RadialGradient>,

      // Conic
      css::FunctionNotation<css::ValueId::ConicGradient, ConicGradient>,
      css::FunctionNotation<css::ValueId::RepeatingConicGradient, ConicGradient>>;

    // Creates a platform gradient from the style representation.
    KRYS_NODISCARD Ref<boo::Gradient> CreatePlatformGradient(const Gradient &, const FloatSize &,
                                                             const RenderStyle &) noexcept;

    // Returns whether it caching based on the gradient's stops is allowed.
    KRYS_NODISCARD bool StopsAreCacheable(const Gradient &) noexcept;

    // Returns whether the gradient is opaque.
    KRYS_NODISCARD bool IsOpaque(const Gradient &, const RenderStyle &) noexcept;

  }
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::LinearGradient, 3)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::RadialGradient::Ellipse, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::RadialGradient::Circle, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::RadialGradient, 3)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::ConicGradient::GradientBoxType, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::ConicGradient, 3)

template <typename C, typename P>
constexpr bool krys::boo::css::TreatAsTupleLike<krys::boo::style::GradientColorStop<C, P>> = true;

namespace std
{
  template <typename C, typename P>
  class tuple_size<krys::boo::style::GradientColorStop<C, P>> : public std::integral_constant<size_t, 2>
  {
  };
  template <size_t I, typename C, typename P>
  class tuple_element<I, krys::boo::style::GradientColorStop<C, P>>
  {
  public:
    using type =
      decltype(krys::boo::style::get<I>(std::declval<krys::boo::style::GradientColorStop<C, P>>()));
  };

}
