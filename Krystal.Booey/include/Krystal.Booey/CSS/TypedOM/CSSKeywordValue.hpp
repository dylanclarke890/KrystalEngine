#pragma once

#include "Krystal.Booey/CSS/TypedOM/CSSStyleValue.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::css
{
  class CSSKeywordValue;

  /// @brief A keyword, as a string or a keyword value.
  /// @see https://drafts.css-houdini.org/css-typed-om/#typedefdef-csskeywordish
  using CSSKeywordish = Variant<CSSOMString, RefPtr<CSSKeywordValue>>;

  /// @brief A CSS keyword, or an identifier.
  /// @see https://drafts.css-houdini.org/css-typed-om/#csskeywordvalue
  class CSSKeywordValue final : public CSSStyleValue
  {
  private:
    CSSOMString _value;

  public:
    /// @return A `TypeError` if @p value is empty.
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSKeywordValue>> Create(const CSSOMString &value) noexcept;

    KRYS_NODISCARD const CSSOMString &Value() const noexcept
    {
      return _value;
    }
    /// @return A `TypeError`, leaving the value unchanged, if @p value is empty.
    dom::ExceptionOr<void> SetValue(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSStyleValueType StyleValueType() const noexcept final
    {
      return CSSStyleValueType::CSSKeywordValue;
    }

    /// @see https://drafts.css-houdini.org/css-typed-om/#rectify-a-keywordish-value
    KRYS_NODISCARD static Ref<CSSKeywordValue> RectifyKeywordish(CSSKeywordish &&keywordish) noexcept;

    KRYS_NODISCARD RefPtr<css::Value> ToCSSValue() const noexcept final;

  private:
    void Serialise(StringBuilder &builder, EnumSet<SerialisationArguments> arguments) const noexcept final;

    explicit CSSKeywordValue(const CSSOMString &value) noexcept : _value(value)
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSKeywordValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return styleValue.StyleValueType() == krys::boo::css::CSSStyleValueType::CSSKeywordValue;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
