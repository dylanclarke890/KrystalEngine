#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Shader.hpp"
#include "Krystal.Gfx.OpenGL/Utils.hpp"
#include "Krystal.Gfx/FontType.hpp"
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
  };

  class Font
  {
    NO_COPY(Font)

    constexpr static int MaxGlyphsPerDrawCall = 4'096;
    constexpr static int VerticesPerGlyph = 6; // 2 triangles per glyph

    FontAtlas _bitmapAtlas {};
    FontType _type {FontType::Bitmap};
    GLuint _vao;
    GLuint _vbo;
    List<TextVertex> _vertexBuffer;

  public:
    Font(FontType type, const FontAtlas &bitmapAtlas) noexcept : _type(type), _bitmapAtlas(bitmapAtlas)
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

    ~Font() noexcept
    {
      if (_bitmapAtlas.Texture != 0u)
      {
        glDeleteTextures(1, &_bitmapAtlas.Texture);
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

    Font(Font &&other) noexcept
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
      glBindTextureUnit(0, _bitmapAtlas.Texture);

      Maths::Vec2 pos = position;

      auto count = text.size();
      while (count > 0)
      {
        auto batchSize = Maths::Min(count, static_cast<size_t>(MaxGlyphsPerDrawCall));
        Span<const char> batch(text.data() + (text.size() - count), batchSize);
        count -= batchSize;

        _vertexBuffer.clear();
        for (const char c : batch)
        {
          const Character &ch = _bitmapAtlas.Characters[c];
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

  private:
    void Swap(Font &other) noexcept
    {
      std::swap(other._bitmapAtlas, _bitmapAtlas);
      std::swap(other._type, _type);
      std::swap(other._vao, _vao);
      std::swap(other._vbo, _vbo);
      std::swap(other._vertexBuffer, _vertexBuffer);
    }
  };
}