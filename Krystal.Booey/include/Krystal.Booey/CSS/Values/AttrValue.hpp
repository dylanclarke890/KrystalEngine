#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief An `attr()` value: an attribute name and an optional fallback, e.g. `attr(data-label, "none")`.
  class AttrValue final : public Value
  {
  private:
    CSSOMString _attributeName;
    RefPtr<Value> _fallback;

    AttrValue(CSSOMString &&attributeName, RefPtr<Value> &&fallback) noexcept
        : Value(ValueType::Attr), _attributeName(krys::move(attributeName)), _fallback(krys::move(fallback))
    {
    }

  public:
    KRYS_NODISCARD static Ref<AttrValue> Create(CSSOMString attributeName,
                                                RefPtr<Value> fallback = nullptr) noexcept;

    KRYS_NODISCARD const CSSOMString AttributeName() const noexcept
    {
      return _attributeName;
    }

    KRYS_NODISCARD const Value *Fallback() const noexcept
    {
      return _fallback.get();
    }

    KRYS_NODISCARD bool Equals(const AttrValue &other) const noexcept;
    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::AttrValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsAttrValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
