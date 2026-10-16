#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/ExtractorState.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class RenderElement;
  class RenderStyle;
}

namespace krys::boo::css
{
  class MutableStyleProperties;
  class PrimitiveValue;
  class Value;
  class ValuePool;
  struct SerialisationContext;
}

namespace krys::boo::dom
{
  class Element;
  class Node;
}

namespace krys::boo::style
{
  /// @brief Extracts the computed (or resolved) values of an element's properties.
  class Extractor
  {
  private:
    RefPtr<dom::Element> _element;
    Maybe<style::PseudoElementIdentifier> _pseudoElementIdentifier;
    bool _allowVisitedStyle;

  public:
    explicit Extractor(dom::Node *, bool allowVisitedStyle = false) noexcept;
    Extractor(dom::Node *, bool allowVisitedStyle, const Maybe<style::PseudoElementIdentifier> &) noexcept;
    explicit Extractor(dom::Element *, bool allowVisitedStyle = false) noexcept;
    Extractor(dom::Element *, bool allowVisitedStyle, const Maybe<style::PseudoElementIdentifier> &) noexcept;

    enum class UpdateLayout : bool
    {
      No,
      Yes
    };

    /// @brief Extract a css::Value for the specified property.
    KRYS_NODISCARD RefPtr<css::Value> PropertyValue(
      css::PropertyId, UpdateLayout = UpdateLayout::Yes,
      ExtractorState::PropertyValueType = ExtractorState::PropertyValueType::Resolved) const noexcept;

    /// @brief Extract a serialised value for the specified property.
    KRYS_NODISCARD css::CSSOMString PropertyValueSerialisation(
      css::PropertyId, const css::SerialisationContext &, UpdateLayout = UpdateLayout::Yes,
      ExtractorState::PropertyValueType = ExtractorState::PropertyValueType::Resolved) const noexcept;

    /// @brief Extract a css::Value for the specified property using the provided RenderStyle and
    /// RenderElement.
    KRYS_NODISCARD RefPtr<css::Value> PropertyValueInStyle(
      const RenderStyle &, css::PropertyId, css::ValuePool &, const RenderElement * = nullptr,
      ExtractorState::PropertyValueType = ExtractorState::PropertyValueType::Resolved) const noexcept;

    /// @brief Extract a serialised value for the specified property using the provided RenderStyle and
    /// RenderElement.
    KRYS_NODISCARD css::CSSOMString PropertyValueSerialisationInStyle(
      const RenderStyle &, css::PropertyId, const css::SerialisationContext &, css::ValuePool &,
      const RenderElement * = nullptr,
      ExtractorState::PropertyValueType = ExtractorState::PropertyValueType::Resolved) const noexcept;

    /// @brief Extract a css::Value for the specified custom property.
    KRYS_NODISCARD RefPtr<css::Value>
      CustomPropertyValue(const css::CSSOMStringAtom &propertyName) const noexcept;

    /// @brief Extract a serialised value for the specified custom property.
    KRYS_NODISCARD css::CSSOMString
      CustomPropertyValueSerialisation(const css::CSSOMStringAtom &propertyName,
                                       const css::SerialisationContext &) const noexcept;

    /// @brief Extract a serialised value for the specified custom property using the provided RenderStyle.
    KRYS_NODISCARD css::CSSOMString
      CustomPropertyValueSerialisationInStyle(const RenderStyle &, const css::CSSOMStringAtom &propertyName,
                                              const css::SerialisationContext &) const noexcept;

    // Helper methods for HTML editing.
    KRYS_NODISCARD Ref<css::MutableStyleProperties>
      CopyProperties(Span<const css::PropertyId>) const noexcept;
    KRYS_NODISCARD Ref<css::MutableStyleProperties> CopyProperties() const noexcept;
    KRYS_NODISCARD RefPtr<css::PrimitiveValue> GetFontSizeCSSValuePreferringKeyword() const noexcept;
    KRYS_NODISCARD bool UseFixedFontDefaultSize() const noexcept;
    KRYS_NODISCARD bool PropertyMatches(css::PropertyId, const css::Value *) const noexcept;

    static bool UpdateStyleIfNeededForProperty(dom::Element &, css::PropertyId) noexcept;

  private:
    /// @brief The renderer we should use for resolving layout-dependent properties.
    KRYS_NODISCARD const RenderElement *ComputeRenderer() const noexcept;

    /// @brief The RenderStyle we should use for resolving non-layout-dependent properties.
    KRYS_NODISCARD const RenderStyle *ComputeStyle(css::PropertyId, UpdateLayout,
                                                   UniquePtr<RenderStyle> &) const noexcept;

    /// @brief The RenderStyle we should use for resolving custom properties.
    KRYS_NODISCARD const RenderStyle *ComputeStyleForCustomProperty(UniquePtr<RenderStyle> &) const noexcept;
  };
}
