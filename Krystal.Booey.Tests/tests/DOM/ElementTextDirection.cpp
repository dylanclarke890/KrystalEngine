#include "Krystal.Booey/DOM/ElementTextDirection.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/DOM/Dicts/ShadowRootInit.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/HTMLBDIElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextAreaElement.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    // Hebrew and Arabic words: right-to-left strong characters.
    constexpr auto Shalom = u8"שלום";
    constexpr auto Salam = u8"سلام";

    KRYS_NODISCARD Ref<HTMLDocument> Parse(utf8_string &&markup) noexcept
    {
      return style::tests::ResolverFixture::Parse(krys::move(markup));
    }

    KRYS_NODISCARD Element &ById(Document &document, DOMStringView id)
    {
      auto element = document.GetElementById(id);
      REQUIRE(element);
      return *element;
    }

    /// @brief @p element's directionality, checked against `:dir()`.
    KRYS_NODISCARD std::string Dir(const Element &element)
    {
      auto direction = element.EffectiveTextDirection() == TextDirection::LTR ? "ltr" : "rtl";
      auto matches = element.Matches(direction == std::string("ltr") ? u8":dir(ltr)" : u8":dir(rtl)");
      REQUIRE_FALSE(matches.HasException());
      CHECK(matches.Value());
      return direction;
    }

    KRYS_NODISCARD utf8_string Markup(std::initializer_list<utf8_stringview> parts)
    {
      utf8_string markup = u8"<!DOCTYPE html>";
      for (auto part : parts)
      {
        markup += part;
      }
      return markup;
    }
  }

  TEST_CASE("ElementTextDirection: the dir attribute's states", "[DOM][ElementTextDirection]")
  {
    CHECK(ParseTextDirectionState(u8"ltr") == TextDirectionState::LTR);
    CHECK(ParseTextDirectionState(u8"RTL") == TextDirectionState::RTL);
    CHECK(ParseTextDirectionState(u8"Auto") == TextDirectionState::Auto);
    CHECK(ParseTextDirectionState(u8"") == TextDirectionState::Undefined);
    CHECK(ParseTextDirectionState(u8"up") == TextDirectionState::Undefined);
    CHECK(ParseTextDirectionState(null) == TextDirectionState::Undefined);

    auto document = Parse(u8"<!DOCTYPE html><p id=p dir=auto></p><bdi id=b></bdi><bdi id=c dir=ltr></bdi>");
    CHECK(Is<html::HTMLBDIElement>(ById(*document, u8"b")));
    CHECK(ElementHasAutoTextDirectionState(ById(*document, u8"p")));
    CHECK(ElementHasAutoTextDirectionState(ById(*document, u8"b")));
    CHECK_FALSE(ElementHasAutoTextDirectionState(ById(*document, u8"c")));
    CHECK(ElementHasValidTextDirectionState(ById(*document, u8"c")));
    CHECK_FALSE(ElementHasValidTextDirectionState(ById(*document, u8"b")));
  }

  TEST_CASE("ElementTextDirection: auto directionality from text", "[DOM][ElementTextDirection]")
  {
    auto document = Parse(Markup({
      u8"<p id=rtl dir=auto>",
      Shalom,
      u8" world</p>",
      u8"<p id=ltr dir=auto>hello ",
      Shalom,
      u8"</p>",
      u8"<p id=neutral dir=auto>123 !</p>",
      u8"<p id=empty dir=auto></p>",
      u8"<div id=nested dir=auto>(<b>",
      Salam,
      u8"</b>)<span id=child></span></div>",
      u8"<div id=skips dir=auto><bdi>",
      Shalom,
      u8"</bdi><span dir=rtl>",
      Shalom,
      u8"</span>",
      u8"<input value=",
      Shalom,
      u8"><textarea>",
      Shalom,
      u8"</textarea>",
      u8"<style>",
      Shalom,
      u8"{}</style>abc</div>",
      u8"<div id=outer dir=auto><p id=inner dir=auto>abc</p>",
      Shalom,
      u8"</div>",
    }));

    CHECK(Dir(ById(*document, u8"rtl")) == "rtl");
    CHECK(Dir(ById(*document, u8"ltr")) == "ltr");
    // Text without a strong character decides nothing, and nothing is left to right.
    CHECK_FALSE(ComputeAutoDirectionality(ById(*document, u8"neutral")));
    CHECK(Dir(ById(*document, u8"neutral")) == "ltr");
    CHECK(Dir(ById(*document, u8"empty")) == "ltr");
    // Descendants' text counts, and descendants take the directionality.
    CHECK(Dir(ById(*document, u8"nested")) == "rtl");
    CHECK(Dir(ById(*document, u8"child")) == "rtl");
    // <bdi>, elements with a dir, text fields and <style> are skipped.
    CHECK(Dir(ById(*document, u8"skips")) == "ltr");
    // An element whose dir is auto is skipped by its ancestor's.
    CHECK(Dir(ById(*document, u8"outer")) == "rtl");
    CHECK(Dir(ById(*document, u8"inner")) == "ltr");
  }

  TEST_CASE("ElementTextDirection: inherited directionality", "[DOM][ElementTextDirection]")
  {
    auto document = Parse(Markup({
      u8"<div id=rtl dir=rtl><p id=p></p><bdi id=bdi>abc</bdi><bdi id=bdi-rtl>",
      Shalom,
      u8"</bdi>",
      u8"<input id=tel type=tel><input id=text></div>",
      u8"<p id=other></p>",
    }));

    CHECK(Dir(ById(*document, u8"p")) == "rtl");
    CHECK(Dir(ById(*document, u8"other")) == "ltr");
    // A <bdi> without a dir is auto.
    CHECK(Dir(ById(*document, u8"bdi")) == "ltr");
    CHECK(Dir(ById(*document, u8"bdi-rtl")) == "rtl");
    // Telephone fields are left to right.
    CHECK(Dir(ById(*document, u8"tel")) == "ltr");
    CHECK(Dir(ById(*document, u8"text")) == "rtl");

    SECTION("from the document element")
    {
      auto rtlDocument = Parse(u8"<!DOCTYPE html><html dir=rtl><body><p id=p></p></body></html>");
      CHECK(Dir(ById(*rtlDocument, u8"p")) == "rtl");
      CHECK(Dir(*rtlDocument->Body()) == "rtl");
      auto p = rtlDocument->CreateElement(u8"p").ReleaseValue();
      // A detached element takes the document element's.
      CHECK(Dir(*p) == "rtl");
    }

    SECTION("an inserted element takes its parent's")
    {
      auto span = document->CreateElement(u8"span").ReleaseValue();
      REQUIRE_FALSE(ById(*document, u8"p").AppendChild(*span).HasException());
      CHECK(Dir(*span) == "rtl");
      REQUIRE_FALSE(ById(*document, u8"other").AppendChild(*span).HasException());
      CHECK(Dir(*span) == "ltr");
      REQUIRE_FALSE(span->Remove().HasException());
      CHECK(Dir(*span) == "ltr");
    }

    SECTION("dir changes")
    {
      auto &rtl = ById(*document, u8"rtl");
      REQUIRE_FALSE(rtl.SetAttribute(u8"dir", u8"ltr").HasException());
      CHECK(Dir(ById(*document, u8"p")) == "ltr");
      CHECK(Dir(ById(*document, u8"text")) == "ltr");
      // Auto: the <bdi>s and the text fields are skipped, which leaves no strong character.
      REQUIRE_FALSE(rtl.SetAttribute(u8"dir", u8"auto").HasException());
      CHECK(Dir(rtl) == "ltr");
      CHECK(Dir(ById(*document, u8"p")) == "ltr");
      REQUIRE_FALSE(ById(*document, u8"p").AppendChild(*document->CreateTextNode(Shalom)).HasException());
      CHECK(Dir(rtl) == "rtl");
      CHECK(Dir(ById(*document, u8"text")) == "rtl");
      rtl.RemoveAttribute(u8"dir");
      CHECK(Dir(rtl) == "ltr");
      CHECK(Dir(ById(*document, u8"p")) == "ltr");
    }
  }

  TEST_CASE("ElementTextDirection: changes to the text", "[DOM][ElementTextDirection]")
  {
    auto document = Parse(u8"<!DOCTYPE html><div id=d dir=auto><span id=s>abc</span><p id=p></p></div>");
    auto &d = ById(*document, u8"d");
    auto &s = ById(*document, u8"s");
    auto &p = ById(*document, u8"p");
    auto &text = Downcast<Text>(*s.FirstChild());
    CHECK(Dir(d) == "ltr");

    SECTION("changing the data")
    {
      REQUIRE_FALSE(text.Data(utf8_string(Shalom)).HasException());
      CHECK(Dir(d) == "rtl");
      CHECK(Dir(p) == "rtl");
      REQUIRE_FALSE(text.InsertData(0, u8"x").HasException());
      CHECK(Dir(d) == "ltr");
      CHECK(Dir(p) == "ltr");
    }

    SECTION("inserting and removing text")
    {
      REQUIRE_FALSE(s.InsertBefore(*document->CreateTextNode(Salam), &text).HasException());
      CHECK(Dir(d) == "rtl");
      CHECK(Dir(s) == "rtl");
      REQUIRE_FALSE(Downcast<Text>(*s.FirstChild()).Remove().HasException());
      CHECK(Dir(d) == "ltr");
      CHECK(Dir(s) == "ltr");
    }

    SECTION("inserting an element with text")
    {
      auto b = document->CreateElement(u8"b").ReleaseValue();
      REQUIRE_FALSE(b->AppendChild(*document->CreateTextNode(Shalom)).HasException());
      REQUIRE_FALSE(d.InsertBefore(*b, &s).HasException());
      CHECK(Dir(d) == "rtl");
      CHECK(Dir(*b) == "rtl");
      // Text later added to the inserted element still counts.
      auto &bText = Downcast<Text>(*b->FirstChild());
      REQUIRE_FALSE(bText.Data(u8"abc").HasException());
      CHECK(Dir(d) == "ltr");
    }

    SECTION("replaced elements are neutral")
    {
      REQUIRE_FALSE(text.Data(u8"").HasException());
      CHECK(Dir(d) == "ltr");
      auto image = document->CreateElement(u8"img").ReleaseValue();
      REQUIRE_FALSE(image->SetAttribute(u8"alt", utf8_string(Shalom)).HasException());
      REQUIRE_FALSE(d.InsertBefore(*image, &s).HasException());
      CHECK(Dir(d) == "ltr");
    }
  }

  TEST_CASE("ElementTextDirection: form controls", "[DOM][ElementTextDirection]")
  {
    auto document = Parse(Markup({
      u8"<input id=i dir=auto value=",
      Shalom,
      u8">",
      u8"<input id=empty dir=auto>",
      u8"<input id=image type=image dir=auto value=",
      Shalom,
      u8">",
      u8"<input id=button type=button dir=auto value=",
      Shalom,
      u8">",
      u8"<textarea id=t dir=auto>abc</textarea>",
      u8"<div id=d dir=auto>",
      Shalom,
      u8"<input id=named name=n dirname=n.dir></div>",
    }));
    auto &i = Downcast<html::HTMLInputElement>(ById(*document, u8"i"));
    auto &t = Downcast<html::HTMLTextAreaElement>(ById(*document, u8"t"));

    CHECK(Dir(i) == "rtl");
    // An empty value decides nothing; a non-empty one without a strong character is ltr.
    CHECK_FALSE(ComputeAutoDirectionality(ById(*document, u8"empty")));
    // An image button's value doesn't count; a button's does.
    CHECK(Dir(ById(*document, u8"image")) == "ltr");
    CHECK(Dir(ById(*document, u8"button")) == "rtl");
    CHECK(Dir(t) == "ltr");

    SECTION("values")
    {
      REQUIRE_FALSE(i.SetValue(u8"123").HasException());
      CHECK(ComputeAutoDirectionality(i) == TextDirection::LTR);
      CHECK(Dir(i) == "ltr");
      REQUIRE_FALSE(t.SetValue(utf8_string(Salam)).HasException());
      CHECK(Dir(t) == "rtl");
      // The value attribute, while the value isn't dirty.
      auto &empty = Downcast<html::HTMLInputElement>(ById(*document, u8"empty"));
      REQUIRE_FALSE(empty.SetAttribute(u8"value", utf8_string(Salam)).HasException());
      CHECK(Dir(empty) == "rtl");
    }

    SECTION("type changes")
    {
      REQUIRE_FALSE(i.SetAttribute(u8"type", u8"tel").HasException());
      CHECK(Dir(i) == "rtl");
      REQUIRE_FALSE(i.SetAttribute(u8"type", u8"image").HasException());
      CHECK(Dir(i) == "ltr");
      REQUIRE_FALSE(i.SetAttribute(u8"type", u8"text").HasException());
      CHECK(Dir(i) == "rtl");
    }

    SECTION("dirname's direction")
    {
      auto &named = Downcast<html::HTMLInputElement>(ById(*document, u8"named"));
      CHECK(named.DirectionForFormData() == u8"rtl");
      REQUIRE_FALSE(ById(*document, u8"d").SetAttribute(u8"dir", u8"ltr").HasException());
      CHECK(named.DirectionForFormData() == u8"ltr");
      CHECK(i.DirectionForFormData() == u8"rtl");
    }
  }

  TEST_CASE("ElementTextDirection: slots", "[DOM][ElementTextDirection]")
  {
    auto document =
      Parse(Markup({u8"<div id=host><span>", Shalom, u8"</span></div><div id=other>abc</div>"}));
    auto &host = ById(*document, u8"host");
    auto shadow = host.AttachShadow({}).ReleaseValue();
    auto slot = document->CreateElement(u8"slot").ReleaseValue();
    REQUIRE_FALSE(slot->SetAttribute(u8"dir", u8"auto").HasException());
    auto wrapper = document->CreateElement(u8"p").ReleaseValue();
    REQUIRE_FALSE(wrapper->SetAttribute(u8"dir", u8"auto").HasException());
    REQUIRE_FALSE(wrapper->AppendChild(*slot).HasException());
    REQUIRE_FALSE(shadow->AppendChild(*wrapper).HasException());

    // A slot's auto directionality is its assigned nodes'.
    CHECK(Dir(*slot) == "rtl");
    // An auto element containing a slot takes the host's directionality.
    CHECK(Dir(*wrapper) == "ltr");

    // Assigning other nodes updates it.
    REQUIRE_FALSE(Downcast<Element>(*host.FirstChild()).Remove().HasException());
    REQUIRE_FALSE(host.AppendChild(*document->CreateTextNode(u8"abc")).HasException());
    CHECK(Dir(*slot) == "ltr");
    REQUIRE_FALSE(host.SetAttribute(u8"dir", u8"rtl").HasException());
    CHECK(Dir(*wrapper) == "rtl");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ElementTextDirection: the direction of dir=auto",
                   "[DOM][ElementTextDirection]")
  {
    style::tests::ResolverFixture fixture {
      Markup({u8"<p id=p dir=auto>", Shalom, u8"</p><bdi id=b>abc</bdi>"})};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    auto &p = fixture.ById(u8"p");

    auto direction = [](const Element &element)
    {
      auto *style = element.GetRenderStyle();
      REQUIRE(style);
      return style->GetWritingMode().ComputedTextDirection();
    };
    CHECK(direction(p) == TextDirection::RTL);
    CHECK(direction(fixture.ById(u8"b")) == TextDirection::LTR);

    // Changing the text that decides it restyles the element.
    REQUIRE_FALSE(Downcast<Text>(*p.FirstChild()).Data(u8"abc").HasException());
    CHECK(p.NeedsStyleRecalc());
    document.UpdateStyleIfNeeded();
    CHECK(direction(p) == TextDirection::LTR);
  }
}
