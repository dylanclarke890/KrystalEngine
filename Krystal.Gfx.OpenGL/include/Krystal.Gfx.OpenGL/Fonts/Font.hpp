#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Shaders/Shader.hpp"
#include "Krystal.Gfx.OpenGL/Utils.hpp"
#include "Krystal.Gfx/Common.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/Span.hpp"
#include "Krystal.Maths/Clamp.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx::OpenGL
{
  struct TextVertex
  {
    Maths::Vec2 Position {};
    Maths::Vec2 UV {};

    static VertexBufferLayout Layout() noexcept
    {
      return {
        {VertexAttributeType::Float, 2}, // Position
        {VertexAttributeType::Float, 2}  // UV
      };
    }
  };

  struct Character
  {
    Maths::Vec2u Size {};    // Size of glyph
    Maths::Vec2i Bearing {}; // Offset from baseline to left/top of glyph
    uint32 Advance {0u};     // Offset to advance to next glyph

    Maths::Vec2 UVMin; // (u0, v0)
    Maths::Vec2 UVMax; // (u1, v1)
  };

  struct FontAtlas
  {
    GLuint Texture {0u};
    Map<char, Character> Characters;
    Maths::Vec2u AtlasSize {0u};
    GLenum Format;
  };

  struct SDFParams
  {
    float EMSizeInPixels {64.0f};
    float PixelRange {4.0f};
    float MiterLimit {1.0f};
  };

  class Font
  {
    NO_COPY(Font)

    constexpr static int MaxGlyphsPerDrawCall = 4'096;
    constexpr static int VerticesPerGlyph = 6; // 2 triangles per glyph

    FontType _type {FontType::Bitmap};
    float _ptSize {};
    IO::Path _path;
    FontAtlas _atlas {};
    GLuint _vao {};
    GLuint _vbo {};
    List<TextVertex> _vertexBuffer {};
    SDFParams _sdfParams {};

  public:
    Font(FontType type, float ptSize, const IO::Path &path, GLenum format) noexcept
        : _type(type), _ptSize(ptSize), _path(path)
    {
      _atlas.Format = format;
      CreateVertexArray();
    }

    Font(FontType type, float ptSize, const IO::Path &path, const FontAtlas &bitmapAtlas,
         const SDFParams &sdfParams) noexcept
        : _type(type), _ptSize(ptSize), _path(path), _atlas(bitmapAtlas), _sdfParams(sdfParams)
    {
      CreateVertexArray();
    }

    ~Font() noexcept
    {
      if (_atlas.Texture != 0u)
      {
        glDeleteTextures(1, &_atlas.Texture);
      }
      if (_vbo != 0u)
      {
        glDeleteBuffers(1, &_vbo);
      }
      if (_vao != 0u)
      {
        glDeleteVertexArrays(1, &_vao);
      }
    }

    Font(Font &&other) noexcept : _path("")
    {
      Swap(other);
    }

    Font &operator=(Font &&other) noexcept
    {
      if (this != &other)
      {
        Swap(other);
      }
      return *this;
    }

    void DrawText(const string &text, const Maths::Vec2 &position, float scale = 1.0f)
    {
      glBindVertexArray(_vao);
      glBindTextureUnit(0, _atlas.Texture);

      Maths::Vec2 pos = position;

      if (_type != FontType::Bitmap)
      {
        scale = (scale * _ptSize) / _sdfParams.EMSizeInPixels;
      }

      auto count = text.size();
      while (count > 0)
      {
        auto batchSize = Maths::Min(count, static_cast<size_t>(MaxGlyphsPerDrawCall));
        Span<const char> batch(text.data() + (text.size() - count), batchSize);
        count -= batchSize;

        _vertexBuffer.clear();
        for (const char c : batch)
        {
          const Character &ch = _atlas.Characters[c];
          float posX = pos.x + ch.Bearing.x * scale;
          float posY = pos.y - (ch.Size.y - ch.Bearing.y) * scale;
          float w = ch.Size.x * scale;
          float h = ch.Size.y * scale;

          _vertexBuffer.push_back({{posX, posY + h}, {ch.UVMin.x, ch.UVMin.y}});
          _vertexBuffer.push_back({{posX, posY}, {ch.UVMin.x, ch.UVMax.y}});
          _vertexBuffer.push_back({{posX + w, posY}, {ch.UVMax.x, ch.UVMax.y}});
          _vertexBuffer.push_back({{posX, posY + h}, {ch.UVMin.x, ch.UVMin.y}});
          _vertexBuffer.push_back({{posX + w, posY}, {ch.UVMax.x, ch.UVMax.y}});
          _vertexBuffer.push_back({{posX + w, posY + h}, {ch.UVMax.x, ch.UVMin.y}});

          pos.x += ch.Advance * scale;
        }

        glNamedBufferSubData(_vbo, 0, static_cast<GLsizeiptr>(sizeof(TextVertex) * _vertexBuffer.size()),
                             _vertexBuffer.data());
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(_vertexBuffer.size()));
      }
    }

    FontType Type() const noexcept
    {
      return _type;
    }

    float PtSize() const noexcept
    {
      return _ptSize;
    }

    const IO::Path &Path() const noexcept
    {
      return _path;
    }

    void SetAtlasData(const List<uint8> &data, const Maths::Vec2u &atlasSize,
                      const Map<char, Character> &characters) noexcept
    {
      if (_atlas.Texture != 0u)
      {
        glDeleteTextures(1, &_atlas.Texture);
      }

      glCreateTextures(GL_TEXTURE_2D, 1, &_atlas.Texture);

      GLenum format = _atlas.Format;
      GLenum internalFormat;
      switch (format)
      {
        case GL_RED:  internalFormat = GL_R8; break;
        case GL_RG:   internalFormat = GL_RG8; break;
        case GL_RGB:  internalFormat = GL_RGB8; break;
        case GL_RGBA: internalFormat = GL_RGBA8; break;
        default:
          assert(false && "Unsupported texture format.");
          internalFormat = GL_R8;
          break;
      }

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTextureStorage2D(_atlas.Texture, 1, internalFormat, atlasSize.x, atlasSize.y);
      glTextureSubImage2D(_atlas.Texture, 0, 0, 0, atlasSize.x, atlasSize.y, format, GL_UNSIGNED_BYTE,
                          data.data());
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

      glTextureParameteri(_atlas.Texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTextureParameteri(_atlas.Texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTextureParameteri(_atlas.Texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTextureParameteri(_atlas.Texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      _atlas.AtlasSize = atlasSize;
      _atlas.Characters = characters;
    }

    const SDFParams &SDFParams() const noexcept
    {
      return _sdfParams;
    }

    const FontAtlas &Atlas() const noexcept
    {
      return _atlas;
    }

  private:
    void Swap(Font &other) noexcept
    {
      std::swap(other._type, _type);
      std::swap(other._ptSize, _ptSize);
      std::swap(other._path, _path);
      std::swap(other._atlas, _atlas);
      std::swap(other._vao, _vao);
      std::swap(other._vbo, _vbo);
      std::swap(other._vertexBuffer, _vertexBuffer);
    }

    void CreateVertexArray() noexcept
    {
      glCreateVertexArrays(1, &_vao);
      glCreateBuffers(1, &_vbo);

      glBindVertexArray(_vao);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo);
      Utils::ApplyVertexBufferLayout(TextVertex::Layout());

      auto bufferSize = sizeof(TextVertex) * VerticesPerGlyph * MaxGlyphsPerDrawCall;
      glNamedBufferStorage(_vbo, bufferSize, 0, GL_DYNAMIC_STORAGE_BIT);
      _vertexBuffer.reserve(VerticesPerGlyph * MaxGlyphsPerDrawCall);
    }
  };
}