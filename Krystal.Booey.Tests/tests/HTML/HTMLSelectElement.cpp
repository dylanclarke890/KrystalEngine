#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/HTML/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/HTMLOptionsCollection.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Platform/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/WindowsKeyboardCodes.hpp"

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
        ids += item->GetAttribute(u8"id").value_or(dom::DOMString(u8"?"));
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
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLSelectElement: list items", "[HTML][HTMLSelectElement]")
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

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLSelectElement: selection", "[HTML][HTMLSelectElement]")
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

    // A clean option's selectedness follows its selected attribute; a dirty one's doesn't.
    REQUIRE_FALSE(OptionById(fixture, u8"c").SetAttribute(u8"selected", u8"").HasException());
    CHECK(select.SelectedIndex() == 2);
    REQUIRE_FALSE(OptionById(fixture, u8"a").SetAttribute(u8"selected", u8"").HasException());
    CHECK(select.SelectedIndex() == 2);

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

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLSelectElement: options text and labels",
                   "[HTML][HTMLSelectElement]")
  {
    EditorFixture fixture(
      u8"<select id=s><option id=a label=Label>  Some <b>bold</b>  text <script>x</script></option>"
      u8"<option id=b>B</option></select>");

    auto &a = OptionById(fixture, u8"a");
    CHECK(a.Text() == u8"Some bold text");
    CHECK(a.Label() == u8"Label");
    CHECK(a.Value() == u8"Some bold text");
    CHECK(OptionById(fixture, u8"b").Label() == u8"B");

    // Setting the text keeps a menu list's selection.
    auto &select = SelectById(fixture, u8"s");
    select.SelectedIndex(1);
    OptionById(fixture, u8"b").Text(u8"Bee");
    CHECK(select.SelectedIndex() == 1);
    CHECK(select.Value() == u8"Bee");

    auto created = HTMLOptionElement::CreateForLegacyFactoryFunction(*fixture.Document, u8"New",
                                                                     dom::DOMString(u8"n"), true, false);
    REQUIRE_FALSE(created.HasException());
    auto option = created.ReleaseValue();
    CHECK(option->Text() == u8"New");
    CHECK(option->Value() == u8"n");
    CHECK(option->DefaultSelected());
    CHECK_FALSE(option->Selected());
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
}
