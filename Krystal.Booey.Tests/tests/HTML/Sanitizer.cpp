#include "Krystal.Booey/HTML/Sanitizer/Sanitizer.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/HTML/Sanitizer/SanitizerAlgorithms.hpp"
#include "Krystal.Booey/HTML/Sanitizer/SanitizerConstants.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"

namespace krys::boo::html::tests
{
  namespace
  {
    const dom::DOMStringAtom SVG {infra::Namespaces::SVG};
    const dom::DOMStringAtom MathML {infra::Namespaces::MathML};
    const dom::DOMStringAtom XLink {infra::Namespaces::XLink};

    KRYS_NODISCARD Ref<Sanitizer> Make(const SanitizerConfig &config)
    {
      auto sanitizer = Sanitizer::Create(config);
      REQUIRE_FALSE(sanitizer.HasException());
      return sanitizer.ReleaseValue();
    }

    KRYS_NODISCARD List<dom::DOMString> Names(const List<SanitizerElementNamespace> &list) noexcept
    {
      List<dom::DOMString> names;
      for (auto &item : list)
      {
        names.push_back(item.Name);
      }
      return names;
    }

    KRYS_NODISCARD List<dom::DOMString> Names(const List<SanitizerAttributeNamespace> &list) noexcept
    {
      List<dom::DOMString> names;
      for (auto &item : list)
      {
        names.push_back(item.Name);
      }
      return names;
    }

    KRYS_NODISCARD bool
      Allows(const SanitizerConfig &config, dom::DOMStringView attribute, dom::DOMStringView value = u8"",
             dom::DOMStringView element = u8"div",
             dom::DOMStringAtom elementNamespace = dom::DOMStringAtom(infra::Namespaces::HTML),
             dom::DOMStringAtom attributeNamespace = dom::DOMStringAtom::Null()) noexcept
    {
      return SanitizerAlgorithms::AllowsAttribute(attribute, attributeNamespace, value, element,
                                                  elementNamespace, config);
    }
  }

  TEST_CASE("Sanitizer: canonicalizing a configuration", "[HTML][Sanitizer]")
  {
    SECTION("an empty configuration with permissive defaults allows everything")
    {
      auto config = Make({})->Get();
      CHECK_FALSE(config.Elements);
      REQUIRE(config.RemoveElements);
      CHECK(config.RemoveElements->empty());
      REQUIRE(config.RemoveAttributes);
      CHECK(config.RemoveAttributes->empty());
      REQUIRE(config.RemoveProcessingInstructions);
      CHECK_FALSE(config.ProcessingInstructions);
      CHECK(config.Comments == true);
      CHECK(config.JavascriptURLs == true);
      CHECK_FALSE(config.DataAttributes);
    }

    SECTION("without permissive defaults, nothing extra is allowed")
    {
      auto config =
        SanitizerAlgorithms::Configure({.Attributes = List<SanitizerAttributeNamespace> {}}, false);
      REQUIRE_FALSE(config.HasException());
      auto &value = config.Value();
      CHECK(value.ProcessingInstructions);
      CHECK(value.Comments == false);
      CHECK(value.DataAttributes == false);
      CHECK(value.JavascriptURLs == false);
    }

    SECTION("strings take the default namespaces and an empty namespace is null")
    {
      SanitizerConfig input;
      input.Elements = List<SanitizerElementNamespaceWithAttributes> {
        u8"div", SanitizerElementNamespaceWithAttributes {u8"circle", SVG}};
      input.Elements->back().Attributes =
        List<SanitizerAttributeNamespace> {u8"cx", {u8"href", XLink}, {u8"title", dom::DOMStringAtom(u8"")}};
      auto config = Make(input)->Get();
      REQUIRE(config.Elements->size() == 2);
      auto &circle = (*config.Elements)[1];
      CHECK((*config.Elements)[0].Namespace == dom::DOMStringAtom(infra::Namespaces::HTML));
      REQUIRE((*config.Elements)[0].RemoveAttributes);
      CHECK((*config.Elements)[0].RemoveAttributes->empty());
      CHECK(circle.Namespace == SVG);
      REQUIRE(circle.Attributes);
      for (auto &attribute : *circle.Attributes)
      {
        if (attribute.Name == u8"href")
        {
          CHECK(attribute.Namespace == XLink);
        }
        else
        {
          CHECK(attribute.Namespace == dom::DOMStringAtom::Null());
        }
      }
    }
  }

