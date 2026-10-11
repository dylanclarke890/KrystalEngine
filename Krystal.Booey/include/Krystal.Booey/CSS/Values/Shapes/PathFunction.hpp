#pragma once

#include "Krystal.Booey/CSS/Values/Shapes/FillRule.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathByteStream.hpp"

namespace krys::boo::css
{
  // <path()> = path( <'fill-rule'>? , <string> )
  // https://drafts.csswg.org/css-shapes-1/#funcdef-basic-shape-path
  struct Path
  {
    struct PathData
    {
      svg::SVGPathByteStream ByteStream;

      bool operator==(const PathData &) const = default;
    };

    Maybe<FillRule> FillRule;
    PathData Data;

    bool operator==(const Path &) const = default;
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
    void operator()(CSSOMString &builder, const SerialisationContext &context, const Path &value) noexcept;
  };

  template <>
  struct ComputedStyleDependenciesCollector<Path::PathData>
  {
    void operator()(ComputedStyleDependencies &, const Path::PathData &) noexcept;
  };

  template <>
  struct ValueChildrenVisitor<Path::PathData>
  {
    IterationStatus operator()(const Func<IterationStatus(css::Value &)> &, const Path::PathData &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Path, 2)
