#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class LoaderStrategy;

  /// @brief The strategies the host provides for what Booey can't do itself.
  /// @note The host owns the strategies; they must outlive their use.
  class PlatformStrategies
  {
  private:
    LoaderStrategy *_loaderStrategy {nullptr};

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
  };
}