  TEST_CASE("Sanitizer: invalid configurations throw", "[HTML][Sanitizer]")
  {
    auto invalid = [](const SanitizerConfig &config)
    {
      auto result = Sanitizer::Create(config);
      return result.HasException() && result.GetException().Code() == dom::ExceptionCode::TypeError;
    };

    CHECK(invalid({.Elements = List<SanitizerElementNamespaceWithAttributes> {u8"p"},
                   .RemoveElements = List<SanitizerElementNamespace> {u8"b"}}));
    CHECK(invalid({.Attributes = List<SanitizerAttributeNamespace> {u8"id"},
                   .RemoveAttributes = List<SanitizerAttributeNamespace> {u8"class"}}));
    CHECK(invalid({.ProcessingInstructions = List<SanitizerProcessingInstruction> {u8"a"},
                   .RemoveProcessingInstructions = List<SanitizerProcessingInstruction> {u8"b"}}));
    CHECK(invalid({.Elements = List<SanitizerElementNamespaceWithAttributes> {u8"p", u8"p"}}));
    CHECK(invalid({.RemoveElements = List<SanitizerElementNamespace> {u8"p"},
                   .ReplaceWithChildrenElements = List<SanitizerElementNamespace> {u8"p"}}));
    CHECK(invalid({.ReplaceWithChildrenElements = List<SanitizerElementNamespace> {u8"html"}}));
    CHECK(invalid({.ReplaceWithChildrenElements = List<SanitizerElementNamespace> {{u8"svg", SVG}}}));
    CHECK(invalid({.Attributes = List<SanitizerAttributeNamespace> {u8"data-x"}, .DataAttributes = true}));
    CHECK(invalid({.RemoveAttributes = List<SanitizerAttributeNamespace> {}, .DataAttributes = true}));

    SanitizerConfig localAndGlobal;
    localAndGlobal.Elements = List<SanitizerElementNamespaceWithAttributes> {u8"p"};
    localAndGlobal.Elements->back().Attributes = List<SanitizerAttributeNamespace> {u8"id"};
    localAndGlobal.Attributes = List<SanitizerAttributeNamespace> {u8"id"};
    CHECK(invalid(localAndGlobal));

    SanitizerConfig removeNotSubset;
    removeNotSubset.Elements = List<SanitizerElementNamespaceWithAttributes> {u8"p"};
    removeNotSubset.Elements->back().RemoveAttributes = List<SanitizerAttributeNamespace> {u8"id"};
    removeNotSubset.Attributes = List<SanitizerAttributeNamespace> {u8"class"};
    CHECK(invalid(removeNotSubset));

    SanitizerConfig bothLocalLists;
    bothLocalLists.Elements = List<SanitizerElementNamespaceWithAttributes> {u8"p"};
    bothLocalLists.Elements->back().Attributes = List<SanitizerAttributeNamespace> {u8"id"};
    bothLocalLists.Elements->back().RemoveAttributes = List<SanitizerAttributeNamespace> {u8"class"};
    CHECK(invalid(bothLocalLists));
    bothLocalLists.Attributes = List<SanitizerAttributeNamespace> {u8"class"};
    CHECK_FALSE(invalid(bothLocalLists));
  }

