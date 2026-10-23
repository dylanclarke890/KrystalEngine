#include "Krystal.UI/PopupMenuOverlay.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/PopupMenuClient.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.UI/View.hpp"
#include <algorithm>
#include <cmath>
#include <format>

namespace krys::ui
{
  namespace
  {
    /// @brief The tallest a popup gets, in items, before it scrolls.
    constexpr int32 MaximumVisibleItems = 20;

    void AppendEscaped(utf8_string &html, utf8_stringview text) noexcept
    {
      for (auto c : text)
      {
        switch (c)
        {
          case u8'&': html += u8"&amp;"; break;
          case u8'<': html += u8"&lt;"; break;
          case u8'>': html += u8"&gt;"; break;
          case u8'"': html += u8"&quot;"; break;
          default:    html += c; break;
        }
      }
    }

    KRYS_NODISCARD utf8_string ToCSS(const boo::Color &color) noexcept
    {
      auto components = color.ToResolvedColorComponentsInColorSpace(ColourSpace::SRGB).Components;
      auto channel = [](float value)
      {
        return static_cast<int32>(std::lround(std::clamp(value, 0.f, 1.f) * 255.f));
      };
      auto text = std::format("rgb({} {} {} / {})", channel(components[0]), channel(components[1]),
                              channel(components[2]), std::clamp(components[3], 0.f, 1.f));
      return utf8_string(text.begin(), text.end());
    }

    /// @brief The font of @p style, as a CSS `font` value.
    KRYS_NODISCARD utf8_string ToCSSFont(const boo::PopupMenuStyle &style) noexcept
    {
      auto &description = style.Font().GetFontDescription();
      utf8_string families;
      for (auto &family : description.Families())
      {
        auto name = family.View();
        // Generic families have an internal name.
        if (name.starts_with(u8"-webkit-"))
        {
          name.remove_prefix(8);
          families += name;
        }
        else
        {
          families += u8"\"";
          AppendEscaped(families, name);
          families += u8"\"";
        }
        families += u8", ";
      }
      families += u8"sans-serif";
      auto size = std::format("{}px ", description.ComputedSize());
      return utf8_string(size.begin(), size.end()) + families;
    }
  }

  PopupMenuOverlay::PopupMenuOverlay(View &owner, boo::PopupMenuClient &client) noexcept
      : _owner(&owner), _client(&client)
  {
  }

  PopupMenuOverlay::~PopupMenuOverlay() noexcept = default;

  void PopupMenuOverlay::Show(const boo::IntRect &rect, boo::LocalFrameView &view,
                              int32 selectedIndex) noexcept
  {
    if (!_client || !_owner)
    {
      return;
    }

    // The control's rect is in the page's coordinates; the popup is placed in the viewport's.
    _controlRect = rect;
    auto scrollPosition = view.GetScrollPosition();
    _controlRect.Move(-scrollPosition.X(), -scrollPosition.Y());
    _highlightedIndex = selectedIndex;
    Build();

    _visible = true;
    _owner->ShowPopup(*this);
  }

  void PopupMenuOverlay::Hide() noexcept
  {
    if (!_visible)
    {
      return;
    }

    // The owner lets go of the popup, which its client still holds while it's told.
    Ref<PopupMenuOverlay> protectedThis = ShareRef(*this);
    _visible = false;
    if (_owner)
    {
      _owner->PopupHidden(*this);
    }
    if (_client)
    {
      _client->PopupDidHide();
    }
  }

  void PopupMenuOverlay::UpdateFromElement() noexcept
  {
    if (_visible)
    {
      Build();
    }
  }

  void PopupMenuOverlay::DisconnectClient() noexcept
  {
    _client = nullptr;
    Hide();
  }

