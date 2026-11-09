#include "Krystal.Booey/HTML/Forms/ValidatedFormListedElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFieldSetElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/Events/PlatformKeyboardEvent.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::EditorFixture;
  using boo::tests::EventLog;
  using boo::tests::FormFixture;
  using boo::tests::Str;

  namespace
  {
    /// @brief A chrome client that keeps the validation message it's asked to show.
    struct ValidationChromeClient final : ChromeClient
    {
      const dom::Element *Anchor {nullptr};
      dom::DOMString Message;
      uint32 ShowCount {0};

      void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept final
      {
      }

      void ShowValidationMessage(dom::Element &anchor, const dom::DOMString &message) noexcept final
      {
        Anchor = &anchor;
        Message = message;
        ++ShowCount;
      }

      void HideValidationMessage(const dom::Element &anchor) noexcept final
      {
        if (Anchor == &anchor)
        {
          HideAnyValidationMessage();
        }
      }

      void HideAnyValidationMessage() noexcept final
      {
        Anchor = nullptr;
        Message.clear();
      }

      KRYS_NODISCARD bool IsValidationMessageVisible(const dom::Element &anchor) const noexcept final
      {
        return Anchor == &anchor;
      }
    };

    void PressEscape(LocalFrame &frame) noexcept
    {
      (void)frame.GetEventHandler().KeyEvent(PlatformKeyboardEvent(PlatformEvent::Type::KeyDown, u8"", u8"",
                                                                   u8"Escape", u8"", u8"", 0, false, false,
                                                                   false, {}, MonotonicTime::Now()));
    }

    KRYS_NODISCARD bool Matches(const dom::Element &element, dom::DOMStringView selector)
    {
      auto result = element.Matches(selector);
      REQUIRE_FALSE(result.HasException());
      return result.ReleaseValue();
    }
  }

  TEST_CASE("Constraint validation: value missing", "[HTML][Forms][html:suffering-from-being-missing]")
  {
    FormFixture fixture(u8R"(<form id=f><fieldset id=fs><input id=t required></fieldset>
      <textarea id=ta required></textarea><select id=s required><option value="">-</option><option id=x>x</option>
      </select></form>)");
    auto &form = fixture.Get<HTMLFormElement>(u8"f");
    auto &t = fixture.Get<HTMLInputElement>(u8"t");
    auto &ta = fixture.Get<HTMLTextAreaElement>(u8"ta");
    auto &s = fixture.Get<HTMLSelectElement>(u8"s");

    CHECK(t.WillValidate());
    CHECK(t.ValueMissing());
    CHECK(ta.ValueMissing());
    CHECK(s.ValueMissing());
    CHECK(Str(t.ValidationMessage()) == "Fill out this field");
    CHECK(Matches(t, u8":invalid"));
    CHECK(Matches(t, u8":required"));
    CHECK(Matches(fixture.ById(u8"fs"), u8":invalid"));
    CHECK(Matches(form, u8":invalid"));
    CHECK_FALSE(Matches(t, u8":user-invalid"));

    auto invalidEvents = EventLog::ListenTo(t, {u8"invalid"});
    CHECK_FALSE(form.CheckValidity());
    CHECK(invalidEvents->Joined() == "invalid");

    (void)t.SetValue(u8"a");
    (void)ta.SetValue(u8"b");
    fixture.Get<HTMLOptionElement>(u8"x").Selected(true);
    CHECK_FALSE(t.ValueMissing());
    CHECK_FALSE(s.ValueMissing());
    CHECK(Matches(t, u8":valid"));
    CHECK(Matches(fixture.ById(u8"fs"), u8":valid"));
    CHECK(Matches(form, u8":valid"));
    CHECK(form.CheckValidity());
  }

  TEST_CASE(
    "Constraint validation: barred controls",
    "[HTML][Forms][html:barred-from-constraint-validation][html:concept-fe-disabled][html:concept-fe-mutable]"
    "[html:dom-cva-willvalidate]")
  {
    FormFixture fixture(
      u8R"(<form><input id=hidden type=hidden required><input id=disabled disabled required>
      <input id=readonly readonly required><button id=button type=button></button><button id=submit></button>
      <fieldset disabled><legend><input id=inlegend></legend><input id=infieldset></fieldset></form>)");
    CHECK_FALSE(fixture.Get<HTMLInputElement>(u8"hidden").WillValidate());
    CHECK_FALSE(fixture.Get<HTMLInputElement>(u8"disabled").WillValidate());
    CHECK_FALSE(fixture.Get<HTMLInputElement>(u8"readonly").WillValidate());
    CHECK_FALSE(fixture.Get<HTMLButtonElement>(u8"button").WillValidate());
    CHECK(fixture.Get<HTMLButtonElement>(u8"submit").WillValidate());

    // A disabled fieldset disables its descendants, except those in its first legend.
    auto &inLegend = fixture.Get<HTMLInputElement>(u8"inlegend");
    auto &inFieldset = fixture.Get<HTMLInputElement>(u8"infieldset");
    CHECK_FALSE(inLegend.IsDisabledFormControl());
    CHECK(inFieldset.IsDisabledFormControl());
    CHECK(Matches(inFieldset, u8":disabled"));
    CHECK(Matches(inLegend, u8":enabled"));
    CHECK(Matches(fixture.ById(u8"readonly"), u8":read-only"));
    CHECK(Matches(inLegend, u8":read-write"));
  }

  TEST_CASE("Constraint validation: custom validity",
            "[HTML][Forms][html:dom-cva-setcustomvalidity][html:custom-validity-error-message]")
  {
    FormFixture fixture(u8R"(<input id=t>)");
    auto &t = fixture.Get<HTMLInputElement>(u8"t");
    t.SetCustomValidity(u8"Not like that");
    CHECK(t.CustomError());
    CHECK(Str(t.ValidationMessage()) == "Not like that");

    // Its newlines are normalized.
    t.SetCustomValidity(u8"a\r\nb\rc");
    CHECK(t.ValidationMessage() == u8"a\nb\nc");
    CHECK(Matches(t, u8":invalid"));
    t.SetCustomValidity(u8"");
    CHECK_FALSE(t.CustomError());
    CHECK(Matches(t, u8":valid"));
  }

  TEST_CASE("Constraint validation: the host shows the validation message",
            "[HTML][Forms][html:report-validity-steps][html:interactively-validate-the-constraints]")
  {
    auto chromeClient = CreateUnique<ValidationChromeClient>();
    auto *client = chromeClient.get();
    EditorFixture fixture(u8"<form id=f><input id=ok><input id=t required pattern=x><input id=u required>"
                          u8"<input id=hidden required style=display:none></form><input id=other>",
                          krys::move(chromeClient));
    auto &form = Downcast<HTMLFormElement>(fixture.ById(u8"f"));
    auto &t = Downcast<HTMLInputElement>(fixture.ById(u8"t"));
    auto &u = Downcast<HTMLInputElement>(fixture.ById(u8"u"));

    SECTION("reportValidity() focuses the control and shows its message there")
    {
      CHECK_FALSE(t.ReportValidity());
      CHECK(client->Anchor == &t);
      CHECK(client->Message == t.ValidationMessage());
      CHECK_FALSE(client->Message.empty());
      CHECK(t.IsShowingValidationMessage());
      CHECK(fixture.Document->FocusedElement() == &t);

      // Another control's message takes its place.
      CHECK_FALSE(u.ReportValidity());
      CHECK(client->Anchor == &u);
      CHECK_FALSE(t.IsShowingValidationMessage());
    }

    SECTION("a valid control or one that isn't rendered shows none")
    {
      CHECK(Downcast<HTMLInputElement>(fixture.ById(u8"ok")).ReportValidity());
      CHECK_FALSE(Downcast<HTMLInputElement>(fixture.ById(u8"hidden")).ReportValidity());
      CHECK(client->ShowCount == 0);
    }

    SECTION("typing hides it, even while the control is still invalid")
    {
      CHECK_FALSE(t.ReportValidity());
      t.Select();
      CHECK(fixture.GetEditor().InsertText(u8"a", nullptr));
      CHECK(t.PatternMismatch());
      CHECK(client->Anchor == nullptr);
    }

    SECTION("losing the focus hides it")
    {
      CHECK_FALSE(t.ReportValidity());
      Downcast<HTMLInputElement>(fixture.ById(u8"other")).Focus();
      CHECK(client->Anchor == nullptr);
    }

    SECTION("Escape hides it")
    {
      CHECK_FALSE(t.ReportValidity());
      PressEscape(*fixture.Frame);
      CHECK(client->Anchor == nullptr);
    }

    SECTION("submitting shows the first invalid control's message")
    {
      REQUIRE_FALSE(form.RequestSubmit().HasException());
      CHECK(client->Anchor == &t);
      CHECK(fixture.Document->FocusedElement() == &t);
    }

    SECTION("removing the control hides it")
    {
      CHECK_FALSE(t.ReportValidity());
      t.Remove();
      CHECK(client->Anchor == nullptr);
    }
  }
}
