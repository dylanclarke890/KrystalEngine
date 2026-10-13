#pragma once

#include "Krystal.Booey/CSS/Values/Shapes/PathFunction.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  // `PathValue` is used to represent a path for the `d` property from SVG.
  // https://svgwg.org/svg2-draft/paths.html#DProperty
  /// @brief A `path()` value, e.g. `path("M 0 0 L 10 10")`.
  class PathValue final : public Value
  {
  private:
    PathFunction _path;

    explicit PathValue(PathFunction &&path) noexcept;

  public:
    KRYS_NODISCARD static Ref<PathValue> Create(PathFunction path) noexcept;

    KRYS_NODISCARD const PathFunction &GetPath() const noexcept
    {
      return _path;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const PathValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::PathValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsPathValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
