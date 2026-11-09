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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/Transforms/Transform.hpp"

#include "Krystal.Booey/CSS/Values/TransformListValue.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutSize.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformOperations.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  Transform CSSValueConversion<Transform>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (value.ValueId() == css::ValueId::None)
    {
      return css::keywords::None {};
    }

    auto *transformList = RequiredDowncast<css::TransformListValue>(state, value);
    if (!transformList)
    {
      return css::keywords::None {};
    }

    return TransformList {
      TransformList::Container::map(*transformList, [&](auto &transform)
                                    { return ConvertCSSValueToStyle<TransformFunction>(state, transform); })};
  }

  Ref<css::Value> CSSValueCreation<Transform>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                          const Transform &value) noexcept
  {
    css::ValueListBuilder list;
    for (auto &transformFunction : value)
    {
      list.push_back(CreateCSSValue(pool, style, transformFunction));
    }

    if (list.empty())
    {
      return CreateCSSValue(pool, style, css::keywords::None {});
    }

    return css::TransformListValue::Create(krys::move(list));
  }

  // MARK: - Blending

  bool Blending<Transform>::CanBlend(const Transform &from, const Transform &to,
                                     CompositeOperation compositeOperation) noexcept
  {
    return style::CanBlend(from._value, to._value, compositeOperation);
  }

  Transform Blending<Transform>::Blend(const Transform &from, const Transform &to,
                                       const BlendingContext &context) noexcept
  {
    auto blendedTransformList = style::Blend(from._value, to._value, context);

    if (blendedTransformList.isEmpty())
    {
      return css::keywords::None {};
    }

    return Transform {krys::move(blendedTransformList)};
  }

  // MARK: - Platform

  TransformOperations ToPlatform<Transform>::operator()(const Transform &value,
                                                        const FloatSize &size) noexcept
  {
    List<Ref<TransformOperation>> operations;
    for (const auto &transformFunction : value)
    {
      operations.push_back(style::ConvertToPlatform(transformFunction, size));
    }
    return TransformOperations {krys::move(operations)};
  }
}
