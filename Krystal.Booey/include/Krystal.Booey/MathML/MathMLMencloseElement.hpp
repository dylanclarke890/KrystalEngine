#pragma once

#include "Krystal.Booey/MathML/MathMLRowElement.hpp"

namespace krys::boo::mathml
{
  /// @brief The `<menclose>` element: a row drawn with the notations its `notation` attribute lists.
  /// @see https://www.w3.org/TR/MathML3/chapter3.html#presm.menclose
  class MathMLMencloseElement final : public MathMLRowElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(MathMLMencloseElement);

  public:
    enum MencloseNotationFlag : uint16
    {
      LongDiv = 1 << 1,
      RoundedBox = 1 << 2,
      Circle = 1 << 3,
      Left = 1 << 4,
      Right = 1 << 5,
      Top = 1 << 6,
      Bottom = 1 << 7,
      UpDiagonalStrike = 1 << 8,
      DownDiagonalStrike = 1 << 9,
      VerticalStrike = 1 << 10,
      HorizontalStrike = 1 << 11,
      UpDiagonalArrow = 1 << 12, // FIXME: updiagonalarrow is not implemented. See http://wkb.ug/127466
      PhasorAngle = 1 << 13      // FIXME: phasorangle is not implemented. See http://wkb.ug/127466
      // We do not implement the Radical notation. Authors should instead use the <msqrt> element.
    };

  private:
    Maybe<uint16> _notationFlags;

  public:
    explicit MathMLMencloseElement(dom::Document &document) noexcept;

    KRYS_NODISCARD bool HasNotation(MencloseNotationFlag notationFlag) noexcept;

  private:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept final;

    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;

    void ParseNotationAttribute() noexcept;

    void ClearNotations() noexcept
    {
      _notationFlags = 0;
    }

    void AddNotation(MencloseNotationFlag notationFlag) noexcept
    {
      *_notationFlags = static_cast<uint16>(*_notationFlags | notationFlag);
    }

    void AddNotationFlags(dom::DOMStringView notation) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::mathml::MathMLMencloseElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::mathml::MathMLElement &element) noexcept
  {
    return element.Interface() == krys::boo::mathml::MathMLElementInterface::Menclose;
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::mathml::MathMLElement>(node);
    return element && IsType(*element);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
