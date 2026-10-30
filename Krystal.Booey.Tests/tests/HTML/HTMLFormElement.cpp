#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"
#include "Krystal.Booey/DOM/FormDataEvent.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/HTML/SubmitEvent.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::EventLog;
  using boo::tests::FormFixture;
  using boo::tests::Str;

  namespace
  {
    /// @brief Records the submitter of each submit event and the entries of each formdata event.
    struct SubmissionEvents : dom::EventListenerCallback
    {
      List<std::string> Submitters;
      List<std::string> Entries;
      bool PreventSubmit {false};

      void HandleEvent(dom::Event &event) noexcept override
      {
        if (auto *submitEvent = DynamicDowncast<SubmitEvent>(event))
        {
          auto *submitter = submitEvent->Submitter();
          Submitters.push_back(submitter ? Str(submitter->GetAttribute(u8"id").value_or(u8"")) : "null");
          if (PreventSubmit)
          {
            event.PreventDefault();
          }
        }
        else if (auto *formDataEvent = DynamicDowncast<dom::FormDataEvent>(event))
        {
          for (auto &item : formDataEvent->FormData().Items())
          {
            Entries.push_back(Str(item.Name) + "=" + Str(std::get<dom::DOMString>(item.Data)));
          }
          formDataEvent->FormData().Append(u8"added", u8"1");
        }
      }
    };
  }

  TEST_CASE("HTMLFormElement: submission", "[HTML][Forms]")
  {
    FormFixture fixture(u8R"(<form id=f action="https://example.com/search" target=results>
      <input name=q value="a b&c"><input name=n type=number value=3><input type=checkbox name=on checked>
      <input type=checkbox name=off><input type=radio name=r value=x><input type=radio name=r value=y checked>
      <select name=s><option>one</option><option selected>two</option></select><textarea name=t>line1
line2</textarea><input id=disabled name=disabled disabled value=no><input value=noname><input type=submit id=go name=go
      value=Go><button id=other name=other value=o></button></form>)");
    auto &form = fixture.Get<HTMLFormElement>(u8"f");
    auto events = CreateRef<SubmissionEvents>();
    form.AddEventListener(dom::DOMStringAtom(u8"submit"), Ref<dom::EventListenerCallback> {events}, false);
    form.AddEventListener(dom::DOMStringAtom(u8"formdata"), Ref<dom::EventListenerCallback> {events}, false);

    SECTION("submit() skips the submit event and validation")
    {
      form.Submit();
      REQUIRE(fixture.Submissions().size() == 1);
      auto &submission = fixture.Submissions()[0];
      CHECK(submission.Method == FormSubmission::Method::Get);
      CHECK(submission.URL
            == "https://example.com/search?q=a+b%26c&n=3&on=on&r=y&s=two&t=line1%0D%0Aline2&added=1");
      CHECK(submission.Target == "results");
      CHECK(events->Submitters.empty());
      CHECK(events->Entries.size() == 6);
    }

    SECTION("requestSubmit() fires submit, which can cancel it")
    {
      REQUIRE_FALSE(form.RequestSubmit(&fixture.Get<HTMLInputElement>(u8"go")).HasException());
      REQUIRE(fixture.Submissions().size() == 1);
      CHECK(fixture.Submissions()[0].Submitter == "go");
      CHECK(fixture.Submissions()[0].URL.find("&go=Go&") != std::string::npos);
      REQUIRE(events->Submitters.size() == 1);
      CHECK(events->Submitters[0] == "go");

      events->PreventSubmit = true;
      REQUIRE_FALSE(form.RequestSubmit().HasException());
      CHECK(fixture.Submissions().size() == 1);
      CHECK(events->Submitters.back() == "null");

      // The submitter must be one of the form's submit buttons.
      CHECK(form.RequestSubmit(&fixture.Get<HTMLInputElement>(u8"disabled")).HasException());
    }

    SECTION("Clicking a submit button submits with it")
    {
      fixture.Get<HTMLButtonElement>(u8"other").Click();
      REQUIRE(fixture.Submissions().size() == 1);
      CHECK(fixture.Submissions()[0].Submitter == "other");
      CHECK(fixture.Submissions()[0].URL.find("&other=o&") != std::string::npos);
      CHECK(fixture.Submissions()[0].URL.find("go=Go") == std::string::npos);
    }

    SECTION("POST bodies")
    {
      form.Method(u8"post");
      form.Submit();
      REQUIRE(fixture.Submissions().size() == 1);
      CHECK(fixture.Submissions()[0].Method == FormSubmission::Method::Post);
      CHECK(fixture.Submissions()[0].URL == "https://example.com/search");
      CHECK(fixture.Submissions()[0].ContentType == "application/x-www-form-urlencoded");
      CHECK(fixture.Submissions()[0].Body.starts_with("q=a+b%26c&n=3"));

      form.EncType(u8"text/plain");
      form.Submit();
      REQUIRE(fixture.Submissions().size() == 2);
      CHECK(fixture.Submissions()[1].Body.starts_with("q=a b&c\r\nn=3\r\n"));

      form.EncType(u8"multipart/form-data");
      form.Submit();
      REQUIRE(fixture.Submissions().size() == 3);
      auto &multipart = fixture.Submissions()[2];
      CHECK(multipart.ContentType == "multipart/form-data");
      CHECK(multipart.Boundary.starts_with("----BooeyFormBoundary"));
      CHECK(multipart.Body.starts_with("--" + multipart.Boundary + "\r\n"));
      CHECK(multipart.Body.ends_with("--" + multipart.Boundary + "--\r\n"));
      CHECK(multipart.Body.find("Content-Disposition: form-data; name=\"q\"\r\n\r\na b&c\r\n")
            != std::string::npos);
    }
  }

  TEST_CASE("HTMLFormElement: image buttons", "[HTML][Forms]")
  {
    FormFixture fixture(u8R"(<form action="https://example.com/"><input name=a value=1>
      <input type=image id=named name=pic><input type=image id=unnamed></form>)");

    // A click from script has no position, so the coordinates are 0.
    fixture.Get<HTMLInputElement>(u8"named").Click();
    REQUIRE(fixture.Submissions().size() == 1);
    CHECK(fixture.Submissions()[0].URL == "https://example.com/?a=1&pic.x=0&pic.y=0");

    fixture.Get<HTMLInputElement>(u8"unnamed").Click();
    REQUIRE(fixture.Submissions().size() == 2);
    CHECK(fixture.Submissions()[1].URL == "https://example.com/?a=1&x=0&y=0");
  }

  TEST_CASE("HTMLFormElement: interactive validation", "[HTML][Forms]")
  {
    FormFixture fixture(u8R"(<form id=f action="https://example.com/"><input id=t name=t required>
      <input type=submit id=go></form>)");
    auto &form = fixture.Get<HTMLFormElement>(u8"f");
    auto &t = fixture.Get<HTMLInputElement>(u8"t");
    auto invalid = EventLog::ListenTo(t, {u8"invalid"});

    REQUIRE_FALSE(form.RequestSubmit().HasException());
    CHECK(fixture.Submissions().empty());
    CHECK(invalid->Joined() == "invalid");

    SECTION("novalidate skips it")
    {
      form.NoValidate(true);
      REQUIRE_FALSE(form.RequestSubmit().HasException());
      CHECK(fixture.Submissions().size() == 1);
    }

    SECTION("formnovalidate on the submitter skips it")
    {
      auto &go = fixture.Get<HTMLInputElement>(u8"go");
      go.FormNoValidate(true);
      REQUIRE_FALSE(form.RequestSubmit(&go).HasException());
      CHECK(fixture.Submissions().size() == 1);
    }

    SECTION("A valid form submits")
    {
      (void)t.SetValue(u8"x");
      fixture.Get<HTMLInputElement>(u8"go").Click();
      CHECK(fixture.Submissions().size() == 1);
    }
  }

  TEST_CASE("HTMLFormElement: implicit submission", "[HTML][Forms]")
  {
    FormFixture fixture(u8R"(<form id=f action="https://example.com/"><input id=t name=t value=v>
      <input type=submit id=go name=go value=Go></form>)");
    auto &t = fixture.Get<HTMLInputElement>(u8"t");

    dom::KeyboardEventInit init;
    init.Bubbles = true;
    init.Cancelable = true;
    init.CharCode = '\r';
    auto enter = CreateRef<dom::KeyboardEvent>(dom::DOMStringAtom(u8"keypress"), init);
    (void)t.DispatchEvent(*enter);
    REQUIRE(fixture.Submissions().size() == 1);
    // The default button submits, as if it were clicked.
    CHECK(fixture.Submissions()[0].Submitter == "go");
    CHECK(fixture.Submissions()[0].URL == "https://example.com/?t=v&go=Go");
  }

  TEST_CASE("HTMLFormElement: reset", "[HTML][Forms]")
  {
    FormFixture fixture(u8R"(<form id=f><input id=t value=a><input id=c type=checkbox checked>
      <textarea id=ta>default</textarea><select id=s><option>one</option><option id=two>two</option></select>
      <input type=reset id=r></form>)");
    auto &form = fixture.Get<HTMLFormElement>(u8"f");
    auto &t = fixture.Get<HTMLInputElement>(u8"t");
    auto &c = fixture.Get<HTMLInputElement>(u8"c");
    auto &ta = fixture.Get<HTMLTextAreaElement>(u8"ta");
    auto &s = fixture.Get<HTMLSelectElement>(u8"s");
    (void)t.SetValue(u8"changed");
    c.Checked(false);
    (void)ta.SetValue(u8"changed");
    fixture.Get<HTMLOptionElement>(u8"two").Selected(true);
    auto events = EventLog::ListenTo(form, {u8"reset"});

    SECTION("A reset button resets the form")
    {
      fixture.Get<HTMLInputElement>(u8"r").Click();
      CHECK(events->Joined() == "reset");
      CHECK(Str(t.Value()) == "a");
      CHECK(c.Checked());
      CHECK(Str(ta.Value()) == "default");
      CHECK(s.SelectedIndex() == 0);
    }

    SECTION("Canceling the reset event keeps the values")
    {
      events->PreventDefault = true;
      form.Reset();
      CHECK(Str(t.Value()) == "changed");
      CHECK_FALSE(c.Checked());
    }
  }
}
