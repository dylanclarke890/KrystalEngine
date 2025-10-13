#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Queue.hpp"
#include "Krystal.Lib/SmartPointers.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI/Element.hpp"
#include <cassert>

namespace Krys::UI
{
  class ElementPool
  {
  private:
    struct ElementEntry
    {
      Unique<Element> Resource;
      uint16 Generation = 1u;
    };

    List<ElementEntry> _elements;
    Queue<uint16> _freeIndices;

  public:
    template <DerivedFrom<Element> TElement, typename... TArgs>
    NO_DISCARD ElementHandle Create(TArgs &&...args)
    {
      uint16 index = NextIndex();
      if (index >= _elements.size()) [[unlikely]]
      {
        _elements.push_back(ElementEntry {});
      }

      ElementEntry &entry = _elements[index];
      auto handle = CreateHandle(index, entry.Generation);

      entry.Resource = CreateUnique<TElement>(std::forward<TArgs>(args)...);
      entry.Resource->_id = handle;

      return handle;
    }

    template <DerivedFrom<Element> TElement>
    NO_DISCARD TElement &Get(ElementHandle handle)
    {
      assert(handle.IsValid() && "Invalid handle.");

      uint16 index = GetIndex(handle);
      uint16 generation = GetGeneration(handle);
      if (index >= _elements.size()) [[unlikely]]
      {
        throw std::out_of_range("Invalid handle: index out of range");
      }

      ElementEntry &entry = _elements[index];
      if (entry.Resource.get() == nullptr || entry.Generation != generation) [[unlikely]]
      {
        throw std::invalid_argument("Invalid handle: generation mismatch or resource not in use");
      }

      return static_cast<TElement &>(*entry.Resource);
    }

    template <DerivedFrom<Element> TElement>
    NO_DISCARD TElement *TryGet(ElementHandle handle) noexcept
    {
      assert(handle.IsValid() && "Invalid handle.");

      try
      {
        return &Get<TElement>(handle);
      }
      catch (...)
      {
        return nullptr;
      }
    }

    bool Remove(ElementHandle handle) noexcept
    {
      assert(handle.IsValid() && "Invalid handle.");

      uint16 index = GetIndex(handle);
      uint16 generation = GetGeneration(handle);

      if (index >= _elements.size()) [[unlikely]]
      {
        return false;
      }

      ElementEntry &entry = _elements[index];
      if (entry.Resource.get() == nullptr || entry.Generation != generation) [[unlikely]]
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
      return _elements.begin();
    }

    auto end() noexcept
    {
      return _elements.end();
    }

  private:
    NO_DISCARD ElementHandle CreateHandle(uint16 index, uint16 generation) const noexcept
    {
      ElementHandle handle {};
      handle.Id = (static_cast<uint32>(generation) << 16) | index;

      return handle;
    }

    NO_DISCARD uint16 GetIndex(ElementHandle handle) const noexcept
    {
      assert(handle.IsValid() && "Invalid handle.");

      return handle.Id & 0xFFFFu;
    }

    NO_DISCARD uint16 GetGeneration(ElementHandle handle) const noexcept
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
        return static_cast<uint16>(_elements.size());
      }
    }
  };
}