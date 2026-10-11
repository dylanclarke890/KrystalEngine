#pragma once

#include "Krystal.Booey/CSS/Values/GlobalKeyword.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"
#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"

namespace krys::boo::css
{
  class CustomPropertyValue final : public Value
  {
  public:
    using VariantValue = Variant<Ref<VariableReferenceValue>, Ref<VariableData>, GlobalKeyword>;

  private:
    const CSSOMStringAtom _name;
    const VariantValue _value;
    mutable CSSOMString _cachedCSSText;

    CustomPropertyValue(const CSSOMStringAtom &name, VariantValue &&value) noexcept
        : css::Value(ValueType::CustomProperty), _name(name), _value(krys::move(value))
    {
    }

  public:
    KRYS_NODISCARD static Ref<CustomPropertyValue> CreateEmpty(const CSSOMStringAtom &name) noexcept;

    KRYS_NODISCARD static Ref<CustomPropertyValue> CreateUnresolved(const CSSOMStringAtom &name,
                                                                    Ref<VariableReferenceValue> &&) noexcept;

    KRYS_NODISCARD static Ref<CustomPropertyValue> CreateSyntaxAll(const CSSOMStringAtom &name,
                                                                   Ref<VariableData> &&) noexcept;

    KRYS_NODISCARD static Ref<CustomPropertyValue> CreateWithCSSWideKeyword(const CSSOMStringAtom &name,
                                                                            GlobalKeyword) noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD const VariantValue &Value() const noexcept
    {
      return _value;
    }

    KRYS_NODISCARD Ref<const VariableData> AsVariableData() const noexcept;

    KRYS_NODISCARD bool IsCurrentColor() const noexcept;

    KRYS_NODISCARD bool IsVariableReference() const noexcept;
    KRYS_NODISCARD bool IsVariableData() const noexcept;
    KRYS_NODISCARD bool IsGlobalKeyword() const noexcept;

    KRYS_NODISCARD Maybe<GlobalKeyword> TryGlobalKeyword() const noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const CustomPropertyValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(const Func<IterationStatus(css::Value &)> &func) const noexcept;

  private:
    KRYS_NODISCARD const SmallList<Token> &Tokens() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CustomPropertyValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsCustomPropertyValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()