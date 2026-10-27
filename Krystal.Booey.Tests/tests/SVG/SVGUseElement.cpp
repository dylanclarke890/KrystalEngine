#include "Krystal.Booey/SVG/SVGUseElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/SVG/SVGGElement.hpp"
#include "Krystal.Booey/SVG/SVGRectElement.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    template <typename T>
    KRYS_NODISCARD T &ById(dom::Document &document, dom::DOMStringView id) noexcept
    {
      auto element = document.GetElementById(id);
      krys_assert(element);
      if constexpr (SameType<T, SVGElement>)
      {
        return Downcast<SVGElement>(*element);
      }
      else
      {
        return Downcast<T>(Downcast<SVGElement>(*element));
      }
    }

    // The root of @p use's instances, after the document's style is updated.
    KRYS_NODISCARD SVGElement *TargetClone(ResolverFixture &fixture, SVGUseElement &use) noexcept
    {
      (void)fixture.Document->UpdateLayout();
      auto *root = use.UserAgentShadowRoot();
      return root ? DynamicDowncast<SVGElement>(root->FirstChild()) : nullptr;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGUseElement: instances", "[SVG][SVGUseElement]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
                             u8R"html(<defs><rect id=r width=10 height=20 fill=green /></defs>)html"
                             u8R"html(<use id=u href="#r" x=30 y=40 /></svg></body>)html"};
    auto &use = ById<SVGUseElement>(*fixture.Document, u8"u");
    auto &rect = ById<SVGRectElement>(*fixture.Document, u8"r");

    // The target is cloned into the use's shadow tree, where the clone is an instance of it.
    auto *clone = TargetClone(fixture, use);
    REQUIRE(clone);
    REQUIRE(Is<SVGRectElement>(*clone));
    CHECK(clone->CorrespondingElement() == &rect);
    CHECK(clone->CorrespondingUseElement() == &use);
    REQUIRE(rect.Instances().size() == 1);
    CHECK(rect.Instances()[0].get() == clone);

    // The use is translated by its x and y, and draws the clone.
    CHECK(ExternalRepresentation(fixture.Frame.get()) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x104
  RenderBlock {HTML} at (0,0) size 800x104
    RenderBody {BODY} at (0,0) size 800x104
      RenderSVGRoot {svg} at (30,40) size 10x20
        RenderSVGHiddenContainer {defs} at (0,0) size 0x0
          RenderSVGRect {rect} at (0,0) size 10x20 [fill={[type=SOLID] [color=#008000]}] [x=0.00] [y=0.00] [width=10.00] [height=20.00]
        RenderSVGContainer {use} at (30,40) size 10x20 [transform={m=((1.00,0.00)(0.00,1.00)) t=(30.00,40.00)}]
          RenderSVGRect {rect} at (30,40) size 10x20 [fill={[type=SOLID] [color=#008000]}] [x=0.00] [y=0.00] [width=10.00] [height=20.00]
)");

    // Its bounding box includes the translation.
    auto bbox = static_cast<SVGGraphicsElement &>(use).GetBBox();
    CHECK(bbox == FloatRect(30, 40, 10, 20));

    // The instances follow their original.
    (void)rect.SetAttribute(u8"width", u8"15");
    clone = TargetClone(fixture, use);
    REQUIRE(clone);
    CHECK(clone->GetAttribute(u8"width") == dom::DOMString(u8"15"));
    CHECK(Downcast<SVGRectElement>(*clone).Width().ValueInSpecifiedUnits() == 15);

    // A new target is followed too.
    (void)use.SetAttribute(u8"href", u8"#missing");
    CHECK_FALSE(TargetClone(fixture, use));
    CHECK(rect.Instances().empty());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGUseElement: pending targets", "[SVG][SVGUseElement]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body><svg><use id=u href="#later" /><g id=g /></svg></body>)html"};
    auto &use = ById<SVGUseElement>(*fixture.Document, u8"u");

    // A use whose target doesn't exist yet waits for an element to take its id.
    CHECK_FALSE(TargetClone(fixture, use));
    CHECK(use.HasPendingResources());
    CHECK(fixture.Document->IsIdOfPendingSVGResource(dom::DOMStringAtom(u8"later")));

    (void)ById<SVGGElement>(*fixture.Document, u8"g").SetAttribute(u8"id", u8"later");
    auto *clone = TargetClone(fixture, use);
    REQUIRE(clone);
    CHECK(Is<SVGGElement>(*clone));
    CHECK_FALSE(use.HasPendingResources());
    CHECK_FALSE(fixture.Document->IsIdOfPendingSVGResource(dom::DOMStringAtom(u8"later")));

    // Removing the target empties the use, which waits for the id again.
    auto &target = *clone->CorrespondingElement();
    (void)target.Remove();
    CHECK_FALSE(TargetClone(fixture, use));
    CHECK(use.HasPendingResources());

    // A removed use stops waiting.
    Ref<SVGUseElement> protectedUse = ShareRef(use);
    CHECK(fixture.Document->IsPendingSVGResource(use, dom::DOMStringAtom(u8"later")));
    (void)use.Remove();
    CHECK_FALSE(use.HasPendingResources());
    CHECK_FALSE(fixture.Document->IsIdOfPendingSVGResource(dom::DOMStringAtom(u8"later")));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGUseElement: symbols and nested uses", "[SVG][SVGUseElement]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body><svg>)html"
      u8R"html(<symbol id=s viewBox="0 0 10 10" width=30><rect id=r width=10 height=10 /></symbol>)html"
      u8R"html(<g id=g><use id=inner href="#s" width=50 /><circle r=5 /><foo /></g>)html"
      u8R"html(<use id=outer href="#g" x=5 />)html"
      u8R"html(<g id=a><use id=cycle href="#a" /></g></svg></body>)html"};

    // A symbol is instanced as an <svg>, sized by the use's width and height, or else its own.
    auto *symbolClone = TargetClone(fixture, ById<SVGUseElement>(*fixture.Document, u8"inner"));
    REQUIRE(symbolClone);
    REQUIRE(Is<SVGSvgElement>(*symbolClone));
    CHECK(symbolClone->GetAttribute(u8"width") == dom::DOMString(u8"50"));
    CHECK_FALSE(symbolClone->HasAttribute(u8"height"));
    CHECK(symbolClone->GetAttribute(u8"viewBox") == dom::DOMString(u8"0 0 10 10"));
    CHECK(symbolClone->CorrespondingElement() == &ById<SVGElement>(*fixture.Document, u8"s"));

    // A use inside the target becomes a <g> holding its own target's instances, without its x, y, width,
    // height and href; elements a use can't instance (unknown elements here) are left out.
    auto *gClone = TargetClone(fixture, ById<SVGUseElement>(*fixture.Document, u8"outer"));
    REQUIRE(gClone);
    REQUIRE(Is<SVGGElement>(*gClone));
    auto *innerClone = DynamicDowncast<SVGElement>(gClone->FirstChild());
    REQUIRE(innerClone);
    CHECK(Is<SVGGElement>(*innerClone));
    CHECK(innerClone->GetAttribute(u8"id") == dom::DOMString(u8"inner"));
    CHECK_FALSE(innerClone->HasAttribute(u8"href"));
    CHECK_FALSE(innerClone->HasAttribute(u8"width"));
    CHECK(innerClone->CorrespondingElement() == &ById<SVGElement>(*fixture.Document, u8"inner"));
    auto *nestedSymbolClone = DynamicDowncast<SVGElement>(innerClone->FirstChild());
    REQUIRE(nestedSymbolClone);
    CHECK(Is<SVGSvgElement>(*nestedSymbolClone));
    CHECK(nestedSymbolClone->CorrespondingElement() == &ById<SVGElement>(*fixture.Document, u8"s"));
    CHECK(gClone->ChildElementCount() == 2);

    // A use inside its own target draws nothing.
    CHECK_FALSE(TargetClone(fixture, ById<SVGUseElement>(*fixture.Document, u8"cycle")));

    // Changing the target of the inner use rebuilds the outer use's instances.
    auto &symbolRect = ById<SVGRectElement>(*fixture.Document, u8"r");
    REQUIRE(symbolRect.Instances().size() == 2);
    (void)symbolRect.SetAttribute(u8"height", u8"4");
    gClone = TargetClone(fixture, ById<SVGUseElement>(*fixture.Document, u8"outer"));
    REQUIRE(symbolRect.Instances().size() == 2);
    for (auto &instance : symbolRect.Instances())
    {
      CHECK(instance->GetAttribute(u8"height") == dom::DOMString(u8"4"));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGUseElement: style and events", "[SVG][SVGUseElement]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><style>.c { stroke: red } use rect { stroke: blue }</style><body><svg>)html"
      u8R"html(<defs><rect id=r class=c width=10 height=10 /><rect id=plain width=10 height=10 /></defs>)html"
      u8R"html(<use id=u href="#r" fill=green /><use id=v href="#plain" fill=green /></svg></body>)html"};
    auto &use = ById<SVGUseElement>(*fixture.Document, u8"u");
    auto &rect = ById<SVGRectElement>(*fixture.Document, u8"r");

    // An instance is styled as its original is (selectors match the original, not the clone), inheriting from
    // its place in the shadow tree.
    auto *clone = TargetClone(fixture, use);
    REQUIRE(clone);
    REQUIRE(clone->Renderer());
    REQUIRE(rect.Renderer());
    CHECK(clone->Renderer()->Style().Stroke().IsColor());
    CHECK(clone->Renderer()->Style().Stroke() == rect.Renderer()->Style().Stroke());
    CHECK_FALSE(clone->Renderer()->Style().Fill() == rect.Renderer()->Style().Fill());
    auto *plainClone = TargetClone(fixture, ById<SVGUseElement>(*fixture.Document, u8"v"));
    REQUIRE(plainClone);
    REQUIRE(plainClone->Renderer());
    CHECK(plainClone->Renderer()->Style().Stroke().IsNone());

    // The original's listeners hear events at its instances, whether added before or after they were made.
    int32 before = 0;
    int32 after = 0;
    auto beforeHandle = rect.AddEventListener(u8"ping", [&](dom::Event &) { ++before; });
    (void)rect.SetAttribute(u8"x", u8"1");
    clone = TargetClone(fixture, use);
    REQUIRE(clone);
    auto afterHandle = rect.AddEventListener(u8"ping", [&](dom::Event &) { ++after; });
    (void)clone->DispatchEvent(*CreateRef<dom::Event>(dom::DOMStringAtom(u8"ping")));
    CHECK(before == 1);
    CHECK(after == 1);

    // Removing a listener removes it from the instances.
    afterHandle = {};
    (void)clone->DispatchEvent(*CreateRef<dom::Event>(dom::DOMStringAtom(u8"ping")));
    CHECK(before == 2);
    CHECK(after == 1);
  }
}
