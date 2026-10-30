#include "Krystal.Booey/CSS/Rules/ViewTransitionRule.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD Maybe<ViewTransitionNavigation>
      ToViewTransitionNavigationEnum(RefPtr<Value> navigation) noexcept
    {
      if (!navigation || !navigation->IsPrimitiveValue())
      {
        return {};
      }

      auto &primitiveNavigationValue = Downcast<PrimitiveValue>(*navigation);
      krys_debug_assert(primitiveNavigationValue.IsValueId());

      if (primitiveNavigationValue.ValueId() == ValueId::Auto)
      {
        return ViewTransitionNavigation::Auto;
      }
      return ViewTransitionNavigation::None;
    }
  }

  ViewTransitionRule::ViewTransitionRule(Ref<StyleProperties> &&properties) noexcept
      : Rule(RuleType::ViewTransition)
  {
    _navigation = ToViewTransitionNavigationEnum(properties->GetPropertyCSSValue(PropertyId::Navigation));

    if (auto value = properties->GetPropertyCSSValue(PropertyId::Types))
    {
      auto processSingleValue = [&](const Value &currentValue)
      {
        if (currentValue.IsCustomIdent())
        {
          _types.emplace_back(currentValue.CustomIdent());
        }
      };
      if (auto *list = DynamicDowncast<ValueList>(*value))
      {
        for (const auto &currentValue : *list)
        {
          processSingleValue(currentValue);
        }
      }
      else
      {
        processSingleValue(*value);
      }
    }
  }

  ViewTransitionRule::~ViewTransitionRule() noexcept = default;

  Ref<ViewTransitionRule> ViewTransitionRule::Create(Ref<StyleProperties> &&properties) noexcept
  {
    return AdoptRef(*new ViewTransitionRule(krys::move(properties)));
  }

  Ref<ViewTransitionRule> ViewTransitionRule::Copy() const noexcept
  {
    return AdoptRef(*new ViewTransitionRule(*this));
  }
}
