#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.IO/Image.hpp"
#include "Krystal.Lib/Pair.hpp"
#include "Krystal.Lib/Span.hpp"
#include "Krystal.Lib/StronglyTypedBool.hpp"
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
    Texture(ImageViewHandle imageView, SamplerHandle sampler) noexcept
        : _imageView(imageView), _sampler(sampler)
    {
      assert(_imageView.IsValid() && "Image view handle must be valid.");
      assert(_sampler.IsValid() && "Sampler handle must be valid.");
    }

    ~Texture() noexcept = default;

    Texture(Texture &&other) noexcept : _sampler {}, _imageView {}
    {
      Swap(other);
    }

    Texture &operator=(Texture &&other) noexcept
    {
      if (this != &other)
      {
        Swap(other);
      }
      return *this;
    }

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