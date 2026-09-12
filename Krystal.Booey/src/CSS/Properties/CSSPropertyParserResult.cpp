#include "Krystal.Booey/CSS/Properties/CSSPropertyParserResult.hpp"
#include "Krystal.Booey/CSS/Properties/CSSPropertyShorthand.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"

namespace krys::boo::css
{
  void CSSPropertyParserResult::AddProperty(Property &&property) noexcept
  {
    ParsedProperties.emplace_back(krys::move(property));
  }

  void CSSPropertyParserResult::AddProperty(CSSPropertyParserState &state, CSSPropertyId property,
                                            CSSPropertyId currentShorthand, RefPtr<CSSValue> &&value,
                                            IsImportant important, IsImplicit implicit) noexcept
  {
    size_t shorthandIndex = 0uz;
    IsSetFromShorthand setFromShorthand = IsSetFromShorthand(false);

    if (currentShorthand != CSSPropertyId::Invalid)
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
    assert(IsExposed(property, &state.Context.PropertySettings) || setFromShorthand || IsInternal(property));

    if (value && !value->IsImplicitInitialValue())
      AddProperty(Property(property, krys::move(value), important, implicit, setFromShorthand,
                           static_cast<uint8>(shorthandIndex)));
    else
    {
      assert(setFromShorthand);
      AddProperty(Property(property, ShareRef(CSSPrimitiveValue::ImplicitInitialValue()), important,
                           IsImplicit(true), setFromShorthand, static_cast<uint8>(shorthandIndex)));
    }
  }

#pragma region Utils

  void CSSPropertyParserResult::AddPropertyForCurrentShorthand(CSSPropertyParserState &state,
                                                               CSSPropertyId longhand,
                                                               RefPtr<CSSValue> &&value,
                                                               IsImplicit implicit) noexcept
  {
    AddProperty(state, longhand, state.CurrentProperty, krys::move(value), state.Important, implicit);
  }

  void CSSPropertyParserResult::AddPropertyForAllLonghandsOfShorthand(CSSPropertyParserState &state,
                                                                      CSSPropertyId shorthand,
                                                                      RefPtr<CSSValue> &&value,
                                                                      IsImportant important,
                                                                      IsImplicit implicit) noexcept
  {
    for (auto longhand : ShorthandForProperty(shorthand))
    {
      AddProperty(state, longhand, shorthand, ShareRef(*value), important, implicit);
    }
  }

  void CSSPropertyParserResult::AddPropertyForAllLonghandsOfCurrentShorthand(CSSPropertyParserState &state,
                                                                             RefPtr<CSSValue> &&value,
                                                                             IsImplicit implicit) noexcept
  {
    AddPropertyForAllLonghandsOfShorthand(state, state.CurrentProperty, krys::move(value), state.Important,
                                          implicit);
  }

#pragma endregion
}