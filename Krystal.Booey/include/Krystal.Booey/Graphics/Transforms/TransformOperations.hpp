/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Graham Dennis (graham.dennis@gmail.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <algorithm>

namespace krys::boo
{
  struct BlendingContext;

  /// @brief A list of transform operations, applied in order.
  class TransformOperations
  {
  public:
    using const_iterator = List<Ref<TransformOperation>>::const_iterator;
    using const_reverse_iterator = List<Ref<TransformOperation>>::const_reverse_iterator;
    using value_type = List<Ref<TransformOperation>>::value_type;

  private:
    List<Ref<TransformOperation>> _operations;

  public:
    TransformOperations() noexcept = default;
    explicit TransformOperations(Ref<TransformOperation> &&) noexcept;
    explicit TransformOperations(List<Ref<TransformOperation>> &&) noexcept;

    KRYS_NODISCARD bool operator==(const TransformOperations &) const noexcept;

    KRYS_NODISCARD TransformOperations Clone() const noexcept;

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
    KRYS_NODISCARD const TransformOperation *At(size_t index) const noexcept KRYS_LIFETIME_BOUND
    {
      return index < _operations.size() ? _operations[index].get() : nullptr;
    }

    KRYS_NODISCARD const Ref<TransformOperation> &operator[](size_t i) const noexcept KRYS_LIFETIME_BOUND
    {
      return _operations[i];
    }
    KRYS_NODISCARD const Ref<TransformOperation> &First() const noexcept KRYS_LIFETIME_BOUND
    {
      return _operations.front();
    }
    KRYS_NODISCARD const Ref<TransformOperation> &Last() const noexcept KRYS_LIFETIME_BOUND
    {
      return _operations.back();
    }

    void Apply(TransformationMatrix &, uint32 start = 0) const noexcept;

    template <TransformOperation::Type operationType>
    KRYS_NODISCARD bool HasTransformOfType() const noexcept
    {
      return std::ranges::any_of(_operations, [](const auto &op) { return op->GetType() == operationType; });
    }

    KRYS_NODISCARD bool IsInvertible() const noexcept;
    KRYS_NODISCARD bool ContainsNonInvertibleMatrix() const noexcept;
  };

  KRYS_NODISCARD TransformOperations Blend(const TransformOperations &from, const TransformOperations &to,
                                           const BlendingContext &) noexcept;
}
