#pragma once

#include "Krystal.Booey/CSS/TypedOM/CSSNumericValue.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo::css
{
  /// @brief The operands of a math value.
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssnumericarray
  class CSSNumericArray : public RefCounted<CSSNumericArray>
  {
    KRYS_NON_COPY_MOVABLE(CSSNumericArray);

  private:
    List<Ref<CSSNumericValue>> _array;

    explicit CSSNumericArray(List<Ref<CSSNumericValue>> &&values) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSNumericArray> Create(List<CSSNumberish> &&numberishes) noexcept;
    KRYS_NODISCARD static Ref<CSSNumericArray> Create(List<Ref<CSSNumericValue>> &&values) noexcept;

    KRYS_NODISCARD size_t Length() const noexcept
    {
      return _array.size();
    }
    /// @return Null if @p index is out of range.
    KRYS_NODISCARD RefPtr<CSSNumericValue> Item(size_t index) noexcept;
    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) const noexcept
    {
      return index < _array.size();
    }
    KRYS_NODISCARD const List<Ref<CSSNumericValue>> &Array() const noexcept
    {
      return _array;
    }
    void ForEach(const Func<void(const CSSNumericValue &, bool first)> &function) noexcept;
  };
}
