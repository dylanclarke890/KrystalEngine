#include "Krystal.UI/ColorChooserOverlay.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Platform/ColorChooserClient.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.UI/View.hpp"
#include <algorithm>
#include <charconv>
#include <format>

namespace krys::ui
{
  namespace
  {
    // Greys, then eight hues from dark to light.
    constexpr std::array<uint32, 40> Palette {
      0x000000, 0x404040, 0x606060, 0x808080, 0xA0A0A0, 0xC0C0C0, 0xE0E0E0, 0xFFFFFF, 0x800000, 0x804000,
      0x808000, 0x008000, 0x008080, 0x000080, 0x400080, 0x800040, 0xFF0000, 0xFF8000, 0xFFFF00, 0x00FF00,
      0x00FFFF, 0x0000FF, 0x8000FF, 0xFF0080, 0xFF8080, 0xFFC080, 0xFFFF80, 0x80FF80, 0x80FFFF, 0x8080FF,
      0xC080FF, 0xFF80C0, 0xFFC0C0, 0xFFE0C0, 0xFFFFC0, 0xC0FFC0, 0xC0FFFF, 0xC0C0FF, 0xE0C0FF, 0xFFC0E0,
    };

    KRYS_NODISCARD boo::Color ColorFromHex(uint32 hex) noexcept
    {
      return SRGBA<uint8> {static_cast<uint8>(hex >> 16), static_cast<uint8>((hex >> 8) & 0xFF),
                           static_cast<uint8>(hex & 0xFF)};
    }

    /// @brief Whether @p a and @p b are the same colour, as colour inputs see them.
    KRYS_NODISCARD bool SameColor(const boo::Color &a, const boo::Color &b) noexcept
    {
      return boo::SerialisationForHTML(a) == boo::SerialisationForHTML(b);
    }

    KRYS_NODISCARD utf8_string Number(int32 value) noexcept
    {
      auto text = std::format("{}", value);
      return utf8_string(text.begin(), text.end());
    }
  }

  ColorChooserOverlay::ColorChooserOverlay(View &owner, boo::ColorChooserClient &client,
                                           const boo::Color &initialColor) noexcept
      : _owner(&owner), _client(&client), _selectedColor(initialColor)
  {
    Show();
  }

  ColorChooserOverlay::~ColorChooserOverlay() noexcept = default;

  void ColorChooserOverlay::ReattachColorChooser(const boo::Color &color) noexcept
  {
    _selectedColor = color;
    Show();
  }

  void ColorChooserOverlay::SetSelectedColor(const boo::Color &color) noexcept
  {
    _selectedColor = color;
    if (_visible)
    {
      Build();
    }
  }

  void ColorChooserOverlay::EndChooser() noexcept
  {
    // The owner and the client let go of the picker as they're told.
    Ref<ColorChooserOverlay> protectedThis = ShareRef(*this);
    if (std::exchange(_visible, false) && _owner)
    {
      HiddenFrom(*_owner, *this);
    }
    if (auto *client = std::exchange(_client, nullptr))
    {
      client->DidEndChooser();
    }
  }

  void ColorChooserOverlay::Hide() noexcept
  {
    EndChooser();
  }

  void ColorChooserOverlay::Show() noexcept
  {
    if (!_client || !_owner)
    {
      return;
    }

    Build();
    if (!_visible)
    {
      _visible = true;
      ShowIn(*_owner, *this);
    }
  }

