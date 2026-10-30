#include "Krystal.Booey/Storage/StorageArea.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"

namespace krys::boo
{
  uint32 StorageMap::SizeInBytes(dom::DOMStringView string) noexcept
  {
    uint64 codeUnits = 0;
    for (text::CodePointIterator it(Span<const char8>(string.data(), string.size())); !it.AtEnd(); ++it)
    {
      codeUnits += *it > 0xFFFF ? 2 : 1;
    }
    return static_cast<uint32>(std::min<uint64>(codeUnits * 2, UINT32_MAX));
  }

  Maybe<dom::DOMString> StorageMap::Key(uint32 index) const noexcept
  {
    if (index >= Length())
    {
      return null;
    }
    return _keys[index];
  }

  Maybe<dom::DOMString> StorageMap::GetItem(dom::DOMStringView key) const noexcept
  {
    auto it = _map.find(dom::DOMString(key));
    if (it == _map.end())
    {
      return null;
    }
    return it->second;
  }

  bool StorageMap::SetItem(const dom::DOMString &key, const dom::DOMString &value,
                           Maybe<dom::DOMString> &oldValue) noexcept
  {
    uint64 newSize = _currentSize;
    auto it = _map.find(key);
    if (it != _map.end())
    {
      oldValue = it->second;
      newSize -= SizeInBytes(it->second);
    }
    else
    {
      oldValue = null;
      newSize += SizeInBytes(key);
    }
    newSize += SizeInBytes(value);

    if (_quotaSize != NoQuota && newSize > _quotaSize)
    {
      return false;
    }

    if (it != _map.end())
    {
      it->second = value;
    }
    else
    {
      _map.emplace(key, value);
      _keys.push_back(key);
    }
    _currentSize = static_cast<uint32>(newSize);
    return true;
  }

  void StorageMap::SetItemIgnoringQuota(const dom::DOMString &key, const dom::DOMString &value) noexcept
  {
    auto quota = std::exchange(_quotaSize, NoQuota);
    Maybe<dom::DOMString> oldValue;
    (void)SetItem(key, value, oldValue);
    _quotaSize = quota;
  }

  Maybe<dom::DOMString> StorageMap::RemoveItem(const dom::DOMString &key) noexcept
  {
    auto it = _map.find(key);
    if (it == _map.end())
    {
      return null;
    }
    Maybe<dom::DOMString> oldValue = krys::move(it->second);
    _currentSize -= SizeInBytes(key) + SizeInBytes(*oldValue);
    _map.erase(it);
    std::erase(_keys, key);
    return oldValue;
  }

  void StorageMap::Clear() noexcept
  {
    _map.clear();
    _keys.clear();
    _currentSize = 0;
  }

  bool StorageMap::Contains(dom::DOMStringView key) const noexcept
  {
    return _map.contains(dom::DOMString(key));
  }

  Ref<StorageArea> MemoryStorageProvider::LocalStorageArea(dom::DOMStringView origin) noexcept
  {
    dom::DOMString key(origin);
    auto it = _areas.find(key);
    if (it != _areas.end())
    {
      return it->second;
    }
    Ref<StorageArea> area =
      MemoryStorageArea::Create(StorageType::Local, MemoryStorageArea::DefaultLocalStorageQuota);
    _areas.emplace(krys::move(key), area);
    return area;
  }

  MemoryStorageProvider &MemoryStorageProvider::Singleton() noexcept
  {
    static MemoryStorageProvider provider;
    return provider;
  }
}
