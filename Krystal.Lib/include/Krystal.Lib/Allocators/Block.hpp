#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/SmartPointers.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>

namespace Krys
{
  class Page
  {
    NO_COPY(Page)

  private:
    void *_data {nullptr};

  public:
    static constexpr size_t DefaultSize = 4'096u;

    explicit Page(size_t size = DefaultSize) : _data(std::malloc(size))
    {
      assert(_data && "Failed to allocate page");
    }

    ~Page()
    {
      std::free(_data);
    }

    Page(Page &&other) noexcept : _data(other._data)
    {
      other._data = nullptr;
    }

    Page &operator=(Page &&other) noexcept
    {
      if (this != &other)
      {
        std::free(_data);
        _data = other._data;
        other._data = nullptr;
      }
      return *this;
    }

    void *Data() noexcept
    {
      return _data;
    }

    const void *Data() const noexcept
    {
      return _data;
    }
  };

  struct BlockSnapshot
  {
    size_t Count {0};
    size_t Offset {0};
  };

  class Block
  {
    NO_COPY_MOVE(Block)

  private:
    size_t _pageSize;
    List<Unique<Page>> _pages;
    List<size_t> _offsets; // per-page offsets

  public:
    explicit Block(size_t pageSize = Page::DefaultSize) : _pageSize(pageSize)
    {
      assert(_pageSize > 0);
      _pages.reserve(4);
      _offsets.reserve(4);
      _pages.emplace_back(CreateUnique<Page>(_pageSize));
      _offsets.push_back(0);
    }

    ~Block() = default;

    void *AllocateBytes(size_t size)
    {
      assert(size <= _pageSize && "Allocation larger than page size not supported");

      size_t &offset = _offsets.back();
      if (_pageSize - offset < size)
      {
        // Allocate a new page
        _pages.emplace_back(CreateUnique<Page>(_pageSize));
        _offsets.push_back(0);
      }

      void *pageData = _pages.back()->Data();
      void *ptr = static_cast<uint8 *>(pageData) + _offsets.back();
      _offsets.back() += size;
      return ptr;
    }

    template <typename T>
    T *Allocate(size_t count = 1)
    {
      return static_cast<T *>(AllocateBytes(sizeof(T) * count));
    }

    NO_DISCARD size_t PageSize() const noexcept
    {
      return _pageSize;
    }

    NO_DISCARD size_t PageCount() const noexcept
    {
      return _pages.size();
    }

    NO_DISCARD size_t AllocatedBytes() const noexcept
    {
      return _pages.size() * _pageSize + _offsets.capacity() * sizeof(size_t)
             + _pages.capacity() * sizeof(std::unique_ptr<Page>) + sizeof(*this);
    }

    void Snapshot(BlockSnapshot &snapshot) const noexcept
    {
      snapshot.Count = _pages.size();
      snapshot.Offset = _offsets.back();
    }

    bool Restore(const BlockSnapshot &snapshot) noexcept
    {
      if (snapshot.Count == 0 || snapshot.Count > _pages.size())
      {
        return false;
      }
      if (snapshot.Count == _pages.size() && _offsets.back() < snapshot.Offset)
      {
        return false;
      }

      // Free pages past snapshot
      while (_pages.size() > snapshot.Count)
      {
        _pages.pop_back();
        _offsets.pop_back();
      }

      _offsets.back() = snapshot.Offset;
      return true;
    }

    void Clear()
    {
      _pages.clear();
      _offsets.clear();
      _pages.emplace_back(std::make_unique<Page>(_pageSize));
      _offsets.push_back(0);
    }

    NO_DISCARD void *PageData(size_t pageIndex) noexcept
    {
      assert(pageIndex < _pages.size());
      return _pages[pageIndex]->Data();
    }

    NO_DISCARD const void *PageData(size_t pageIndex) const noexcept
    {
      assert(pageIndex < _pages.size());
      return _pages[pageIndex]->Data();
    }

    NO_DISCARD size_t PageUsed(size_t pageIndex) const noexcept
    {
      assert(pageIndex < _offsets.size());
      return _offsets[pageIndex];
    }
  };
}
