#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/DataTransfer.hpp"
#include "Krystal.Booey/DOM/DragEvent.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Editing/Markup.hpp"
#include "Krystal.Booey/FileAPI/FileList.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListItems.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.UI.Tests/ViewTestUtils.hpp"
#include "Krystal.UI/View.hpp"
#include <string>

namespace krys::ui::tests
{
  namespace
  {
    constexpr utf8_stringview Page =
      u8"<!DOCTYPE html><style>body{margin:0;font:10px/20px sans-serif}div{height:40px}</style>"
      u8"<div id=src draggable=true>source</div><div id=dst>target</div><div id=e "
      u8"contenteditable>editable</div>";

    KRYS_NODISCARD std::string Narrow(utf8_stringview text) noexcept
    {
      return std::string(reinterpret_cast<const char *>(text.data()), text.size());
    }

    /// @brief Where the view draws the drag image, and how opaque, if it draws one.
    struct DrawnDragImage
    {
      boo::FloatRect Rect;
      float Alpha {1};
    };

    /// @brief The drag image @p view draws over its page: the last image buffer it draws.
    KRYS_NODISCARD Maybe<DrawnDragImage> DragImageIn(View &view) noexcept
    {
      Maybe<DrawnDragImage> drawn;
      float alpha = 1;
      auto displayList = view.Record();
      for (const auto &item : displayList->Items())
      {
        if (auto *setState = std::get_if<boo::display_list::SetState>(&item))
        {
          alpha = setState->State().Alpha();
        }
        else if (auto *draw = std::get_if<boo::display_list::DrawImageBuffer>(&item))
        {
          drawn = DrawnDragImage {draw->DestinationRect(), alpha};
        }
      }
      return drawn;
    }

    /// @brief The types of the drag events at @p view's document, with their targets' ids.
    struct DragLog
    {
      std::string Text;
      List<boo::dom::EventListenerHandle> Handles;

      explicit DragLog(View &view) noexcept
      {
        for (auto type : {u8"dragstart", u8"dragenter", u8"dragover", u8"dragleave", u8"drop", u8"dragend"})
        {
          Handles.push_back(view.Document().AddEventListener(
            type,
            [this](boo::dom::Event &event)
            {
              auto *node = DynamicDowncast<boo::dom::Node>(event.Target().get());
              auto *element = DynamicDowncast<boo::dom::Element>(node);
              if (!Text.empty())
              {
                Text += ' ';
              }
              Text +=
                Narrow(event.Type().View()) + "@"
                + (element ? Narrow(element->GetAttribute(u8"id").value_or(boo::dom::DOMString())) : "");
            }));
        }
      }
    };
  }

