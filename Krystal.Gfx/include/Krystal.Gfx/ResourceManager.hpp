#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Nullable.hpp"
#include "Krystal.Lib/Types/Queue.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
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
    KRYS_NODISCARD THandle Add(T &&resource) noexcept
    {
      uint16 index = NextIndex();
      if (index >= _resources.size()) KRYS_UNLIKELY
      {
        _resources.push_back(ResourceEntry {});
      }

      ResourceEntry &entry = _resources[index];
      entry.Resource = std::move(resource);

      return CreateHandle(index, entry.Generation);
    }

    /// @brief Sets the resource for the given handle. This can either be used to set a resource for a handle
    /// obtained from `NextHandle`, or to replace an existing resource.
    KRYS_NODISCARD void Set(THandle handle, T &&resource)
    {
      assert(handle.IsValid() && "Invalid handle.");

      uint16 index = GetIndex(handle);
      uint16 generation = GetGeneration(handle);

      ResourceEntry &entry = GetResourceEntry(index, generation, false);
      entry.Resource = std::move(resource);
    }

    KRYS_NODISCARD T &Get(THandle handle)
    {
      assert(handle.IsValid() && "Invalid handle.");

      uint16 index = GetIndex(handle);
      uint16 generation = GetGeneration(handle);

      ResourceEntry &entry = GetResourceEntry(index, generation);
      return *entry.Resource;
    }

    KRYS_NODISCARD const T &Get(THandle handle) const
    {
      assert(handle.IsValid() && "Invalid handle.");

      uint16 index = GetIndex(handle);
      uint16 generation = GetGeneration(handle);

      const ResourceEntry &entry = GetResourceEntry(index, generation);
      return *entry.Resource;
    }

    KRYS_NODISCARD T *TryGet(THandle handle) noexcept
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

    KRYS_NODISCARD const T *TryGet(THandle handle) const noexcept
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
    KRYS_NODISCARD THandle NextHandle() noexcept
    {
      uint16 index = NextIndex();
      if (index >= _resources.size()) KRYS_UNLIKELY
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
    KRYS_NODISCARD THandle CreateHandle(uint16 index, uint16 generation) const noexcept
    {
      THandle handle {};
      handle.Id = (static_cast<uint32>(generation) << 16) | index;

      return handle;
    }

    KRYS_NODISCARD uint16 GetIndex(THandle handle) const noexcept
    {
      assert(handle.IsValid() && "Invalid handle.");

      return handle.Id & 0xFFFFu;
    }

    KRYS_NODISCARD uint16 GetGeneration(THandle handle) const noexcept
    {
      assert(handle.IsValid() && "Invalid handle.");

      return (handle.Id >> 16) & 0xFFFFu;
    }

    KRYS_NODISCARD uint16 NextIndex() noexcept
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

    KRYS_NODISCARD ResourceEntry &GetResourceEntry(uint16 index, uint16 generation,
                                               bool ensureResourceIsNotInUse = true)
    {
      if (index >= _resources.size()) KRYS_UNLIKELY
      {
        throw std::out_of_range("Invalid handle: index out of range");
      }

      ResourceEntry &entry = _resources[index];

      if (entry.Generation != generation) KRYS_UNLIKELY
      {
        throw std::invalid_argument("Invalid handle: generation mismatch");
      }

      if (ensureResourceIsNotInUse && !entry.Resource.has_value()) KRYS_UNLIKELY
      {
        throw std::invalid_argument("Invalid handle: resource not in use");
      }

      return entry;
    }

    KRYS_NODISCARD const ResourceEntry &GetResourceEntry(uint16 index, uint16 generation,
                                                     bool ensureResourceIsNotInUse = true) const
    {
      return const_cast<ResourceManager<T, THandle> *>(this)->GetResourceEntry(index, generation,
                                                                               ensureResourceIsNotInUse);
    }
  };
}