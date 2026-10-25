#pragma once

#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DOMRect.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/DOM/PointerEvent.hpp"
#include "Krystal.Booey/DOM/TouchEvent.hpp"
#include "Krystal.Booey/DOM/TouchList.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <algorithm>
#include <format>
#include <string>

namespace krys::editor
{
  /// @brief The editor page's pointer samples, made from C++: `#pointer-pad` shows each pointer over it and
  /// the touches it holds in `#pointer-readout`; `#drag-chip` is dragged about `#drag-area` with pointer
  /// capture;
  /// `#lock-box` locks the pointer when clicked and shows the movement it gets in `#lock-readout`; and
  /// `#gesture-log` shows the last click, double click or context menu in `#pan-area`, which pans and flings
  /// with touch. A page without them is left alone.
  class PagePointers
  {
    KRYS_NON_COPY_MOVABLE(PagePointers);

  private:
    WeakPtr<boo::dom::Document> _document;
    List<boo::dom::EventListenerHandle> _listeners;
    /// @brief Each pointer over the pad, by its id (in order), as the readout shows it.
    List<std::pair<int32, std::string>> _pointers;
    uint32 _touches {0};
    /// @brief The pointer dragging the chip, and where on the chip it took it.
    int32 _dragPointer {-1};
    double _grabX {0};
    double _grabY {0};
    /// @brief The movement the locked box has had since it was locked.
    double _lockedX {0};
    double _lockedY {0};

  public:
    explicit PagePointers(boo::dom::Document &document) noexcept
        : _document(WeakPtr<boo::dom::Document>::WithRef(document))
    {
      if (!document.GetElementById(u8"pointer-pad"))
      {
        return;
      }

      auto listen = [&](RefPtr<boo::dom::EventTarget> target, utf8_stringview type,
                        Func<void(boo::dom::Event &)> &&callback)
      {
        if (target)
        {
          _listeners.push_back(target->AddEventListener(boo::dom::DOMStringAtom(type), krys::move(callback)));
        }
      };

      // The pad: every pointer over it, and the touches it holds.
      RefPtr<boo::dom::EventTarget> pad = document.GetElementById(u8"pointer-pad");
      for (auto type : {u8"pointerover", u8"pointermove", u8"pointerdown", u8"pointerup"})
      {
        listen(pad, type,
               [this](boo::dom::Event &event) { ShowPointer(static_cast<boo::dom::PointerEvent &>(event)); });
      }
      for (auto type : {u8"pointerleave", u8"pointercancel"})
      {
        listen(pad, type,
               [this](boo::dom::Event &event)
               {
                 auto id = static_cast<int32>(static_cast<boo::dom::PointerEvent &>(event).PointerId());
                 std::erase_if(_pointers, [&](const auto &pointer) { return pointer.first == id; });
                 ShowPad();
               });
      }
      for (auto type : {u8"touchstart", u8"touchmove", u8"touchend", u8"touchcancel"})
      {
        listen(pad, type,
               [this](boo::dom::Event &event)
               {
                 auto *touches = static_cast<boo::dom::TouchEvent &>(event).TargetTouches();
                 _touches = touches ? touches->Length() : 0;
                 ShowPad();
               });
      }

      // The chip: the pointer that presses it captures itself to it, and drags it until it lets go.
      RefPtr<boo::dom::EventTarget> chip = document.GetElementById(u8"drag-chip");
      listen(chip, u8"pointerdown",
             [this](boo::dom::Event &event)
             {
               auto &pointer = static_cast<boo::dom::PointerEvent &>(event);
               if (auto element = ById(u8"drag-chip"))
               {
                 _grabX = pointer.OffsetX();
                 _grabY = pointer.OffsetY();
                 (void)element->SetPointerCapture(static_cast<int32>(pointer.PointerId()));
               }
             });
      listen(chip, u8"gotpointercapture",
             [this](boo::dom::Event &event)
             {
               _dragPointer = static_cast<int32>(static_cast<boo::dom::PointerEvent &>(event).PointerId());
               SetText(u8"drag-chip", std::format("Dragging (pointer {})", _dragPointer));
             });
      listen(chip, u8"lostpointercapture",
             [this](boo::dom::Event &)
             {
               _dragPointer = -1;
               SetText(u8"drag-chip", "Drag me");
             });
      listen(chip, u8"pointermove",
             [this](boo::dom::Event &event) { DragChip(static_cast<boo::dom::PointerEvent &>(event)); });

      // The lock box: a click locks the pointer to it, and the mouse's movement comes to it.
      RefPtr<boo::dom::EventTarget> box = document.GetElementById(u8"lock-box");
      listen(box, u8"click",
             [this](boo::dom::Event &)
             {
               if (auto element = ById(u8"lock-box"))
               {
                 element->RequestPointerLock();
               }
             });
      listen(box, u8"mousemove",
             [this](boo::dom::Event &event)
             {
               auto &mouse = static_cast<boo::dom::MouseEvent &>(event);
               if (!IsLocked())
               {
                 return;
               }
               _lockedX += mouse.MovementX();
               _lockedY += mouse.MovementY();
               SetText(u8"lock-readout",
                       std::format("Locked: moved {:.0f}, {:.0f}; {:.0f}, {:.0f} in all. Escape unlocks it.",
                                   mouse.MovementX(), mouse.MovementY(), _lockedX, _lockedY));
             });
      RefPtr<boo::dom::EventTarget> target = ShareRefPtr(&document);
      listen(target, u8"pointerlockchange", [this](boo::dom::Event &) { ShowLock(); });
      listen(target, u8"pointerlockerror",
             [this](boo::dom::Event &) { SetText(u8"lock-readout", "The pointer can't be locked here."); });

      // The pan area: what taps, double taps and long presses do there.
      RefPtr<boo::dom::EventTarget> area = document.GetElementById(u8"pan-area");
      for (auto type : {u8"click", u8"dblclick", u8"contextmenu"})
      {
        listen(area, type,
               [this](boo::dom::Event &event) { ShowGesture(static_cast<boo::dom::MouseEvent &>(event)); });
      }

      ShowPad();
      ShowLock();
    }