  TEST_CASE("Sanitizer: the default configuration", "[HTML][Sanitizer]")
  {
    auto config = Sanitizer::Create()->Get();
    REQUIRE(config.Elements);
    auto has = [&](dom::DOMStringView name, dom::DOMStringAtom namespaceURI)
    {
      return SanitizerAlgorithms::ActionForElementName(name, namespaceURI, config) == SanitizerAction::Keep;
    };
    CHECK(has(u8"p", dom::DOMStringAtom(infra::Namespaces::HTML)));
    CHECK(has(u8"h6", dom::DOMStringAtom(infra::Namespaces::HTML)));
    CHECK(has(u8"mfrac", MathML));
    CHECK(has(u8"foreignObject", SVG));
    CHECK_FALSE(has(u8"script", dom::DOMStringAtom(infra::Namespaces::HTML)));
    CHECK_FALSE(has(u8"img", dom::DOMStringAtom(infra::Namespaces::HTML)));
    CHECK_FALSE(has(u8"use", SVG));
    CHECK(config.Comments == false);
    CHECK(config.DataAttributes == false);
    CHECK(config.JavascriptURLs == false);

    CHECK(Allows(config, u8"lang"));
    CHECK(Allows(config, u8"href", u8"https://example.com", u8"a"));
    CHECK_FALSE(Allows(config, u8"href", u8"javascript:alert(1)", u8"a"));
    CHECK_FALSE(Allows(config, u8"onclick", u8"x", u8"a"));
    CHECK_FALSE(Allows(config, u8"data-x"));
    CHECK_FALSE(Allows(config, u8"cite", u8"", u8"p"));
    CHECK(Allows(config, u8"cite", u8"", u8"blockquote"));
  }

  TEST_CASE("Sanitizer: get() sorts its lists", "[HTML][Sanitizer]")
  {
    auto sanitizer =
      Make({.RemoveElements = List<SanitizerElementNamespace> {u8"u", {u8"g", SVG}, u8"b"},
            .RemoveAttributes = List<SanitizerAttributeNamespace> {u8"z", {u8"href", XLink}, u8"a"}});
    auto config = sanitizer->Get();
    CHECK(Names(*config.RemoveElements) == List<dom::DOMString> {u8"b", u8"u", u8"g"});
    CHECK(Names(*config.RemoveAttributes) == List<dom::DOMString> {u8"a", u8"z", u8"href"});
  }

  TEST_CASE("Sanitizer: modifying a configuration", "[HTML][Sanitizer]")
  {
    SECTION("with an allow-list")
    {
      auto sanitizer = Make({.Elements = List<SanitizerElementNamespaceWithAttributes> {u8"p"},
                             .Attributes = List<SanitizerAttributeNamespace> {u8"id"}});
      CHECK(sanitizer->AllowElement(u8"b"));
      CHECK_FALSE(sanitizer->AllowElement(u8"b"));
      CHECK(sanitizer->RemoveElement(u8"b"));
      CHECK_FALSE(sanitizer->RemoveElement(u8"b"));
      CHECK(sanitizer->ReplaceElementWithChildren(u8"p"));
      CHECK(SanitizerAlgorithms::ActionForElementName(u8"p", dom::DOMStringAtom(infra::Namespaces::HTML),
                                                      sanitizer->Configuration())
            == SanitizerAction::ReplaceWithChildren);
      CHECK_FALSE(sanitizer->ReplaceElementWithChildren(u8"html"));
      CHECK(sanitizer->AllowAttribute(u8"class"));
      CHECK_FALSE(sanitizer->AllowAttribute(u8"class"));
      CHECK(sanitizer->RemoveAttribute(u8"id"));
      CHECK(Names(*sanitizer->Get().Attributes) == List<dom::DOMString> {u8"class"});

      // Allowing an element's local attributes that the global list has drops them.
      SanitizerElementNamespaceWithAttributes span {u8"span"};
      span.Attributes = List<SanitizerAttributeNamespace> {u8"class", u8"title"};
      CHECK(sanitizer->AllowElement(span));
      auto config = sanitizer->Get();
      auto it =
        std::ranges::find_if(*config.Elements, [](const auto &element) { return element.Name == u8"span"; });
      REQUIRE(it != config.Elements->end());
      CHECK(Names(*it->Attributes) == List<dom::DOMString> {u8"title"});
    }

    SECTION("with a remove-list")
    {
      auto sanitizer = Make({});
      CHECK(sanitizer->RemoveElement(u8"b"));
      CHECK(sanitizer->AllowElement(u8"b"));
      CHECK_FALSE(sanitizer->AllowElement(u8"i"));
      SanitizerElementNamespaceWithAttributes withAttributes {u8"i"};
      withAttributes.Attributes = List<SanitizerAttributeNamespace> {u8"id"};
      CHECK_FALSE(sanitizer->AllowElement(withAttributes));
      CHECK(sanitizer->RemoveAttribute(u8"id"));
      CHECK(sanitizer->AllowAttribute(u8"id"));
      CHECK_FALSE(sanitizer->AllowAttribute(u8"id"));
    }

    SECTION("the flags")
    {
      auto sanitizer =
        Make({.Attributes = List<SanitizerAttributeNamespace> {u8"data-a", u8"id"}, .DataAttributes = false});
      CHECK_FALSE(sanitizer->SetComments(true));
      CHECK(sanitizer->SetComments(false));
      CHECK_FALSE(sanitizer->SetDataAttributes(false));
      CHECK(sanitizer->SetDataAttributes(true));
      CHECK(Names(*sanitizer->Get().Attributes) == List<dom::DOMString> {u8"id"});
      CHECK(sanitizer->SetJavascriptURLs(false));
      CHECK_FALSE(Make({})->SetDataAttributes(true));
    }

    SECTION("processing instructions")
    {
      auto sanitizer = Make({});
      CHECK(sanitizer->RemoveProcessingInstruction(u8"xml-stylesheet"));
      CHECK_FALSE(sanitizer->RemoveProcessingInstruction(u8"xml-stylesheet"));
      CHECK_FALSE(SanitizerAlgorithms::AllowsProcessingInstructionTarget(u8"xml-stylesheet",
                                                                         sanitizer->Configuration()));
      CHECK(sanitizer->AllowProcessingInstruction(u8"xml-stylesheet"));
      CHECK(SanitizerAlgorithms::AllowsProcessingInstructionTarget(u8"xml-stylesheet",
                                                                   sanitizer->Configuration()));
    }

    SECTION("removing unsafe content")
    {
      auto sanitizer = Make({});
      CHECK(sanitizer->RemoveUnsafe());
      auto config = sanitizer->Get();
      auto removed = Names(*config.RemoveElements);
      CHECK(std::ranges::find(removed, u8"script") != removed.end());
      CHECK(std::ranges::find(removed, u8"iframe") != removed.end());
      auto removedAttributes = Names(*config.RemoveAttributes);
      CHECK(std::ranges::find(removedAttributes, u8"onclick") != removedAttributes.end());
      CHECK(std::ranges::find(removedAttributes, u8"onpointerdown") != removedAttributes.end());
      CHECK(config.JavascriptURLs == false);
      CHECK_FALSE(sanitizer->RemoveUnsafe());
    }
  }

