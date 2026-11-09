/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/AnchorPosition/PositionTryFallback.hpp"
#include "Krystal.Booey/CSS/Parser/ParserMode.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  bool PositionTryFallback::PositionArea::operator==(
    const PositionTryFallback::PositionArea &other) const noexcept
  {
    if (Properties && other.Properties)
    {
      if (Properties == other.Properties)
      {
        return true;
      }

      auto lhsPositionArea = Properties->GetPropertyCSSValue(css::PropertyId::PositionArea);
      krys_debug_assert(lhsPositionArea);
      auto rhsPositionArea = other.Properties->GetPropertyCSSValue(css::PropertyId::PositionArea);
      krys_debug_assert(rhsPositionArea);

      return *lhsPositionArea == *rhsPositionArea;
    }

    return !Properties && !other.Properties;
  }

  bool PositionTryFallback::operator==(const PositionTryFallback &other) const noexcept
  {
    if (PositionAreaFallback.Properties && other.PositionAreaFallback.Properties)
    {
      return PositionAreaFallback == other.PositionAreaFallback;
    }

    if (!PositionAreaFallback.Properties && !other.PositionAreaFallback.Properties)
    {
      return RuleAndTacticsFallback == other.RuleAndTacticsFallback;
    }

    // If we got here, this and other don't have the same type (e.g comparing position-area with rule +
    // tactics)
    return false;
  }

  // MARK: - Conversion

  PositionTryFallback CSSValueConversion<PositionTryFallback>::operator()(BuilderState &state,
                                                                          const css::Value &value) noexcept
  {
    if (const auto *valueList = DynamicDowncast<css::ValueList>(value))
    {
      if (valueList->Separator() != css::ValueSeparator::SpaceSeparator)
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return {};
      }

      auto rule = Maybe<ScopedName> {};
      auto tactics = css::SpaceSeparatedList<PositionTryFallbackTactic> {};

      for (const auto &item : *valueList)
      {
        switch (item.ValueId())
        {
          case css::ValueId::FlipBlock: tactics.Value.push_back(PositionTryFallbackTactic::FlipBlock); break;
          case css::ValueId::FlipInline:
            tactics.Value.push_back(PositionTryFallbackTactic::FlipInline);
            break;
          case css::ValueId::FlipStart: tactics.Value.push_back(PositionTryFallbackTactic::FlipStart); break;
          case css::ValueId::FlipX:     tactics.Value.push_back(PositionTryFallbackTactic::FlipX); break;
          case css::ValueId::FlipY:     tactics.Value.push_back(PositionTryFallbackTactic::FlipY); break;
          case css::ValueId::Invalid:
          {
            const auto *primitive = DynamicDowncast<css::PrimitiveValue>(item);
            if (primitive && primitive->IsCustomIdent() && !rule)
            {
              rule = ScopedName {css::CSSOMStringAtom {primitive->StringValue()}, state.StyleScopeOrdinal()};
              break;
            }
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return {};
          }
          default: state.SetCurrentPropertyInvalidAtComputedValueTime(); return {};
        }
      }

      PositionTryFallback result;
      result.RuleAndTacticsFallback.Rule = krys::move(rule);
      if (!tactics.isEmpty())
      {
        result.RuleAndTacticsFallback.Tactics = krys::move(tactics);
      }
      return result;
    }

    // Turn the inlined position-area fallback into properties object that can be applied similarly to
    // @position-try declarations.
    css::Property property {css::PropertyId::PositionArea, ShareRef(const_cast<css::Value &>(value))};
    PositionTryFallback result;
    result.PositionAreaFallback.Properties = css::ImmutableStyleProperties::CreateDeduplicating(
      Span<const css::Property> {&property, 1}, css::ParserMode::HTMLStandard);
    return result;
  }

  Ref<css::Value> CSSValueCreation<PositionTryFallback::PositionArea>::operator()(
    css::ValuePool &, const RenderStyle &, const PositionTryFallback::PositionArea &value) noexcept
  {
    return Ref<css::Value> {value.Properties->GetPropertyCSSValue(css::PropertyId::PositionArea)};
  }

  // MARK: - Serialization

  void Serialiser<PositionTryFallback::PositionArea>::operator()(
    StringBuilder &builder, const css::SerialisationContext &context, const RenderStyle &,
    const PositionTryFallback::PositionArea &value) noexcept
  {
    builder.Append(value.Properties->GetPropertyCSSValue(css::PropertyId::PositionArea)->CssText(context));
  }
}
