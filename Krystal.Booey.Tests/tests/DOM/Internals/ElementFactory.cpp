#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLDivElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLUnknownElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/MathML/MathMLElement.hpp"
#include "Krystal.Booey/MathML/MathMLUnknownElement.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGUnknownElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    /// @brief Whether @p element is a @p TInterface, through its namespace's base interface @p TBase.
    template <typename TInterface, typename TBase>
    KRYS_NODISCARD bool ElementIs(Element &element) noexcept
    {
      auto *base = DynamicDowncast<TBase>(element);
      return base != nullptr && Is<TInterface>(*base);
    }
  }

  TEST_CASE("Create an element: the element interface for the local name and namespace is case-sensitive",
            "[DOM][ElementFactory][dom:concept-create-element]")
  {
    auto document = CreateRef<HTMLDocument>();

    SECTION("An uppercase name in the HTML namespace is unknown")
    {
      auto element =
        document->CreateElementNS(DOMStringAtom(infra::Namespaces::HTML), DOMStringAtom(u8"DIV"));
      REQUIRE_FALSE(element.HasException());
      auto *htmlElement = DynamicDowncast<html::HTMLElement>(*element.Value());
      REQUIRE(htmlElement != nullptr);
      CHECK(Is<html::HTMLUnknownElement>(*htmlElement));
    }

    SECTION("A lowercase name in the HTML namespace is its element")
    {
      auto element =
        document->CreateElementNS(DOMStringAtom(infra::Namespaces::HTML), DOMStringAtom(u8"div"));
      REQUIRE_FALSE(element.HasException());
      auto *htmlElement = DynamicDowncast<html::HTMLElement>(*element.Value());
      REQUIRE(htmlElement != nullptr);
      CHECK(Is<html::HTMLDivElement>(*htmlElement));
    }

    SECTION("createElement lowercases the name in an HTML document")
    {
      auto element = document->CreateElement(DOMStringAtom(u8"DIV"));
      REQUIRE_FALSE(element.HasException());
      auto *htmlElement = DynamicDowncast<html::HTMLElement>(*element.Value());
      REQUIRE(htmlElement != nullptr);
      CHECK(Is<html::HTMLDivElement>(*htmlElement));
      CHECK(element.Value()->LocalName() == u8"div");
    }
  }

  TEST_CASE(
    "Create an element: without a definition, it's uncustomized, or undefined for a custom name",
    "[DOM][ElementFactory][dom:concept-create-element][dom:create-an-element-internal]"
    "[dom:concept-element-custom-element-state][dom:concept-element-defined][dom:concept-element-custom]"
    "[dom:concept-element-is-value]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto registry = html::CustomElementRegistry::Create(*document);

    SECTION("The namespace, prefix, local name and registry it's given")
    {
      auto element =
        ElementFactory::Create(*document, QualifiedName {infra::Namespaces::HTML, u8"h", u8"div"},
                               DOMStringAtom::Null(), false, RefPtr<html::CustomElementRegistry>(registry));
      CHECK(ElementIs<html::HTMLDivElement, html::HTMLElement>(*element));
      CHECK(element->NamespaceURI() == infra::Namespaces::HTML);
      CHECK(element->Prefix() == u8"h");
      CHECK(element->LocalName() == u8"div");
      CHECK(element->CustomElementRegistry() == registry);
      CHECK(&element->NodeDocument() == document.get());
      CHECK_FALSE(element->HasAttributes());
      CHECK(element->GetCustomElementState() == CustomElementState::Uncustomized);
      CHECK(ElementAlgorithms::IsDefined(*element));
      CHECK_FALSE(ElementAlgorithms::IsCustom(*element));
    }

    SECTION("The registry defaults to the document's")
    {
      document->SetCustomElementRegistry(RefPtr<html::CustomElementRegistry>(registry));
      auto element = ElementFactory::Create(
        *document, QualifiedName {infra::Namespaces::HTML, DOMStringAtom::Null(), u8"div"});
      CHECK(element->CustomElementRegistry() == registry);
    }

    SECTION("A valid custom element name in the HTML namespace is undefined")
    {
      auto element = ElementFactory::Create(
        *document, QualifiedName {infra::Namespaces::HTML, DOMStringAtom::Null(), u8"x-a"});
      CHECK(element->GetCustomElementState() == CustomElementState::Undefined);
      CHECK_FALSE(ElementAlgorithms::IsDefined(*element));
      CHECK_FALSE(ElementAlgorithms::IsCustom(*element));

      auto svg = ElementFactory::Create(
        *document, QualifiedName {infra::Namespaces::SVG, DOMStringAtom::Null(), u8"x-a"});
      CHECK(ElementIs<svg::SVGUnknownElement, svg::SVGElement>(*svg));
      CHECK(svg->GetCustomElementState() == CustomElementState::Uncustomized);
    }

    SECTION("Unknown names give the namespace's unknown element, or Element")
    {
      CHECK(ElementIs<html::HTMLUnknownElement, html::HTMLElement>(*ElementFactory::Create(
        *document, QualifiedName {infra::Namespaces::HTML, DOMStringAtom::Null(), u8"nope"})));
      CHECK(ElementIs<mathml::MathMLUnknownElement, mathml::MathMLElement>(*ElementFactory::Create(
        *document, QualifiedName {infra::Namespaces::MathML, DOMStringAtom::Null(), u8"nope"})));
      auto element =
        ElementFactory::Create(*document, QualifiedName {u8"urn:x", DOMStringAtom::Null(), u8"nope"});
      CHECK_FALSE(Is<html::HTMLElement>(*element));
      CHECK(element->NamespaceURI() == u8"urn:x");
    }
  }
}
