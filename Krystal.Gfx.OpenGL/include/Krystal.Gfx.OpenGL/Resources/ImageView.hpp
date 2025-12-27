#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Resources/ImageView.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::Gfx::OpenGL
{
  class ImageView : NonCopyable<ImageView>
  {
  private:
    GLuint _id {0u};
    ImageHandle _imageHandle;
    GLenum _target {GL_TEXTURE_2D};
    GLenum _internalFormat {GL_RGBA8};
    SubResourceRange _subResourceRange {};

  public:
    ImageView(ImageHandle imageHandle, GLuint image, GLenum target, GLenum internalFormat,
              SubResourceRange subResourceRange) noexcept
        : _id(0u), _imageHandle(imageHandle), _target(target), _internalFormat(internalFormat),
          _subResourceRange(subResourceRange)
    {
      // We use glGenTextures here instead of glCreateTextures because we need an uninitialized texture object
      // for a texture view.
      glGenTextures(1, &_id);
      glTextureView(_id, _target, image, _internalFormat, _subResourceRange.BaseMipLevel,
                    _subResourceRange.MipLevelCount, _subResourceRange.BaseArrayLayer,
                    _subResourceRange.LayerCount);
    }

    ~ImageView() noexcept
    {
      glDeleteTextures(1, &_id);
    }

    ImageView(ImageView &&other) noexcept
        : _id(std::exchange(other._id, 0u)),
          _imageHandle(std::exchange(other._imageHandle, ImageHandle {0u})),
          _target(std::exchange(other._target, GL_TEXTURE_2D)),
          _internalFormat(std::exchange(other._internalFormat, GL_RGBA8)),
          _subResourceRange(std::exchange(other._subResourceRange, SubResourceRange {}))
    {
    }

    ImageView &operator=(ImageView &&other) noexcept
    {
      if (this != &other)
      {
        glDeleteTextures(1, &_id);

        _id = std::exchange(other._id, 0u);
        _imageHandle = std::exchange(other._imageHandle, ImageHandle {0u});
        _target = std::exchange(other._target, GL_TEXTURE_2D);
        _internalFormat = std::exchange(other._internalFormat, GL_RGBA8);
        _subResourceRange = std::exchange(other._subResourceRange, SubResourceRange {});
      }
      return *this;
    }

    void Bind(uint32 unit) const noexcept
    {
      glBindTextureUnit(unit, _id);
    }

    KRYS_NODISCARD GLuint Id() const noexcept
    {
      return _id;
    }

    KRYS_NODISCARD ImageHandle Image() const noexcept
    {
      return _imageHandle;
    }

    KRYS_NODISCARD GLenum Target() const noexcept
    {
      return _target;
    }

    KRYS_NODISCARD GLenum InternalFormat() const noexcept
    {
      return _internalFormat;
    }

    KRYS_NODISCARD const SubResourceRange &GetSubResourceRange() const noexcept
    {
      return _subResourceRange;
    }
  };
}