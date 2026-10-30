#include "Krystal.UI/Overlay.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.UI/View.hpp"
#include <algorithm>

namespace krys::ui
{
  void Overlay::ReplaceDocument(View &view, utf8_string &&markup) noexcept
  {
    view.ReplaceDocument(krys::move(markup));
  }

  void Overlay::ShowIn(View &owner, Overlay &overlay) noexcept
  {
    owner.ShowOverlay(overlay);
  }

  void Overlay::HiddenFrom(View &owner, Overlay &overlay) noexcept
  {
    owner.OverlayHidden(overlay);
  }

  int32 Overlay::AlignedLeft(const boo::IntRect &controlRect, int32 width, int32 viewportWidth,
                             bool rightToLeft) noexcept
  {
    auto x = rightToLeft ? controlRect.MaxX() - width : controlRect.X();
    return std::clamp(x, 0, std::max(0, viewportWidth - width));
  }

  boo::IntRect Overlay::Place(const boo::IntRect &controlRect, const boo::IntSize &size,
                              const boo::IntSize &viewportSize, bool rightToLeft) noexcept
  {
    auto height = size.Height();
    auto spaceBelow = std::max(0, viewportSize.Height() - controlRect.MaxY());
    auto spaceAbove = std::max(0, controlRect.Y());
    int32 y = controlRect.MaxY();
    if (height > spaceBelow && spaceAbove > spaceBelow)
    {
      height = std::min(height, spaceAbove);
      y = controlRect.Y() - height;
    }
    else
    {
      height = std::min(height, std::max(spaceBelow, 1));
    }
    auto width = std::min(size.Width(), std::max(viewportSize.Width(), 1));
    auto x = AlignedLeft(controlRect, width, viewportSize.Width(), rightToLeft);
    return boo::IntRect(x, y, std::max(width, 1), std::max(height, 1));
  }

  boo::dom::Element *Overlay::ElementWithAttributeAt(View &view, const boo::IntPoint &point,
                                                     utf8_stringview name) noexcept
  {
    auto attribute = boo::dom::DOMStringAtom(name);
    for (auto *element = view.HitTest(point); element; element = element->ParentElement())
    {
      if (element->HasAttribute(attribute))
      {
        return element;
      }
    }
    return nullptr;
  }

  void Overlay::SetClass(boo::dom::Element &element, utf8_stringview className, bool on) noexcept
  {
    auto classes = element.GetAttribute(u8"class").value_or(boo::dom::DOMString());
    utf8_string padded = u8" " + classes + u8" ";
    utf8_string token = u8" " + utf8_string(className) + u8" ";
    auto position = padded.find(token);
    bool has = position != utf8_string::npos;
    if (on == has)
    {
      return;
    }
    if (on)
    {
      classes += classes.empty() ? utf8_string(className) : u8" " + utf8_string(className);
    }
    else
    {
      padded.replace(position, token.size(), u8" ");
      auto first = padded.find_first_not_of(u8' ');
      auto last = padded.find_last_not_of(u8' ');
      classes = first == utf8_string::npos ? utf8_string() : padded.substr(first, last - first + 1);
    }
    (void)element.SetAttribute(boo::dom::DOMStringAtom(u8"class"), krys::move(classes));
  }

  void Overlay::AppendEscaped(utf8_string &html, utf8_stringview text) noexcept
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
}
