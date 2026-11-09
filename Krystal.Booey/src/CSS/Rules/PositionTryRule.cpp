/*
 * Copyright (C) 2024-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
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

#include "Krystal.Booey/CSS/Rules/PositionTryRule.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"

namespace krys::boo::css
{
  PositionTryRule::PositionTryRule(CSSOMStringAtom &&name, Ref<StyleProperties> &&properties) noexcept
      : Rule(RuleType::PositionTry), _name(krys::move(name)), _properties(krys::move(properties))
  {
  }

  PositionTryRule::PositionTryRule(const PositionTryRule &other) noexcept
      : Rule(other), _name(other._name), _properties(other.ProtectedProperties()->MutableCopy())
  {
  }

  PositionTryRule::~PositionTryRule() noexcept = default;

  Ref<PositionTryRule> PositionTryRule::Create(CSSOMStringAtom &&name,
                                               Ref<StyleProperties> &&properties) noexcept
  {
    return AdoptRef(*new PositionTryRule(krys::move(name), krys::move(properties)));
  }

  Ref<PositionTryRule> PositionTryRule::Copy() const noexcept
  {
    return AdoptRef(*new PositionTryRule(*this));
  }

  Ref<StyleProperties> PositionTryRule::ProtectedProperties() const noexcept
  {
    return _properties;
  }

  MutableStyleProperties &PositionTryRule::MutableProperties() noexcept
  {
    if (!Is<MutableStyleProperties>(*_properties))
    {
      _properties = _properties->MutableCopy();
    }
    return Downcast<MutableStyleProperties>(*_properties);
  }

  Ref<MutableStyleProperties> PositionTryRule::ProtectedMutableProperties() noexcept
  {
    return ShareRef(MutableProperties());
  }
}
