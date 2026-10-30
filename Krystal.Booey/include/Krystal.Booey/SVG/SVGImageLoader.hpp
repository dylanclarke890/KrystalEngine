#pragma once

#include "Krystal.Booey/Loader/ImageLoader.hpp"

namespace krys::boo::svg
{
  class SVGImageElement;

  /// @brief The image loader of `<image>`: fires `error` if the load failed, or `load` once the element's
  /// required resources have loaded.
  class SVGImageLoader final : public ImageLoader
  {
  public:
    explicit SVGImageLoader(SVGImageElement &element) noexcept;
    ~SVGImageLoader() noexcept override;

  private:
    void DispatchLoadEvent() noexcept override;
  };
}
