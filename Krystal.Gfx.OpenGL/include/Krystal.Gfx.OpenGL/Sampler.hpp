#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Lib/Macros.hpp"
#include <type_traits>

namespace Krys::Gfx::OpenGL
{
  class Sampler
  {
    NO_COPY(Sampler)

  private:
    GLuint _id {0u};
    GLenum _minFilter {GL_LINEAR};
    GLenum _magFilter {GL_LINEAR};
    GLenum _wrapS {GL_REPEAT};
    GLenum _wrapT {GL_REPEAT};
    GLenum _wrapR {GL_REPEAT};
    float _anisotropicLevel {1.0f};

  public:
    Sampler(GLenum minFilter, GLenum magFilter, GLenum wrapS, GLenum wrapT, GLenum wrapR,
            float anisotropicLevel) noexcept
        : _minFilter(minFilter), _magFilter(magFilter), _wrapS(wrapS), _wrapT(wrapT), _wrapR(wrapR),
          _anisotropicLevel(anisotropicLevel)
    {
      glCreateSamplers(1, &_id);
      glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, _minFilter);
      glSamplerParameteri(_id, GL_TEXTURE_MAG_FILTER, _magFilter);
      glSamplerParameteri(_id, GL_TEXTURE_WRAP_S, _wrapS);
      glSamplerParameteri(_id, GL_TEXTURE_WRAP_T, _wrapT);
      glSamplerParameteri(_id, GL_TEXTURE_WRAP_R, _wrapR);
      if (_anisotropicLevel > 1.0f)
      {
        glSamplerParameterf(_id, GL_TEXTURE_MAX_ANISOTROPY, _anisotropicLevel);
      }
    }

    ~Sampler() noexcept
    {
      if (_id != 0u)
      {
        glDeleteSamplers(1, &_id);
      }
    }

    Sampler(Sampler &&other) noexcept : _id(0u)
    {
      Swap(other);
    }

    Sampler &operator=(Sampler &&other) noexcept
    {
      if (this != &other)
      {
        Swap(other);
      }
      return *this;
    }

    GLuint Id() const noexcept
    {
      return _id;
    }

  private:
    void Swap(Sampler &other) noexcept
    {
      std::swap(_id, other._id);
      std::swap(_minFilter, other._minFilter);
      std::swap(_magFilter, other._magFilter);
      std::swap(_wrapS, other._wrapS);
      std::swap(_wrapT, other._wrapT);
      std::swap(_wrapR, other._wrapR);
      std::swap(_anisotropicLevel, other._anisotropicLevel);
    }
  };
}