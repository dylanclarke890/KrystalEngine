#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElement.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/HTML/Parser/DocumentParser.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"

namespace krys::boo::html::tests
{
  using Log = List<utf8_string>;

  inline utf8_string Text(const dom::DOMStringAtom &value) noexcept
  {
    return value == null ? utf8_string(u8"null") : value.ToString();
  }

  /// @brief Logs its callbacks, with the element's id.
  class LoggingElement : public CustomElement
  {
  private:
    Log &_log;

  public:
    LoggingElement(HTMLElement &element, Log &log) noexcept : CustomElement(element), _log(log)
    {
      _log.emplace_back(u8"constructed");
    }

    void ConnectedCallback() noexcept override
    {
      _log.push_back(u8"connected " + Id());
    }

    void DisconnectedCallback() noexcept override
    {
      _log.push_back(u8"disconnected " + Id());
    }

    void ConnectedMoveCallback() noexcept override
    {
      _log.push_back(u8"moved " + Id());
    }

    void AdoptedCallback(dom::Document &, dom::Document &) noexcept override
    {
      _log.push_back(u8"adopted " + Id());
    }

    void AttributeChangedCallback(dom::DOMStringAtom localName, dom::DOMStringAtom oldValue,
                                  dom::DOMStringAtom newValue, dom::DOMStringAtom) noexcept override
    {
      _log.push_back(u8"attribute " + localName.ToString() + u8" " + Text(oldValue) + u8" " + Text(newValue));
    }

  private:
    utf8_string Id() const noexcept
    {
      return Element().GetAttribute(u8"id").value_or(u8"");
    }
  };

  /// @brief A document in a frame, which gives it a global registry, parsed after the test defines its
  /// elements.
  struct CustomElementFixture : css::tests::ValueFixture
  {
    RefPtr<LocalFrame> Frame;
    Ref<dom::HTMLDocument> Document;
    Log Calls;

    CustomElementFixture() noexcept : Frame(LocalFrame::Create()), Document(CreateRef<dom::HTMLDocument>())
    {
      EventLoop::Current().RunUntilIdle();
      Frame->CreateView({200, 100});
      Frame->SetDocument(ShareRefPtr<dom::Document>(Document.get()));
    }

    ~CustomElementFixture() noexcept
    {
      EventLoop::Current().RunUntilIdle();
    }

    CustomElementRegistry &Registry() const noexcept
    {
      auto *registry = Document->CustomElementRegistry().get(); // The document keeps it alive.
      return *registry;
    }

    void Define(dom::DOMStringAtom name, List<dom::DOMStringAtom> observed = {}) noexcept
    {
      auto result = Registry().Define(krys::move(name), [this](HTMLElement &element)
                                      { return CreateUnique<LoggingElement>(element, Calls); },
                                      {.ObservedAttributes = krys::move(observed)});
      REQUIRE_FALSE(result.HasException());
    }

    void Parse(utf8_string &&markup) noexcept
    {
      auto parser = CreateRef<DocumentParser>(*Document);
      parser->InputStream().Append(u8"<!DOCTYPE html><body>" + krys::move(markup), IsEOF::Yes);
      parser->PumpTokenizer();
    }

    /// @brief The body, parsing an empty page first if there's none.
    HTMLBodyElement &Body() noexcept
    {
      if (!Document->Body())
      {
        Parse(u8"");
      }
      auto *body = Document->Body().get(); // The document keeps it alive.
      return *body;
    }

    dom::Element &ById(dom::DOMStringView id) const noexcept
    {
      auto element = Document->GetElementById(id);
      REQUIRE(element);
      return *element;
    }

    Ref<dom::Element> Create(dom::DOMStringAtom name) noexcept
    {
      auto element = Document->CreateElement(krys::move(name));
      REQUIRE_FALSE(element.HasException());
      return element.ReleaseValue();
    }

    Log TakeCalls() noexcept
    {
      return std::exchange(Calls, {});
    }
  };
}
