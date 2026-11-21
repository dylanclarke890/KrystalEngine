/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2019-2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Containers/RedBlackTree.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys
{
  /// @brief A closed interval `[low, high]` holding a piece of user data. Ordering and equality include the
  /// user data.
  /// @tparam T The endpoint type: copyable, and comparable with `<` and `==`.
  /// @tparam TUserData The data type: copyable, and comparable with `<` and `==`.
  template <typename T, typename TUserData>
  class Interval
  {
  private:
    T _low;
    T _high;
    TUserData _data {};
    T _maxHigh;

  public:
    Interval(const T &low, const T &high, TUserData &&data = {}) noexcept
        : _low(low), _high(high), _data(krys::move(data)), _maxHigh(high)
    {
    }

    Interval(const T &low, const T &high, const TUserData &data) noexcept
        : _low(low), _high(high), _data(data), _maxHigh(high)
    {
    }

    KRYS_NODISCARD const T &Low() const noexcept
    {
      return _low;
    }

    KRYS_NODISCARD const T &High() const noexcept
    {
      return _high;
    }

    KRYS_NODISCARD const TUserData &Data() const noexcept
    {
      return _data;
    }

    KRYS_NODISCARD bool Overlaps(const T &low, const T &high) const noexcept
    {
      return !(_high < low || high < _low);
    }

    KRYS_NODISCARD bool Overlaps(const Interval &other) const noexcept
    {
      return Overlaps(other._low, other._high);
    }

    /// @brief The highest endpoint in the interval tree's subtree rooted at this interval's node.
    KRYS_NODISCARD const T &MaxHigh() const noexcept
    {
      return _maxHigh;
    }

    void SetMaxHigh(const T &maxHigh) noexcept
    {
      _maxHigh = maxHigh;
    }

    KRYS_NODISCARD bool operator<(const Interval &other) const noexcept
    {
      if (_low < other._low)
      {
        return true;
      }
      if (other._low < _low)
      {
        return false;
      }
      if (_high < other._high)
      {
        return true;
      }
      if (other._high < _high)
      {
        return false;
      }
      return _data < other._data;
    }

    KRYS_NODISCARD bool operator==(const Interval &other) const noexcept
    {
      return _low == other._low && _high == other._high && _data == other._data;
    }
  };

  /// @brief Keeps each node's `MaxHigh` the highest endpoint of its subtree.
  struct IntervalNodeUpdater
  {
    template <typename TNode>
    static bool Update(TNode &node) noexcept
    {
      auto *curMax = &node.Data().High();
      auto *left = node.Left();
      if (left)
      {
        if (*curMax < left->Data().MaxHigh())
        {
          curMax = &left->Data().MaxHigh();
        }
      }
      auto *right = node.Right();
      if (right)
      {
        if (*curMax < right->Data().MaxHigh())
        {
          curMax = &right->Data().MaxHigh();
        }
      }
      // This is phrased like this to avoid needing operator!= on type T.
      if (!(*curMax == node.Data().MaxHigh()))
      {
        node.Data().SetMaxHigh(*curMax);
        return true;
      }
      return false;
    }
  };

  /// @brief An interval tree, an augmented red-black tree supporting O(lg n) insertion, removal and querying
  /// of intervals.
  template <typename T, typename TUserData>
  class IntervalTree final : public RedBlackTree<Interval<T, TUserData>, IntervalNodeUpdater>
  {
    using Base = RedBlackTree<Interval<T, TUserData>, IntervalNodeUpdater>;
    using IntervalNode = typename Base::Node;

  public:
    using IntervalType = Interval<T, TUserData>;

    /// @brief Collects the intervals that overlap a query interval.
    class OverlapsSearchAdapter
    {
    private:
      List<IntervalType> &_result;
      const IntervalType &_interval;

    public:
      OverlapsSearchAdapter(List<IntervalType> &result, const IntervalType &interval) noexcept
          : _result(result), _interval(interval)
      {
      }

      KRYS_NODISCARD const T &LowValue() const noexcept
      {
        return _interval.Low();
      }

      KRYS_NODISCARD const T &HighValue() const noexcept
      {
        return _interval.High();
      }

      void CollectIfNeeded(const IntervalType &data) const noexcept
      {
        if (data.Overlaps(_interval))
        {
          _result.push_back(data);
        }
      }
    };

    /// @brief All the intervals in the tree that overlap @p interval, sorted by the `<` operator.
    KRYS_NODISCARD List<IntervalType> allOverlaps(const IntervalType &interval) const noexcept
    {
      List<IntervalType> result;
      OverlapsSearchAdapter adapter(result, interval);
      allOverlapsWithAdapter(adapter);
      return result;
    }

    /// @brief Visits the intervals that may overlap `[adapter.LowValue(), adapter.HighValue()]`, in
    /// increasing order of their low endpoints, by calling `adapter.CollectIfNeeded(interval)`.
    template <typename TAdapter>
    void allOverlapsWithAdapter(TAdapter &adapter) const noexcept
    {
      SearchForOverlapsFrom(this->Root(), adapter);
    }

    /// @brief The interval with the smallest low endpoint greater than @p point.
    KRYS_NODISCARD Maybe<IntervalType> nextIntervalAfter(const T &point) const noexcept
    {
      auto *next = SmallestNodeGreaterThanFrom(point, this->Root());
      if (!next)
      {
        return null;
      }
      return next->Data();
    }

    KRYS_NODISCARD bool checkInvariants() const noexcept
    {
      if (!Base::checkInvariants())
      {
        return false;
      }
      if (!this->Root())
      {
        return true;
      }
      return CheckInvariantsFromNode(this->Root(), nullptr);
    }

  private:
    // Starting from the given node, adds all overlaps with the given
    // interval to the result vector. The intervals are sorted by
    // increasing low endpoint.
    template <typename TAdapter>
    void SearchForOverlapsFrom(IntervalNode *node, TAdapter &adapter) const noexcept
    {
      if (!node)
      {
        return;
      }

      // Because the intervals are sorted by left endpoint, inorder
      // traversal produces results sorted as desired.

      // See whether we need to traverse the left subtree.
      IntervalNode *left = node->Left();
      if (left
          // This is phrased this way to avoid the need for operator
          // <= on type T.
          && !(left->Data().MaxHigh() < adapter.LowValue()))
      {
        SearchForOverlapsFrom<TAdapter>(left, adapter);
      }

      // Check for overlap with current node.
      adapter.CollectIfNeeded(node->Data());

      // See whether we need to traverse the right subtree.
      // This is phrased this way to avoid the need for operator <=
      // on type T.
      if (!(adapter.HighValue() < node->Data().Low()))
      {
        SearchForOverlapsFrom<TAdapter>(node->Right(), adapter);
      }
    }

    KRYS_NODISCARD IntervalNode *SmallestNodeGreaterThanFrom(const T &point,
                                                             IntervalNode *node) const noexcept
    {
      if (!node)
      {
        return nullptr;
      }

      if (!(point < node->Data().Low()))
      {
        return SmallestNodeGreaterThanFrom(point, node->Right());
      }

      if (auto *left = SmallestNodeGreaterThanFrom(point, node->Left()))
      {
        return left;
      }

      return node;
    }

    bool CheckInvariantsFromNode(IntervalNode *node, T *currentMaxValue) const noexcept
    {
      // These assignments are only done in order to avoid requiring
      // a default constructor on type T.
      T leftMaxValue(node->Data().MaxHigh());
      T rightMaxValue(node->Data().MaxHigh());
      IntervalNode *left = node->Left();
      IntervalNode *right = node->Right();
      if (left)
      {
        if (!CheckInvariantsFromNode(left, &leftMaxValue))
        {
          return false;
        }
      }
      if (right)
      {
        if (!CheckInvariantsFromNode(right, &rightMaxValue))
        {
          return false;
        }
      }
      if (!left && !right)
      {
        // Base case.
        if (currentMaxValue)
        {
          *currentMaxValue = node->Data().High();
        }
        return (node->Data().High() == node->Data().MaxHigh());
      }
      T localMaxValue(node->Data().MaxHigh());
      if (!left || !right)
      {
        if (left)
        {
          localMaxValue = leftMaxValue;
        }
        else
        {
          localMaxValue = rightMaxValue;
        }
      }
      else
      {
        localMaxValue = (leftMaxValue < rightMaxValue) ? rightMaxValue : leftMaxValue;
      }
      if (localMaxValue < node->Data().High())
      {
        localMaxValue = node->Data().High();
      }
      if (!(localMaxValue == node->Data().MaxHigh()))
      {
        return false;
      }
      if (currentMaxValue)
      {
        *currentMaxValue = localMaxValue;
      }
      return true;
    }
  };
}
