#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::threads
{
  class WorkQueue;
}

namespace krys::boo
{
  class LoaderStrategy;
  class PasteboardStrategy;
  class PathStrokeStrategy;

  /// @brief The strategies the host provides for what Booey can't do itself.
  /// @note The host owns the strategies; they must outlive their use.
  class PlatformStrategies
  {
  private:
    LoaderStrategy *_loaderStrategy {nullptr};
    PasteboardStrategy *_pasteboardStrategy {nullptr};
    PathStrokeStrategy *_pathStrokeStrategy {nullptr};
    threads::WorkQueue *_workQueue {nullptr};

  public:
    KRYS_NODISCARD static PlatformStrategies &Singleton() noexcept;

    /// @brief How resources are loaded; null until the host sets one, and then loads fail.
    KRYS_NODISCARD LoaderStrategy *GetLoaderStrategy() const noexcept
    {
      return _loaderStrategy;
    }
    void SetLoaderStrategy(LoaderStrategy *loaderStrategy) noexcept
    {
      _loaderStrategy = loaderStrategy;
    }

    /// @brief How the system clipboard is written; null until the host sets one, and then copying does
    /// nothing.
    KRYS_NODISCARD PasteboardStrategy *GetPasteboardStrategy() const noexcept
    {
      return _pasteboardStrategy;
    }
    void SetPasteboardStrategy(PasteboardStrategy *pasteboardStrategy) noexcept
    {
      _pasteboardStrategy = pasteboardStrategy;
    }

    /// @brief How strokes become geometry; null until the host sets one, and then nothing is inside a stroke
    /// and strokes' bounds are estimated.
    KRYS_NODISCARD PathStrokeStrategy *GetPathStrokeStrategy() const noexcept
    {
      return _pathStrokeStrategy;
    }
    void SetPathStrokeStrategy(PathStrokeStrategy *pathStrokeStrategy) noexcept
    {
      _pathStrokeStrategy = pathStrokeStrategy;
    }

    /// @brief Where work that can run off the main thread goes, its completions posting back to the event
    /// loop's inbox; null until the host sets one, and then that work runs on the main thread when it's
    /// needed.
    KRYS_NODISCARD threads::WorkQueue *GetWorkQueue() const noexcept
    {
      return _workQueue;
    }

    void SetWorkQueue(threads::WorkQueue *workQueue) noexcept
    {
      _workQueue = workQueue;
    }
  };
}
