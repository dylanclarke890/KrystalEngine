#include "Krystal.Booey/HTML/Algorithms/HyperlinkAlgorithms.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLHeadElement.hpp"

namespace krys::boo::html::tests
{
  namespace
  {
    /// @brief An HTML document with a head holding a base element whose target is @p baseTarget.
    KRYS_NODISCARD Ref<dom::HTMLDocument> DocumentWithBaseTarget(utf8_stringview baseTarget) noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto html = document->CreateElement(u8"html").ReleaseValue();
      auto head = document->CreateElement(u8"head").ReleaseValue();
      auto base = document->CreateElement(u8"base").ReleaseValue();
      REQUIRE_FALSE(base->SetAttribute(u8"target", dom::DOMString(baseTarget)).HasException());
      REQUIRE_FALSE(head->AppendChild(*base).HasException());
      REQUIRE_FALSE(html->AppendChild(*head).HasException());
      REQUIRE_FALSE(document->AppendChild(*html).HasException());
      return document;
    }

    KRYS_NODISCARD Ref<HTMLElement> CreateWithTarget(dom::Document &document, utf8_stringview localName,
                                                     Maybe<utf8_stringview> target) noexcept
    {
      auto element = document.CreateElement(dom::DOMStringAtom(localName)).ReleaseValue();
      if (target)
      {
        REQUIRE_FALSE(element->SetAttribute(u8"target", dom::DOMString(*target)).HasException());
      }
      REQUIRE_FALSE(document.DocumentElement()->AppendChild(*element).HasException());
      return Downcast<HTMLElement>(krys::move(element));
    }
  }

  TEST_CASE("HyperlinkAlgorithms::EffectiveTarget: the target attribute, or the first base element's target",
            "[HTML][HyperlinkAlgorithms][html:get-an-element's-target]")
  {
    auto document = DocumentWithBaseTarget(u8"_top");

    CHECK(HyperlinkAlgorithms::EffectiveTarget(*CreateWithTarget(*document, u8"a", u8"frame")) == u8"frame");
    CHECK(HyperlinkAlgorithms::EffectiveTarget(*CreateWithTarget(*document, u8"a", null)) == u8"_top");

    // An empty target attribute is the target; the base element's isn't used.
    CHECK(HyperlinkAlgorithms::EffectiveTarget(*CreateWithTarget(*document, u8"area", u8"")) == u8"");

    // A target with a tab or newline and a < is _blank.
    CHECK(HyperlinkAlgorithms::EffectiveTarget(*CreateWithTarget(*document, u8"a", u8"x\n<y")) == u8"_blank");
    CHECK(HyperlinkAlgorithms::EffectiveTarget(*CreateWithTarget(*document, u8"a", u8"x<y")) == u8"x<y");
  }

  TEST_CASE("HTMLFormElement::EffectiveTarget: the base element's target is checked for dangling markup too",
            "[HTML][HTMLFormElement][html:get-an-element's-target]")
  {
    auto document = DocumentWithBaseTarget(u8"x\t<y");
    auto form = CreateWithTarget(*document, u8"form", null);
    CHECK(Downcast<HTMLFormElement>(*form).EffectiveTarget(nullptr, nullptr) == u8"_blank");

    auto targeted = CreateWithTarget(*document, u8"form", u8"frame");
    CHECK(Downcast<HTMLFormElement>(*targeted).EffectiveTarget(nullptr, nullptr) == u8"frame");
  }
}
