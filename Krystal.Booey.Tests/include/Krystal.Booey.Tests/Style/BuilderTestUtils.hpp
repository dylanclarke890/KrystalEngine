#pragma once

#include "Krystal.Booey.Tests/CSS/RuleTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/StylePropertiesTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/Style/PropertyCascade.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <string>

namespace krys::boo::style::tests
{
  /// @brief Fixture for tests that convert parsed CSS values to computed values: a document, a style and its
  /// parent style, and a builder state over them.
  struct BuilderFixture : css::tests::ValueFixture
  {
    Ref<dom::HTMLDocument> Document {CreateRef<dom::HTMLDocument>()};
    RenderStyle ParentStyle;
    RenderStyle Style;
    PropertyCascade::Property CurrentProperty {};
    UniquePtr<BuilderState> State;

    BuilderFixture() noexcept
    {
      Reset();
    }

    /// @brief Recreates the builder state, for `element` if given.
    void Reset(const dom::Element *element = nullptr) noexcept
    {
      State.reset();
      State = CreateUnique<BuilderState>(Style, BuilderContext {.Document = Document,
                                                                .ParentStyle = &ParentStyle,
                                                                .RootElementStyle = nullptr,
                                                                .Element = ShareRefPtr(element)});
      State->SetCurrentProperty(&CurrentProperty);
    }

    /// @brief A parser context with every settings-gated property the value tests use enabled.
    KRYS_NODISCARD static css::ParserContext ParseContext() noexcept
    {
      auto context = css::tests::DeclarationTestContext();
      context.PropertySettings.accentColorEnabled = true;
      context.PropertySettings.cssAnchorPositioningEnabled = true;
      context.PropertySettings.cssLineFitEdgeEnabled = true;
      context.PropertySettings.cssMathDepthEnabled = true;
      context.PropertySettings.cssRhythmicSizingEnabled = true;
      context.PropertySettings.cssScrollbarColorEnabled = true;
      context.PropertySettings.cssScrollbarGutterEnabled = true;
      context.PropertySettings.cssScrollbarWidthEnabled = true;
      context.PropertySettings.cssTextAutospaceEnabled = true;
      context.PropertySettings.cssTextBoxTrimEnabled = true;
      context.PropertySettings.cssTextSpacingTrimEnabled = true;
      context.PropertySettings.scrollDrivenAnimationsEnabled = true;
      context.PropertySettings.viewTransitionsEnabled = true;
      context.springTimingFunctionEnabled = true;
      return context;
    }

    /// @brief Parses `value` as the value of `property`.
    KRYS_NODISCARD Ref<css::Value> Parse(css::PropertyId property, utf8_stringview value) noexcept
    {
      utf8_string declaration {css::ToString(property)};
      declaration += u8": ";
      declaration += value;
      auto properties = css::MutableStyleProperties::Create(css::ParserMode::HTMLStandard);
      css::tests::AddDeclaration(*properties, declaration, ParseContext());
      auto parsed = properties->GetPropertyCSSValue(property);
      krys_assert_msg(parsed, "The value doesn't parse");
      Ref<css::Value> result = krys::move(parsed);
      return result;
    }

    /// @brief Converts the parsed `value` of `property` to `T`, as the builder applies it.
    template <typename T>
    KRYS_NODISCARD T Convert(css::PropertyId property, utf8_stringview value) noexcept
    {
      auto parsed = Parse(property, value);
      CurrentProperty.Id = property;
      return ConvertCSSValueToStyle<T>(*State, *parsed);
    }

    /// @brief Whether the last conversion made the current property invalid at computed-value time.
    KRYS_NODISCARD bool IsInvalid() const noexcept
    {
      return State->IsCurrentPropertyInvalidAtComputedValueTime();
    }

    /// @brief Serialises a computed value.
    template <typename T>
    KRYS_NODISCARD std::string Serialised(const T &value) const noexcept
    {
      return css::tests::ToNarrow(Serialise(css::DefaultSerialisationContext(), Style, value));
    }

    /// @brief Creates a CSS value from a computed value and serialises it.
    template <typename T>
    KRYS_NODISCARD std::string Created(const T &value) const noexcept
    {
      return css::tests::CssText(*CreateCSSValue(css::ValuePool::MainThreadPool(), Style, value));
    }
  };
}
