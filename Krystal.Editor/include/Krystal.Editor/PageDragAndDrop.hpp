#pragma once

#include "Krystal.Booey/DOM/DataTransfer.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DOMTokenList.hpp"
#include "Krystal.Booey/DOM/DragEvent.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/FileAPI/File.hpp"
#include "Krystal.Booey/FileAPI/FileList.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <format>
#include <string>

namespace krys::editor
{
  /// @brief The editor page's drag and drop sample, made from C++: the cards (`.drag-card`) move between and
  /// within the lists (`.drag-list`) they're dropped on; `#drop-zone` takes anything dragged onto it and
  /// lists what was dropped in `#drop-readout`; `#drag-log` shows the drag events in `#drag-and-drop` (with
  /// their data's types, and the operations allowed and chosen) and the files the file input takes, the
  /// latest last; `#drag-clear` empties it. A page without them is left alone.
  class PageDragAndDrop
  {
    KRYS_NON_COPY_MOVABLE(PageDragAndDrop);

  private:
    WeakPtr<boo::dom::Document> _document;
    List<boo::dom::EventListenerHandle> _listeners;
    /// @brief The card being dragged, while one is.
    RefPtr<boo::dom::Element> _dragged;
    /// @brief The lines the log shows, the oldest first.
    List<std::string> _lines;

  public:
    /// @brief How many lines the log shows.
    static constexpr size_t MaxLines {12};

    explicit PageDragAndDrop(boo::dom::Document &document) noexcept
        : _document(WeakPtr<boo::dom::Document>::WithRef(document))
    {
      RefPtr<boo::dom::EventTarget> sample = document.GetElementById(u8"drag-and-drop");
      if (!sample)
      {
        return;
      }

      _listeners.push_back(sample->AddEventListener(
        boo::dom::DOMStringAtom(u8"dragstart"), [this](boo::dom::Event &event) { StartCardDrag(event); }));
      for (auto type : {u8"dragenter", u8"dragover"})
      {
        _listeners.push_back(sample->AddEventListener(boo::dom::DOMStringAtom(type),
                                                      [this](boo::dom::Event &event) { DragOver(event); }));
      }
      _listeners.push_back(sample->AddEventListener(boo::dom::DOMStringAtom(u8"dragleave"),
                                                    [this](boo::dom::Event &event) { DragLeave(event); }));
      _listeners.push_back(sample->AddEventListener(boo::dom::DOMStringAtom(u8"drop"),
                                                    [this](boo::dom::Event &event) { Drop(event); }));
      _listeners.push_back(sample->AddEventListener(boo::dom::DOMStringAtom(u8"dragend"),
                                                    [this](boo::dom::Event &) { EndCardDrag(); }));
      _listeners.push_back(sample->AddEventListener(boo::dom::DOMStringAtom(u8"change"),
                                                    [this](boo::dom::Event &event) { FilesChosen(event); }));
      // Every drag event but those of each move (`drag` and `dragover`) goes in the log, as the handlers
      // above left it.
      for (auto type : {u8"dragstart", u8"dragenter", u8"dragleave", u8"drop", u8"dragend"})
      {
        _listeners.push_back(sample->AddEventListener(boo::dom::DOMStringAtom(type),
                                                      [this](boo::dom::Event &event) { Log(event); }));
      }
      if (RefPtr<boo::dom::EventTarget> clear = document.GetElementById(u8"drag-clear"))
      {
        _listeners.push_back(clear->AddEventListener(boo::dom::DOMStringAtom(u8"click"),
                                                     [this](boo::dom::Event &)
                                                     {
                                                       _lines.clear();
                                                       ShowLog();
                                                     }));
      }
      ShowLog();
      ShowDropped(u8"Drop text, links or files here, from the page or another application.");
    }

  private:
    KRYS_NODISCARD static std::string Narrow(boo::dom::DOMStringView text) noexcept
    {
      return std::string(text.begin(), text.end());
    }

    KRYS_NODISCARD static boo::dom::DOMString Widen(const std::string &text) noexcept
    {
      return boo::dom::DOMString(reinterpret_cast<const char8_t *>(text.data()), text.size());
    }

    /// @brief The element @p event went to, if it went to one.
    KRYS_NODISCARD static boo::dom::Element *TargetElement(boo::dom::Event &event) noexcept
    {
      auto *node = DynamicDowncast<boo::dom::Node>(event.Target().get());
      return node ? DynamicDowncast<boo::dom::Element>(*node) : nullptr;
    }

