#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Shapes/FillRule.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathByteStream.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  // <path()> = path( <'fill-rule'>? , <string> )
  // https://drafts.csswg.org/css-shapes-1/#funcdef-basic-shape-path
  struct Path
  {
    struct PathData
    {
      svg::SVGPathByteStream ByteStream;

      KRYS_NODISCARD bool operator==(const PathData &) const noexcept = default;
    };

    Maybe<boo::css::FillRule> FillRule;
    PathData Data;

    KRYS_NODISCARD bool operator==(const Path &) const noexcept = default;
  };

  using PathFunction = FunctionNotation<ValueId::Path, Path>;

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Path &value) noexcept
  {
    if constexpr (!I)
    {
      return value.FillRule;
    }
    else if constexpr (I == 1)
    {
      return value.Data;
    }
  }

  template <>
  struct Serialiser<Path>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context, const Path &value) noexcept;
  };

  template <>
  struct ComputedStyleDependenciesCollector<Path::PathData>
  {
    void operator()(ComputedStyleDependencies &dependencies, const Path::PathData &data) noexcept;
  };

  template <>
  struct ValueChildrenVisitor<Path::PathData>
  {
    KRYS_NODISCARD IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                                              const Path::PathData &data) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Path, 2)
