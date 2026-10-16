#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Position.hpp"
#include "Krystal.Booey/CSS/Values/Shapes/FillRule.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  // <coordinate-pair> = <length-percentage>{2}
  using CoordinatePair = SpaceSeparatedPoint<LengthPercentage<>>;

  // <by-to> = by | to
  // https://drafts.csswg.org/css-shapes-2/#typedef-shape-by-to
  // Indicates if a command is relative or absolute.
  using CommandAffinity = Variant<keywords::By, keywords::To>;

  // <arc-sweep> = cw | ccw
  using ArcSweep = Variant<keywords::Cw, keywords::Ccw>;

  // <arc-size> = large | small
  using ArcSize = Variant<keywords::Large, keywords::Small>;

  // <control-point-anchor> = start | end | origin
  using ControlPointAnchor = Variant<keywords::Start, keywords::End, keywords::Origin>;

  // <to-position> = to <position>
  struct ToPosition
  {
    constexpr static auto Affinity = keywords::To {};

    Position Offset;

    KRYS_NODISCARD bool operator==(const ToPosition &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(ToPosition, Offset);

  template <>
  struct Serialiser<ToPosition>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context, const ToPosition &) noexcept;
  };

  // <by-coordinate-pair> = by <coordinate-pair>
  struct ByCoordinatePair
  {
    constexpr static auto Affinity = keywords::By {};

    CoordinatePair Offset;

    KRYS_NODISCARD bool operator==(const ByCoordinatePair &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(ByCoordinatePair, Offset);

  template <>
  struct Serialiser<ByCoordinatePair>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const ByCoordinatePair &) noexcept;
  };

  // <relative-control-point> = [<coordinate-pair> [from [start | end | origin]]?]
  // Specified https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773
  struct RelativeControlPoint
  {
    using AnchorType = ControlPointAnchor;

    CoordinatePair Offset;
    Maybe<AnchorType> Anchor;

    KRYS_NODISCARD bool operator==(const RelativeControlPoint &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const RelativeControlPoint &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Offset;
    }
    else if constexpr (I == 1)
    {
      return value.Anchor;
    }
  }

  template <>
  struct Serialiser<RelativeControlPoint>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const RelativeControlPoint &) noexcept;
  };

  // <to-control-point> = [<position> | <relative-control-point>]
  // Specified https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773
  struct AbsoluteControlPoint
  {
    using AnchorType = ControlPointAnchor;

    Position Offset;
    Maybe<AnchorType> Anchor;

    KRYS_NODISCARD bool operator==(const AbsoluteControlPoint &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const AbsoluteControlPoint &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Offset;
    }
    else if constexpr (I == 1)
    {
      return value.Anchor;
    }
  }

  template <>
  struct Serialiser<AbsoluteControlPoint>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const AbsoluteControlPoint &) noexcept;
  };

  // <move-command> = move [to <position>] | [by <coordinate-pair>]
  // https://drafts.csswg.org/css-shapes-2/#typedef-shape-move-command
  // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773
  struct MoveCommand
  {
    using To = ToPosition;
    using By = ByCoordinatePair;

    constexpr static auto Name = ValueId::Move;

    Variant<To, By> ToBy;

    KRYS_NODISCARD bool operator==(const MoveCommand &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(MoveCommand, ToBy);

  template <>
  struct Serialiser<MoveCommand>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const MoveCommand &) noexcept;
  };

  // <line-command> = line [to <position>] | [by <coordinate-pair>]
  // https://drafts.csswg.org/css-shapes-2/#typedef-shape-line-command
  // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773
  struct LineCommand
  {
    using To = ToPosition;
    using By = ByCoordinatePair;

    constexpr static auto Name = ValueId::Line;

    Variant<To, By> ToBy;

    KRYS_NODISCARD bool operator==(const LineCommand &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(LineCommand, ToBy);

  template <>
  struct Serialiser<LineCommand>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const LineCommand &) noexcept;
  };

  // <horizontal-line-command> = hline [ to [ <length-percentage> | left | center | right | x-start | x-end ]
  //                                  | by <length-percentage> ]
  // https://drafts.csswg.org/css-shapes-2/#typedef-shape-hv-line-command
  // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2426552611
  struct HLineCommand
  {
    constexpr static auto Name = ValueId::Hline;

    struct To
    {
      constexpr static auto Affinity = keywords::To {};

      TwoComponentPositionHorizontal Offset;

      KRYS_NODISCARD bool operator==(const To &) const noexcept = default;
    };

    struct By
    {
      constexpr static auto Affinity = keywords::By {};

      LengthPercentage<> Offset;

      KRYS_NODISCARD bool operator==(const By &) const noexcept = default;
    };

    Variant<To, By> ToBy;

    KRYS_NODISCARD bool operator==(const HLineCommand &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(HLineCommand::By, Offset);
  DEFINE_TYPE_WRAPPER_GET(HLineCommand::To, Offset);
  DEFINE_TYPE_WRAPPER_GET(HLineCommand, ToBy);

  template <>
  struct Serialiser<HLineCommand::To>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const HLineCommand::To &) noexcept;
  };

  template <>
  struct Serialiser<HLineCommand::By>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const HLineCommand::By &) noexcept;
  };

  template <>
  struct Serialiser<HLineCommand>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const HLineCommand &) noexcept;
  };

  // <vertical-line-command> = vline [ to [ <length-percentage> | top | center | bottom | y-start | y-end ]
  //                                | by <length-percentage> ]
  // https://drafts.csswg.org/css-shapes-2/#typedef-shape-hv-line-command
  // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2426552611
  struct VLineCommand
  {
    constexpr static auto Name = ValueId::Vline;

    struct To
    {
      constexpr static auto Affinity = keywords::To {};

      TwoComponentPositionVertical Offset;

      KRYS_NODISCARD bool operator==(const To &) const noexcept = default;
    };

    struct By
    {
      constexpr static auto Affinity = keywords::By {};

      LengthPercentage<> Offset;

      KRYS_NODISCARD bool operator==(const By &) const noexcept = default;
    };

    Variant<To, By> ToBy;

    KRYS_NODISCARD bool operator==(const VLineCommand &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(VLineCommand::By, Offset);
  DEFINE_TYPE_WRAPPER_GET(VLineCommand::To, Offset);
  DEFINE_TYPE_WRAPPER_GET(VLineCommand, ToBy);

  template <>
  struct Serialiser<VLineCommand::To>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const VLineCommand::To &) noexcept;
  };

  template <>
  struct Serialiser<VLineCommand::By>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const VLineCommand::By &) noexcept;
  };

  template <>
  struct Serialiser<VLineCommand>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const VLineCommand &) noexcept;
  };

  // <curve-command> = curve [to <position> with <to-control-point> [/ <to-control-point>]?]
  //                       | [by <coordinate-pair> with <relative-control-point>
  //                          [/ <relative-control-point>]?]
  // https://drafts.csswg.org/css-shapes-2/#typedef-shape-curve-command
  // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773
  struct CurveCommand
  {
    constexpr static auto Name = ValueId::Curve;

    struct To
    {
      constexpr static auto Affinity = keywords::To {};

      Position Offset;
      AbsoluteControlPoint ControlPoint1;
      Maybe<AbsoluteControlPoint> ControlPoint2;

      KRYS_NODISCARD bool operator==(const To &) const noexcept = default;
    };

    struct By
    {
      constexpr static auto Affinity = keywords::By {};

      CoordinatePair Offset;
      RelativeControlPoint ControlPoint1;
      Maybe<RelativeControlPoint> ControlPoint2;

      KRYS_NODISCARD bool operator==(const By &) const noexcept = default;
    };

    Variant<To, By> ToBy;

    KRYS_NODISCARD bool operator==(const CurveCommand &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const CurveCommand::To &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Offset;
    }
    else if constexpr (I == 1)
    {
      return value.ControlPoint1;
    }
    else if constexpr (I == 2)
    {
      return value.ControlPoint2;
    }
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const CurveCommand::By &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Offset;
    }
    else if constexpr (I == 1)
    {
      return value.ControlPoint1;
    }
    else if constexpr (I == 2)
    {
      return value.ControlPoint2;
    }
  }

  DEFINE_TYPE_WRAPPER_GET(CurveCommand, ToBy);

  template <>
  struct Serialiser<CurveCommand::To>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const CurveCommand::To &) noexcept;
  };

  template <>
  struct Serialiser<CurveCommand::By>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const CurveCommand::By &) noexcept;
  };

  template <>
  struct Serialiser<CurveCommand>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const CurveCommand &) noexcept;
  };

  // <smooth-command> = smooth [to <position> [with <to-control-point>]?]
  //                         | [by <coordinate-pair> [with <relative-control-point>]?]
  // https://drafts.csswg.org/css-shapes-2/#typedef-shape-smooth-command
  // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773
  struct SmoothCommand
  {
    constexpr static auto Name = ValueId::Smooth;

    struct To
    {
      constexpr static auto Affinity = keywords::To {};

      Position Offset;
      Maybe<AbsoluteControlPoint> ControlPoint;

      KRYS_NODISCARD bool operator==(const To &) const noexcept = default;
    };

    struct By
    {
      constexpr static auto Affinity = keywords::By {};

      CoordinatePair Offset;
      Maybe<RelativeControlPoint> ControlPoint;

      KRYS_NODISCARD bool operator==(const By &) const noexcept = default;
    };

    Variant<To, By> ToBy;

    KRYS_NODISCARD bool operator==(const SmoothCommand &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const SmoothCommand::To &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Offset;
    }
    else if constexpr (I == 1)
    {
      return value.ControlPoint;
    }
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const SmoothCommand::By &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Offset;
    }
    else if constexpr (I == 1)
    {
      return value.ControlPoint;
    }
  }

  DEFINE_TYPE_WRAPPER_GET(SmoothCommand, ToBy);

  template <>
  struct Serialiser<SmoothCommand::To>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const SmoothCommand::To &) noexcept;
  };

  template <>
  struct Serialiser<SmoothCommand::By>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const SmoothCommand::By &) noexcept;
  };

  template <>
  struct Serialiser<SmoothCommand>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const SmoothCommand &) noexcept;
  };

  // <arc-command> = arc [to <position>] | [by <coordinate-pair>] of <length-percentage>{1,2}
  //                     [<arc-sweep>? || <arc-size>? || [rotate <angle>]?]
  // https://drafts.csswg.org/css-shapes-2/#typedef-shape-arc-command
  // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773
  struct ArcCommand
  {
    using To = ToPosition;
    using By = ByCoordinatePair;
    using SizeOfEllipse = MinimallySerialisingSpaceSeparatedSize<LengthPercentage<>>;

    constexpr static auto Name = ValueId::Arc;

    Variant<To, By> ToBy;
    SizeOfEllipse Size;
    boo::css::ArcSweep ArcSweep;
    boo::css::ArcSize ArcSize;
    Angle<> Rotation;

    KRYS_NODISCARD bool operator==(const ArcCommand &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const ArcCommand &value) noexcept
  {
    if constexpr (!I)
    {
      return value.ToBy;
    }
    else if constexpr (I == 1)
    {
      return value.Size;
    }
    else if constexpr (I == 2)
    {
      return value.ArcSweep;
    }
    else if constexpr (I == 3)
    {
      return value.ArcSize;
    }
    else if constexpr (I == 4)
    {
      return value.Rotation;
    }
  }

  template <>
  struct Serialiser<ArcCommand>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context, const ArcCommand &) noexcept;
  };

  // <close> = close
  // https://drafts.csswg.org/css-shapes-2/#valdef-shape-close
  using CloseCommand = keywords::Close;

  // <shape-command> = <move-command> | <line-command> | <hv-line-command> | <curve-command>
  //                 | <smooth-command> | <arc-command> | close
  // https://drafts.csswg.org/css-shapes-2/#typedef-shape-command
  using ShapeCommand = Variant<MoveCommand, LineCommand, HLineCommand, VLineCommand, CurveCommand,
                               SmoothCommand, ArcCommand, CloseCommand>;

  // shape() = shape( <'fill-rule'>? from <coordinate-pair>, <shape-command>#)
  // https://drafts.csswg.org/css-shapes-2/#shape-function
  struct Shape
  {
    using CommandsType = CommaSeparatedList<ShapeCommand>;

    Maybe<boo::css::FillRule> FillRule;
    // FIXME: The spec says this should be a <coordinate-pair>, but the tests and some comments indicate it
    // has changed to position.
    Position StartingPoint;
    CommandsType Commands;

    KRYS_NODISCARD bool operator==(const Shape &) const noexcept = default;
  };

  using ShapeFunction = FunctionNotation<ValueId::Shape, Shape>;

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Shape &value) noexcept
  {
    if constexpr (!I)
    {
      return value.FillRule;
    }
    else if constexpr (I == 1)
    {
      return value.StartingPoint;
    }
    else if constexpr (I == 2)
    {
      return value.Commands;
    }
  }

  template <>
  struct Serialiser<Shape>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context, const Shape &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::ToPosition, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::ByCoordinatePair, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::RelativeControlPoint, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::AbsoluteControlPoint, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::MoveCommand, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::LineCommand, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::HLineCommand::To, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::HLineCommand::By, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::HLineCommand, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::VLineCommand::To, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::VLineCommand::By, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::VLineCommand, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::CurveCommand::To, 3)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::CurveCommand::By, 3)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::CurveCommand, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::SmoothCommand::To, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::SmoothCommand::By, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::SmoothCommand, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::ArcCommand, 5)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Shape, 3)
