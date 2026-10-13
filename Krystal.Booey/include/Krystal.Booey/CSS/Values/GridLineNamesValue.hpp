#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/FixedVector.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::css
{
  /// @brief A bracketed list of grid line names, e.g. `[header-start main-start]`.
  class GridLineNamesValue final : public Value
  {
  private:
    FixedVector<CSSOMString> _names;

    explicit GridLineNamesValue(Span<const CSSOMString> names) noexcept;

  public:
    KRYS_NODISCARD static Ref<GridLineNamesValue> Create(Span<const CSSOMString> names) noexcept;

    KRYS_NODISCARD Span<const CSSOMString> Names() const noexcept
    {
      return _names.span();
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const GridLineNamesValue &other) const noexcept
    {
      return _names == other._names;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::GridLineNamesValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsGridLineNamesValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
