#pragma once

#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  enum class ValueId : uint16;

  /// @brief Stores the track sizes and line numbers when the auto-repeat syntax is used.
  /// @note Right now the auto-repeat syntax is as follows:
  /// `<auto-repeat>  = repeat( [ auto-fill | auto-fit ], <line-names>? <fixed-size> <line-names>? )`
  ///
  /// meaning that only one fixed size track is allowed. It could be argued that a different class storing two
  /// GridLineNamesValue and one Value (for the track size) fits better but the CSSWG has left the door open
  /// to allow more than one track in the future. That's why we're using a list, it's prepared for future
  /// changes and it also allows us to keep the parsing algorithm almost intact.
  class GridAutoRepeatValue final : public ValueContainingList
  {
  private:
    bool _isAutoFit {false};

    GridAutoRepeatValue(bool isAutoFit, ValueListBuilder builder) noexcept;

  public:
    /// @param id `auto-fill` or `auto-fit`.
    KRYS_NODISCARD static Ref<GridAutoRepeatValue> Create(css::ValueId id, ValueListBuilder builder) noexcept;

    KRYS_NODISCARD css::ValueId AutoRepeatID() const noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const GridAutoRepeatValue &other) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::GridAutoRepeatValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsGridAutoRepeatValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
