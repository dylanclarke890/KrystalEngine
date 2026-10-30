#pragma once

#include "Krystal.Booey/History/BackForwardClient.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class HistoryItem;
  class Page;

  /// @brief A page's session history: its back/forward list (the client's), and going back and forward
  /// through it.
  class BackForwardController final
  {
    KRYS_NON_COPY_MOVABLE(BackForwardController);

  private:
    Page &_page;
    Ref<BackForwardClient> _client;

  public:
    BackForwardController(Page &page, Ref<BackForwardClient> &&client) noexcept;
    ~BackForwardController() noexcept;

    KRYS_NODISCARD BackForwardClient &Client() const noexcept
    {
      return *_client;
    }

    /// @brief Whether there's an item @p distance items from the current one (before it, if negative).
    KRYS_NODISCARD bool CanGoBackOrForward(int32 distance) const noexcept;

    /// @brief Goes @p distance items back (if negative) or forward, or as far as the list goes.
    void GoBackOrForward(int32 distance) noexcept;

    /// @return Whether there was an item to go back to.
    bool GoBack() noexcept;
    /// @return Whether there was an item to go forward to.
    bool GoForward() noexcept;

    void AddItem(Ref<HistoryItem> &&item) noexcept;
    void SetCurrentItem(HistoryItem &item) noexcept;

    /// @brief How many items the list has.
    KRYS_NODISCARD uint32 Count() const noexcept;
    KRYS_NODISCARD uint32 BackCount() const noexcept;
    KRYS_NODISCARD uint32 ForwardCount() const noexcept;

    KRYS_NODISCARD bool ContainsItem(const HistoryItem &item) const noexcept;

    KRYS_NODISCARD RefPtr<HistoryItem> BackItem() const noexcept;
    KRYS_NODISCARD RefPtr<HistoryItem> CurrentItem() const noexcept;
    KRYS_NODISCARD RefPtr<HistoryItem> ForwardItem() const noexcept;
    KRYS_NODISCARD RefPtr<HistoryItem> ItemAtIndex(int32 index) const noexcept;
    KRYS_NODISCARD List<Ref<HistoryItem>> AllItems() const noexcept;

    void Close() noexcept;
  };
}
