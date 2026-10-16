#include "Krystal.Booey/DOM/UserActionElementSet.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::dom::tests
{
  using style::tests::ResolverFixture;
  using Type = HitTestRequest::Type;

  namespace
  {
    const boo::Color Blue {boo::Color::blue};

    KRYS_NODISCARD std::string Width(const Element &element)
    {
      auto *style = element.GetRenderStyle();
      REQUIRE(style);
      return ResolverFixture::Serialised(*style, style->Width());
    }

    KRYS_NODISCARD bool IsBlue(const Element &element)
    {
      auto *style = element.GetRenderStyle();
      REQUIRE(style);
      return style->GetColor() == Blue;
    }

    const HitTestRequest Move {{Type::Move}};
    const HitTestRequest Press {{Type::Active}};
    const HitTestRequest MoveWhilePressed {{Type::Active, Type::Move}};
    const HitTestRequest Release {{Type::Release}};
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "UserActionElementSet: flags", "[DOM][UserActionElementSet]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><p id=p></p>)"};
    auto &p = fixture.ById(u8"p");
    auto &set = fixture.Document->UserActionElements();

    CHECK_FALSE(p.IsUserActionElement());
    set.SetHovered(p, true);
    set.SetActive(p, true);
    CHECK(p.IsUserActionElement());
    CHECK(p.Hovered());
    CHECK(p.Active());
    CHECK_FALSE(p.Focused());

    set.SetHovered(p, false);
    CHECK(p.IsUserActionElement());
    set.ClearActiveAndHovered(p);
    CHECK_FALSE(p.IsUserActionElement());
    CHECK_FALSE(p.Active());

    set.SetFocused(p, true);
    set.Clear();
    CHECK_FALSE(p.IsUserActionElement());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "UserActionElementSet: hover and active",
                   "[DOM][UserActionElementSet]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      :hover > b { color: rgb(0, 0, 255) }
      i:hover { width: 5px }
      em:active { color: rgb(0, 0, 255) }
    </style><body><div id=d><b id=b><i id=i>x</i></b><em id=em>y</em></div><span id=s></span></body>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    auto &d = fixture.ById(u8"d");
    auto &b = fixture.ById(u8"b");
    auto &i = fixture.ById(u8"i");
    auto &em = fixture.ById(u8"em");
    auto &s = fixture.ById(u8"s");

    SECTION("Hover follows the pointer through the ancestors")
    {
      document.UpdateHoverActiveState(Move, &i);
      CHECK(document.HoveredElement() == &i);
      CHECK(i.Hovered());
      CHECK(b.Hovered());
      CHECK(d.Hovered());
      CHECK(document.Body()->Hovered());
      CHECK_FALSE(s.Hovered());
      CHECK(s.StyleValidity() == style::Validity::Valid);
      document.UpdateStyleIfNeeded();
      CHECK(Width(i) == "5px");
      CHECK(IsBlue(b));

      // Moving to a sibling keeps the common ancestors hovered.
      document.UpdateHoverActiveState(Move, &em);
      CHECK_FALSE(i.Hovered());
      CHECK_FALSE(b.Hovered());
      CHECK(d.Hovered());
      CHECK(em.Hovered());
      document.UpdateStyleIfNeeded();
      CHECK(Width(i) == "auto");
      CHECK(IsBlue(b));

      // Leaving the document clears the chain.
      document.UpdateHoverActiveState(Move, nullptr);
      CHECK_FALSE(document.HoveredElement());
      CHECK_FALSE(d.Hovered());
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(IsBlue(b));
    }

    SECTION("A press makes the chain active until the release")
    {
      document.UpdateHoverActiveState(Press, &em);
      CHECK(em.Active());
      CHECK(em.IsInActiveChain());
      CHECK(d.Active());
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(em));

      // While pressed, hover only changes within the active chain: leaving the pressed element unhovers it,
      // and elements outside the chain don't become hovered.
      document.UpdateHoverActiveState(MoveWhilePressed, &s);
      CHECK_FALSE(s.Hovered());
      CHECK_FALSE(em.Hovered());
      CHECK(em.Active());

      document.UpdateHoverActiveState(Release, &em);
      CHECK_FALSE(em.Active());
      CHECK_FALSE(em.IsInActiveChain());
      CHECK_FALSE(d.Active());
      CHECK(em.Hovered());
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(IsBlue(em));
    }

    SECTION("Removing the hovered element moves hover to its parent")
    {
      document.UpdateHoverActiveState(Press, &i);
      REQUIRE(i.Hovered());
      REQUIRE_FALSE(b.RemoveChild(i).HasException());
      CHECK(document.HoveredElement() == &b);
      CHECK_FALSE(i.IsUserActionElement());
      CHECK(b.Hovered());

      // So does the active chain.
      document.UpdateHoverActiveState(Release, &b);
      CHECK_FALSE(b.Active());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "UserActionElementSet: :has(:hover)",
                   "[DOM][UserActionElementSet]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      section:has(.x:hover) { color: rgb(0, 0, 255) }
    </style><body><section id=s><p><b id=b class=x></b><i id=i></i></p></section></body>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    auto &s = fixture.ById(u8"s");
    REQUIRE_FALSE(IsBlue(s));

    document.UpdateHoverActiveState(Move, &fixture.ById(u8"b"));
    document.UpdateStyleIfNeeded();
    CHECK(IsBlue(s));

    document.UpdateHoverActiveState(Move, &fixture.ById(u8"i"));
    document.UpdateStyleIfNeeded();
    CHECK_FALSE(IsBlue(s));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "UserActionElementSet: hover quirk",
                   "[DOM][UserActionElementSet]")
  {
    ResolverFixture fixture {u8R"(<style>
      :hover { color: rgb(0, 0, 255) }
      p:hover { width: 5px }
    </style><body><p id=p></p><a id=a href=x></a></body>)"};
    auto &document = *fixture.Document;
    REQUIRE(document.InQuirksMode());
    document.UpdateStyleIfNeeded();
    auto &p = fixture.ById(u8"p");
    auto &a = fixture.ById(u8"a");

    document.UpdateHoverActiveState(Move, &p);
    document.UpdateStyleIfNeeded();
    // In quirks mode a lone `:hover` only matches links.
    CHECK_FALSE(IsBlue(p));
    CHECK(ResolverFixture::Serialised(*p.GetRenderStyle(), p.GetRenderStyle()->Width()) == "5px");

    document.UpdateHoverActiveState(Move, &a);
    document.UpdateStyleIfNeeded();
    CHECK(IsBlue(a));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "UserActionElementSet: focus state",
                   "[DOM][UserActionElementSet]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      :focus { color: rgb(0, 0, 255) }
      :focus-within { width: 5px }
      :focus-visible { height: 5px }
    </style><body><div id=d><button id=b></button><input id=i></div><div id=host></div></body>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    auto &d = fixture.ById(u8"d");
    auto &b = fixture.ById(u8"b");
    auto &i = fixture.ById(u8"i");
    auto width = [](Element &element)
    {
      return ResolverFixture::Serialised(*element.GetRenderStyle(), element.GetRenderStyle()->Width());
    };
    auto height = [](Element &element)
    {
      return ResolverFixture::Serialised(*element.GetRenderStyle(), element.GetRenderStyle()->Height());
    };

    SECTION(":focus, :focus-within and :focus-visible")
    {
      b.SetFocus(true);
      CHECK(b.Focused());
      CHECK(b.HasFocusWithin());
      CHECK(d.HasFocusWithin());
      CHECK_FALSE(b.HasFocusVisible());
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(b));
      CHECK(width(d) == "5px");
      CHECK(height(b) == "auto");

      b.SetFocus(false);
      CHECK_FALSE(d.HasFocusWithin());
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(IsBlue(b));
      CHECK(width(d) == "auto");

      // Visible focus, and text fields always show focus.
      b.SetFocus(true, FocusVisibility::Visible);
      CHECK(b.HasFocusVisible());
      b.SetFocus(false);
      i.SetFocus(true);
      CHECK(i.HasFocusVisible());
      document.UpdateStyleIfNeeded();
      CHECK(height(i) == "5px");
    }

    SECTION("Focus only matches while the document is focused and active")
    {
      b.SetFocus(true);
      document.UpdateStyleIfNeeded();
      REQUIRE(IsBlue(b));

      document.SetFocusedAndActive(false);
      CHECK(b.StyleValidity() != style::Validity::Valid);
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(IsBlue(b));
      CHECK(width(d) == "auto");

      document.SetFocusedAndActive(true);
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(b));
    }

    SECTION("A shadow host matches :focus when its shadow tree has the focused element")
    {
      auto &host = fixture.ById(u8"host");
      auto shadow = host.AttachShadow({}).ReleaseValue();
      auto inner = document.CreateElement(u8"button").ReleaseValue();
      REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());
      document.UpdateStyleIfNeeded();

      inner->SetFocus(true);
      CHECK(shadow->ContainsFocusedElement());
      CHECK(host.HasFocusWithin());
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(host));

      inner->SetFocus(false);
      CHECK_FALSE(shadow->ContainsFocusedElement());
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(IsBlue(host));
    }
  }
}
