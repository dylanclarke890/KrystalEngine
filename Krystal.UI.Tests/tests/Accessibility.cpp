#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Events/EventListener.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.PAL/IWindow.hpp"
#include "Krystal.UI.Tests/ViewTestUtils.hpp"
#include "Krystal.UI/View.hpp"

#ifdef _WIN32
  #include "Krystal.PAL/Platform.hpp"
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <ole2.h>
  #include <uiautomation.h>
  #include <windows.h>
  // Windows' macro would hide Krystal.PAL's function.
  #undef CreateWindow
#endif

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

  TEST_CASE("View: accessibility of text lines", "[UI][Accessibility]")
  {
    View view({400, 300});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0; font: 16px monospace } * { width: 100px }</style>"
      u8"<textarea id=area aria-label=Notes>aaaa bbbb cccc dddd\neeee</textarea>"
      u8"<p id=para aria-label=Para>aaaa bbbb cccc dddd</p>");
    WaitForLoad(view);
    view.UpdateRendering();

    AccessibilityWindow window;
    view.AttachAccessibility(window, {0, 0});
    auto &tree = *window.Tree;
    auto root = tree.Root();

    // Wrapped lines start lines, as line breaks do.
    auto area = FindByName(tree, root, u8"Notes");
    REQUIRE(area != 0);
    auto text = tree.Text(area);
    auto starts = tree.TextLineStarts(area);
    REQUIRE(starts.size() >= 2);
    CHECK(std::ranges::is_sorted(starts));
    CHECK(std::ranges::find(starts, text.find(u8'e')) != starts.end());
    CHECK(starts.front() < text.find(u8'e'));
    CHECK(text[starts.front() - 1] == u8' ');

    // A paragraph's lines too.
    auto para = FindByRole(tree, root, pal::AccessibilityRole::Paragraph);
    REQUIRE(para != 0);
    auto paraStarts = tree.TextLineStarts(para);
    REQUIRE_FALSE(paraStarts.empty());
    CHECK(tree.Text(para)[paraStarts.front() - 1] == u8' ');
  }

  TEST_CASE("View: accessibility of overlays", "[UI][Accessibility]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0; font: 16px sans-serif }</style>"
                  u8"<select id=menu aria-label=Fruit><option>Apple</option><option>Plum</option></select>");
    WaitForLoad(view);
    view.UpdateRendering();

    AccessibilityWindow window;
    view.AttachAccessibility(window, {0, 50});
    auto &tree = *window.Tree;
    auto root = tree.Root();
    auto menu = FindByName(tree, root, u8"Fruit");
    REQUIRE(menu != 0);
    auto pageChildren = tree.Children(root).size();
    CHECK(FindByName(tree, root, u8"Plum") == 0);

    // Opening the menu list's popup adds its page as the root's last child.
    REQUIRE(tree.PerformAction(menu, pal::AccessibilityAction::Press));
    auto *overlay = view.ShownOverlay();
    REQUIRE(overlay);
    view.UpdateRendering();
    auto children = tree.Children(root);
    REQUIRE(children.size() == pageChildren + 1);
    auto popup = children.back();
    CHECK(tree.Parent(popup) == root);
    CHECK(window.HasEvent(root, pal::AccessibilityEvent::ChildrenChanged));
    CHECK(window.HasEvent(popup, pal::AccessibilityEvent::MenuOpened));
    auto plum = FindByName(tree, popup, u8"Plum");
    REQUIRE(plum != 0);
    CHECK(tree.Parent(plum) != 0);

    // Its nodes are where the overlay is in the window, and hit testing finds them there.
    auto bounds = tree.Node(plum)->Bounds;
    CHECK(bounds.Y >= static_cast<float>(overlay->Rect().Y() + 50));
    auto hit = tree.HitTest(bounds.X + bounds.Width / 2, bounds.Y + bounds.Height / 2);
    CHECK((hit == plum || tree.Parent(hit) == plum));

    // Closing it removes them.
    overlay->Hide();
    CHECK_FALSE(view.ShownOverlay());
    CHECK(std::ranges::find(window.Removed, popup) != window.Removed.end());
    CHECK(tree.Children(root).size() == pageChildren);
    CHECK_FALSE(tree.Node(plum));
    CHECK_FALSE(tree.Node(popup));
  }
