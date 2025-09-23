#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Shader.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx::OpenGL
{
  struct Character
  {
    GLuint Texture {0u};     // ID handle of the glyph texture
    Maths::Vec2u Size {};    // Size of glyph
    Maths::Vec2i Bearing {}; // Offset from baseline to left/top of glyph
    uint32 Advance {0u};     // Offset to advance to next glyph
  };

  class Font
  {
    NO_COPY(Font)

    Map<char, Character> _characters;
    GLuint VAO, VBO;

  public:
    Font(const Map<char, Character> &characters) noexcept : _characters(characters)
    {
      glCreateVertexArrays(1, &VAO);
      glCreateBuffers(1, &VBO);

      glBindVertexArray(VAO);
      glBindBuffer(GL_ARRAY_BUFFER, VBO);

      glNamedBufferData(VBO, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    }

    ~Font()
    {
      for (const auto &[_, character] : _characters)
      {
        glDeleteTextures(1, &character.Texture);
      }
    }

    Font(Font &&other) noexcept
    {
      std::swap(other.VAO, VAO);
      std::swap(other.VBO, VBO);
      _characters = std::move(other._characters);
      other._characters.clear();
    }

    Font &operator=(Font &&other) noexcept
    {
      if (this != &other)
      {
        std::swap(other.VAO, VAO);
        std::swap(other.VBO, VBO);
        _characters = std::move(other._characters);
        other._characters.clear();
      }
      return *this;
    }

    void DrawText(Shader &s, string text, float x, float y, float scale, Maths::Vec3 color)
    {
      // activate corresponding render state
      s.SetUniform("textColor", color);
      glBindVertexArray(VAO);

      for (const char c : text)
      {
        Character ch = _characters[c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        if (c != ' ')
        {
          // update VBO for each character
          float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f}, {xpos, ypos, 0.0f, 1.0f},     {xpos + w, ypos, 1.0f, 1.0f},

            {xpos, ypos + h, 0.0f, 0.0f}, {xpos + w, ypos, 1.0f, 1.0f}, {xpos + w, ypos + h, 1.0f, 0.0f}};

          glBindTextureUnit(0, ch.Texture);

          // update content of VBO memory
          glNamedBufferSubData(VBO, 0, sizeof(vertices), vertices);

          // render glyph texture over quad
          glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
      }
    }
  };
}