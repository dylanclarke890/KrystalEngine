#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Events/EventListener.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.PAL/IWindow.hpp"
#include "Krystal.UI.Tests/ViewTestUtils.hpp"
#include "Krystal.UI/View.hpp"

namespace krys::ui::tests
{
  namespace
  {
    /// @brief A window that records what it's told about its accessibility tree.
    class AccessibilityWindow final : public pal::IWindow
    {
    private:
      string _title;

    public:
      pal::IAccessibilityTree *Tree {nullptr};
      bool Active {true};
      List<Pair<pal::AccessibilityNodeID, pal::AccessibilityEvent>> Events;
      List<utf8_string> TextChanges;
      List<pal::AccessibilityNodeID> Removed;

      KRYS_NODISCARD pal::WindowHandle GetWindowHandle() const noexcept override
      {
        return {};
      }
      void ProcessMessages() noexcept override
      {
      }
      void SetTitle(const string &title) noexcept override
      {
        _title = title;
      }
      KRYS_NODISCARD const string &GetTitle() const noexcept override
      {
        return _title;
      }
      KRYS_NODISCARD pal::WindowSize GetSize() const noexcept override
      {
        return {};
      }
      KRYS_NODISCARD Maybe<uint32> DisplayRefreshRate() const noexcept override
      {
        return null;
      }
      void Show() noexcept override
      {
      }
      void Hide() noexcept override
      {
      }
      void SetCallbacks(pal::WindowCallbacks &&) noexcept override
      {
      }
      void SetCursor(pal::CursorShape) noexcept override
      {
      }
      void SetCursorImage(Ref<pal::CursorImage>) noexcept override
      {
      }
      KRYS_NODISCARD pal::CursorShape GetCursor() const noexcept override
      {
        return pal::CursorShape::Arrow;
      }
      KRYS_NODISCARD pal::CursorImage *GetCursorImage() const noexcept override
      {
        return nullptr;
      }
      bool SetPointerLocked(bool) noexcept override
      {
        return false;
      }
      KRYS_NODISCARD bool IsPointerLocked() const noexcept override
      {
        return false;
      }
      void SetTextInputEnabled(bool) noexcept override
      {
      }
      KRYS_NODISCARD bool IsTextInputEnabled() const noexcept override
      {
        return false;
      }
      void SetTextInputRect(const pal::TextInputRect &) noexcept override
      {
      }
      void CompleteComposition() noexcept override
      {
      }
      void CancelComposition() noexcept override
      {
      }
      void SetDropOperation(Maybe<pal::DropOperation>) noexcept override
      {
      }

      void SetAccessibilityTree(pal::IAccessibilityTree *tree) noexcept override
      {
        Tree = tree;
      }
      KRYS_NODISCARD bool IsAccessibilityActive() const noexcept override
      {
        return Active;
      }
      void RaiseAccessibilityEvent(pal::AccessibilityNodeID node,
                                   pal::AccessibilityEvent event) noexcept override
      {
        Events.push_back({node, event});
      }
      void RaiseAccessibilityTextChange(pal::AccessibilityNodeID, pal::AccessibilityTextChange change, uint64,
                                        utf8_stringview text) noexcept override
      {
        TextChanges.push_back(utf8_string(change == pal::AccessibilityTextChange::Inserted ? u8"+" : u8"-")
                              + utf8_string(text));
      }
      void AccessibilityNodeRemoved(pal::AccessibilityNodeID node) noexcept override
      {
        Removed.push_back(node);
      }

      KRYS_NODISCARD bool HasEvent(pal::AccessibilityNodeID node,
                                   pal::AccessibilityEvent event) const noexcept
      {
        for (const auto &[eventNode, eventType] : Events)
        {
          if (eventNode == node && eventType == event)
          {
            return true;
          }
        }
        return false;
      }
    };

    /// @brief The first node under @p node (itself included) whose name is @p name; 0 if none.
    KRYS_NODISCARD pal::AccessibilityNodeID
      FindByName(pal::IAccessibilityTree &tree, pal::AccessibilityNodeID node, utf8_stringview name) noexcept
    {
      if (auto info = tree.Node(node); info && info->Name == name)
      {
        return node;
      }
      for (auto child : tree.Children(node))
      {
        if (auto found = FindByName(tree, child, name))
        {
          return found;
        }
      }
      return 0;
    }

    /// @brief The first node under @p node (itself included) with @p role; 0 if none.
    KRYS_NODISCARD pal::AccessibilityNodeID FindByRole(pal::IAccessibilityTree &tree,
                                                       pal::AccessibilityNodeID node,
                                                       pal::AccessibilityRole role) noexcept
    {
      if (auto info = tree.Node(node); info && info->Role == role)
      {
        return node;
      }
      for (auto child : tree.Children(node))
      {
        if (auto found = FindByRole(tree, child, role))
        {
          return found;
        }
      }
      return 0;
    }
  }