  void PopupMenuOverlay::Build() noexcept
  {
    krys_debug_assert(_client && _owner);

    auto menuStyle = _client->MenuStyle();
    auto paddingLeft = _client->ClientPaddingLeft().ToInt();
    auto paddingRight = _client->ClientPaddingRight().ToInt();

    utf8_string html = u8"<!DOCTYPE html><style>"
                       u8"html { width: max-content; min-width: 100%; box-sizing: border-box; margin: 0;"
                       u8" border: 1px solid GrayText; cursor: default; background: ";
    html += ToCSS(menuStyle.BackgroundColor());
    html += u8"; color: ";
    html += ToCSS(menuStyle.ForegroundColor());
    html += u8"; font: ";
    html += ToCSSFont(menuStyle);
    html += u8"; }\nbody { margin: 0; padding: 2px 0; }\n";
    html +=
      std::format(".item {{ padding: 1px {}px 1px {}px; white-space: pre; }}\n", paddingRight, paddingLeft)
      | std::ranges::to<utf8_string>();
    html += u8".label { font-weight: bold; }\n"
            u8".disabled { color: GrayText; }\n"
            u8".highlighted { background: Highlight; color: HighlightText; }\n"
            u8"hr { margin: 2px 0; border: none; border-top: 1px solid GrayText; }\n"
            u8"</style><body>";

    auto listSize = std::max(0, _client->ListSize());
    for (int32 i = 0; i < listSize; ++i)
    {
      auto index = static_cast<uint32>(i);
      if (_client->ItemIsSeparator(index))
      {
        html += std::format("<hr data-index={}>", i) | std::ranges::to<utf8_string>();
        continue;
      }

      auto itemStyle = _client->ItemStyle(index);
      if (itemStyle.IsDisplayNone())
      {
        continue;
      }

      html += std::format("<div data-index={} class=\"item", i) | std::ranges::to<utf8_string>();
      if (_client->ItemIsLabel(index))
      {
        html += u8" label";
      }
      else if (!_client->ItemIsEnabled(index))
      {
        html += u8" disabled";
      }
      html += u8"\" style=\"color: ";
      html += ToCSS(itemStyle.ForegroundColor());
      if (itemStyle.GetBackgroundColorType()
          == boo::PopupMenuStyle::BackgroundColorType::CustomBackgroundColor)
      {
        html += u8"; background: ";
        html += ToCSS(itemStyle.BackgroundColor());
      }
      if (!itemStyle.IsVisible())
      {
        html += u8"; visibility: hidden";
      }
      if (itemStyle.GetTextDirection() == boo::TextDirection::RTL)
      {
        html += u8"; direction: rtl";
      }
      html += u8"\"";
      auto toolTip = _client->ItemToolTip(index);
      if (!toolTip.empty())
      {
        html += u8" title=\"";
        AppendEscaped(html, toolTip);
        html += u8"\"";
      }
      html += u8">";
      auto text = _client->ItemText(index);
      AppendEscaped(html, text.empty() ? utf8_stringview(u8" ") : utf8_stringview(text));
      html += u8"</div>";
    }

    // The items are laid out at the control's width, as wide as they need to be, and as tall as they are.
    auto viewport = _owner->ViewportSize();
    auto width = std::max(_controlRect.Width(), 1);
    if (!_view)
    {
      _view = CreateUnique<View>(boo::IntSize(width, viewport.Height()));
    }
    else
    {
      _view->Resize(boo::IntSize(width, viewport.Height()));
    }
    _view->LoadHTML(krys::move(html));
    (void)_view->Document().UpdateLayout();

    _itemElements.assign(static_cast<size_t>(listSize), nullptr);
    for (boo::dom::Node *node = _view->Document().DocumentElement().get(); node;)
    {
      if (auto *element = DynamicDowncast<boo::dom::Element>(*node))
      {
        if (auto index = element->GetAttribute(u8"data-index"))
        {
          auto value = std::stoi(std::string(index->begin(), index->end()));
          if (value >= 0 && value < listSize)
          {
            _itemElements[static_cast<size_t>(value)] = ShareRefPtr(element);
          }
        }
      }
      node = boo::dom::TreeQueries::Next(*node);
    }

    boo::IntSize contentSize(width, 0);
    int32 itemHeight = 0;
    if (auto *root = _view->Document().DocumentElement().get())
    {
      if (auto *box = DynamicDowncast<boo::RenderBox>(root->Renderer()))
      {
        contentSize = boo::IntSize(box->Width().ToInt(), box->Height().ToInt());
      }
    }
    for (auto &element : _itemElements)
    {
      if (auto *box = element ? DynamicDowncast<boo::RenderBox>(element->Renderer()) : nullptr)
      {
        itemHeight = std::max(itemHeight, box->Height().ToInt());
      }
    }

    // Below the control if it fits, else wherever there's more room, scrolling if it doesn't fit there.
    auto height = contentSize.Height();
    if (itemHeight)
    {
      height = std::min(height, itemHeight * MaximumVisibleItems);
    }
    auto spaceBelow = std::max(0, viewport.Height() - _controlRect.MaxY());
    auto spaceAbove = std::max(0, _controlRect.Y());
    int32 y = _controlRect.MaxY();
    if (height > spaceBelow && spaceAbove > spaceBelow)
    {
      height = std::min(height, spaceAbove);
      y = _controlRect.Y() - height;
    }
    else
    {
      height = std::min(height, std::max(spaceBelow, 1));
    }
    width = std::min(std::max(contentSize.Width(), width), std::max(viewport.Width(), 1));
    auto x = std::clamp(_controlRect.X(), 0, std::max(0, viewport.Width() - width));
    _rect = boo::IntRect(x, y, width, std::max(height, 1));
    _view->Resize(_rect.Size());

    auto highlighted = _highlightedIndex;
    _highlightedIndex = -1;
    Highlight(highlighted);
  }

  int32 PopupMenuOverlay::ItemAt(const boo::IntPoint &point) const noexcept
  {
    if (!_view)
    {
      return -1;
    }

    for (auto *element = _view->HitTest(point); element; element = element->ParentElement())
    {
      for (size_t i = 0; i < _itemElements.size(); ++i)
      {
        if (_itemElements[i].get() == element)
        {
          return static_cast<int32>(i);
        }
      }
    }
    return -1;
  }

