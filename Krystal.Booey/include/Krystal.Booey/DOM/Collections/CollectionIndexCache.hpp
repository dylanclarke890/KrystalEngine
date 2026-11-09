/*
 * Copyright (C) 2013-2017 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Base.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::dom
{
  /// @brief What a live collection knows of its items: the last one reached by index, so moving on from it
  /// doesn't walk from the start again, and how many there are, with all of them, once that's been asked.
  /// @tparam TCollection Has `WillValidateIndexCache()`, called before the cache goes from knowing nothing to
  /// knowing something, `CollectionBegin()` and `CollectionNext(TNode &)`, null at the end.
  /// @note The nodes aren't kept alive: the collection invalidates the cache as they leave it.
  template <typename TCollection, typename TNode>
  class CollectionIndexCache
  {
  private:
    TNode *_current {nullptr};
    size_t _currentIndex {0};
    size_t _count {0};
    List<TNode *> _list;
    bool _isCountValid {false};
    bool _isListValid {false};

  public:
    KRYS_NODISCARD bool HasValidCache() const noexcept
    {
      return _current || _isCountValid;
    }

    /// @brief The number of nodes in the collection.
    KRYS_NODISCARD size_t Count(const TCollection &collection) noexcept
    {
      if (!_isCountValid)
      {
        if (!HasValidCache())
        {
          collection.WillValidateIndexCache();
        }

        for (auto *node = collection.CollectionBegin(); node; node = collection.CollectionNext(*node))
        {
          _list.push_back(node);
        }
        _count = _list.size();
        _isCountValid = true;
        _isListValid = true;
      }
      return _count;
    }

    /// @brief The node at @p index, or null if there's none.
    KRYS_NODISCARD TNode *At(const TCollection &collection, size_t index) noexcept
    {
      if (_isCountValid && index >= _count)
      {
        return nullptr;
      }

      if (_isListValid)
      {
        return _list[index];
      }

      if (!_current || index < _currentIndex)
      {
        if (!HasValidCache())
        {
          collection.WillValidateIndexCache();
        }

        _current = collection.CollectionBegin();
        _currentIndex = 0;
        if (!_current)
        {
          _count = 0;
          _isCountValid = true;
          return nullptr;
        }
      }

      while (_currentIndex < index)
      {
        auto *next = collection.CollectionNext(*_current);
        if (!next)
        {
          // The index wasn't reached, but the walk found how many there are.
          _count = _currentIndex + 1;
          _isCountValid = true;
          return nullptr;
        }
        _current = next;
        ++_currentIndex;
      }
      return _current;
    }

    void Invalidate() noexcept
    {
      _current = nullptr;
      _isCountValid = false;
      _isListValid = false;
      _list.clear();
    }
  };
}