    /// @brief The element around @p element (or itself) that matches @p selectors, if there's one.
    KRYS_NODISCARD static RefPtr<boo::dom::Element> Closest(boo::dom::Element *element,
                                                            boo::dom::DOMStringView selectors) noexcept
    {
      if (!element)
      {
        return nullptr;
      }
      auto closest = element->Closest(selectors);
      return closest.HasValue() ? closest.Value() : nullptr;
    }

    /// @brief The lists the cards are dropped on.
    KRYS_NODISCARD List<RefPtr<boo::dom::Element>> Lists() const noexcept
    {
      List<RefPtr<boo::dom::Element>> lists;
      for (auto id : {u8"drag-todo", u8"drag-done"})
      {
        if (auto list = _document ? _document->GetElementById(id) : nullptr)
        {
          lists.push_back(krys::move(list));
        }
      }
      return lists;
    }

    /// @brief A card's drag carries its text, and only moves it.
    void StartCardDrag(boo::dom::Event &event) noexcept
    {
      auto card = Closest(TargetElement(event), u8".drag-card");
      auto &dataTransfer = *Downcast<boo::dom::DragEvent>(event).GetDataTransfer();
      if (!card || !_document)
      {
        return;
      }
      dataTransfer.SetData(*_document, u8"text/plain", card->TextContent().value_or(boo::dom::DOMString()));
      dataTransfer.SetEffectAllowed(u8"move");
      (void)card->ClassList().Add(boo::dom::DOMString(u8"dragging"));
      _dragged = krys::move(card);
    }

    void EndCardDrag() noexcept
    {
      if (_dragged)
      {
        (void)_dragged->ClassList().Remove(boo::dom::DOMString(u8"dragging"));
        _dragged = nullptr;
      }
      MarkOver(nullptr);
    }

    /// @brief Marks @p target (a list or the drop zone) as what a drop would land on, and nothing else.
    void MarkOver(boo::dom::Element *target) noexcept
    {
      auto lists = Lists();
      if (auto zone = _document ? _document->GetElementById(u8"drop-zone") : nullptr)
      {
        lists.push_back(krys::move(zone));
      }
      for (auto &element : lists)
      {
        (void)element->ClassList().Toggle(boo::dom::DOMString(u8"over"), element.get() == target);
      }
    }

    /// @brief The lists take the cards, and the drop zone takes anything, as a copy where the source allows
    /// one.
    void DragOver(boo::dom::Event &event) noexcept
    {
      auto *target = TargetElement(event);
      auto &dataTransfer = *Downcast<boo::dom::DragEvent>(event).GetDataTransfer();
      if (auto zone = Closest(target, u8"#drop-zone"))
      {
        auto allowed = dataTransfer.EffectAllowed();
        if (allowed == u8"uninitialized" || allowed == u8"all" || allowed.starts_with(u8"copy"))
        {
          dataTransfer.SetDropEffect(u8"copy");
        }
        event.PreventDefault();
        MarkOver(zone.get());
      }
      else if (auto list = Closest(target, u8".drag-list"); list && _dragged)
      {
        dataTransfer.SetDropEffect(u8"move");
        event.PreventDefault();
        MarkOver(list.get());
      }
    }

    /// @brief A drag leaving the drop zone unmarks it (its contents don't take the pointer, so a leave there
    /// is the zone's own).
    void DragLeave(boo::dom::Event &event) noexcept
    {
      auto *target = TargetElement(event);
      if (target && target->GetAttribute(u8"id") == boo::dom::DOMString(u8"drop-zone"))
      {
        (void)target->ClassList().Remove(boo::dom::DOMString(u8"over"));
      }
    }

    void Drop(boo::dom::Event &event) noexcept
    {
      auto *target = TargetElement(event);
      if (Closest(target, u8"#drop-zone"))
      {
        event.PreventDefault();
        ShowDrop(Downcast<boo::dom::DragEvent>(event));
      }
      else if (auto list = Closest(target, u8".drag-list"); list && _dragged)
      {
        event.PreventDefault();
        DropCard(*list, Closest(target, u8".drag-card"), Downcast<boo::dom::DragEvent>(event).ClientY());
      }
      MarkOver(nullptr);
    }

