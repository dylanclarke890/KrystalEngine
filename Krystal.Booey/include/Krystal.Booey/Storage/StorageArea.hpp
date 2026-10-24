#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief Which storage an area belongs to: a page's session storage, or an origin's local storage.
  enum class StorageType : uint8
  {
    Session,
    Local,
  };

  /// @brief The key/value pairs of one origin's storage, with a quota (in bytes of UTF-16, as script counts
  /// them). Keys keep the order they were added in.
  /// @see https://html.spec.whatwg.org/#storage-2
  class StorageMap
  {
  public:
    static constexpr uint32 NoQuota = UINT32_MAX;

  private:
    HashMap<dom::DOMString, dom::DOMString> _map;
    List<dom::DOMString> _keys;
    uint32 _currentSize {0};
    uint32 _quotaSize;

  public:
    explicit StorageMap(uint32 quota) noexcept : _quotaSize(quota)
    {
    }

    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return static_cast<uint32>(_keys.size());
    }

    KRYS_NODISCARD Maybe<dom::DOMString> Key(uint32 index) const noexcept;
    KRYS_NODISCARD Maybe<dom::DOMString> GetItem(dom::DOMStringView key) const noexcept;

    /// @param oldValue The value @p key had, if any.
    /// @return False, changing nothing, if the item would take the map past its quota.
    bool SetItem(const dom::DOMString &key, const dom::DOMString &value,
                 Maybe<dom::DOMString> &oldValue) noexcept;

    /// @brief Sets the item whatever the quota (for items read back from disk).
    void SetItemIgnoringQuota(const dom::DOMString &key, const dom::DOMString &value) noexcept;

    /// @return The value @p key had, if any.
    Maybe<dom::DOMString> RemoveItem(const dom::DOMString &key) noexcept;

    void Clear() noexcept;

    KRYS_NODISCARD bool Contains(dom::DOMStringView key) const noexcept;

    /// @brief The keys, in order.
    KRYS_NODISCARD const List<dom::DOMString> &Keys() const noexcept
    {
      return _keys;
    }

    /// @brief @p string's size in bytes of UTF-16.
    KRYS_NODISCARD static uint32 SizeInBytes(dom::DOMStringView string) noexcept;
  };

  /// @brief Where a `Storage` object's items are kept: the host can keep local storage its own way (on disk)
  /// by implementing it, or by deriving from `MemoryStorageArea`.
  class StorageArea : public RefCounted<StorageArea>
  {
    KRYS_NON_COPY_MOVABLE(StorageArea);

  public:
    StorageArea() noexcept = default;
    virtual ~StorageArea() noexcept = default;

    KRYS_NODISCARD virtual uint32 Length() noexcept = 0;
    KRYS_NODISCARD virtual Maybe<dom::DOMString> Key(uint32 index) noexcept = 0;
    KRYS_NODISCARD virtual Maybe<dom::DOMString> Item(dom::DOMStringView key) noexcept = 0;

    /// @param oldValue The value @p key had, if any.
    /// @return False, changing nothing, if the area's quota doesn't allow it.
    virtual bool SetItem(const dom::DOMString &key, const dom::DOMString &value,
                         Maybe<dom::DOMString> &oldValue) noexcept = 0;

    /// @return The value @p key had, if any.
    virtual Maybe<dom::DOMString> RemoveItem(const dom::DOMString &key) noexcept = 0;

    /// @return Whether the area had items.
    virtual bool Clear() noexcept = 0;

    KRYS_NODISCARD virtual StorageType GetStorageType() const noexcept = 0;
  };

  /// @brief A storage area kept in memory.
  class MemoryStorageArea : public StorageArea
  {
  protected:
    StorageMap _map;
    StorageType _storageType;

  public:
    /// @brief The quota of a local storage area, as upstream: 5MB.
    static constexpr uint32 DefaultLocalStorageQuota = 5 * 1'024 * 1'024;

    MemoryStorageArea(StorageType storageType, uint32 quota) noexcept : _map(quota), _storageType(storageType)
    {
    }

    KRYS_NODISCARD static Ref<MemoryStorageArea> Create(StorageType storageType, uint32 quota) noexcept
    {
      return CreateRef<MemoryStorageArea>(storageType, quota);
    }

    KRYS_NODISCARD uint32 Length() noexcept override
    {
      return _map.Length();
    }

    KRYS_NODISCARD Maybe<dom::DOMString> Key(uint32 index) noexcept override
    {
      return _map.Key(index);
    }

    KRYS_NODISCARD Maybe<dom::DOMString> Item(dom::DOMStringView key) noexcept override
    {
      return _map.GetItem(key);
    }

    bool SetItem(const dom::DOMString &key, const dom::DOMString &value,
                 Maybe<dom::DOMString> &oldValue) noexcept override
    {
      return _map.SetItem(key, value, oldValue);
    }

    Maybe<dom::DOMString> RemoveItem(const dom::DOMString &key) noexcept override
    {
      return _map.RemoveItem(key);
    }

    bool Clear() noexcept override
    {
      bool hadItems = _map.Length() != 0;
      _map.Clear();
      return hadItems;
    }

    KRYS_NODISCARD StorageType GetStorageType() const noexcept override
    {
      return _storageType;
    }
  };

  /// @brief Gives each origin its local storage area: the host implements it to keep local storage (on disk,
  /// say); without one, local storage is kept in memory for as long as the process runs.
  class StorageProvider
  {
  public:
    virtual ~StorageProvider() noexcept = default;

    /// @brief The local storage area of @p origin (a serialised origin), the same one each time.
    KRYS_NODISCARD virtual Ref<StorageArea> LocalStorageArea(dom::DOMStringView origin) noexcept = 0;
  };

  /// @brief Keeps local storage in memory, by origin.
  class MemoryStorageProvider final : public StorageProvider
  {
  private:
    HashMap<dom::DOMString, Ref<StorageArea>> _areas;

  public:
    KRYS_NODISCARD Ref<StorageArea> LocalStorageArea(dom::DOMStringView origin) noexcept override;

    /// @brief The provider used when the host sets none.
    KRYS_NODISCARD static MemoryStorageProvider &Singleton() noexcept;
  };
}
