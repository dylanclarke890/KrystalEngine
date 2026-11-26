#include "Krystal.Booey/Accessibility/ARIAMixin.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Dom/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/Dom/Nodes/Element.hpp"
#include "Krystal.Booey/Dom/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::ax::mixins::tests
{
  namespace
  {
    /// @brief A document whose body holds a `div#host` and the elements it refers to, `span#a` and `span#b`.
    struct ReflectionFixture
    {
      Ref<boo::Document> Document {CreateRef<HTMLDocument>()};
      Ref<Element> Host {Create(u8"div", u8"host")};
      Ref<Element> A {Create(u8"span", u8"a")};
      Ref<Element> B {Create(u8"span", u8"b")};

      ReflectionFixture() noexcept
      {
        auto html = Document->CreateElement(u8"html").ReleaseValue();
        auto body = Document->CreateElement(u8"body").ReleaseValue();
        REQUIRE_FALSE(Document->AppendChild(*html).HasException());
        REQUIRE_FALSE(html->AppendChild(*body).HasException());
        REQUIRE_FALSE(body->AppendChild(*Host).HasException());
        REQUIRE_FALSE(body->AppendChild(*A).HasException());
        REQUIRE_FALSE(body->AppendChild(*B).HasException());
      }

      KRYS_NODISCARD Ref<Element> Create(DOMStringAtom localName, DOMStringView id) noexcept
      {
        auto element = Document->CreateElement(krys::move(localName)).ReleaseValue();
        REQUIRE_FALSE(element->SetAttribute(u8"id", DOMString(id)).HasException());
        return element;
      }
    };

    /// @brief The elements of @p elements, null if it is.
    KRYS_NODISCARD Maybe<List<const Element *>> Raw(const Maybe<List<Ref<Element>>> &elements) noexcept
    {
      if (!elements)
      {
        return null;
      }
      List<const Element *> raw;
      for (auto &element : *elements)
      {
        raw.push_back(element.get());
      }
      return raw;
    }
  }

  TEST_CASE_METHOD(ReflectionFixture, "ARIAMixin: string attributes reflect their content attributes",
                   "[Accessibility][ARIAMixin]")
  {
    SECTION("null while the attribute is absent")
    {
      CHECK_FALSE(Host->Role());
      CHECK_FALSE(Host->AriaLabel());
    }

    SECTION("the attribute's value, empty or not")
    {
      REQUIRE_FALSE(Host->SetAttribute(u8"aria-label", u8"Close").HasException());
      REQUIRE_FALSE(Host->SetAttribute(u8"role", u8"").HasException());
      CHECK(Host->AriaLabel() == u8"Close");
      CHECK(Host->Role() == u8"");
    }

    SECTION("setting sets the attribute, and null removes it")
    {
      Host->Role(DOMString(u8"button"));
      Host->AriaPressed(DOMString(u8"mixed"));
      CHECK(Host->GetAttribute(u8"role") == u8"button");
      CHECK(Host->GetAttribute(u8"aria-pressed") == u8"mixed");
      Host->AriaPressed(null);
      CHECK_FALSE(Host->HasAttribute(u8"aria-pressed"));
    }

    SECTION("on elements in any namespace")
    {
      auto svg = Document->CreateElementNS(infra::namespaces::SVG, u8"rect").ReleaseValue();
      svg->AriaHidden(DOMString(u8"true"));
      CHECK(svg->GetAttribute(u8"aria-hidden") == u8"true");
      CHECK(svg->AriaHidden() == u8"true");
    }
  }

  TEST_CASE_METHOD(ReflectionFixture, "ARIAMixin: ariaActiveDescendantElement",
                   "[Accessibility][ARIAMixin][html:attr-associated-element]")
  {
    SECTION("the element whose ID the attribute holds")
    {
      REQUIRE_FALSE(Host->SetAttribute(u8"aria-activedescendant", u8"b").HasException());
      CHECK(Host->AriaActiveDescendantElement().get() == B.get());
      REQUIRE_FALSE(Host->SetAttribute(u8"aria-activedescendant", u8"missing").HasException());
      CHECK_FALSE(Host->AriaActiveDescendantElement());
    }

    SECTION("an element set explicitly, until the attribute changes")
    {
      Host->AriaActiveDescendantElement(A.get());
      CHECK(Host->GetAttribute(u8"aria-activedescendant") == u8"");
      CHECK(Host->AriaActiveDescendantElement().get() == A.get());
      REQUIRE_FALSE(Host->SetAttribute(u8"aria-activedescendant", u8"b").HasException());
      CHECK(Host->AriaActiveDescendantElement().get() == B.get());
    }

    SECTION("an element set explicitly that leaves the tree isn't returned")
    {
      Host->AriaActiveDescendantElement(A.get());
      REQUIRE_FALSE(A->Remove().HasException());
      CHECK_FALSE(Host->AriaActiveDescendantElement());
    }

    SECTION("null removes the attribute")
    {
      Host->AriaActiveDescendantElement(A.get());
      Host->AriaActiveDescendantElement(nullptr);
      CHECK_FALSE(Host->HasAttribute(u8"aria-activedescendant"));
      CHECK_FALSE(Host->AriaActiveDescendantElement());
    }
  }

  TEST_CASE_METHOD(ReflectionFixture, "ARIAMixin: element list attributes",
                   "[Accessibility][ARIAMixin][html:attr-associated-elements]")
  {
    SECTION("null while the attribute is absent")
    {
      CHECK_FALSE(Host->AriaLabelledByElements());
    }

    SECTION("the elements whose IDs the attribute holds, skipping the missing ones")
    {
      REQUIRE_FALSE(Host->SetAttribute(u8"aria-labelledby", u8" b missing\ta ").HasException());
      CHECK(Raw(Host->AriaLabelledByElements()) == List<const Element *> {B.get(), A.get()});
      REQUIRE_FALSE(Host->SetAttribute(u8"aria-describedby", u8"").HasException());
      CHECK(Raw(Host->AriaDescribedByElements()) == List<const Element *> {});
    }

    SECTION("only elements in the element's tree")
    {
      auto fragment = Document->CreateDocumentFragment();
      auto detached = Create(u8"span", u8"c");
      REQUIRE_FALSE(fragment->AppendChild(*detached).HasException());
      REQUIRE_FALSE(Host->SetAttribute(u8"aria-owns", u8"c a").HasException());
      CHECK(Raw(Host->AriaOwnsElements()) == List<const Element *> {A.get()});
    }

    SECTION("elements set explicitly, until the attribute changes")
    {
      Host->AriaControlsElements(List<Ref<Element>> {B, A});
      CHECK(Host->GetAttribute(u8"aria-controls") == u8"");
      CHECK(Raw(Host->AriaControlsElements()) == List<const Element *> {B.get(), A.get()});
      REQUIRE_FALSE(Host->SetAttribute(u8"aria-controls", u8"a").HasException());
      CHECK(Raw(Host->AriaControlsElements()) == List<const Element *> {A.get()});
    }

    SECTION("elements set explicitly that leave the tree aren't returned")
    {
      Host->AriaFlowToElements(List<Ref<Element>> {A, B});
      REQUIRE_FALSE(A->Remove().HasException());
      CHECK(Raw(Host->AriaFlowToElements()) == List<const Element *> {B.get()});
    }

    SECTION("null removes the attribute")
    {
      Host->AriaDetailsElements(List<Ref<Element>> {A});
      Host->AriaDetailsElements(null);
      CHECK_FALSE(Host->HasAttribute(u8"aria-details"));
      CHECK_FALSE(Host->AriaDetailsElements());
    }
  }
}