  TEST_CASE("View: drags in the page", "[UI][Drag]")
  {
    View view({400, 300});
    view.LoadHTML(utf8_string(Page));
    WaitForLoad(view);
    view.UpdateRendering();
    DragLog log(view);

    auto dragStart =
      view.Document().AddEventListener(u8"dragstart",
                                       [&](boo::dom::Event &event)
                                       {
                                         Downcast<boo::dom::DragEvent>(event).GetDataTransfer()->SetData(
                                           view.Document(), u8"text/plain", u8"payload");
                                       });

    // The press on the element and a move past the hysteresis start a drag, which the view runs: it enters
    // the page where the mouse is.
    view.MousePressed({10, 10}, pal::MouseButton::Left);
    view.MouseMoved({10, 30});
    REQUIRE(view.IsDragging());
    CHECK(log.Text == "dragstart@src dragenter@src");

    SECTION("Dropping on a target that takes it")
    {
      std::string dropped;
      auto accept = [](boo::dom::Event &event)
      {
        auto *node = DynamicDowncast<boo::dom::Node>(event.Target().get());
        auto *element = DynamicDowncast<boo::dom::Element>(node);
        if (element && element->GetAttribute(u8"id") == boo::dom::DOMString(u8"dst"))
        {
          Downcast<boo::dom::DragEvent>(event).GetDataTransfer()->SetDropEffect(u8"copy");
          event.PreventDefault();
        }
      };
      auto enter = view.Document().AddEventListener(u8"dragenter", accept);
      auto over = view.Document().AddEventListener(u8"dragover", accept);
      auto drop = view.Document().AddEventListener(
        u8"drop",
        [&](boo::dom::Event &event)
        {
          dropped = Narrow(
            Downcast<boo::dom::DragEvent>(event).GetDataTransfer()->GetData(view.Document(), u8"text/plain"));
          event.PreventDefault();
        });

      // Over the source, nothing takes it.
      CHECK(view.CurrentCursor() == pal::CursorShape::DragNone);
      view.MouseMoved({10, 60});
      CHECK(view.CurrentCursor() == pal::CursorShape::DragCopy);
      view.MouseMoved({12, 62});
      view.MouseReleased({12, 62}, pal::MouseButton::Left);
      CHECK_FALSE(view.IsDragging());
      CHECK(dropped == "payload");
      CHECK(log.Text
            == "dragstart@src dragenter@src dragenter@dst dragleave@src dragover@dst drop@dst dragend@src");
    }

    SECTION("Moving text into editable content")
    {
      view.MouseMoved({100, 90});
      CHECK(view.CurrentCursor() == pal::CursorShape::DragCopy);
      view.MouseReleased({100, 90}, pal::MouseButton::Left);
      CHECK(Narrow(boo::SerializeFragment(*view.Document().GetElementById(u8"e"),
                                          boo::SerializedNodes::SubtreesOfChildren))
            == "editablepayload");
    }

    SECTION("Leaving the view, and coming back")
    {
      view.MouseMoved({10, 60});
      view.MouseLeft();
      CHECK(view.CurrentCursor() == pal::CursorShape::DragNone);
      CHECK(log.Text.ends_with("dragleave@dst"));
      view.MouseMoved({10, 61});
      CHECK(log.Text.ends_with("dragenter@dst"));
      view.MouseReleased({10, 61}, pal::MouseButton::Left);
      CHECK_FALSE(view.IsDragging());
      CHECK(log.Text.ends_with("dragend@src"));
    }

    SECTION("Escape cancels it")
    {
      view.MouseMoved({10, 60});
      CHECK(view.KeyPressed(pal::Key::Escape));
      CHECK_FALSE(view.IsDragging());
      CHECK(log.Text.ends_with("dragleave@dst dragend@src"));

      // The release that follows doesn't start another, and the next move isn't a drag.
      view.MouseReleased({10, 60}, pal::MouseButton::Left);
      view.MouseMoved({10, 70});
      CHECK_FALSE(view.IsDragging());
    }

    SECTION("The image follows the cursor, faded, until the drop")
    {
      // The element was pressed 10px into it, so it's drawn 10px up and left of the cursor.
      auto drawn = DragImageIn(view);
      REQUIRE(drawn);
      CHECK(drawn->Rect == boo::FloatRect(0, 20, 400, 40));
      CHECK(drawn->Alpha == 0.75f);

      view.MouseMoved({50, 100});
      CHECK(view.NeedsPaint());
      drawn = DragImageIn(view);
      REQUIRE(drawn);
      CHECK(drawn->Rect == boo::FloatRect(40, 90, 400, 40));

      view.MouseReleased({50, 100}, pal::MouseButton::Left);
      CHECK_FALSE(DragImageIn(view));
    }

    SECTION("The window losing focus cancels it")
    {
      view.SetFocused(false);
      CHECK_FALSE(view.IsDragging());
      CHECK(log.Text.ends_with("dragend@src"));
      view.SetFocused(true);
    }
  }

