#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/DataTransfer.hpp"
#include "Krystal.Booey/DOM/DragEvent.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Editing/Markup.hpp"
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

    SECTION("The window losing focus cancels it")
    {
      view.SetFocused(false);
      CHECK_FALSE(view.IsDragging());
      CHECK(log.Text.ends_with("dragend@src"));
      view.SetFocused(true);
    }
  }
}
