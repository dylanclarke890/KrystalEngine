#pragma once

#include "Krystal.Booey/Style/Values/Shapes/PathFunction.hpp"

namespace krys::boo::style
{
  // <'d'> = none | <path()>
  // https://svgwg.org/svg2-draft/paths.html#DProperty
  // NOTE: The type is `SVGPathData` as `D` is just a bit too opaque.
  struct SVGPathData
  {
    SVGPathData(css::keywords::None) noexcept
    {
    }

    SVGPathData(PathFunction &&path) noexcept : _path {krys::move(path)}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return !_path;
    }
    KRYS_NODISCARD bool IsPath() const noexcept
    {
      return !!_path;
    }
    KRYS_NODISCARD const Maybe<PathFunction> &TryPath() const noexcept KRYS_LIFETIME_BOUND
    {
      return _path;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(css::keywords::None {});
      }
      return visitor(*_path);
    }

    KRYS_NODISCARD bool operator==(const SVGPathData &) const noexcept = default;

  private:
    Maybe<PathFunction> _path {};
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<SVGPathData>
  {
    KRYS_NODISCARD SVGPathData operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<SVGPathData>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const SVGPathData &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<SVGPathData>
  {
    void operator()(css::CSSOMString &, const css::SerialisationContext &, const RenderStyle &,
                    const SVGPathData &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<SVGPathData>
  {
    KRYS_NODISCARD bool CanBlend(const SVGPathData &, const SVGPathData &) noexcept;
    KRYS_NODISCARD SVGPathData Blend(const SVGPathData &, const SVGPathData &,
                                     const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SVGPathData)
