#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.IO/Image.hpp"
#include "Krystal.Lib/Pair.hpp"
#include "Krystal.Lib/Span.hpp"
#include "Krystal.Lib/StronglyTypedBool.hpp"
#include <cassert>

namespace Krys::Gfx::OpenGL
{
  using TextureData = Span<const byte>;

  struct IsCubemap : StronglyTypedTrue<IsCubemap>
  {
  };

  class Texture
  {
    NO_COPY(Texture)

  private:
    GLuint _id;
    GLenum _type;
    GLenum _internalFormat;
    GLsizei _width;
    GLsizei _height;
    GLsizei _depth;
    uint32 _mipLevels;

  public:
    Texture(GLenum internalFormat, GLsizei width, uint32 mipLevels) noexcept
        : _id(0u), _type(GL_TEXTURE_1D), _internalFormat(internalFormat), _width(width), _height(1),
          _depth(1), _mipLevels(std::max(1u, mipLevels))
    {
      glCreateTextures(_type, 1, &_id);
      AllocateStorage();
    }

    Texture(GLenum internalFormat, GLsizei width, GLsizei height, uint32 mipLevels) noexcept
        : _id(0u), _type(GL_TEXTURE_2D), _internalFormat(internalFormat), _width(width), _height(height),
          _depth(1), _mipLevels(std::max(1u, mipLevels))
    {
      glCreateTextures(_type, 1, &_id);
      AllocateStorage();
    }

    Texture(GLenum internalFormat, GLsizei width, GLsizei height, uint32 mipLevels, IsCubemap) noexcept
        : _id(0u), _type(GL_TEXTURE_CUBE_MAP), _internalFormat(internalFormat), _width(width),
          _height(height), _depth(1), _mipLevels(std::max(1u, mipLevels))
    {
      glCreateTextures(_type, 1, &_id);
      AllocateStorage();
    }

    Texture(GLenum type, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth,
            uint32 mipLevels) noexcept
        : _id(0u), _type(type), _internalFormat(internalFormat), _width(width), _height(height),
          _depth(depth), _mipLevels(std::max(1u, mipLevels))
    {
      assert((_type == GL_TEXTURE_3D || _type == GL_TEXTURE_2D_ARRAY) && "Invalid type for 3D/array ctor");
      glCreateTextures(_type, 1, &_id);
      AllocateStorage();
    }

    ~Texture() noexcept
    {
      if (_id != 0u)
      {
        glDeleteTextures(1, &_id);
      }
    }

    Texture(Texture &&other) noexcept
        : _id(0u), _type(GL_NONE), _internalFormat(GL_NONE), _width(0), _height(0), _depth(1), _mipLevels(1u)
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

    GLuint GetId() const noexcept
    {
      return _id;
    }

    void Bind(GLuint unit) const noexcept
    {
      glBindTextureUnit(unit, _id);
    }

    void SetParameter(GLenum pname, GLint param) const noexcept
    {
      glTextureParameteri(_id, pname, param);
    }

    void SetParameter(GLenum pname, GLfloat param) const noexcept
    {
      glTextureParameterf(_id, pname, param);
    }

    /// @brief Sets the contents of a 2d texture.
    /// @param data The texture data.
    /// @param format Format of the pixel data (e.g., GL_RGB, GL_RGBA).
    /// @param dataType Data type of the pixel data (e.g., GL_UNSIGNED_BYTE, GL_FLOAT).
    void SetData(const TextureData &data, GLenum format, GLenum dataType) const noexcept
    {
      assert(_type == GL_TEXTURE_2D && "SetData only supports 2D textures.");
      glTextureSubImage2D(_id, 0, 0, 0, _width, _height, format, dataType, data.data());
    }

    /// @brief Sets the contents of a cubemap face.
    /// @param face The face to update (e.g., GL_TEXTURE_CUBE_MAP_POSITIVE_X).
    /// @param data The texture data.
    /// @param format Format of the pixel data (e.g., GL_RGB, GL_RGBA).
    /// @param dataType Data type of the pixel data (e.g., GL_UNSIGNED_BYTE, GL_FLOAT).
    void SetData(GLenum face, const TextureData &data, GLenum format, GLenum dataType) const noexcept
    {
      assert(_type == GL_TEXTURE_CUBE_MAP && "SetData only supports cubemap textures.");
      glTextureSubImage3D(_id, 0, 0, 0, face - GL_TEXTURE_CUBE_MAP_POSITIVE_X, _width, _height, 1, format,
                          dataType, data.data());
    }

    void GenerateMipmaps() const noexcept
    {
      if (_mipLevels > 1u)
      {
        glGenerateTextureMipmap(_id);
      }
    }

  private:
    void AllocateStorage() const noexcept
    {
      switch (_type)
      {
        case GL_TEXTURE_1D:       glTextureStorage1D(_id, _mipLevels, _internalFormat, _width); break;
        case GL_TEXTURE_CUBE_MAP: // Cubemap faces are implicit; just like 2D but 6 faces
        case GL_TEXTURE_2D:       glTextureStorage2D(_id, _mipLevels, _internalFormat, _width, _height); break;
        case GL_TEXTURE_3D:
        case GL_TEXTURE_2D_ARRAY:
          glTextureStorage3D(_id, _mipLevels, _internalFormat, _width, _height, _depth);
          break;

        default: assert(false && "Unsupported texture type");
      }
    }

    void Swap(Texture &other) noexcept
    {
      std::swap(_id, other._id);
      std::swap(_type, other._type);
      std::swap(_internalFormat, other._internalFormat);
      std::swap(_width, other._width);
      std::swap(_height, other._height);
      std::swap(_depth, other._depth);
      std::swap(_mipLevels, other._mipLevels);
    }
  };
}