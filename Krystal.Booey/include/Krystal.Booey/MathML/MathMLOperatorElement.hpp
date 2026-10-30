#pragma once

#include "Krystal.Booey/MathML/MathMLOperatorDictionary.hpp"
#include "Krystal.Booey/MathML/MathMLTokenElement.hpp"

namespace krys::boo::mathml
{
  /// @brief The `<mo>` element: an operator, whose form, spacing and properties come from its attributes and
  /// the operator dictionary.
  /// @see https://w3c.github.io/mathml-core/#operator-fence-separator-or-accent-mo
  class MathMLOperatorElement final : public MathMLTokenElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(MathMLOperatorElement);

  public:
    struct OperatorChar
    {
      char32 Character {0};
      bool IsVertical {true};
    };

  private:
    struct OperatorProperties
    {
      uint16 Flags {0};
      uint16 DirtyFlags {MathMLOperatorDictionary::AllFlags};
    };

    Maybe<OperatorChar> _operatorChar;
    Maybe<MathMLOperatorDictionary::Property> _dictionaryProperty;
    OperatorProperties _properties;
    Maybe<Length> _leadingSpace;
    Maybe<Length> _trailingSpace;
    Maybe<Length> _minSize;
    Maybe<Length> _maxSize;

  public:
    explicit MathMLOperatorElement(dom::Document &document) noexcept;

    KRYS_NODISCARD static OperatorChar ParseOperatorChar(dom::DOMStringView string) noexcept;
    KRYS_NODISCARD const OperatorChar &GetOperatorChar() noexcept;

    void SetOperatorFormDirty() noexcept
    {
      _dictionaryProperty.reset();
    }

    KRYS_NODISCARD MathMLOperatorDictionary::Form Form() noexcept
    {
      return DictionaryProperty().Form;
    }

    KRYS_NODISCARD bool HasProperty(MathMLOperatorDictionary::Flag flag) noexcept;
    KRYS_NODISCARD Length DefaultLeadingSpace() noexcept;
    KRYS_NODISCARD Length DefaultTrailingSpace() noexcept;
    KRYS_NODISCARD const Length &LeadingSpace() noexcept;
    KRYS_NODISCARD const Length &TrailingSpace() noexcept;
    KRYS_NODISCARD const Length &MinSize() noexcept;
    KRYS_NODISCARD const Length &MaxSize() noexcept;

  private:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept final;
    void OnChildrenChanged() noexcept final;
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;

    KRYS_NODISCARD MathMLOperatorDictionary::Property ComputeDictionaryProperty() noexcept;
    KRYS_NODISCARD const MathMLOperatorDictionary::Property &DictionaryProperty() noexcept;
    void ComputeOperatorFlag(MathMLOperatorDictionary::Flag flag) noexcept;
    void UpdateRendererFromElement() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::mathml::MathMLOperatorElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::mathml::MathMLElement &element) noexcept
  {
    return element.Interface() == krys::boo::mathml::MathMLElementInterface::Operator;
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::mathml::MathMLElement>(node);
    return element && IsType(*element);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