  void ColorChooserOverlay::Build() noexcept
  {
    krys_debug_assert(_client && _owner);

    // The input's suggestions first, then the palette.
    _colors = _client->SuggestedColors();
    for (auto hex : Palette)
    {
      _colors.push_back(ColorFromHex(hex));
    }

    // A right-to-left input's picker is mirrored: the swatches run from the right.
    _rightToLeft = _client->IsElementRTL();
    utf8_string html = _rightToLeft ? u8"<!DOCTYPE html><html dir=rtl><style>" : u8"<!DOCTYPE html><style>";
    html += u8"html { width: max-content; margin: 0; border: 1px solid GrayText; cursor: default;"
            u8" background: Canvas; color: CanvasText; font: 13px sans-serif; }\n"
            u8"body { margin: 0; padding: 4px; display: grid; gap: 2px;"
            u8" grid-template-columns: repeat("
            + Number(Columns)
            + u8", 18px); }\n"
              u8".swatch { height: 18px; box-sizing: border-box; border: 1px solid GrayText; }\n"
              u8".selected { outline: 2px solid CanvasText; outline-offset: -2px; }\n"
              u8".highlighted { border: 2px solid Highlight; }\n"
              u8"</style><body>";
    _highlightedIndex = -1;
    for (size_t i = 0; i < _colors.size(); ++i)
    {
      auto selected = SameColor(_colors[i], _selectedColor);
      if (selected && _highlightedIndex < 0)
      {
        _highlightedIndex = static_cast<int32>(i);
      }
      html += u8"<div data-index=" + Number(static_cast<int32>(i)) + u8" class=\"swatch"
              + (selected ? utf8_string(u8" selected") : utf8_string()) + u8"\" title=\"";
      auto serialised = boo::SerialisationForHTML(_colors[i]);
      AppendEscaped(html, serialised);
      html += u8"\" style=\"background: ";
      AppendEscaped(html, serialised);
      html += u8"\"></div>";
    }

    auto viewport = _owner->ViewportSize();
    if (!_view)
    {
      _view = CreateUnique<View>(viewport);
    }
    else
    {
      _view->Resize(viewport);
    }
    ReplaceDocument(*_view, krys::move(html));
    (void)_view->Document().UpdateLayout();

    _swatches.assign(_colors.size(), nullptr);
    for (boo::dom::Node *node = _view->Document().DocumentElement().get(); node;
         node = boo::dom::TreeQueries::Next(*node))
    {
      if (auto *element = DynamicDowncast<boo::dom::Element>(*node))
      {
        if (auto attribute = element->GetAttribute(u8"data-index"))
        {
          size_t index = 0;
          auto *begin = reinterpret_cast<const char *>(attribute->data());
          auto [end, error] = std::from_chars(begin, begin + attribute->size(), index);
          if (error == std::errc() && index < _swatches.size())
          {
            _swatches[index] = ShareRefPtr(element);
          }
        }
      }
    }

    boo::IntSize contentSize(1, 1);
    if (auto *root = _view->Document().DocumentElement().get())
    {
      if (auto *box = DynamicDowncast<boo::RenderBox>(root->Renderer()))
      {
        contentSize = boo::IntSize(box->Width().ToInt(), box->Height().ToInt());
      }
    }
    _rect = Place(_client->ElementRectRelativeToRootView(), contentSize, viewport, _rightToLeft);
    _view->Resize(_rect.Size());

    auto highlighted = std::exchange(_highlightedIndex, -1);
    Highlight(highlighted < 0 ? 0 : highlighted);
  }

  void ColorChooserOverlay::Highlight(int32 index) noexcept
  {
    if (index < 0 || static_cast<size_t>(index) >= _swatches.size() || index == _highlightedIndex)
    {
      return;
    }
    if (_highlightedIndex >= 0 && _swatches[static_cast<size_t>(_highlightedIndex)])
    {
      SetClass(*_swatches[static_cast<size_t>(_highlightedIndex)], u8"highlighted", false);
    }
    _highlightedIndex = index;
    if (_swatches[static_cast<size_t>(index)])
    {
      SetClass(*_swatches[static_cast<size_t>(index)], u8"highlighted", true);
    }
  }

  void ColorChooserOverlay::Choose(int32 index) noexcept
  {
    if (index < 0 || static_cast<size_t>(index) >= _colors.size())
    {
      return;
    }

    // The picker hides before its client hears of the choice, as a popup does.
    Ref<ColorChooserOverlay> protectedThis = ShareRef(*this);
    auto color = _colors[static_cast<size_t>(index)];
    auto *client = _client;
    EndChooser();
    if (client)
    {
      client->DidChooseColor(color);
    }
  }

  void ColorChooserOverlay::MouseMoved(const boo::IntPoint &point) noexcept
  {
    if (!_view)
    {
      return;
    }
    if (auto *swatch = ElementWithAttributeAt(*_view, point, u8"data-index"))
    {
      for (size_t i = 0; i < _swatches.size(); ++i)
      {
        if (_swatches[i].get() == swatch)
        {
          Highlight(static_cast<int32>(i));
          return;
        }
      }
    }
  }

  void ColorChooserOverlay::MouseReleased(const boo::IntPoint &point) noexcept
  {
    if (!_view)
    {
      return;
    }
    if (auto *swatch = ElementWithAttributeAt(*_view, point, u8"data-index"))
    {
      for (size_t i = 0; i < _swatches.size(); ++i)
      {
        if (_swatches[i].get() == swatch)
        {
          Choose(static_cast<int32>(i));
          return;
        }
      }
    }
  }

  bool ColorChooserOverlay::MouseWheel(const boo::IntPoint &, float, float) noexcept
  {
    return true;
  }

  bool ColorChooserOverlay::KeyPressed(pal::Key key, pal::KeyModifiers) noexcept
  {
    auto count = static_cast<int32>(_colors.size());
    auto move = [&](int32 delta)
    {
      auto index = _highlightedIndex + delta;
      if (index >= 0 && index < count)
      {
        Highlight(index);
      }
    };
    switch (key)
    {
      case pal::Key::LeftArrow:   move(_rightToLeft ? 1 : -1); return true;
      case pal::Key::RightArrow:  move(_rightToLeft ? -1 : 1); return true;
      case pal::Key::UpArrow:     move(-Columns); return true;
      case pal::Key::DownArrow:   move(Columns); return true;
      case pal::Key::Enter:
      case pal::Key::KeypadEnter:
      case pal::Key::Space:       Choose(_highlightedIndex); return true;
      case pal::Key::Escape:
      case pal::Key::Tab:         EndChooser(); return true;
      default:                    return true;
    }
  }
}
