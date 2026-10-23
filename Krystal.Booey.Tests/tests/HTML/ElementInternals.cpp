#include "Krystal.Booey/HTML/ElementInternals.hpp"
#include "Krystal.Booey.Tests/HTML/CustomElementTestUtils.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomStateSet.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLFieldSetElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlsCollection.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::html::tests
{
  namespace
  {
    /// @brief A custom element that attaches its internals when it's constructed, and logs its form
    /// callbacks.
    class InternalsElement : public CustomElement
    {
    private:
      Log &_log;

    public:
      RefPtr<ElementInternals> Internals;

      InternalsElement(HTMLElement &element, Log &log) noexcept : CustomElement(element), _log(log)
      {
        auto internals = element.AttachInternals();
        if (!internals.HasException())
        {
          Internals = internals.ReleaseValue();
        }
      }

      void FormAssociatedCallback(HTMLFormElement *form) noexcept override
      {
        _log.push_back(form ? u8"form " + Element().GetAttribute(u8"id").value_or(u8"") : u8"no form");
      }

      void FormResetCallback() noexcept override
      {
        _log.push_back(u8"reset");
      }

      void FormDisabledCallback(bool disabled) noexcept override
      {
        _log.push_back(disabled ? u8"disabled" : u8"enabled");
      }
    };

    struct InternalsFixture : CustomElementFixture
    {
      void DefineInternals(dom::DOMStringAtom name, CustomElementOptions &&options = {}) noexcept
      {
        auto result = Registry().Define(
          name, [this](HTMLElement &element) { return CreateUnique<InternalsElement>(element, Calls); },
          krys::move(options));
        REQUIRE_FALSE(result.HasException());
      }

      ElementInternals &InternalsOf(dom::DOMStringView id) noexcept
      {
        auto *behaviour = dynamic_cast<InternalsElement *>(ById(id).GetCustomElement());
        REQUIRE(behaviour);
        REQUIRE(behaviour->Internals);
        return *behaviour->Internals;
      }

      Ref<DOMFormData> Submitted(dom::DOMStringView formId) noexcept
      {
        auto formData = DOMFormData::Create(&Downcast<HTMLFormElement>(ById(formId)), nullptr);
        REQUIRE_FALSE(formData.HasException());
        return formData.ReleaseValue();
      }
    };
  }

  TEST_CASE_METHOD(InternalsFixture, "attachInternals", "[HTML][CustomElement][ElementInternals]")
  {
    DefineInternals(u8"x-plain");
    DefineInternals(u8"x-sealed", {.DisableInternals = true});
    Parse(u8"<div id=div></div><x-plain id=plain></x-plain><x-sealed id=sealed></x-sealed><x-later id=later>"
          u8"</x-later>");

    SECTION("only custom elements have internals, once")
    {
      CHECK(Downcast<HTMLElement>(ById(u8"div")).AttachInternals().HasException());
      CHECK(Downcast<HTMLElement>(ById(u8"later")).AttachInternals().HasException());
      (void)InternalsOf(u8"plain");
      auto again = Downcast<HTMLElement>(ById(u8"plain")).AttachInternals();
      REQUIRE(again.HasException());
      CHECK(again.GetException().Code() == dom::ExceptionCode::NotSupportedError);
    }

    SECTION("a definition can disable them")
    {
      auto *behaviour = dynamic_cast<InternalsElement *>(ById(u8"sealed").GetCustomElement());
      REQUIRE(behaviour);
      CHECK_FALSE(behaviour->Internals);
    }

    SECTION("an element that isn't form-associated has no form members")
    {
      auto &internals = InternalsOf(u8"plain");
      CHECK(internals.Element() == &ById(u8"plain"));
      CHECK(internals.Form().HasException());
      CHECK(internals.SetFormValue(dom::DOMString(u8"1")).HasException());
      CHECK(internals.WillValidate().HasException());
      CHECK(internals.Labels().HasException());
    }

    SECTION("the shadow root, even closed, once attached as a custom element")
    {
      auto &element = ById(u8"plain");
      REQUIRE_FALSE(element.AttachShadow({.Mode = dom::ShadowRootMode::Closed}).HasException());
      CHECK(element.ShadowRoot() == nullptr);
      CHECK(InternalsOf(u8"plain").ShadowRoot() != nullptr);
    }
  }

  TEST_CASE_METHOD(InternalsFixture, "Custom states and :state()", "[HTML][CustomElement][ElementInternals]")
  {
    DefineInternals(u8"x-toggle");
    Parse(u8"<style>x-toggle:state(off) { visibility: hidden }</style><x-toggle id=t>text</x-toggle>");
    auto &element = ById(u8"t");
    auto &states = InternalsOf(u8"t").States();

    (void)Document->UpdateLayout();
    REQUIRE(element.Renderer());
    CHECK(element.Renderer()->Style().GetVisibility() == Visibility::Visible);

    CHECK(states.Add(u8"off"));
    CHECK_FALSE(states.Add(u8"off"));
    CHECK(states.Has(u8"off"));
    CHECK(element.Matches(u8":state(off)").ReleaseValue());
    CHECK_FALSE(element.Matches(u8":state(on)").ReleaseValue());
    (void)Document->UpdateLayout();
    CHECK(element.Renderer()->Style().GetVisibility() == Visibility::Hidden);

    CHECK(states.Delete(u8"off"));
    CHECK_FALSE(element.Matches(u8":state(off)").ReleaseValue());
    (void)Document->UpdateLayout();
    CHECK(element.Renderer()->Style().GetVisibility() == Visibility::Visible);

    (void)states.Add(u8"a");
    (void)states.Add(u8"b");
    CHECK(states.Size() == 2);
    states.Clear();
    CHECK(states.Size() == 0);
  }

  TEST_CASE_METHOD(InternalsFixture, "Form-associated custom elements",
                   "[HTML][CustomElement][ElementInternals]")
  {
    DefineInternals(u8"x-rating", {.FormAssociated = true});
    Parse(u8"<form id=f><label id=l for=a>Rating</label><x-rating id=a name=rating></x-rating>"
          u8"<fieldset id=fs><x-rating id=b name=inner></x-rating></fieldset></form><x-rating "
          u8"id=out></x-rating>");
    auto &form = Downcast<HTMLFormElement>(ById(u8"f"));

    SECTION("they join their form, as listed elements")
    {
      CHECK(TakeCalls() == Log {u8"form a", u8"form b"});
      CHECK(InternalsOf(u8"a").Form().Value().get() == &form);
      CHECK(InternalsOf(u8"out").Form().Value() == nullptr);
      CHECK(form.Elements()->Length() == 3);
    }

    SECTION("they submit their form values")
    {
      REQUIRE_FALSE(InternalsOf(u8"a").SetFormValue(dom::DOMString(u8"4")).HasException());
      auto entries = DOMFormData::Create();
      entries->Append(u8"x", u8"1");
      entries->Append(u8"y", u8"2");
      REQUIRE_FALSE(InternalsOf(u8"b").SetFormValue(RefPtr<DOMFormData>(entries)).HasException());
      entries->Append(u8"z", u8"3");

      auto submitted = Submitted(u8"f");
      REQUIRE(submitted->Items().size() == 3);
      CHECK(submitted->Get(u8"rating") == u8"4");
      CHECK(submitted->Get(u8"x") == u8"1");
      CHECK(submitted->Get(u8"y") == u8"2");
      CHECK_FALSE(submitted->Has(u8"z"));
    }

    SECTION("they set their validity")
    {
      auto &internals = InternalsOf(u8"a");
      CHECK(internals.WillValidate().Value());
      CHECK(internals.CheckValidity().Value());
      CHECK(ById(u8"a").Matches(u8":valid").ReleaseValue());

      auto noMessage = internals.SetValidity({.ValueMissing = true}, u8"");
      REQUIRE(noMessage.HasException());
      CHECK(noMessage.GetException().Code() == dom::ExceptionCode::TypeError);

      REQUIRE_FALSE(internals.SetValidity({.ValueMissing = true}, u8"Pick a rating").HasException());
      CHECK(internals.Validity().Value().ValueMissing());
      CHECK_FALSE(internals.Validity().Value().Valid());
      CHECK(internals.ValidationMessage().Value() == u8"Pick a rating");
      CHECK_FALSE(internals.CheckValidity().Value());
      CHECK(ById(u8"a").Matches(u8":invalid").ReleaseValue());
      CHECK_FALSE(form.CheckValidity());

      auto outside =
        internals.SetValidity({.CustomError = true}, u8"Elsewhere", &Downcast<HTMLElement>(ById(u8"b")));
      REQUIRE(outside.HasException());
      CHECK(outside.GetException().Code() == dom::ExceptionCode::NotFoundError);

      REQUIRE_FALSE(internals.SetValidity().HasException());
      CHECK(internals.CheckValidity().Value());
      CHECK(internals.ValidationMessage().Value().empty());
    }

    SECTION("a disabled fieldset disables them")
    {
      (void)TakeCalls();
      Downcast<HTMLFieldSetElement>(ById(u8"fs")).Disabled(true);
      CHECK(TakeCalls() == Log {u8"disabled"});
      CHECK(ById(u8"b").Matches(u8":disabled").ReleaseValue());
      CHECK_FALSE(InternalsOf(u8"b").WillValidate().Value());

      (void)ById(u8"a").SetAttribute(u8"disabled", u8"");
      CHECK(TakeCalls() == Log {u8"disabled"});
      ById(u8"a").RemoveAttribute(u8"disabled");
      CHECK(TakeCalls() == Log {u8"enabled"});
    }

    SECTION("resetting the form resets them")
    {
      (void)TakeCalls();
      form.Reset();
      CHECK(TakeCalls() == Log {u8"reset", u8"reset"});
    }

    SECTION("they're labelable")
    {
      auto labels = InternalsOf(u8"a").Labels();
      REQUIRE_FALSE(labels.HasException());
      REQUIRE(labels.Value());
      CHECK(labels.Value()->Length() == 1);
    }

    SECTION("moving them out of the form")
    {
      (void)TakeCalls();
      REQUIRE_FALSE(Body().AppendChild(ById(u8"a")).HasException());
      CHECK(TakeCalls() == Log {u8"no form"});
    }
  }

  TEST_CASE_METHOD(InternalsFixture, "Form-associated custom elements upgraded late",
                   "[HTML][CustomElement][ElementInternals]")
  {
    Parse(u8"<form id=f><x-late id=a name=late disabled></x-late></form>");
    auto &form = Downcast<HTMLFormElement>(ById(u8"f"));
    CHECK(form.Elements()->Length() == 0);

    DefineInternals(u8"x-late", {.FormAssociated = true});
    CHECK(TakeCalls() == Log {u8"form a", u8"disabled"});
    CHECK(form.Elements()->Length() == 1);
    CHECK(InternalsOf(u8"a").Form().Value().get() == &form);
    CHECK(ById(u8"a").Matches(u8":disabled").ReleaseValue());
  }
}
