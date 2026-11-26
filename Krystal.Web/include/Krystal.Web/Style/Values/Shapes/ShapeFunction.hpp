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

#include "Krystal.Web/Css/Values/Shapes/ShapeFunction.hpp"
#include "Krystal.Web/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Web/Style/Values/Primitives/Position.hpp"
#include "Krystal.Web/Style/Values/Shapes/FillRule.hpp"
#include "Krystal.Web/Style/Values/Shapes/PathComputation.hpp"
#include "Krystal.Web/Style/Values/Shapes/WindRuleComputation.hpp"

namespace krys::web::style
{
  struct Path;

  // <coordinate-pair> = <length-percentage>{2}
  using CoordinatePair = css::SpaceSeparatedPoint<LengthPercentage<>>;

  using CommandAffinity = css::CommandAffinity;
  using ArcSweep = css::ArcSweep;
  using ArcSize = css::ArcSize;

  // <control-point-anchor> = start | end | origin
  using ControlPointAnchor = css::ControlPointAnchor;

  // <to-position> = to <position>
  struct ToPosition
  {
    constexpr static CommandAffinity Affinity = css::keywords::To {};

    style::Position Offset;

    KRYS_NODISCARD bool operator==(const ToPosition &) const noexcept = default;
  };

  KRYS_DEFINE_TYPE_WRAPPER_GET(ToPosition, Offset);
  KRYS_DEFINE_TYPE_MAPPING(css::ToPosition, ToPosition)

  // <by-coordinate-pair> = by <coordinate-pair>
  struct ByCoordinatePair
  {
    constexpr static CommandAffinity Affinity = css::keywords::By {};

    CoordinatePair Offset;

    KRYS_NODISCARD bool operator==(const ByCoordinatePair &) const noexcept = default;
  };

  KRYS_DEFINE_TYPE_WRAPPER_GET(ByCoordinatePair, Offset);
  KRYS_DEFINE_TYPE_MAPPING(css::ByCoordinatePair, ByCoordinatePair)

  // <relative-control-point> = [<coordinate-pair> [from [start | end | origin]]?]
  // Specified https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773
  struct RelativeControlPoint
  {
    using AnchorType = ControlPointAnchor;
    constexpr static auto DefaultAnchor = AnchorType {css::keywords::Start {}};

    CoordinatePair Offset;
    Maybe<AnchorType> Anchor;

    KRYS_NODISCARD bool operator==(const RelativeControlPoint &) const noexcept = default;
  };

  template <size_t I>
  const auto &get(const RelativeControlPoint &value)
  {
    if constexpr (!I)
    {
      return value.Offset;
    }
    if constexpr (I == 1)
    {
      return value.Anchor;
    }
  }
  KRYS_DEFINE_TYPE_MAPPING(css::RelativeControlPoint, RelativeControlPoint)

  template <>
  struct Blending<RelativeControlPoint>
  {
    KRYS_NODISCARD static bool CanBlend(const RelativeControlPoint &, const RelativeControlPoint &) noexcept;

    KRYS_NODISCARD static RelativeControlPoint
      Blend(const RelativeControlPoint &, const RelativeControlPoint &, const BlendingContext &) noexcept;
  };

  // <to-control-point> = [<position> | <relative-control-point>]
  // Specified https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773
  struct AbsoluteControlPoint
  {
    using AnchorType = ControlPointAnchor;
    constexpr static auto DefaultAnchor = AnchorType {css::keywords::Origin {}};

    style::Position Offset;
    Maybe<AnchorType> Anchor;

    KRYS_NODISCARD bool operator==(const AbsoluteControlPoint &) const noexcept = default;
  };

  template <size_t I>
  const auto &get(const AbsoluteControlPoint &value)
  {
    if constexpr (!I)
    {
      return value.Offset;
    }
    if constexpr (I == 1)
    {
      return value.Anchor;
    }
  }
  KRYS_DEFINE_TYPE_MAPPING(css::AbsoluteControlPoint, AbsoluteControlPoint)

  template <>
  struct Blending<AbsoluteControlPoint>
  {
    KRYS_NODISCARD static bool CanBlend(const AbsoluteControlPoint &, const AbsoluteControlPoint &) noexcept;

    KRYS_NODISCARD static AbsoluteControlPoint
      Blend(const AbsoluteControlPoint &, const AbsoluteControlPoint &, const BlendingContext &) noexcept;
  };

  // MARK: - Move Command

