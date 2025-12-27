#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Mappers/Enums/FilterMode.hpp"
#include "Krystal.Gfx.OpenGL/Mappers/Enums/WrapMode.hpp"
#include "Krystal.Gfx/Resources/Sampler.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"

namespace Krys::Gfx::OpenGL
{
  class Sampler : NonCopyable<Sampler>
  {
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

    Sampler(Sampler &&other) noexcept
        : _id(std::exchange(other._id, 0u)), _minFilter(std::exchange(other._minFilter, GL_LINEAR)),
          _magFilter(std::exchange(other._magFilter, GL_LINEAR)),
          _wrapS(std::exchange(other._wrapS, GL_REPEAT)), _wrapT(std::exchange(other._wrapT, GL_REPEAT)),
          _wrapR(std::exchange(other._wrapR, GL_REPEAT)),
          _anisotropicLevel(std::exchange(other._anisotropicLevel, 1.0f))
    {
    }

    Sampler &operator=(Sampler &&other) noexcept
    {
      if (this != &other)
      {
        glDeleteSamplers(1, &_id);

        _id = std::exchange(other._id, 0u);
        _minFilter = std::exchange(other._minFilter, GL_LINEAR);
        _magFilter = std::exchange(other._magFilter, GL_LINEAR);
        _wrapS = std::exchange(other._wrapS, GL_REPEAT);
        _wrapT = std::exchange(other._wrapT, GL_REPEAT);
        _wrapR = std::exchange(other._wrapR, GL_REPEAT);
        _anisotropicLevel = std::exchange(other._anisotropicLevel, 1.0f);
      }
      return *this;
    }

    GLuint Id() const noexcept
    {
      return _id;
    }

    void Bind(GLuint unit) const noexcept
    {
      glBindSampler(unit, _id);
    }
  };
}