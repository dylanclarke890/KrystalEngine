#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Common.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx::OpenGL
{
  class ImageView
  {
    NO_COPY(ImageView)

  private:
    GLuint _id {0u};
    ImageHandle _imageHandle;
    GLuint _image;
    GLenum _target {GL_TEXTURE_2D};
    GLenum _internalFormat {GL_RGBA8};
    SubResourceRange _subResourceRange {};

  public:
    MOVE_SWAP(ImageView)

    ImageView(ImageHandle imageHandle, GLuint image, GLenum target, GLenum internalFormat,
              SubResourceRange subResourceRange) noexcept
        : _id(0u), _imageHandle(imageHandle), _image(image), _target(target), _internalFormat(internalFormat),
          _subResourceRange(subResourceRange)
    {
      glGenTextures(1, &_id);
      glTextureView(_id, _target, _image, _internalFormat, _subResourceRange.BaseMipLevel,
                    _subResourceRange.MipLevelCount, _subResourceRange.BaseArrayLayer,
                    _subResourceRange.LayerCount);
    }

    ~ImageView() noexcept
    {
      if (_id != 0u)
      {
        glDeleteTextures(1, &_id);
      }
    }

    void Bind(uint32 unit) const noexcept
    {
      glBindTextureUnit(unit, _id);
    }

    NO_DISCARD GLuint Id() const noexcept
    {
      return _id;
    }

    NO_DISCARD ImageHandle Image() const noexcept
    {
      return _imageHandle;
    }

    NO_DISCARD GLenum Target() const noexcept
    {
      return _target;
    }

    NO_DISCARD GLenum InternalFormat() const noexcept
    {
      return _internalFormat;
    }

    NO_DISCARD const SubResourceRange &GetSubResourceRange() const noexcept
    {
      return _subResourceRange;
    }

  private:
    void Swap(ImageView &other) noexcept
    {
      std::swap(other._id, _id);
      std::swap(other._target, _target);
      std::swap(other._internalFormat, _internalFormat);
      std::swap(other._subResourceRange, _subResourceRange);
    }
  };
}