  void PopupMenuOverlay::MouseMoved(const boo::IntPoint &point) noexcept
  {
    auto index = ItemAt(point);
    if (IsSelectable(index))
    {
      Highlight(index);
    }
  }

  void PopupMenuOverlay::MouseReleased(const boo::IntPoint &point) noexcept
  {
    auto index = ItemAt(point);
    if (IsSelectable(index))
    {
      Choose(index);
    }
  }

  bool PopupMenuOverlay::MouseWheel(const boo::IntPoint &point, float notchesX, float notchesY) noexcept
  {
    return _view && _view->MouseWheel(point, notchesX, notchesY);
  }

  bool PopupMenuOverlay::KeyPressed(pal::Key key, pal::KeyModifiers) noexcept
  {
    auto listSize = static_cast<int32>(_itemElements.size());
    switch (key)
    {
      case pal::Key::DownArrow:
      case pal::Key::RightArrow:  Highlight(NextSelectableIndex(_highlightedIndex, 1)); return true;
      case pal::Key::UpArrow:
      case pal::Key::LeftArrow:   Highlight(NextSelectableIndex(_highlightedIndex, -1)); return true;
      case pal::Key::Home:        Highlight(NextSelectableIndex(-1, 1)); return true;
      case pal::Key::End:         Highlight(NextSelectableIndex(listSize, -1)); return true;
      case pal::Key::Enter:
      case pal::Key::KeypadEnter:
      case pal::Key::Space:       Choose(_highlightedIndex); return true;
      case pal::Key::Escape:
      case pal::Key::Tab:         Hide(); return true;
      default:                    return true;
    }
  }

  void PopupMenuOverlay::Highlight(int32 index) noexcept
  {
    if (index == _highlightedIndex)
    {
      return;
    }

    auto setHighlighted = [this](int32 i, bool highlighted)
    {
      if (i < 0 || static_cast<size_t>(i) >= _itemElements.size() || !_itemElements[static_cast<size_t>(i)])
      {
        return;
      }
      auto &element = *_itemElements[static_cast<size_t>(i)];
      auto classes = element.GetAttribute(u8"class").value_or(boo::dom::DOMString());
      constexpr utf8_stringview suffix = u8" highlighted";
      if (highlighted && !classes.ends_with(suffix))
      {
        classes += suffix;
      }
      else if (!highlighted && classes.ends_with(suffix))
      {
        classes.resize(classes.size() - suffix.size());
      }
      (void)element.SetAttribute(boo::dom::DOMStringAtom(u8"class"), krys::move(classes));
    };

    setHighlighted(_highlightedIndex, false);
    _highlightedIndex = index;
    setHighlighted(_highlightedIndex, true);

    // Keep the highlighted item in view.
    if (index < 0 || static_cast<size_t>(index) >= _itemElements.size()
        || !_itemElements[static_cast<size_t>(index)])
    {
      return;
    }
    (void)_view->Document().UpdateLayout();
    auto *box = DynamicDowncast<boo::RenderBox>(_itemElements[static_cast<size_t>(index)]->Renderer());
    if (!box)
    {
      return;
    }
    auto itemRect = box->AbsoluteBoundingBoxRect();
    auto scroll = _view->ScrollPosition();
    auto visibleHeight = _rect.Height();
    if (itemRect.Y() < scroll.Y())
    {
      _view->ScrollTo(boo::IntPoint(scroll.X(), itemRect.Y()));
    }
    else if (itemRect.MaxY() > scroll.Y() + visibleHeight)
    {
      _view->ScrollTo(boo::IntPoint(scroll.X(), itemRect.MaxY() - visibleHeight));
    }
  }

  void PopupMenuOverlay::Choose(int32 index) noexcept
  {
    if (!IsSelectable(index))
    {
      return;
    }

    // The popup hides before its client hears of the choice, as a platform's menu does.
    Ref<PopupMenuOverlay> protectedThis = ShareRef(*this);
    Hide();
    if (_client)
    {
      _client->ValueChanged(static_cast<uint32>(index));
    }
  }

  bool PopupMenuOverlay::IsSelectable(int32 index) const noexcept
  {
    if (!_client || index < 0 || static_cast<size_t>(index) >= _itemElements.size()
        || !_itemElements[static_cast<size_t>(index)])
    {
      return false;
    }
    auto i = static_cast<uint32>(index);
    return _client->ItemIsEnabled(i) && !_client->ItemIsLabel(i) && !_client->ItemIsSeparator(i);
  }

  int32 PopupMenuOverlay::NextSelectableIndex(int32 index, int32 direction) const noexcept
  {
    auto listSize = static_cast<int32>(_itemElements.size());
    for (auto i = index + direction; i >= 0 && i < listSize; i += direction)
    {
      if (IsSelectable(i))
      {
        return i;
      }
    }
    return IsSelectable(index) ? index : _highlightedIndex;
  }
}
