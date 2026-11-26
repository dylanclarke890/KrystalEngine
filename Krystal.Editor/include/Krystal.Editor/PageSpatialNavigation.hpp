#pragma once

#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Web/Dom/Events/EventListenerHandle.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Events/UI/KeyboardEvent.hpp"
#include "Krystal.Web/Html/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Web/Html/Elements/HTMLSelectElement.hpp"
#include <format>
#include <string>

namespace krys::editor
{
  /// @brief The editor page's spatial navigation sample, made from C++: the arrow keys move the focus
  /// spatially within `#nav-grid` (as a pad does anywhere), and `#nav-log` shows what was pressed or chosen
  /// there. A page without it is left alone.
  class PageSpatialNavigation
  {
    KRYS_NON_COPY_MOVABLE(PageSpatialNavigation);

  private:
    WeakPtr<web::Document> _document;
    List<web::EventListenerHandle> _listeners;

  public:
    explicit PageSpatialNavigation(web::Document &document) noexcept : _document(CreateWeakPtr(&document))
    {
      RefPtr<web::EventTarget> grid = document.GetElementById(u8"nav-grid");
      if (!grid)
      {
        return;
      }

      auto listen = [&](utf8_stringview type, Func<void(web::Event &)> &&callback)
      {
        _listeners.push_back(grid->AddEventListener(web::DOMStringAtom(type), krys::move(callback)));
      };

      // The arrow keys move the focus as the D-pad does, and the log shows what was pressed.
      listen(u8"keydown", [this](web::Event &event) { MoveFocus(static_cast<web::KeyboardEvent &>(event)); });
      listen(u8"click",
             [this](web::Event &event)
             {
               auto *target = TargetElement(event);
               if (target && target->LocalName() == u8"button")
               {
                 SetText(u8"nav-log",
                         std::format("Pressed “{}”", Narrow(target->TextContent().value_or(u8""))));
               }
             });
      listen(u8"change",
             [this](web::Event &event)
             {
               auto *target = TargetElement(event);
               if (!target)
               {
                 return;
               }
               auto *select = DynamicDowncast<web::HTMLSelectElement>(*target);
               auto value = select ? select->Value() : target->GetAttribute(u8"value").value_or(u8"");
               SetText(u8"nav-log", std::format("Chose “{}”", Narrow(value)));
             });
    }

  private:
    void SetText(utf8_stringview id, const std::string &text) noexcept
    {
      if (auto element = _document ? _document->GetElementById(id) : nullptr)
      {
        (void)element->TextContent(
          web::DOMString(reinterpret_cast<const char8_t *>(text.data()), text.size()));
      }
    }

    KRYS_NODISCARD static web::Element *TargetElement(web::Event &event) noexcept
    {
      auto *node = DynamicDowncast<web::Node>(event.Target().get());
      return node ? DynamicDowncast<web::Element>(*node) : nullptr;
    }

    KRYS_NODISCARD static std::string Narrow(web::DOMStringView text) noexcept
    {
      return std::string(text.begin(), text.end());
    }

    void MoveFocus(web::KeyboardEvent &event) noexcept
    {
      using FocusDirection = web::html::FocusDirection;
      if (!_document || event.CtrlKey() || event.AltKey() || event.MetaKey() || event.ShiftKey())
      {
        return;
      }
      auto key = event.Key();
      auto direction = key == u8"ArrowUp"      ? FocusDirection::Up
                       : key == u8"ArrowDown"  ? FocusDirection::Down
                       : key == u8"ArrowLeft"  ? FocusDirection::Left
                       : key == u8"ArrowRight" ? FocusDirection::Right
                                               : FocusDirection::None;
      if (direction != FocusDirection::None
          && web::html::FocusAlgorithms::DirectionalNavigation(*_document, direction))
      {
        event.PreventDefault();
      }
    }
  };
}
