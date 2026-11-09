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

#include "Krystal.Booey/Rendering/Iterators/OrderIterator.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>

namespace krys::boo
{
  OrderIterator::OrderIterator(RenderBox &containerBox) noexcept : _containerBox(&containerBox)
  {
    Reset();
  }

  RenderBox *OrderIterator::First() noexcept
  {
    Reset();
    return Next();
  }

  int32 OrderIterator::CurrentOrderValue() const noexcept
  {
    return _reversedOrder ? _orderValues[_orderValues.size() - 1 - _orderValuesIndex]
                          : _orderValues[_orderValuesIndex];
  }

  RenderBox *OrderIterator::Next() noexcept
  {
    do
    {
      if (!_currentChild)
      {
        if (_orderValuesIndex == _orderValues.size())
        {
          return nullptr;
        }

        if (!_isReset)
        {
          ++_orderValuesIndex;
          if (_orderValuesIndex == _orderValues.size())
          {
            return nullptr;
          }
        }
        else
        {
          _isReset = false;
        }
        _currentChild = _reversedOrder ? _containerBox->LastChildBox() : _containerBox->FirstChildBox();
      }
      else
      {
        _currentChild =
          _reversedOrder ? _currentChild->PreviousSiblingBox() : _currentChild->NextSiblingBox();
      }
    } while (!_currentChild || _currentChild->Style().GetOrder().Value != CurrentOrderValue());

    return _currentChild;
  }

  void OrderIterator::Reset() noexcept
  {
    _currentChild = nullptr;
    _orderValuesIndex = 0;
    _isReset = true;
  }

  bool OrderIterator::ShouldSkipChild(const RenderObject &child) const noexcept
  {
    return child.IsOutOfFlowPositioned() || child.IsExcludedFromNormalLayout();
  }

  OrderIterator OrderIterator::Reverse() const noexcept
  {
    auto reversedIterator = *this;
    reversedIterator._reversedOrder = !_reversedOrder;
    reversedIterator.Reset();
    return reversedIterator;
  }

  OrderIteratorPopulator::OrderIteratorPopulator(OrderIterator &iterator) noexcept : _iterator(iterator)
  {
    _iterator._orderValues.clear();
  }

  OrderIteratorPopulator::~OrderIteratorPopulator() noexcept
  {
    auto &orderValues = _iterator._orderValues;
    std::ranges::sort(orderValues);
    orderValues.erase(std::ranges::unique(orderValues).begin(), orderValues.end());
    _iterator.Reset();
  }

  bool OrderIteratorPopulator::CollectChild(const RenderBox &child) noexcept
  {
    _iterator._orderValues.push_back(child.Style().GetOrder().Value);
    return !_iterator.ShouldSkipChild(child);
  }
}
