#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"
#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"
#include "Krystal.Booey/SVG/SVGLengthValue.hpp"

namespace krys::boo
{
  class RenderObject;
}

namespace krys::boo::dom
{
  struct DOMPointInit;
}

namespace krys::boo::svg
{
  class SVGPoint;
  class SVGRect;

  enum SVGLengthAdjustType : uint8
  {
    SVGLengthAdjustUnknown,
    SVGLengthAdjustSpacing,
    SVGLengthAdjustSpacingAndGlyphs
  };

  template <>
  struct SVGPropertyTraits<SVGLengthAdjustType>
  {
    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return SVGLengthAdjustSpacingAndGlyphs;
    }

    KRYS_NODISCARD static dom::DOMString ToString(SVGLengthAdjustType type) noexcept
    {
      switch (type)
      {
        case SVGLengthAdjustUnknown:
        {
          return {};
        }
        case SVGLengthAdjustSpacing:
        {
          return u8"spacing";
        }
        case SVGLengthAdjustSpacingAndGlyphs:
        {
          return u8"spacingAndGlyphs";
        }
      }

      krys_assert_msg(false, "Not reached");
      return {};
    }

    KRYS_NODISCARD static SVGLengthAdjustType FromString(SVGElement &, dom::DOMStringView value) noexcept
    {
      if (value == u8"spacingAndGlyphs")
      {
        return SVGLengthAdjustSpacingAndGlyphs;
      }
      if (value == u8"spacing")
      {
        return SVGLengthAdjustSpacing;
      }
      return SVGLengthAdjustUnknown;
    }
  };

  /// @brief The base of the text content elements (`<text>`, `<tspan>`, `<textPath>`): their text length and
  /// its adjustment, and the queries on their laid out characters.
  /// @note Character numbers are UTF-8 code units, as the DOM's offsets are.
  /// @see https://svgwg.org/svg2-draft/text.html#InterfaceSVGTextContentElement
  class SVGTextContentElement : public SVGGraphicsElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGTextContentElement);

  private:
    Ref<SVGAnimatedLength> _textLength;
    Ref<SVGAnimatedEnumeration> _lengthAdjust;
    SVGLengthValue _specifiedTextLength {SVGLengthMode::Other};

  public:
    enum : uint8
    {
      LENGTHADJUST_UNKNOWN = SVGLengthAdjustUnknown,
      LENGTHADJUST_SPACING = SVGLengthAdjustSpacing,
      LENGTHADJUST_SPACINGANDGLYPHS = SVGLengthAdjustSpacingAndGlyphs
    };

    /// @see https://svgwg.org/svg2-draft/text.html#__svg__SVGTextContentElement__getNumberOfChars
    KRYS_NODISCARD uint32 GetNumberOfChars() noexcept;
    /// @see https://svgwg.org/svg2-draft/text.html#__svg__SVGTextContentElement__getComputedTextLength
    KRYS_NODISCARD float GetComputedTextLength() noexcept;
    /// @see https://svgwg.org/svg2-draft/text.html#__svg__SVGTextContentElement__getSubStringLength
    KRYS_NODISCARD dom::ExceptionOr<float> GetSubStringLength(uint32 charnum, uint32 nchars) noexcept;
    /// @see https://svgwg.org/svg2-draft/text.html#__svg__SVGTextContentElement__getStartPositionOfChar
    KRYS_NODISCARD dom::ExceptionOr<Ref<SVGPoint>> GetStartPositionOfChar(uint32 charnum) noexcept;
    /// @see https://svgwg.org/svg2-draft/text.html#__svg__SVGTextContentElement__getEndPositionOfChar
    KRYS_NODISCARD dom::ExceptionOr<Ref<SVGPoint>> GetEndPositionOfChar(uint32 charnum) noexcept;
    /// @see https://svgwg.org/svg2-draft/text.html#__svg__SVGTextContentElement__getExtentOfChar
    KRYS_NODISCARD dom::ExceptionOr<Ref<SVGRect>> GetExtentOfChar(uint32 charnum) noexcept;
    /// @see https://svgwg.org/svg2-draft/text.html#__svg__SVGTextContentElement__getRotationOfChar
    KRYS_NODISCARD dom::ExceptionOr<float> GetRotationOfChar(uint32 charnum) noexcept;
    /// @see https://svgwg.org/svg2-draft/text.html#__svg__SVGTextContentElement__getCharNumAtPosition
    KRYS_NODISCARD int32 GetCharNumAtPosition(dom::DOMPointInit &&) noexcept;
    /// @see https://svgwg.org/svg2-draft/text.html#__svg__SVGTextContentElement__selectSubString
    dom::ExceptionOr<void> SelectSubString(uint32 charnum, uint32 nchars) noexcept;

    KRYS_NODISCARD static const SVGTextContentElement *ElementFromRenderer(const RenderObject *) noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGTextContentElement, SVGGraphicsElement>;

    KRYS_NODISCARD const SVGLengthValue &SpecifiedTextLength() const noexcept
    {
      return _specifiedTextLength;
    }
    KRYS_NODISCARD const SVGLengthValue &TextLength() const noexcept
    {
      return _textLength->CurrentValue();
    }
    KRYS_NODISCARD SVGLengthAdjustType LengthAdjust() const noexcept
    {
      return _lengthAdjust->CurrentValue<SVGLengthAdjustType>();
    }

    KRYS_NODISCARD SVGAnimatedLength &TextLengthAnimated() noexcept;
    KRYS_NODISCARD SVGAnimatedEnumeration &LengthAdjustAnimated() noexcept
    {
      return *_lengthAdjust;
    }

  protected:
    SVGTextContentElement(dom::Document &, UniquePtr<SVGPropertyRegistry> &&, SVGElementInterface,
                          SVGElementFlags = SVGElementFlags::None) noexcept;

    KRYS_NODISCARD bool IsValid() const noexcept override
    {
      return SVGTests::IsValid();
    }

    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override;
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept override;

    KRYS_NODISCARD bool SelfHasRelativeLengths() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGTextContentElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGTextContentElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
