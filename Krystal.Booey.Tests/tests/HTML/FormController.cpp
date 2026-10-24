#include "Krystal.Booey/HTML/FormController.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/LoadTestUtils.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/History/BackForwardController.hpp"
#include "Krystal.Booey/History/HistoryItem.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElement.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/ElementInternals.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"

namespace krys::boo::tests
{
  namespace
  {
    constexpr utf8_stringview FormPage = u8"https://example.com/form.html";
    constexpr utf8_stringview NextPage = u8"https://example.com/next.html";

    struct FormStateFixture : LoadFixture
    {
      explicit FormStateFixture(utf8_stringview markup) noexcept
      {
        Strategy.AddText(FormPage, markup, u8"text/html");
        Strategy.AddText(NextPage, u8"<!DOCTYPE html><p>Next", u8"text/html");
      }

      void Visit(utf8_stringview url) noexcept
      {
        LoadURL(url);
        Run();
      }

      static void Run() noexcept
      {
        html::EventLoop::Current().RunUntilIdle();
      }

      void GoBack() noexcept
      {
        REQUIRE(TestPage->BackForward().GoBack());
        Run();
      }

      KRYS_NODISCARD html::HTMLInputElement &Input(dom::DOMStringView id) const noexcept
      {
        auto *input = DynamicDowncast<html::HTMLInputElement>(Document().GetElementById(id).get());
        REQUIRE(input);
        return *input;
      }

      KRYS_NODISCARD html::HTMLSelectElement &Select(dom::DOMStringView id) const noexcept
      {
        auto *select = DynamicDowncast<html::HTMLSelectElement>(Document().GetElementById(id).get());
        REQUIRE(select);
        return *select;
      }

      KRYS_NODISCARD html::HTMLOptionElement &Option(dom::DOMStringView id) const noexcept
      {
        auto *option = DynamicDowncast<html::HTMLOptionElement>(Document().GetElementById(id).get());
        REQUIRE(option);
        return *option;
      }

      KRYS_NODISCARD html::HTMLTextAreaElement &TextArea(dom::DOMStringView id) const noexcept
      {
        auto *textArea = DynamicDowncast<html::HTMLTextAreaElement>(Document().GetElementById(id).get());
        REQUIRE(textArea);
        return *textArea;
      }
    };