#ifdef _WIN32
  namespace
  {
    /// @brief Releases a COM interface when it goes.
    template <typename T>
    struct ComRef
    {
      T *Pointer {nullptr};

      ComRef() noexcept = default;
      ComRef(const ComRef &) = delete;
      ComRef &operator=(const ComRef &) = delete;
      ~ComRef() noexcept
      {
        if (Pointer)
        {
          Pointer->Release();
        }
      }

      T *operator->() const noexcept
      {
        return Pointer;
      }
      T **operator&() noexcept
      {
        return &Pointer;
      }
      explicit operator bool() const noexcept
      {
        return Pointer != nullptr;
      }
    };

    /// @brief The first element under @p root named @p name.
    void FindByName(IUIAutomation &automation, IUIAutomationElement &root, const wchar_t *name,
                    ComRef<IUIAutomationElement> &found) noexcept
    {
      VARIANT value;
      value.vt = VT_BSTR;
      value.bstrVal = ::SysAllocString(name);
      ComRef<IUIAutomationCondition> condition;
      (void)automation.CreatePropertyCondition(UIA_NamePropertyId, value, &condition);
      ::VariantClear(&value);
      if (condition)
      {
        (void)root.FindFirst(TreeScope_Descendants, condition.Pointer, &found);
      }
    }
  }

  TEST_CASE("View: UI Automation", "[UI][Accessibility]")
  {
    pal::Initialise();
    auto window = pal::CreateWindow({.Title = "Accessibility", .Size = {400, 300}, .Visible = false});
    REQUIRE(window.has_value());
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 }</style>"
                  u8"<button id=button style='width: 100px; height: 40px'>Go</button>"
                  u8"<input id=field aria-label=Name value=Hello>"
                  u8"<input id=box type=checkbox aria-label=Agree>"
                  u8"<textarea aria-label=Notes style='width: 100px; font: 16px monospace'>"
                  u8"aaaa bbbb cccc</textarea>");
    WaitForLoad(view);
    view.UpdateRendering();
    int32 clicks = 0;
    struct ClickCounter final : boo::dom::EventListenerCallback
    {
      int32 &Clicks;
      explicit ClickCounter(int32 &clicks) noexcept : Clicks(clicks)
      {
      }
      void HandleEvent(boo::dom::Event &) noexcept override
      {
        ++Clicks;
      }
    };
    view.Document()
      .GetElementById(u8"button")
      ->AddEventListener(boo::dom::DOMStringAtom(u8"click"),
                         Ref<boo::dom::EventListenerCallback>(CreateRef<ClickCounter>(clicks)), false);
    view.AttachAccessibility(**window);

    auto handle = (*window)->GetWindowHandle().As<HWND>();
    {
      ComRef<IUIAutomation> automation;
      REQUIRE(SUCCEEDED(::CoCreateInstance(__uuidof(CUIAutomation), nullptr, CLSCTX_INPROC_SERVER,
                                           __uuidof(IUIAutomation), reinterpret_cast<void **>(&automation))));
      ComRef<IUIAutomationElement> root;
      REQUIRE(SUCCEEDED(automation->ElementFromHandle(handle, &root)));
      REQUIRE(root);

      ComRef<IUIAutomationElement> button;
      FindByName(*automation.Pointer, *root.Pointer, L"Go", button);
      REQUIRE(button);
      CONTROLTYPEID type = 0;
      CHECK(SUCCEEDED(button->get_CurrentControlType(&type)));
      CHECK(type == UIA_ButtonControlTypeId);
      ComRef<IUIAutomationInvokePattern> invoke;
      REQUIRE(SUCCEEDED(button->GetCurrentPatternAs(UIA_InvokePatternId, __uuidof(IUIAutomationInvokePattern),
                                                    reinterpret_cast<void **>(&invoke))));
      REQUIRE(invoke);
      CHECK(SUCCEEDED(invoke->Invoke()));
      CHECK(clicks == 1);

      ComRef<IUIAutomationElement> field;
      FindByName(*automation.Pointer, *root.Pointer, L"Name", field);
      REQUIRE(field);
      ComRef<IUIAutomationValuePattern> value;
      REQUIRE(SUCCEEDED(field->GetCurrentPatternAs(UIA_ValuePatternId, __uuidof(IUIAutomationValuePattern),
                                                   reinterpret_cast<void **>(&value))));
      REQUIRE(value);
      BSTR text = nullptr;
      CHECK(SUCCEEDED(value->get_CurrentValue(&text)));
      CHECK(std::wstring(text ? text : L"") == L"Hello");
      ::SysFreeString(text);
      BSTR changed = ::SysAllocString(L"Changed");
      CHECK(SUCCEEDED(value->SetValue(changed)));
      ::SysFreeString(changed);
      CHECK(Downcast<boo::html::HTMLInputElement>(*view.Document().GetElementById(u8"field")).Value()
            == u8"Changed");

      ComRef<IUIAutomationTextPattern> textPattern;
      REQUIRE(SUCCEEDED(field->GetCurrentPatternAs(UIA_TextPatternId, __uuidof(IUIAutomationTextPattern),
                                                   reinterpret_cast<void **>(&textPattern))));
      REQUIRE(textPattern);
      ComRef<IUIAutomationTextRange> range;
      REQUIRE(SUCCEEDED(textPattern->get_DocumentRange(&range)));
      BSTR all = nullptr;
      CHECK(SUCCEEDED(range->GetText(-1, &all)));
      CHECK(std::wstring(all ? all : L"") == L"Changed");
      ::SysFreeString(all);

      // The line unit follows wrapped lines.
      ComRef<IUIAutomationElement> notes;
      FindByName(*automation.Pointer, *root.Pointer, L"Notes", notes);
      REQUIRE(notes);
      ComRef<IUIAutomationTextPattern> notesText;
      REQUIRE(SUCCEEDED(notes->GetCurrentPatternAs(UIA_TextPatternId, __uuidof(IUIAutomationTextPattern),
                                                   reinterpret_cast<void **>(&notesText))));
      REQUIRE(notesText);
      ComRef<IUIAutomationTextRange> line;
      REQUIRE(SUCCEEDED(notesText->get_DocumentRange(&line)));
      CHECK(SUCCEEDED(line->MoveEndpointByRange(TextPatternRangeEndpoint_End, line.Pointer,
                                                TextPatternRangeEndpoint_Start)));
      CHECK(SUCCEEDED(line->ExpandToEnclosingUnit(TextUnit_Line)));
      BSTR firstLine = nullptr;
      CHECK(SUCCEEDED(line->GetText(-1, &firstLine)));
      CHECK(std::wstring(firstLine ? firstLine : L"") == L"aaaa bbbb ");
      ::SysFreeString(firstLine);

      ComRef<IUIAutomationElement> box;
      FindByName(*automation.Pointer, *root.Pointer, L"Agree", box);
      REQUIRE(box);
      ComRef<IUIAutomationTogglePattern> toggle;
      REQUIRE(SUCCEEDED(box->GetCurrentPatternAs(UIA_TogglePatternId, __uuidof(IUIAutomationTogglePattern),
                                                 reinterpret_cast<void **>(&toggle))));
      REQUIRE(toggle);
      CHECK(SUCCEEDED(toggle->Toggle()));
      ToggleState state = ToggleState_Off;
      CHECK(SUCCEEDED(toggle->get_CurrentToggleState(&state)));
      CHECK(state == ToggleState_On);
    }
    view.DetachAccessibility();
    window->reset();
    pal::Shutdown();
  }
#endif
}
