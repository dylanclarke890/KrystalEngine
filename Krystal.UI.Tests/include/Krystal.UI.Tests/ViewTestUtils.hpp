#pragma once

#include "Krystal.Booey/History/BackForwardCache.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.UI/View.hpp"

namespace krys::ui::tests
{
  /// @brief Turns the back/forward cache off for as long as it lives, so the pages a view leaves are freed.
  class BackForwardCacheDisabler
  {
    KRYS_NON_COPY_MOVABLE(BackForwardCacheDisabler);

  private:
    uint32 _previousMaxSize;

  public:
    BackForwardCacheDisabler() noexcept : _previousMaxSize(boo::BackForwardCache::Singleton().MaxSize())
    {
      boo::BackForwardCache::Singleton().SetMaxSize(0);
    }

    ~BackForwardCacheDisabler() noexcept
    {
      boo::BackForwardCache::Singleton().SetMaxSize(_previousMaxSize);
    }
  };

  /// @brief Runs the event loop's tasks until @p view's load has committed and finished (or failed), as a
  /// host's frames would.
  inline void WaitForLoad(View &view) noexcept
  {
    auto &eventLoop = boo::html::EventLoop::Current();
    while (view.IsLoading() && eventLoop.RunNextTask())
    {
    }
  }

  /// @brief Runs the event loop's tasks until @p view's load has committed (its document parsed), leaving its
  /// subresources to load.
  inline void WaitForCommit(View &view) noexcept
  {
    auto &eventLoop = boo::html::EventLoop::Current();
    while (view.Frame().Loader().ProvisionalDocumentLoader() && eventLoop.RunNextTask())
    {
    }
  }
  /// @brief Loads the page at @p file into @p view and lets its resources load, as Chrome's capture does:
  /// images and style sheets load as networking tasks, and web fonts once laying text out asks for them.
  /// @return Whether the page loaded.
  inline bool LoadAndSettle(View &view, const std::filesystem::path &file) noexcept
  {
    if (!view.LoadFile(file).has_value())
    {
      return false;
    }
    WaitForLoad(view);
    for (int32 pass = 0; pass < 4; ++pass)
    {
      view.UpdateRendering();
      boo::html::EventLoop::Current().RunUntilIdle();
    }
    return true;
  }
}
