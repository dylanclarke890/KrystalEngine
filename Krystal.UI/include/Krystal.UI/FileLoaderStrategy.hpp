#pragma once

#include "Krystal.Booey/Loader/LoaderStrategy.hpp"

namespace krys::ui
{
  /// @brief Loads `file:` URLs for Booey pages: reads the file and reports its type from its extension.
  /// @note Other URLs fail to load.
  // FIXME: Read files off the main thread; they're read when asked for, and Booey queues the completion.
  class FileLoaderStrategy final : public boo::LoaderStrategy
  {
  public:
    void LoadResource(boo::LocalFrame &, const boo::ResourceRequest &request,
                      const boo::ResourceLoaderOptions &,
                      Func<void(boo::LoadResult &&)> &&completionHandler) noexcept override;
  };
}
