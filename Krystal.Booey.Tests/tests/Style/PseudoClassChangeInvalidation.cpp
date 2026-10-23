#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/PseudoClassStateInvalidation.hpp"

namespace krys::boo::style::tests
{
  namespace
  {
    const boo::Color Blue {boo::Color::blue};

    KRYS_NODISCARD Ref<dom::Element> Create(dom::Document &document, dom::DOMStringAtom localName)
    {
      auto element = document.CreateElement(localName);
      REQUIRE_FALSE(element.HasException());
      return element.ReleaseValue();
    }

    KRYS_NODISCARD bool IsBlue(const dom::Element &element)
    {
      auto *style = element.GetRenderStyle();
      REQUIRE(style);
      return style->GetColor() == Blue;
    }

    template <typename T>
    KRYS_NODISCARD T &HTML(dom::Element &element)
    {
      return Downcast<T>(Downcast<html::HTMLElement>(element));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "PseudoClassChangeInvalidation: invalidation keys",
                   "[Style][Invalidation]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><p id=Id class="a b" title=t lang=en></p>)"};
    auto keys = MakePseudoClassInvalidationKeys(css::SelectorPseudoClass::Hover, fixture.ById(u8"Id"));

    auto key = [](InvalidationKeyType type, dom::DOMStringView value = {})
    {
      return value.empty() ? MakePseudoClassInvalidationKey(css::SelectorPseudoClass::Hover, type)
                           : MakePseudoClassInvalidationKey(css::SelectorPseudoClass::Hover, type,
                                                            css::CSSOMStringAtom(value));
    };
    REQUIRE(keys.size() == 7);
    CHECK(keys[0] == key(InvalidationKeyType::Id, u8"Id"));
    CHECK(keys[1] == key(InvalidationKeyType::Class, u8"a"));
    CHECK(keys[2] == key(InvalidationKeyType::Class, u8"b"));
    // `id` and `class` are unlikely to have attribute selectors; `title` and `lang` aren't.
    CHECK(keys[3] == key(InvalidationKeyType::Attribute, u8"title"));
    CHECK(keys[4] == key(InvalidationKeyType::Attribute, u8"lang"));
    CHECK(keys[5] == key(InvalidationKeyType::Tag, u8"p"));
    CHECK(keys[6] == key(InvalidationKeyType::Universal));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "PseudoClassChangeInvalidation: element states",
                   "[Style][Invalidation]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      #c:checked + span, #c:indeterminate + span, :link, :default + span { color: rgb(0, 0, 255) }
      #r:required + span, #r:read-only + span, #r:placeholder-shown + span { color: rgb(0, 0, 255) }
      option:checked { color: rgb(0, 0, 255) }
    </style><body>
      <input id=c type=checkbox><span id=cs></span>
      <a id=a></a>
      <input id=r><span id=rs></span>
      <select><option>first</option><option id=o>o</option></select>
    </body>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    auto &cs = fixture.ById(u8"cs");
    auto &rs = fixture.ById(u8"rs");
    auto &input = HTML<html::HTMLInputElement>(fixture.ById(u8"c"));
    auto &text = HTML<html::HTMLInputElement>(fixture.ById(u8"r"));
    REQUIRE_FALSE(IsBlue(cs));

    SECTION(":checked")
    {
      input.Checked(true);
      CHECK(cs.StyleValidity() == Validity::ElementInvalid);
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(cs));

      input.Checked(false);
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(IsBlue(cs));
    }

