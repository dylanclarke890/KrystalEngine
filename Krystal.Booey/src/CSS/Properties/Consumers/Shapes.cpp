#include "Krystal.Booey/CSS/Properties/Consumers/Shapes.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/AngleDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Background.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Image.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Position.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParsing.hpp"
#include "Krystal.Booey/CSS/Values/BasicShapeValue.hpp"
#include "Krystal.Booey/CSS/Values/PathValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Shapes/BasicShape.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathByteStream.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathUtilities.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  using namespace css::Literals;

  template <ValueId Name, typename T>
  KRYS_NODISCARD static BasicShape ToBasicShape(T &&parameters) noexcept
  {
    return BasicShape {FunctionNotation<Name, T> {krys::move(parameters)}};
  }

  template <ValueId Name, typename T>
  KRYS_NODISCARD static Maybe<BasicShape> ToBasicShape(Maybe<T> &&parameters) noexcept
  {
    if (!parameters)
    {
      return null;
    }

    return ToBasicShape<Name>(krys::move(*parameters));
  }

  KRYS_NODISCARD static Maybe<FillRule> FillRuleFromValueId(ValueId id) noexcept
  {
    switch (id)
    {
      case ValueId::Nonzero:
      {
        return FillRule {keywords::Nonzero {}};
      }
      case ValueId::Evenodd:
      {
        return FillRule {keywords::Evenodd {}};
      }
      default:
      {
        return null;
      }
    }
  }

  KRYS_NODISCARD static Maybe<FillRule> PeekFillRule(TokenRange &tokens) noexcept
  {
    // <'fill-rule'> = nonzero | evenodd
    // https://svgwg.org/svg2-draft/painting.html#FillRuleProperty

    return PeekIdentUsingMapping(tokens, FillRuleFromValueId);
  }

  KRYS_NODISCARD static Maybe<FillRule> ConsumeFillRule(TokenRange &tokens) noexcept
  {
    auto result = PeekFillRule(tokens);
    if (result)
    {
      tokens.Discard();
      tokens.DiscardWhitespace();
    }
    return result;
  }

  template <typename Container>
  KRYS_NODISCARD static Maybe<Container> ConsumePair(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    auto tokensCopy = tokens;

    auto p1 = MetaConsumer<typename Container::value_type>::Consume(tokensCopy, state);
    if (!p1)
    {
      return null;
    }
    auto p2 = MetaConsumer<typename Container::value_type>::Consume(tokensCopy, state);
    if (!p2)
    {
      return null;
    }

    tokens = tokensCopy;
    return Container {krys::move(*p1), krys::move(*p2)};
  }

  KRYS_NODISCARD static Maybe<CoordinatePair> ConsumeCoordinatePair(TokenRange &tokens,
                                                                    PropertyParserState &state) noexcept
  {
    // <coordinate-pair> = <length-percentage>{2}
    // https://drafts.csswg.org/css-shapes-2/#typedef-shape-coordinate-pair

    return ConsumePair<CoordinatePair>(tokens, state);
  }

  KRYS_NODISCARD static Maybe<ControlPointAnchor> ControlPointAnchorFromValueId(ValueId id) noexcept
  {
    switch (id)
    {
      case ValueId::Start:
      {
        return ControlPointAnchor {keywords::Start {}};
      }
      case ValueId::End:
      {
        return ControlPointAnchor {keywords::End {}};
      }
      case ValueId::Origin:
      {
        return ControlPointAnchor {keywords::Origin {}};
      }
      default:
      {
        return null;
      }
    }
  }

  KRYS_NODISCARD static Maybe<RelativeControlPoint>
    ConsumeRelativeControlPoint(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <relative-control-point> = [<coordinate-pair> [from [start | end | origin]]?]
    // Specified https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773

    auto tokensCopy = tokens;

    auto offset = ConsumeCoordinatePair(tokensCopy, state);
    if (!offset)
    {
      return null;
    }

    Maybe<ControlPointAnchor> anchor;
    if (ConsumeIdentRaw<ValueId::From>(tokensCopy))
    {
      anchor = ConsumeIdentUsingMapping(tokensCopy, ControlPointAnchorFromValueId);
      if (!anchor)
      {
        return null;
      }
    }

    tokens = tokensCopy;

    return RelativeControlPoint {.Offset = krys::move(*offset), .Anchor = krys::move(anchor)};
  }

  KRYS_NODISCARD static Maybe<AbsoluteControlPoint>
    ConsumeAbsoluteControlPoint(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <to-control-point> = [<position> | <relative-control-point>]
    // Specified https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773

    // FIXME: [<length-percentage> <length-percentage>] is valid for both <position> and
    // <relative-control-point>. We currently try <relative-control-point> first, but this ambiguity should
    // probably be explicitly resolved in the spec.

    if (auto relativeControlPoint = ConsumeRelativeControlPoint(tokens, state))
    {
      return AbsoluteControlPoint {.Offset = Position {TwoComponentPositionHorizontalVertical {
                                     TwoComponentPositionHorizontal {relativeControlPoint->Offset.x()},
                                     TwoComponentPositionVertical {relativeControlPoint->Offset.y()}}},
                                   .Anchor = relativeControlPoint->Anchor};
    }
    if (auto position = ConsumePositionUnresolved(tokens, state))
    {
      return AbsoluteControlPoint {.Offset = krys::move(*position), .Anchor = null};
    }
    return null;
  }

  template <typename TExtent>
  KRYS_NODISCARD static Maybe<TExtent> RadialExtentFromValueId(ValueId id) noexcept
  {
    switch (id)
    {
      case ValueId::ClosestSide:
      {
        return TExtent {keywords::ClosestSide {}};
      }
      case ValueId::ClosestCorner:
      {
        return TExtent {keywords::ClosestCorner {}};
      }
      case ValueId::FarthestSide:
      {
        return TExtent {keywords::FarthestSide {}};
      }
      case ValueId::FarthestCorner:
      {
        return TExtent {keywords::FarthestCorner {}};
      }
      default:
      {
        return null;
      }
    }
  }

  KRYS_NODISCARD static Circle::RadialSize ConsumeCircleRadialSize(TokenRange &tokens,
                                                                   PropertyParserState &state) noexcept
  {
    // NOTE: For circle(), it uses a modified <radial-size> as the two `length-percentage` values are invalid
    // as expressed in the text.
    // <radial-size>    = <radial-extent> | <length [0,∞]>
    // <radial-extent>  = closest-corner | closest-side | farthest-corner | farthest-side
    // Default to `farthest-corner` if no radial-size is provided.
    // https://drafts.csswg.org/css-images-3/#typedef-radial-size

    // FIXME: The above text is what the spec says, but the current implementation uses the following grammar:
    // <radial-size>    = <radial-extent> | <length-percentage [0,∞]>
    // <radial-extent>  = closest-corner | closest-side | farthest-corner | farthest-side
    // Default to `closest-side` if no radial-size is provided.

    // Default to `closest-side` if no radial-size is provided.
    // FIXME: The spec says that `farthest-corner` should be the default, but this does not match the tests.
    auto DefaultValue = []
    {
      return Circle::RadialSize {Circle::Extent {keywords::ClosestSide {}}};
    };

    if (tokens.Peek().Type() == TokenType::Ident)
    {
      if (auto extent = ConsumeIdentUsingMapping(tokens, RadialExtentFromValueId<Circle::Extent>))
      {
        return Circle::RadialSize {*extent};
      }
      return DefaultValue();
    }

    auto length = MetaConsumer<LengthPercentage<NonNegative>>::Consume(tokens, state);
    if (!length)
    {
      return DefaultValue();
    }

    return Circle::RadialSize {*length};
  }

  KRYS_NODISCARD static Maybe<Circle>
    ConsumeBasicShapeCircleFunctionParameters(TokenRange &args, PropertyParserState &state) noexcept
  {
    // <circle()> = circle( <radial-size>? [ at <position> ]? )
    // https://drafts.csswg.org/css-shapes/#funcdef-basic-shape-circle

    auto radius = ConsumeCircleRadialSize(args, state);

    Maybe<Position> position;
    if (ConsumeIdentRaw<ValueId::At>(args))
    {
      position = ConsumePositionUnresolved(args, state);
      if (!position)
      {
        return null;
      }
    }

    return Circle {.Radius = krys::move(radius), .Position = krys::move(position)};
  }

  KRYS_NODISCARD static Maybe<Ellipse::RadialSize>
    ConsumeEllipseRadialSize(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <radial-size>    = <radial-extent> | <length [0,∞]> | <length-percentage [0,∞]>{2}
    // <radial-extent>  = closest-corner | closest-side | farthest-corner | farthest-side
    // Default to `farthest-corner` if no radial-size is provided.
    // https://drafts.csswg.org/css-images-3/#typedef-radial-size

    // FIXME: The above text is what the spec says, but the current implementation uses the following grammar:
    // <radial-size>    = <radial-extent> | <length-percentage [0,∞]>
    // <radial-extent>  = closest-corner | closest-side | farthest-corner | farthest-side
    // Default to `closest-side` if no radial-size is provided.

    if (tokens.Peek().Type() == TokenType::Ident)
    {
      if (auto extent = ConsumeIdentUsingMapping(tokens, RadialExtentFromValueId<Ellipse::Extent>))
      {
        return Ellipse::RadialSize {*extent};
      }
      return null;
    }

    auto length = MetaConsumer<LengthPercentage<NonNegative>>::Consume(tokens, state);
    if (!length)
    {
      return null;
    }

    return Ellipse::RadialSize {*length};
  }

  KRYS_NODISCARD static Maybe<Ellipse>
    ConsumeBasicShapeEllipseFunctionParameters(TokenRange &args, PropertyParserState &state) noexcept
  {
    // <ellipse()>      = ellipse( <radial-size>? [ at <position> ]? )
    // <radial-size>    = <radial-extent> | <length [0,∞]> | <length-percentage [0,∞]>{2}
    // <radial-extent>  = closest-corner | closest-side | farthest-corner | farthest-side
    // Default to `farthest-corner` if no radial-size is provided.
    // https://drafts.csswg.org/css-shapes/#funcdef-basic-shape-ellipse

    // FIXME: The above text is what the spec says, but the current implementation uses the following grammar:
    // <ellipse()>      = ellipse( [<shape-radius>{2}]? [at <position>]? )
    // <radial-size>    = <radial-extent> | <length-percentage [0,∞]>
    // <radial-extent>  = closest-corner | closest-side | farthest-corner | farthest-side
    // Default to `closest-side closest-side` if no radial-size is provided.

    auto ConsumeRadialSizePair = [&] -> Maybe<SpaceSeparatedPair<Ellipse::RadialSize>>
    {
      if (auto radiusX = ConsumeEllipseRadialSize(args, state))
      {
        auto radiusY = ConsumeEllipseRadialSize(args, state);
        if (!radiusY)
        {
          return null;
        }
        return SpaceSeparatedPair<Ellipse::RadialSize> {krys::move(*radiusX), krys::move(*radiusY)};
      }

      return SpaceSeparatedPair<Ellipse::RadialSize> {
        Ellipse::RadialSize {Ellipse::Extent {keywords::ClosestSide {}}},
        Ellipse::RadialSize {Ellipse::Extent {keywords::ClosestSide {}}}};
    };
    auto radii = ConsumeRadialSizePair();
    if (!radii)
    {
      return null;
    }

    Maybe<Position> position;
    if (ConsumeIdentRaw<ValueId::At>(args))
    {
      position = ConsumePositionUnresolved(args, state);
      if (!position)
      {
        return null;
      }
    }

    return Ellipse {.Radii = krys::move(*radii), .Position = krys::move(position)};
  }

  KRYS_NODISCARD static Maybe<Polygon>
    ConsumeBasicShapePolygonFunctionParameters(TokenRange &args, PropertyParserState &state) noexcept
  {
    // <polygon()> = polygon( <'fill-rule'>? [ round <length> ]? , [<length-percentage> <length-percentage>]#
    // ) https://drafts.csswg.org/css-shapes/#funcdef-basic-shape-polygon

    // FIXME: The above text is what the spec says, but the current implementation does not support the
    // "round" clause.

    auto fillRule = ConsumeFillRule(args);

    // FIXME: Consume optional `round` clause here.

    if (fillRule)
    {
      if (!ConsumeComma(args))
      {
        return null;
      }
    }

    Polygon::VerticesType::Container vertices;
    do
    {
      auto vertex = ConsumePair<Polygon::Vertex>(args, state);
      if (!vertex)
      {
        return null;
      }
      vertices.push_back(krys::move(*vertex));
    } while (ConsumeComma(args));

    return Polygon {.FillRule = krys::move(fillRule), .Vertices = {krys::move(vertices)}};
  }

  KRYS_NODISCARD static Maybe<Path>
    ConsumeBasicShapePathFunctionParameters(TokenRange &args, PathParsingOption options) noexcept
  {
    // <path()> = path( <'fill-rule'>? , <string> )
    // https://drafts.csswg.org/css-shapes/#funcdef-basic-shape-path

    if (HasFlag(options, PathParsingOption::RejectPath))
    {
      return null;
    }

    auto fillRule = PeekFillRule(args);
    if (fillRule)
    {
      if (HasFlag(options, PathParsingOption::RejectPathFillRule))
      {
        return null;
      }

      args.Discard();
      args.DiscardWhitespace();
      if (!ConsumeComma(args))
      {
        return null;
      }
    }

    if (args.Peek().Type() != TokenType::String)
    {
      return null;
    }

    svg::SVGPathByteStream byteStream;
    if (!svg::BuildSVGPathByteStreamFromString(args.ConsumeIncludingWhitespace().IdentCodePoints(),
                                               byteStream, svg::PathParsingMode::UnalteredParsing)
        || byteStream.IsEmpty())
    {
      return null;
    }

    return Path {.FillRule = krys::move(fillRule), .Data = {.ByteStream = krys::move(byteStream)}};
  }

  KRYS_NODISCARD static Maybe<CommandAffinity> CommandAffinityFromValueId(ValueId id) noexcept
  {
    switch (id)
    {
      case ValueId::To:
      {
        return CommandAffinity {keywords::To {}};
      }
      case ValueId::By:
      {
        return CommandAffinity {keywords::By {}};
      }
      default:
      {
        return null;
      }
    }
  }

  KRYS_NODISCARD static Maybe<CommandAffinity> ConsumeShapeCommandAffinity(TokenRange &tokens) noexcept
  {
    // <by-to> = by | to
    // https://drafts.csswg.org/css-shapes-2/#typedef-shape-by-to

    return ConsumeIdentUsingMapping(tokens, CommandAffinityFromValueId);
  }

  KRYS_NODISCARD static Maybe<MoveCommand> ConsumeShapeMoveCommand(TokenRange &tokens,
                                                                   PropertyParserState &state) noexcept
  {
    // <move-command> = move [to <position>] | [by <coordinate-pair>]
    // https://drafts.csswg.org/css-shapes-2/#typedef-shape-move-command
    // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773

    auto affinity = ConsumeShapeCommandAffinity(tokens);
    if (!affinity)
    {
      return null;
    }

    return std::visit(CreateVisitor(
                        [&](keywords::To) -> Maybe<MoveCommand>
                        {
                          auto position = ConsumePositionUnresolved(tokens, state);
                          if (!position)
                          {
                            return null;
                          }
                          return MoveCommand {.ToBy = MoveCommand::To {.Offset = krys::move(*position)}};
                        },
                        [&](keywords::By) -> Maybe<MoveCommand>
                        {
                          auto coordinatePair = ConsumeCoordinatePair(tokens, state);
                          if (!coordinatePair)
                          {
                            return null;
                          }
                          return MoveCommand {.ToBy =
                                                MoveCommand::By {.Offset = krys::move(*coordinatePair)}};
                        }),
                      *affinity);
  }

  KRYS_NODISCARD static Maybe<LineCommand> ConsumeShapeLineCommand(TokenRange &tokens,
                                                                   PropertyParserState &state) noexcept
  {
    // <line-command> = line [to <position>] | [by <coordinate-pair>]
    // https://drafts.csswg.org/css-shapes-2/#typedef-shape-line-command
    // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773

    auto affinity = ConsumeShapeCommandAffinity(tokens);
    if (!affinity)
    {
      return null;
    }

    return std::visit(CreateVisitor(
                        [&](keywords::To) -> Maybe<LineCommand>
                        {
                          auto position = ConsumePositionUnresolved(tokens, state);
                          if (!position)
                          {
                            return null;
                          }
                          return LineCommand {.ToBy = LineCommand::To {.Offset = krys::move(*position)}};
                        },
                        [&](keywords::By) -> Maybe<LineCommand>
                        {
                          auto coordinatePair = ConsumeCoordinatePair(tokens, state);
                          if (!coordinatePair)
                          {
                            return null;
                          }
                          return LineCommand {.ToBy =
                                                LineCommand::By {.Offset = krys::move(*coordinatePair)}};
                        }),
                      *affinity);
  }

  KRYS_NODISCARD static Maybe<HLineCommand> ConsumeShapeHLineCommand(TokenRange &tokens,
                                                                     PropertyParserState &state) noexcept
  {
    // <horizontal-line-command> = hline [ to [ <length-percentage> | left | center | right | x-start | x-end
    // ] | by <length-percentage> ] https://drafts.csswg.org/css-shapes-2/#typedef-shape-hv-line-command
    // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2426552611
    auto affinity = ConsumeShapeCommandAffinity(tokens);
    if (!affinity)
    {
      return null;
    }

    return std::visit(CreateVisitor(
                        [&](keywords::To) -> Maybe<HLineCommand>
                        {
                          auto offset = ConsumeTwoComponentPositionHorizontalUnresolved(tokens, state);
                          if (!offset)
                          {
                            return null;
                          }
                          return HLineCommand {.ToBy = HLineCommand::To {.Offset = krys::move(*offset)}};
                        },
                        [&](keywords::By) -> Maybe<HLineCommand>
                        {
                          auto offset = MetaConsumer<LengthPercentage<>>::Consume(tokens, state);
                          if (!offset)
                          {
                            return null;
                          }
                          return HLineCommand {.ToBy = HLineCommand::By {.Offset = krys::move(*offset)}};
                        }),
                      *affinity);
  }

  KRYS_NODISCARD static Maybe<VLineCommand> ConsumeShapeVLineCommand(TokenRange &tokens,
                                                                     PropertyParserState &state) noexcept
  {
    // <hv-line-command> = [... | vline] <by-to> <length-percentage>
    // https://drafts.csswg.org/css-shapes-2/#typedef-shape-hv-line-command

    auto affinity = ConsumeShapeCommandAffinity(tokens);
    if (!affinity)
    {
      return null;
    }

    return std::visit(CreateVisitor(
                        [&](keywords::To) -> Maybe<VLineCommand>
                        {
                          auto offset = ConsumeTwoComponentPositionVerticalUnresolved(tokens, state);
                          if (!offset)
                          {
                            return null;
                          }
                          return VLineCommand {.ToBy = VLineCommand::To {.Offset = krys::move(*offset)}};
                        },
                        [&](keywords::By) -> Maybe<VLineCommand>
                        {
                          auto offset = MetaConsumer<LengthPercentage<>>::Consume(tokens, state);
                          if (!offset)
                          {
                            return null;
                          }
                          return VLineCommand {.ToBy = VLineCommand::By {.Offset = krys::move(*offset)}};
                        }),
                      *affinity);
  }

  KRYS_NODISCARD static Maybe<CurveCommand> ConsumeShapeCurveCommand(TokenRange &tokens,
                                                                     PropertyParserState &state) noexcept
  {
    // <curve-command> = curve [to <position> with <to-control-point> [/ <to-control-point>]?]
    //                       | [by <coordinate-pair> with <relative-control-point> [/
    //                       <relative-control-point>]?]
    // https://drafts.csswg.org/css-shapes-2/#typedef-shape-curve-command
    // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773

    auto affinity = ConsumeShapeCommandAffinity(tokens);
    if (!affinity)
    {
      return null;
    }

    return std::visit(
      CreateVisitor(
        [&](keywords::To) -> Maybe<CurveCommand>
        {
          auto position = ConsumePositionUnresolved(tokens, state);
          if (!position)
          {
            return null;
          }

          if (!ConsumeIdentRaw<ValueId::With>(tokens))
          {
            return null;
          }

          auto controlPoint1 = ConsumeAbsoluteControlPoint(tokens, state);
          if (!controlPoint1)
          {
            return null;
          }

          Maybe<AbsoluteControlPoint> controlPoint2;
          if (ConsumeSlash(tokens))
          {
            controlPoint2 = ConsumeAbsoluteControlPoint(tokens, state);
            if (!controlPoint2)
            {
              return null;
            }
          }

          return CurveCommand {.ToBy = CurveCommand::To {.Offset = krys::move(*position),
                                                         .ControlPoint1 = krys::move(*controlPoint1),
                                                         .ControlPoint2 = krys::move(controlPoint2)}};
        },
        [&](keywords::By) -> Maybe<CurveCommand>
        {
          auto coordinatePair = ConsumeCoordinatePair(tokens, state);
          if (!coordinatePair)
          {
            return null;
          }

          if (!ConsumeIdentRaw<ValueId::With>(tokens))
          {
            return null;
          }

          auto controlPoint1 = ConsumeRelativeControlPoint(tokens, state);
          if (!controlPoint1)
          {
            return null;
          }

          Maybe<RelativeControlPoint> controlPoint2;
          if (ConsumeSlash(tokens))
          {
            controlPoint2 = ConsumeRelativeControlPoint(tokens, state);
            if (!controlPoint2)
            {
              return null;
            }
          }

          return CurveCommand {.ToBy = CurveCommand::By {.Offset = krys::move(*coordinatePair),
                                                         .ControlPoint1 = krys::move(*controlPoint1),
                                                         .ControlPoint2 = krys::move(controlPoint2)}};
        }),
      *affinity);
  }

  KRYS_NODISCARD static Maybe<SmoothCommand> ConsumeShapeSmoothCommand(TokenRange &tokens,
                                                                       PropertyParserState &state) noexcept
  {
    // <smooth-command> = smooth [to <position> [with <to-control-point>]?]
    //                         | [by <coordinate-pair> [with <relative-control-point>]?]
    // https://drafts.csswg.org/css-shapes-2/#typedef-shape-smooth-command
    // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773

    auto affinity = ConsumeShapeCommandAffinity(tokens);
    if (!affinity)
    {
      return null;
    }

    return std::visit(
      CreateVisitor(
        [&](keywords::To) -> Maybe<SmoothCommand>
        {
          auto position = ConsumePositionUnresolved(tokens, state);
          if (!position)
          {
            return null;
          }

          Maybe<AbsoluteControlPoint> controlPoint;
          if (ConsumeIdentRaw<ValueId::With>(tokens))
          {
            controlPoint = ConsumeAbsoluteControlPoint(tokens, state);
            if (!controlPoint)
            {
              return null;
            }
          }

          return SmoothCommand {.ToBy = SmoothCommand::To {.Offset = krys::move(*position),
                                                           .ControlPoint = krys::move(controlPoint)}};
        },
        [&](keywords::By) -> Maybe<SmoothCommand>
        {
          auto coordinatePair = ConsumeCoordinatePair(tokens, state);
          if (!coordinatePair)
          {
            return null;
          }

          Maybe<RelativeControlPoint> controlPoint;
          if (ConsumeIdentRaw<ValueId::With>(tokens))
          {
            controlPoint = ConsumeRelativeControlPoint(tokens, state);
            if (!controlPoint)
            {
              return null;
            }
          }

          return SmoothCommand {.ToBy = SmoothCommand::By {.Offset = krys::move(*coordinatePair),
                                                           .ControlPoint = krys::move(controlPoint)}};
        }),
      *affinity);
  }

  KRYS_NODISCARD static Maybe<ArcCommand> ConsumeShapeArcCommand(TokenRange &tokens,
                                                                 PropertyParserState &state) noexcept
  {
    // <arc-command> = arc [to <position>] | [by <coordinate-pair>] of <length-percentage>{1,2} [<arc-sweep>?
    // || <arc-size>? || [rotate <angle>]?] https://drafts.csswg.org/css-shapes-2/#typedef-shape-arc-command
    // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773

    auto affinity = ConsumeShapeCommandAffinity(tokens);
    if (!affinity)
    {
      return null;
    }

    using ToBy = Variant<ArcCommand::To, ArcCommand::By>;
    auto toBy = std::visit(CreateVisitor(
                             [&](keywords::To) -> Maybe<ToBy>
                             {
                               auto position = ConsumePositionUnresolved(tokens, state);
                               if (!position)
                               {
                                 return null;
                               }
                               return ToBy {ArcCommand::To {krys::move(*position)}};
                             },
                             [&](keywords::By) -> Maybe<ToBy>
                             {
                               auto coordinatePair = ConsumeCoordinatePair(tokens, state);
                               if (!coordinatePair)
                               {
                                 return null;
                               }
                               return ToBy {ArcCommand::By {krys::move(*coordinatePair)}};
                             }),
                           *affinity);
    if (!toBy)
    {
      return null;
    }

    if (!ConsumeIdentRaw<ValueId::Of>(tokens))
    {
      return null;
    }

    auto length1 = MetaConsumer<LengthPercentage<>>::Consume(tokens, state);
    if (!length1)
    {
      return null;
    }
    auto length2 = MetaConsumer<LengthPercentage<>>::Consume(tokens, state);
    if (!length2)
    {
      length2 = length1; // Copy `length1` to `length2` if there is only one length consumed.
    }

    Maybe<ArcSweep> arcSweep;
    Maybe<ArcSize> arcSize;
    Maybe<Angle<>> angle;

    auto AtEndOfCommand = [&]
    {
      return tokens.IsAtEnd() || tokens.Peek().Type() == TokenType::Comma;
    };

    while (!AtEndOfCommand())
    {
      auto ident =
        ConsumeIdentRaw<ValueId::Cw, ValueId::Ccw, ValueId::Large, ValueId::Small, ValueId::Rotate>(tokens);
      if (!ident)
      {
        return null;
      }

      switch (*ident)
      {
        case ValueId::Cw:
        {
          if (arcSweep)
          {
            return null;
          }
          arcSweep = keywords::Cw {};
          break;
        }
        case ValueId::Ccw:
        {
          if (arcSweep)
          {
            return null;
          }
          arcSweep = keywords::Ccw {};
          break;
        }
        case ValueId::Large:
        {
          if (arcSize)
          {
            return null;
          }
          arcSize = keywords::Large {};
          break;
        }
        case ValueId::Small:
        {
          if (arcSize)
          {
            return null;
          }
          arcSize = keywords::Small {};
          break;
        }
        case ValueId::Rotate:
        {
          if (angle)
          {
            return null;
          }

          angle = MetaConsumer<Angle<>>::Consume(tokens, state);
          if (!angle)
          {
            return null;
          }
          break;
        }
        default:
        {
          break;
        }
      }
    }

    return ArcCommand {.ToBy = krys::move(*toBy),
                       .Size = {krys::move(*length1), krys::move(*length2)},
                       .ArcSweep = arcSweep.value_or(ArcSweep {keywords::Ccw {}}),
                       .ArcSize = arcSize.value_or(ArcSize {keywords::Small {}}),
                       .Rotation = angle.value_or(Angle<> {0_css_deg})};
  }

  KRYS_NODISCARD static Maybe<ShapeCommand> ConsumeShapeCommand(TokenRange &tokens,
                                                                PropertyParserState &state) noexcept
  {
    if (tokens.Peek().Type() != TokenType::Ident)
    {
      return null;
    }

    auto id = tokens.ConsumeIncludingWhitespace().ValueId();
    switch (id)
    {
      case ValueId::Move:
      {
        if (auto command = ConsumeShapeMoveCommand(tokens, state))
        {
          return ShapeCommand {krys::move(*command)};
        }
        break;
      }
      case ValueId::Line:
      {
        if (auto command = ConsumeShapeLineCommand(tokens, state))
        {
          return ShapeCommand {krys::move(*command)};
        }
        break;
      }
      case ValueId::Hline:
      {
        if (auto command = ConsumeShapeHLineCommand(tokens, state))
        {
          return ShapeCommand {krys::move(*command)};
        }
        break;
      }
      case ValueId::Vline:
      {
        if (auto command = ConsumeShapeVLineCommand(tokens, state))
        {
          return ShapeCommand {krys::move(*command)};
        }
        break;
      }
      case ValueId::Curve:
      {
        if (auto command = ConsumeShapeCurveCommand(tokens, state))
        {
          return ShapeCommand {krys::move(*command)};
        }
        break;
      }
      case ValueId::Smooth:
      {
        if (auto command = ConsumeShapeSmoothCommand(tokens, state))
        {
          return ShapeCommand {krys::move(*command)};
        }
        break;
      }
      case ValueId::Arc:
      {
        if (auto command = ConsumeShapeArcCommand(tokens, state))
        {
          return ShapeCommand {krys::move(*command)};
        }
        break;
      }
      case ValueId::Close:
      {
        return ShapeCommand {CloseCommand {}};
      }
      default:
      {
        break;
      }
    }

    return null;
  }

  KRYS_NODISCARD static Maybe<Shape>
    ConsumeBasicShapeShapeFunctionParameters(TokenRange &args, PropertyParserState &state) noexcept
  {
    // shape() = shape( <'fill-rule'>? from <coordinate-pair>, <shape-command># )
    // https://drafts.csswg.org/css-shapes-2/#shape-function

    if (!state.Context.cssShapeFunctionEnabled)
    {
      return null;
    }

    auto fillRule = ConsumeFillRule(args);

    if (!ConsumeIdentRaw<ValueId::From>(args))
    {
      return null;
    }

    // FIXME: The spec says this should be a <coordinate-pair>, but the tests and some comments indicate it
    // has changed to position.
    auto startingPoint = ConsumePositionUnresolved(args, state);
    if (!startingPoint)
    {
      return null;
    }

    if (!ConsumeComma(args))
    {
      return null;
    }

    Shape::CommandsType::Container commands;
    do
    {
      auto command = ConsumeShapeCommand(args, state);
      if (!command)
      {
        return null;
      }

      commands.push_back(krys::move(*command));
    } while (ConsumeComma(args));

    return Shape {.FillRule = krys::move(fillRule),
                  .StartingPoint = krys::move(*startingPoint),
                  .Commands = {krys::move(commands)}};
  }

  // MARK: - <rect()>

  KRYS_NODISCARD static Maybe<RectShape::Edge> ConsumeBasicShapeRectEdge(TokenRange &args,
                                                                         PropertyParserState &state) noexcept
  {
    // <rect-edge> = [ <length-percentage> | auto ]

    if (args.Peek().Type() == TokenType::Ident)
    {
      if (args.Peek().ValueId() == ValueId::Auto)
      {
        args.Discard();
        args.DiscardWhitespace();
        return RectShape::Edge {keywords::Auto {}};
      }
      return null;
    }

    if (auto edge = MetaConsumer<LengthPercentage<>>::Consume(args, state))
    {
      return RectShape::Edge {krys::move(*edge)};
    }

    return null;
  }

  KRYS_NODISCARD static Maybe<SpaceSeparatedRectEdges<RectShape::Edge>>
    ConsumeBasicShapeRectEdges(TokenRange &args, PropertyParserState &state) noexcept
  {
    // <rect-edges> = <rect-edge>{4}

    auto top = ConsumeBasicShapeRectEdge(args, state);
    if (!top)
    {
      return null;
    }
    auto right = ConsumeBasicShapeRectEdge(args, state);
    if (!right)
    {
      return null;
    }
    auto bottom = ConsumeBasicShapeRectEdge(args, state);
    if (!bottom)
    {
      return null;
    }
    auto left = ConsumeBasicShapeRectEdge(args, state);
    if (!left)
    {
      return null;
    }
    return SpaceSeparatedRectEdges<RectShape::Edge> {krys::move(*top), krys::move(*right),
                                                     krys::move(*bottom), krys::move(*left)};
  }

  KRYS_NODISCARD static Maybe<RectShape>
    ConsumeBasicShapeRectFunctionParameters(TokenRange &args, PropertyParserState &state) noexcept
  {
    // <rect()> = rect( [ <length-percentage> | auto ]{4} [ round <'border-radius'> ]? )
    // https://drafts.csswg.org/css-shapes-1/#funcdef-basic-shape-rect

    auto edges = ConsumeBasicShapeRectEdges(args, state);
    if (!edges)
    {
      return null;
    }

    Maybe<BorderRadius> radii;
    if (ConsumeIdentRaw<ValueId::Round>(args))
    {
      radii = ConsumeUnresolvedBorderRadius(args, state);
      if (!radii)
      {
        return null;
      }
    }

    return RectShape {.Edges = krys::move(*edges), .Radii = radii.value_or(BorderRadius::DefaultValue())};
  }

  // MARK: - <xywh()>

  KRYS_NODISCARD static Maybe<Xywh>
    ConsumeBasicShapeXywhFunctionParameters(TokenRange &args, PropertyParserState &state) noexcept
  {
    // <xywh()> = xywh( <length-percentage>{2} <length-percentage [0,∞]>{2} [ round <'border-radius'> ]? )
    // https://drafts.csswg.org/css-shapes-1/#funcdef-basic-shape-xywh

    auto location = ConsumePair<Xywh::LocationType>(args, state);
    if (!location)
    {
      return null;
    }
    auto size = ConsumePair<Xywh::SizeType>(args, state);
    if (!size)
    {
      return null;
    }

    Maybe<BorderRadius> radii;
    if (ConsumeIdentRaw<ValueId::Round>(args))
    {
      radii = ConsumeUnresolvedBorderRadius(args, state);
      if (!radii)
      {
        return null;
      }
    }

    return Xywh {.Location = krys::move(*location),
                 .Size = krys::move(*size),
                 .Radii = radii.value_or(BorderRadius::DefaultValue())};
  }

  // MARK: - <inset()>

  KRYS_NODISCARD static Maybe<Inset::InsetsType>
    ConsumeBasicShapeInsetInsets(TokenRange &args, PropertyParserState &state) noexcept
  {
    // <insets> = <length-percentage>{1,4}

    auto inset1 = MetaConsumer<LengthPercentage<>>::Consume(args, state);
    if (!inset1)
    {
      return null;
    }

    auto inset2 = MetaConsumer<LengthPercentage<>>::Consume(args, state);
    if (!inset2)
    {
      return CompleteQuad<Inset::InsetsType>(krys::move(*inset1));
    }

    auto inset3 = MetaConsumer<LengthPercentage<>>::Consume(args, state);
    if (!inset3)
    {
      return CompleteQuad<Inset::InsetsType>(krys::move(*inset1), krys::move(*inset2));
    }

    auto inset4 = MetaConsumer<LengthPercentage<>>::Consume(args, state);
    if (!inset4)
    {
      return CompleteQuad<Inset::InsetsType>(krys::move(*inset1), krys::move(*inset2), krys::move(*inset3));
    }

    return Inset::InsetsType {krys::move(*inset1), krys::move(*inset2), krys::move(*inset3),
                              krys::move(*inset4)};
  }

  KRYS_NODISCARD static Maybe<Inset>
    ConsumeBasicShapeInsetFunctionParameters(TokenRange &args, PropertyParserState &state) noexcept
  {
    // <inset()> = inset( <length-percentage>{1,4} [ round <'border-radius'> ]? )
    // https://drafts.csswg.org/css-shapes-1/#funcdef-basic-shape-inset

    auto insets = ConsumeBasicShapeInsetInsets(args, state);
    if (!insets)
    {
      return null;
    }

    Maybe<BorderRadius> radii;
    if (ConsumeIdentRaw<ValueId::Round>(args))
    {
      radii = ConsumeUnresolvedBorderRadius(args, state);
      if (!radii)
      {
        return null;
      }
    }

    return Inset {.Insets = krys::move(*insets), .Radii = radii.value_or(BorderRadius::DefaultValue())};
  }

  // MARK: - <basic-shape>

  RefPtr<Value> ConsumeBasicShape(TokenRange &tokens, PropertyParserState &state,
                                  PathParsingOption options) noexcept
  {
    // <basic-shape> = <circle()> | <ellipse() | <inset()> | <path()> | <polygon()> | <rect()> | <shape()> |
    // <xywh()>
    // https://drafts.csswg.org/css-shapes/#typedef-basic-shape

    auto tokensCopy = tokens;

    if (tokensCopy.Peek().Type() != TokenType::Function)
    {
      return {};
    }

    auto id = tokensCopy.Peek().FunctionId();
    auto args = ConsumeFunction(tokensCopy);

    Maybe<BasicShape> result;
    if (id == ValueId::Circle)
    {
      result = ToBasicShape<ValueId::Circle>(ConsumeBasicShapeCircleFunctionParameters(args, state));
    }
    else if (id == ValueId::Ellipse)
    {
      result = ToBasicShape<ValueId::Ellipse>(ConsumeBasicShapeEllipseFunctionParameters(args, state));
    }
    else if (id == ValueId::Polygon)
    {
      result = ToBasicShape<ValueId::Polygon>(ConsumeBasicShapePolygonFunctionParameters(args, state));
    }
    else if (id == ValueId::Inset)
    {
      result = ToBasicShape<ValueId::Inset>(ConsumeBasicShapeInsetFunctionParameters(args, state));
    }
    else if (id == ValueId::Rect)
    {
      result = ToBasicShape<ValueId::Rect>(ConsumeBasicShapeRectFunctionParameters(args, state));
    }
    else if (id == ValueId::Xywh)
    {
      result = ToBasicShape<ValueId::Xywh>(ConsumeBasicShapeXywhFunctionParameters(args, state));
    }
    else if (id == ValueId::Path)
    {
      result = ToBasicShape<ValueId::Path>(ConsumeBasicShapePathFunctionParameters(args, options));
    }
    else if (id == ValueId::Shape)
    {
      result = ToBasicShape<ValueId::Shape>(ConsumeBasicShapeShapeFunctionParameters(args, state));
    }

    if (!result || !args.IsAtEnd())
    {
      return {};
    }

    tokens = tokensCopy;
    return BasicShapeValue::Create(krys::move(*result));
  }

  RefPtr<Value> ConsumePath(TokenRange &tokens, PropertyParserState &) noexcept
  {
    // <path()> = path( <'fill-rule'>? , <string> )
    // https://drafts.csswg.org/css-shapes/#funcdef-basic-shape-path

    if (tokens.Peek().Type() != TokenType::Function)
    {
      return nullptr;
    }
    if (tokens.Peek().FunctionId() != ValueId::Path)
    {
      return nullptr;
    }

    auto args = ConsumeFunction(tokens);
    auto result = ConsumeBasicShapePathFunctionParameters(args, PathParsingOption::None);
    if (!result || !args.IsAtEnd())
    {
      return nullptr;
    }

    return PathValue::Create(PathFunction {.Parameters = krys::move(*result)});
  }

  RefPtr<Value> ConsumeShapeOutside(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'shape-outside'> = none | [ <basic-shape> || <shape-box> ] | <image>
    // https://drafts.csswg.org/css-shapes-1/#propdef-shape-outside

    if (auto imageOrNoneValue = ConsumeImageOrNone(tokens, state))
    {
      return imageOrNoneValue;
    }

    ValueListBuilder list;
    auto boxValue = PropertyParsing::ConsumeShapeBox(tokens);
    bool hasShapeValue = false;

    // FIXME: The spec says we should allows `path()` functions.
    if (auto basicShape = ConsumeBasicShape(tokens, state, PathParsingOption::RejectPath))
    {
      list.push_back(krys::move(basicShape));
      hasShapeValue = true;
    }
    if (!boxValue)
    {
      boxValue = PropertyParsing::ConsumeShapeBox(tokens);
    }

    // margin-box is the default.
    if (boxValue && (boxValue->ValueId() != ValueId::MarginBox || !hasShapeValue))
    {
      list.push_back(krys::move(boxValue));
    }

    if (list.empty())
    {
      return nullptr;
    }

    return ValueList::CreateSpaceSeparated(krys::move(list));
  }
}
