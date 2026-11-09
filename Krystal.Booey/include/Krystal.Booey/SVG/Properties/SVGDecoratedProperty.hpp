/*
 * Copyright (C) 2019-2024 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::svg
{
  /// @brief A value that the DOM sees as a `TDecoration` (an enumeration's `unsigned short`) and that is kept
  /// as another type.
  template <typename TDecoration>
  class SVGDecoratedProperty : public RefCounted<SVGDecoratedProperty<TDecoration>>
  {
  public:
    SVGDecoratedProperty() noexcept = default;
    virtual ~SVGDecoratedProperty() noexcept = default;

    virtual void SetValueInternal(const TDecoration &) noexcept = 0;
    virtual bool SetValue(const TDecoration &value) noexcept
    {
      SetValueInternal(value);
      return true;
    }

    // Used internally. It doesn't check for HighestExposedEnumValue for example.
    KRYS_NODISCARD virtual TDecoration ValueInternal() const noexcept = 0;

    // Used by the DOM APIs.
    KRYS_NODISCARD virtual TDecoration Value() const noexcept
    {
      return ValueInternal();
    }

    KRYS_NODISCARD virtual dom::DOMString ValueAsString() const noexcept = 0;
    KRYS_NODISCARD virtual Ref<SVGDecoratedProperty<TDecoration>> Clone() noexcept = 0;
  };
}
