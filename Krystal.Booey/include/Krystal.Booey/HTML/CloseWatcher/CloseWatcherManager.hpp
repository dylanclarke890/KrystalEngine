#pragma once

#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcher.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class KeyboardEvent;
}

namespace krys::boo::html
{
  /// @brief A window's close watchers, in groups: each user activation allows one more group, so a page can't
  /// trap the user with watchers made without one; a close request closes the last group.
  /// @see https://html.spec.whatwg.org/#close-watcher-manager
  class CloseWatcherManager final : public RefCounted<CloseWatcherManager>
  {
    KRYS_NON_COPY_MOVABLE(CloseWatcherManager);

  private:
    List<List<Ref<CloseWatcher>>> _groups;
    uint32 _allowedNumberOfGroups {1};
    bool _nextUserInteractionAllowsNewGroup {true};

  public:
    CloseWatcherManager() noexcept = default;

    KRYS_NODISCARD static Ref<CloseWatcherManager> Create() noexcept
    {
      return CreateRef<CloseWatcherManager>();
    }

    /// @see https://html.spec.whatwg.org/#establish-a-close-watcher
    void Add(Ref<CloseWatcher> watcher) noexcept;
    /// @see https://html.spec.whatwg.org/#close-watcher-destroy
    void Remove(CloseWatcher &watcher) noexcept;

    /// @see https://html.spec.whatwg.org/#notify-the-close-watcher-manager-about-user-activation
    void NotifyAboutUserActivation() noexcept;
    /// @brief Whether a close request can be canceled: there's a group to spare.
    KRYS_NODISCARD bool CanPreventClose() const noexcept;

    /// @brief Escape's close request: processes the close watchers, unless @p event was handled or is
    /// untrusted.
    /// @return Whether an enabled watcher was asked.
    bool EscapeKeyHandler(dom::KeyboardEvent &event) noexcept;
    /// @brief A close request (Escape, or another user action the host treats as one): asks the last group's
    /// watchers to close, newest first, until one cancels.
    /// @return Whether an enabled watcher was asked.
    /// @see https://html.spec.whatwg.org/#process-close-watchers
    bool ProcessCloseWatchers() noexcept;

    /// @brief Destroys every watcher: the window's document stopped.
    void DestroyAll() noexcept;
  };
}
