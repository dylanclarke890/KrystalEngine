#include "Krystal.Booey/Style/Extractor.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/Text.hpp"

namespace krys::boo::style::tests
{
  namespace
  {
    KRYS_NODISCARD std::string Computed(const Extractor &extractor, css::PropertyId property)
    {
      return css::tests::ToNarrow(
        extractor.PropertyValueSerialisation(property, css::DefaultSerialisationContext()));
    }

    KRYS_NODISCARD std::string ComputedValue(const Extractor &extractor, css::PropertyId property)
    {
      auto value = extractor.PropertyValue(property);
      REQUIRE(value);
      return css::tests::ToNarrow(value->CssText(css::DefaultSerialisationContext()));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Extractor: longhands", "[Style][Extractor]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      #a { color: rgb(0, 0, 255); margin-top: 3px; font-size: 20px; line-height: 1.5; letter-spacing: 0;
           width: 50%; min-width: auto; max-height: none; transform: rotate(45deg); counter-reset: x 2 y;
           position: absolute; float: left; widows: 3; white-space: pre; z-index: 4 }
      #b { line-height: 150%; letter-spacing: 2px; margin-inline-start: 7px; direction: rtl }
    </style><div id=a></div><div id=b></div>)"};
    auto &a = fixture.ById(u8"a");
    auto &b = fixture.ById(u8"b");
    Extractor ea {&a};
    Extractor eb {&b};

    // The extractor brings the styles up to date.
    CHECK(fixture.Document->NeedsStyleRecalc());
    CHECK(Computed(ea, css::PropertyId::Color) == "rgb(0, 0, 255)");
    CHECK_FALSE(fixture.Document->NeedsStyleRecalc());

    CHECK(Computed(ea, css::PropertyId::Display) == "block");
    CHECK(Computed(ea, css::PropertyId::MarginTop) == "3px");
    CHECK(Computed(ea, css::PropertyId::FontSize) == "20px");
    CHECK(Computed(ea, css::PropertyId::LineHeight) == "30px");
    CHECK(Computed(eb, css::PropertyId::LineHeight) == "24px");
    CHECK(Computed(ea, css::PropertyId::ZIndex) == "4");
    CHECK(Computed(ea, css::PropertyId::Widows) == "3");
    CHECK(Computed(eb, css::PropertyId::Widows) == "2");

    // A computed letter-spacing of zero is `normal`.
    CHECK(Computed(ea, css::PropertyId::LetterSpacing) == "normal");
    CHECK(Computed(eb, css::PropertyId::LetterSpacing) == "2px");

    // Without a renderer, sizes are their computed values.
    CHECK(Computed(ea, css::PropertyId::Width) == "50%");
    CHECK(Computed(ea, css::PropertyId::MinWidth) == "0px");
    CHECK(Computed(ea, css::PropertyId::MaxHeight) == "none");

    // Out-of-flow boxes don't float, and a resolved transform needs a renderer.
    CHECK(Computed(ea, css::PropertyId::Float) == "none");
    CHECK(Computed(ea, css::PropertyId::Transform) == "none");
    CHECK(css::tests::ToNarrow(ea.PropertyValue(css::PropertyId::Transform, Extractor::UpdateLayout::Yes,
                                                ExtractorState::PropertyValueType::Computed)
                                 ->CssText(css::DefaultSerialisationContext()))
          == "rotate(45deg)");

    CHECK(Computed(ea, css::PropertyId::CounterReset) != "none");
    CHECK(Computed(ea, css::PropertyId::CounterIncrement) == "none");

    // Logical properties resolve against the writing mode.
    CHECK(Computed(eb, css::PropertyId::MarginInlineStart) == "7px");
    CHECK(Computed(eb, css::PropertyId::MarginRight) == "7px");
    CHECK(Computed(eb, css::PropertyId::Direction) == "rtl");

    // The document element's direction is its initial value unless it is set explicitly.
    Extractor root {fixture.Document->DocumentElement().get()};
    CHECK(Computed(root, css::PropertyId::Direction) == "ltr");

