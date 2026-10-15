#include "Krystal.Booey/Style/Values/Shapes/ShapeFunction.hpp"

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PathFunction.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathBuilder.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathByteStreamSource.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathParser.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathSource.hpp"

namespace krys::boo::style
{
  // MARK: - Control Point Evaluation

  template <typename ControlPoint>
  KRYS_NODISCARD static ControlPointAnchor
    EvaluateControlPointAnchoring(const ControlPoint &value, ControlPointAnchor defaultValue) noexcept
  {
    if (value.Anchor)
    {
      return *value.Anchor;
    }
    return defaultValue;
  }

  template <typename ControlPoint>
  KRYS_NODISCARD static FloatPoint EvaluateControlPointOffset(const ControlPoint &value,
                                                              const FloatSize &boxSize) noexcept
  {
    return Evaluate<FloatPoint>(value.Offset, boxSize, style::ZoomNeeded {});
  }

  template <typename ControlPoint>
  KRYS_NODISCARD static FloatPoint
    ResolveControlPoint(CommandAffinity affinity, FloatPoint currentPosition, FloatPoint segmentOffset,
                        const ControlPoint &controlPoint, const FloatSize &boxSize) noexcept
  {
    auto controlPointOffset = EvaluateControlPointOffset(controlPoint, boxSize);

    auto DefaultAnchor = (std::holds_alternative<css::keywords::By>(affinity))
                           ? RelativeControlPoint::DefaultAnchor
                           : AbsoluteControlPoint::DefaultAnchor;
    auto controlPointAnchoring = EvaluateControlPointAnchoring(controlPoint, DefaultAnchor);

    auto absoluteControlPoint = krys::SwitchOn(
      controlPointAnchoring,
      [&](css::keywords::Start)
      {
        auto absoluteStartPoint = currentPosition;
        return absoluteStartPoint + controlPointOffset;
      },
      [&](css::keywords::End)
      {
        auto absoluteEndPoint = (std::holds_alternative<css::keywords::By>(affinity))
                                  ? currentPosition + ToFloatSize(segmentOffset)
                                  : segmentOffset;
        return absoluteEndPoint + controlPointOffset;
      },
      [&](css::keywords::Origin) { return controlPointOffset; });

    if (std::holds_alternative<css::keywords::By>(affinity))
    {
      return absoluteControlPoint - ToFloatSize(currentPosition);
    }
    return absoluteControlPoint;
  }

  // MARK: - ShapeSVGPathSource

  class ShapeSVGPathSource final : public svg::SVGPathSource
  {
  public:
    explicit ShapeSVGPathSource(const Position &startPoint, const Shape &shape,
                                const FloatSize &boxSize) noexcept
        : _start(startPoint), _shape(shape), _boxSize(boxSize), _endIndex(shape.Commands.size())
    {
    }

  private:
    KRYS_NODISCARD bool HasMoreData() const noexcept override
    {
      return _nextIndex < _endIndex;
    }

    bool MoveToNextToken() noexcept override
    {
      return true;
    }

    KRYS_NODISCARD svg::SVGPathSegType NextCommand(svg::SVGPathSegType) noexcept override
    {
      auto type = SegmentTypeAtIndex(_nextIndex);
      ++_nextIndex;
      return type;
    }

    KRYS_NODISCARD Maybe<svg::SVGPathSegType> ParseSVGSegmentType() noexcept override
    {
      // This represents the initial move to to set the "from" position.
      krys_debug_assert(!_nextIndex);
      return svg::SVGPathSegType::MoveToAbs;
    }

    KRYS_NODISCARD Maybe<MoveToSegment> ParseMoveToSegment(FloatPoint) noexcept override
    {
      if (!_nextIndex)
      {
        return MoveToSegment {Evaluate<FloatPoint>(_start, _boxSize, style::ZoomNeeded {})};
      }

      auto &moveCommand = CurrentValue<MoveCommand>();

      return MoveToSegment {Evaluate<FloatPoint>(moveCommand.ToBy, _boxSize, style::ZoomNeeded {})};
    }

    KRYS_NODISCARD Maybe<LineToSegment> ParseLineToSegment(FloatPoint) noexcept override
    {
      auto &lineCommand = CurrentValue<LineCommand>();

      return LineToSegment {Evaluate<FloatPoint>(lineCommand.ToBy, _boxSize, style::ZoomNeeded {})};
    }

