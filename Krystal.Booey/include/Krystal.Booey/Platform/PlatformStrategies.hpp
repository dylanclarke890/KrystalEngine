#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class LoaderStrategy;
  class PasteboardStrategy;
  class PathStrokeStrategy;
  class StorageProvider;

  /// @brief The strategies the host provides for what Booey can't do itself.
  /// @note The host owns the strategies; they must outlive their use.
  class PlatformStrategies
  {
  private:
    LoaderStrategy *_loaderStrategy {nullptr};
    PasteboardStrategy *_pasteboardStrategy {nullptr};
    PathStrokeStrategy *_pathStrokeStrategy {nullptr};
    StorageProvider *_storageProvider {nullptr};

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

    /// @brief Where origins' local storage is kept; null until the host sets one, and then it's kept in
    /// memory.
    KRYS_NODISCARD StorageProvider *GetStorageProvider() const noexcept
    {
      return _storageProvider;
    }
    void SetStorageProvider(StorageProvider *storageProvider) noexcept
    {
      _storageProvider = storageProvider;
    }
  };
}
