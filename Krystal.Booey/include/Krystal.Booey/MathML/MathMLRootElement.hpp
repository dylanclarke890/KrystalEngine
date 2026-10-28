#pragma once

#include "Krystal.Booey/MathML/MathMLRowElement.hpp"

namespace krys::boo::mathml
{
  enum class RootType : uint8
  {
    SquareRoot,
    RootWithIndex,
  };

  /// @brief The `<msqrt>` and `<mroot>` elements: a square root, and a root with an index.
  /// @see https://w3c.github.io/mathml-core/#radicals-msqrt-mroot
  class MathMLRootElement final : public MathMLRowElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(MathMLRootElement);

  private:
    const boo::mathml::RootType _rootType;

  public:
    MathMLRootElement(dom::Document &document, boo::mathml::RootType rootType) noexcept
        : MathMLRowElement(document, MathMLElementInterface::Root), _rootType(rootType)
    {
    }

    KRYS_NODISCARD boo::mathml::RootType GetRootType() const noexcept
    {
      return _rootType;
    }

    // FIXME: The element is made a RenderMathMLRoot, whose radical operator is reset and layout invalidated
    // when the element's children change, once there are MathML renderers.
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::mathml::MathMLRootElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::mathml::MathMLElement &element) noexcept
  {
    return element.Interface() == krys::boo::mathml::MathMLElementInterface::Root;
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::mathml::MathMLElement>(node);
    return element && IsType(*element);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
