#pragma once

#include "Krystal.Booey/Loader/ImageLoader.hpp"

namespace krys::boo::html
{
  /// @brief The image loader of HTML elements: fires `load`, or `error` if the load failed.
  class HTMLImageLoader final : public ImageLoader
  {
  public:
    explicit HTMLImageLoader(dom::Element &element) noexcept;
    ~HTMLImageLoader() noexcept override;

  private:
    void DispatchLoadEvent() noexcept override;
    void NotifyFinished(CachedResource &resource) noexcept override;
  };
}
