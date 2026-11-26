#pragma once

#include "Krystal.Core/Macros.hpp"
#include "Krystal.Web/Loader/LoaderStrategy.hpp"

namespace krys
{
  class WorkQueue;
}

namespace krys::boo
{
  /// @brief Loads `file:` URLs for Krystal.Web pages: reads the file and reports its type from its extension.
  /// @note Other URLs fail to load.
  class FileLoaderStrategy final : public web::LoaderStrategy
  {
    KRYS_NON_COPY_MOVABLE(FileLoaderStrategy);

  private:
    WorkQueue *_readQueue;

  public:
    /// @brief Reads files on @p readQueue, as jobs that may block (so a `JobSystem` keeps its other jobs'
    /// workers), and gives Krystal.Web each file once it's read, through its event loop's inbox. Without a queue,
    /// files are read as they're asked for.
    explicit FileLoaderStrategy(WorkQueue *readQueue = nullptr) noexcept : _readQueue(readQueue)
    {
    }

    void LoadResource(web::Frame &, const net::ResourceRequest &request, const web::ResourceLoaderOptions &,
                      Func<void(web::LoadResult &&)> &&completionHandler) noexcept override;
  };
}
