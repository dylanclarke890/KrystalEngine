#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.UI/Element.hpp"

namespace Krys::UI
{
  struct Head
  {
    NO_COPY_MOVE(Head)

  private:
    Map<string, uint32> Fonts;

  public:
    Head() noexcept = default;

    ~Head() noexcept = default;
  };

  struct DocumentSettings
  {
    BoundingBox Viewport {0.f, 0.f, 0.f, 0.f};
    Gfx::FontHandle DefaultFont {0u};
  };

  class Document
  {
    NO_COPY_MOVE(Document)

  private:
    Head _head {};
    Body _body;

    BoundingBox _viewport {};
    Gfx::FontHandle _defaultFont {};

  public:
    Document(const DocumentSettings &settings) noexcept
        : _head(), _body(), _viewport(settings.Viewport), _defaultFont(settings.DefaultFont)
    {
    }

    ~Document() noexcept = default;

    Body &GetBody() noexcept
    {
      return _body;
    }
  };
}