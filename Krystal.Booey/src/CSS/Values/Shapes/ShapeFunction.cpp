#include "Krystal.Booey/CSS/Values/Shapes/ShapeFunction.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::css
{
  namespace
  {
    void AppendKeyword(CSSOMString &builder, ValueId keyword) noexcept
    {
      builder.append(ToLower(keyword));
    }

    // Writes ` <keyword> `.
    void AppendSpacedKeyword(CSSOMString &builder, ValueId keyword) noexcept
    {
      builder.append(u8" ");
      AppendKeyword(builder, keyword);
      builder.append(u8" ");
    }

    // Writes `<affinity> <offset>`, shared by the `to`/`by` halves of every command.
    template <typename T>
    void SerialiseAffinityAndOffset(CSSOMString &builder, const SerialisationContext &context,
                                    const T &value) noexcept
    {
      AppendKeyword(builder, value.Affinity.Value);
      builder.append(u8" ");
      Serialise(builder, context, value.Offset);
    }

    // Writes `<name> <to-by>`, shared by every command.
    template <typename T>
    void SerialiseNameAndToBy(CSSOMString &builder, const SerialisationContext &context,
                              const T &value) noexcept
    {
      AppendKeyword(builder, value.Name);
      builder.append(u8" ");
      Serialise(builder, context, value.ToBy);
    }

    template <typename T>
    void SerialiseControlPoint(CSSOMString &builder, const SerialisationContext &context,
                               const T &value) noexcept
    {
      Serialise(builder, context, value.Offset);

      if (value.Anchor)
      {
        AppendSpacedKeyword(builder, ValueId::From);
        Serialise(builder, context, *value.Anchor);
      }
    }

    template <typename T>
    void SerialiseCurveToBy(CSSOMString &builder, const SerialisationContext &context,
                            const T &value) noexcept
    {
      SerialiseAffinityAndOffset(builder, context, value);

      AppendSpacedKeyword(builder, ValueId::With);
      Serialise(builder, context, value.ControlPoint1);
      if (value.ControlPoint2)
      {
        builder.append(u8" / ");
        Serialise(builder, context, *value.ControlPoint2);
      }
    }

    template <typename T>
    void SerialiseSmoothToBy(CSSOMString &builder, const SerialisationContext &context,
                             const T &value) noexcept
    {
      SerialiseAffinityAndOffset(builder, context, value);

      if (value.ControlPoint)
      {
        AppendSpacedKeyword(builder, ValueId::With);
        Serialise(builder, context, *value.ControlPoint);
      }
    }
  }

  void Serialiser<ToPosition>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                          const ToPosition &value) noexcept
  {
    // <to-position> = to <position>

    SerialiseAffinityAndOffset(builder, context, value);
  }

  void Serialiser<ByCoordinatePair>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                                const ByCoordinatePair &value) noexcept
  {
    // <by-coordinate-pair> = by <coordinate-pair>

    SerialiseAffinityAndOffset(builder, context, value);
  }

  void Serialiser<RelativeControlPoint>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                                    const RelativeControlPoint &value) noexcept
  {
    // <relative-control-point> = [<coordinate-pair> [from [start | end | origin]]?]
    // Specified https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773

    SerialiseControlPoint(builder, context, value);
  }

  void Serialiser<AbsoluteControlPoint>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                                    const AbsoluteControlPoint &value) noexcept
  {
    // <to-control-point> = [<position> | <relative-control-point>]
    // Specified https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773

    // Representation diverges from grammar due to overlap between <position> and <relative-control-point>.

    SerialiseControlPoint(builder, context, value);
  }

  void Serialiser<MoveCommand>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                           const MoveCommand &value) noexcept
  {
    // <move-command> = move [to <position>] | [by <coordinate-pair>]
    // https://drafts.csswg.org/css-shapes-2/#typedef-shape-move-command
    // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773

    SerialiseNameAndToBy(builder, context, value);
  }

  void Serialiser<LineCommand>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                           const LineCommand &value) noexcept
  {
    // <line-command> = line [to <position>] | [by <coordinate-pair>]
    // https://drafts.csswg.org/css-shapes-2/#typedef-shape-line-command
    // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773

    SerialiseNameAndToBy(builder, context, value);
  }

  void Serialiser<HLineCommand::To>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                                const HLineCommand::To &value) noexcept
  {
    SerialiseAffinityAndOffset(builder, context, value);
  }

  void Serialiser<HLineCommand::By>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                                const HLineCommand::By &value) noexcept
  {
    SerialiseAffinityAndOffset(builder, context, value);
  }

  void Serialiser<HLineCommand>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                            const HLineCommand &value) noexcept
  {
    // <horizontal-line-command> = hline [ to [ <length-percentage> | left | center | right
    //                                           | x-start | x-end ]
    //                                  | by <length-percentage> ]
    // https://drafts.csswg.org/css-shapes-2/#typedef-shape-hv-line-command
    // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2426552611

    SerialiseNameAndToBy(builder, context, value);
  }

  void Serialiser<VLineCommand::To>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                                const VLineCommand::To &value) noexcept
  {
    SerialiseAffinityAndOffset(builder, context, value);
  }

  void Serialiser<VLineCommand::By>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                                const VLineCommand::By &value) noexcept
  {
    SerialiseAffinityAndOffset(builder, context, value);
  }

  void Serialiser<VLineCommand>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                            const VLineCommand &value) noexcept
  {
    // <vertical-line-command> = vline [ to [ <length-percentage> | top | center | bottom | y-start | y-end ]
    //                                | by <length-percentage> ]
    // https://drafts.csswg.org/css-shapes-2/#typedef-shape-hv-line-command
    // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2426552611

    SerialiseNameAndToBy(builder, context, value);
  }

  void Serialiser<CurveCommand::To>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                                const CurveCommand::To &value) noexcept
  {
    SerialiseCurveToBy(builder, context, value);
  }

  void Serialiser<CurveCommand::By>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                                const CurveCommand::By &value) noexcept
  {
    SerialiseCurveToBy(builder, context, value);
  }

  void Serialiser<CurveCommand>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                            const CurveCommand &value) noexcept
  {
    // <curve-command> = curve [to <position> with <to-control-point> [/ <to-control-point>]?]
    //                       | [by <coordinate-pair> with <relative-control-point>
    //                          [/ <relative-control-point>]?]
    // https://drafts.csswg.org/css-shapes-2/#typedef-shape-curve-command
    // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773

    SerialiseNameAndToBy(builder, context, value);
  }

  void Serialiser<SmoothCommand::To>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                                 const SmoothCommand::To &value) noexcept
  {
    SerialiseSmoothToBy(builder, context, value);
  }

  void Serialiser<SmoothCommand::By>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                                 const SmoothCommand::By &value) noexcept
  {
    SerialiseSmoothToBy(builder, context, value);
  }

  void Serialiser<SmoothCommand>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                             const SmoothCommand &value) noexcept
  {
    // <smooth-command> = smooth [to <position> [with <to-control-point>]?]
    //                         | [by <coordinate-pair> [with <relative-control-point>]?]
    // https://drafts.csswg.org/css-shapes-2/#typedef-shape-smooth-command
    // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773

    SerialiseNameAndToBy(builder, context, value);
  }

  void Serialiser<ArcCommand>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                          const ArcCommand &value) noexcept
  {
    // <arc-command> = arc [to <position>] | [by <coordinate-pair>] of <length-percentage>{1,2}
    //                     [<arc-sweep>? || <arc-size>? || [rotate <angle>]?]
    // https://drafts.csswg.org/css-shapes-2/#typedef-shape-arc-command
    // Modified by https://github.com/w3c/csswg-drafts/issues/10649#issuecomment-2412816773

    SerialiseNameAndToBy(builder, context, value);

    AppendSpacedKeyword(builder, ValueId::Of);
    if (value.Size.Width() == value.Size.Height())
    {
      Serialise(builder, context, value.Size.Width());
    }
    else
    {
      Serialise(builder, context, value.Size);
    }

    if (!std::holds_alternative<keywords::Ccw>(value.ArcSweep))
    {
      builder.append(u8" ");
      Serialise(builder, context, value.ArcSweep);
    }

    if (!std::holds_alternative<keywords::Small>(value.ArcSize))
    {
      builder.append(u8" ");
      Serialise(builder, context, value.ArcSize);
    }

    if (value.Rotation != 0_css_deg)
    {
      AppendSpacedKeyword(builder, ValueId::Rotate);
      Serialise(builder, context, value.Rotation);
    }
  }

  void Serialiser<Shape>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                     const Shape &value) noexcept
  {
    // shape() = shape( <'fill-rule'>? from <coordinate-pair>, <shape-command>#)

    if (value.FillRule && !std::holds_alternative<keywords::Nonzero>(*value.FillRule))
    {
      Serialise(builder, context, *value.FillRule);
      builder.append(u8" ");
    }

    AppendKeyword(builder, ValueId::From);
    builder.append(u8" ");
    Serialise(builder, context, value.StartingPoint);
    builder.append(u8", ");
    Serialise(builder, context, value.Commands);
  }
}