    constexpr utf8_stringview ControlsMarkup = u8R"(<!DOCTYPE html>
<form action=submit.html?session=1>
  <input id=text name=text value=initial>
  <input id=check type=checkbox name=check>
  <input id=r1 type=radio name=r value=1 checked><input id=r2 type=radio name=r value=2>
  <select id=one name=one><option id=a>a<option id=b>b<option id=c>c</select>
  <select id=many name=many multiple><option id=ma>a<option id=mb>b<option id=mc>c</select>
  <textarea id=area name=area>initial</textarea>
  <input id=hidden type=hidden name=hidden value=initial>
  <input id=password type=password name=password>
  <input id=submit type=submit name=submit value=Go>
</form>
<form autocomplete=off><input id=off name=off></form>
<input id=outside name=outside>)";
  }

  TEST_CASE("Going back restores the form controls' state", "[HTML][FormController]")
  {
    FormStateFixture fixture(ControlsMarkup);
    fixture.Visit(FormPage);

    (void)fixture.Input(u8"text").SetValue(u8"typed");
    fixture.Input(u8"check").SetChecked(true);
    fixture.Input(u8"r2").SetChecked(true);
    fixture.Option(u8"b").Selected(true);
    fixture.Option(u8"ma").Selected(true);
    fixture.Option(u8"mc").Selected(true);
    (void)fixture.TextArea(u8"area").SetValue(u8"typed");
    dom::ElementAlgorithms::SetAttributeValue(fixture.Input(u8"hidden"), dom::DOMStringAtom(u8"value"),
                                              dom::DOMString(u8"changed"));
    (void)fixture.Input(u8"password").SetValue(u8"secret");
    (void)fixture.Input(u8"off").SetValue(u8"typed");
    (void)fixture.Input(u8"outside").SetValue(u8"typed");

    SECTION("going back")
    {
      fixture.Visit(NextPage);
      CHECK_FALSE(fixture.TestPage->BackForward().BackItem()->DocumentState().empty());
      fixture.GoBack();

      CHECK(fixture.Input(u8"text").Value() == u8"typed");
      CHECK(fixture.Input(u8"check").Checked());
      CHECK(fixture.Input(u8"r2").Checked());
      CHECK_FALSE(fixture.Input(u8"r1").Checked());
      CHECK(fixture.Select(u8"one").Value() == u8"b");
      CHECK(fixture.Option(u8"ma").Selected());
      CHECK_FALSE(fixture.Option(u8"mb").Selected());
      CHECK(fixture.Option(u8"mc").Selected());
      CHECK(fixture.TextArea(u8"area").Value() == u8"typed");
      CHECK(fixture.Input(u8"hidden").Value() == u8"changed");
      CHECK(fixture.Input(u8"submit").Value() == u8"Go");

      // Password fields and forms with autocomplete=off aren't saved.
      CHECK(fixture.Input(u8"password").Value().empty());
      CHECK(fixture.Input(u8"off").Value().empty());

      // Controls without a form are matched by their name and type too.
      CHECK(fixture.Input(u8"outside").Value() == u8"typed");

      // The state was given out.
      CHECK_FALSE(fixture.Document().GetFormController().HasFormStateToRestore());
    }

    SECTION("a reload doesn't restore it")
    {
      fixture.Frame().Loader().Reload();
      fixture.Run();
      CHECK(fixture.Input(u8"text").Value() == u8"initial");
      CHECK_FALSE(fixture.Input(u8"check").Checked());
      CHECK(fixture.Input(u8"r1").Checked());
    }

    SECTION("loading the page again doesn't restore it")
    {
      fixture.Visit(NextPage);
      fixture.Visit(FormPage);
      CHECK(fixture.Input(u8"text").Value() == u8"initial");
      CHECK(fixture.Input(u8"r1").Checked());
    }
  }

  TEST_CASE("The form state serialization", "[HTML][FormController]")
  {
    FormStateFixture fixture(u8"<!DOCTYPE html><form><input id=a name=a></form><input id=b name=b value=x>");
    fixture.Visit(FormPage);
    (void)fixture.Input(u8"a").SetValue(u8"1");

    auto state = fixture.Document().FormElementsState();
    REQUIRE(state.size() == 12);
    // The form key: the action without its query, its first named controls, and the form's index.
    CHECK(state[1] == u8"https://example.com/form.html [a ] #0");
    CHECK(state[2] == u8"1");
    CHECK(state[3] == u8"a");
    CHECK(state[4] == u8"text");
    CHECK(state[5] == u8"1");
    CHECK(state[6] == u8"1");
    // An unchanged control saves no state.
    CHECK(state[7] == u8"No owner");
    CHECK(state[8] == u8"1");
    CHECK(state[9] == u8"b");
    CHECK(state[10] == u8"text");
    CHECK(state[11] == u8"0");

    SECTION("a corrupt state restores nothing")
    {
      auto &controller = fixture.Document().GetFormController();
      controller.SetStateForNewFormElements({dom::DOMString(u8"not a signature")});
      CHECK_FALSE(controller.HasFormStateToRestore());
      state[4] = u8"Not A Type";
      controller.SetStateForNewFormElements(state);
      CHECK_FALSE(controller.HasFormStateToRestore());
    }
  }

  namespace
  {
    /// @brief A form-associated custom element whose state is its `value` attribute, and which logs the state
    /// it's given back.
    class StatefulElement final : public html::CustomElement
    {
    private:
      List<std::string> &_log;
      RefPtr<html::ElementInternals> _internals;

    public:
      StatefulElement(html::HTMLElement &element, List<std::string> &log) noexcept
          : CustomElement(element), _log(log)
      {
        auto internals = element.AttachInternals();
        REQUIRE_FALSE(internals.HasException());
        _internals = internals.ReleaseValue();
      }

      void SetState(html::CustomElementFormValue &&state) noexcept
      {
        REQUIRE_FALSE(
          _internals->SetFormValue(html::CustomElementFormValue(nullptr), krys::move(state)).HasException());
      }

      void FormStateRestoreCallback(const html::CustomElementFormValue &state,
                                    html::FormStateRestoreMode mode) noexcept override
      {
        std::string entry = mode == html::FormStateRestoreMode::Restore ? "restore" : "autocomplete";
        if (auto *string = std::get_if<dom::DOMString>(&state))
        {
          entry += " " + Narrow(*string);
        }
        else if (auto *formData = std::get_if<RefPtr<html::DOMFormData>>(&state))
        {
          for (auto &item : (*formData)->Items())
          {
            entry += " " + Narrow(item.Name) + "=" + Narrow(item.Data);
          }
        }
        _log.push_back(entry);
      }
    };
  }

  TEST_CASE("Form-associated custom elements are given their state back", "[HTML][FormController]")
  {
    FormStateFixture fixture(
      u8"<!DOCTYPE html><form><state-element id=one name=one></state-element><state-element id=two "
      u8"name=two></state-element></form>");
    List<std::string> log;
    fixture.Client->OnClearWindowObject = [&]
    {
      auto &registry = fixture.Document().Window()->EnsureCustomElementRegistry();
      auto result =
        registry.Define(u8"state-element", [&](html::HTMLElement &element)
                        { return CreateUnique<StatefulElement>(element, log); }, {.FormAssociated = true});
      REQUIRE_FALSE(result.HasException());
    };
    fixture.Visit(FormPage);

    auto behaviour = [&](dom::DOMStringView id) -> StatefulElement &
    {
      auto *element =
        dynamic_cast<StatefulElement *>(fixture.Document().GetElementById(id)->GetCustomElement());
      REQUIRE(element);
      return *element;
    };
    behaviour(u8"one").SetState(dom::DOMString(u8"saved"));
    auto entries = html::DOMFormData::Create();
    entries->Append(u8"x", u8"1");
    entries->Append(u8"y", u8"2");
    behaviour(u8"two").SetState(RefPtr<html::DOMFormData>(entries));

    fixture.Visit(NextPage);
    fixture.GoBack();
    CHECK(log == List<std::string> {"restore saved", "restore x=1 y=2"});
  }
}
