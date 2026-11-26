#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"
#include "Krystal.Web/Dom/Events/EventListener.hpp"
#include "Krystal.Web/Dom/Nodes/HTMLDocument.hpp"
#include "Krystal.Web/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Web/Html/Parser/DocumentParser.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include "Krystal.Web/Rendering/RenderBox.hpp"
#include "Krystal.Web/Style/Scope/Scope.hpp"

namespace krys::web::tests
{
  /// @brief A document parsed in a frame, as a navigation's is, so its images load as they're inserted.
  struct PageInFrameFixture : css::tests::ValueFixture
  {
    SizeI ViewSize;
    RefPtr<web::Frame> Frame;
    Ref<HTMLDocument> Document;

    explicit PageInFrameFixture(utf8_string markup, SizeI viewSize = {200, 100},
                                CachePolicy cachePolicy = CachePolicy::Use) noexcept
        : PageInFrameFixture(web::Frame::Create(), krys::move(markup), viewSize, cachePolicy)
    {
    }

    /// @brief As above, in @p frame.
    PageInFrameFixture(web::Frame &frame, utf8_string markup, SizeI viewSize = {200, 100}) noexcept
        : PageInFrameFixture(ShareRef(frame), krys::move(markup), viewSize, CachePolicy::Use)
    {
    }

    PageInFrameFixture(Ref<web::Frame> frame, utf8_string markup, SizeI viewSize,
                       CachePolicy cachePolicy) noexcept
        : ViewSize(viewSize), Frame(krys::move(frame)), Document(CreateRef<HTMLDocument>())
    {
      html::EventLoop::Current().RunUntilIdle();
      Frame->SetSubresourceCachePolicy(cachePolicy);
      Frame->CreateView(ViewSize);
      Frame->SetDocument(ShareRefPtr<web::Document>(Document.get()));
      auto parser = CreateRef<html::DocumentParser>(*Document);
      parser->InputStream().Append(u8"<!DOCTYPE html>" + krys::move(markup), IsEOF::Yes);
      parser->PumpTokenizer();
      Document->StyleScope().FlushPendingUpdate();
    }

    KRYS_NODISCARD Element &ById(DOMStringView id) const noexcept
    {
      auto element = Document->GetElementById(id);
      krys_assert(element);
      return *element;
    }

    /// @brief The size of @p id's renderer, laid out.
    KRYS_NODISCARD SizeI RendererSize(DOMStringView id) noexcept
    {
      (void)Document->UpdateLayout();
      auto *box = DynamicDowncast<RenderBox>(ById(id).Renderer());
      REQUIRE(box);
      return SizeI(box->Width().ToInt(), box->Height().ToInt());
    }

    /// @brief The display list of the whole view, laid out, as text.
    KRYS_NODISCARD utf8_string Painted() const noexcept
    {
      (void)Document->UpdateLayout();
      display_list::RecorderImpl recorder(
        SizeF(static_cast<float>(ViewSize.Width()), static_cast<float>(ViewSize.Height())));
      Frame->View()->PaintContents(recorder, RectI(PointI(), ViewSize));
      return recorder.TakeDisplayList()->AsText({});
    }
  };

  /// @brief Counts an element's `load` and `error` events.
  struct LoadEvents : EventListenerCallback
  {
    int32 Loads {0};
    int32 Errors {0};

    void HandleEvent(Event &event) noexcept override
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

    KRYS_NODISCARD static Ref<LoadEvents> ListenTo(Element &element) noexcept
    {
      auto events = CreateRef<LoadEvents>();
      element.AddEventListener(u8"load", events, false);
      element.AddEventListener(u8"error", events, false);
      return events;
    }
  };
}
