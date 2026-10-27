#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey.Tests/SVG/TestSVGElement.hpp"
#include "Krystal.Booey/CSS/Properties/ImmutableStyleProperties.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/SVGSymbolElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD std::string Hints(const dom::Element &element) noexcept
    {
      const auto *style = Downcast<dom::StyledElement>(element).PresentationalHintStyle();
      return style ? css::tests::ToNarrow(style->AsText(css::DefaultSerialisationContext())) : std::string {};
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGElement: presentation attributes",
                   "[SVG][PresentationalHints]")
  {
    using css::PropertyId;
    CHECK(
      SVGElement::CssPropertyIdForSVGAttributeName(dom::DOMStringAtom(u8"fill"), dom::DOMStringAtom::Null())
      == PropertyId::Fill);
    CHECK(SVGElement::CssPropertyIdForSVGAttributeName(dom::DOMStringAtom(u8"font-stretch"),
                                                       dom::DOMStringAtom::Null())
          == PropertyId::FontWidth);
    CHECK(SVGElement::CssPropertyIdForSVGAttributeName(dom::DOMStringAtom(u8"transform-origin"),
                                                       dom::DOMStringAtom::Null())
          == PropertyId::TransformOrigin);
    CHECK(SVGElement::CssPropertyIdForSVGAttributeName(dom::DOMStringAtom(u8"d"), dom::DOMStringAtom::Null())
          == PropertyId::D);
    CHECK(
      SVGElement::CssPropertyIdForSVGAttributeName(dom::DOMStringAtom(u8"href"), dom::DOMStringAtom::Null())
      == PropertyId::Invalid);
    CHECK(SVGElement::CssPropertyIdForSVGAttributeName(dom::DOMStringAtom(u8"fill"),
                                                       dom::DOMStringAtom(infra::Namespaces::XLink))
          == PropertyId::Invalid);

    auto document =
      ResolverFixture::Parse(u8R"(<svg><rect id=r x=1 y=2 width=10 height=5 fill=red stroke-width=2
      opacity=0.5 bogus=1 fill-opacity=nonsense /><circle id=c cx=0 cy=0 r=4 /></svg>)");
    CHECK(Hints(*document->GetElementById(u8"r"))
          == "x: 1px; y: 2px; width: 10px; height: 5px; fill: red; stroke-width: 2px; opacity: 0.5;");
    CHECK(Hints(*document->GetElementById(u8"c")) == "cx: 0px; cy: 0px; r: 4px;");

    SECTION("Attribute changes")
    {
      auto &rect = *document->GetElementById(u8"r");
      REQUIRE_FALSE(rect.SetAttribute(u8"fill", u8"blue").HasException());
      CHECK(Hints(rect).find("fill: blue;") != std::string::npos);
    }

    SECTION("Resolution")
    {
      ResolverFixture fixture {u8"<!DOCTYPE html><svg><g id=g visibility=hidden display=none></g></svg>"};
      auto style = fixture.Resolve(fixture.ById(u8"g")).Style;
      CHECK(style->Display() == DisplayType::None);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGElement: owner and viewport elements", "[SVG][SVGElement]")
  {
    auto document = ResolverFixture::Parse(
      u8"<svg id=outer><g id=g><svg id=inner><rect id=r /></svg></g></svg><p id=p></p>");
    auto &outer = Downcast<SVGElement>(*document->GetElementById(u8"outer"));
    auto &g = Downcast<SVGElement>(*document->GetElementById(u8"g"));
    auto &inner = Downcast<SVGElement>(*document->GetElementById(u8"inner"));
    auto &r = Downcast<SVGElement>(*document->GetElementById(u8"r"));

    CHECK(outer.IsOutermostSVGSVGElement());
    CHECK_FALSE(outer.IsInnerSVGSVGElement());
    CHECK(inner.IsInnerSVGSVGElement());
    CHECK_FALSE(inner.IsOutermostSVGSVGElement());
    CHECK_FALSE(g.IsOutermostSVGSVGElement());

    CHECK(outer.OwnerSVGElement() == nullptr);
    CHECK(g.OwnerSVGElement() == &outer);
    CHECK(inner.OwnerSVGElement() == &outer);
    CHECK(r.OwnerSVGElement() == &inner);

    CHECK(outer.ViewportElement() == nullptr);
    CHECK(g.ViewportElement() == &outer);
    CHECK(r.ViewportElement() == &inner);

    // A <symbol> is a viewport element, unless only <svg> elements are asked for.
    auto symbol = AdoptRef(*new SVGSymbolElement(*document));
    auto child = CreateTestSVGElement(*document);
    REQUIRE_FALSE(g.AppendChild(*symbol).HasException());
    REQUIRE_FALSE(symbol->AppendChild(*child).HasException());
    CHECK(child->ViewportElement() == symbol.get());
    CHECK(child->ViewportElement(SVGElement::ViewportElementType::SVGSVGOnly) == &outer);

    // Out of a document, an <svg> is outermost.
    auto detached = CreateTestSVGElement(*document);
    CHECK(detached->OwnerSVGElement() == nullptr);
    CHECK(detached->ViewportElement() == nullptr);
    REQUIRE_FALSE(document->GetElementById(u8"p")->AppendChild(*detached).HasException());
    CHECK(detached->OwnerSVGElement() == nullptr);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGElement: relative lengths", "[SVG][SVGElement]")
  {
    // Sized, so that the <svg>'s own lengths aren't relative.
    auto document = ResolverFixture::Parse(u8"<svg id=root width=10 height=10></svg>");
    auto &root = Downcast<SVGElement>(*document->GetElementById(u8"root"));
    auto group = CreateTestSVGElement(*document);
    auto relative = CreateTestSVGElement(*document, true);
    CHECK_FALSE(root.HasRelativeLengths());
    CHECK_FALSE(group->HasRelativeLengths());

    // An element with relative lengths makes its ancestors have them.
    REQUIRE_FALSE(group->AppendChild(*relative).HasException());
    CHECK(relative->HasRelativeLengths());
    CHECK(group->HasRelativeLengths());
    REQUIRE_FALSE(root.AppendChild(*group).HasException());
    CHECK(root.HasRelativeLengths());

    REQUIRE_FALSE(group->RemoveChild(*relative).HasException());
    CHECK_FALSE(group->HasRelativeLengths());
    CHECK_FALSE(root.HasRelativeLengths());

    // The tree is told again when an element's own lengths change.
    REQUIRE_FALSE(group->AppendChild(*relative).HasException());
    CHECK(root.HasRelativeLengths());
    relative->SelfRelative = false;
    relative->UpdateRelativeLengthsInformation();
    CHECK_FALSE(group->HasRelativeLengths());
    CHECK_FALSE(root.HasRelativeLengths());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGElement: instances", "[SVG][SVGElement]")
  {
    auto document = ResolverFixture::Parse(u8"<svg id=root></svg>");
    auto original = CreateTestSVGElement(*document);
    auto instance = CreateTestSVGElement(*document);
    CHECK(original->Instances().empty());
    CHECK(instance->CorrespondingElement() == nullptr);
    CHECK(instance->CorrespondingUseElement() == nullptr);

    instance->SetCorrespondingElement(original.get());
    CHECK(instance->CorrespondingElement() == original.get());
    REQUIRE(original->Instances().size() == 1);
    CHECK(original->Instances()[0].get() == instance.get());

    // Instances aren't invalidated while their updates are blocked.
    {
      SVGElement::InstanceUpdateBlocker blocker {*original};
      CHECK(original->InstanceUpdatesBlocked());
      original->InvalidateInstances();
      CHECK(original->Instances().size() == 1);
    }
    CHECK_FALSE(original->InstanceUpdatesBlocked());

    // A change to the original's presentation attribute lets go of its instances.
    REQUIRE_FALSE(original->SetAttribute(u8"fill", u8"red").HasException());
    CHECK(original->Instances().empty());
    CHECK(instance->CorrespondingElement() == nullptr);

    instance->SetCorrespondingElement(original.get());
    {
      SVGElement::InstanceInvalidationGuard guard {*original};
      CHECK(original->Instances().size() == 1);
    }
    CHECK(original->Instances().empty());

    // An instance that lets go of its original leaves its instances, and an original that's destroyed
    // leaves its instances without one.
    instance->SetCorrespondingElement(original.get());
    instance->SetCorrespondingElement(nullptr);
    CHECK(original->Instances().empty());
    {
      auto other = CreateTestSVGElement(*document);
      instance->SetCorrespondingElement(other.get());
    }
    CHECK(instance->CorrespondingElement() == nullptr);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGElement: referencing elements", "[SVG][SVGElement]")
  {
    auto document = ResolverFixture::Parse(u8"<svg id=root></svg>");
    auto &root = *document->GetElementById(u8"root");
    auto target = CreateTestSVGElement(*document);
    auto referencing = CreateTestSVGElement(*document);
    CHECK(target->ReferencingElements().empty());

    target->AddReferencingElement(*referencing);
    REQUIRE(target->ReferencingElements().size() == 1);
    CHECK(target->ReferencingElements()[0].get() == referencing.get());
    referencing->RemoveElementReference();
    CHECK(target->ReferencingElements().empty());

    // A target that leaves the document clears the targets of the elements that refer to it.
    REQUIRE_FALSE(root.AppendChild(*target).HasException());
    target->AddReferencingElement(*referencing);
    REQUIRE_FALSE(root.RemoveChild(*target).HasException());
    CHECK(referencing->ClearedTargets == 1);
    CHECK(target->ReferencingElements().empty());
  }
}
