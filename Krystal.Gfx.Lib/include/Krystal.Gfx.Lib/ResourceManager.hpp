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
    /// @brief Adds a new resource and returns a handle to it.
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

    /// @brief Sets the resource for the given handle. This can either be used to set a resource for a handle
    /// obtained from `NextHandle`, or to replace an existing resource.
    NO_DISCARD void Set(THandle handle, T &&resource)
    {
      assert(handle.IsValid() && "Invalid handle.");

      uint16 index = GetIndex(handle);
      uint16 generation = GetGeneration(handle);

      ResourceEntry &entry = GetResourceEntry(index, generation, false);
      entry.Resource = std::move(resource);
    }

    NO_DISCARD T &Get(THandle handle)
    {
      assert(handle.IsValid() && "Invalid handle.");

      uint16 index = GetIndex(handle);
      uint16 generation = GetGeneration(handle);

      ResourceEntry &entry = GetResourceEntry(index, generation);
      return *entry.Resource;
    }

    NO_DISCARD const T &Get(THandle handle) const
    {
      assert(handle.IsValid() && "Invalid handle.");

      uint16 index = GetIndex(handle);
      uint16 generation = GetGeneration(handle);

      const ResourceEntry &entry = GetResourceEntry(index, generation);
      return *entry.Resource;
    }

    NO_DISCARD T *TryGet(THandle handle) noexcept
    {
      assert(handle.IsValid() && "Invalid handle.");

      try
      {
        return &Get(handle);
      }
      catch (...)
      {
        return nullptr;
      }
    }

    NO_DISCARD const T *TryGet(THandle handle) const noexcept
    {
      assert(handle.IsValid() && "Invalid handle.");
      try
      {
        return &Get(handle);
      }
      catch (...)
      {
        return nullptr;
      }
    }

    /// @brief Removes the resource associated with the given handle.
    bool Remove(THandle handle) noexcept
    {
      assert(handle.IsValid() && "Invalid handle.");

      uint16 index = GetIndex(handle);
      uint16 generation = GetGeneration(handle);
      ResourceEntry &entry = GetResourceEntry(index, generation);

      entry.Resource.reset();
      entry.Generation++;

      _freeIndices.push(index);

      return true;
    }

    /// @brief Gets the next available handle for a new resource. The resource must then be set using `Set`
    /// instead of `Add`.
    NO_DISCARD THandle NextHandle() noexcept
    {
      uint16 index = NextIndex();
      if (index >= _resources.size()) [[unlikely]]
      {
        _resources.push_back(ResourceEntry {});
      }
      ResourceEntry &entry = _resources[index];
      return CreateHandle(index, entry.Generation);
    }

    auto begin() noexcept
    {
      return _resources.begin();
    }

    auto end() noexcept
    {
      return _resources.end();
    }

    void Clear() noexcept
    {
      _resources.clear();
      while (!_freeIndices.empty())
      {
        _freeIndices.pop();
      }
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

    NO_DISCARD ResourceEntry &GetResourceEntry(uint16 index, uint16 generation,
                                               bool ensureResourceIsNotInUse = true)
    {
      if (index >= _resources.size()) [[unlikely]]
      {
        throw std::out_of_range("Invalid handle: index out of range");
      }

      ResourceEntry &entry = _resources[index];

      if (entry.Generation != generation) [[unlikely]]
      {
        throw std::invalid_argument("Invalid handle: generation mismatch");
      }

      if (ensureResourceIsNotInUse && !entry.Resource.has_value()) [[unlikely]]
      {
        throw std::invalid_argument("Invalid handle: resource not in use");
      }

      return entry;
    }

    NO_DISCARD const ResourceEntry &GetResourceEntry(uint16 index, uint16 generation,
                                                     bool ensureResourceIsNotInUse = true) const
    {
      return const_cast<ResourceManager<T, THandle> *>(this)->GetResourceEntry(index, generation,
                                                                               ensureResourceIsNotInUse);
    }
  };
}