    CHECK(ComputedValue(ea, css::PropertyId::Color) == "rgb(0, 0, 255)");
    CHECK(ComputedValue(ea, css::PropertyId::WhiteSpace) == "pre");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Extractor: shorthands", "[Style][Extractor]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      #a { margin: 1px 2px; padding: 3px; border-radius: 1px 2px; gap: 4px 5px; flex-flow: column wrap;
           border: 2px solid rgb(255, 0, 0); text-decoration: underline; overflow: hidden scroll;
           columns: 3; font: italic 12px/2 serif }
      #b { border-top: 1px solid; border-bottom: 2px solid; flex-flow: row wrap }
    </style><div id=a></div><div id=b></div>)"};
    Extractor ea {&fixture.ById(u8"a")};
    Extractor eb {&fixture.ById(u8"b")};

    CHECK(Computed(ea, css::PropertyId::Margin) == "1px 2px");
    CHECK(Computed(ea, css::PropertyId::Padding) == "3px");
    CHECK(Computed(ea, css::PropertyId::BorderRadius) == "1px 2px");
    CHECK(Computed(ea, css::PropertyId::Gap) == "4px 5px");
    CHECK(Computed(ea, css::PropertyId::FlexFlow) == "column wrap");
    CHECK(Computed(eb, css::PropertyId::FlexFlow) == "wrap");
    CHECK(Computed(ea, css::PropertyId::Border) == "2px solid rgb(255, 0, 0)");
    CHECK(Computed(ea, css::PropertyId::Overflow) == "hidden scroll");
    CHECK(Computed(ea, css::PropertyId::Columns) == "3");
    CHECK(Computed(ea, css::PropertyId::Font) == "italic 12px / 24px serif");

    // A border whose sides differ has no shorthand value.
    CHECK(Computed(eb, css::PropertyId::Border).empty());
    CHECK_FALSE(eb.PropertyValue(css::PropertyId::Border));

    CHECK(ComputedValue(ea, css::PropertyId::Margin) == "1px 2px");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Extractor: custom properties and copies", "[Style][Extractor]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      #a { --x: 1px  2px; color: rgb(1, 2, 3) }
    </style><div id=a><span id=b></span></div>)"};
    Extractor ea {&fixture.ById(u8"a")};
    Extractor eb {&fixture.ById(u8"b")};

    CHECK(
      css::tests::ToNarrow(ea.CustomPropertyValueSerialisation(u8"--x", css::DefaultSerialisationContext()))
      == "1px  2px");
    CHECK(
      css::tests::ToNarrow(eb.CustomPropertyValueSerialisation(u8"--x", css::DefaultSerialisationContext()))
      == "1px  2px");
    CHECK(ea.CustomPropertyValueSerialisation(u8"--y", css::DefaultSerialisationContext()).empty());
    CHECK_FALSE(ea.CustomPropertyValue(u8"--y"));
    REQUIRE(ea.CustomPropertyValue(u8"--x"));

    auto copy = ea.CopyProperties();
    CHECK(css::tests::ToNarrow(copy->GetPropertyValue(css::PropertyId::Color)) == "rgb(1, 2, 3)");

    auto value = ea.PropertyValue(css::PropertyId::Color);
    CHECK(ea.PropertyMatches(css::PropertyId::Color, value.get()));
    CHECK_FALSE(eb.PropertyMatches(css::PropertyId::Display, value.get()));

    // An extractor for a text node uses its parent element.
    auto text = fixture.Document->CreateTextNode(u8"t");
    REQUIRE_FALSE(fixture.ById(u8"a").AppendChild(*text).HasException());
    Extractor et {static_cast<dom::Node *>(text.get())};
    CHECK(Computed(et, css::PropertyId::Color) == "rgb(1, 2, 3)");
    Extractor none {static_cast<dom::Node *>(nullptr)};
    CHECK(Computed(none, css::PropertyId::Color).empty());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Extractor: direct serialisation matches the values",
                   "[Style][Extractor]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      #a { animation: 2s ease-in 1s infinite reverse both paused spin; transition: opacity 1s;
           background: url(https://example.com/a.png) no-repeat 10px 20px / cover, rgb(0, 128, 0); mask: none;
           grid-template: [a] 10px [b] / 1fr repeat(2, 20px); counter-increment: c 2; content: "x";
           box-shadow: 1px 1px red; text-shadow: 2px 2px blue; offset: 10px 20px; scale: 2;
           translate: 1px 2px; rotate: 10deg; font-variant: small-caps; text-wrap: nowrap balance;
           line-clamp: 3; container: card / inline-size; view-timeline: --v inline; scroll-timeline: --s }
    </style><div id=a></div>)"};
    Extractor ea {&fixture.ById(u8"a")};
    CHECK(Computed(ea, css::PropertyId::AnimationName) == "spin");
    CHECK(Computed(ea, css::PropertyId::AnimationDuration) == "2s");
    CHECK(Computed(ea, css::PropertyId::GridTemplateColumns) == "1fr repeat(2, 20px)");
    CHECK(Computed(ea, css::PropertyId::GridTemplateRows) == "[a] 10px [b]");
    CHECK(Computed(ea, css::PropertyId::BackgroundImage) == "url(\"https://example.com/a.png\"), none");
    CHECK(Computed(ea, css::PropertyId::BackgroundRepeat) == "no-repeat, repeat");
    CHECK(Computed(ea, css::PropertyId::BoxShadow) == "rgb(255, 0, 0) 1px 1px 0px 0px");
    CHECK(Computed(ea, css::PropertyId::Content) == "\"x\"");
    CHECK(Computed(ea, css::PropertyId::CounterIncrement) == "c 2");
    CHECK(Computed(ea, css::PropertyId::Scale) == "2");
    CHECK(Computed(ea, css::PropertyId::Animation) == "2s ease-in 1s infinite reverse both paused spin");
    CHECK(Computed(ea, css::PropertyId::Transition) == "opacity 1s");
    CHECK(Computed(ea, css::PropertyId::Container) == "card / inline-size");
    CHECK(Computed(ea, css::PropertyId::TextWrap) == "nowrap balance");

    // Direct serialisation and serialisation through a value must agree for every computed property.
    for (auto property : css::ComputedPropertyIds)
    {
      CAPTURE(css::tests::ToNarrow(css::ToString(property)));
      auto direct = Computed(ea, property);
      auto value = ea.PropertyValue(property);
      if (!value)
      {
        CHECK(direct.empty());
        continue;
      }
      CHECK(direct == css::tests::ToNarrow(value->CssText(css::DefaultSerialisationContext())));
    }
  }
}
