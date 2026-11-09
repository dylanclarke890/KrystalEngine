// Copyright 2015 The Chromium Authors. All rights reserved.
// Copyright (C) 2016-2025 Apple Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  class Builder;
}

namespace krys::boo::css
{
  /// @brief The value set on each longhand of a shorthand whose value contains `var()` or `env()` references.
  /// The shorthand is parsed again, and the longhand's value picked out, once the references are resolved.
  class PendingSubstitutionValue final : public Value
  {
  private:
    const PropertyId _shorthandPropertyId;
    const Ref<VariableReferenceValue> _shorthandValue;

    mutable List<Property> _cachedPropertyValues;

    PendingSubstitutionValue(PropertyId shorthandPropertyId,
                             Ref<VariableReferenceValue> &&shorthandValue) noexcept
        : Value(ValueType::PendingSubstitution), _shorthandPropertyId(shorthandPropertyId),
          _shorthandValue(krys::move(shorthandValue))
    {
    }

  public:
    KRYS_NODISCARD static Ref<PendingSubstitutionValue>
      Create(PropertyId shorthandPropertyId, Ref<VariableReferenceValue> &&shorthandValue) noexcept
    {
      return AdoptRef(*new PendingSubstitutionValue(shorthandPropertyId, krys::move(shorthandValue)));
    }

    KRYS_NODISCARD VariableReferenceValue &ShorthandValue() const noexcept
    {
      return *_shorthandValue;
    }

    KRYS_NODISCARD PropertyId ShorthandPropertyId() const noexcept
    {
      return _shorthandPropertyId;
    }

    KRYS_NODISCARD bool Equals(const PendingSubstitutionValue &other) const noexcept
    {
      return _shorthandValue.get() == other._shorthandValue.get();
    }

    KRYS_NODISCARD static CSSOMString CustomCSSText(const SerialisationContext &) noexcept
    {
      return {};
    }

    KRYS_NODISCARD RefPtr<Value> ResolveValue(style::Builder &builder, PropertyId propertyId) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      if (func(*_shorthandValue) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      return IterationStatus::Continue;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::PendingSubstitutionValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsPendingSubstitutionValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
