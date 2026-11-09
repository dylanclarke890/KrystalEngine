/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 * Copyright (C) 2013 Igalia S.L. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class RenderBox;
  class RenderObject;

  /// @brief Iterates over a flex or grid container's child boxes in `order`-modified document order (or its
  /// reverse).
  class OrderIterator
  {
    friend class OrderIteratorPopulator;

  private:
    RenderBox *_containerBox;
    RenderBox *_currentChild {nullptr};

    // The distinct order values of the children, ascending.
    List<int32> _orderValues;
    // The position in _orderValues, counted from the end when the order is reversed.
    size_t _orderValuesIndex {0};
    bool _isReset {false};
    bool _reversedOrder {false};

  public:
    explicit OrderIterator(RenderBox &containerBox) noexcept;

    KRYS_NODISCARD RenderBox *CurrentChild() const noexcept
    {
      return _currentChild;
    }
    RenderBox *First() noexcept;
    RenderBox *Next() noexcept;
    KRYS_NODISCARD OrderIterator Reverse() const noexcept;
    KRYS_NODISCARD bool ShouldSkipChild(const RenderObject &child) const noexcept;

  private:
    void Reset() noexcept;
    KRYS_NODISCARD int32 CurrentOrderValue() const noexcept;
  };

  /// @brief Collects the order values of a container's children into an @ref OrderIterator, and resets the
  /// iterator when it goes out of scope.
  class OrderIteratorPopulator
  {
  private:
    OrderIterator &_iterator;

  public:
    explicit OrderIteratorPopulator(OrderIterator &iterator) noexcept;
    ~OrderIteratorPopulator() noexcept;

    /// @return Whether @p child takes part in the container's layout (it isn't skipped).
    bool CollectChild(const RenderBox &child) noexcept;
  };
}
