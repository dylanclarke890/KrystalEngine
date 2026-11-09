#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLLegendElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include <string>

namespace krys::boo::tests
{
  namespace
  {
    using Strings = List<std::string>;
    using Modifier = PlatformEvent::Modifier;

    struct Recorder : dom::EventListenerCallback
    {
      Strings &Records;

      explicit Recorder(Strings &records) noexcept : Records(records)
      {
      }

      void HandleEvent(dom::Event &event) noexcept override
      {
        auto *element = DynamicDowncast<dom::Element>(DynamicDowncast<dom::Node>(event.Target().get()));
        auto id = element ? element->GetAttribute(u8"id").value_or(dom::DOMString {}) : dom::DOMString {};
        auto type = event.Type().View();
        Records.push_back(std::string(type.begin(), type.end()) + ":" + std::string(id.begin(), id.end()));
      }
    };

    struct AccessKeyFixture : EditorFixture
    {
      Strings Records;
      Ref<dom::EventListenerCallback> Listener {CreateRef<Recorder>(Records)};

      explicit AccessKeyFixture(utf8_stringview markup) noexcept : EditorFixture(markup)
      {
        Listen(*Document);
      }

      void Listen(dom::EventTarget &target) noexcept
      {
        for (auto type : {u8"keydown", u8"keypress", u8"focus", u8"click"})
        {
          target.AddEventListener(dom::DOMStringAtom(type), Ref {Listener}, true);
        }
      }

