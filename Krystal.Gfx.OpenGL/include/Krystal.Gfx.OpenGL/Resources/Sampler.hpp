#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Resources/Sampler.hpp"
#include "Krystal.Lib/Macros.hpp"

namespace Krys::Gfx::OpenGL
{
  NO_DISCARD constexpr GLenum MapFilterMode(FilterMode mode) noexcept
  {
    switch (mode)
    {
      case FilterMode::Nearest:              return GL_NEAREST;
      case FilterMode::Linear:               return GL_LINEAR;
      case FilterMode::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
      case FilterMode::LinearMipmapNearest:  return GL_LINEAR_MIPMAP_NEAREST;
      case FilterMode::NearestMipmapLinear:  return GL_NEAREST_MIPMAP_LINEAR;
      case FilterMode::LinearMipmapLinear:   return GL_LINEAR_MIPMAP_LINEAR;
      default:                               assert(false && "Unknown enum value: FilterMode"); return GL_INVALID_ENUM;
    }
  }

  NO_DISCARD constexpr FilterMode MapFilterMode(GLenum mode) noexcept
  {
    switch (mode)
    {
      case GL_NEAREST:                return FilterMode::Nearest;
      case GL_LINEAR:                 return FilterMode::Linear;
      case GL_NEAREST_MIPMAP_NEAREST: return FilterMode::NearestMipmapNearest;
      case GL_LINEAR_MIPMAP_NEAREST:  return FilterMode::LinearMipmapNearest;
      case GL_NEAREST_MIPMAP_LINEAR:  return FilterMode::NearestMipmapLinear;
      case GL_LINEAR_MIPMAP_LINEAR:   return FilterMode::LinearMipmapLinear;
      default:                        assert(false && "Unknown GLenum value for FilterMode"); return FilterMode::Nearest;
    }
  }

  NO_DISCARD constexpr GLenum MapWrapMode(WrapMode mode) noexcept
  {
    switch (mode)
    {
      case WrapMode::Repeat:        return GL_REPEAT;
      case WrapMode::ClampToEdge:   return GL_CLAMP_TO_EDGE;
      case WrapMode::ClampToBorder: return GL_CLAMP_TO_BORDER;
      default:                      assert(false && "Unknown enum value: WrapMode"); return GL_INVALID_ENUM;
    }
  }

  NO_DISCARD constexpr WrapMode MapWrapMode(GLenum mode) noexcept
  {
    switch (mode)
    {
      case GL_REPEAT:          return WrapMode::Repeat;
      case GL_CLAMP_TO_EDGE:   return WrapMode::ClampToEdge;
      case GL_CLAMP_TO_BORDER: return WrapMode::ClampToBorder;
      default:                 assert(false && "Unknown GLenum value for WrapMode"); return WrapMode::Repeat;
    }
  }

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
    MOVE_SWAP(Sampler)

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