  // <move-command> = move [to <position>] | [by <coordinate-pair>]
  // https://drafts.csswg.org/css-shapes-2/#typedef-shape-move-command
  // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773
  struct MoveCommand
  {
    constexpr static auto Name = css::ValueId::Move;
    using To = ToPosition;
    using By = ByCoordinatePair;
    Variant<To, By> ToBy;

    KRYS_NODISCARD bool operator==(const MoveCommand &) const noexcept = default;
  };

  KRYS_DEFINE_TYPE_WRAPPER_GET(MoveCommand, ToBy);
  KRYS_DEFINE_TYPE_MAPPING(css::MoveCommand, MoveCommand)

  // MARK: - Line Command

  // <line-command> = line [to <position>] | [by <coordinate-pair>]
  // https://drafts.csswg.org/css-shapes-2/#typedef-shape-line-command
  // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773
  struct LineCommand
  {
    constexpr static auto Name = css::ValueId::Line;
    using To = ToPosition;
    using By = ByCoordinatePair;
    Variant<To, By> ToBy;

    KRYS_NODISCARD bool operator==(const LineCommand &) const noexcept = default;
  };

  KRYS_DEFINE_TYPE_WRAPPER_GET(LineCommand, ToBy);
  KRYS_DEFINE_TYPE_MAPPING(css::LineCommand, LineCommand)

  // MARK: - HLine Command

  // <horizontal-line-command> = hline [ to [ <length-percentage> | left | center | right | x-start | x-end ]
  // | by <length-percentage> ] https://drafts.csswg.org/css-shapes-2/#typedef-shape-hv-line-command Modified
  // by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2426552611
  struct HLineCommand
  {
    constexpr static auto Name = css::ValueId::Hline;

    struct To
    {
      constexpr static CommandAffinity Affinity = css::keywords::To {};

      TwoComponentPositionHorizontal Offset;

      KRYS_NODISCARD bool operator==(const To &) const noexcept = default;
    };

    struct By
    {
      constexpr static CommandAffinity Affinity = css::keywords::By {};

      LengthPercentage<> Offset;

      KRYS_NODISCARD bool operator==(const By &) const noexcept = default;
    };

    Variant<To, By> ToBy;

    KRYS_NODISCARD bool operator==(const HLineCommand &) const noexcept = default;
  };

  KRYS_DEFINE_TYPE_WRAPPER_GET(HLineCommand::By, Offset);
  KRYS_DEFINE_TYPE_WRAPPER_GET(HLineCommand::To, Offset);
  KRYS_DEFINE_TYPE_WRAPPER_GET(HLineCommand, ToBy);
  KRYS_DEFINE_TYPE_MAPPING(css::HLineCommand::To, HLineCommand::To)
  KRYS_DEFINE_TYPE_MAPPING(css::HLineCommand::By, HLineCommand::By)
  KRYS_DEFINE_TYPE_MAPPING(css::HLineCommand, HLineCommand)

  // MARK: - VLine Command

  // <vertical-line-command> = vline [ to [ <length-percentage> | top | center | bottom | y-start | y-end ] |
  // by <length-percentage> ] https://drafts.csswg.org/css-shapes-2/#typedef-shape-hv-line-command Modified by
  // https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2426552611
  struct VLineCommand
  {
    constexpr static auto Name = css::ValueId::Vline;

    struct To
    {
      constexpr static CommandAffinity Affinity = css::keywords::To {};

      TwoComponentPositionVertical Offset;

      KRYS_NODISCARD bool operator==(const To &) const noexcept = default;
    };

    struct By
    {
      constexpr static CommandAffinity Affinity = css::keywords::By {};

      LengthPercentage<> Offset;

      KRYS_NODISCARD bool operator==(const By &) const noexcept = default;
    };

    Variant<To, By> ToBy;

    KRYS_NODISCARD bool operator==(const VLineCommand &) const noexcept = default;
  };

  KRYS_DEFINE_TYPE_WRAPPER_GET(VLineCommand::By, Offset);
  KRYS_DEFINE_TYPE_WRAPPER_GET(VLineCommand::To, Offset);
  KRYS_DEFINE_TYPE_WRAPPER_GET(VLineCommand, ToBy);
  KRYS_DEFINE_TYPE_MAPPING(css::VLineCommand::To, VLineCommand::To)
  KRYS_DEFINE_TYPE_MAPPING(css::VLineCommand::By, VLineCommand::By)
  KRYS_DEFINE_TYPE_MAPPING(css::VLineCommand, VLineCommand)

  // MARK: - Curve Command

