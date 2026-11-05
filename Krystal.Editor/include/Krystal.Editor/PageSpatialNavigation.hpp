#pragma once

#include "Krystal.Booey/DOM/Events/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/Events/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
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
    WeakPtr<boo::dom::Document> _document;
    List<boo::dom::EventListenerHandle> _listeners;

  public:
    explicit PageSpatialNavigation(boo::dom::Document &document) noexcept
        : _document(WeakPtr<boo::dom::Document>::WithRef(document))
    {
      RefPtr<boo::dom::EventTarget> grid = document.GetElementById(u8"nav-grid");
      if (!grid)
      {
        return;
      }

      auto listen = [&](utf8_stringview type, Func<void(boo::dom::Event &)> &&callback)
      {
        _listeners.push_back(grid->AddEventListener(boo::dom::DOMStringAtom(type), krys::move(callback)));
      };

      // The arrow keys move the focus as the D-pad does, and the log shows what was pressed.
      listen(u8"keydown",
             [this](boo::dom::Event &event) { MoveFocus(static_cast<boo::dom::KeyboardEvent &>(event)); });
      listen(u8"click",
             [this](boo::dom::Event &event)
             {
               auto *target = TargetElement(event);
               if (target && target->LocalName() == u8"button")
               {
                 SetText(u8"nav-log",
                         std::format("Pressed “{}”", Narrow(target->TextContent().value_or(u8""))));
               }
             });
      listen(u8"change",
             [this](boo::dom::Event &event)
             {
               auto *target = TargetElement(event);
               if (!target)
               {
                 return;
               }
               auto *select = DynamicDowncast<boo::html::HTMLSelectElement>(*target);
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
          boo::dom::DOMString(reinterpret_cast<const char8_t *>(text.data()), text.size()));
      }
    }

    KRYS_NODISCARD static boo::dom::Element *TargetElement(boo::dom::Event &event) noexcept
    {
      auto *node = DynamicDowncast<boo::dom::Node>(event.Target().get());
      return node ? DynamicDowncast<boo::dom::Element>(*node) : nullptr;
    }

    KRYS_NODISCARD static std::string Narrow(boo::dom::DOMStringView text) noexcept
    {
      return std::string(text.begin(), text.end());
    }

    void MoveFocus(boo::dom::KeyboardEvent &event) noexcept
    {
      using FocusDirection = boo::dom::FocusDirection;
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
          && boo::html::FocusAlgorithms::DirectionalNavigation(*_document, direction))
      {
        event.PreventDefault();
      }
    }
  };
}