    KRYS_NODISCARD Maybe<LineToHorizontalSegment> ParseLineToHorizontalSegment(FloatPoint) noexcept override
    {
      auto &lineCommand = CurrentValue<HLineCommand>();

      return LineToHorizontalSegment {
        Evaluate<float>(lineCommand.ToBy, _boxSize.Width(), style::ZoomNeeded {})};
    }

    KRYS_NODISCARD Maybe<LineToVerticalSegment> ParseLineToVerticalSegment(FloatPoint) noexcept override
    {
      auto &lineCommand = CurrentValue<VLineCommand>();

      return LineToVerticalSegment {
        Evaluate<float>(lineCommand.ToBy, _boxSize.Height(), style::ZoomNeeded {})};
    }

    KRYS_NODISCARD Maybe<CurveToCubicSegment> ParseCurveToCubicSegment(FloatPoint currentPosition) noexcept override
    {
      auto &curveCommand = CurrentValue<CurveCommand>();

      return krys::SwitchOn(
        curveCommand.ToBy,
        [&](const auto &value)
        {
          auto offset = Evaluate<FloatPoint>(value.Offset, _boxSize, style::ZoomNeeded {});
          return CurveToCubicSegment {
            ResolveControlPoint(value.Affinity, currentPosition, offset, value.ControlPoint1, _boxSize),
            ResolveControlPoint(value.Affinity, currentPosition, offset, value.ControlPoint2.value(),
                                _boxSize),
            offset};
        });
    }

    KRYS_NODISCARD Maybe<CurveToQuadraticSegment> ParseCurveToQuadraticSegment(FloatPoint currentPosition) noexcept override
    {
      auto &curveCommand = CurrentValue<CurveCommand>();

      return krys::SwitchOn(
        curveCommand.ToBy,
        [&](const auto &value)
        {
          auto offset = Evaluate<FloatPoint>(value.Offset, _boxSize, style::ZoomNeeded {});
          return CurveToQuadraticSegment {
            ResolveControlPoint(value.Affinity, currentPosition, offset, value.ControlPoint1, _boxSize),
            offset};
        });
    }

    KRYS_NODISCARD Maybe<CurveToCubicSmoothSegment> ParseCurveToCubicSmoothSegment(FloatPoint currentPosition) noexcept override
    {
      auto &smoothCommand = CurrentValue<SmoothCommand>();

      return krys::SwitchOn(
        smoothCommand.ToBy,
        [&](const auto &value)
        {
          krys_debug_assert(value.ControlPoint);
          auto offset = Evaluate<FloatPoint>(value.Offset, _boxSize, style::ZoomNeeded {});
          return CurveToCubicSmoothSegment {ResolveControlPoint(value.Affinity, currentPosition, offset,
                                                                value.ControlPoint.value(), _boxSize),
                                            offset};
        });
    }

    KRYS_NODISCARD Maybe<CurveToQuadraticSmoothSegment> ParseCurveToQuadraticSmoothSegment(FloatPoint) noexcept override
    {
      auto &smoothCommand = CurrentValue<SmoothCommand>();

      return krys::SwitchOn(smoothCommand.ToBy,
                            [&](const auto &value)
                            {
                              return CurveToQuadraticSmoothSegment {
                                Evaluate<FloatPoint>(value.Offset, _boxSize, style::ZoomNeeded {})};
                            });
    }

    KRYS_NODISCARD Maybe<ArcToSegment> ParseArcToSegment(FloatPoint) noexcept override
    {
      auto &arcCommand = CurrentValue<ArcCommand>();

      auto radius = Evaluate<FloatSize>(arcCommand.Size, _boxSize, style::ZoomNeeded {});
      return ArcToSegment {.Rx = radius.Width(),
                           .Ry = radius.Height(),
                           .Angle = static_cast<float>(arcCommand.Rotation.Value),
                           .LargeArc = std::holds_alternative<css::keywords::Large>(arcCommand.ArcSize),
                           .Sweep = std::holds_alternative<css::keywords::Cw>(arcCommand.ArcSweep),
                           .TargetPoint =
                             Evaluate<FloatPoint>(arcCommand.ToBy, _boxSize, style::ZoomNeeded {})};
    }