  // <curve-command> = curve [to <position> with <to-control-point> [/ <to-control-point>]?]
  //                       | [by <coordinate-pair> with <relative-control-point> [/
  //                       <relative-control-point>]?]
  // https://drafts.csswg.org/css-shapes-2/#typedef-shape-curve-command
  // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773
  struct CurveCommand
  {
    constexpr static auto Name = css::ValueId::Curve;

    struct To
    {
      constexpr static CommandAffinity Affinity = css::keywords::To {};

      style::Position Offset;
      AbsoluteControlPoint ControlPoint1;
      Maybe<AbsoluteControlPoint> ControlPoint2;

      KRYS_NODISCARD bool operator==(const To &) const noexcept = default;
    };

    struct By
    {
      constexpr static CommandAffinity Affinity = css::keywords::By {};

      CoordinatePair Offset;
      RelativeControlPoint ControlPoint1;
      Maybe<RelativeControlPoint> ControlPoint2;

      KRYS_NODISCARD bool operator==(const By &) const noexcept = default;
    };

    Variant<To, By> ToBy;

    KRYS_NODISCARD bool operator==(const CurveCommand &) const noexcept = default;
  };

  template <size_t I>
  const auto &get(const CurveCommand::To &value)
  {
    if constexpr (!I)
    {
      return value.Offset;
    }
    if constexpr (I == 1)
    {
      return value.ControlPoint1;
    }
    if constexpr (I == 2)
    {
      return value.ControlPoint2;
    }
  }

  template <size_t I>
  const auto &get(const CurveCommand::By &value)
  {
    if constexpr (!I)
    {
      return value.Offset;
    }
    if constexpr (I == 1)
    {
      return value.ControlPoint1;
    }
    if constexpr (I == 2)
    {
      return value.ControlPoint2;
    }
  }

  KRYS_DEFINE_TYPE_WRAPPER_GET(CurveCommand, ToBy);

  KRYS_DEFINE_TYPE_MAPPING(css::CurveCommand, CurveCommand)
  KRYS_DEFINE_TYPE_MAPPING(css::CurveCommand::To, CurveCommand::To)
  KRYS_DEFINE_TYPE_MAPPING(css::CurveCommand::By, CurveCommand::By)

  // MARK: - Smooth Command

  // <smooth-command> = smooth [to <position> [with <to-control-point>]?]
  //                         | [by <coordinate-pair> [with <relative-control-point>]?]
  // https://drafts.csswg.org/css-shapes-2/#typedef-shape-smooth-command
  // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773
  struct SmoothCommand
  {
    constexpr static auto Name = css::ValueId::Smooth;

    struct To
    {
      constexpr static CommandAffinity Affinity = css::keywords::To {};

      style::Position Offset;
      Maybe<AbsoluteControlPoint> ControlPoint;

      KRYS_NODISCARD bool operator==(const To &) const noexcept = default;
    };

    struct By
    {
      constexpr static CommandAffinity Affinity = css::keywords::By {};

      CoordinatePair Offset;
      Maybe<RelativeControlPoint> ControlPoint;

      KRYS_NODISCARD bool operator==(const By &) const noexcept = default;
    };

    Variant<To, By> ToBy;

    KRYS_NODISCARD bool operator==(const SmoothCommand &) const noexcept = default;
  };

  template <size_t I>
  const auto &get(const SmoothCommand::To &value)
  {
    if constexpr (!I)
    {
      return value.Offset;
    }
    if constexpr (I == 1)
    {
      return value.ControlPoint;
    }
  }

  template <size_t I>
  const auto &get(const SmoothCommand::By &value)
  {
    if constexpr (!I)
    {
      return value.Offset;
    }
    if constexpr (I == 1)
    {
      return value.ControlPoint;
    }
  }

  KRYS_DEFINE_TYPE_WRAPPER_GET(SmoothCommand, ToBy);

  KRYS_DEFINE_TYPE_MAPPING(css::SmoothCommand, SmoothCommand)
  KRYS_DEFINE_TYPE_MAPPING(css::SmoothCommand::To, SmoothCommand::To)
  KRYS_DEFINE_TYPE_MAPPING(css::SmoothCommand::By, SmoothCommand::By)

  // MARK: - Arc Command

  // <arc-command> = arc [to <position>] | [by <coordinate-pair>] of <length-percentage>{1,2} [<arc-sweep>? ||
  // <arc-size>? || [rotate <angle>]?] https://drafts.csswg.org/css-shapes-2/#typedef-shape-arc-command
  // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773
  struct ArcCommand
  {
    constexpr static auto Name = css::ValueId::Arc;
    using To = ToPosition;
    using By = ByCoordinatePair;
    Variant<To, By> ToBy;