  TEST_CASE("View: accessibility", "[UI][Accessibility]")
  {
    View view({400, 300});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0; font: 16px sans-serif }</style>"
      u8"<button id=button style='width: 100px; height: 40px'>Go</button>"
      u8"<input id=field aria-label=Name value=Hello>"
      u8"<input id=box type=checkbox aria-label=Agree>"
      u8"<input id=range type=range min=0 max=10 value=4 step=2 aria-label=Volume>"
      u8"<select id=list size=2 aria-label=Fruit><option>Apple</option><option selected>Pear</option>"
      u8"</select><h2>Title</h2>");
    WaitForLoad(view);
    view.UpdateRendering();

    AccessibilityWindow window;
    view.AttachAccessibility(window, {0, 50});
    REQUIRE(window.Tree == view.Accessibility());
    auto &tree = *window.Tree;

    auto root = tree.Root();
    REQUIRE(root != 0);
    CHECK(tree.Parent(root) == 0);
    auto button = FindByName(tree, root, u8"Go");
    REQUIRE(button != 0);
    auto field = FindByName(tree, root, u8"Name");
    auto box = FindByName(tree, root, u8"Agree");
    auto range = FindByName(tree, root, u8"Volume");
    auto list = FindByName(tree, root, u8"Fruit");
    REQUIRE((field && box && range && list));

    SECTION("Nodes")
    {
      auto info = tree.Node(button);
      REQUIRE(info);
      CHECK(info->Role == pal::AccessibilityRole::Button);
      CHECK(info->AriaRole == u8"button");
      CHECK(info->States.Contains(pal::AccessibilityState::Focusable));
      CHECK(info->Actions.Contains(pal::AccessibilityAction::Press));
      // The view's top left is at the origin in the window.
      CHECK(info->Bounds.X == 0);
      CHECK(info->Bounds.Y == 50);
      CHECK(info->Bounds.Width == 100);
      CHECK(info->Bounds.Height == 40);

      auto fieldInfo = tree.Node(field);
      CHECK(fieldInfo->Role == pal::AccessibilityRole::Edit);
      CHECK(fieldInfo->Value == u8"Hello");
      CHECK(fieldInfo->States.Contains(pal::AccessibilityState::Editable));

      auto rangeInfo = tree.Node(range);
      CHECK(rangeInfo->Role == pal::AccessibilityRole::Slider);
      REQUIRE(rangeInfo->Range);
      CHECK(rangeInfo->Range->Value == 4);
      CHECK(rangeInfo->Range->Maximum == 10);
      CHECK(rangeInfo->Range->Step == 2);

      auto heading = FindByRole(tree, root, pal::AccessibilityRole::Heading);
      REQUIRE(heading != 0);
      CHECK(tree.Node(heading)->HeadingLevel == 2);

      CHECK(tree.SelectedChildren(list).size() == 1);
      CHECK_FALSE(tree.Node(0));
    }

    SECTION("Actions")
    {
      CHECK(tree.PerformAction(box, pal::AccessibilityAction::Press));
      CHECK(Downcast<boo::html::HTMLInputElement>(*view.Document().GetElementById(u8"box")).Checked());
      CHECK(tree.Node(box)->States.Contains(pal::AccessibilityState::Checked));

      CHECK(tree.PerformAction(field, pal::AccessibilityAction::Focus));
      CHECK(tree.Focus() == field);
      // The tree hears of the focus change once the page is laid out, on its own timer.
      view.UpdateRendering();
      boo::html::EventLoop::Current().RunUntilIdle();
      CHECK(window.HasEvent(field, pal::AccessibilityEvent::Focused));

      CHECK(tree.SetValue(field, u8"Changed"));
      CHECK(Downcast<boo::html::HTMLInputElement>(*view.Document().GetElementById(u8"field")).Value()
            == u8"Changed");

      CHECK(tree.SetRangeValue(range, 8));
      CHECK(tree.Node(range)->Range->Value == 8);
      CHECK(tree.PerformAction(range, pal::AccessibilityAction::Decrement));
      CHECK(tree.Node(range)->Range->Value == 6);
    }

    SECTION("Hit testing")
    {
      CHECK(tree.HitTest(10, 60) == button);
      // Above the view isn't the page.
      CHECK(tree.HitTest(10, 10) != button);
    }

    SECTION("Text")
    {
      CHECK(tree.Text(field) == u8"Hello");
      CHECK(tree.SetTextSelection(field, {1, 3}));
      auto selection = tree.TextSelection(field);
      REQUIRE(selection);
      CHECK(selection->Start == 1);
      CHECK(selection->Length == 3);
      auto bounds = tree.TextBounds(field, {0, 5});
      REQUIRE_FALSE(bounds.empty());
      CHECK(bounds.front().Y >= 50);
    }

    SECTION("Removed nodes")
    {
      view.Document().GetElementById(u8"button")->Remove();
      view.UpdateRendering();
      CHECK(std::ranges::find(window.Removed, button) != window.Removed.end());
      CHECK_FALSE(tree.Node(button));
    }

    SECTION("Detaching")
    {
      view.DetachAccessibility();
      CHECK(window.Tree == nullptr);
    }
  }
}
