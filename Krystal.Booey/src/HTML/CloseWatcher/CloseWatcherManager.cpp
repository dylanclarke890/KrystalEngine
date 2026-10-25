#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcherManager.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include <ranges>

namespace krys::boo::html
{
  void CloseWatcherManager::Add(Ref<CloseWatcher> watcher) noexcept
  {
    if (_groups.size() < _allowedNumberOfGroups)
    {
      List<Ref<CloseWatcher>> newGroup;
      newGroup.push_back(krys::move(watcher));
      _groups.push_back(krys::move(newGroup));
    }
    else
    {
      krys_debug_assert(!_groups.empty());
      _groups.back().push_back(krys::move(watcher));
    }

    _nextUserInteractionAllowsNewGroup = true;
  }

  void CloseWatcherManager::Remove(CloseWatcher &watcher) noexcept
  {
    for (auto group = _groups.begin(); group != _groups.end();)
    {
      std::erase_if(*group,
                    [&watcher](const Ref<CloseWatcher> &current) { return current.get() == &watcher; });
      if (group->empty())
      {
        group = _groups.erase(group);
      }
      else
      {
        ++group;
      }
    }
  }

  void CloseWatcherManager::NotifyAboutUserActivation() noexcept
  {
    if (_nextUserInteractionAllowsNewGroup)
    {
      ++_allowedNumberOfGroups;
    }

    _nextUserInteractionAllowsNewGroup = false;
  }

  bool CloseWatcherManager::CanPreventClose() const noexcept
  {
    return _groups.size() < _allowedNumberOfGroups;
  }

  bool CloseWatcherManager::EscapeKeyHandler(dom::KeyboardEvent &event) noexcept
  {
    if (!event.DefaultHandled() && event.IsTrusted() && event.Key() == u8"Escape")
    {
      return ProcessCloseWatchers();
    }

    if (_allowedNumberOfGroups > 1)
    {
      --_allowedNumberOfGroups;
    }
    return false;
  }

  bool CloseWatcherManager::ProcessCloseWatchers() noexcept
  {
    bool processedACloseWatcher = false;
    if (!_groups.empty())
    {
      auto groupCopy = _groups.back();
      for (auto &watcher : groupCopy | std::views::reverse)
      {
        if (watcher->IsEnabled())
        {
          processedACloseWatcher = true;
        }
        if (!watcher->RequestToClose(true))
        {
          break;
        }
      }
    }

    if (_allowedNumberOfGroups > 1)
    {
      --_allowedNumberOfGroups;
    }
    return processedACloseWatcher;
  }

  void CloseWatcherManager::DestroyAll() noexcept
  {
    auto groups = _groups;
    for (auto &group : groups)
    {
      for (auto &watcher : group)
      {
        watcher->Destroy();
      }
    }
  }
}
