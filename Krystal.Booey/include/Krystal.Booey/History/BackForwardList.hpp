#pragma once

#include "Krystal.Booey/History/BackForwardClient.hpp"
#include "Krystal.Booey/History/HistoryItem.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  /// @brief A page's back/forward list, in memory: up to its capacity of items (100 by default), the oldest
  /// going first.
  class BackForwardList final : public BackForwardClient, public CanMakeWeakPtr<BackForwardList>
  {
    KRYS_NON_COPY_MOVABLE(BackForwardList);

  private:
    static constexpr uint32 DefaultCapacity = 100;
    static constexpr uint32 NoCurrentItemIndex = UINT32_MAX;

    List<Ref<HistoryItem>> _entries;
    HashSet<const HistoryItem *> _entryHash;
    uint32 _current {NoCurrentItemIndex};
    uint32 _capacity {DefaultCapacity};
    bool _closed {false};
    bool _enabled {true};

  public:
    BackForwardList() noexcept = default;
    ~BackForwardList() noexcept override;

    KRYS_NODISCARD static Ref<BackForwardList> Create() noexcept
    {
      return CreateRef<BackForwardList>();
    }

    void AddItem(Ref<HistoryItem> &&newItem) noexcept override;
    void GoBack() noexcept;
    void GoForward() noexcept;
    void GoToItem(HistoryItem &item) noexcept override;

    KRYS_NODISCARD RefPtr<HistoryItem> BackItem() const noexcept;
    KRYS_NODISCARD RefPtr<HistoryItem> CurrentItem() const noexcept;
    KRYS_NODISCARD RefPtr<HistoryItem> ForwardItem() const noexcept;
    KRYS_NODISCARD RefPtr<HistoryItem> ItemAtIndex(int32 index) noexcept override;

    KRYS_NODISCARD List<Ref<HistoryItem>> AllItems() noexcept override
    {
      return _entries;
    }

    /// @brief Up to @p limit items before the current one, the oldest first.
    KRYS_NODISCARD List<Ref<HistoryItem>> BackListWithLimit(int32 limit) const noexcept;
    /// @brief Up to @p limit items after the current one, the nearest first.
    KRYS_NODISCARD List<Ref<HistoryItem>> ForwardListWithLimit(int32 limit) const noexcept;

    KRYS_NODISCARD uint32 Capacity() const noexcept
    {
      return _capacity;
    }
    /// @brief Sets how many items the list keeps; the newest above @p size go.
    void SetCapacity(uint32 size) noexcept;

    KRYS_NODISCARD bool Enabled() const noexcept
    {
      return _enabled;
    }
    /// @brief Whether items are added; disabling the list clears it.
    void SetEnabled(bool enabled) noexcept;

    KRYS_NODISCARD uint32 BackListCount() const noexcept override;
    KRYS_NODISCARD uint32 ForwardListCount() const noexcept override;
    KRYS_NODISCARD bool ContainsItem(const HistoryItem &item) const noexcept override;

    void Close() noexcept override;
    KRYS_NODISCARD bool Closed() const noexcept
    {
      return _closed;
    }

    void RemoveItem(HistoryItem &item) noexcept;

    KRYS_NODISCARD const List<Ref<HistoryItem>> &Entries() const noexcept
    {
      return _entries;
    }
  };
}
