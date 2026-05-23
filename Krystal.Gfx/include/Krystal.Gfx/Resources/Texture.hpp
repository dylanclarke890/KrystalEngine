#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include <cassert>

namespace Krys::Gfx::OpenGL
{
  enum class TextureOwner : uint8
  {
    TextureRegistry,
    Other
  };

  class Texture : NonCopyable<Texture>
  {
  private:
    ImageViewHandle _imageView {};
    SamplerHandle _sampler {};
    TextureOwner _owner {TextureOwner::Other};

  public:
    Texture(ImageViewHandle imageView, SamplerHandle sampler, TextureOwner owner) noexcept
        : _imageView(imageView), _sampler(sampler), _owner(owner)
    {
      assert(_imageView.IsValid() && "Image view handle must be valid.");
      assert(_sampler.IsValid() && "Sampler handle must be valid.");
    }

    ~Texture() noexcept = default;

    Texture(Texture &&other) noexcept
        : _imageView(std::exchange(other._imageView, ImageViewHandle {0u})),
          _sampler(std::exchange(other._sampler, SamplerHandle {0u})),
          _owner(std::exchange(other._owner, TextureOwner::Other))
    {
    }

    Texture &operator=(Texture &&other) noexcept
    {
      if (this != &other)
      {
        _imageView = std::exchange(other._imageView, ImageViewHandle {0u});
        _sampler = std::exchange(other._sampler, SamplerHandle {0u});
        _owner = std::exchange(other._owner, TextureOwner::Other);
      }
      return *this;
    }

    KRYS_NODISCARD SamplerHandle Sampler() const noexcept
    {
      return _sampler;
    }

    KRYS_NODISCARD ImageViewHandle ImageView() const noexcept
    {
      return _imageView;
    }

    KRYS_NODISCARD TextureOwner Owner() const noexcept
    {
      return _owner;
    }
  };
}