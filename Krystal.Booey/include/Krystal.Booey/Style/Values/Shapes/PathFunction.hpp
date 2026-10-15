#pragma once

#include "Krystal.Booey/CSS/Values/Shapes/PathFunction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Shapes/FillRule.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PathComputation.hpp"
#include "Krystal.Booey/Style/Values/Shapes/WindRuleComputation.hpp"

namespace krys::boo::style
{
  enum class PathConversion : bool
  {
    None,
    ForceAbsolute
  };

  struct Path
  {
    struct PathData
    {
      svg::SVGPathByteStream ByteStream;

      KRYS_NODISCARD bool operator==(const PathData &) const noexcept = default;
    };

    Maybe<style::FillRule> FillRule;
    PathData Data;

    float Zoom {1};

    KRYS_NODISCARD bool operator==(const Path &) const noexcept = default;
  };
  using PathFunction = css::FunctionNotation<css::ValueId::Path, Path>;

  template <size_t I>
  const auto &get(const Path &value)
  {
    if constexpr (!I)
    {
      return value.FillRule;
    }
    else if constexpr (I == 1)
    {
      return value.Data;
    }
    else if constexpr (I == 2)
    {
      return value.Zoom;
    }
  }

  // MARK: - Conversion

  // Non-standard parameters, `conversion` and `zoom`, are needed in some instances of Style <-> CSS
  // conversions for Path.

  template <>
  struct ToCSS<Path>
  {
    KRYS_NODISCARD css::Path operator()(const Path &, const RenderStyle &,
                                        PathConversion = PathConversion::None) noexcept;
  };
  template <>
  struct ToStyle<css::Path>
  {
    KRYS_NODISCARD Path operator()(const css::Path &, const BuilderState &,
                                   Maybe<float> zoom = 1.0f) noexcept;
  };

  template <>
  struct ToCSS<Path::PathData>
  {
    KRYS_NODISCARD css::Path::PathData operator()(const Path::PathData &, const RenderStyle &,
                                              PathConversion = PathConversion::None) noexcept;
  };
  template <>
  struct ToStyle<css::Path::PathData>
  {
    KRYS_NODISCARD Path::PathData operator()(const css::Path::PathData &, const BuilderState &) noexcept;
  };

  template <>
  struct CSSValueCreation<PathFunction>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &, const PathFunction &,
                                              PathConversion = PathConversion::None) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<Path>
  {
    void operator()(css::CSSOMString &, const css::SerialisationContext &, const RenderStyle &, const Path &,
                    PathConversion = PathConversion::None) noexcept;
  };

  // MARK: - Path

  template <>
  struct PathComputation<Path>
  {
    KRYS_NODISCARD boo::Path operator()(const Path &, const FloatRect &) noexcept;
  };

  // MARK: - Wind Rule

  template <>
  struct WindRuleComputation<Path>
  {
    KRYS_NODISCARD boo::WindRule operator()(const Path &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<Path>
  {
    KRYS_NODISCARD bool CanBlend(const Path &, const Path &) noexcept;
    KRYS_NODISCARD Path Blend(const Path &, const Path &, const BlendingContext &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::Path, 2)
