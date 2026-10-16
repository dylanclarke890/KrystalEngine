#pragma once

#include "Krystal.Booey.Tests/CSS/RuleTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/StylePropertiesTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/Builder.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/MatchResult.hpp"
#include "Krystal.Booey/Style/PropertyCascade.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <string>

namespace krys::boo::style::tests
{
  /// @brief Fixture for tests that convert parsed CSS values to computed values: a document, a style and its
  /// parent style, and a builder state over them.
  /// @note The state has no current property (only `Builder` applies properties), so conversions that read
  /// the property's id, origin or style scope are tested with `Build`.
  struct BuilderFixture : css::tests::ValueFixture
  {
    Ref<dom::HTMLDocument> Document {CreateRef<dom::HTMLDocument>()};
    // Both start with the document's default font (16px), as resolved styles do.
    RenderStyle ParentStyle {RenderStyle::Clone(*Resolver::DefaultStyleForElement(*Document, nullptr))};
    RenderStyle Style {RenderStyle::Clone(*Resolver::DefaultStyleForElement(*Document, nullptr))};
    RefPtr<const dom::Element> Element;
    Maybe<BuilderPositionTryFallback> PositionTryFallback;
    UniquePtr<BuilderState> State;

    BuilderFixture() noexcept
    {
      Reset();
    }

    /// @brief Recreates the builder state, for `element` if given.
    void Reset(const dom::Element *element = nullptr) noexcept
    {
      Element = ShareRefPtr(element);
      ResetState();
    }

    /// @brief Recreates the builder state for the same element and position-try fallback, which clears what
    /// the last conversion marked invalid at computed-value time.
    void ResetState() noexcept
    {
      State.reset();
      State = CreateUnique<BuilderState>(Style, BuilderContext {.Document = Document,
                                                                .ParentStyle = &ParentStyle,
                                                                .RootElementStyle = nullptr,
                                                                .Element = Element,
                                                                .PositionTryFallback = PositionTryFallback});
    }

    /// @brief A parser context with every settings-gated property the value tests use enabled.
    KRYS_NODISCARD static css::ParserContext ParseContext() noexcept
    {
      auto context = css::tests::DeclarationTestContext();
      context.PropertySettings.colorFilterEnabled = true;
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

    /// @brief Builds the style from `property: value`, declared by the author in `scope`, as the resolver
    /// would: the conversions see the property being applied.
    void Build(css::PropertyId property, utf8_stringview value,
               ScopeOrdinal scope = ScopeOrdinal::Element) noexcept
    {
      utf8_string declaration {css::ToString(property)};
      declaration += u8": ";
      declaration += value;
      auto properties = css::MutableStyleProperties::Create(css::ParserMode::HTMLStandard);
      css::tests::AddDeclaration(*properties, declaration, ParseContext());
      krys_assert_msg(properties->PropertyCount(), "The value doesn't parse");

      auto matched = MatchResult::Create();
      matched->AuthorDeclarations.push_back(
        MatchedProperties {.Properties = properties->ImmutableCopyIfNeeded(), .StyleScopeOrdinal = scope});
      Builder builder {Style,
                       BuilderContext {.Document = Document,
                                       .ParentStyle = &ParentStyle,
                                       .Element = Element,
                                       .PositionTryFallback = PositionTryFallback},
                       *matched};
      builder.ApplyAllProperties();
    }

    /// @brief Converts the parsed `value` of `property` to `T`, as the builder applies it.
    template <typename T>
    KRYS_NODISCARD T Convert(css::PropertyId property, utf8_stringview value) noexcept
    {
      auto parsed = Parse(property, value);
      ResetState();
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
