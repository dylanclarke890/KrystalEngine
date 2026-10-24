#include "Krystal.Booey/History/BackForwardController.hpp"
#include "Krystal.Booey/History/HistoryItem.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"

namespace krys::boo
{
  BackForwardController::BackForwardController(Page &page, Ref<BackForwardClient> &&client) noexcept
      : _page(page), _client(krys::move(client))
  {
  }

  BackForwardController::~BackForwardController() noexcept = default;

  RefPtr<HistoryItem> BackForwardController::BackItem() const noexcept
  {
    return ItemAtIndex(-1);
  }

  RefPtr<HistoryItem> BackForwardController::CurrentItem() const noexcept
  {
    return ItemAtIndex(0);
  }

  RefPtr<HistoryItem> BackForwardController::ForwardItem() const noexcept
  {
    return ItemAtIndex(1);
  }

  bool BackForwardController::CanGoBackOrForward(int32 distance) const noexcept
  {
    if (!distance)
    {
      return true;
    }
    if (distance > 0 && static_cast<uint32>(distance) <= ForwardCount())
    {
      return true;
    }
    if (distance < 0 && static_cast<uint32>(-distance) <= BackCount())
    {
      return true;
    }
    return false;
  }

  void BackForwardController::GoBackOrForward(int32 distance) noexcept
  {
    if (!distance)
    {
      return;
    }

    RefPtr<HistoryItem> historyItem = ItemAtIndex(distance);
    if (!historyItem)
    {
      if (distance > 0)
      {
        if (auto forwardCount = static_cast<int32>(ForwardCount()))
        {
          historyItem = ItemAtIndex(forwardCount);
        }
      }
      else
      {
        if (auto backCount = static_cast<int32>(BackCount()))
        {
          historyItem = ItemAtIndex(-backCount);
        }
      }
    }

    if (!historyItem)
    {
      return;
    }

    Ref<Page> page = ShareRef(_page);
    page->GoToItem(page->MainFrame(), *historyItem, FrameLoadType::IndexedBackForward);
  }

  bool BackForwardController::GoBack() noexcept
  {
    RefPtr<HistoryItem> historyItem = BackItem();
    if (!historyItem)
    {
      return false;
    }

    Ref<Page> page = ShareRef(_page);
    page->GoToItem(page->MainFrame(), *historyItem, FrameLoadType::Back);
    return true;
  }

  bool BackForwardController::GoForward() noexcept
  {
    RefPtr<HistoryItem> historyItem = ForwardItem();
    if (!historyItem)
    {
      return false;
    }

    Ref<Page> page = ShareRef(_page);
    page->GoToItem(page->MainFrame(), *historyItem, FrameLoadType::Forward);
    return true;
  }

  void BackForwardController::AddItem(Ref<HistoryItem> &&item) noexcept
  {
    _client->AddItem(krys::move(item));
  }

  void BackForwardController::SetCurrentItem(HistoryItem &item) noexcept
  {
    _client->GoToItem(item);
  }

  bool BackForwardController::ContainsItem(const HistoryItem &item) const noexcept
  {
    return _client->ContainsItem(item);
  }

  uint32 BackForwardController::Count() const noexcept
  {
    return _client->BackListCount() + 1 + _client->ForwardListCount();
  }

  uint32 BackForwardController::BackCount() const noexcept
  {
    return _client->BackListCount();
  }

  uint32 BackForwardController::ForwardCount() const noexcept
  {
    return _client->ForwardListCount();
  }

  RefPtr<HistoryItem> BackForwardController::ItemAtIndex(int32 index) const noexcept
  {
    return _client->ItemAtIndex(index);
  }

  List<Ref<HistoryItem>> BackForwardController::AllItems() const noexcept
  {
    return _client->AllItems();
  }

  void BackForwardController::Close() noexcept
  {
    _client->Close();
  }
}
