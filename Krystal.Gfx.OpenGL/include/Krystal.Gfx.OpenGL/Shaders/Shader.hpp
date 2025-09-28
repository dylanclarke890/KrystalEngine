#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <cassert>

namespace Krys::Gfx::OpenGL
{
  class Shader
  {
    NO_COPY(Shader)

    GLuint _id;

  public:
    Shader(const string &vertex, const string &fragment) noexcept : _id(glCreateProgram())
    {
      auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
      auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

      CreateShader(vertex, vertexShader);
      CreateShader(fragment, fragmentShader);

      glAttachShader(_id, vertexShader);
      glAttachShader(_id, fragmentShader);

      Link();

      glDeleteShader(vertexShader);
      glDeleteShader(fragmentShader);
    }

    Shader(const string &vertex, const string &geometry, const string &fragment) noexcept
        : _id(glCreateProgram())
    {
      auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
      auto geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
      auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

      CreateShader(vertex, vertexShader);
      CreateShader(geometry, geometryShader);
      CreateShader(fragment, fragmentShader);

      glAttachShader(_id, vertexShader);
      glAttachShader(_id, geometryShader);
      glAttachShader(_id, fragmentShader);

      Link();

      glDeleteShader(vertexShader);
      glDeleteShader(geometryShader);
      glDeleteShader(fragmentShader);
    }

    Shader(Shader &&other) noexcept : _id(0u)
    {
      Swap(other);
    }

    Shader &operator=(Shader &&other) noexcept
    {
      if (this != &other)
      {
        Swap(other);
      }
      return *this;
    }

    ~Shader() noexcept
    {
      if (_id != 0u)
      {
        glDeleteProgram(_id);
      }
    }

    void Bind() const noexcept
    {
      glUseProgram(_id);
    }

    template <typename T>
    void SetUniform(const string &uniformName, const T &value, bool required = false) const noexcept
    {
      using namespace Krys::Maths;

      GLint location = glGetUniformLocation(_id, uniformName.c_str());
      if (location == -1)
      {
        assert(!required && "Uniform not found in shader, possibly optimized out by the compiler.");
        return;
      }

#define IsUniformType(type) constexpr(std::is_same_v<T, type>)

      if IsUniformType (bool)
        glProgramUniform1i(_id, location, value);
      else if IsUniformType (int32)
        glProgramUniform1i(_id, location, value);
      else if IsUniformType (uint32)
        glProgramUniform1ui(_id, location, value);
      else if IsUniformType (float32)
        glProgramUniform1f(_id, location, value);
      else if IsUniformType (Vec2)
        glProgramUniform2f(_id, location, value.x, value.y);
      else if IsUniformType (Vec3)
        glProgramUniform3f(_id, location, value.x, value.y, value.z);
      else if IsUniformType (Vec4)
        glProgramUniform4f(_id, location, value.x, value.y, value.z, value.w);
      else if IsUniformType (Mat2)
        glProgramUniformMatrix2fv(_id, location, 1, GL_FALSE, &value[0].x);
      else if IsUniformType (Mat3)
        glProgramUniformMatrix3fv(_id, location, 1, GL_FALSE, &value[0].x);
      else if IsUniformType (Mat4)
        glProgramUniformMatrix4fv(_id, location, 1, GL_FALSE, &value[0].x);
      else if IsUniformType (List<int32>)
        glProgramUniform1iv(_id, location, static_cast<GLsizei>(value.size()), value.data());
      else if IsUniformType (List<uint32>)
        glProgramUniform1uiv(_id, location, static_cast<GLsizei>(value.size()), value.data());
      else if IsUniformType (List<float32>)
        glProgramUniform1fv(_id, location, static_cast<GLsizei>(value.size()), value.data());
      else if IsUniformType (List<Vec2>)
        glProgramUniform2fv(_id, location, static_cast<GLsizei>(value.size()), &value[0].x);
      else if IsUniformType (List<Vec3>)
        glProgramUniform3fv(_id, location, static_cast<GLsizei>(value.size()), &value[0].x);
      else if IsUniformType (List<Vec4>)
        glProgramUniform4fv(_id, location, static_cast<GLsizei>(value.size()), &value[0].x);
      else if IsUniformType (List<Mat2>)
        glProgramUniformMatrix2fv(_id, location, static_cast<GLsizei>(value.size()), GL_FALSE,
                                  &value[0][0].x);
      else if IsUniformType (List<Mat3>)
        glProgramUniformMatrix3fv(_id, location, static_cast<GLsizei>(value.size()), GL_FALSE,
                                  &value[0][0].x);
      else if IsUniformType (List<Mat4>)
        glProgramUniformMatrix4fv(_id, location, static_cast<GLsizei>(value.size()), GL_FALSE,
                                  &value[0][0].x);
      else
        static_assert(false, "Unsupported uniform type.");

#undef IsUniformType
    }

  private:
    void CreateShader(const string &source, GLuint shader) noexcept
    {
      const auto *src = source.c_str();
      glShaderSource(shader, 1, &src, NULL);
      glCompileShader(shader);

      int success;
      char infoLog[512];
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (!success)
      {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        assert(false && "Shader compilation failed. See log for details.");
      }
    }

    void Link() const noexcept
    {
      glLinkProgram(_id);

      int success;
      char infoLog[512];
      glGetProgramiv(_id, GL_LINK_STATUS, &success);
      if (!success)
      {
        glGetProgramInfoLog(_id, 512, NULL, infoLog);
        assert(false && "Shader program linking failed. See log for details.");
      }
    }

    void Swap(Shader &other) noexcept
    {
      std::swap(_id, other._id);
    }
  };
}