    KRYS_NODISCARD svg::SVGPathSegType SegmentTypeAtIndex(size_t index) const noexcept
    {
      if (index >= _shape.Commands.size())
      {
        return svg::SVGPathSegType::Unknown;
      }

      return krys::SwitchOn(
        _shape.Commands[index],
        [&](const MoveCommand &command)
        {
          return std::holds_alternative<MoveCommand::To>(command.ToBy) ? svg::SVGPathSegType::MoveToAbs
                                                                       : svg::SVGPathSegType::MoveToRel;
        },
        [&](const LineCommand &command)
        {
          return std::holds_alternative<LineCommand::To>(command.ToBy) ? svg::SVGPathSegType::LineToAbs
                                                                       : svg::SVGPathSegType::LineToRel;
        },
        [&](const HLineCommand &command)
        {
          return std::holds_alternative<HLineCommand::To>(command.ToBy) ? svg::SVGPathSegType::LineToHorizontalAbs
                                                                        : svg::SVGPathSegType::LineToHorizontalRel;
        },
        [&](const VLineCommand &command)
        {
          return std::holds_alternative<VLineCommand::To>(command.ToBy) ? svg::SVGPathSegType::LineToVerticalAbs
                                                                        : svg::SVGPathSegType::LineToVerticalRel;
        },
        [&](const CurveCommand &command)
        {
          return krys::SwitchOn(command.ToBy,
                                [](const auto &value)
                                {
                                  if (value.ControlPoint2)
                                  {
                                    return std::holds_alternative<css::keywords::To>(value.Affinity)
                                             ? svg::SVGPathSegType::CurveToCubicAbs
                                             : svg::SVGPathSegType::CurveToCubicRel;
                                  }
                                  return std::holds_alternative<css::keywords::To>(value.Affinity)
                                           ? svg::SVGPathSegType::CurveToQuadraticAbs
                                           : svg::SVGPathSegType::CurveToQuadraticRel;
                                });
        },
        [&](const SmoothCommand &command)
        {
          return krys::SwitchOn(command.ToBy,
                                [](const auto &value)
                                {
                                  if (value.ControlPoint)
                                  {
                                    return std::holds_alternative<css::keywords::To>(value.Affinity)
                                             ? svg::SVGPathSegType::CurveToCubicSmoothAbs
                                             : svg::SVGPathSegType::CurveToCubicSmoothRel;
                                  }
                                  return std::holds_alternative<css::keywords::To>(value.Affinity)
                                           ? svg::SVGPathSegType::CurveToQuadraticSmoothAbs
                                           : svg::SVGPathSegType::CurveToQuadraticSmoothRel;
                                });
        },
        [&](const ArcCommand &command)
        {
          return std::holds_alternative<ArcCommand::To>(command.ToBy) ? svg::SVGPathSegType::ArcAbs
                                                                      : svg::SVGPathSegType::ArcRel;
        },
        [&](const CloseCommand &) { return svg::SVGPathSegType::ClosePath; });
    }

    template <typename T>
    const T &CurrentValue() const
    {
      krys_debug_assert(_nextIndex);
      krys_debug_assert(_nextIndex <= _shape.Commands.size());
      return std::get<T>(_shape.Commands[_nextIndex - 1]);
    }

    Position _start;
    const Shape &_shape;
    FloatSize _boxSize;
    size_t _endIndex {0};
    size_t _nextIndex {0};
  };

  // MARK: - ShapeConversionPathConsumer

  class ShapeConversionPathConsumer final : public svg::SVGPathConsumer
  {
  public:
    explicit ShapeConversionPathConsumer(css::CommaSeparatedList<ShapeCommand>::Container &commands) noexcept : _commands(commands)
    {
    }

    KRYS_NODISCARD const Maybe<Position> &InitialMove() const noexcept
    {
      return _initialMove;
    }

  private:
    KRYS_NODISCARD static Position MakePosition(FloatPoint p) noexcept
    {
      return {p};
    }

    KRYS_NODISCARD static CoordinatePair MakeCoordinatePair(FloatPoint p) noexcept
    {
      return {LengthPercentage<>::Dimension {p.X()}, LengthPercentage<>::Dimension {p.Y()}};
    }

    KRYS_NODISCARD static Position AbsoluteOffsetPoint(FloatPoint p) noexcept
    {
      return MakePosition(p);
    }

    KRYS_NODISCARD static CoordinatePair RelativeOffsetPoint(FloatPoint p) noexcept
    {
      return MakeCoordinatePair(p);
    }

    KRYS_NODISCARD static Variant<ToPosition, ByCoordinatePair>
      FromOffsetPoint(const FloatPoint &offsetPoint, svg::PathCoordinateMode mode) noexcept
    {
      switch (mode)
      {
        case svg::PathCoordinateMode::AbsoluteCoordinates: return style::ToPosition {AbsoluteOffsetPoint(offsetPoint)};
        case svg::PathCoordinateMode::RelativeCoordinates: return ByCoordinatePair {RelativeOffsetPoint(offsetPoint)};
      }
      krys_unreachable();
    }

