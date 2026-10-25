#include "Krystal.Booey/CSS/TypedOM/CSSKeywordValue.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"

namespace krys::boo::css
{
  Ref<CSSKeywordValue> CSSKeywordValue::RectifyKeywordish(CSSKeywordish &&keywordish) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#rectify-a-keywordish-value
    return krys::SwitchOn(
      keywordish, [](const CSSOMString &string) { return AdoptRef(*new CSSKeywordValue(string)); },
      [](RefPtr<CSSKeywordValue> &value)
      {
        krys_assert(value);
        return Ref<CSSKeywordValue> {krys::move(value)};
      });
  }

  dom::ExceptionOr<Ref<CSSKeywordValue>> CSSKeywordValue::Create(const CSSOMString &value) noexcept
  {
    if (value.empty())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    return AdoptRef(*new CSSKeywordValue(value));
  }

  dom::ExceptionOr<void> CSSKeywordValue::SetValue(const CSSOMString &value) noexcept
  {
    if (value.empty())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    _value = value;
    return {};
  }

  void CSSKeywordValue::Serialise(StringBuilder &builder, EnumSet<SerialisationArguments>) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#keywordvalue-serialization
    SerialiseIdentifier(_value, builder);
  }

  RefPtr<css::Value> CSSKeywordValue::ToCSSValue() const noexcept
  {
    auto keyword = ValueIdFromName(_value);
    if (keyword == ValueId::Invalid)
    {
      return PrimitiveValue::CreateCustomIdent(_value);
    }
    return PrimitiveValue::Create(keyword);
  }
}