  TEST_CASE("View: drags from other applications", "[UI][Drag]")
  {
    View view({400, 300});
    view.LoadHTML(utf8_string(Page));
    WaitForLoad(view);
    view.UpdateRendering();
    DragLog log(view);

    auto entered = [](float x, float y, pal::DragContent content,
                      pal::DropOperations operations = {pal::DropOperation::Copy, pal::DropOperation::Move})
    {
      pal::DragInput input;
      input.Action = pal::DragAction::Entered;
      input.ClientX = x;
      input.ClientY = y;
      input.AllowedOperations = operations;
      input.Content = krys::move(content);
      return input;
    };
    auto next = [](pal::DragAction action, const pal::DragInput &entered)
    {
      pal::DragInput input = entered;
      input.Action = action;
      input.Content = {};
      return input;
    };

    SECTION("Text dropped into editable content")
    {
      pal::DragContent content;
      content.Text = u8"dropped";
      auto input = entered(100, 90, krys::move(content));

      CHECK(view.SystemDragInput({100, 90}, input) == pal::DropOperation::Copy);
      CHECK(view.SystemDragInput({100, 91}, next(pal::DragAction::Moved, input)) == pal::DropOperation::Copy);
      CHECK(view.SystemDragInput({100, 91}, next(pal::DragAction::Dropped, input))
            == pal::DropOperation::Copy);
      CHECK(Narrow(boo::SerializeFragment(*view.Document().GetElementById(u8"e"),
                                          boo::SerializedNodes::SubtreesOfChildren))
            == "editabledropped");
      CHECK(log.Text == "dragenter@e dragover@e drop@e");
    }

    SECTION("The page reads what's dropped, and decides what the drop does")
    {
      std::string text, uriList, types;
      size_t files = 0;
      auto accept = [](boo::dom::Event &event)
      {
        auto *node = DynamicDowncast<boo::dom::Node>(event.Target().get());
        auto *element = DynamicDowncast<boo::dom::Element>(node);
        if (element && element->GetAttribute(u8"id") == boo::dom::DOMString(u8"dst"))
        {
          event.PreventDefault();
        }
      };
      auto enter = view.Document().AddEventListener(u8"dragenter", accept);
      auto over = view.Document().AddEventListener(u8"dragover", accept);
      auto drop = view.Document().AddEventListener(
        u8"drop",
        [&](boo::dom::Event &event)
        {
          auto &dataTransfer = *Downcast<boo::dom::DragEvent>(event).GetDataTransfer();
          text = Narrow(dataTransfer.GetData(view.Document(), u8"text/plain"));
          uriList = Narrow(dataTransfer.GetData(view.Document(), u8"text/uri-list"));
          for (auto &type : dataTransfer.Types(view.Document()))
          {
            types += (types.empty() ? "" : " ") + Narrow(type);
          }
          files = dataTransfer.Files(view.Document()).Length();
          event.PreventDefault();
        });

      SECTION("A link: the source's move is the default")
      {
        pal::DragContent content;
        content.Text = u8"a link";
        content.URL = u8"https://example.com/page";
        content.URLTitle = u8"Example";
        pal::DropOperations all {pal::DropOperation::Copy, pal::DropOperation::Move,
                                 pal::DropOperation::Link};
        auto input = entered(10, 50, krys::move(content), all);

        CHECK(view.SystemDragInput({10, 50}, input) == pal::DropOperation::Move);
        CHECK(view.SystemDragInput({10, 50}, next(pal::DragAction::Dropped, input))
              == pal::DropOperation::Move);
        CHECK(text == "a link");
        CHECK(uriList == "https://example.com/page");
        CHECK(types == "text/plain text/html text/uri-list");
        CHECK(files == 0);
      }

      SECTION("Files: never moved, as the page only reads them")
      {
        pal::DragContent content;
        content.Files = {u8"C:/Krystal/notes.txt", u8"C:/Krystal/image.png"};
        pal::DropOperations all {pal::DropOperation::Copy, pal::DropOperation::Move,
                                 pal::DropOperation::Link};
        auto input = entered(10, 50, krys::move(content), all);

        CHECK(view.SystemDragInput({10, 50}, input) == pal::DropOperation::Copy);
        CHECK(view.SystemDragInput({10, 50}, next(pal::DragAction::Dropped, input))
              == pal::DropOperation::Copy);
        CHECK(types == "Files text/uri-list");
        CHECK(files == 2);
      }
    }

    SECTION("Leaving the view, and the page")
    {
      pal::DragContent content;
      content.Text = u8"dropped";
      auto input = entered(10, 50, krys::move(content));

      CHECK_FALSE(view.SystemDragInput({10, 50}, input));
      // Off the page (over the host's own interface), it leaves it.
      CHECK_FALSE(view.SystemDragInput({10, -5}, next(pal::DragAction::Moved, input)));
      CHECK(log.Text == "dragenter@dst dragleave@dst");
      CHECK(view.SystemDragInput({100, 90}, next(pal::DragAction::Moved, input)) == pal::DropOperation::Copy);
      CHECK_FALSE(view.SystemDragInput({100, 90}, next(pal::DragAction::Left, input)));
      CHECK(log.Text.ends_with("dragenter@e dragleave@e"));

      // Input after it left, until another enters, is ignored.
      CHECK_FALSE(view.SystemDragInput({100, 90}, next(pal::DragAction::Dropped, input)));
      CHECK(log.Text.ends_with("dragleave@e"));
      CHECK(Narrow(boo::SerializeFragment(*view.Document().GetElementById(u8"e"),
                                          boo::SerializedNodes::SubtreesOfChildren))
            == "editable");
    }
  }

