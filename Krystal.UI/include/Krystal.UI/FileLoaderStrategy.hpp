#pragma once

#include "Krystal.Booey/Loader/LoaderStrategy.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys
{
  class WorkQueue;
}

namespace krys::ui
{
  /// @brief Loads `file:` URLs for Booey pages: reads the file and reports its type from its extension.
  /// @note Other URLs fail to load.
  class FileLoaderStrategy final : public boo::LoaderStrategy
  {
    KRYS_NON_COPY_MOVABLE(FileLoaderStrategy);

  private:
    WorkQueue *_readQueue;

  public:
    /// @brief Reads files on @p readQueue, as jobs that may block (so a `JobSystem` keeps its other jobs'
    /// workers), and gives Booey each file once it's read, through its event loop's inbox. Without a queue,
    /// files are read as they're asked for.
    explicit FileLoaderStrategy(WorkQueue *readQueue = nullptr) noexcept : _readQueue(readQueue)
    {
    }

    void LoadResource(boo::LocalFrame &, const boo::ResourceRequest &request,
                      const boo::ResourceLoaderOptions &,
                      Func<void(boo::LoadResult &&)> &&completionHandler) noexcept override;
  };
}
