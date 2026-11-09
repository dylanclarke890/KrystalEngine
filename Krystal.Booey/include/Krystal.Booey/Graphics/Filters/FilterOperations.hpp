/*
 * Copyright (C) 2011-2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Animation/CompositeOperation.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <algorithm>

namespace krys::boo
{
  struct BlendingContext;

  /// @brief A list of filter operations, applied in order.
  class FilterOperations
  {
  public:
    using const_iterator = List<Ref<FilterOperation>>::const_iterator;
    using const_reverse_iterator = List<Ref<FilterOperation>>::const_reverse_iterator;
    using value_type = List<Ref<FilterOperation>>::value_type;

  private:
    List<Ref<FilterOperation>> _operations;

  public:
    FilterOperations() noexcept = default;
    explicit FilterOperations(List<Ref<FilterOperation>> &&) noexcept;

    KRYS_NODISCARD bool operator==(const FilterOperations &) const noexcept;

    KRYS_NODISCARD FilterOperations Clone() const noexcept;

    KRYS_NODISCARD const_iterator begin() const noexcept KRYS_LIFETIME_BOUND
    {
      return _operations.begin();
    }
    KRYS_NODISCARD const_iterator end() const noexcept KRYS_LIFETIME_BOUND
    {
      return _operations.end();
    }
    KRYS_NODISCARD const_reverse_iterator rbegin() const noexcept KRYS_LIFETIME_BOUND
    {
      return _operations.rbegin();
    }
    KRYS_NODISCARD const_reverse_iterator rend() const noexcept KRYS_LIFETIME_BOUND
    {
      return _operations.rend();
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _operations.empty();
    }
    KRYS_NODISCARD size_t size() const noexcept
    {
      return _operations.size();
    }
    KRYS_NODISCARD const FilterOperation *At(size_t index) const noexcept KRYS_LIFETIME_BOUND
    {
      return index < _operations.size() ? _operations[index].get() : nullptr;
    }

    KRYS_NODISCARD const Ref<FilterOperation> &operator[](size_t i) const noexcept KRYS_LIFETIME_BOUND
    {
      return _operations[i];
    }
    KRYS_NODISCARD const Ref<FilterOperation> &First() const noexcept KRYS_LIFETIME_BOUND
    {
      return _operations.front();
    }
    KRYS_NODISCARD const Ref<FilterOperation> &Last() const noexcept KRYS_LIFETIME_BOUND
    {
      return _operations.back();
    }

    KRYS_NODISCARD bool OperationsMatch(const FilterOperations &) const noexcept;

    KRYS_NODISCARD bool HasOutsets() const noexcept
    {
      return !Outsets().IsZero();
    }
    KRYS_NODISCARD IntOutsets Outsets() const noexcept;

    KRYS_NODISCARD bool HasFilterThatAffectsOpacity() const noexcept;
    KRYS_NODISCARD bool HasFilterThatMovesPixels() const noexcept;
    KRYS_NODISCARD bool HasFilterThatShouldBeRestrictedBySecurityOrigin() const noexcept;

    template <FilterOperation::Type Type>
    KRYS_NODISCARD bool HasFilterOfType() const noexcept
    {
      return std::ranges::any_of(_operations, [](const auto &op) { return op->GetType() == Type; });
    }

    KRYS_NODISCARD bool HasReferenceFilter() const noexcept;

    KRYS_NODISCARD bool CanInterpolate(const FilterOperations &, CompositeOperation) const noexcept;
    KRYS_NODISCARD FilterOperations Blend(const FilterOperations &, const BlendingContext &) const noexcept;
  };
}