  TEST_CASE("Sanitizer: attributes with javascript: URLs", "[HTML][Sanitizer]")
  {
    auto config = Make({})->Get();
    REQUIRE(config.JavascriptURLs == true);
    config.JavascriptURLs = false;
    CHECK_FALSE(Allows(config, u8"href", u8"  JavaScript:x", u8"a"));
    CHECK(Allows(config, u8"href", u8"javascript:x", u8"link"));
    CHECK_FALSE(Allows(config, u8"action", u8"javascript:x", u8"form"));
    CHECK_FALSE(Allows(config, u8"formaction", u8"javascript:x", u8"button"));
    CHECK_FALSE(Allows(config, u8"href", u8"javascript:x", u8"a", SVG, XLink));
    CHECK_FALSE(Allows(config, u8"href", u8"javascript:x", u8"mrow", MathML));
    CHECK_FALSE(Allows(config, u8"attributeName", u8"href", u8"animate", SVG));
    CHECK(Allows(config, u8"attributeName", u8"fill", u8"animate", SVG));
  }

  TEST_CASE("Sanitizer: code unit order", "[HTML][Sanitizer]")
  {
    using infra::StringAlgorithms;
    CHECK(StringAlgorithms::CodeUnitLessThan(u8"a", u8"b"));
    CHECK(StringAlgorithms::CodeUnitLessThan(u8"a", u8"ab"));
    CHECK_FALSE(StringAlgorithms::CodeUnitLessThan(u8"ab", u8"ab"));
    // U+1F600 is a surrogate pair in UTF-16, which sorts before U+FF21.
    CHECK(StringAlgorithms::CodeUnitLessThan(u8"\U0001F600", u8"Ａ"));
    CHECK_FALSE(StringAlgorithms::CodeUnitLessThan(u8"Ａ", u8"\U0001F600"));
  }
}
