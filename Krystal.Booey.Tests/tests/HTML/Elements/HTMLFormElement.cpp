#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"
#include "Krystal.Booey/DOM/Events/FormDataEvent.hpp"
#include "Krystal.Booey/DOM/Events/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/Collections/HTMLFormControlsCollection.hpp"
#include "Krystal.Booey/HTML/Collections/RadioNodeList.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/HTML/Events/SubmitEvent.hpp"
#include "Krystal.Booey/HTML/Forms/DOMFormData.hpp"

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

  TEST_CASE(
    "HTMLFormElement: autocomplete, elements, length, named items and validity",
    "[HTML][Forms][html:dom-form-autocomplete][html:attr-form-autocomplete][html:dom-form-elements]"
    "[html:dom-form-length][html:past-names-map][html:dom-form-checkvalidity][html:dom-form-reportvalidity]")
  {
    FormFixture fixture(u8R"(<form id=f><input id=a name=a required><input type=image id=img name=i>
      <output id=o></output><fieldset id=fs></fieldset></form>)");
    auto &form = fixture.Get<HTMLFormElement>(u8"f");
    auto &a = fixture.Get<HTMLInputElement>(u8"a");

    CHECK(Str(form.Autocomplete()) == "on");
    form.Autocomplete(u8"OFF");
    CHECK(Str(form.Autocomplete()) == "off");
    form.Autocomplete(u8"maybe");
    CHECK(Str(form.Autocomplete()) == "on");

    // The same collection each time, without image buttons.
    auto elements = form.Elements();
    CHECK(form.Elements().get() == elements.get());
    CHECK(elements->Length() == 3);
    CHECK(form.Length() == 3);

    // A named item found once is still found by that name when renamed.
    auto item = form.NamedItem(u8"a");
    REQUIRE(item);
    CHECK(std::get<RefPtr<dom::Element>>(*item).get() == &a);
    REQUIRE_FALSE(a.SetAttribute(u8"name", u8"b").HasException());
    item = form.NamedItem(u8"a");
    REQUIRE(item);
    CHECK(std::get<RefPtr<dom::Element>>(*item).get() == &a);

    auto invalid = EventLog::ListenTo(a, {u8"invalid"});
    CHECK_FALSE(form.CheckValidity());
    CHECK(invalid->Joined() == "invalid");
    (void)a.SetValue(u8"x");
    CHECK(form.CheckValidity());
    CHECK(form.ReportValidity());
  }

  TEST_CASE(
    "HTMLFormElement: method, enctype and encoding are limited to known values",
    "[HTML][Forms][html:dom-fs-method][html:dom-fs-enctype][html:dom-fs-encoding][html:attr-fs-method]"
    "[html:attr-fs-enctype][html:form-submission-attributes]")
  {
    FormFixture fixture(u8"<form id=f></form><form id=m method=POST enctype=bogus></form>");
    auto &form = fixture.Get<HTMLFormElement>(u8"f");
    CHECK(Str(form.Method()) == "get");
    CHECK(Str(form.EncType()) == "application/x-www-form-urlencoded");
    auto &other = fixture.Get<HTMLFormElement>(u8"m");
    CHECK(Str(other.Method()) == "post");
    CHECK(Str(other.EncType()) == "application/x-www-form-urlencoded");
    CHECK(Str(other.Encoding()) == "application/x-www-form-urlencoded");

    // encoding reflects enctype.
    other.Encoding(u8"Multipart/Form-Data");
    CHECK(Str(other.GetAttribute(u8"enctype").value_or(u8"")) == "Multipart/Form-Data");
    CHECK(Str(other.EncType()) == "multipart/form-data");
    other.Method(u8"dialog");
    CHECK(Str(other.Method()) == "dialog");
  }

  TEST_CASE("HTMLFormElement: formMethod and formEnctype have no missing value default",
            "[HTML][Forms][html:dom-fs-formmethod][html:dom-fs-formenctype][html:attr-fs-formmethod]"
            "[html:attr-fs-formenctype]")
  {
    FormFixture fixture(
      u8"<form><input id=i type=submit><input id=b type=submit formmethod=bogus "
      u8"formenctype=TEXT/PLAIN><button id=bt formmethod=POST formenctype=x>B</button></form>");
    auto &input = fixture.Get<HTMLInputElement>(u8"i");
    CHECK(Str(input.FormMethod()).empty());
    CHECK(Str(input.FormEnctype()).empty());
    auto &other = fixture.Get<HTMLInputElement>(u8"b");
    CHECK(Str(other.FormMethod()) == "get");
    CHECK(Str(other.FormEnctype()) == "text/plain");
    auto &button = fixture.Get<HTMLButtonElement>(u8"bt");
    CHECK(Str(button.FormMethod()) == "post");
    CHECK(Str(button.EncType()) == "application/x-www-form-urlencoded");
  }

  TEST_CASE("HTMLFormElement: a submitter's formmethod picks the encoding",
            "[HTML][Forms][html:concept-fs-method][html:concept-fs-enctype][html:concept-form-submit]")
  {
    FormFixture fixture(u8R"(<form id=f action="https://example.com/" method=post enctype=text/plain>
      <input name=a value="1 2"><input type=submit id=get formmethod=get></form>)");
    // A GET submission is URL-encoded whatever the enctype.
    fixture.Get<HTMLInputElement>(u8"get").Click();
    REQUIRE(fixture.Submissions().size() == 1);
    CHECK(fixture.Submissions()[0].URL == "https://example.com/?a=1+2");
  }

  TEST_CASE("HTMLFormElement: the entry list leaves out controls in datalists",
            "[HTML][Forms][html:constructing-the-form-data-set][html:create-an-entry]"
            "[html:convert-to-a-list-of-name-value-pairs][html:text/plain-encoding-algorithm]"
            "[html:picking-an-encoding-for-the-form]")
  {
    FormFixture fixture(u8R"(<form id=p action="https://example.com/p" method=post enctype=text/plain
      accept-charset="bogus utf-8"><input name="a&#10;b" value="x"><datalist><input name=d value=1></datalist>
      <input type=hidden name=_charset_><select name=s><option disabled selected>1</option></select>
      <input type=checkbox name=cb checked><input type=checkbox name=un><input type=file name=fl>
      <input name=dn value=hi dirname=dn.dir></form>)");
    fixture.Get<HTMLFormElement>(u8"p").Submit();
    REQUIRE(fixture.Submissions().size() == 1);
    CHECK(fixture.Submissions()[0].Body
          == "a\r\nb=x\r\n_charset_=UTF-8\r\ncb=on\r\nfl=\r\ndn=hi\r\ndn.dir=ltr\r\n");
  }

  TEST_CASE("HTMLFormElement: multipart bodies normalize newlines",
            "[HTML][Forms][html:multipart/form-data-encoding-algorithm]")
  {
    FormFixture fixture(u8"<form id=mp action=https://example.com/m method=post enctype=multipart/form-data>"
                        u8"<textarea name=x>a\nb</textarea></form>");
    fixture.Get<HTMLFormElement>(u8"mp").Submit();
    REQUIRE(fixture.Submissions().size() == 1);
    CHECK(fixture.Submissions()[0].Body.find("a\r\nb") != std::string::npos);
    CHECK(fixture.Submissions()[0].Body.find("name=\"x\"") != std::string::npos);
  }

  TEST_CASE(
    "HTMLFormElement: constraint validation fires invalid at each invalid control, in tree order",
    "[HTML][Forms][html:statically-validate-the-constraints][html:interactively-validate-the-constraints]"
    "[html:check-validity-steps][html:dom-cva-checkvalidity][html:dom-cva-validationmessage]"
    "[html:candidate-for-constraint-validation][html:concept-fv-valid]")
  {
    FormFixture fixture(
      u8R"(<form id=f action="https://example.com/"><input id=a required><input id=b required>
      </form>)");
    auto &a = fixture.Get<HTMLInputElement>(u8"a");
    auto &b = fixture.Get<HTMLInputElement>(u8"b");
    auto &form = fixture.Get<HTMLFormElement>(u8"f");
    auto aEvents = EventLog::ListenTo(a, {u8"invalid"});
    auto bEvents = EventLog::ListenTo(b, {u8"invalid"});
    CHECK_FALSE(a.CheckValidity());
    CHECK(aEvents->Joined() == "invalid");
    CHECK_FALSE(a.ValidationMessage().empty());
    CHECK_FALSE(form.CheckValidity());
    CHECK(aEvents->Joined() == "invalid invalid");
    CHECK(bEvents->Joined() == "invalid");

    // A canceled invalid event is handled, but the form is still invalid.
    aEvents->PreventDefault = true;
    REQUIRE_FALSE(form.RequestSubmit().HasException());
    CHECK(fixture.Submissions().empty());

    (void)a.SetValue(u8"x");
    (void)b.SetValue(u8"x");
    CHECK(a.ValidationMessage().empty());
    CHECK(form.CheckValidity());
  }

  TEST_CASE("HTMLFormElement: submitting from a submit event, or from a dialog",
            "[HTML][Forms][html:concept-form-submit][html:firing-submission-events][html:submit-dialog]")
  {
    FormFixture fixture(u8R"(<form id=f action="https://example.com/"></form>
      <dialog id=d open><form method=dialog><button id=db value=ok>OK</button></form></dialog>
      <form method=dialog><button id=ndb>x</button></form>)");
    auto &form = fixture.Get<HTMLFormElement>(u8"f");
    int submits = 0;
    auto listener = form.AddEventListener(u8"submit",
                                          [&](dom::Event &)
                                          {
                                            ++submits;
                                            (void)form.RequestSubmit();
                                          });
    REQUIRE_FALSE(form.RequestSubmit().HasException());
    CHECK(submits == 1);
    CHECK(fixture.Submissions().size() == 1);

    // The dialog method closes the form's dialog with the submitter's value, and doesn't navigate.
    auto &dialog = fixture.Get<HTMLDialogElement>(u8"d");
    fixture.Get<HTMLButtonElement>(u8"db").Click();
    CHECK_FALSE(dialog.Open());
    CHECK(Str(dialog.ReturnValue()) == "ok");
    fixture.Get<HTMLButtonElement>(u8"ndb").Click();
    CHECK(fixture.Submissions().size() == 1);
  }

  TEST_CASE("HTMLFormElement: implicit submission without a submit button needs a single blocking field",
            "[HTML][Forms][html:implicit-submission][html:field-that-blocks-implicit-submission]"
            "[html:default-button]")
  {
    FormFixture fixture(
      u8R"(<form action="https://example.com/t"><input id=t1 name=t1><input id=t2 name=t2></form>
      <form action="https://example.com/o"><input id=o1 name=o1><input type=checkbox></form>)");
    dom::KeyboardEventInit init;
    init.Bubbles = true;
    init.Cancelable = true;
    init.CharCode = '\r';
    auto enter = [&](utf8_stringview id)
    {
      (void)fixture.Get<HTMLInputElement>(id).DispatchEvent(
        *CreateRef<dom::KeyboardEvent>(dom::DOMStringAtom(u8"keypress"), init));
    };
    enter(u8"t1");
    CHECK(fixture.Submissions().empty());
    enter(u8"o1");
    REQUIRE(fixture.Submissions().size() == 1);
    CHECK(fixture.Submissions()[0].URL == "https://example.com/o?o1=");
  }

  TEST_CASE("HTMLFormElement: submission",
            "[HTML][Forms][html:dom-form-submit][html:dom-form-requestsubmit][html:dom-submitevent-submitter]"
            "[html:dom-formdataevent-formdata]")
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

  TEST_CASE("HTMLFormElement: image buttons", "[HTML][Forms][html:image-button-state-(type=image)]")
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

  TEST_CASE("HTMLFormElement: interactive validation",
            "[HTML][Forms][html:dom-form-requestsubmit][html:concept-fs-novalidate]")
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

  TEST_CASE("HTMLFormElement: implicit submission", "[HTML][Forms][html:implicit-submission]")
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

  TEST_CASE(
    "HTMLFormElement: reset",
    "[HTML][Forms][html:dom-form-reset][html:reset-button-state-(type=reset)][html:concept-form-reset]")
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
