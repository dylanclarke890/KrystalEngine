/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