    template <typename Command>
    KRYS_NODISCARD static Variant<typename Command::To, typename Command::By>
      FromOffsetLength(float offset, svg::PathCoordinateMode mode) noexcept
    {
      switch (mode)
      {
        case svg::PathCoordinateMode::AbsoluteCoordinates:
          return typename Command::To {.Offset = {LengthPercentage<>::Dimension {offset}}};
        case svg::PathCoordinateMode::RelativeCoordinates:
          return typename Command::By {.Offset = LengthPercentage<>::Dimension {offset}};
      }
      krys_unreachable();
    }

    KRYS_NODISCARD static style::AbsoluteControlPoint MakeAbsoluteControlPoint(const FloatPoint &controlPoint) noexcept
    {
      return {MakePosition(controlPoint), std::nullopt};
    }

    KRYS_NODISCARD static style::RelativeControlPoint MakeRelativeControlPoint(const FloatPoint &controlPoint) noexcept
    {
      return {MakeCoordinatePair(controlPoint), std::nullopt};
    }

    void IncrementPathSegmentCount() noexcept override
    {
    }

    KRYS_NODISCARD bool ContinueConsuming() noexcept override
    {
      return true;
    }

    void MoveTo(const FloatPoint &offsetPoint, bool, svg::PathCoordinateMode mode) noexcept override
    {
      if (_commands.empty() && mode == svg::PathCoordinateMode::AbsoluteCoordinates && !_initialMove)
      {
        _initialMove = MakePosition(offsetPoint);
        return;
      }

      _commands.push_back(MoveCommand {.ToBy = FromOffsetPoint(offsetPoint, mode)});
    }

    void LineTo(const FloatPoint &offsetPoint, svg::PathCoordinateMode mode) noexcept override
    {
      _commands.push_back(LineCommand {.ToBy = FromOffsetPoint(offsetPoint, mode)});
    }

    void LineToHorizontal(float length, svg::PathCoordinateMode mode) noexcept override
    {
      _commands.push_back(HLineCommand {.ToBy = FromOffsetLength<HLineCommand>(length, mode)});
    }

    void LineToVertical(float length, svg::PathCoordinateMode mode) noexcept override
    {
      _commands.push_back(VLineCommand {.ToBy = FromOffsetLength<VLineCommand>(length, mode)});
    }

    void CurveToCubic(const FloatPoint &controlPoint1, const FloatPoint &controlPoint2,
                      const FloatPoint &offsetPoint, svg::PathCoordinateMode mode) noexcept override
    {
      switch (mode)
      {
        case svg::PathCoordinateMode::AbsoluteCoordinates:
          _commands.push_back(CurveCommand {.ToBy = CurveCommand::To {
                                           .Offset = AbsoluteOffsetPoint(offsetPoint),
                                           .ControlPoint1 = MakeAbsoluteControlPoint(controlPoint1),
                                           .ControlPoint2 = MakeAbsoluteControlPoint(controlPoint2),
                                         }});
          break;
        case svg::PathCoordinateMode::RelativeCoordinates:
          _commands.push_back(CurveCommand {.ToBy = CurveCommand::By {
                                           .Offset = RelativeOffsetPoint(offsetPoint),
                                           .ControlPoint1 = MakeRelativeControlPoint(controlPoint1),
                                           .ControlPoint2 = MakeRelativeControlPoint(controlPoint2),
                                         }});
          break;
      }
    }

    void CurveToQuadratic(const FloatPoint &controlPoint, const FloatPoint &offsetPoint,
                          svg::PathCoordinateMode mode) noexcept override
    {
      switch (mode)
      {
        case svg::PathCoordinateMode::AbsoluteCoordinates:
          _commands.push_back(CurveCommand {.ToBy = CurveCommand::To {
                                           .Offset = AbsoluteOffsetPoint(offsetPoint),
                                           .ControlPoint1 = MakeAbsoluteControlPoint(controlPoint),
                                           .ControlPoint2 = null,
                                         }});
          break;
        case svg::PathCoordinateMode::RelativeCoordinates:
          _commands.push_back(CurveCommand {.ToBy = CurveCommand::By {
                                           .Offset = RelativeOffsetPoint(offsetPoint),
                                           .ControlPoint1 = MakeRelativeControlPoint(controlPoint),
                                           .ControlPoint2 = null,
                                         }});
          break;
      }
    }

