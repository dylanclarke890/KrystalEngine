#include "Krystal.Booey/Style/Values/Fonts/FontFamily.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Graphics/SystemFontDatabase.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::style
{
  namespace
  {
    // FIXME: `document().settings().standardFontFamily()` once there's a `Settings` port. The default
    // settings have no standard font family, so it's the empty string.
    KRYS_NODISCARD css::CSSOMStringAtom StandardFontFamily(const BuilderState &) noexcept
    {
      return css::CSSOMStringAtom::Empty();
    }
  }

  // MARK: - Conversion

  FontFamilies CSSValueConversion<FontFamilies>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      if (primitiveValue->IsFontFamily())
      {
        return {css::CSSOMStringAtom {primitiveValue->StringValue()}, FontFamilyKind::Specified};
      }

      auto valueID = primitiveValue->ValueId();
      if (valueID == css::ValueId::WebkitBody)
      {
        return {StandardFontFamily(state), FontFamilyKind::Specified};
      }

      if (auto family = css::PropertyParserHelpers::GenericFontFamily(valueID);
          family != css::CSSOMStringAtom::Null())
      {
        return {family, FontFamilyKind::Generic};
      }

      if (css::PropertyParserHelpers::IsSystemFontShorthand(valueID))
      {
        auto family = SystemFontDatabase::Singleton().SystemFontShorthandFamily(
          css::PropertyParserHelpers::LowerFontShorthand(valueID));
        krys_debug_assert(!family.View().empty());

        return {family, FontFamilyKind::Generic};
      }

      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return {css::CSSOMStringAtom::Null(), FontFamilyKind::Generic};
    }

    auto valueList = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!valueList)
    {
      return {css::CSSOMStringAtom::Null(), FontFamilyKind::Generic};
    }

    Maybe<FontFamilyKind> firstFontKind;
    List<css::CSSOMStringAtom> families;
    for (const auto &contentValue : *valueList)
    {
      auto [family, kind] = [&]() -> Pair<css::CSSOMStringAtom, FontFamilyKind>
      {
        if (contentValue.IsFontFamily())
        {
          return {css::CSSOMStringAtom {contentValue.StringValue()}, FontFamilyKind::Specified};
        }

        auto valueID = contentValue.ValueId();
        if (valueID == css::ValueId::WebkitBody)
        {
          return {StandardFontFamily(state), FontFamilyKind::Specified};
        }

        return {css::PropertyParserHelpers::GenericFontFamily(valueID), FontFamilyKind::Generic};
      }();

      if (family == css::CSSOMStringAtom::Null())
      {
        continue;
      }

      if (!firstFontKind)
      {
        firstFontKind = kind;
      }

      families.push_back(family);
    }

    if (families.empty())
    {
      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return {css::CSSOMStringAtom::Null(), FontFamilyKind::Generic};
    }

    return {krys::move(families), *firstFontKind};
  }
}
