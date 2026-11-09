#include "Krystal.Booey/Style/Values/Fonts/FontFamily.hpp"
#include "Krystal.Booey/CSS/Font/GenericFontFamilyNames.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
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
      if (auto family = css::PropertyParserHelpers::GenericFontFamily(valueID);
          family != css::CSSOMStringAtom::Null())
      {
        return {family, FontFamilyKind::Generic};
      }

      if (css::PropertyParserHelpers::IsSystemFontShorthand(valueID))
      {
        // The host's family for the keyword, or else the standard family.
        const auto &font = state.GetDocument()
                             .GetSettings()
                             .Values.SystemFonts[css::PropertyParserHelpers::LowerFontShorthand(valueID)];
        if (font.Family == StringAtom::Null() || font.Family.View().empty())
        {
          return {css::GenericFontFamilyNames::StandardFamily(), FontFamilyKind::Generic};
        }
        return {css::CSSOMStringAtom {font.Family}, FontFamilyKind::Specified};
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