    void CurveToCubicSmooth(const FloatPoint &controlPoint, const FloatPoint &offsetPoint,
                            svg::PathCoordinateMode mode) noexcept override
    {
      switch (mode)
      {
        case svg::PathCoordinateMode::AbsoluteCoordinates:
          _commands.push_back(SmoothCommand {.ToBy = SmoothCommand::To {
                                            .Offset = AbsoluteOffsetPoint(offsetPoint),
                                            .ControlPoint = MakeAbsoluteControlPoint(controlPoint),
                                          }});
          break;
        case svg::PathCoordinateMode::RelativeCoordinates:
          _commands.push_back(SmoothCommand {.ToBy = SmoothCommand::By {
                                            .Offset = RelativeOffsetPoint(offsetPoint),
                                            .ControlPoint = MakeRelativeControlPoint(controlPoint),
                                          }});
          break;
      }
    }

    void CurveToQuadraticSmooth(const FloatPoint &offsetPoint, svg::PathCoordinateMode mode) noexcept override
    {
      switch (mode)
      {
        case svg::PathCoordinateMode::AbsoluteCoordinates:
          _commands.push_back(SmoothCommand {.ToBy = SmoothCommand::To {
                                            .Offset = AbsoluteOffsetPoint(offsetPoint),
                                            .ControlPoint = null,
                                          }});
          break;
        case svg::PathCoordinateMode::RelativeCoordinates:
          _commands.push_back(SmoothCommand {.ToBy = SmoothCommand::By {
                                            .Offset = RelativeOffsetPoint(offsetPoint),
                                            .ControlPoint = null,
                                          }});
          break;
      }
    }

    void ArcTo(float r1, float r2, float angle, bool largeArcFlag, bool sweepFlag,
               const FloatPoint &offsetPoint, svg::PathCoordinateMode mode) noexcept override
    {
      _commands.push_back(ArcCommand {
        .ToBy = FromOffsetPoint(offsetPoint, mode),
        .Size = {LengthPercentage<>::Dimension {r1}, LengthPercentage<>::Dimension {r2}},
        .ArcSweep = sweepFlag ? ArcSweep {css::keywords::Cw {}} : ArcSweep {css::keywords::Ccw {}},
        .ArcSize = largeArcFlag ? ArcSize {css::keywords::Large {}} : ArcSize {css::keywords::Small {}},
        .Rotation = {angle},
      });
    }

    void ClosePath() noexcept override
    {
      _commands.push_back(CloseCommand {});
    }

    css::CommaSeparatedList<ShapeCommand>::Container &_commands;
    Maybe<Position> _initialMove;
  };

  template <typename T>
  KRYS_NODISCARD T BlendWithPreferredValue(const T &from, const T &to, const T &preferredValue,
                                           const BlendingContext &context) noexcept
  {
    if (context.Progress <= 0)
    {
      return from;
    }

    if (context.Progress >= 1)
    {
      return to;
    }

    if (from == to)
    {
      return from;
    }

    return preferredValue;
  }

  // MARK: - RelativeControlPoint (blending)

  bool Blending<RelativeControlPoint>::CanBlend(const RelativeControlPoint &a,
                                                const RelativeControlPoint &b) noexcept
  {
    return style::CanBlend(a.Offset, b.Offset)
           && a.Anchor.value_or(RelativeControlPoint::DefaultAnchor)
                == b.Anchor.value_or(RelativeControlPoint::DefaultAnchor);
  }

  RelativeControlPoint Blending<RelativeControlPoint>::Blend(const RelativeControlPoint &a,
                                                             const RelativeControlPoint &b,
                                                             const BlendingContext &context) noexcept
  {
    return {.Offset = krys::boo::style::Blend(a.Offset, b.Offset, context),
            .Anchor = a.Anchor.has_value() && b.Anchor.has_value() ? a.Anchor : null};
  }

  // MARK: - AbsoluteControlPoint (blending)

  bool Blending<AbsoluteControlPoint>::CanBlend(const AbsoluteControlPoint &a,
                                                const AbsoluteControlPoint &b) noexcept
  {
    return style::CanBlend(a.Offset, b.Offset)
           && a.Anchor.value_or(AbsoluteControlPoint::DefaultAnchor)
                == b.Anchor.value_or(AbsoluteControlPoint::DefaultAnchor);
  }

