/*
 * Copyright (C) 2022-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Filters/FilterEffectApplier.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class FilterEffect;

  /// @brief The results of a filter's effects as it's applied, so an effect read by several others is applied
  /// once; and the accelerated backend that applies them.
  class FilterResults
  {
  public:
    /// @brief Makes the applier of an effect for the accelerated backend; null when it can't apply it.
    using AcceleratedApplierCreator = Func<UniquePtr<FilterEffectApplier>(const FilterEffect &)>;

  private:
    // The value is a list of FilterEffects, whose FilterImages depend on the key FilterImage.
    using FilterEffectSet = HashSet<Ref<FilterEffect>>;

    HashMap<Ref<FilterEffect>, Ref<FilterImage>> _results;
    HashMap<Ref<FilterImage>, FilterEffectSet> _resultReferences;
    AcceleratedApplierCreator _acceleratedApplierCreator;

  public:
    explicit FilterResults(AcceleratedApplierCreator &&acceleratedApplierCreator = nullptr) noexcept;
    ~FilterResults() noexcept;

    KRYS_NODISCARD UniquePtr<FilterEffectApplier>
      CreateAcceleratedApplier(const FilterEffect &effect) const noexcept;

    KRYS_NODISCARD FilterImage *EffectResult(FilterEffect &effect) const noexcept;
    void SetEffectResult(FilterEffect &effect, Span<const Ref<FilterImage>> inputs,
                         Ref<FilterImage> &&result) noexcept;
    void ClearEffectResult(FilterEffect &effect) noexcept;

  private:
    KRYS_NODISCARD size_t MemoryCost() const noexcept;
    KRYS_NODISCARD bool CanCacheResult(const FilterImage &result) const noexcept;
  };
}
