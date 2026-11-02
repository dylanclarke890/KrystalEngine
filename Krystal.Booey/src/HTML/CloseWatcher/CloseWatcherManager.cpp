#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcherManager.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include <ranges>

namespace krys::boo::html
{
  // https://html.spec.whatwg.org/#establish-a-close-watcher
  void CloseWatcherManager::Add(Ref<CloseWatcher> watcher) noexcept
  {
    // 4. If manager's groups's size is less than manager's allowed number of groups, then append
    //    « closeWatcher » to manager's groups.
    if (_groups.size() < _allowedNumberOfGroups)
    {
      List<Ref<CloseWatcher>> newGroup;
      newGroup.push_back(krys::move(watcher));
      _groups.push_back(krys::move(newGroup));
    }
    // 5. Otherwise:
    else
    {
      // 5.1. Assert: manager's groups's size is at least 1 in this branch, since manager's allowed number of
      //      groups is always at least 1.
      krys_debug_assert(!_groups.empty());
      // 5.2. Append closeWatcher to manager's groups's last item.
      _groups.back().push_back(krys::move(watcher));
    }

    // 6. Set manager's next user interaction allows a new group to true.
    _nextUserInteractionAllowsNewGroup = true;
  }

  // https://html.spec.whatwg.org/#close-watcher-destroy
  void CloseWatcherManager::Remove(CloseWatcher &watcher) noexcept
  {
    // 2. For each group of manager's groups: remove closeWatcher from group.
    // 3. Remove any item from manager's groups that is empty.
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

  // https://html.spec.whatwg.org/#notify-the-close-watcher-manager-about-user-activation
  void CloseWatcherManager::NotifyAboutUserActivation() noexcept
  {
    // 1. Let manager be window's close watcher manager.
    // 2. If manager's next user interaction allows a new group is true, then increment manager's allowed
    //    number of groups.
    if (_nextUserInteractionAllowsNewGroup)
    {
      ++_allowedNumberOfGroups;
    }

    // 3. Set manager's next user interaction allows a new group to false.
    _nextUserInteractionAllowsNewGroup = false;
  }

  bool CloseWatcherManager::CanPreventClose() const noexcept
  {
    return _groups.size() < _allowedNumberOfGroups;
  }

  // https://html.spec.whatwg.org/#close-request-steps
  // (Run from the Escape keydown's default action, so once it fired uncanceled.)
  bool CloseWatcherManager::EscapeKeyHandler(dom::KeyboardEvent &event) noexcept
  {
    // 1. If document's fullscreen element is not null: fully exit fullscreen given document's node
    //    navigable's top-level traversable's active document, and return.
    // (There are no fullscreen elements.)
    // 2. Optionally, skip to the step labeled alternative processing.
    // 3. Fire any relevant events, per UI Events or other relevant specifications.
    // 4. Let event be null if no such events are fired, or the Event object representing one of the fired
    //    events otherwise.
    // 5. If event is not null, and its canceled flag is set, then return.
    // 6. If document is not fully active, then return.
    // (Each close watcher's request to close checks it.)
    // 7. Let closedSomething be the result of processing close watchers on document's relevant global object.
    // 8. If closedSomething is true, then return.
    // 9. Alternative processing: Otherwise, there was nothing watching for a close request.
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

  // https://html.spec.whatwg.org/#process-close-watchers
  bool CloseWatcherManager::ProcessCloseWatchers() noexcept
  {
    // 1. Let processedACloseWatcher be false.
    bool processedACloseWatcher = false;
    // 2. If window's close watcher manager's groups is not empty:
    if (!_groups.empty())
    {
      // 2.1. Let group be the last item in window's close watcher manager's groups.
      auto groupCopy = _groups.back();
      // 2.2. For each closeWatcher of group, in reverse order:
      for (auto &watcher : groupCopy | std::views::reverse)
      {
        // 2.2.1. If the result of running closeWatcher's get enabled state is true, set
        // processedACloseWatcher
        //        to true.
        if (watcher->IsEnabled())
        {
          processedACloseWatcher = true;
        }
        // 2.2.2. Let shouldProceed be the result of requesting to close closeWatcher with true.
        // 2.2.3. If shouldProceed is false, then break.
        if (!watcher->RequestToClose(true))
        {
          break;
        }
      }
    }

    // 3. If window's close watcher manager's allowed number of groups is greater than 1, decrement it by 1.
    if (_allowedNumberOfGroups > 1)
    {
      --_allowedNumberOfGroups;
    }
    // 4. Return processedACloseWatcher.
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
