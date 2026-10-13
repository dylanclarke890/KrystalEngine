#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleImage;
}

namespace krys::boo::style
{
  class BuilderState;
}

namespace krys::boo::css
{
  /// @brief The value of a `paint()` function.
  /// @note https://drafts.css-houdini.org/css-paint-api/#paint-notation
  class PaintImageValue final : public Value
  {
  private:
    CSSOMString _name;
    const Ref<VariableData> _arguments;

    PaintImageValue(CSSOMString &&name, Ref<VariableData> &&arguments) noexcept;

  public:
    KRYS_NODISCARD static Ref<PaintImageValue> Create(CSSOMString name, Ref<VariableData> arguments) noexcept;

    KRYS_NODISCARD const CSSOMString &Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD bool Equals(const PaintImageValue &other) const noexcept
    {
      return _name == other._name;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    // FIXME: not defined until style::BuilderState and StyleImage are ported.
    KRYS_NODISCARD RefPtr<StyleImage> CreateStyleImage(const style::BuilderState &state) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::PaintImageValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsPaintImageValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
