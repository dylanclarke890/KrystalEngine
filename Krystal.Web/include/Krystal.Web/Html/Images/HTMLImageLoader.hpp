#pragma once

#include "Krystal.Web/Loader/ImageLoader.hpp"

namespace krys::web::html
{
  /// @brief The image loader of HTML elements: fires `load`, or `error` if the load failed.
  class HTMLImageLoader final : public ImageLoader
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLImageLoader);

  public:
    explicit HTMLImageLoader(web::Element &element) noexcept;

    ~HTMLImageLoader() noexcept override;

  private:
    void DispatchLoadEvent() noexcept override;

    void NotifyFinished(CachedResource &resource) noexcept override;
  };
}
