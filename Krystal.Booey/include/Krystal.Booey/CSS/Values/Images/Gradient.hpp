#pragma once

#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Position.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/Graphics/Color/ColorInterpolationMethod.hpp"
#include "Krystal.Core/Colour/AlphaPremultiplication.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::css
{
  // MARK: - Common Types

  using DeprecatedGradientPosition = SpaceSeparatedArray<NumberOrPercentage<>, 2>;

  using Horizontal = Variant<keywords::Left, keywords::Right>;
  using Vertical = Variant<keywords::Top, keywords::Bottom>;

  using RadialGradientExtent =
    Variant<keywords::ClosestCorner, keywords::ClosestSide, keywords::FarthestCorner, keywords::FarthestSide>;
  using PrefixedRadialGradientExtent =
    Variant<keywords::ClosestCorner, keywords::ClosestSide, keywords::FarthestCorner, keywords::FarthestSide,
            keywords::Contain, keywords::Cover>;

  // MARK: - Gradient Color Interpolation Definitions.

  struct GradientColorInterpolationMethod
  {
    enum class Default : bool
    {
      SRGB,
      OKLab
    };

    ColourInterpolationMethod Method;
    Default DefaultMethod;

    KRYS_NODISCARD static GradientColorInterpolationMethod
      LegacyMethod(krys::AlphaPremultiplication alphaPremultiplication) noexcept
    {
      return {{ColourInterpolationMethod::SRGB {}, alphaPremultiplication}, Default::SRGB};
    }

    KRYS_NODISCARD bool operator==(const GradientColorInterpolationMethod &) const noexcept = default;
  };

  template <>
  struct ComputedStyleDependenciesCollector<GradientColorInterpolationMethod>
  {
    constexpr void operator()(ComputedStyleDependencies &, const GradientColorInterpolationMethod &) noexcept
    {
    }
  };

  template <>
  struct ValueChildrenVisitor<GradientColorInterpolationMethod>
  {
    KRYS_NODISCARD constexpr IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &,
                                                        const GradientColorInterpolationMethod &) noexcept
    {
      return IterationStatus::Continue;
    }
  };

  // MARK: - Gradient Color Stop Definitions.

  template <typename Stop>
  using GradientColorStopList = CommaSeparatedList<Stop, 2>;

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
  struct Serialiser<GradientAngularColorStop>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const GradientAngularColorStop &stop) noexcept;
  };

  template <>
  struct Serialiser<GradientLinearColorStop>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const GradientLinearColorStop &stop) noexcept;
  };

  template <>
  struct Serialiser<GradientDeprecatedColorStop>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const GradientDeprecatedColorStop &stop) noexcept;
  };

  // MARK: - LinearGradient

  struct LinearGradient
  {
    using GradientLineType =
      Variant<Angle<>, Horizontal, Vertical, SpaceSeparatedTuple<Horizontal, Vertical>>;

    GradientColorInterpolationMethod ColourInterpolationMethod;
    GradientLineType GradientLine;
    GradientLinearColorStopList Stops;

    KRYS_NODISCARD bool operator==(const LinearGradient &) const noexcept = default;
  };

  template <>
  struct Serialiser<LinearGradient>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const LinearGradient &gradient) noexcept;
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

  // MARK: - PrefixedLinearGradient

  struct PrefixedLinearGradient
  {
    using GradientLineType =
      Variant<Angle<>, Horizontal, Vertical, SpaceSeparatedTuple<Horizontal, Vertical>>;

    GradientColorInterpolationMethod ColourInterpolationMethod;
    GradientLineType GradientLine;
    GradientLinearColorStopList Stops;

    KRYS_NODISCARD bool operator==(const PrefixedLinearGradient &) const noexcept = default;
  };

  template <>
  struct Serialiser<PrefixedLinearGradient>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const PrefixedLinearGradient &gradient) noexcept;
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

  // MARK: - DeprecatedLinearGradient

  struct DeprecatedLinearGradient
  {
    using GradientLineType = CommaSeparatedArray<DeprecatedGradientPosition, 2>;

    GradientColorInterpolationMethod ColourInterpolationMethod;
    GradientLineType GradientLine;
    GradientDeprecatedColorStopList Stops;

    KRYS_NODISCARD bool operator==(const DeprecatedLinearGradient &) const noexcept = default;
  };

  template <>
  struct Serialiser<DeprecatedLinearGradient>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const DeprecatedLinearGradient &gradient) noexcept;
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

  // MARK: - RadialGradient

  struct RadialGradient
  {
    using Extent = RadialGradientExtent;

    struct Ellipse
    {
      using SizeType = SpaceSeparatedArray<LengthPercentage<NonNegative>, 2>;

      Variant<SizeType, Extent> Size;
      Maybe<boo::css::Position> Position;

      KRYS_NODISCARD bool operator==(const Ellipse &) const noexcept = default;
    };

    struct Circle
    {
      using Length = css::Length<NonNegative>;

      Variant<Length, Extent> Size;
      Maybe<boo::css::Position> Position;

      KRYS_NODISCARD bool operator==(const Circle &) const noexcept = default;
    };

    using GradientBoxType = Variant<Ellipse, Circle>;

    GradientColorInterpolationMethod ColourInterpolationMethod;
    GradientBoxType GradientBox;
    GradientLinearColorStopList Stops;

    KRYS_NODISCARD bool operator==(const RadialGradient &) const noexcept = default;
  };

  template <>
  struct Serialiser<RadialGradient::Ellipse>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const RadialGradient::Ellipse &ellipse) noexcept;
  };

  template <>
  struct Serialiser<RadialGradient::Circle>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const RadialGradient::Circle &circle) noexcept;
  };

  template <>
  struct Serialiser<RadialGradient>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const RadialGradient &gradient) noexcept;
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

  // MARK: - PrefixedRadialGradient

  struct PrefixedRadialGradient
  {
    using Extent = PrefixedRadialGradientExtent;

    struct Ellipse
    {
      using SizeType = SpaceSeparatedArray<LengthPercentage<NonNegative>, 2>;

      Maybe<Variant<SizeType, Extent>> Size;
      Maybe<boo::css::Position> Position;

      KRYS_NODISCARD bool operator==(const Ellipse &) const noexcept = default;
    };

    struct Circle
    {
      Maybe<Extent> Size;
      Maybe<boo::css::Position> Position;

      KRYS_NODISCARD bool operator==(const Circle &) const noexcept = default;
    };

    using GradientBoxType = Variant<Ellipse, Circle>;

    GradientColorInterpolationMethod ColourInterpolationMethod;
    GradientBoxType GradientBox;
    GradientLinearColorStopList Stops;

    KRYS_NODISCARD bool operator==(const PrefixedRadialGradient &) const noexcept = default;
  };

  template <>
  struct Serialiser<PrefixedRadialGradient::Ellipse>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const PrefixedRadialGradient::Ellipse &ellipse) noexcept;
  };

  template <>
  struct Serialiser<PrefixedRadialGradient::Circle>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const PrefixedRadialGradient::Circle &circle) noexcept;
  };

  template <>
  struct Serialiser<PrefixedRadialGradient>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const PrefixedRadialGradient &gradient) noexcept;
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

  // MARK: - DeprecatedRadialGradient

  struct DeprecatedRadialGradient
  {
    struct GradientBoxType
    {
      DeprecatedGradientPosition First;
      Number<NonNegative> FirstRadius;
      DeprecatedGradientPosition Second;
      Number<NonNegative> SecondRadius;

      KRYS_NODISCARD bool operator==(const GradientBoxType &) const noexcept = default;
    };

    GradientColorInterpolationMethod ColourInterpolationMethod;
    GradientBoxType GradientBox;
    GradientDeprecatedColorStopList Stops;

    KRYS_NODISCARD bool operator==(const DeprecatedRadialGradient &) const noexcept = default;
  };

  template <>
  struct Serialiser<DeprecatedRadialGradient::GradientBoxType>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const DeprecatedRadialGradient::GradientBoxType &gradientBox) noexcept;
  };

  template <>
  struct Serialiser<DeprecatedRadialGradient>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const DeprecatedRadialGradient &gradient) noexcept;
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

  // MARK: - ConicGradient

  struct ConicGradient
  {
    struct GradientBoxType
    {
      Maybe<boo::css::Angle<>> Angle;
      Maybe<boo::css::Position> Position;

      KRYS_NODISCARD bool operator==(const GradientBoxType &) const noexcept = default;
    };

    GradientColorInterpolationMethod ColourInterpolationMethod;
    GradientBoxType GradientBox;
    GradientAngularColorStopList Stops;

    KRYS_NODISCARD bool operator==(const ConicGradient &) const noexcept = default;
  };

  template <>
  struct Serialiser<ConicGradient::GradientBoxType>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const ConicGradient::GradientBoxType &gradientBox) noexcept;
  };

  template <>
  struct Serialiser<ConicGradient>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const ConicGradient &gradient) noexcept;
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

  // MARK: - Gradient (variant)

  using Gradient = Variant<
    // Linear
    FunctionNotation<ValueId::LinearGradient, LinearGradient>,
    FunctionNotation<ValueId::RepeatingLinearGradient, LinearGradient>,
    FunctionNotation<ValueId::WebkitLinearGradient, PrefixedLinearGradient>,
    FunctionNotation<ValueId::WebkitRepeatingLinearGradient, PrefixedLinearGradient>,
    FunctionNotation<ValueId::WebkitGradient, DeprecatedLinearGradient>,

    // Radial
    FunctionNotation<ValueId::RadialGradient, RadialGradient>,
    FunctionNotation<ValueId::RepeatingRadialGradient, RadialGradient>,
    FunctionNotation<ValueId::WebkitRadialGradient, PrefixedRadialGradient>,
    FunctionNotation<ValueId::WebkitRepeatingRadialGradient, PrefixedRadialGradient>,
    FunctionNotation<ValueId::WebkitGradient, DeprecatedRadialGradient>,

    // Conic
    FunctionNotation<ValueId::ConicGradient, ConicGradient>,
    FunctionNotation<ValueId::RepeatingConicGradient, ConicGradient>>;
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::LinearGradient, 3)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::PrefixedLinearGradient, 3)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::DeprecatedLinearGradient, 3)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::RadialGradient::Ellipse, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::RadialGradient::Circle, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::RadialGradient, 3)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::PrefixedRadialGradient::Ellipse, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::PrefixedRadialGradient::Circle, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::PrefixedRadialGradient, 3)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::DeprecatedRadialGradient::GradientBoxType, 4)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::DeprecatedRadialGradient, 3)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::ConicGradient::GradientBoxType, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::ConicGradient, 3)

template <typename C, typename P>
constexpr bool krys::boo::css::TreatAsTupleLike<krys::boo::css::GradientColorStop<C, P>> = true;

namespace std
{
  template <typename C, typename P>
  class tuple_size<krys::boo::css::GradientColorStop<C, P>> : public std::integral_constant<size_t, 2>
  {
  };

  template <size_t I, typename C, typename P>
  class tuple_element<I, krys::boo::css::GradientColorStop<C, P>>
  {
  public:
    using type = decltype(krys::boo::css::get<I>(std::declval<krys::boo::css::GradientColorStop<C, P>>()));
  };
}
