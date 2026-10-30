#include "Krystal.Booey/DOM/DatasetDOMStringMap.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include <string>

namespace krys::boo::dom::tests
{
  namespace
  {
    KRYS_NODISCARD std::string DatasetText(DOMStringView text) noexcept
    {
      return std::string(reinterpret_cast<const char *>(text.data()), text.size());
    }

    KRYS_NODISCARD List<std::string> DatasetNames(const DatasetDOMStringMap &dataset) noexcept
    {
      List<std::string> names;
      for (const auto &name : dataset.SupportedPropertyNames())
      {
        names.push_back(DatasetText(name));
      }
      return names;
    }
  }

  TEST_CASE("DatasetDOMStringMap", "[DOM][Element][DatasetDOMStringMap]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = document->CreateElement(DOMStringAtom(u8"div")).ReleaseValue();
    auto &dataset = element->Dataset();

    SECTION("is the same object each time")
    {
      CHECK(&element->Dataset() == &dataset);
      CHECK(&dataset.GetElement() == element.get());
    }

    SECTION("names the data attributes as properties, in attribute order")
    {
      (void)element->SetAttribute(DOMStringAtom(u8"data-foo-bar"), u8"1");
      (void)element->SetAttribute(DOMStringAtom(u8"title"), u8"2");
      (void)element->SetAttribute(DOMStringAtom(u8"data-x"), u8"3");
      (void)element->SetAttribute(DOMStringAtom(u8"data-"), u8"4");
      (void)element->SetAttribute(DOMStringAtom(u8"data--a"), u8"5");
      (void)element->SetAttribute(DOMStringAtom(u8"data-a-1"), u8"6");
      (void)element->SetAttribute(DOMStringAtom(u8"datax"), u8"7");
      CHECK(DatasetNames(dataset) == List<std::string> {"fooBar", "x", "", "A", "a-1"});
    }

    SECTION("skips data attributes with uppercase letters")
    {
      // Only an attribute set with a namespace keeps its case in an HTML document.
      (void)element->SetAttributeNS(DOMStringAtom::Null(), DOMStringAtom(u8"data-Upper"), u8"1");
      CHECK(DatasetNames(dataset).empty());
      CHECK_FALSE(dataset.IsSupportedPropertyName(u8"Upper"));
    }

    SECTION("gets a data attribute's value by its property name")
    {
      (void)element->SetAttribute(DOMStringAtom(u8"data-foo-bar"), u8"value");
      CHECK(dataset.IsSupportedPropertyName(u8"fooBar"));
      REQUIRE(dataset.NamedItem(u8"fooBar"));
      CHECK(DatasetText(*dataset.NamedItem(u8"fooBar")) == "value");

      // Only the attribute's own property name finds it.
      CHECK_FALSE(dataset.NamedItem(u8"foo-bar"));
      CHECK_FALSE(dataset.IsSupportedPropertyName(u8"foo-bar"));
      CHECK_FALSE(dataset.NamedItem(u8"missing"));
    }

    SECTION("only data attributes have property names")
    {
      (void)element->SetAttribute(DOMStringAtom(u8"abcdefoo"), u8"1");
      CHECK_FALSE(dataset.IsSupportedPropertyName(u8"foo"));
      CHECK_FALSE(dataset.NamedItem(u8"foo"));
      CHECK_FALSE(dataset.NamedItem(u8""));
    }

    SECTION("sets the data attribute for a property name")
    {
      REQUIRE_FALSE(dataset.SetNamedItem(u8"fooBar", u8"1").HasException());
      CHECK(element->GetAttribute(u8"data-foo-bar") == u8"1");
      REQUIRE_FALSE(dataset.SetNamedItem(u8"fooBar", u8"2").HasException());
      CHECK(element->GetAttribute(u8"data-foo-bar") == u8"2");
      REQUIRE_FALSE(dataset.SetNamedItem(u8"", u8"3").HasException());
      CHECK(element->GetAttribute(u8"data-") == u8"3");
      REQUIRE_FALSE(dataset.SetNamedItem(u8"-1", u8"4").HasException());
      CHECK(element->GetAttribute(u8"data--1") == u8"4");
    }

    SECTION("throws for names it can't set")
    {
      auto result = dataset.SetNamedItem(u8"foo-bar", u8"1");
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::SyntaxError);

      result = dataset.SetNamedItem(u8"a b", u8"1");
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
      CHECK_FALSE(element->HasAttributes());
    }

    SECTION("deletes the data attribute for a property name")
    {
      (void)element->SetAttribute(DOMStringAtom(u8"data-foo-bar"), u8"1");
      (void)element->SetAttribute(DOMStringAtom(u8"data-other"), u8"2");
      dataset.DeleteNamedProperty(u8"fooBar");
      CHECK_FALSE(element->HasAttribute(u8"data-foo-bar"));
      CHECK(element->HasAttribute(u8"data-other"));
      dataset.DeleteNamedProperty(u8"missing");
      CHECK(DatasetNames(dataset) == List<std::string> {"other"});
    }

    SECTION("works on SVG elements")
    {
      auto svg = document->CreateElementNS(DOMStringAtom(infra::Namespaces::SVG), DOMStringAtom(u8"svg"))
                   .ReleaseValue();
      REQUIRE_FALSE(svg->Dataset().SetNamedItem(u8"shape", u8"round").HasException());
      CHECK(svg->GetAttribute(u8"data-shape") == u8"round");
    }
  }
}
