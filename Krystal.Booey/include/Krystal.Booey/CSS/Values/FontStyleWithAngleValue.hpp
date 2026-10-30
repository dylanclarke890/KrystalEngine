#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `font-style: oblique <angle>` value.
  /// @note This class is currently only used for oblique. If we use it for more styles in the future we'll
  /// need to store the keyword.
  class FontStyleWithAngleValue final : public Value
  {
  public:
    using ObliqueAngle = Angle<Range {-90, 90}>;

  private:
    ObliqueAngle _obliqueAngle;

    explicit FontStyleWithAngleValue(ObliqueAngle &&obliqueAngle) noexcept;

  public:
    KRYS_NODISCARD static Ref<FontStyleWithAngleValue> Create(ObliqueAngle obliqueAngle) noexcept;

    KRYS_NODISCARD const ObliqueAngle &GetObliqueAngle() const noexcept
    {
      return _obliqueAngle;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const FontStyleWithAngleValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontStyleWithAngleValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsFontStyleWithAngleValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
