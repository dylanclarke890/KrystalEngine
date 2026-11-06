#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/DOM/Collections/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/EventTarget.hpp"
#include "Krystal.Booey/HTML/Collections/HTMLOptionsCollection.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Platform/Events/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/Events/WindowsKeyboardCodes.hpp"

namespace krys::boo::tests
{
  using html::HTMLOptionElement;
  using html::HTMLSelectElement;

  namespace
  {
    KRYS_NODISCARD HTMLSelectElement &SelectById(EditorFixture &fixture, utf8_stringview id) noexcept
    {
      return Downcast<HTMLSelectElement>(fixture.ById(id));
    }

    KRYS_NODISCARD HTMLOptionElement &OptionById(EditorFixture &fixture, utf8_stringview id) noexcept
    {
      return Downcast<HTMLOptionElement>(fixture.ById(id));
    }

    /// @brief The IDs of the select's list items, in order.
    KRYS_NODISCARD utf8_string ListItemIds(const HTMLSelectElement &select) noexcept
    {
      utf8_string ids;
      for (auto &item : select.ListItems())
      {
        if (!ids.empty())
        {
          ids += u8" ";
        }
        ids += item->GetAttribute(u8"id").value_or(u8"?");
      }
      return ids;
    }

    /// @brief Presses the key @p key (with the key identifier @p identifier) in the frame, as the host does.
    bool PressKey(EditorFixture &fixture, utf8_string key, utf8_string identifier, int32 keyCode) noexcept
    {
      return fixture.Frame->GetEventHandler().KeyEvent(
        PlatformKeyboardEvent(PlatformEvent::Type::RawKeyDown, u8"", u8"", key, key, krys::move(identifier),
                              keyCode, false, false, false, {}, {}));
    }

    /// @brief Types the character @p c in the frame, as the host does after its key down.
    bool TypeChar(EditorFixture &fixture, char8_t c) noexcept
    {
      utf8_string text(1uz, c);
      return fixture.Frame->GetEventHandler().KeyEvent(
        PlatformKeyboardEvent(PlatformEvent::Type::Char, text, text, text, u8"", u8"", static_cast<int32>(c),
                              false, false, false, {}, {}));
    }

    /// @brief Counts the events of a type fired at a target.
    struct EventCounter final : dom::EventListenerCallback
    {
      int32 Count {0};

      void HandleEvent(dom::Event &) noexcept override
      {
        ++Count;
      }
    };

    /// @brief A chrome client counting the reports of a focused select's options changing.
    struct SelectChromeClient final : ChromeClient
    {
      int32 *OptionChanges {nullptr};

      explicit SelectChromeClient(int32 &optionChanges) noexcept : OptionChanges(&optionChanges)
      {
      }

      void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept final
      {
      }

      void FocusedSelectElementDidChangeOptions(const HTMLSelectElement &) noexcept final
      {
        ++*OptionChanges;
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "HTMLSelectElement: the chrome client hears of a focused select's options changing",
                   "[HTML][HTMLSelectElement]")
  {
    int32 optionChanges = 0;
    EditorFixture fixture(u8"<select id=s><option>a</select><select id=other><option>b</select>",
                          CreateUnique<SelectChromeClient>(optionChanges));
    auto &select = SelectById(fixture, u8"s");
    auto addOption = [&]
    {
      auto option = fixture.Document->CreateElement(u8"option").ReleaseValue();
      REQUIRE_FALSE(select.AppendChild(*option).HasException());
    };

    SECTION("not when it isn't focused")
    {
      addOption();
      CHECK(optionChanges == 0);
    }

    SECTION("when it's focused")
    {
      select.Focus();
      REQUIRE(fixture.Document->FocusedElement() == &select);
      addOption();
      CHECK(optionChanges > 0);
    }
  }

