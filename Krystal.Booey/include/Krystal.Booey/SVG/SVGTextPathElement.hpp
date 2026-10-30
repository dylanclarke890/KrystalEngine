#pragma once

#include "Krystal.Booey/SVG/SVGTextContentElement.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"

namespace krys::boo::svg
{
  enum SVGTextPathMethodType : uint8
  {
    SVGTextPathMethodUnknown = 0,
    SVGTextPathMethodAlign,
    SVGTextPathMethodStretch
  };

  enum SVGTextPathSpacingType : uint8
  {
    SVGTextPathSpacingUnknown = 0,
    SVGTextPathSpacingAuto,
    SVGTextPathSpacingExact
  };

  template <>
  struct SVGPropertyTraits<SVGTextPathMethodType>
  {
    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return SVGTextPathMethodStretch;
    }

    KRYS_NODISCARD static dom::DOMString ToString(SVGTextPathMethodType type) noexcept
    {
      switch (type)
      {
        case SVGTextPathMethodUnknown:
        {
          return {};
        }
        case SVGTextPathMethodAlign:
        {
          return u8"align";
        }
        case SVGTextPathMethodStretch:
        {
          return u8"stretch";
        }
      }

      krys_assert_msg(false, "Not reached");
      return {};
    }

    KRYS_NODISCARD static SVGTextPathMethodType FromString(SVGElement &, dom::DOMStringView value) noexcept
    {
      if (value == u8"align")
      {
        return SVGTextPathMethodAlign;
      }
      if (value == u8"stretch")
      {
        return SVGTextPathMethodStretch;
      }
      return SVGTextPathMethodUnknown;
    }
  };

  template <>
  struct SVGPropertyTraits<SVGTextPathSpacingType>
  {
    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return SVGTextPathSpacingExact;
    }

    KRYS_NODISCARD static dom::DOMString ToString(SVGTextPathSpacingType type) noexcept
    {
      switch (type)
      {
        case SVGTextPathSpacingUnknown:
        {
          return {};
        }
        case SVGTextPathSpacingAuto:
        {
          return u8"auto";
        }
        case SVGTextPathSpacingExact:
        {
          return u8"exact";
        }
      }

      krys_assert_msg(false, "Not reached");
      return {};
    }

    KRYS_NODISCARD static SVGTextPathSpacingType FromString(SVGElement &, dom::DOMStringView value) noexcept
    {
      if (value == u8"auto")
      {
        return SVGTextPathSpacingAuto;
      }
      if (value == u8"exact")
      {
        return SVGTextPathSpacingExact;
      }
      return SVGTextPathSpacingUnknown;
    }
  };

  /// @brief The `<textPath>` element: text laid out along a referenced shape's path.
  /// @see https://svgwg.org/svg2-draft/text.html#InterfaceSVGTextPathElement
  class SVGTextPathElement final : public SVGTextContentElement, public SVGURIReference
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGTextPathElement);

  private:
    Ref<SVGAnimatedLength> _startOffset;
    Ref<SVGAnimatedEnumeration> _method;
    Ref<SVGAnimatedEnumeration> _spacing;

  public:
    // Forward declare enumerations in the W3C naming scheme, for IDL generation.
    enum : uint8
    {
      TEXTPATH_METHODTYPE_UNKNOWN = SVGTextPathMethodUnknown,
      TEXTPATH_METHODTYPE_ALIGN = SVGTextPathMethodAlign,
      TEXTPATH_METHODTYPE_STRETCH = SVGTextPathMethodStretch,
      TEXTPATH_SPACINGTYPE_UNKNOWN = SVGTextPathSpacingUnknown,
      TEXTPATH_SPACINGTYPE_AUTO = SVGTextPathSpacingAuto,
      TEXTPATH_SPACINGTYPE_EXACT = SVGTextPathSpacingExact
    };

    explicit SVGTextPathElement(dom::Document &) noexcept;
    ~SVGTextPathElement() noexcept override;

    KRYS_NODISCARD const SVGLengthValue &StartOffset() const noexcept
    {
      return _startOffset->CurrentValue();
    }
    KRYS_NODISCARD SVGTextPathMethodType Method() const noexcept
    {
      return _method->CurrentValue<SVGTextPathMethodType>();
    }
    KRYS_NODISCARD SVGTextPathSpacingType Spacing() const noexcept
    {
      return _spacing->CurrentValue<SVGTextPathSpacingType>();
    }

    KRYS_NODISCARD SVGAnimatedLength &StartOffsetAnimated() noexcept
    {
      return *_startOffset;
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &MethodAnimated() noexcept
    {
      return *_method;
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &SpacingAnimated() noexcept
    {
      return *_spacing;
    }

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGTextPathElement, SVGTextContentElement, SVGURIReference>;

  private:
    void OnPostConnection() noexcept override;

    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept override;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
    KRYS_NODISCARD bool ChildShouldCreateRenderer(const dom::Node &) const noexcept final;
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final;

    void ClearResourceReferences() noexcept;
    void BuildPendingResource() noexcept override;

    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;

    KRYS_NODISCARD bool SelfHasRelativeLengths() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGTextPathElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGTextPathElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
