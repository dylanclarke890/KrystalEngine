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

    using DeprecatedGradientPosition = css::SpaceSeparatedArray<NumberOrPercentage<>, 2>;

    using Horizontal = css::Horizontal;
    using Vertical = css::Vertical;

    using RadialGradientExtent = css::RadialGradientExtent;
    using PrefixedRadialGradientExtent = css::PrefixedRadialGradientExtent;

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

    using GradientDeprecatedColorStopColor = Color;
    using GradientDeprecatedColorStopPosition = NumberOrPercentageResolvedToNumber<>;
    using GradientDeprecatedColorStop =
      GradientColorStop<GradientDeprecatedColorStopColor, GradientDeprecatedColorStopPosition>;
    using GradientDeprecatedColorStopList = GradientColorStopList<GradientDeprecatedColorStop>;

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
    struct ToCSS<GradientDeprecatedColorStop>
    {
      KRYS_NODISCARD css::GradientDeprecatedColorStop operator()(const GradientDeprecatedColorStop &,
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
    template <>
    struct ToStyle<css::GradientDeprecatedColorStop>
    {
      KRYS_NODISCARD GradientDeprecatedColorStop operator()(const css::GradientDeprecatedColorStop &,
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

    // MARK: - PrefixedLinearGradient

    struct PrefixedLinearGradient
    {
      using GradientLineType =
        Variant<Angle<>, Horizontal, Vertical, css::SpaceSeparatedTuple<Horizontal, Vertical>>;

      GradientColorInterpolationMethod ColourInterpolationMethod;
      GradientLineType GradientLine;
      GradientLinearColorStopList Stops;

      KRYS_NODISCARD bool operator==(const PrefixedLinearGradient &) const noexcept = default;
    };

    template <size_t I>
    KRYS_NODISCARD const auto &get(const PrefixedLinearGradient &gradient) noexcept
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

    DEFINE_TYPE_MAPPING(css::PrefixedLinearGradient, PrefixedLinearGradient)

    // MARK: - DeprecatedLinearGradient

    struct DeprecatedLinearGradient
    {
      using GradientLineType = css::CommaSeparatedArray<DeprecatedGradientPosition, 2>;

      GradientColorInterpolationMethod ColourInterpolationMethod;
      GradientLineType GradientLine;
      GradientDeprecatedColorStopList Stops;

      KRYS_NODISCARD bool operator==(const DeprecatedLinearGradient &) const noexcept = default;
    };

    template <size_t I>
    KRYS_NODISCARD const auto &get(const DeprecatedLinearGradient &gradient) noexcept
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

    DEFINE_TYPE_MAPPING(css::DeprecatedLinearGradient, DeprecatedLinearGradient)

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

    // MARK: - PrefixedRadialGradient

    struct PrefixedRadialGradient
    {
      using Extent = css::PrefixedRadialGradient::Extent;
      struct Ellipse
      {
        using SizeType = css::SpaceSeparatedArray<LengthPercentage<css::NonNegative>, 2>;
        Maybe<Variant<SizeType, Extent>> Size;
        Maybe<style::Position> Position;
        KRYS_NODISCARD bool operator==(const Ellipse &) const noexcept = default;
      };
      struct Circle
      {
        Maybe<Extent> Size;
        Maybe<style::Position> Position;
        KRYS_NODISCARD bool operator==(const Circle &) const noexcept = default;
      };
      using GradientBoxType = Variant<Ellipse, Circle>;

      GradientColorInterpolationMethod ColourInterpolationMethod;
      GradientBoxType GradientBox;
      GradientLinearColorStopList Stops;

      KRYS_NODISCARD bool operator==(const PrefixedRadialGradient &) const noexcept = default;
    };

    template <size_t I>
    KRYS_NODISCARD const auto &get(const PrefixedRadialGradient::Ellipse &ellipse) noexcept
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
    KRYS_NODISCARD const auto &get(const PrefixedRadialGradient::Circle &circle) noexcept
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
    KRYS_NODISCARD const auto &get(const PrefixedRadialGradient &gradient) noexcept
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

    DEFINE_TYPE_MAPPING(css::PrefixedRadialGradient::Ellipse, PrefixedRadialGradient::Ellipse)
    DEFINE_TYPE_MAPPING(css::PrefixedRadialGradient::Circle, PrefixedRadialGradient::Circle)
    DEFINE_TYPE_MAPPING(css::PrefixedRadialGradient, PrefixedRadialGradient)

    // MARK: - DeprecatedRadialGradient

    struct DeprecatedRadialGradient
    {
      struct GradientBoxType
      {
        DeprecatedGradientPosition First;
        Number<css::NonNegative> FirstRadius;
        DeprecatedGradientPosition Second;
        Number<css::NonNegative> SecondRadius;

        KRYS_NODISCARD bool operator==(const GradientBoxType &) const noexcept = default;
      };

      GradientColorInterpolationMethod ColourInterpolationMethod;
      GradientBoxType GradientBox;
      GradientDeprecatedColorStopList Stops;

      KRYS_NODISCARD bool operator==(const DeprecatedRadialGradient &) const noexcept = default;
    };

    template <size_t I>
    KRYS_NODISCARD const auto &get(const DeprecatedRadialGradient::GradientBoxType &gradientBox) noexcept
    {
      if constexpr (!I)
      {
        return gradientBox.First;
      }
      else if constexpr (I == 1)
      {
        return gradientBox.FirstRadius;
      }
      else if constexpr (I == 2)
      {
        return gradientBox.Second;
      }
      else if constexpr (I == 3)
      {
        return gradientBox.SecondRadius;
      }
    }

    template <size_t I>
    KRYS_NODISCARD const auto &get(const DeprecatedRadialGradient &gradient) noexcept
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

    DEFINE_TYPE_MAPPING(css::DeprecatedRadialGradient::GradientBoxType,
                        DeprecatedRadialGradient::GradientBoxType)
    DEFINE_TYPE_MAPPING(css::DeprecatedRadialGradient, DeprecatedRadialGradient)

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
      css::FunctionNotation<css::ValueId::WebkitLinearGradient, PrefixedLinearGradient>,
      css::FunctionNotation<css::ValueId::WebkitRepeatingLinearGradient, PrefixedLinearGradient>,
      css::FunctionNotation<css::ValueId::WebkitGradient, DeprecatedLinearGradient>,

      // Radial
      css::FunctionNotation<css::ValueId::RadialGradient, RadialGradient>,
      css::FunctionNotation<css::ValueId::RepeatingRadialGradient, RadialGradient>,
      css::FunctionNotation<css::ValueId::WebkitRadialGradient, PrefixedRadialGradient>,
      css::FunctionNotation<css::ValueId::WebkitRepeatingRadialGradient, PrefixedRadialGradient>,
      css::FunctionNotation<css::ValueId::WebkitGradient, DeprecatedRadialGradient>,

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
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::PrefixedLinearGradient, 3)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::DeprecatedLinearGradient, 3)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::RadialGradient::Ellipse, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::RadialGradient::Circle, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::RadialGradient, 3)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::PrefixedRadialGradient::Ellipse, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::PrefixedRadialGradient::Circle, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::PrefixedRadialGradient, 3)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::DeprecatedRadialGradient::GradientBoxType, 4)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::DeprecatedRadialGradient, 3)
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
