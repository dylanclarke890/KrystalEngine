/*
 * Copyright (C) 2018-2019 Apple Inc. All rights reserved.
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
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::svg
{
  enum class AnimationMode : uint8;
  enum class CalcMode : uint8;
}

namespace krys::boo
{
  class SVGAnimatedProperty;
  class SVGAttributeAnimator;
  class SVGProperty;

  /// @brief An element's registry of the properties that reflect its attributes.
  class SVGPropertyRegistry
  {
  public:
    SVGPropertyRegistry() noexcept = default;

    virtual ~SVGPropertyRegistry() noexcept = default;

    virtual void DetachAllProperties() const noexcept = 0;

    KRYS_NODISCARD virtual QualifiedName PropertyAttributeName(const SVGProperty &) const noexcept = 0;

    KRYS_NODISCARD virtual QualifiedName
      AnimatedPropertyAttributeName(const SVGAnimatedProperty &) const noexcept = 0;

    virtual void SetAnimatedPropertyDirty(const QualifiedName &, SVGAnimatedProperty &) const noexcept = 0;

    KRYS_NODISCARD virtual Maybe<DOMString> Synchronize(const QualifiedName &) const noexcept = 0;

    KRYS_NODISCARD virtual List<Pair<QualifiedName, DOMString>> SynchronizeAllAttributes() const noexcept = 0;

    KRYS_NODISCARD virtual bool IsAnimatedPropertyAttribute(const QualifiedName &) const noexcept = 0;

    KRYS_NODISCARD virtual bool IsAnimatedStylePropertyAttribute(const QualifiedName &) const noexcept = 0;

    KRYS_NODISCARD virtual RefPtr<SVGAttributeAnimator> CreateAnimator(const QualifiedName &,
                                                                       svg::AnimationMode, svg::CalcMode,
                                                                       bool isAccumulated,
                                                                       bool isAdditive) const noexcept = 0;

    virtual void AppendAnimatedInstance(const QualifiedName &attributeName,
                                        SVGAttributeAnimator &) const noexcept = 0;
  };
}