  AbsoluteControlPoint Blending<AbsoluteControlPoint>::Blend(const AbsoluteControlPoint &a,
                                                             const AbsoluteControlPoint &b,
                                                             const BlendingContext &context) noexcept
  {
    return {.Offset = krys::boo::style::Blend(a.Offset, b.Offset, context),
            .Anchor = a.Anchor.has_value() && b.Anchor.has_value() ? a.Anchor : null};
  }

  // MARK: - ArcCommand (blending)

  bool Blending<ArcCommand>::CanBlend(const ArcCommand &a, const ArcCommand &b) noexcept
  {
    return style::CanBlend(a.ToBy, b.ToBy);
  }

  ArcCommand Blending<ArcCommand>::Blend(const ArcCommand &a, const ArcCommand &b,
                                         const BlendingContext &context) noexcept
  {
    return {
      .ToBy = krys::boo::style::Blend(a.ToBy, b.ToBy, context),
      .Size = krys::boo::style::Blend(a.Size, b.Size, context),
      .ArcSweep = BlendWithPreferredValue(a.ArcSweep, b.ArcSweep, ArcSweep {css::keywords::Cw {}}, context),
      .ArcSize = BlendWithPreferredValue(a.ArcSize, b.ArcSize, ArcSize {css::keywords::Large {}}, context),
      .Rotation = krys::boo::style::Blend(a.Rotation, b.Rotation, context),
    };
  }

  // MARK: - Shape (path conversion)

  boo::Path PathComputation<Shape>::operator()(const Shape &value, const FloatRect &boundingBox) noexcept
  {
    // FIXME: We should do some caching here.
    auto pathSource = ShapeSVGPathSource(value.StartingPoint, value, boundingBox.Size());

    boo::Path path;
    svg::SVGPathBuilder builder(path);
    (void)svg::SVGPathParser::Parse(pathSource, builder);

    path.Translate(ToFloatSize(boundingBox.Location()));

    return path;
  }

  // MARK: - Wind Rule

  boo::WindRule WindRuleComputation<Shape>::operator()(const Shape &value) noexcept
  {
    return (!value.FillRule || std::holds_alternative<css::keywords::Nonzero>(*value.FillRule))
             ? WindRule::NonZero
             : WindRule::EvenOdd;
  }

  // MARK: - Shape (blending)

  bool Blending<Shape>::CanBlend(const Shape &a, const Shape &b) noexcept
  {
    return ComputeWindRule(a) == ComputeWindRule(b) && style::CanBlend(a.Commands, b.Commands);
  }

  Shape Blending<Shape>::Blend(const Shape &a, const Shape &b, const BlendingContext &context) noexcept
  {
    return {.FillRule = a.FillRule,
            .StartingPoint = krys::boo::style::Blend(a.StartingPoint, b.StartingPoint, context),
            .Commands = krys::boo::style::Blend(a.Commands, b.Commands, context)};
  }

  KRYS_NODISCARD bool CanBlendShapeWithPath(const Shape &shape, const Path &path) noexcept
  {
    if (ComputeWindRule(shape) != ComputeWindRule(path))
    {
      return false;
    }

    // FIXME: This can be made less expensive by specializing a path
    // consumer to check validity, rather than fully constructing the
    // shape just for the canBlend check.
    //
    // Alternatively, the style::CanBlend() and Blend() functions could be
    // merged, allowing for only a single traversal.

    auto shapeFromPath = MakeShapeFromPath(path);
    return shapeFromPath && style::CanBlend(shape, *shapeFromPath);
  }

  KRYS_NODISCARD Maybe<Shape> MakeShapeFromPath(const Path &path) noexcept
  {
    // FIXME: Not clear how to convert a initial Move command to the Shape's "from" parameter.
    // https://github.com/w3c/csswg-drafts/issues/10740

    css::CommaSeparatedList<ShapeCommand>::Container shapeCommands;
    ShapeConversionPathConsumer converter(shapeCommands);
    svg::SVGPathByteStreamSource source(path.Data.ByteStream);

    if (!svg::SVGPathParser::Parse(source, converter, svg::PathParsingMode::UnalteredParsing))
    {
      return {};
    }

    return Shape {.FillRule = path.FillRule,
                  .StartingPoint = converter.InitialMove().value_or(
                    Position {LengthPercentage<>::Dimension {0}, LengthPercentage<>::Dimension {0}}),
                  .Commands = {krys::move(shapeCommands)}};
  }
}
