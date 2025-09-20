#pragma once

#include "Krystal.Gfx.OpenGL/Hooks/gl.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.IO/Streams/NativeFileStream.hpp"
#include "Krystal.IO/Streams/StreamUtils.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <cassert>

namespace Krys::Gfx::OpenGL
{
  class Shader
  {
    GLuint _handle;

  public:
    Shader(const Krys::IO::Path &vertexFilepath, const Krys::IO::Path &fragmentFilepath) noexcept
        : _handle(glCreateProgram())
    {
      auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
      CreateShader(vertexFilepath, vertexShader);

      auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
      CreateShader(fragmentFilepath, fragmentShader);

      glAttachShader(_handle, vertexShader);
      glAttachShader(_handle, fragmentShader);

      Link();

      glDeleteShader(vertexShader);
      glDeleteShader(fragmentShader);
    }

    Shader(const Krys::IO::Path &vertexFilepath, const Krys::IO::Path &geometryFilepath,
           const Krys::IO::Path &fragmentFilepath) noexcept
        : _handle(glCreateProgram())
    {
      auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
      CreateShader(vertexFilepath, vertexShader);

      auto geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
      CreateShader(geometryFilepath, geometryShader);

      auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
      CreateShader(fragmentFilepath, fragmentShader);

      glAttachShader(_handle, vertexShader);
      glAttachShader(_handle, geometryShader);
      glAttachShader(_handle, fragmentShader);

      Link();

      glDeleteShader(vertexShader);
      glDeleteShader(geometryShader);
      glDeleteShader(fragmentShader);
    }

    ~Shader() noexcept
    {
      glDeleteProgram(_handle);
    }

    void Bind() const noexcept
    {
      glUseProgram(_handle);
    }

    template <typename T>
    void SetUniform(const string &uniformName, const T &value, bool required = false) const noexcept
    {
      using namespace Krys::Maths;

      GLint location = glGetUniformLocation(_handle, uniformName.c_str());
      if (location == -1)
      {
        assert(!required && "Uniform not found in shader, possibly optimized out by the compiler.");
        return;
      }

#define IsUniformType(type) constexpr(std::is_same_v<T, type>)

      if IsUniformType (bool)
        glProgramUniform1i(_handle, location, value);
      else if IsUniformType (int32)
        glProgramUniform1i(_handle, location, value);
      else if IsUniformType (uint32)
        glProgramUniform1ui(_handle, location, value);
      else if IsUniformType (float32)
        glProgramUniform1f(_handle, location, value);
      else if IsUniformType (Vec2)
        glProgramUniform2f(_handle, location, value.x, value.y);
      else if IsUniformType (Vec3)
        glProgramUniform3f(_handle, location, value.x, value.y, value.z);
      else if IsUniformType (Vec4)
        glProgramUniform4f(_handle, location, value.x, value.y, value.z, value.w);
      else if IsUniformType (Mat2)
        glProgramUniformMatrix2fv(_handle, location, 1, GL_FALSE, &value[0].x);
      else if IsUniformType (Mat3)
        glProgramUniformMatrix3fv(_handle, location, 1, GL_FALSE, &value[0].x);
      else if IsUniformType (Mat4)
        glProgramUniformMatrix4fv(_handle, location, 1, GL_FALSE, &value[0].x);
      else if IsUniformType (List<int32>)
        glProgramUniform1iv(_handle, location, static_cast<GLsizei>(value.size()), value.data());
      else if IsUniformType (List<uint32>)
        glProgramUniform1uiv(_handle, location, static_cast<GLsizei>(value.size()), value.data());
      else if IsUniformType (List<float32>)
        glProgramUniform1fv(_handle, location, static_cast<GLsizei>(value.size()), value.data());
      else if IsUniformType (List<Vec2>)
        glProgramUniform2fv(_handle, location, static_cast<GLsizei>(value.size()), &value[0].x);
      else if IsUniformType (List<Vec3>)
        glProgramUniform3fv(_handle, location, static_cast<GLsizei>(value.size()), &value[0].x);
      else if IsUniformType (List<Vec4>)
        glProgramUniform4fv(_handle, location, static_cast<GLsizei>(value.size()), &value[0].x);
      else if IsUniformType (List<Mat2>)
        glProgramUniformMatrix2fv(_handle, location, static_cast<GLsizei>(value.size()), GL_FALSE,
                                  &value[0][0].x);
      else if IsUniformType (List<Mat3>)
        glProgramUniformMatrix3fv(_handle, location, static_cast<GLsizei>(value.size()), GL_FALSE,
                                  &value[0][0].x);
      else if IsUniformType (List<Mat4>)
        glProgramUniformMatrix4fv(_handle, location, static_cast<GLsizei>(value.size()), GL_FALSE,
                                  &value[0][0].x);
      else
        static_assert(false, "Unsupported uniform type.");

#undef IsUniformType
    }

  private:
    void CreateShader(const Krys::IO::Path &filepath, unsigned int shader) noexcept
    {
      auto reader = Krys::IO::NativeFileReader(filepath);
      auto sourceResult = Krys::IO::StreamUtils::ReadAllText(reader);
      assert(sourceResult.has_value());

      auto source = sourceResult->c_str();
      glShaderSource(shader, 1, &source, NULL);
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
      glLinkProgram(_handle);

      int success;
      char infoLog[512];
      glGetProgramiv(_handle, GL_LINK_STATUS, &success);
      if (!success)
      {
        glGetProgramInfoLog(_handle, 512, NULL, infoLog);
        assert(false && "Shader program linking failed. See log for details.");
      }
    }
  };
}