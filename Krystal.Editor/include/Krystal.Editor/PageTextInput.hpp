#pragma once

#include "Krystal.Booey/Dom/Events/EventListenerHandle.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Dom/Nodes/Element.hpp"
#include "Krystal.Booey/Events/UI/CompositionEvent.hpp"
#include "Krystal.Booey/Events/UI/InputEvent.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include <format>
#include <string>

namespace krys::editor
{
  /// @brief The editor page's input method sample, made from C++: `#ime-log` shows the composition events
  /// and the `beforeinput` and `input` events of the fields in `#ime-fields` (with their data, and whether
  /// they're part of a composition), the latest last; `#ime-clear` empties it. A page without them is left
  /// alone.
  class PageTextInput
  {
    KRYS_NON_COPY_MOVABLE(PageTextInput);

  private:
    WeakPtr<boo::Document> _document;
    List<boo::EventListenerHandle> _listeners;
    /// @brief The lines the log shows, the oldest first.
    List<std::string> _lines;

  public:
    /// @brief How many events the log shows.
    constexpr static size_t MaxLines {12};

    explicit PageTextInput(boo::Document &document) noexcept : _document(CreateWeakPtr(&document))
    {
      RefPtr<boo::EventTarget> fields = document.GetElementById(u8"ime-fields");
      if (!fields)
      {
        return;
      }

      for (auto type :
           {u8"compositionstart", u8"compositionupdate", u8"compositionend", u8"beforeinput", u8"input"})
      {
        _listeners.push_back(
          fields->AddEventListener(boo::DOMStringAtom(type), [this](boo::Event &event) { Log(event); }));
      }
      if (RefPtr<boo::EventTarget> clear = document.GetElementById(u8"ime-clear"))
      {
        _listeners.push_back(clear->AddEventListener(u8"click",
                                                     [this](boo::Event &)
                                                     {
                                                       _lines.clear();
                                                       Show();
                                                     }));
      }
      Show();
    }

  private:
    KRYS_NODISCARD static std::string Narrow(boo::DOMStringView text) noexcept
    {
      return std::string(text.begin(), text.end());
    }

    /// @brief Adds @p event to the log: the field it went to, its type, its data and whether it's part of a
    /// composition.
    void Log(boo::Event &event) noexcept
    {
      std::string field = "?";
      if (auto *node = DynamicDowncast<boo::Node>(event.Target().get()))
      {
        if (auto *element = DynamicDowncast<boo::Element>(*node))
        {
          field = Narrow(element->LocalName().View());
        }
      }

      auto line = std::format("{}: {}", field, Narrow(event.Type().View()));
      if (event.IsCompositionEvent())
      {
        line += std::format(" “{}”", Narrow(Downcast<boo::CompositionEvent>(event).Data()));
      }
      else if (event.IsInputEvent())
      {
        auto &input = Downcast<boo::InputEvent>(event);
        line += " " + Narrow(input.InputType());
        if (input.Data())
        {
          line += std::format(" “{}”", Narrow(*input.Data()));
        }
        if (input.IsComposing())
        {
          line += " (composing)";
        }
      }

      _lines.push_back(krys::move(line));
      if (_lines.size() > MaxLines)
      {
        _lines.erase(_lines.begin());
      }
      Show();
    }

    void Show() noexcept
    {
      auto log = _document ? _document->GetElementById(u8"ime-log") : nullptr;
      if (!log)
      {
        return;
      }
      std::string text;
      for (auto &line : _lines)
      {
        text += line + "\n";
      }
      if (text.empty())
      {
        text = "Type into a field with an input method (or the emoji panel, Win+.) to see its events here.";
      }
      else
      {
        text.pop_back();
      }
      (void)log->TextContent(boo::DOMString(reinterpret_cast<const char8_t *>(text.data()), text.size()));
    }
  };
}