      /// @brief Presses @p key with @p modifiers, returning what happened after the keydown.
      Strings Press(utf8_stringview key, EnumSet<Modifier> modifiers = {Modifier::AltKey}) noexcept
      {
        Records.clear();
        (void)Frame->GetEventHandler().KeyEvent(PlatformKeyboardEvent(
          PlatformEvent::Type::KeyDown, utf8_string(key), utf8_string(key), utf8_string(key), u8"", u8"", 0,
          false, false, modifiers.Contains(Modifier::AltKey), modifiers, MonotonicTime::Now()));
        Strings records;
        for (auto &record : Records)
        {
          if (!record.starts_with("keydown"))
          {
            records.push_back(record);
          }
        }
        return records;
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Access keys: which elements they find",
                   "[Page][EventHandler][html:assigned-access-key]")
  {
    AccessKeyFixture fixture(
      u8"<button id=plain accesskey=b>b</button><button id=upper accesskey=U>u</button>"
      u8"<button id=first accesskey=z>1</button><button id=last accesskey=z>2</button>"
      u8"<button id=tokens accesskey='too x'>t</button><button id=long accesskey=word>l</button>"
      u8"<button id=accented accesskey=\u00e9>e</button><div id=host></div>");

    SECTION("the key is pressed with Alt, and maybe Shift, and it replaces the keypress")
    {
      CHECK(fixture.Press(u8"b") == Strings {"focus:plain", "click:plain"});
      CHECK(fixture.Press(u8"b", {Modifier::AltKey, Modifier::ShiftKey}) == Strings {"click:plain"});
      CHECK(fixture.Press(u8"b", {}) == Strings {"keypress:plain"});
      CHECK(fixture.Press(u8"b", {Modifier::AltKey, Modifier::ControlKey}) == Strings {"keypress:plain"});
      CHECK(fixture.Press(u8"b", {Modifier::AltKey, Modifier::MetaKey}) == Strings {"keypress:plain"});
      CHECK(fixture.Press(u8"q") == Strings {"keypress:plain"});
    }

    SECTION("cancelling the keydown doesn't stop it")
    {
      struct Canceller : dom::EventListenerCallback
      {
        void HandleEvent(dom::Event &event) noexcept override
        {
          event.PreventDefault();
        }
      };
      fixture.Document->AddEventListener(dom::DOMStringAtom(u8"keydown"), CreateRef<Canceller>(), false);
      CHECK(fixture.Press(u8"b") == Strings {"focus:plain", "click:plain"});
    }

    SECTION("case doesn't matter, and the last element with the key wins")
    {
      CHECK(fixture.Press(u8"u") == Strings {"focus:upper", "click:upper"});
      CHECK(fixture.Press(u8"z") == Strings {"focus:last", "click:last"});
    }

    SECTION("the key is the first token of one code point")
    {
      CHECK(fixture.ById(u8"tokens").AssignedAccessKey() == u8"x");
      CHECK(fixture.Press(u8"x") == Strings {"focus:tokens", "click:tokens"});
      CHECK(fixture.ById(u8"long").AssignedAccessKey().empty());
      CHECK(fixture.Press(u8"word") == Strings {"keypress:tokens"});
      CHECK(fixture.ById(u8"accented").AssignedAccessKey() == u8"\u00e9");
      CHECK(fixture.Press(u8"\u00e9") == Strings {"focus:accented", "click:accented"});
    }

    SECTION("it follows the attribute as it changes")
    {
      auto &plain = fixture.ById(u8"plain");
      REQUIRE_FALSE(plain.SetAttribute(u8"accesskey", u8"k").HasException());
      CHECK(fixture.Press(u8"b") == Strings {"keypress:"});
      CHECK(fixture.Press(u8"k") == Strings {"focus:plain", "click:plain"});
    }

    SECTION("it reaches into shadow trees")
    {
      auto shadow = fixture.ById(u8"host").AttachShadow({}).ReleaseValue();
      auto inner = fixture.Document->CreateElement(u8"button").ReleaseValue();
      REQUIRE_FALSE(inner->SetAttribute(u8"accesskey", u8"s").HasException());
      REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK(fixture.Press(u8"s") == Strings {"focus:host", "click:host"});
    }

    SECTION("accessKeyLabel names the key and its modifier")
    {
      CHECK(Downcast<html::HTMLElement>(fixture.ById(u8"plain")).AccessKeyLabel() == u8"Alt+b");
      CHECK(Downcast<html::HTMLElement>(fixture.ById(u8"tokens")).AccessKeyLabel() == u8"Alt+x");
      CHECK(Downcast<html::HTMLElement>(fixture.ById(u8"long")).AccessKeyLabel().empty());
      CHECK(fixture.Document->Body()->AccessKeyLabel().empty());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Access keys: what they do",
                   "[Page][EventHandler][html:command-facet-action]")
  {
    AccessKeyFixture fixture(
      u8"<input id=text accesskey=a><input id=date type=date accesskey=b><input id=gone type=hidden "
      u8"accesskey=c>"
      u8"<input id=off disabled accesskey=d><textarea id=area accesskey=e></textarea>"
      u8"<button id=nobox accesskey=f style=display:none>f</button><div id=plain accesskey=g>g</div>"
      u8"<label id=for accesskey=h for=target>h</label><input id=target>"
      u8"<label id=empty accesskey=i>i</label>"
      u8"<svg width=9 height=9 id=svg accesskey=j tabindex=0></svg>"
      u8"<math id=math accesskey=k tabindex=0><mi>x</mi></math>"
      u8"<fieldset><legend id=legend accesskey=l>l</legend><label id=l2 accesskey=m>m</label>"
      u8"<span>x</span><button id=delegate>d</button><button id=after>a</button></fieldset>"
      u8"<fieldset><legend id=alone accesskey=n>n</legend><span>x</span></fieldset>"
      u8"<legend id=orphan accesskey=o>o</legend>");
    (void)fixture.Document->UpdateLayout();

    CHECK(fixture.Press(u8"a") == Strings {"focus:text", "click:text"});
    CHECK(fixture.Press(u8"b") == Strings {"focus:date"});
    CHECK(fixture.Press(u8"c").empty());
    CHECK(fixture.Press(u8"d").empty());
    CHECK(fixture.Press(u8"e") == Strings {"focus:area"});
    // What can't be focused is still clicked.
    CHECK(fixture.Press(u8"f") == Strings {"click:nobox"});
    CHECK(fixture.Press(u8"g") == Strings {"click:plain"});
    // A label runs its control's command.
    CHECK(fixture.Press(u8"h") == Strings {"focus:target", "click:target"});
    CHECK(fixture.Press(u8"i") == Strings {"click:empty"});
    CHECK(fixture.Press(u8"j") == Strings {"focus:svg", "click:svg"});
    CHECK(fixture.Press(u8"k") == Strings {"focus:math"});

    // A legend runs its delegatee's: not a label's, and the first of the rest.
    CHECK(Downcast<html::HTMLLegendElement>(fixture.ById(u8"legend")).AccessKeyDelegatee().get()
          == &fixture.ById(u8"delegate"));
    CHECK(fixture.Press(u8"l") == Strings {"focus:delegate", "click:delegate"});
    // Without one, it's clicked like any other element.
    CHECK_FALSE(Downcast<html::HTMLLegendElement>(fixture.ById(u8"alone")).AccessKeyDelegatee());
    CHECK(fixture.Press(u8"n") == Strings {"click:alone"});
    CHECK_FALSE(Downcast<html::HTMLLegendElement>(fixture.ById(u8"orphan")).AccessKeyDelegatee());
    CHECK(fixture.Press(u8"o") == Strings {"click:orphan"});
  }
}
