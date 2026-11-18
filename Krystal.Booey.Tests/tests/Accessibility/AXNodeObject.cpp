#include "Krystal.Booey/Accessibility/AXNodeObject.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AXTestUtils.hpp"
#include "Krystal.Booey.Tests/HTML/CustomElementTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/CustomElement/ElementInternals.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AXNodeObject: HTML elements' roles", "[Accessibility][AXNodeObject]")
  {
    AXFixture page(u8"<header id=header>H</header>"
                   u8"<main id=main>"
                   u8"<h2 id=heading>Title</h2>"
                   u8"<p id=paragraph>Text <a id=link href='#'>link</a></p>"
                   u8"<button id=button>Go</button>"
                   u8"<input id=text><input id=checkbox type=checkbox><input id=radio type=radio>"
                   u8"<input id=range type=range><input id=search type=search>"
                   u8"<input id=switch type=checkbox switch>"
                   u8"<textarea id=textarea></textarea>"
                   u8"<select id=select><option>A</option></select>"
                   u8"<select id=listbox multiple><option>A</option></select>"
                   u8"<ul id=list><li id=item>One</li></ul>"
                   u8"<section id=section>S</section>"
                   u8"<section id=named-section aria-label=Named>S</section>"
                   u8"<article><header id=section-header>H</header></article>"
                   u8"<hr id=rule><dialog id=dialog open>D</dialog>"
                   u8"</main>"
                   u8"<footer id=footer>F</footer>");

    CHECK(page.Object(u8"header").Role() == AXRole::LandmarkBanner);
    CHECK(page.Object(u8"main").Role() == AXRole::LandmarkMain);
    CHECK(page.Object(u8"heading").Role() == AXRole::Heading);
    CHECK(page.Object(u8"heading").HeadingLevel() == 2);
    CHECK(page.Object(u8"paragraph").Role() == AXRole::Paragraph);
    CHECK(page.Object(u8"link").Role() == AXRole::Link);
    CHECK(page.Object(u8"button").Role() == AXRole::Button);
    CHECK(page.Object(u8"text").Role() == AXRole::TextField);
    CHECK(page.Object(u8"checkbox").Role() == AXRole::Checkbox);
    CHECK(page.Object(u8"radio").Role() == AXRole::RadioButton);
    CHECK(page.Object(u8"range").Role() == AXRole::Slider);
    CHECK(page.Object(u8"search").Role() == AXRole::SearchField);
    CHECK(page.Object(u8"switch").Role() == AXRole::Switch);
    CHECK(page.Object(u8"textarea").Role() == AXRole::TextArea);
    CHECK(page.Object(u8"select").Role() == AXRole::PopUpButton);
    CHECK(page.Object(u8"listbox").Role() == AXRole::ListBox);
    CHECK(page.Object(u8"list").Role() == AXRole::List);
    CHECK(page.Object(u8"item").Role() == AXRole::ListItem);
    CHECK(page.Object(u8"section").Role() == AXRole::TextGroup);
    CHECK(page.Object(u8"named-section").Role() == AXRole::LandmarkRegion);
    CHECK(page.Object(u8"section-header").Role() == AXRole::SectionHeader);
    CHECK(page.Object(u8"rule").Role() == AXRole::HorizontalRule);
    CHECK(page.Object(u8"dialog").Role() == AXRole::ApplicationDialog);
    CHECK(page.Object(u8"footer").Role() == AXRole::LandmarkContentInfo);
  }

  TEST_CASE("AXNodeObject: ARIA roles", "[Accessibility][AXNodeObject]")
  {
    AXFixture page(u8"<div id=button role=button>Go</div>"
                   u8"<div id=toggle role=button aria-pressed=false>Go</div>"
                   u8"<div id=fallback role='bogus tab'>Tab</div>"
                   u8"<div id=textbox role=textbox aria-multiline=true></div>"
                   u8"<div id=region role=region>No name</div>"
                   u8"<div id=named-region role=region aria-label=Named>R</div>"
                   u8"<div id=presentation role=presentation aria-label=Labelled>P</div>"
                   u8"<div role=menu><div id=option role=option>O</div></div>"
                   u8"<div id=heading role=heading aria-level=4>H</div>"
                   u8"<div id=default-level role=heading>H</div>");

    CHECK(page.Object(u8"button").Role() == AXRole::Button);
    CHECK(page.Object(u8"button").AriaRoleAttribute() == AXRole::Button);
    CHECK(page.Object(u8"toggle").Role() == AXRole::ToggleButton);
    CHECK(page.Object(u8"fallback").Role() == AXRole::Tab);
    CHECK(page.Object(u8"textbox").Role() == AXRole::TextArea);
    CHECK(page.Object(u8"region").Role() == AXRole::Generic);
    CHECK(page.Object(u8"named-region").Role() == AXRole::LandmarkRegion);
    CHECK(page.Object(u8"presentation").AriaRoleAttribute() == AXRole::Unknown);
    CHECK(page.Object(u8"option").Role() == AXRole::MenuItem);
    CHECK(page.Object(u8"heading").HeadingLevel() == 4);
    CHECK(page.Object(u8"default-level").HeadingLevel() == 2);
  }

  TEST_CASE("AXNodeObject: the computed role string", "[Accessibility][AXNodeObject]")
  {
    AXFixture page(u8"<button id=button>Go</button><input id=text><h1 id=heading>H</h1>"
                   u8"<div id=toggle role=button aria-pressed=true>T</div>");
    CHECK(page.Object(u8"button").ComputedRoleString() == u8"button");
    CHECK(page.Object(u8"text").ComputedRoleString() == u8"textbox");
    CHECK(page.Object(u8"heading").ComputedRoleString() == u8"heading");
    CHECK(page.Object(u8"toggle").ComputedRoleString() == u8"button");
  }

  namespace
  {
    /// @brief A custom element that sets its default ARIA semantics through its internals, which it doesn't
    /// keep a reference to.
    class DefaultSemanticsElement : public html::CustomElement
    {
    public:
      explicit DefaultSemanticsElement(HTMLElement &element) noexcept : CustomElement(element)
      {
        auto internals = element.AttachInternals();
        REQUIRE_FALSE(internals.HasException());
        internals.Value()->Role(DOMString(u8"checkbox"));
        internals.Value()->AriaChecked(DOMString(u8"true"));
      }
    };
  }

  TEST_CASE("AXNodeObject: a custom element's default ARIA semantics",
            "[Accessibility][AXNodeObject][html:accessibility-semantics]")
  {
    html::tests::CustomElementFixture page;
    REQUIRE_FALSE(page.Registry()
                    .Define(u8"x-check", [](HTMLElement &element)
                            { return CreateUnique<DefaultSemanticsElement>(element); }, {})
                    .HasException());
    page.Parse(u8"<x-check id=check>Label</x-check><x-check id=overridden role=switch aria-checked=false>"
               u8"</x-check>");
    AXObjectCache::EnableAccessibility();
    (void)page.Document->UpdateLayout();
    auto *cache = page.Document->AxObjectCache();
    REQUIRE(cache);

    auto *object = cache->GetOrCreate(page.ById(u8"check"));
    REQUIRE(object);
    CHECK(object->Role() == AXRole::Checkbox);
    CHECK(object->IsChecked());
    CHECK(object->HasAttribute(u8"aria-checked"));
    CHECK_FALSE(page.ById(u8"check").HasAttribute(u8"aria-checked"));

    // The element's own attributes win over the defaults.
    auto *overridden = cache->GetOrCreate(page.ById(u8"overridden"));
    REQUIRE(overridden);
    CHECK(overridden->Role() == AXRole::Switch);
    CHECK_FALSE(overridden->IsChecked());
    AXObjectCache::DisableAccessibility();
  }

  TEST_CASE("AXNodeObject: states", "[Accessibility][AXNodeObject]")
  {
    AXFixture page(u8"<input id=checked type=checkbox checked>"
                   u8"<div id=aria-checked role=checkbox aria-checked=mixed>C</div>"
                   u8"<button id=disabled disabled>D</button>"
                   u8"<div aria-disabled=true><button id=aria-disabled>D</button></div>"
                   u8"<input id=required required>"
                   u8"<select id=multiple multiple><option>A</option></select>"
                   u8"<details id=details open><summary id=summary>S</summary>D</details>"
                   u8"<div id=expanded role=button aria-expanded=false>E</div>"
                   u8"<div id=pressed role=button aria-pressed=true>P</div>");

    CHECK(page.Object(u8"checked").IsChecked());
    CHECK(page.Object(u8"aria-checked").CheckboxOrRadioValue() == AXButtonState::Mixed);
    CHECK(page.Object(u8"aria-checked").IsIndeterminate());
    CHECK_FALSE(page.Object(u8"disabled").IsEnabled());
    CHECK_FALSE(page.Object(u8"aria-disabled").IsEnabled());
    CHECK(page.Object(u8"required").IsRequired());
    CHECK(page.Object(u8"multiple").IsMultiSelectable());
    CHECK(page.Object(u8"details").SupportsExpanded());
    CHECK(page.Object(u8"details").IsExpanded());
    CHECK(page.Object(u8"summary").IsExpanded());
    CHECK(page.Object(u8"expanded").SupportsExpanded());
    CHECK_FALSE(page.Object(u8"expanded").IsExpanded());
    CHECK(page.Object(u8"pressed").IsPressed());
  }

  TEST_CASE("AXNodeObject: expanded states", "[Accessibility][AXNodeObject]")
  {
    AXFixture page(u8"<button id=target popovertarget=popover>Open</button>"
                   u8"<button id=command commandfor=popover command=toggle-popover>Toggle</button>"
                   u8"<button id=dialog-command commandfor=dialog command=show-modal>Show</button>"
                   u8"<div id=popover popover>Popover</div><dialog id=dialog>D</dialog>"
                   u8"<div id=generic aria-expanded=true>G</div>"
                   u8"<div id=invalid role=button aria-expanded=maybe>I</div>");

    // A popover invoker's expanded state is whether the popover is showing.
    CHECK(page.Object(u8"target").SupportsExpanded());
    CHECK_FALSE(page.Object(u8"target").IsExpanded());
    CHECK(page.Object(u8"command").SupportsExpanded());
    REQUIRE_FALSE(Downcast<HTMLElement>(page.ById(u8"popover")).ShowPopover().HasException());
    CHECK(page.Object(u8"target").IsExpanded());
    CHECK(page.Object(u8"command").IsExpanded());
    // A command that isn't a popover's doesn't make it expandable.
    CHECK_FALSE(page.Object(u8"dialog-command").SupportsExpanded());

    // aria-expanded only applies to the roles that support it, with a valid value.
    CHECK_FALSE(page.Object(u8"generic").SupportsExpanded());
    CHECK_FALSE(page.Object(u8"generic").IsExpanded());
    CHECK_FALSE(page.Object(u8"invalid").SupportsExpanded());
  }

  TEST_CASE("AXNodeObject: an area's role follows its URL", "[Accessibility][AXNodeObject]")
  {
    AXFixture page(u8"<img src='data:,' usemap=#map width=20 height=20>"
                   u8"<map name=map><area id=area shape=rect coords=0,0,10,10></map>");
    auto &area = page.Object(u8"area");
    CHECK(area.Role() != AXRole::Link);
    REQUIRE_FALSE(page.ById(u8"area").SetAttribute(u8"href", u8"#target").HasException());
    (void)page.TakeNotifications();
    CHECK(area.Role() == AXRole::Link);
  }

  TEST_CASE("AXNodeObject: focus", "[Accessibility][AXNodeObject]")
  {
    AXFixture page(u8"<button id=button>B</button><div id=div>D</div><div id=focusable tabindex=0>F</div>");
    CHECK(page.Object(u8"button").CanSetFocusAttribute());
    CHECK_FALSE(page.Object(u8"div").CanSetFocusAttribute());
    CHECK(page.Object(u8"focusable").CanSetFocusAttribute());

    CHECK_FALSE(page.Object(u8"button").IsFocused());
    DynamicDowncast<HTMLElement>(page.ById(u8"button"))->Focus();
    CHECK(page.Object(u8"button").IsFocused());
  }

  TEST_CASE("AXNodeObject: accessible names", "[Accessibility][AXNodeObject][accname]")
  {
    AXFixture page(u8"<button id=content>Save <b>file</b></button>"
                   u8"<button id=aria-label aria-label='Close dialog'>X</button>"
                   u8"<span id=first>First</span><span id=second>Second</span>"
                   u8"<button id=labelledby aria-labelledby='second first' aria-label=Ignored>X</button>"
                   u8"<span id=hidden-label hidden>Hidden text</span>"
                   u8"<button id=hidden-labelledby aria-labelledby=hidden-label>X</button>"
                   u8"<label for=field>Name</label><input id=field>"
                   u8"<label>Email <input id=nested></label>"
                   u8"<input id=placeholder placeholder='Search here'>"
                   u8"<input id=titled title='Title text' placeholder=Placeholder>"
                   u8"<img id=image alt='A cat' src='data:,'>"
                   u8"<input id=submit type=submit><input id=reset type=reset value=Clear>"
                   u8"<fieldset id=fieldset><legend>Group</legend></fieldset>"
                   u8"<div id=title title=Tooltip></div>"
                   u8"<a id=link href='#'>Go <span aria-hidden=true>(hidden)</span>home</a>"
                   u8"<div id=paragraph>Not named from content</div>"
                   u8"<h1 id=heading>Heading <span style='display: none'>gone</span>text</h1>");

    CHECK(page.Object(u8"content").AccessibleName() == u8"Save file");
    CHECK(page.Object(u8"aria-label").AccessibleName() == u8"Close dialog");
    CHECK(page.Object(u8"labelledby").AccessibleName() == u8"Second First");
    CHECK(page.Object(u8"hidden-labelledby").AccessibleName() == u8"Hidden text");
    CHECK(page.Object(u8"field").AccessibleName() == u8"Name");
    CHECK(page.Object(u8"nested").AccessibleName() == u8"Email");
    CHECK(page.Object(u8"placeholder").AccessibleName() == u8"Search here");
    CHECK(page.Object(u8"titled").AccessibleName() == u8"Title text");
    CHECK(page.Object(u8"image").AccessibleName() == u8"A cat");
    CHECK(page.Object(u8"submit").AccessibleName() == u8"Submit");
    CHECK(page.Object(u8"reset").AccessibleName() == u8"Clear");
    CHECK(page.Object(u8"fieldset").AccessibleName() == u8"Group");
    CHECK(page.Object(u8"title").AccessibleName() == u8"Tooltip");
    CHECK(page.Object(u8"link").AccessibleName() == u8"Go home");
    CHECK(page.Object(u8"paragraph").AccessibleName().empty());
    CHECK(page.Object(u8"heading").AccessibleName() == u8"Heading text");
  }

  TEST_CASE("AXNodeObject: an embedded control's value is part of a name",
            "[Accessibility][AXNodeObject][accname]")
  {
    AXFixture page(u8"<label id=label>Flash <input id=times value=5> times</label>"
                   u8"<div id=check role=checkbox aria-labelledby=label></div>");
    CHECK(page.Object(u8"check").AccessibleName() == u8"Flash 5 times");
  }

  TEST_CASE("AXNodeObject: accessible descriptions", "[Accessibility][AXNodeObject][accname]")
  {
    AXFixture page(u8"<span id=help>Must be 8 characters</span>"
                   u8"<input id=password aria-label=Password aria-describedby=help>");
    CHECK(page.Object(u8"password").AccessibleDescription() == u8"Must be 8 characters");
  }

  TEST_CASE("AXNodeObject: descriptions from aria-description and title",
            "[Accessibility][AXNodeObject][accname]")
  {
    AXFixture page(u8"<button id=described aria-description='Saves the file'>Save</button>"
                   u8"<button id=titled title='Saves the file'>Save</button>"
                   u8"<button id=title-is-name title=Tooltip></button>"
                   u8"<span id=help>Help</span>"
                   u8"<button id=both aria-describedby=help aria-description=Ignored>B</button>");
    CHECK(page.Object(u8"described").AccessibleDescription() == u8"Saves the file");
    // The title describes the element when it isn't its name.
    CHECK(page.Object(u8"titled").AccessibleDescription() == u8"Saves the file");
    CHECK(page.Object(u8"title-is-name").AccessibleName() == u8"Tooltip");
    CHECK(page.Object(u8"title-is-name").AccessibleDescription().empty());
    CHECK(page.Object(u8"both").AccessibleDescription() == u8"Help");
  }

  TEST_CASE("AXNodeObject: a list box's chosen options are part of a name",
            "[Accessibility][AXNodeObject][accname]")
  {
    AXFixture page(u8"<label id=label>Fruit <div role=listbox>"
                   u8"<div role=option>Apple</div><div role=option aria-selected=true>Pear</div>"
                   u8"</div></label><div id=check role=checkbox aria-labelledby=label></div>");
    CHECK(page.Object(u8"check").AccessibleName() == u8"Fruit Pear");
  }

  TEST_CASE("AXNodeObject: generated content is part of a name", "[Accessibility][AXNodeObject][accname]")
  {
    AXFixture page(
      u8"<style>#gen::before { content: 'Hello ' } #gen::after { content: '!' }"
      u8"#alt::before { content: '*' / 'Starred' } #decorative::before { content: '*' / '' }</style>"
      u8"<button id=gen>World</button><button id=alt> file</button><button id=decorative>Plain</button>");
    CHECK(page.Object(u8"gen").AccessibleName() == u8"Hello World!");
    // CSS alt text stands in for the content.
    CHECK(page.Object(u8"alt").AccessibleName() == u8"Starred file");
    CHECK(page.Object(u8"decorative").AccessibleName() == u8"Plain");
  }

  TEST_CASE("AXNodeObject: the web area's name", "[Accessibility][AXNodeObject][accname]")
  {
    AXFixture page(u8"<title id=title>Document</title><p>Text</p>");
    auto *webArea = page.Cache().GetOrCreate(*page.Document);
    REQUIRE(webArea);
    CHECK(webArea->AccessibleName() == u8"Document");
    (void)page.TakeNotifications();

    // A new title is a text change of the web area.
    REQUIRE_FALSE(page.Document->Title(u8"Renamed").HasException());
    CHECK(AXFixture::WasPosted(page.TakeNotifications(), *webArea, AXNotification::TextChanged));
    CHECK(webArea->AccessibleName() == u8"Renamed");

    // The html element's aria-label comes first.
    REQUIRE_FALSE(
      page.Document->DocumentElement()->SetAttribute(u8"aria-label", u8"Labelled").HasException());
    CHECK(webArea->AccessibleName() == u8"Labelled");
  }

  TEST_CASE("AXNodeObject: values", "[Accessibility][AXNodeObject]")
  {
    AXFixture page(u8"<input id=text value=Typed><input id=password type=password value=secret>"
                   u8"<textarea id=textarea>Area</textarea>"
                   u8"<select id=select><option>A</option><option selected>B</option></select>");
    CHECK(page.Object(u8"text").StringValue() == u8"Typed");
    CHECK(page.Object(u8"password").StringValue().empty());
    CHECK(page.Object(u8"textarea").StringValue() == u8"Area");
    CHECK(page.Object(u8"select").StringValue() == u8"B");
  }

  TEST_CASE("AXNodeObject: values of combo boxes and labelled options", "[Accessibility][AXNodeObject]")
  {
    AXFixture page(u8"<div id=combo role=combobox><div role=listbox>"
                   u8"<div role=option>Apple</div><div role=option aria-selected=true>Pear</div>"
                   u8"</div></div>"
                   u8"<select id=labelled size=2><option>A</option>"
                   u8"<option selected aria-label='Banana fruit'>B</option></select>");
    CHECK(page.Object(u8"combo").StringValue() == u8"Pear");
    // A selected option's aria-label overrides its text.
    CHECK(page.Object(u8"labelled").StringValue() == u8"Banana fruit");
  }

  namespace
  {
    /// @brief The names of the objects under @p object with @p role, in order and separated by commas.
    KRYS_NODISCARD std::string NamesWithRole(AXObject &object, AXRole role) noexcept
    {
      std::string names;
      for (const auto &child : object.Children())
      {
        if (child->Role() == role)
        {
          auto name = child->AccessibleName();
          names += (names.empty() ? "" : ",") + std::string(name.begin(), name.end());
        }
        auto nested = NamesWithRole(*child, role);
        if (!nested.empty())
        {
          names += (names.empty() ? "" : ",") + nested;
        }
      }
      return names;
    }
  }

  TEST_CASE("AXNodeObject: form controls' internal parts are named", "[Accessibility][AXNodeObject]")
  {
    AXFixture page(u8"<input id=date type=date value=2026-11-05><input id=time type=time>"
                   u8"<input id=search type=search value=Query>");

    // A date or time input's fields are spin buttons named after their units, valued while they have one.
    CHECK(NamesWithRole(page.Object(u8"date"), AXRole::SpinButton) == "year,month,day");
    CHECK(NamesWithRole(page.Object(u8"time"), AXRole::SpinButton).starts_with("hour,minutes"));
    auto *dateShadow = page.ById(u8"date").UserAgentShadowRoot();
    REQUIRE(dateShadow);
    auto day = dateShadow->QuerySelector(u8"[aria-label=day]").Value();
    REQUIRE(day);
    CHECK(day->GetAttribute(u8"aria-valuenow") == u8"5");
    CHECK(day->GetAttribute(u8"aria-valuetext") == u8"05");
    auto *timeShadow = page.ById(u8"time").UserAgentShadowRoot();
    REQUIRE(timeShadow);
    auto hour = timeShadow->QuerySelector(u8"[aria-label=hour]").Value();
    REQUIRE(hour);
    CHECK_FALSE(hour->HasAttribute(u8"aria-valuenow"));

    // A search field's cancel button is named (for when its shadow tree is exposed; a text field's isn't).
    auto *searchShadow = page.ById(u8"search").UserAgentShadowRoot();
    REQUIRE(searchShadow);
    auto cancel = searchShadow->QuerySelector(u8"[role=button]").Value();
    REQUIRE(cancel);
    CHECK(cancel->GetAttribute(u8"aria-label") == u8"cancel");
  }
}
