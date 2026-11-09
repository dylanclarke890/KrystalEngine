/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/TypedOM/Transform/CSSTransformComponent.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSStyleValueFactory.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"

namespace krys::boo::css
{
  CSSTransformComponent::~CSSTransformComponent() noexcept = default;

  CSSOMString CSSTransformComponent::ToString() const noexcept
  {
    StringBuilder builder;
    Serialise(builder);
    return krys::move(builder).ToString();
  }

  dom::ExceptionOr<List<Ref<CSSNumericValue>>>
    CSSTransformComponent::ReifyArguments(const FunctionValue &function, dom::Document &document) noexcept
  {
    List<Ref<CSSNumericValue>> arguments;
    for (const auto &argument : function)
    {
      auto value = CSSStyleValueFactory::ReifyValue(document, argument, {});
      if (value.HasException())
      {
        return value.ReleaseException();
      }
      auto styleValue = value.ReleaseValue();
      auto *numericValue = DynamicDowncast<CSSNumericValue>(*styleValue);
      if (!numericValue)
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      arguments.push_back(ShareRef(*numericValue));
    }
    return arguments;
  }
}
