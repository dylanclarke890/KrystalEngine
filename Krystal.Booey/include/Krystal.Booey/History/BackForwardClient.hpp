#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class HistoryItem;

  /// @brief A page's back/forward list: its history items, and which is current.
  /// @note `BackForwardList` keeps them in memory.
  class BackForwardClient : public RefCounted<BackForwardClient>
  {
    KRYS_NON_COPY_MOVABLE(BackForwardClient);

  public:
    BackForwardClient() noexcept = default;
    virtual ~BackForwardClient() noexcept = default;

    /// @brief Adds @p item after the current item, which it becomes; the items after the current item go.
    virtual void AddItem(Ref<HistoryItem> &&item) noexcept = 0;

    /// @brief Makes @p item the current item.
    virtual void GoToItem(HistoryItem &item) noexcept = 0;

    KRYS_NODISCARD virtual List<Ref<HistoryItem>> AllItems() noexcept = 0;

    /// @brief The item @p index items after the current one (before it, if negative); null if there's none.
    KRYS_NODISCARD virtual RefPtr<HistoryItem> ItemAtIndex(int32 index) noexcept = 0;

    KRYS_NODISCARD virtual uint32 BackListCount() const noexcept = 0;
    KRYS_NODISCARD virtual uint32 ForwardListCount() const noexcept = 0;
    KRYS_NODISCARD virtual bool ContainsItem(const HistoryItem &item) const noexcept = 0;

    virtual void Close() noexcept = 0;
  };
}
