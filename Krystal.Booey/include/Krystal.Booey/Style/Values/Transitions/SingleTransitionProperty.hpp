#pragma once

#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/Animation/WebAnimationUtilities.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <single-transition-property> = all | <custom-ident>
  // https://www.w3.org/TR/css-transitions-1/#single-transition-property
  struct SingleTransitionProperty
  {
    struct UnknownProperty
    {
      css::CustomIdentifier Value;

      KRYS_NODISCARD bool operator==(const UnknownProperty &) const noexcept = default;
    };
    struct SingleProperty
    {
      AnimatableCSSProperty Value;

      template <typename... F>
      decltype(auto) SwitchOn(F &&...f) const noexcept
      {
        auto visitor = krys::CreateVisitor(std::forward<F>(f)...);
        return visitor(css::CustomIdentifier {AnimatablePropertyAsString(Value)});
      }

      KRYS_NODISCARD bool operator==(const SingleProperty &) const noexcept = default;
    };

    SingleTransitionProperty(css::keywords::All keyword) noexcept : _value {keyword}
    {
    }

    SingleTransitionProperty(css::keywords::None keyword) noexcept : _value {keyword}
    {
    }

    SingleTransitionProperty(css::CustomIdentifier &&identifier) noexcept
        : _value {FromCustomIdentifier(krys::move(identifier))}
    {
    }

    SingleTransitionProperty(css::PropertyId propertyID) noexcept
        : _value {SingleProperty {.Value = propertyID}}
    {
    }

    KRYS_NODISCARD bool IsAll() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::All>(_value);
    }
    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::None>(_value);
    }
    KRYS_NODISCARD bool IsUnknownProperty() const noexcept
    {
      return krys::HoldsAlternative<UnknownProperty>(_value);
    }
    KRYS_NODISCARD bool IsSingleProperty() const noexcept
    {
      return krys::HoldsAlternative<SingleProperty>(_value);
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const SingleTransitionProperty &) const noexcept = default;

  private:
    using Kind = Variant<css::keywords::All, css::keywords::None, UnknownProperty, SingleProperty>;

    KRYS_NODISCARD static Kind FromCustomIdentifier(css::CustomIdentifier &&identifier) noexcept
    {
      if (css::IsCustomPropertyName(identifier.Value.View()))
      {
        return Kind {SingleProperty {.Value = krys::move(identifier.Value)}};
      }
      return Kind {UnknownProperty {.Value = krys::move(identifier)}};
    }

    Kind _value;
  };

  DEFINE_TYPE_WRAPPER_GET(SingleTransitionProperty::UnknownProperty, Value);

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<SingleTransitionProperty>
  {
    KRYS_NODISCARD SingleTransitionProperty operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::SingleTransitionProperty::UnknownProperty)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SingleTransitionProperty::SingleProperty)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SingleTransitionProperty)
