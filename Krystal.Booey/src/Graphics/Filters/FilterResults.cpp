/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Filters/FilterResults.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"

namespace krys::boo
{
  FilterResults::FilterResults(AcceleratedApplierCreator &&acceleratedApplierCreator) noexcept
      : _acceleratedApplierCreator(krys::move(acceleratedApplierCreator))
  {
  }

  FilterResults::~FilterResults() noexcept = default;

  UniquePtr<FilterEffectApplier>
    FilterResults::CreateAcceleratedApplier(const FilterEffect &effect) const noexcept
  {
    if (!_acceleratedApplierCreator)
    {
      return nullptr;
    }
    return _acceleratedApplierCreator(effect);
  }

  FilterImage *FilterResults::EffectResult(FilterEffect &effect) const noexcept
  {
    auto it = _results.find(ShareRef(effect));
    return it == _results.end() ? nullptr : it->second.get();
  }

  size_t FilterResults::MemoryCost() const noexcept
  {
    size_t memoryCost = 0u;
    for (const auto &[effect, result] : _results)
    {
      memoryCost += result->MemoryCost();
    }
    return memoryCost;
  }

  bool FilterResults::CanCacheResult(const FilterImage &result) const noexcept
  {
    static constexpr size_t maxAllowedMemoryCost = 100u * 1'024u * 1'024u;
    return MemoryCost() + result.MemoryCost() <= maxAllowedMemoryCost;
  }

  void FilterResults::SetEffectResult(FilterEffect &effect, Span<const Ref<FilterImage>> inputs,
                                      Ref<FilterImage> &&result) noexcept
  {
    if (!CanCacheResult(*result))
    {
      return;
    }

    _results.insert_or_assign(ShareRef(effect), krys::move(result));

    for (const auto &input : inputs)
    {
      _resultReferences[input].insert(ShareRef(effect));
    }
  }

  void FilterResults::ClearEffectResult(FilterEffect &effect) noexcept
  {
    auto it = _results.find(ShareRef(effect));
    if (it == _results.end())
    {
      return;
    }

    auto result = it->second;
    _results.erase(it);

    auto references = _resultReferences.find(result);
    if (references == _resultReferences.end())
    {
      return;
    }
    auto dependents = krys::move(references->second);
    _resultReferences.erase(references);
    for (const auto &reference : dependents)
    {
      ClearEffectResult(*reference);
    }
  }
}