    SECTION(":indeterminate")
    {
      input.Indeterminate(true);
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(cs));
    }

    SECTION(":checked and :default for options")
    {
      auto &option = HTML<html::HTMLOptionElement>(fixture.ById(u8"o"));
      option.Selected(true);
      CHECK(option.StyleValidity() == Validity::ElementInvalid);
      document.UpdateStyleIfNeeded();
      // Options aren't rendered, so their style is computed on demand.
      auto *style = option.ComputedStyle();
      REQUIRE(style);
      CHECK(style->GetColor() == Blue);
    }

    SECTION(":link")
    {
      auto &a = fixture.ById(u8"a");
      REQUIRE_FALSE(a.SetAttribute(u8"href", u8"#x").HasException());
      CHECK(a.StyleValidity() == Validity::ElementInvalid);
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(a));

      // Changing the URL doesn't change whether it's a link.
      REQUIRE_FALSE(a.SetAttribute(u8"href", u8"#y").HasException());
      CHECK(a.StyleValidity() == Validity::Valid);
    }

    SECTION(":required, :read-only and :placeholder-shown")
    {
      REQUIRE_FALSE(text.SetAttribute(u8"required", u8"").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(rs));
      text.RemoveAttribute(u8"required");
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(IsBlue(rs));

      REQUIRE_FALSE(text.SetAttribute(u8"readonly", u8"").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(rs));
      text.RemoveAttribute(u8"readonly");
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(IsBlue(rs));

      REQUIRE_FALSE(text.SetAttribute(u8"placeholder", u8"p").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(rs));
      text.Value(u8"v");
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(IsBlue(rs));
    }

    SECTION(":checked and :default follow the input's type")
    {
      REQUIRE_FALSE(text.SetAttribute(u8"checked", u8"").HasException());
      document.UpdateStyleIfNeeded();
      REQUIRE_FALSE(IsBlue(rs));
      // A checked radio button is :default (and :read-only).
      REQUIRE_FALSE(text.SetAttribute(u8"type", u8"radio").HasException());
      CHECK(rs.StyleValidity() != Validity::Valid);
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(rs));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "PseudoClassChangeInvalidation: inherited and tree states",
                   "[Style][Invalidation]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      p:lang(fr), p:dir(rtl), :disabled, p:read-write { color: rgb(0, 0, 255) }
      :default { width: 5px }
      textarea:placeholder-shown { height: 5px }
    </style><body>
      <div id=d><p id=p></p><p id=own lang=en dir=ltr></p></div>
      <fieldset id=f><legend><input id=l></legend><input id=i></fieldset>
      <form id=form><button id=b1></button></form>
      <textarea id=t placeholder=p></textarea>
    </body>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    auto &d = fixture.ById(u8"d");
    auto &p = fixture.ById(u8"p");
    auto &own = fixture.ById(u8"own");

    SECTION(":lang() follows the ancestors' lang")
    {
      REQUIRE_FALSE(d.SetAttribute(u8"lang", u8"fr").HasException());
      CHECK(p.StyleValidity() == Validity::ElementInvalid);
      CHECK(own.StyleValidity() == Validity::Valid);
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(p));
      CHECK_FALSE(IsBlue(own));
    }

    SECTION(":dir() follows the ancestors' dir")
    {
      REQUIRE_FALSE(d.SetAttribute(u8"dir", u8"rtl").HasException());
      CHECK(p.StyleValidity() == Validity::ElementInvalid);
      CHECK(own.StyleValidity() == Validity::Valid);
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(p));
    }

    SECTION(":read-write follows contenteditable")
    {
      REQUIRE_FALSE(d.SetAttribute(u8"contenteditable", u8"").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(p));
      CHECK(IsBlue(own));
    }

    SECTION(":disabled follows disabled fieldsets and their first legend")
    {
      auto &f = fixture.ById(u8"f");
      auto &i = fixture.ById(u8"i");
      auto &l = fixture.ById(u8"l");
      REQUIRE_FALSE(f.SetAttribute(u8"disabled", u8"").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(i));
      CHECK_FALSE(IsBlue(l));

      // Another first legend disables the controls in the old one.
      auto legend = Create(document, u8"legend");
      REQUIRE_FALSE(f.InsertBefore(*legend, l.ParentElement()).HasException());
      CHECK(l.StyleValidity() != Validity::Valid);
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(l));

      REQUIRE_FALSE(f.RemoveChild(*legend).HasException());
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(IsBlue(l));
    }

    SECTION(":default follows the form's first submit button")
    {
      auto &form = fixture.ById(u8"form");
      auto &b1 = fixture.ById(u8"b1");
      auto width = [&](dom::Element &element)
      {
        auto *style = element.GetRenderStyle();
        REQUIRE(style);
        return ResolverFixture::Serialised(*style, style->Width());
      };
      REQUIRE(width(b1) == "5px");

      auto b0 = Create(document, u8"button");
      REQUIRE_FALSE(form.InsertBefore(*b0, &b1).HasException());
      // The form invalidates the subtrees of its old and new default buttons.
      CHECK(b1.StyleValidity() >= Validity::ElementInvalid);
      document.UpdateStyleIfNeeded();
      CHECK(width(*b0) == "5px");
      CHECK(width(b1) == "auto");

      REQUIRE_FALSE(b0->SetAttribute(u8"type", u8"button").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(width(*b0) == "auto");
      CHECK(width(b1) == "5px");
    }

    SECTION(":placeholder-shown follows a text area's text")
    {
      auto &t = fixture.ById(u8"t");
      auto height = [&]
      {
        auto *style = t.GetRenderStyle();
        REQUIRE(style);
        return ResolverFixture::Serialised(*style, style->Height());
      };
      REQUIRE(height() == "5px");
      auto text = document.CreateTextNode(u8"x");
      REQUIRE_FALSE(t.AppendChild(*text).HasException());
      // The text area's new value invalidates its subtree.
      CHECK(t.StyleValidity() >= Validity::ElementInvalid);
      document.UpdateStyleIfNeeded();
      CHECK(height() == "auto");

      REQUIRE_FALSE(text->ReplaceData(0, 1, u8"").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(height() == "5px");
    }
  }
}