  TEST_CASE_METHOD(
    css::tests::ValueFixture, "HTMLSelectElement: list items",
    "[HTML][HTMLSelectElement][html:concept-select-option-list][html:dom-select-type]"
    "[html:dom-select-length][html:dom-select-item][html:dom-select-nameditem][html:dom-select-add]")
  {
    EditorFixture fixture(u8"<select id=s>"
                          u8"<option id=a>A</option>"
                          u8"<optgroup id=g label=' Group  one '><option id=b>B</option><option id=c "
                          u8"disabled>C</option></optgroup>"
                          u8"<hr id=hr>"
                          u8"<div id=d><option id=e>E</option></div>"
                          u8"<datalist id=dl><option id=x>X</option></datalist>"
                          u8"</select>");

    auto &select = SelectById(fixture, u8"s");

    // Options, optgroups and hrs are list items, wherever they are in the select, except in datalists.
    CHECK(ListItemIds(select) == u8"a g b c hr e");
    CHECK(select.Length() == 4u);
    CHECK(select.Options()->Length() == 4uz);
    CHECK(select.Item(1u) == &OptionById(fixture, u8"b"));
    CHECK(select.NamedItem(u8"e") == &OptionById(fixture, u8"e"));
    CHECK(OptionById(fixture, u8"e").Index() == 3);
    CHECK(OptionById(fixture, u8"x").OwnerSelectElement() == nullptr);

    // An option in a disabled optgroup is disabled, and its label is indented.
    CHECK(OptionById(fixture, u8"c").IsDisabledFormControl());
    CHECK(OptionById(fixture, u8"b").TextIndentedToRespectGroupLabel() == u8"    B");
    CHECK(Downcast<html::HTMLOptGroupElement>(fixture.ById(u8"g")).GroupLabelText() == u8"Group one");
    REQUIRE_FALSE(fixture.ById(u8"g").SetAttribute(u8"disabled", u8"").HasException());
    CHECK(OptionById(fixture, u8"b").IsDisabledFormControl());

    // A single select selects its first option.
    CHECK(select.SelectedIndex() == 0);
    CHECK(select.Value() == u8"A");
    CHECK(select.FormControlType() == u8"select-one");
    CHECK(select.UsesMenuList());

    SECTION("Removing and inserting items")
    {
      Ref<HTMLOptionElement> e = ShareRef(OptionById(fixture, u8"e"));
      (void)fixture.ById(u8"d").Remove();
      CHECK(ListItemIds(select) == u8"a g b c hr");
      CHECK(e->OwnerSelectElement() == nullptr);

      // An inserted selected option is selected.
      auto option = CreateRef<HTMLOptionElement>(*fixture.Document);
      option->DefaultSelected(true);
      REQUIRE_FALSE(select.Add(RefPtr<HTMLOptionElement>(option), HTMLSelectElement::HTMLElementOrInt {0})
                      .HasException());
      CHECK(select.SelectedIndex() == 0);
      CHECK(select.Item(0u) == option.get());
      CHECK_FALSE(OptionById(fixture, u8"a").Selected());

      select.Remove(0);
      CHECK(select.Length() == 3u);
      CHECK(select.SelectedIndex() == 0);
    }

    SECTION("The length")
    {
      REQUIRE_FALSE(select.Length(6u).HasException());
      CHECK(select.Length() == 6u);
      REQUIRE_FALSE(select.Length(1u).HasException());
      CHECK(ListItemIds(select) == u8"a g hr");
    }

    SECTION("Setting an item")
    {
      auto option = CreateRef<HTMLOptionElement>(*fixture.Document);
      REQUIRE_FALSE(option->SetAttribute(u8"id", u8"n").HasException());
      // The option replaces the one at the index, and goes before the next one, which is the end here.
      REQUIRE_FALSE(select.SetItem(3u, option.get()).HasException());
      CHECK(ListItemIds(select) == u8"a g b c hr n");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "HTMLSelectElement: the options collection's add(), remove(), length and indexed setter",
                   "[HTML][HTMLSelectElement][html:dom-htmloptionscollection-add]"
                   "[html:htmloptionscollection-remove-an-option][html:dom-htmloptionscollection-remove]"
                   "[html:dom-htmloptionscollection-length][html:append-new-option-elements]")
  {
    EditorFixture fixture(
      u8"<select id=s><option id=a>A</option><optgroup id=g><option id=b>B</option></optgroup>"
      u8"<option id=c>C</option></select><div id=outside><option id=o></option></div>");
    auto &select = SelectById(fixture, u8"s");
    auto newOption = [&](utf8_stringview id)
    {
      auto option = CreateRef<HTMLOptionElement>(*fixture.Document);
      REQUIRE_FALSE(option->SetAttribute(u8"id", dom::DOMString(id)).HasException());
      return option;
    };
    auto before = [&](utf8_stringview id)
    {
      return HTMLSelectElement::HTMLElementOrInt {ShareRefPtr<html::HTMLElement>(&OptionById(fixture, id))};
    };

    SECTION("add() inserts into the parent of the element or indexth option it goes before")
    {
      auto n = newOption(u8"n");
      REQUIRE_FALSE(select.Add(RefPtr<HTMLOptionElement>(n), before(u8"b")).HasException());
      CHECK(ListItemIds(select) == u8"a g n b c");
      CHECK(n->ParentNode() == &fixture.ById(u8"g"));

      REQUIRE_FALSE(
        select.Add(RefPtr<HTMLOptionElement>(newOption(u8"m")), HTMLSelectElement::HTMLElementOrInt {2})
          .HasException());
      CHECK(ListItemIds(select) == u8"a g n m b c");
      REQUIRE_FALSE(
        select.Add(RefPtr<HTMLOptionElement>(newOption(u8"z")), HTMLSelectElement::HTMLElementOrInt {-1})
          .HasException());
      REQUIRE_FALSE(
        select.Add(RefPtr<HTMLOptionElement>(newOption(u8"y")), HTMLSelectElement::HTMLElementOrInt {99})
          .HasException());
      CHECK(ListItemIds(select) == u8"a g n m b c z y");
    }

    SECTION("add() throws for an element outside the select, and does nothing to add an option before itself")
    {
      auto result = select.Add(RefPtr<HTMLOptionElement>(newOption(u8"n")), before(u8"o"));
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == dom::ExceptionCode::NotFoundError);
      REQUIRE_FALSE(select.Add(ShareRefPtr(&OptionById(fixture, u8"c")), before(u8"c")).HasException());
      CHECK(ListItemIds(select) == u8"a g b c");
    }

    SECTION("remove() removes the indexth option from its parent, and ignores other indices")
    {
      select.Remove(-1);
      select.Remove(3);
      CHECK(select.Length() == 3u);
      select.Remove(1);
      CHECK(ListItemIds(select) == u8"a g c");
    }

    SECTION("Growing the length appends new options, up to 100,000")
    {
      REQUIRE_FALSE(select.Length(5u).HasException());
      CHECK(select.Length() == 5u);
      CHECK(select.Item(4u)->ParentNode() == &select);
      CHECK(select.Item(4u)->LocalName() == u8"option");
      REQUIRE_FALSE(select.Length(100'001u).HasException());
      CHECK(select.Length() == 5u);
    }

    SECTION("Setting an indexed option replaces the one there in its parent, or appends past the end")
    {
      auto n = newOption(u8"n");
      REQUIRE_FALSE(select.SetItem(1u, n.get()).HasException());
      CHECK(ListItemIds(select) == u8"a g n c");
      CHECK(n->ParentNode() == &fixture.ById(u8"g"));

      REQUIRE_FALSE(select.SetItem(5u, newOption(u8"m").get()).HasException());
      CHECK(select.Length() == 6u);
      CHECK(select.Item(5u)->GetAttribute(u8"id") == u8"m");

      REQUIRE_FALSE(select.SetItem(0u, nullptr).HasException());
      CHECK(select.Length() == 5u);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLSelectElement: selection",
                   "[HTML][HTMLSelectElement][html:concept-form-reset-control][html:dom-select-type]"
                   "[html:reset-a-select's-selectedness]")
  {
    EditorFixture fixture(u8"<select id=s>"
                          u8"<option id=a value=1>A</option>"
                          u8"<option id=b value=2 selected>B</option>"
                          u8"<option id=c value=3>C</option>"
                          u8"</select>"
                          u8"<select id=m multiple>"
                          u8"<option id=ma selected>A</option>"
                          u8"<option id=mb>B</option>"
                          u8"<option id=mc selected>C</option>"
                          u8"</select>");

    auto &select = SelectById(fixture, u8"s");
    CHECK(select.SelectedIndex() == 1);
    CHECK(select.Value() == u8"2");

    select.Value(u8"3");
    CHECK(select.SelectedIndex() == 2);
    CHECK_FALSE(OptionById(fixture, u8"b").Selected());

    select.SelectedIndex(-1);
    CHECK(select.SelectedIndex() == -1);
    CHECK(select.Value().empty());

    // Selecting an option deselects the others.
    OptionById(fixture, u8"a").Selected(true);
    CHECK(select.SelectedIndex() == 0);

    // A clean option's selectedness follows its selected attribute; a dirty one's doesn't. Setting the value
    // made c dirty.
    REQUIRE_FALSE(OptionById(fixture, u8"c").SetAttribute(u8"selected", u8"").HasException());
    CHECK(select.SelectedIndex() == 0);
    OptionById(fixture, u8"b").RemoveAttribute(u8"selected");
    REQUIRE_FALSE(OptionById(fixture, u8"b").SetAttribute(u8"selected", u8"").HasException());
    CHECK(select.SelectedIndex() == 1);
    REQUIRE_FALSE(OptionById(fixture, u8"a").SetAttribute(u8"selected", u8"").HasException());
    CHECK(select.SelectedIndex() == 1);

    // Resetting selects the last option with the attribute, and cleans the options.
    select.Reset();
    CHECK(select.SelectedIndex() == 2);
    OptionById(fixture, u8"a").RemoveAttribute(u8"selected");
    REQUIRE_FALSE(OptionById(fixture, u8"a").SetAttribute(u8"selected", u8"").HasException());
    CHECK(select.SelectedIndex() == 0);

    auto &multiple = SelectById(fixture, u8"m");
    CHECK(multiple.FormControlType() == u8"select-multiple");
    CHECK_FALSE(multiple.UsesMenuList());
    CHECK(multiple.SelectedIndex() == 0);
    CHECK(multiple.SelectedOptions()->Length() == 2uz);
    OptionById(fixture, u8"mb").Selected(true);
    CHECK(multiple.SelectedOptions()->Length() == 3uz);

    // Dropping multiple keeps the first selected option.
    multiple.Multiple(false);
    CHECK(multiple.SelectedOptions()->Length() == 1uz);
    CHECK(multiple.SelectedIndex() == 0);

    // A list box (a size above one) doesn't select an option of its own.
    select.Size(4u);
    CHECK_FALSE(select.UsesMenuList());
    select.SelectedIndex(-1);
    CHECK(select.SelectedIndex() == -1);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "HTMLSelectElement: the selectedness setting algorithm selects the first enabled option",
                   "[HTML][HTMLSelectElement][html:selectedness-setting-algorithm][html:ask-for-a-reset]"
                   "[html:reset-a-select's-selectedness][html:concept-select-size]")
  {
    EditorFixture fixture(
      u8"<select id=s><option id=a disabled>A</option><option id=b>B</option><option id=c>C</option></select>"
      u8"<select id=z size=0><option>A</option></select>"
      u8"<select id=l size=4><option>A</option><option>B</option></select>");
    auto &select = SelectById(fixture, u8"s");
    CHECK(select.SelectedIndex() == 1);

    // Removing the selected option selects the first enabled one, and with none, none.
    OptionById(fixture, u8"c").Selected(true);
    (void)OptionById(fixture, u8"c").Remove();
    CHECK(select.SelectedIndex() == 1);
    (void)OptionById(fixture, u8"b").Remove();
    CHECK(select.SelectedIndex() == -1);

    // A size of 0 is the default display size, as in browsers.
    CHECK(SelectById(fixture, u8"z").SelectedIndex() == 0);

    // A list box selects nothing of its own, until its display size is 1.
    auto &listBox = SelectById(fixture, u8"l");
    CHECK(listBox.SelectedIndex() == -1);
    listBox.Size(1u);
    CHECK(listBox.SelectedIndex() == 0);
  }

  TEST_CASE_METHOD(
    css::tests::ValueFixture,
    "HTMLSelectElement: the list of options leaves out what's in options, selects and nested optgroups",
    "[HTML][HTMLSelectElement][html:concept-select-option-list][html:dom-select-remove]")
  {
    EditorFixture fixture(u8"<select id=s><option id=a>A</option></select>");
    auto &select = SelectById(fixture, u8"s");
    auto &document = *fixture.Document;

    auto inOption = CreateRef<HTMLOptionElement>(document);
    REQUIRE_FALSE(OptionById(fixture, u8"a").AppendChild(*inOption).HasException());
    auto outer = document.CreateElement(u8"optgroup").ReleaseValue();
    auto inner = document.CreateElement(u8"optgroup").ReleaseValue();
    REQUIRE_FALSE(inner->AppendChild(*CreateRef<HTMLOptionElement>(document)).HasException());
    REQUIRE_FALSE(outer->AppendChild(*inner).HasException());
    REQUIRE_FALSE(select.AppendChild(*outer).HasException());
    auto innerSelect = document.CreateElement(u8"select").ReleaseValue();
    REQUIRE_FALSE(innerSelect->AppendChild(*CreateRef<HTMLOptionElement>(document)).HasException());
    REQUIRE_FALSE(select.AppendChild(*innerSelect).HasException());
    CHECK(select.Length() == 1u);

    // remove() with no arguments removes the select.
    Ref<HTMLSelectElement> keep = ShareRef(select);
    (void)select.Remove();
    CHECK_FALSE(keep->ParentNode());
  }

  TEST_CASE_METHOD(
    css::tests::ValueFixture,
    "HTMLSelectElement: selectedIndex and value select the matching option, or none, and make it dirty",
    "[HTML][HTMLSelectElement][html:set-the-selected-index][html:dom-select-selectedindex]"
    "[html:dom-select-value]")
  {
    EditorFixture fixture(u8"<select id=s><option id=a>A</option><option id=b selected>B</option></select>");
    auto &select = SelectById(fixture, u8"s");
    select.SelectedIndex(5);
    CHECK(select.SelectedIndex() == -1);
    select.Value(u8"A");
    CHECK(select.SelectedIndex() == 0);
    select.Value(u8"nothing");
    CHECK(select.SelectedIndex() == -1);

    // A dirty option doesn't follow its selected attribute.
    select.SelectedIndex(1);
    OptionById(fixture, u8"b").RemoveAttribute(u8"selected");
    CHECK(select.SelectedIndex() == 1);
  }

  TEST_CASE_METHOD(
    css::tests::ValueFixture,
    "HTMLSelectElement: the placeholder label option is the first, empty, option outside optgroups",
    "[HTML][HTMLSelectElement][html:placeholder-label-option]")
  {
    EditorFixture fixture(
      u8"<select id=r required><option value=''>-</option><option>B</option></select>"
      u8"<select id=hr required><hr><option value=''>-</option></select>"
      u8"<select id=g required><optgroup><option value=''>-</option></optgroup></select>");
    auto &select = SelectById(fixture, u8"r");
    CHECK(select.ValueMissing());
    select.SelectedIndex(1);
    CHECK_FALSE(select.ValueMissing());
    CHECK(SelectById(fixture, u8"hr").ValueMissing());
    CHECK_FALSE(SelectById(fixture, u8"g").ValueMissing());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "HTMLSelectElement: options and selectedOptions are the same collections each time",
                   "[HTML][HTMLSelectElement][html:dom-select-options][html:dom-select-selectedoptions]")
  {
    EditorFixture fixture(u8"<select id=s><option>A</option></select>");
    auto &select = SelectById(fixture, u8"s");
    CHECK(select.Options().get() == select.Options().get());
    CHECK(select.SelectedOptions().get() == select.SelectedOptions().get());
  }

  TEST_CASE_METHOD(
    css::tests::ValueFixture, "HTMLSelectElement: picking an option fires input, then change",
    "[HTML][HTMLSelectElement][html:send-select-update-notifications][html:concept-select-pick]")
  {
    EditorFixture fixture(u8"<select id=s><option>A</option><option>B</option></select>");
    auto &select = SelectById(fixture, u8"s");
    List<std::string> events;
    auto log = [&](dom::Event &event)
    {
      events.push_back(Narrow(event.Type().View()) + (event.Bubbles() ? " bubbles" : "")
                       + (event.Composed() ? " composed" : ""));
    };
    auto input = select.AddEventListener(u8"input", log);
    auto change = select.AddEventListener(u8"change", log);
    select.OptionSelectedByUser(1, true);
    html::EventLoop::Current().RunUntilIdle();
    CHECK(events == List<std::string> {"input bubbles composed", "change bubbles"});
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLSelectElement: keyboard", "[HTML][HTMLSelectElement]")
  {
    EditorFixture fixture(u8"<select id=s>"
                          u8"<option id=a>Apple</option>"
                          u8"<option id=b disabled>Banana</option>"
                          u8"<option id=c>Cherry</option>"
                          u8"<option id=d>Cranberry</option>"
                          u8"</select>");

    auto &select = SelectById(fixture, u8"s");
    auto input = CreateRef<EventCounter>();
    auto change = CreateRef<EventCounter>();
    select.AddEventListener(dom::DOMStringAtom(EventNames::Input),
                            ShareRef<dom::EventListenerCallback>(*input), false);
    select.AddEventListener(dom::DOMStringAtom(EventNames::Change),
                            ShareRef<dom::EventListenerCallback>(*change), false);
    select.Focus();
    REQUIRE(fixture.Document->FocusedElement() == &select);

    SECTION("Arrow keys skip disabled options")
    {
      CHECK(PressKey(fixture, u8"ArrowRight", u8"Right", vk::Right));
      CHECK(select.SelectedIndex() == 2);
      CHECK(input->Count == 1);
      CHECK(change->Count == 1);

      CHECK(PressKey(fixture, u8"End", u8"End", vk::End));
      CHECK(select.SelectedIndex() == 3);
      CHECK(PressKey(fixture, u8"Home", u8"Home", vk::Home));
      CHECK(select.SelectedIndex() == 0);
      CHECK(change->Count == 3);
    }

    SECTION("Type-ahead")
    {
      CHECK(TypeChar(fixture, u8'c'));
      CHECK(select.SelectedIndex() == 2);
      // Typing the same character again cycles through the options starting with it.
      CHECK(TypeChar(fixture, u8'c'));
      CHECK(select.SelectedIndex() == 3);
      CHECK(change->Count == 2);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLSelectElement: showPicker()", "[HTML][HTMLSelectElement]")
  {
    EditorFixture fixture(u8"<select id=s><option>Apple</option><option>Cherry</option></select>");
    auto &select = SelectById(fixture, u8"s");
    auto &window = *fixture.Document->Window();

    // Without a user gesture it's not allowed.
    auto result = select.ShowPicker();
    REQUIRE(result.HasException());
    CHECK(result.GetException().Code() == dom::ExceptionCode::NotAllowedError);

    // A gesture allows one call, which uses it up.
    window.NotifyActivated();
    CHECK_FALSE(select.ShowPicker().HasException());
    CHECK_FALSE(window.HasTransientActivation());

    // A disabled select has none, gesture or not.
    (void)select.SetAttribute(u8"disabled", u8"");
    window.NotifyActivated();
    auto disabled = select.ShowPicker();
    REQUIRE(disabled.HasException());
    CHECK(disabled.GetException().Code() == dom::ExceptionCode::InvalidStateError);
  }
}