  TEST_CASE("View: the drag image is painted under the cursor", "[UI][GPU][Drag]")
  {
    constexpr uint32 Width = 400;
    constexpr uint32 Height = 300;
    auto device = gfx::tests::CreateTestDevice();
    auto renderer = gfx::Renderer2D::Create(*device);
    REQUIRE(renderer.has_value());

    View view({static_cast<int32>(Width), static_cast<int32>(Height)});
    view.LoadHTML(u8"<!DOCTYPE html><style>body{margin:0;background:white}</style>"
                  u8"<div draggable=true style=\"width:100px;height:40px;background:#00ff00\"></div>");
    WaitForLoad(view);
    view.UpdateRendering();
    view.MousePressed({10, 10}, pal::MouseButton::Left);
    view.MouseMoved({10, 30});
    REQUIRE(view.IsDragging());
    view.MouseMoved({200, 150});

    auto target =
      device->CreateTexture({.Format = gfx::TextureFormat::RGBA8Unorm,
                             .Width = Width,
                             .Height = Height,
                             .Usage = gfx::TextureUsage::RenderTarget | gfx::TextureUsage::Sampled,
                             .DebugName = u8"Drag"});
    REQUIRE(target.has_value());
    device->BeginFrame();
    auto &commands = device->GetCommandBuffer();
    (*renderer)->Begin(commands, {.Texture = *target,
                                  .Width = Width,
                                  .Height = Height,
                                  .Format = gfx::TextureFormat::RGBA8Unorm,
                                  .ClearColour = Array<float, 4> {0.f, 0.f, 0.f, 0.f}});
    view.Paint(**renderer);
    (*renderer)->End();
    device->Submit(commands);
    device->EndFrame();

    List<Array<uint8, 4>> pixels(static_cast<size_t>(Width) * Height);
    device->ReadTexture(*target, gfx::TextureRegion::Whole2D(Width, Height),
                        Span<byte>(reinterpret_cast<byte *>(pixels.data()), pixels.size() * 4u));
    device->Destroy(*target);
    auto at = [&](uint32 x, uint32 y)
    {
      return pixels[static_cast<size_t>(y) * Width + x];
    };

    // The image is the element at three quarters opacity over the page, 10px up and left of the cursor.
    auto inImage = at(230, 160);
    CHECK(inImage[0] >= 60);
    CHECK(inImage[0] <= 68);
    CHECK(inImage[1] == 255);
    CHECK(inImage[2] >= 60);
    CHECK(inImage[2] <= 68);
    // Beyond it is the page, and the element stays where it is.
    CHECK(at(300, 200) == Array<uint8, 4> {255, 255, 255, 255});
    CHECK(at(50, 20) == Array<uint8, 4> {0, 255, 0, 255});

    view.MouseReleased({200, 150}, pal::MouseButton::Left);
  }
}
