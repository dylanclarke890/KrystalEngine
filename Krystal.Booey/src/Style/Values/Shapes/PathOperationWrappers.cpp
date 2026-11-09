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

#include "Krystal.Booey/Style/Values/Shapes/PathOperationWrappers.hpp"
#include "Krystal.Booey/CSS/Values/BasicShapeValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/RayValue.hpp"
#include "Krystal.Booey/CSS/Values/UrlValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  RefPtr<PathOperation> CSSValueConversion<RefPtr<PathOperation>>::operator()(
    BuilderState &state, const css::Value &value, SupportRayPathOperation supportRayPathOperation) noexcept
  {
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      krys_debug_assert(primitiveValue->ValueId() == css::ValueId::None);
      (void)primitiveValue;
      return nullptr;
    }

    if (const auto *url = DynamicDowncast<css::UrlValue>(value))
    {
      auto styleURL = ToStyleWithScriptExecutionContext(url->URL(), state.GetDocument());

      // FIXME: ReferencePathOperation are not hooked up to support remote URLs yet, so only works with
      // document local references. To see an example of how this should work, see ReferenceFilterOperation
      // which supports both document local and remote URLs.

      auto fragment = svg::SVGURIReference::FragmentIdentifierFromIRIString(styleURL, state.GetDocument());

      const auto &treeScope = [&]() -> const dom::DocumentOrShadowRoot &
      {
        if (const auto *element = state.GetElement())
        {
          return element->TreeScopeForSVGReferences();
        }
        return state.GetDocument();
      }();
      auto target = svg::SVGURIReference::TargetElementFromIRIString(styleURL, treeScope);

      return ReferencePathOperation::Create(styleURL, fragment,
                                            DynamicDowncast<svg::SVGElement>(target.Element.get()));
    }

    if (const auto *ray = DynamicDowncast<css::RayValue>(value))
    {
      if (supportRayPathOperation == SupportRayPathOperation::No)
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return nullptr;
      }
      return RayPathOperation::Create(ConvertToStyle(ray->GetRay(), state));
    }

    RefPtr<PathOperation> operation;
    auto referenceBox = CSSBoxType::BoxMissing;
    auto processSingleValue = [&](const css::Value &singleValue) -> bool
    {
      krys_debug_assert(!Is<css::ValueList>(singleValue));
      if (const auto *ray = DynamicDowncast<css::RayValue>(singleValue))
      {
        if (supportRayPathOperation == SupportRayPathOperation::No)
        {
          state.SetCurrentPropertyInvalidAtComputedValueTime();
          return false;
        }
        operation = RayPathOperation::Create(ConvertToStyle(ray->GetRay(), state));
      }
      else if (const auto *shape = DynamicDowncast<css::BasicShapeValue>(singleValue))
      {
        operation = ShapePathOperation::Create(ConvertToStyle(shape->GetShape(), state, std::nullopt));
      }
      else
      {
        referenceBox = FromCSSValue<CSSBoxType>(singleValue);
      }
      return true;
    };

    if (const auto *list = DynamicDowncast<css::ValueList>(value))
    {
      for (const auto &currentValue : *list)
      {
        if (!processSingleValue(currentValue))
        {
          return nullptr;
        }
      }
    }
    else
    {
      if (!processSingleValue(value))
      {
        return nullptr;
      }
    }

    if (operation)
    {
      operation->SetReferenceBox(referenceBox);
    }
    else
    {
      krys_debug_assert(referenceBox != CSSBoxType::BoxMissing);
      operation = BoxPathOperation::Create(referenceBox);
    }

    return operation;
  }

  Ref<css::Value> CSSValueCreation<RayPath>::operator()(css::ValuePool &, const RenderStyle &style,
                                                        const RayPath &path) noexcept
  {
    return css::RayValue::Create(ConvertToCSS(path.Ray(), style), path.ReferenceBox());
  }

  Ref<css::Value> CSSValueCreation<BasicShapePath>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                               const BasicShapePath &path,
                                                               PathConversion conversion) noexcept
  {
    if (path.ReferenceBox() == CSSBoxType::BoxMissing)
    {
      return css::ValueList::CreateSpaceSeparated(CreateCSSValue(pool, style, path.GetShape(), conversion));
    }
    return css::ValueList::CreateSpaceSeparated(CreateCSSValue(pool, style, path.GetShape(), conversion),
                                                CreateCSSValue(pool, style, path.ReferenceBox()));
  }

  // MARK: - Serialization

  void Serialiser<BasicShapePath>::operator()(StringBuilder &builder,
                                              const css::SerialisationContext &context,
                                              const RenderStyle &style, const BasicShapePath &shape,
                                              PathConversion conversion) noexcept
  {
    if (shape.ReferenceBox() == CSSBoxType::BoxMissing)
    {
      Serialise(builder, context, style, shape.GetShape(), conversion);
      return;
    }

    Serialise(builder, context, style, shape.GetShape(), conversion);
    builder.Append(u8' ');
    Serialise(builder, context, style, shape.ReferenceBox());
  }
}
