#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Nullable.hpp"
#include "Krystal.Lib/Queue.hpp"
#include "Krystal.Lib/Types.hpp"
#include <cassert>

namespace Krys::Gfx
{
  template <typename T>
  concept Resource = MoveConstructible<T> && MoveAssignable<T> && !CopyConstructible<T> && !CopyAssignable<T>;

  template <Resource T, typename THandle>
  class ResourceManager
  {
  private:
    struct ResourceEntry
    {
      Nullable<T> Resource;
      uint16 Generation = 1u;
    };

    List<ResourceEntry> _resources;
    Queue<uint16> _freeIndices;

  public:
    NO_DISCARD THandle Add(T &&resource) noexcept
    {
      uint16 index = NextIndex();
      if (index >= _resources.size()) [[unlikely]]
      {
        _resources.push_back(ResourceEntry {});
      }

      ResourceEntry &entry = _resources[index];
      entry.Resource = std::move(resource);

      return CreateHandle(index, entry.Generation);
    }

    NO_DISCARD T &Get(THandle handle)
    {
      uint16 index = GetIndex(handle);
      uint16 generation = GetGeneration(handle);
      if (index >= _resources.size()) [[unlikely]]
      {
        throw std::out_of_range("Invalid handle: index out of range");
      }

      ResourceEntry &entry = _resources[index];
      if (!entry.Resource.has_value() || entry.Generation != generation) [[unlikely]]
      {
        throw std::invalid_argument("Invalid handle: generation mismatch or resource not in use");
      }

      return *entry.Resource;
    }

    NO_DISCARD T *TryGet(THandle handle) noexcept
    {
      try
      {
        return &Get(handle);
      }
      catch (...)
      {
        return nullptr;
      }
    }

    bool Remove(THandle handle) noexcept
    {
      assert(handle.IsValid() && "Invalid handle.");

      uint16 index = GetIndex(handle);
      uint16 generation = GetGeneration(handle);

      if (index >= _resources.size()) [[unlikely]]
      {
        return false;
      }

      ResourceEntry &entry = _resources[index];
      if (!entry.Resource.has_value() || entry.Generation != generation) [[unlikely]]
      {
        return false;
      }

      entry.Resource.reset();
      entry.Generation++;

      _freeIndices.push(index);

      return true;
    }

    auto begin() noexcept
    {
      return _resources.begin();
    }

    auto end() noexcept
    {
      return _resources.end();
    }

  private:
    NO_DISCARD THandle CreateHandle(uint16 index, uint16 generation) const noexcept
    {
      THandle handle {};
      handle.Id = (static_cast<uint32>(generation) << 16) | index;

      return handle;
    }

    NO_DISCARD uint16 GetIndex(THandle handle) const noexcept
    {
      assert(handle.IsValid() && "Invalid handle.");

      return handle.Id & 0xFFFFu;
    }

    NO_DISCARD uint16 GetGeneration(THandle handle) const noexcept
    {
      assert(handle.IsValid() && "Invalid handle.");

      return (handle.Id >> 16) & 0xFFFFu;
    }

    NO_DISCARD uint16 NextIndex() noexcept
    {
      if (!_freeIndices.empty())
      {
        uint16 index = _freeIndices.front();
        _freeIndices.pop();
        return index;
      }
      else
      {
        return static_cast<uint16>(_resources.size());
      }
    }
  };
}