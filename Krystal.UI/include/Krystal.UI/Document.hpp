#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/SmartPointers.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.UI/Element.hpp"
#include "Krystal.UI/ElementPool.hpp"
#include "Krystal.UI/LayoutEngine.hpp"

namespace Krys::UI
{
  struct DocumentSettings
  {
    BoundingBox Viewport {0.f, 0.f, 0.f, 0.f};
    Gfx::FontHandle DefaultFont {0u};
  };

  class Document
  {
    NO_COPY_MOVE(Document)

  private:
    Body _body;
    ElementPool _elementPool;
    LayoutEngine _layoutEngine;

    BoundingBox _viewport {};
    Gfx::FontHandle _defaultFont {};

  public:
    Document(const DocumentSettings &settings) noexcept
        : _body(), _elementPool(), _layoutEngine(_elementPool), _viewport(settings.Viewport),
          _defaultFont(settings.DefaultFont)
    {
    }

    ~Document() noexcept = default;

    template <DerivedFrom<Element> TElement, typename... TArgs>
    TElement &CreateElement(TArgs &&...args)
    {
      auto handle = _elementPool.Create<TElement>(std::forward<TArgs>(args)...);
      return _elementPool.Get<TElement>(handle);
    }

    Body &GetBody() noexcept
    {
      return _body;
    }

    LayoutEngine &GetLayoutEngine() noexcept
    {
      return _layoutEngine;
    }
  };
}