    /// @brief Moves the dragged card into @p list: before @p card if it was dropped on its upper half, after
    /// it if on its lower half, or at the end if on no card.
    void DropCard(boo::dom::Element &list, RefPtr<boo::dom::Element> card, double clientY) noexcept
    {
      if (card == _dragged)
      {
        return;
      }
      RefPtr<boo::dom::Node> before;
      if (card)
      {
        auto rect = card->GetBoundingClientRect();
        if (clientY < rect->Top() + rect->Height() / 2)
        {
          before = card;
        }
        else
        {
          before = ShareRefPtr(card->NextSibling());
        }
      }
      (void)list.InsertBefore(*_dragged, before.get());
    }

    /// @brief Lists what was dropped on the drop zone: its text, its URLs and its files.
    void ShowDrop(boo::dom::DragEvent &event) noexcept
    {
      if (!_document)
      {
        return;
      }
      auto &dataTransfer = *event.GetDataTransfer();
      std::string text;
      if (auto plain = Narrow(dataTransfer.GetData(*_document, u8"text/plain")); !plain.empty())
      {
        text += std::format("Text: “{}”\n", plain);
      }
      if (auto urls = Narrow(dataTransfer.GetData(*_document, u8"text/uri-list")); !urls.empty())
      {
        text += std::format("URL: {}\n", urls);
      }
      auto &files = dataTransfer.Files(*_document);
      for (uint32 i = 0; i < files.Length(); ++i)
      {
        auto &file = *files.Item(i);
        auto type = file.Type().empty() ? std::string("unknown type") : Narrow(file.Type());
        text += std::format("File: {} ({} bytes, {})\n", Narrow(file.Name()), file.Size(), type);
      }
      if (text.empty())
      {
        text = "Nothing readable was dropped.";
      }
      else
      {
        text.pop_back();
      }
      ShowDropped(Widen(text));
    }

    /// @brief Logs the files the file input took.
    void FilesChosen(boo::dom::Event &event) noexcept
    {
      auto *input = DynamicDowncast<boo::html::HTMLInputElement>(TargetElement(event));
      auto *files = input ? input->Files() : nullptr;
      if (!files)
      {
        return;
      }
      std::string names;
      for (uint32 i = 0; i < files->Length(); ++i)
      {
        names += (names.empty() ? "" : ", ") + Narrow(files->Item(i)->Name());
      }
      AddLine(std::format("input: change, {} file{}{}{}", files->Length(), files->Length() == 1 ? "" : "s",
                          names.empty() ? "" : ": ", names));
    }

    /// @brief Adds @p event to the log: what it went to, its type, its data's types, and the operations it
    /// allows and chose.
    void Log(boo::dom::Event &event) noexcept
    {
      std::string target = "?";
      if (auto *element = TargetElement(event))
      {
        auto id = element->GetAttribute(u8"id");
        target = id && !id->empty() ? Narrow(*id) : Narrow(element->LocalName().View());
      }

      auto line = std::format("{}: {}", target, Narrow(event.Type().View()));
      if (auto dataTransfer = Downcast<boo::dom::DragEvent>(event).GetDataTransfer();
          dataTransfer && _document)
      {
        std::string types;
        for (auto &type : dataTransfer->Types(*_document))
        {
          types += (types.empty() ? "" : " ") + Narrow(type);
        }
        line += std::format(" [{}] {}/{}", types, Narrow(dataTransfer->EffectAllowed()),
                            Narrow(dataTransfer->DropEffect()));
      }
      AddLine(krys::move(line));
    }

    void AddLine(std::string line) noexcept
    {
      _lines.push_back(krys::move(line));
      if (_lines.size() > MaxLines)
      {
        _lines.erase(_lines.begin());
      }
      ShowLog();
    }

    void ShowLog() noexcept
    {
      auto log = _document ? _document->GetElementById(u8"drag-log") : nullptr;
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
        text = "Drag a card, some text or a file to see the drag events here.";
      }
      else
      {
        text.pop_back();
      }
      (void)log->TextContent(Widen(text));
    }

    void ShowDropped(boo::dom::DOMString text) noexcept
    {
      if (auto readout = _document ? _document->GetElementById(u8"drop-readout") : nullptr)
      {
        (void)readout->TextContent(krys::move(text));
      }
    }
  };
}
