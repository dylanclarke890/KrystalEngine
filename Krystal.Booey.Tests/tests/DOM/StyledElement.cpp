#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/Properties/ImmutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/MutationObserver.hpp"
#include "Krystal.Booey/DOM/MutationRecord.hpp"
#include "Krystal.Booey/DOM/NamedNodeMap.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/MathML/MathMLElement.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    using css::tests::ToNarrow;

    Ref<HTMLDocument> Parse(utf8_string &&input) noexcept
    {
      auto document = CreateRef<HTMLDocument>();
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    Ref<StyledElement> AsStyled(Node &node) noexcept
    {
      return ShareRef(Downcast<StyledElement>(node));
    }

    Ref<StyledElement> CreateDiv(Document &document) noexcept
    {
      return AsStyled(*document.CreateElement(u8"div").ReleaseValue());
    }

    std::string StyleAttribute(const Element &element) noexcept
    {
      auto value = element.GetAttribute(u8"style");
      return value ? ToNarrow(*value) : std::string("(null)");
    }

    std::string PropertyValue(css::CSSStyleDeclaration &style, css::CSSOMStringView name) noexcept
    {
      return ToNarrow(style.GetPropertyValue(css::CSSOMString(name)));
    }

    struct Observed
    {
      Ref<MutationObserver> Observer;

      explicit Observed(Node &target)
          : Observer(
              CreateRef<MutationObserver>([](const List<Ref<MutationRecord>> &, MutationObserver &) {}))
      {
        REQUIRE_FALSE(
          Observer->Observe(target, MutationObserverInit {.Attributes = true, .AttributeOldValue = true})
            .HasException());
      }
    };
  }

  TEST_CASE("StyledElement: element types", "[DOM][StyledElement]")
  {
    auto document = CreateRef<HTMLDocument>();

    SECTION("HTML, SVG and MathML elements have an inline style")
    {
      auto html = document->CreateElement(u8"div").ReleaseValue();
      auto svg = document->CreateElementNS(infra::Namespaces::SVG, u8"svg").ReleaseValue();
      auto math = document->CreateElementNS(infra::Namespaces::MathML, u8"math").ReleaseValue();

      REQUIRE(Is<StyledElement>(static_cast<Node &>(*html)));
      REQUIRE(Is<StyledElement>(static_cast<Node &>(*svg)));
      REQUIRE(Is<StyledElement>(static_cast<Node &>(*math)));
      REQUIRE(Is<svg::SVGElement>(static_cast<Node &>(*svg)));
      REQUIRE(Is<mathml::MathMLElement>(static_cast<Node &>(*math)));
      REQUIRE(math->NamespaceURI() == infra::Namespaces::MathML);
      REQUIRE(math->LocalName() == u8"math");
    }

    SECTION("Elements in other namespaces don't")
    {
      auto other = document->CreateElementNS(u8"urn:other", u8"x").ReleaseValue();
      REQUIRE_FALSE(Is<StyledElement>(static_cast<Node &>(*other)));

      REQUIRE_FALSE(other->SetAttribute(u8"style", u8"color: red").HasException());
      REQUIRE(StyleAttribute(*other) == "color: red");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "StyledElement: the style attribute", "[DOM][StyledElement]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = CreateDiv(*document);

    SECTION("No style attribute, no inline style")
    {
      REQUIRE(element->InlineStyle() == nullptr);
      REQUIRE_FALSE(element->HasAttribute(u8"style"));
    }

    SECTION("Setting the attribute parses it")
    {
      REQUIRE_FALSE(element->SetAttribute(u8"style", u8"color: red; margin: 1px 2px").HasException());
      REQUIRE(element->InlineStyle() != nullptr);
      REQUIRE_FALSE(element->InlineStyle()->IsMutable());
      REQUIRE(ToNarrow(element->InlineStyle()->AsText(css::DefaultSerialisationContext()))
              == "color: red; margin: 1px 2px;");

      auto &style = element->Style();
      REQUIRE(PropertyValue(style, u8"color") == "red");
      REQUIRE(PropertyValue(style, u8"margin-left") == "2px");
      REQUIRE(style.Length() == 5u);
      REQUIRE(element->InlineStyle()->IsMutable());
    }

    SECTION("Invalid declarations are dropped, the attribute is kept as written")
    {
      REQUIRE_FALSE(element->SetAttribute(u8"style", u8"color: nonsense; width: 5px; {").HasException());
      auto &style = element->Style();
      REQUIRE(PropertyValue(style, u8"color").empty());
      REQUIRE(PropertyValue(style, u8"width") == "5px");
      REQUIRE(StyleAttribute(*element) == "color: nonsense; width: 5px; {");
    }

    SECTION("Changing the attribute reparses into the same declaration")
    {
      auto &style = element->Style();
      REQUIRE_FALSE(element->SetAttribute(u8"style", u8"color: red").HasException());
      REQUIRE(PropertyValue(style, u8"color") == "red");

      REQUIRE_FALSE(element->SetAttribute(u8"style", u8"color: green").HasException());
      REQUIRE(&element->Style() == &style);
      REQUIRE(PropertyValue(style, u8"color") == "green");
    }

    SECTION("Removing the attribute clears the inline style")
    {
      REQUIRE_FALSE(element->SetAttribute(u8"style", u8"color: red").HasException());
      auto &style = element->Style();
      element->RemoveAttribute(u8"style");
      REQUIRE(style.Length() == 0u);
      REQUIRE_FALSE(element->HasAttribute(u8"style"));
    }

    SECTION("An attribute in another namespace is not the style attribute")
    {
      REQUIRE_FALSE(element->SetAttributeNS(u8"urn:other", u8"style", u8"color: red").HasException());
      REQUIRE(element->InlineStyle() == nullptr);
    }

    SECTION("Setting the attribute through its Attr node")
    {
      REQUIRE_FALSE(element->SetAttribute(u8"style", u8"color: red").HasException());
      element->GetAttributeNode(u8"style")->Value(u8"color: blue");
      REQUIRE(PropertyValue(element->Style(), u8"color") == "blue");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "StyledElement: CSSOM changes update the attribute",
                   "[DOM][StyledElement]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = CreateDiv(*document);
    auto &style = element->Style();

    SECTION("setProperty creates the attribute")
    {
      REQUIRE_FALSE(style.SetProperty(u8"color", u8"red", u8"").HasException());
      REQUIRE(element->HasAttribute(u8"style"));
      REQUIRE(StyleAttribute(*element) == "color: red;");
    }

    SECTION("The attribute is only written when it's read")
    {
      REQUIRE_FALSE(element->SetAttribute(u8"style", u8"color: red").HasException());
      Ref<Attr> attr = ShareRef(*element->GetAttributeNode(u8"style"));

      REQUIRE_FALSE(style.SetProperty(u8"color", u8"blue", u8"").HasException());
      REQUIRE_FALSE(style.SetProperty(u8"width", u8"1px", u8"important").HasException());
      REQUIRE(ToNarrow(attr->Value()) == "color: blue; width: 1px !important;");
      REQUIRE(StyleAttribute(*element) == "color: blue; width: 1px !important;");
    }

    SECTION("Every attribute read sees the change")
    {
      REQUIRE_FALSE(style.SetProperty(u8"color", u8"red", u8"").HasException());
      REQUIRE(element->HasAttributes());

      auto other = CreateDiv(*document);
      REQUIRE_FALSE(other->Style().SetProperty(u8"color", u8"red", u8"").HasException());
      REQUIRE(element->GetAttributeNames() == List<DOMString> {u8"style"});
      REQUIRE(other->Attributes().Length() == 1uz);
      REQUIRE(element->IsEqualNode(other.get()));
    }

    SECTION("removeProperty and cssText")
    {
      REQUIRE_FALSE(style.SetCssText(u8"color: red; width: 1px").HasException());
      REQUIRE(StyleAttribute(*element) == "color: red; width: 1px;");

      REQUIRE(ToNarrow(style.RemoveProperty(u8"color").ReleaseValue()) == "red");
      REQUIRE(StyleAttribute(*element) == "width: 1px;");

      REQUIRE_FALSE(style.SetCssText(u8"").HasException());
      REQUIRE(StyleAttribute(*element) == "");
    }

    SECTION("The attribute's text is rewritten even when cssText sets the same properties")
    {
      REQUIRE_FALSE(element->SetAttribute(u8"style", u8"color:red").HasException());
      REQUIRE_FALSE(style.SetCssText(u8"color:   red").HasException());
      REQUIRE(StyleAttribute(*element) == "color: red;");
    }

    SECTION("Setting the attribute after a CSSOM change replaces it")
    {
      REQUIRE_FALSE(style.SetProperty(u8"color", u8"red", u8"").HasException());
      REQUIRE_FALSE(element->SetAttribute(u8"style", u8"width: 2px").HasException());
      REQUIRE(PropertyValue(style, u8"color").empty());
      REQUIRE(StyleAttribute(*element) == "width: 2px");
    }

    SECTION("Removing the attribute after a CSSOM change")
    {
      REQUIRE_FALSE(style.SetProperty(u8"color", u8"red", u8"").HasException());
      element->RemoveAttribute(u8"style");
      REQUIRE_FALSE(element->HasAttribute(u8"style"));
      REQUIRE(style.Length() == 0u);
    }

    SECTION("The declaration's parent element")
    {
      REQUIRE(style.ParentElement() == element.get());
      REQUIRE(style.ParentRule() == nullptr);
      REQUIRE(style.ParentStyleSheet() == nullptr);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "StyledElement: mutation records", "[DOM][StyledElement]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = CreateDiv(*document);
    auto &style = element->Style();

    SECTION("A CSSOM change queues one record with the old attribute value")
    {
      REQUIRE_FALSE(element->SetAttribute(u8"style", u8"color: red").HasException());
      Observed observed(*element);

      REQUIRE_FALSE(style.SetProperty(u8"color", u8"blue", u8"").HasException());
      REQUIRE_FALSE(style.SetProperty(u8"width", u8"1px", u8"").HasException());

      auto records = observed.Observer->TakeRecords();
      REQUIRE(records.size() == 2uz);
      REQUIRE(records[0]->Type() == u8"attributes");
      REQUIRE(records[0]->AttributeName() == u8"style");
      REQUIRE(records[0]->OldValue() == u8"color: red");
      REQUIRE(records[1]->OldValue() == u8"color: blue;");
    }

    SECTION("The old value is null when there was no attribute")
    {
      Observed observed(*element);
      REQUIRE_FALSE(style.SetProperty(u8"color", u8"blue", u8"").HasException());

      auto records = observed.Observer->TakeRecords();
      REQUIRE(records.size() == 1uz);
      REQUIRE(records[0]->OldValue() == DOMStringAtom::Null());
    }

    SECTION("No record when nothing changes")
    {
      REQUIRE_FALSE(style.SetProperty(u8"color", u8"blue", u8"").HasException());
      Observed observed(*element);

      REQUIRE(style.RemoveProperty(u8"width").ReleaseValue().empty());
      REQUIRE_FALSE(style.SetProperty(u8"color", u8"nonsense", u8"").HasException());
      REQUIRE_FALSE(style.SetProperty(u8"color", u8"blue", u8"bogus").HasException());
      REQUIRE(observed.Observer->TakeRecords().empty());
    }

    SECTION("cssText always queues a record")
    {
      REQUIRE_FALSE(style.SetProperty(u8"color", u8"blue", u8"").HasException());
      Observed observed(*element);

      REQUIRE_FALSE(style.SetCssText(u8"color: blue").HasException());
      auto records = observed.Observer->TakeRecords();
      REQUIRE(records.size() == 1uz);
      REQUIRE(records[0]->OldValue() == u8"color: blue;");
    }

    SECTION("Writing the attribute back doesn't queue a record")
    {
      REQUIRE_FALSE(style.SetProperty(u8"color", u8"blue", u8"").HasException());
      Observed observed(*element);

      REQUIRE(StyleAttribute(*element) == "color: blue;");
      REQUIRE(observed.Observer->TakeRecords().empty());
    }

    SECTION("Setting the attribute after a CSSOM change reports the written-back value")
    {
      REQUIRE_FALSE(style.SetProperty(u8"color", u8"blue", u8"").HasException());
      Observed observed(*element);

      REQUIRE_FALSE(element->SetAttribute(u8"style", u8"width: 1px").HasException());
      auto records = observed.Observer->TakeRecords();
      REQUIRE(records.size() == 1uz);
      REQUIRE(records[0]->OldValue() == u8"color: blue;");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "StyledElement: inline style setters", "[DOM][StyledElement]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = CreateDiv(*document);

    REQUIRE(element->SetInlineStyleProperty(css::PropertyId::Display, css::ValueId::None));
    REQUIRE(element->SetInlineStyleProperty(css::PropertyId::Width, 10.0, css::UnitType::px));
    REQUIRE(element->SetInlineStyleProperty(css::PropertyId::Color, css::CSSOMString(u8"red")));
    REQUIRE(element->SetInlineStyleCustomProperty(u8"--x", u8"1", css::IsImportant(true)));
    REQUIRE(StyleAttribute(*element) == "display: none; width: 10px; color: red; --x: 1 !important;");

    bool didFailParsing = false;
    REQUIRE_FALSE(element->SetInlineStyleProperty(css::PropertyId::Color, css::CSSOMString(u8"nonsense"),
                                                  css::IsImportant(false), &didFailParsing));
    REQUIRE(didFailParsing);

    REQUIRE(element->RemoveInlineStyleProperty(css::PropertyId::Width));
    REQUIRE_FALSE(element->RemoveInlineStyleProperty(css::PropertyId::Width));
    REQUIRE(element->RemoveInlineStyleCustomProperty(u8"--x"));
    REQUIRE(StyleAttribute(*element) == "display: none; color: red;");

    element->RemoveAllInlineStyleProperties();
    REQUIRE(StyleAttribute(*element) == "");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "StyledElement: parser mode", "[DOM][StyledElement]")
  {
    SECTION("Quirks mode accepts unitless lengths on HTML elements")
    {
      auto document = Parse(u8"<div style='width: 10'></div>");
      auto div = AsStyled(*document->Body()->FirstElementChild());
      REQUIRE(document->InQuirksMode());
      REQUIRE(PropertyValue(div->Style(), u8"width") == "10px");

      REQUIRE_FALSE(div->Style().SetProperty(u8"height", u8"5", u8"").HasException());
      REQUIRE(PropertyValue(div->Style(), u8"height") == "5px");
    }

    SECTION("No-quirks mode doesn't")
    {
      auto document = Parse(u8"<!DOCTYPE html><div style='width: 10'></div>");
      auto div = AsStyled(*document->Body()->FirstElementChild());
      REQUIRE_FALSE(document->InQuirksMode());
      REQUIRE(PropertyValue(div->Style(), u8"width").empty());
    }

    SECTION("ParseInlineStyleDeclaration")
    {
      auto document = CreateRef<HTMLDocument>();
      auto element = CreateDiv(*document);
      auto properties = css::Parser::ParseInlineStyleDeclaration(u8"color: red; color: blue", *element);
      REQUIRE(properties->ParserMode() == css::ParserMode::HTMLStandard);
      REQUIRE(ToNarrow(properties->AsText(css::DefaultSerialisationContext())) == "color: blue;");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "StyledElement: cloning", "[DOM][StyledElement]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = CreateDiv(*document);
    REQUIRE_FALSE(element->SetAttribute(u8"style", u8"color: red").HasException());
    REQUIRE_FALSE(element->Style().SetProperty(u8"width", u8"1px", u8"").HasException());

    auto clone = AsStyled(*element->CloneNode().ReleaseValue());
    REQUIRE(StyleAttribute(*clone) == "color: red; width: 1px;");
    REQUIRE(PropertyValue(clone->Style(), u8"width") == "1px");

    // The original keeps its attribute: cloning copies it.
    REQUIRE(element->GetAttributeNode(u8"style")->OwnerElement() == element.get());
    REQUIRE(clone->GetAttributeNode(u8"style") != element->GetAttributeNode(u8"style"));

    REQUIRE_FALSE(clone->Style().SetProperty(u8"color", u8"blue", u8"").HasException());
    REQUIRE(PropertyValue(element->Style(), u8"color") == "red");
  }
}
