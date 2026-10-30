#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

namespace krys::boo::tests
{
  /// @brief A document parsed in a frame, as a navigation's is, so its images load as they're inserted.
  struct PageInFrameFixture : css::tests::ValueFixture
  {
    IntSize ViewSize;
    RefPtr<LocalFrame> Frame;
    Ref<dom::HTMLDocument> Document;

    explicit PageInFrameFixture(utf8_string &&markup, IntSize viewSize = {200, 100},
                                CachePolicy cachePolicy = CachePolicy::Verify) noexcept
        : ViewSize(viewSize), Frame(LocalFrame::Create()), Document(CreateRef<dom::HTMLDocument>())
    {
      html::EventLoop::Current().RunUntilIdle();
      Frame->SetSubresourceCachePolicy(cachePolicy);
      Frame->CreateView(ViewSize);
      Frame->SetDocument(ShareRefPtr<dom::Document>(Document.get()));
      auto parser = CreateRef<html::HTMLDocumentParser>(*Document);
      parser->InputStream().Append(u8"<!DOCTYPE html>" + krys::move(markup), IsEOF(true));
      (void)parser->PumpTokenizer();
      Document->StyleScope().FlushPendingUpdate();
    }

    KRYS_NODISCARD dom::Element &ById(dom::DOMStringView id) noexcept
    {
      auto element = Document->GetElementById(id);
      krys_assert(element);
      return *element;
    }

    /// @brief The size of @p id's renderer, laid out.
    KRYS_NODISCARD IntSize RendererSize(dom::DOMStringView id) noexcept
    {
      (void)Document->UpdateLayout();
      auto *box = DynamicDowncast<RenderBox>(ById(id).Renderer());
      REQUIRE(box);
      return IntSize(box->Width().ToInt(), box->Height().ToInt());
    }

    /// @brief The display list of the whole view, laid out, as text.
    KRYS_NODISCARD utf8_string Painted() noexcept
    {
      (void)Document->UpdateLayout();
      display_list::RecorderImpl recorder(
        FloatSize(static_cast<float>(ViewSize.Width()), static_cast<float>(ViewSize.Height())));
      Frame->View()->PaintContents(recorder, IntRect(IntPoint(), ViewSize));
      return recorder.TakeDisplayList()->AsText({});
    }
  };

  /// @brief Counts an element's `load` and `error` events.
  struct LoadEvents : dom::EventListenerCallback
  {
    int32 Loads {0};
    int32 Errors {0};

    void HandleEvent(dom::Event &event) noexcept override
    {
      if (event.Type() == u8"load")
      {
        ++Loads;
      }
      else if (event.Type() == u8"error")
      {
        ++Errors;
      }
    }

    KRYS_NODISCARD static Ref<LoadEvents> ListenTo(dom::Element &element) noexcept
    {
      auto events = CreateRef<LoadEvents>();
      element.AddEventListener(dom::DOMStringAtom(u8"load"), Ref<dom::EventListenerCallback> {events}, false);
      element.AddEventListener(dom::DOMStringAtom(u8"error"), Ref<dom::EventListenerCallback> {events},
                               false);
      return events;
    }
  };
}
