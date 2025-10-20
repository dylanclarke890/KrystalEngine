#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Macros.hpp"
#include <cassert>

namespace Krys::Gfx::OpenGL
{
  class Texture
  {
    NO_COPY(Texture)

  private:
    ImageViewHandle _imageView {};
    SamplerHandle _sampler {};

  public:
    MOVE_SWAP(Texture)

    Texture(ImageViewHandle imageView, SamplerHandle sampler) noexcept
        : _imageView(imageView), _sampler(sampler)
    {
      assert(_imageView.IsValid() && "Image view handle must be valid.");
      assert(_sampler.IsValid() && "Sampler handle must be valid.");
    }

    ~Texture() noexcept = default;

    NO_DISCARD SamplerHandle Sampler() const noexcept
    {
      return _sampler;
    }

    NO_DISCARD ImageViewHandle ImageView() const noexcept
    {
      return _imageView;
    }

  private:
    void Swap(Texture &other) noexcept
    {
      std::swap(_imageView, other._imageView);
      std::swap(_sampler, other._sampler);
    }
  };
}