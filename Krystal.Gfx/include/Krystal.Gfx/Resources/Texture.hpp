#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include <cassert>

namespace Krys::Gfx::OpenGL
{
  enum class TextureOwner : uint8
  {
    TextureRegistry,
    Other
  };

  class Texture
  {
    NO_COPY(Texture)

  private:
    ImageViewHandle _imageView {};
    SamplerHandle _sampler {};
    TextureOwner _owner {TextureOwner::Other};

  public:
    MOVE_SWAP(Texture)

    Texture(ImageViewHandle imageView, SamplerHandle sampler, TextureOwner owner) noexcept
        : _imageView(imageView), _sampler(sampler), _owner(owner)
    {
      assert(_imageView.IsValid() && "Image view handle must be valid.");
      assert(_sampler.IsValid() && "Sampler handle must be valid.");
    }

    ~Texture() noexcept = default;

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

  private:
    void Swap(Texture &other) noexcept
    {
      std::swap(_imageView, other._imageView);
      std::swap(_sampler, other._sampler);
      std::swap(_owner, other._owner);
    }
  };
}