#include "Krystal.Booey/CSS/Values/Shapes/PathFunction.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathUtilities.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  void Serialiser<Path>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                    const Path &value) noexcept
  {
    // <path()> = path( <'fill-rule'>? , <string> )

    if (value.FillRule && !std::holds_alternative<keywords::Nonzero>(*value.FillRule))
    {
      Serialise(builder, context, *value.FillRule);
      builder.Append(u8", ");
    }

    // FIXME: Add version of `buildStringFromByteStream` that takes a `StringBuilder`.
    CSSOMString pathString;
    (void)svg::BuildStringFromByteStream(value.Data.ByteStream, pathString,
                                         svg::PathParsingMode::UnalteredParsing);
    SerialiseString(pathString, builder);
  }

  void ComputedStyleDependenciesCollector<Path::PathData>::operator()(ComputedStyleDependencies &,
                                                                      const Path::PathData &) noexcept
  {
  }

  IterationStatus
    ValueChildrenVisitor<Path::PathData>::operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &,
                                                     const Path::PathData &) noexcept
  {
    return IterationStatus::Continue;
  }
}
