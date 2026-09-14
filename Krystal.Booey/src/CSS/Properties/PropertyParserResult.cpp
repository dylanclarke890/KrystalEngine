#include "Krystal.Booey/CSS/Properties/PropertyParserResult.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyShorthand.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css
{
  void PropertyParserResult::AddProperty(Property &&property) noexcept
  {
    ParsedProperties.emplace_back(krys::move(property));
  }

  void PropertyParserResult::AddProperty(PropertyParserState &state, PropertyId property,
                                         PropertyId currentShorthand, RefPtr<Value> &&value,
                                         IsImportant important, IsImplicit implicit) noexcept
  {
    size_t shorthandIndex = 0uz;
    IsSetFromShorthand setFromShorthand = IsSetFromShorthand(false);

    if (currentShorthand != PropertyId::Invalid)
    {
      auto shorthands = MatchingShorthandsForLonghand(property);
      setFromShorthand = IsSetFromShorthand(true);
      if (shorthands.size() > 1uz)
      {
        shorthandIndex = IndexOfShorthandProperty(currentShorthand, shorthands);
      }
    }

    // Allow anything to be set from a shorthand (e.g. the CSS all property always sets everything,
    // regardless of whether the longhands are enabled), and allow internal properties as we use
    // them to handle certain DOM-exposed values (e.g. -webkit-font-size-delta from
    // execCommand('FontSizeDelta')).
    krys_debug_assert(IsExposed(property, state.Context.PropertySettings) || setFromShorthand
                      || IsInternal(property));

    if (value && !value->IsImplicitInitialValue())
      AddProperty(Property(property, krys::move(value), important, implicit, setFromShorthand,
                           static_cast<uint8>(shorthandIndex)));
    else
    {
      krys_debug_assert(setFromShorthand);
      AddProperty(Property(property, ShareRef(PrimitiveValue::ImplicitInitialValue()), important,
                           IsImplicit(true), setFromShorthand, static_cast<uint8>(shorthandIndex)));
    }
  }

#pragma region Utils

  void PropertyParserResult::AddPropertyForCurrentShorthand(PropertyParserState &state, PropertyId longhand,
                                                            RefPtr<Value> &&value,
                                                            IsImplicit implicit) noexcept
  {
    AddProperty(state, longhand, state.CurrentProperty, krys::move(value), state.Important, implicit);
  }

  void PropertyParserResult::AddPropertyForAllLonghandsOfShorthand(PropertyParserState &state,
                                                                   PropertyId shorthand,
                                                                   RefPtr<Value> &&value,
                                                                   IsImportant important,
                                                                   IsImplicit implicit) noexcept
  {
    for (auto longhand : ShorthandForProperty(shorthand))
    {
      AddProperty(state, longhand, shorthand, ShareRef(*value), important, implicit);
    }
  }

  void PropertyParserResult::AddPropertyForAllLonghandsOfCurrentShorthand(PropertyParserState &state,
                                                                          RefPtr<Value> &&value,
                                                                          IsImplicit implicit) noexcept
  {
    AddPropertyForAllLonghandsOfShorthand(state, state.CurrentProperty, krys::move(value), state.Important,
                                          implicit);
  }

#pragma endregion
}