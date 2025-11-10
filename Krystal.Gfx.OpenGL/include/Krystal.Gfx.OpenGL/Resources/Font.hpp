#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Shader.hpp"
#include "Krystal.Gfx.OpenGL/Utils.hpp"
#include "Krystal.Gfx/Resources/Font.hpp"
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

  class Font
  {
    NO_COPY(Font)

    constexpr static size_t MaxGlyphsPerDrawCall = 4'096;
    constexpr static size_t VerticesPerGlyph = 6; // 2 triangles per glyph

    FontType _type {FontType::Bitmap};
    FontFamilyHandle _fontFamily;
    TextureHandle _texture;
    float _ptSize {0.f};
    GLuint _vao {};
    GLuint _vbo {};
    List<TextVertex> _vertexBuffer {};
    SDFParams _sdfParams {};
    CharacterMap _characters;
    Maths::Vec2u _atlasSize {0u};
    FontMetrics _metrics {};

    Font(FontType type, FontFamilyHandle fontFamily, TextureHandle texture,
         const FontAtlasData &data) noexcept
        : _type(type), _fontFamily(fontFamily), _texture(texture), _metrics(data.Metrics)
    {
      glCreateVertexArrays(1, &_vao);
      glCreateBuffers(1, &_vbo);

      glBindVertexArray(_vao);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo);
      Utils::ApplyVertexBufferLayout(TextVertex::Layout());

      auto bufferSize = sizeof(TextVertex) * VerticesPerGlyph * MaxGlyphsPerDrawCall;
      glNamedBufferStorage(_vbo, bufferSize, 0, GL_DYNAMIC_STORAGE_BIT);
      _vertexBuffer.reserve(VerticesPerGlyph * MaxGlyphsPerDrawCall);
      _atlasSize = data.Size;
      _characters = data.Characters;
    }

  public:
    ~Font() noexcept
    {
      if (_vbo != 0u)
      {
        glDeleteBuffers(1, &_vbo);
      }

      if (_vao != 0u)
      {
        glDeleteVertexArrays(1, &_vao);
      }
    }

    MOVE_SWAP(Font)

    static Font BitmapAtlas(FontFamilyHandle fontFamily, TextureHandle texture, const FontAtlasData &data,
                            float ptSize) noexcept
    {
      auto font = Font(FontType::Bitmap, fontFamily, texture, data);
      font._ptSize = ptSize;
      return font;
    }

    static Font SDFAtlas(FontFamilyHandle fontFamily, TextureHandle texture, const FontAtlasData &data,
                         const SDFParams &sdfParams = SDFParams::Defaults()) noexcept
    {
      auto font = Font(FontType::SDF, fontFamily, texture, data);
      font._sdfParams = sdfParams;
      return font;
    }

    static Font MSDFAtlas(FontFamilyHandle fontFamily, TextureHandle texture, const FontAtlasData &data,
                          const SDFParams &sdfParams = SDFParams::Defaults()) noexcept
    {
      auto font = Font(FontType::MSDF, fontFamily, texture, data);
      font._sdfParams = sdfParams;
      return font;
    }

    static Font MTSDFAtlas(FontFamilyHandle fontFamily, TextureHandle texture, const FontAtlasData &data,
                           const SDFParams &sdfParams = SDFParams::Defaults()) noexcept
    {
      auto font = Font(FontType::MTSDF, fontFamily, texture, data);
      font._sdfParams = sdfParams;
      return font;
    }

    void DrawText(const string &text, const Maths::Vec2 &position, float scale = 1.0f)
    {
      glBindVertexArray(_vao);

      if (_type != FontType::Bitmap)
      {
        scale = (scale * _ptSize) / _sdfParams.EMSizeInPixels;
      }

      glEnable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

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
          const Character &ch = _characters[c];
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

      glDisable(GL_BLEND);
    }

    FontType Type() const noexcept
    {
      return _type;
    }

    float PtSize() const noexcept
    {
      return _ptSize;
    }

    FontFamilyHandle Family() const noexcept
    {
      return _fontFamily;
    }

    TextureHandle AtlasTexture() const noexcept
    {
      return _texture;
    }

    const SDFParams &SDFParams() const noexcept
    {
      return _sdfParams;
    }

    const Maths::Vec2u &AtlasSize() const noexcept
    {
      return _atlasSize;
    }

    const CharacterMap &Characters() const noexcept
    {
      return _characters;
    }

    const FontMetrics &Metrics() const noexcept
    {
      return _metrics;
    }

  private:
    void Swap(Font &other) noexcept
    {
      std::swap(other._type, _type);
      std::swap(other._ptSize, _ptSize);
      std::swap(other._fontFamily, _fontFamily);
      std::swap(other._texture, _texture);
      std::swap(other._vao, _vao);
      std::swap(other._vbo, _vbo);
      std::swap(other._vertexBuffer, _vertexBuffer);
      std::swap(other._sdfParams, _sdfParams);
      std::swap(other._characters, _characters);
      std::swap(other._atlasSize, _atlasSize);
      std::swap(other._metrics, _metrics);
    }
  };
}