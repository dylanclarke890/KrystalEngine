#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Animation/AnimationEventBase.hpp"
#include "Krystal.Booey/Animation/CSSAnimationEvent.hpp"
#include "Krystal.Booey/Animation/CSSTransitionEvent.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include <string>

namespace krys::boo::tests
{
  /// @brief A page with a 200x100 view, and @p markup parsed into its main frame's document, to run
  /// animations on.
  struct AnimationPageFixture
  {
    Ref<Page> TestPage {Page::Create()};
    RefPtr<dom::HTMLDocument> Document;

    explicit AnimationPageFixture(utf8_string &&markup) noexcept
    {
      auto &frame = TestPage->MainFrame();
      frame.CreateView({200, 100});
      Document = CreateRef<dom::HTMLDocument>();
      frame.SetDocument(ShareRefPtr<dom::Document>(Document.get()));
      auto parser = CreateRef<html::HTMLDocumentParser>(*Document);
      parser->InputStream().Append(krys::move(markup), IsEOF(true));
      parser->PumpTokenizer();
      Document->StyleScope().FlushPendingUpdate();
      TestPage->UpdateRendering();
    }

    ~AnimationPageFixture() noexcept
    {
      TestPage->MainFrame().SetDocument(nullptr);
    }

    KRYS_NODISCARD dom::Element &ById(dom::DOMStringView id) noexcept
    {
      auto element = Document->GetElementById(id);
      REQUIRE(element);
      return *element;
    }

    void SetStyle(dom::DOMStringView id, utf8_stringview style) noexcept
    {
      REQUIRE_FALSE(ById(id).SetAttribute(u8"style", dom::DOMString {style}).HasException());
    }

    /// @brief Runs the event loop @p delta after its last run, then the rendering update if one is scheduled.
    void Frame(MonotonicTime::duration delta = {}) noexcept
    {
      auto &loop = html::EventLoop::Current();
      loop.Run(loop.Now() + delta);
      if (TestPage->IsRenderingUpdateScheduled())
      {
        TestPage->UpdateRendering();
      }
    }

    /// @brief Runs frames until the animations created by a style change are ready: their start time is the
    /// time of the frame after the change.
    void StartAnimations() noexcept
    {
      Frame();
      Frame();
    }

    KRYS_NODISCARD float Opacity(dom::DOMStringView id) noexcept
    {
      auto *renderer = ById(id).Renderer();
      REQUIRE(renderer);
      return renderer->Style().GetOpacity().Value.Value;
    }
  };

  /// @brief Records the animation events fired at an element: their type, then the transition's property or
  /// the animation's name.
  struct AnimationEventRecorder : dom::EventListenerCallback
  {
    List<std::string> Records;

    void HandleEvent(dom::Event &event) noexcept override
    {
      auto type = event.Type().View();
      std::string record(type.begin(), type.end());
      if (auto *transitionEvent = dynamic_cast<CSSTransitionEvent *>(&event))
      {
        const auto &property = transitionEvent->PropertyName();
        record += ' ';
        record.append(property.begin(), property.end());
      }
      else if (auto *animationEvent = dynamic_cast<CSSAnimationEvent *>(&event))
      {
        const auto &name = animationEvent->AnimationName();
        record += ' ';
        record.append(name.begin(), name.end());
      }
      Records.push_back(krys::move(record));
    }

    void Listen(dom::Element &element, std::initializer_list<const char8_t *> types) noexcept
    {
      for (auto type : types)
      {
        element.AddEventListener(dom::DOMStringAtom(type), ShareRef<dom::EventListenerCallback>(*this),
                                 false);
      }
    }
  };
}
