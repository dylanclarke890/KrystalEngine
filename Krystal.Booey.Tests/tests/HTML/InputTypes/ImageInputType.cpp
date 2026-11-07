#include "Krystal.Booey/HTML/InputTypes/ImageInputType.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/TestImages.hpp"
#include "Krystal.Booey/DOM/Nodes/DOMImplementation.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Platform/Events/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Rendering/Replaced/RenderImage.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  namespace
  {
    struct InputFixture : PageInFrameFixture
    {
      using PageInFrameFixture::PageInFrameFixture;

      KRYS_NODISCARD html::HTMLInputElement &Input(dom::DOMStringView id) noexcept
      {
        return Downcast<html::HTMLInputElement>(ById(id));
      }

      KRYS_NODISCARD bool ShowsImage(dom::DOMStringView id) noexcept
      {
        (void)Document->UpdateLayout();
        auto *renderer = DynamicDowncast<RenderImage>(ById(id).Renderer());
        return renderer && renderer->GetCachedImage();
      }

      KRYS_NODISCARD bool IsRenderImage(dom::DOMStringView id) noexcept
      {
        (void)Document->UpdateLayout();
        return Is<RenderImage>(ById(id).Renderer());
      }
    };

    KRYS_NODISCARD size_t Count(const utf8_string &text, utf8_stringview part) noexcept
    {
      size_t count = 0;
      for (auto position = text.find(part); position != utf8_string::npos;
           position = text.find(part, position + 1))
      {
        ++count;
      }
      return count;
    }
  }

  TEST_CASE("ImageInputType: loads its source, fires load and paints",
            "[HTML][ImageInputType][html:attr-input-src][html:input-img-available]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;
    loader.Strategy.Defer = true;

    InputFixture fixture(u8"<input id=i type=image src='file:///a.png'>");
    auto &input = fixture.Input(u8"i");
    auto events = LoadEvents::ListenTo(input);

    REQUIRE(loader.Strategy.Requests == List<utf8_string> {u8"file:///a.png"});
    CHECK(events->Loads == 0);
    CHECK(fixture.IsRenderImage(u8"i"));

    loader.Strategy.CompleteDeferred();
    RunNetworkingTasks();

    CHECK(events->Loads == 1);
    CHECK(events->Errors == 0);
    CHECK(input.Width() == 3u);
    CHECK(input.Height() == 2u);
    CHECK(fixture.RendererSize(u8"i") == IntSize(3, 2));
    CHECK(fixture.ShowsImage(u8"i"));
    CHECK(Count(fixture.Painted(), u8"draw-native-image") == 1u);

    CHECK(input.Renderer()->Style().GetCursorType() == CursorType::Pointer);
    CHECK(fixture.RendererSize(u8"i") == IntSize(3, 2));
  }

  TEST_CASE("ImageInputType: the width and height attributes size its box",
            "[HTML][ImageInputType][html:dom-input-width][html:input-img-dimensions]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;

    InputFixture fixture(u8"<input id=both type=image src='file:///a.png' width=30 height=30>"
                         u8"<input id=width type=image src='file:///a.png' width=6>"
                         u8"<input id=css type=image src='file:///a.png' "
                         u8"style='width: 12px; padding: 5px; border: 2px solid'>");
    RunNetworkingTasks();

    CHECK(fixture.RendererSize(u8"both") == IntSize(30, 30));
    CHECK(fixture.RendererSize(u8"width") == IntSize(6, 4));
    CHECK(fixture.RendererSize(u8"css") == IntSize(26, 22));
    // The content box.
    CHECK(fixture.Input(u8"css").Width() == 12u);
    CHECK(fixture.Input(u8"css").Height() == 8u);
  }

  TEST_CASE("ImageInputType: loads without being rendered",
            "[HTML][ImageInputType][html:input-img-dimensions]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;

    InputFixture fixture(u8"<input id=hidden type=image src='file:///a.png' style='display: none'>");
    auto createdElement = fixture.Document->CreateElement(u8"input").ReleaseValue();
    auto &created = Downcast<html::HTMLInputElement>(*createdElement);
    auto createdEvents = LoadEvents::ListenTo(created);
    created.Type(u8"image");
    created.Src(u8"file:///a.png");
    auto hiddenEvents = LoadEvents::ListenTo(fixture.ById(u8"hidden"));
    RunNetworkingTasks();

    CHECK(createdEvents->Loads == 1);
    CHECK(hiddenEvents->Loads == 1);
    CHECK(created.Width() == 3u);
    CHECK(created.Height() == 2u);
    CHECK(fixture.Input(u8"hidden").Width() == 3u);

    // Each attribute replaces its own natural dimension.
    created.Width(7u);
    CHECK(created.Width() == 7u);
    CHECK(created.Height() == 2u);

    // It shows the loaded image when it is rendered later.
    fixture.ById(u8"hidden").RemoveAttribute(u8"style");
    CHECK(fixture.ShowsImage(u8"hidden"));
    CHECK(fixture.RendererSize(u8"hidden") == IntSize(3, 2));
    CHECK(hiddenEvents->Loads == 1);
  }

  TEST_CASE("ImageInputType: without an image it shows its label",
            "[HTML][ImageInputType][html:attr-input-alt]")
  {
    LoaderFixture loader;

    InputFixture fixture(u8"<style>input { display: block }</style>"
                         u8"<input id=missing type=image alt=Label src='file:///none.png'>"
                         u8"<input id=title type=image title=Label src='file:///none.png'>"
                         u8"<input id=value type=image value=Label src='file:///none.png'>"
                         u8"<input id=submit type=image alt=Submit src='file:///none.png'>"
                         u8"<input id=none type=image src='file:///none.png'>"
                         u8"<input id=nosrc type=image alt=Label>"
                         u8"<input id=nosrcNone type=image>"
                         u8"<input id=empty type=image alt=Label src=''>"
                         u8"<input id=sized type=image alt=Label src='file:///none.png' width=100 height=30>"
                         u8"<div><input id=collapsed style='display: inline' type=image alt='' "
                         u8"src='file:///none.png'></div>",
                         IntSize(400, 400));
    auto missingEvents = LoadEvents::ListenTo(fixture.ById(u8"missing"));
    auto noSourceEvents = LoadEvents::ListenTo(fixture.ById(u8"nosrc"));
    auto emptyEvents = LoadEvents::ListenTo(fixture.ById(u8"empty"));
    RunNetworkingTasks();

    CHECK(missingEvents->Errors == 1);
    CHECK(missingEvents->Loads == 0);
    CHECK(noSourceEvents->Errors == 0);
    CHECK(emptyEvents->Errors == 1);

    auto size = fixture.RendererSize(u8"missing");
    CHECK(size.Width() > 20);
    CHECK(size.Height() > 10);
    CHECK(fixture.RendererSize(u8"title") == size);
    CHECK(fixture.RendererSize(u8"value") == size);
    CHECK(fixture.RendererSize(u8"nosrc") == size);
    CHECK(fixture.RendererSize(u8"empty") == size);
    CHECK(fixture.RendererSize(u8"none") == fixture.RendererSize(u8"submit"));
    CHECK(fixture.RendererSize(u8"nosrcNone") == fixture.RendererSize(u8"submit"));
    CHECK(fixture.RendererSize(u8"sized") == IntSize(100, 30));
    CHECK(fixture.RendererSize(u8"collapsed") == IntSize(0, 0));
    CHECK(fixture.Input(u8"missing").Width() == static_cast<uint32>(size.Width()));

    CHECK(Count(fixture.Painted(), u8"draw-glyphs") == 9u);
    CHECK(Count(fixture.Painted(), u8"draw-native-image") == 0u);
  }

  TEST_CASE("ImageInputType: changing src and alt", "[HTML][ImageInputType][html:attr-input-src]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;
    loader.Strategy.Resources[u8"file:///b.jpg"] = images::JPEG;

    InputFixture fixture(u8"<input id=i type=image alt=a>");
    auto &input = fixture.Input(u8"i");
    auto events = LoadEvents::ListenTo(input);
    RunNetworkingTasks();

    // The alt text is what shows, so its box follows it.
    auto small = fixture.RendererSize(u8"i");
    input.Alt(u8"a much longer label");
    CHECK(fixture.RendererSize(u8"i").Width() > small.Width());

    input.Src(u8"file:///a.png");
    RunNetworkingTasks();
    CHECK(events->Loads == 1);
    CHECK(fixture.RendererSize(u8"i") == IntSize(3, 2));

    // An alt change leaves the image alone.
    input.Alt(u8"a");
    CHECK(fixture.RendererSize(u8"i") == IntSize(3, 2));

    input.Src(u8"file:///b.jpg");
    RunNetworkingTasks();
    CHECK(events->Loads == 2);
    CHECK(loader.Strategy.Requests == List<utf8_string> {u8"file:///a.png", u8"file:///b.jpg"});
    CHECK(fixture.ShowsImage(u8"i"));
    CHECK(fixture.RendererSize(u8"i") != IntSize(3, 2));

    // A failed source shows the label again.
    input.Src(u8"file:///none.png");
    RunNetworkingTasks();
    CHECK(events->Errors == 1);
    CHECK(fixture.RendererSize(u8"i") == small);

    // So does having no source.
    input.Src(u8"file:///a.png");
    RunNetworkingTasks();
    CHECK(fixture.RendererSize(u8"i") == IntSize(3, 2));
    input.RemoveAttribute(u8"src");
    RunNetworkingTasks();
    CHECK_FALSE(fixture.ShowsImage(u8"i"));
    CHECK(fixture.RendererSize(u8"i") == small);
  }

  TEST_CASE("ImageInputType: loading follows the type", "[HTML][ImageInputType][html:attr-input-src]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;
    loader.Strategy.Resources[u8"file:///b.png"] = images::PNG;

    InputFixture fixture(
      u8"<input id=text src='file:///a.png'><input id=first src='file:///b.png' type=image>");
    auto &text = fixture.Input(u8"text");
    // The src attribute came first, and the type started the load.
    CHECK(loader.Strategy.Requests == List<utf8_string> {u8"file:///b.png"});
    RunNetworkingTasks();
    CHECK(fixture.ShowsImage(u8"first"));
    loader.Strategy.Requests.clear();

    CHECK_FALSE(fixture.IsRenderImage(u8"text"));
    CHECK(text.Width() == 0u);
    auto events = LoadEvents::ListenTo(text);
    text.Type(u8"image");
    RunNetworkingTasks();
    CHECK(loader.Strategy.Requests == List<utf8_string> {u8"file:///a.png"});
    CHECK(events->Loads == 1);
    CHECK(fixture.ShowsImage(u8"text"));
    CHECK(fixture.RendererSize(u8"text") == IntSize(3, 2));

    // Away from the state, it's an ordinary control, and a new src waits.
    text.Type(u8"text");
    CHECK_FALSE(fixture.IsRenderImage(u8"text"));
    CHECK(text.Width() == 0u);

    // Back with the same src, nothing is fetched or fired.
    loader.Strategy.Requests.clear();
    text.Type(u8"image");
    RunNetworkingTasks();
    CHECK(loader.Strategy.Requests.empty());
    CHECK(events->Loads == 1);
    CHECK(fixture.ShowsImage(u8"text"));
    CHECK(fixture.RendererSize(u8"text") == IntSize(3, 2));

    text.Type(u8"text");
    text.Src(u8"file:///b.png");
    RunNetworkingTasks();
    CHECK(events->Loads == 1);
    CHECK(text.Width() == 0u);

    // Back with another, it is loaded.
    text.Type(u8"image");
    RunNetworkingTasks();
    CHECK(events->Loads == 2);
    CHECK(fixture.ShowsImage(u8"text"));
  }

  TEST_CASE("ImageInputType: clones load, and adoption starts a load", "[HTML][ImageInputType]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;

    InputFixture fixture(u8"<input id=i type=image src='file:///a.png'>");
    RunNetworkingTasks();
    auto clone = fixture.Input(u8"i").CloneNode().ReleaseValue();
    RunNetworkingTasks();
    CHECK(Downcast<html::HTMLInputElement>(*clone).Width() == 3u);

    // A document without a browsing context loads nothing.
    loader.Strategy.Requests.clear();
    auto inert = fixture.Document->Implementation().CreateHTMLDocument(u8"").ReleaseValue();
    auto element = inert->CreateElement(u8"input").ReleaseValue();
    auto &input = Downcast<html::HTMLInputElement>(*element);
    input.Type(u8"image");
    input.Src(u8"file:///a.png");
    RunNetworkingTasks();
    CHECK(input.Width() == 0u);

    REQUIRE_FALSE(fixture.Document->Body()->AppendChild(input).HasException());
    RunNetworkingTasks();
    CHECK(input.Width() == 3u);
  }

  TEST_CASE("ImageInputType: a click on the image submits where it was",
            "[HTML][ImageInputType][html:concept-input-type-image-coordinate]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;

    Ref<Page> page = Page::Create();
    InputFixture fixture(
      *page,
      u8"<style>body { margin: 0 } dialog { inset: 0; margin: 0; padding: 0; border: 0 }</style>"
      u8"<dialog id=d open><form method=dialog><input id=i type=image src='file:///a.png' width=30 height=20 "
      u8"style='display: block'></form></dialog>");
    RunNetworkingTasks();
    (void)fixture.Document->UpdateLayout();

    auto event = [](PlatformEvent::Type type, double x, double y)
    {
      return PlatformMouseEvent(DoublePoint(x, y), DoublePoint(x, y), MouseButton::Left, type, 1, {},
                                MonotonicTime::Now());
    };
    auto &handler = fixture.Frame->GetEventHandler();
    (void)handler.HandleMousePressEvent(event(PlatformEvent::Type::MousePressed, 5, 7));
    (void)handler.HandleMouseReleaseEvent(event(PlatformEvent::Type::MouseReleased, 5, 7));
    RunNetworkingTasks();

    CHECK(Downcast<html::HTMLDialogElement>(fixture.ById(u8"d")).ReturnValue() == u8"5,7");
  }
}
