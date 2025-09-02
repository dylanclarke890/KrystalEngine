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
  class OpenGLShader
  {
  public:
    OpenGLShader(const Krys::IO::Path &vertexFilepath, const Krys::IO::Path &fragmentFilepath) noexcept
        : _handle(glCreateProgram())
    {
      auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
      CreateShader(vertexFilepath, vertexShader);

      auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
      CreateShader(fragmentFilepath, fragmentShader);

      glAttachShader(_handle, vertexShader);
      glAttachShader(_handle, fragmentShader);
      glLinkProgram(_handle);

      int success;
      char infoLog[512];
      glGetProgramiv(_handle, GL_LINK_STATUS, &success);
      if (!success)
      {
        glGetProgramInfoLog(_handle, 512, NULL, infoLog);
        assert(false && "Shader program linking failed. See log for details.");
      }

      glDeleteShader(vertexShader);
      glDeleteShader(fragmentShader);
    }

    ~OpenGLShader() noexcept
    {
      glDeleteProgram(_handle);
    }

    void Bind() const noexcept
    {
      glUseProgram(_handle);
    }

    template <typename T>
    void SetUniform(const Krys::string &uniformName, const T &value, bool required = true) noexcept
    {
      using namespace Krys::Maths;

      GLint location = glGetUniformLocation(_handle, uniformName.c_str());
      if (location == -1)
      {
        assert(!required && "Uniform not found in shader.");
        // Uniform not found, possibly optimized out by the compiler
        return;
      }

      if constexpr (std::is_same_v<T, bool>)
        glProgramUniform1i(_handle, location, value);
      else if constexpr (std::is_same_v<T, int32>)
        glProgramUniform1i(_handle, location, value);
      else if constexpr (std::is_same_v<T, List<int32>>)
        glProgramUniform1iv(_handle, location, value.size(), value.data());
      else if constexpr (std::is_same_v<T, uint32>)
        glProgramUniform1ui(_handle, location, value);
      else if constexpr (std::is_same_v<T, List<uint32>>)
        glProgramUniform1uiv(_handle, location, value.size(), value.data());
      else if constexpr (std::is_same_v<T, float32>)
        glProgramUniform1f(_handle, location, value);
      else if constexpr (std::is_same_v<T, List<float32>>)
        glProgramUniform1fv(_handle, location, value.size(), value.data());
      else if constexpr (std::is_same_v<T, Vec2>)
        glProgramUniform2f(_handle, location, value.x, value.y);
      else if constexpr (std::is_same_v<T, List<Vec2>>)
        glProgramUniform2fv(_handle, location, value.size(), &value[0].x);
      else if constexpr (std::is_same_v<T, Vec3>)
        glProgramUniform3f(_handle, location, value.x, value.y, value.z);
      else if constexpr (std::is_same_v<T, List<Vec3>>)
        glProgramUniform3fv(_handle, location, value.size(), &value[0].x);
      else if constexpr (std::is_same_v<T, Vec4>)
        glProgramUniform4f(_handle, location, value.x, value.y, value.z, value.w);
      else if constexpr (std::is_same_v<T, List<Vec4>>)
        glProgramUniform4fv(_handle, location, value.size(), &value[0].x);
      else if constexpr (std::is_same_v<T, Mat2>)
        glProgramUniformMatrix2fv(_handle, location, 1, GL_FALSE, &value[0].x);
      else if constexpr (std::is_same_v<T, List<Mat2>>)
        glProgramUniformMatrix2fv(_handle, location, value.size(), GL_FALSE, &value[0][0].x);
      else if constexpr (std::is_same_v<T, Mat3>)
        glProgramUniformMatrix3fv(_handle, location, 1, GL_FALSE, &value[0].x);
      else if constexpr (std::is_same_v<T, List<Mat3>>)
        glProgramUniformMatrix3fv(_handle, location, value.size(), GL_FALSE, &value[0][0].x);
      else if constexpr (std::is_same_v<T, Mat4>)
        glProgramUniformMatrix4fv(_handle, location, 1, GL_FALSE, &value[0].x);
      else if constexpr (std::is_same_v<T, List<Mat4>>)
        glProgramUniformMatrix4fv(_handle, location, value.size(), GL_FALSE, &value[0][0].x);
      else
        assert(false && "Unsupported uniform type.");
    }

    NO_DISCARD GLuint GetNativeHandle() const noexcept
    {
      return _handle;
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

  private:
    GLuint _handle;
  };
}