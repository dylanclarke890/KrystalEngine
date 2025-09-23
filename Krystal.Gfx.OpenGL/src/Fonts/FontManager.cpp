#pragma once

#include "Krystal.Gfx.OpenGL/Fonts/Font.hpp"
#include "Krystal.Gfx.OpenGL/Fonts/FontSystem.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/DebugBreak.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Log/ILogger.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace
{
  using namespace Krys;
  using namespace Krys::Maths;
  using namespace Krys::Gfx::OpenGL;

  FT_Library FontLibraryHandle = nullptr;
}

namespace Krys::Gfx::OpenGL
{
  FontSystem::FontSystem() noexcept
  {
    if (FT_Init_FreeType(&FontLibraryHandle))
    {
      auto *logger = Log::GetGlobalLogger();
      if (logger)
      {
        logger->Error("FREETYPE: Could not init FreeType Library");
        KRYS_DEBUG_BREAK();
      }
    }
  }

  FontSystem::~FontSystem() noexcept
  {
    if (FontLibraryHandle)
    {
      FT_Done_FreeType(FontLibraryHandle);
      FontLibraryHandle = nullptr;
    }
  }

  FontHandle FontSystem::Load(const IO::Path &path, float size) noexcept
  {
    string key = std::format("{0}-{1}", path.ToString(), size);
    if (auto cached = _cache.Get(key); cached.IsValid())
    {
      return cached;
    }

    auto *logger = Log::GetGlobalLogger();

    FT_Face face;
    if (FT_New_Face(FontLibraryHandle, path.ToString().c_str(), 0, &face))
    {
      logger->Error("FREETYPE: Failed to load font");
      KRYS_DEBUG_BREAK();
      return {};
    }

    FT_Set_Pixel_Sizes(face, 0, (uint32)size);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    GLuint textures[128];
    glCreateTextures(GL_TEXTURE_2D, 128, textures);
    Map<char, Character> characters;
    for (uchar c = 0; c < 128; c++)
    {
      // load character glyph
      if (FT_Load_Char(face, c, FT_LOAD_RENDER))
      {
        logger->Error("FREETYPE: Failed to load Glyph {0}", c);
        continue;
      }

      GLuint texture = textures[c];
      auto &glyph = *face->glyph;

      if (c != ' ')
      {
        glTextureStorage2D(texture, 1, GL_R8, glyph.bitmap.width, glyph.bitmap.rows);
        glTextureSubImage2D(texture, 0, 0, 0, glyph.bitmap.width, glyph.bitmap.rows, GL_RED, GL_UNSIGNED_BYTE,
                            glyph.bitmap.buffer);

        glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      }

      characters[c] = {.Texture = texture,
                       .Size = Vec2u {glyph.bitmap.width, glyph.bitmap.rows},
                       .Bearing = Vec2i {glyph.bitmap_left, glyph.bitmap_top},
                       .Advance = (uint32)glyph.advance.x};
    }

    FT_Done_Face(face);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // reset to default

    Font font(characters);
    return _fonts.Add(std::move(font));
  }

  Font &FontSystem::Get(FontHandle handle)
  {
    return _fonts.Get(handle);
  }
}