    using SizeOfEllipse = css::MinimallySerialisingSpaceSeparatedSize<LengthPercentage<>>;
    SizeOfEllipse Size;

    style::ArcSweep ArcSweep;
    style::ArcSize ArcSize;
    Angle<> Rotation;

    KRYS_NODISCARD bool operator==(const ArcCommand &) const noexcept = default;
  };

  template <size_t I>
  const auto &get(const ArcCommand &value)
  {
    if constexpr (!I)
    {
      return value.ToBy;
    }
    if constexpr (I == 1)
    {
      return value.Size;
    }
    if constexpr (I == 2)
    {
      return value.ArcSweep;
    }
    if constexpr (I == 3)
    {
      return value.ArcSize;
    }
    if constexpr (I == 4)
    {
      return value.Rotation;
    }
  }

  KRYS_DEFINE_TYPE_MAPPING(css::ArcCommand, ArcCommand)

  template <>
  struct Blending<ArcCommand>
  {
    KRYS_NODISCARD static bool CanBlend(const ArcCommand &, const ArcCommand &) noexcept;

    KRYS_NODISCARD static ArcCommand Blend(const ArcCommand &, const ArcCommand &,
                                           const BlendingContext &) noexcept;
  };

  // MARK: - Close Command

  // <close> = close
  // https://drafts.csswg.org/css-shapes-2/#valdef-shape-close
  using CloseCommand = css::keywords::Close;

  // MARK: - Shape Command (variant)

  // <shape-command> = <move-command> | <line-command> | <hv-line-command> | <curve-command> |
  // <smooth-command> | <arc-command> | close https://drafts.csswg.org/css-shapes-2/#typedef-shape-command
  using ShapeCommand = Variant<MoveCommand, LineCommand, HLineCommand, VLineCommand, CurveCommand,
                               SmoothCommand, ArcCommand, CloseCommand>;

  // MARK: - <shape()>

  // shape() = shape( <'fill-rule'>? from <coordinate-pair>, <shape-command>#)
  // https://drafts.csswg.org/css-shapes-2/#shape-function
  struct Shape
  {
    using CommandsType = css::CommaSeparatedList<ShapeCommand>;

    Maybe<style::FillRule> FillRule;
    // FIXME: The spec says this should be a <coordinate-pair>, but the tests and some comments indicate it
    // has changed to position.
    Position StartingPoint;
    CommandsType Commands;

    KRYS_NODISCARD bool operator==(const Shape &) const noexcept = default;
  };

  using ShapeFunction = css::FunctionNotation<css::ValueId::Shape, Shape>;

  template <size_t I>
  const auto &get(const Shape &value)
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

  KRYS_DEFINE_TYPE_MAPPING(css::Shape, Shape)

  template <>
  struct PathComputation<Shape>
  {
    KRYS_NODISCARD web::Path operator()(const Shape &, const RectF &) noexcept;
  };

  template <>
  struct WindRuleComputation<Shape>
  {
    KRYS_NODISCARD web::WindRule operator()(const Shape &) noexcept;
  };

  template <>
  struct Blending<Shape>
  {
    KRYS_NODISCARD static bool CanBlend(const Shape &, const Shape &) noexcept;

    KRYS_NODISCARD static Shape Blend(const Shape &, const Shape &, const BlendingContext &) noexcept;
  };

  // Returns whether the shape and path can be interpolated together
  // according to the rules in https://drafts.csswg.org/css-shapes-2/#interpolating-shape.
  KRYS_NODISCARD bool CanBlendShapeWithPath(const Shape &, const Path &) noexcept;

  // Makes a `Shape` representation of `Path`. Returns `null` if the path cannot be parsed.
  KRYS_NODISCARD Maybe<Shape> MakeShapeFromPath(const Path &) noexcept;
}

KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::ToPosition, 1)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::ByCoordinatePair, 1)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::RelativeControlPoint, 2)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::AbsoluteControlPoint, 2)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::MoveCommand, 1)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::LineCommand, 1)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::HLineCommand::To, 1)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::HLineCommand::By, 1)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::HLineCommand, 1)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::VLineCommand::To, 1)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::VLineCommand::By, 1)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::VLineCommand, 1)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::CurveCommand::To, 3)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::CurveCommand::By, 3)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::CurveCommand, 1)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::SmoothCommand::To, 2)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::SmoothCommand::By, 2)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::SmoothCommand, 1)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::ArcCommand, 5)
KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::Shape, 3)
