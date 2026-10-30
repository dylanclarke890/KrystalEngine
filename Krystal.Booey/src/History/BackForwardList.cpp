#include "Krystal.Booey/History/BackForwardList.hpp"
#include "Krystal.Booey/History/BackForwardCache.hpp"
#include <algorithm>

namespace krys::boo
{
  BackForwardList::~BackForwardList() noexcept = default;

  void BackForwardList::AddItem(Ref<HistoryItem> &&newItem) noexcept
  {
    if (!_capacity || !_enabled)
    {
      return;
    }

    // Toss anything in the forward list
    if (_current != NoCurrentItemIndex)
    {
      uint32 targetSize = _current + 1;
      while (_entries.size() > targetSize)
      {
        Ref<HistoryItem> item = _entries.back();
        _entries.pop_back();
        _entryHash.erase(item.get());
        BackForwardCache::Singleton().Remove(*item);
      }
    }

    // Toss the first item if the list is getting too big, as long as we're not using it (or even if we are,
    // if we only want 1 entry).
    if (_entries.size() == _capacity && (_current || _capacity == 1))
    {
      Ref<HistoryItem> item = _entries.front();
      _entries.erase(_entries.begin());
      _entryHash.erase(item.get());
      BackForwardCache::Singleton().Remove(*item);
      --_current;
    }

    _entryHash.insert(newItem.get());
    _entries.insert(_entries.begin() + (_current + 1), krys::move(newItem));
    ++_current;
  }

  void BackForwardList::GoBack() noexcept
  {
    krys_debug_assert(_current > 0 && _current != NoCurrentItemIndex);
    if (_current > 0 && _current != NoCurrentItemIndex)
    {
      _current--;
    }
  }

  void BackForwardList::GoForward() noexcept
  {
    krys_debug_assert(_current < _entries.size() - 1);
    if (_current < _entries.size() - 1)
    {
      _current++;
    }
  }

  void BackForwardList::GoToItem(HistoryItem &item) noexcept
  {
    if (_entries.empty())
    {
      return;
    }

    uint32 index = 0;
    for (; index < _entries.size(); ++index)
    {
      if (_entries[index].get() == &item)
      {
        break;
      }
    }

    if (index < _entries.size())
    {
      _current = index;
    }
  }

  RefPtr<HistoryItem> BackForwardList::BackItem() const noexcept
  {
    if (_current && _current != NoCurrentItemIndex)
    {
      return _entries[_current - 1];
    }
    return nullptr;
  }

  RefPtr<HistoryItem> BackForwardList::CurrentItem() const noexcept
  {
    if (_current != NoCurrentItemIndex)
    {
      return _entries[_current];
    }
    return nullptr;
  }

  RefPtr<HistoryItem> BackForwardList::ForwardItem() const noexcept
  {
    if (!_entries.empty() && _current < _entries.size() - 1)
    {
      return _entries[_current + 1];
    }
    return nullptr;
  }

  List<Ref<HistoryItem>> BackForwardList::BackListWithLimit(int32 limit) const noexcept
  {
    List<Ref<HistoryItem>> list;
    if (_current != NoCurrentItemIndex)
    {
      auto first = static_cast<uint32>(std::max(static_cast<int32>(_current) - limit, 0));
      for (; first < _current; ++first)
      {
        list.push_back(_entries[first]);
      }
    }
    return list;
  }

  List<Ref<HistoryItem>> BackForwardList::ForwardListWithLimit(int32 limit) const noexcept
  {
    krys_debug_assert(limit > -1);
    List<Ref<HistoryItem>> list;
    if (_entries.empty())
    {
      return list;
    }

    auto lastEntry = static_cast<uint32>(_entries.size() - 1);
    if (_current < lastEntry)
    {
      auto last = std::min(_current + static_cast<uint32>(limit), lastEntry);
      for (auto index = _current + 1; index <= last; ++index)
      {
        list.push_back(_entries[index]);
      }
    }
    return list;
  }

  void BackForwardList::SetCapacity(uint32 size) noexcept
  {
    while (size < _entries.size())
    {
      Ref<HistoryItem> item = _entries.back();
      _entries.pop_back();
      _entryHash.erase(item.get());
      BackForwardCache::Singleton().Remove(*item);
    }

    if (!size)
    {
      _current = NoCurrentItemIndex;
    }
    else if (_current != NoCurrentItemIndex && _current > _entries.size() - 1)
    {
      _current = static_cast<uint32>(_entries.size() - 1);
    }

    _capacity = size;
  }

  void BackForwardList::SetEnabled(bool enabled) noexcept
  {
    _enabled = enabled;
    if (!enabled)
    {
      auto capacity = _capacity;
      SetCapacity(0);
      SetCapacity(capacity);
    }
  }

  uint32 BackForwardList::BackListCount() const noexcept
  {
    return _current == NoCurrentItemIndex ? 0 : _current;
  }

  uint32 BackForwardList::ForwardListCount() const noexcept
  {
    return _current == NoCurrentItemIndex ? 0 : static_cast<uint32>(_entries.size()) - _current - 1;
  }

  RefPtr<HistoryItem> BackForwardList::ItemAtIndex(int32 index) noexcept
  {
    if (_current == NoCurrentItemIndex)
    {
      return nullptr;
    }

    // Do range checks without doing math on index to avoid overflow.
    if (index < -static_cast<int32>(_current))
    {
      return nullptr;
    }

    if (index > static_cast<int32>(ForwardListCount()))
    {
      return nullptr;
    }

    return _entries[static_cast<uint32>(index + static_cast<int32>(_current))];
  }

  void BackForwardList::Close() noexcept
  {
    _entries.clear();
    _entryHash.clear();
    _current = NoCurrentItemIndex;
    _closed = true;
  }

  void BackForwardList::RemoveItem(HistoryItem &item) noexcept
  {
    for (uint32 i = 0; i < _entries.size(); ++i)
    {
      if (_entries[i].get() != &item)
      {
        continue;
      }

      _entries.erase(_entries.begin() + i);
      _entryHash.erase(&item);
      if (_current == NoCurrentItemIndex || _current < i)
      {
        break;
      }
      if (_current > i)
      {
        _current--;
      }
      else
      {
        auto count = static_cast<uint32>(_entries.size());
        if (_current >= count)
        {
          _current = count ? count - 1 : NoCurrentItemIndex;
        }
      }
      break;
    }
  }

  bool BackForwardList::ContainsItem(const HistoryItem &entry) const noexcept
  {
    return _entryHash.contains(&entry);
  }
}