  private:
    KRYS_NODISCARD RefPtr<boo::dom::Element> ById(utf8_stringview id) const noexcept
    {
      return _document ? _document->GetElementById(id) : nullptr;
    }

    void SetText(utf8_stringview id, const std::string &text) noexcept
    {
      if (auto element = ById(id))
      {
        (void)element->TextContent(
          boo::dom::DOMString(reinterpret_cast<const char8_t *>(text.data()), text.size()));
      }
    }

    KRYS_NODISCARD static std::string Narrow(boo::dom::DOMStringView text) noexcept
    {
      return std::string(text.begin(), text.end());
    }

    KRYS_NODISCARD bool IsLocked() const noexcept
    {
      auto box = ById(u8"lock-box");
      return box && _document->PointerLockElement() == box;
    }

    void ShowPointer(const boo::dom::PointerEvent &pointer) noexcept
    {
      auto id = static_cast<int32>(pointer.PointerId());
      auto line = std::format("{} {} at {:.0f}, {:.0f}: pressure {:.2f}, tilt {} {}, buttons {}",
                              Narrow(pointer.PointerType()), id, pointer.OffsetX(), pointer.OffsetY(),
                              pointer.Pressure(), pointer.TiltX(), pointer.TiltY(), pointer.Buttons());
      auto it = std::ranges::lower_bound(_pointers, id, {}, &std::pair<int32, std::string>::first);
      if (it != _pointers.end() && it->first == id)
      {
        it->second = krys::move(line);
      }
      else
      {
        _pointers.insert(it, {id, krys::move(line)});
      }
      ShowPad();
    }

    void ShowPad() noexcept
    {
      std::string text;
      for (auto &[id, line] : _pointers)
      {
        text += line + "\n";
      }
      if (_pointers.empty())
      {
        text = "Move a mouse, a pen or a finger over the pad.\n";
      }
      text += std::format("Touches held: {}", _touches);
      SetText(u8"pointer-readout", text);
    }

    void DragChip(const boo::dom::PointerEvent &pointer) noexcept
    {
      auto chip = ById(u8"drag-chip");
      auto area = ById(u8"drag-area");
      if (!chip || !area || static_cast<int32>(pointer.PointerId()) != _dragPointer)
      {
        return;
      }
      // The chip keeps its grab point under the pointer, within the area.
      auto areaRect = area->GetBoundingClientRect();
      auto chipRect = chip->GetBoundingClientRect();
      double left = std::clamp(pointer.ClientX() - areaRect->Left() - _grabX, 0.0,
                               std::max(0.0, areaRect->Width() - chipRect->Width()));
      double top = std::clamp(pointer.ClientY() - areaRect->Top() - _grabY, 0.0,
                              std::max(0.0, areaRect->Height() - chipRect->Height()));
      auto style = std::format("left: {:.0f}px; top: {:.0f}px", left, top);
      (void)chip->SetAttribute(
        u8"style", boo::dom::DOMString(reinterpret_cast<const char8_t *>(style.data()), style.size()));
    }

    void ShowLock() noexcept
    {
      if (IsLocked())
      {
        _lockedX = _lockedY = 0;
        SetText(u8"lock-readout", "Locked: move the mouse. Escape unlocks it.");
      }
      else
      {
        SetText(u8"lock-readout", "Click the box to lock the pointer to it.");
      }
    }

    void ShowGesture(const boo::dom::MouseEvent &event) noexcept
    {
      auto *pointer = DynamicDowncast<boo::dom::PointerEvent>(event);
      auto from = pointer ? " from " + Narrow(pointer->PointerType()) : std::string();
      SetText(u8"gesture-log", std::format("{}{} at {:.0f}, {:.0f}", Narrow(event.Type().View()), from,
                                           event.OffsetX(), event.OffsetY()));
    }
  };
}
