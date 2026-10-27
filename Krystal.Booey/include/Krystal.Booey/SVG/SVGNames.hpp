#pragma once

#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::svg
{
  /// @brief The names of SVG's attributes, in the null namespace.
  /// @note Only the attributes a ported element or mixin uses are here.
  struct SVGNames
  {
    /// @brief The `class` attribute, shared with HTML.
    KRYS_NODISCARD static const dom::QualifiedName &ClassAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &CxAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &CyAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &DAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &FrAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &FxAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &FyAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &GradientTransformAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &GradientUnitsAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &HeightAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &HrefAttr() noexcept;
    /// @brief The `id` attribute, shared with HTML.
    KRYS_NODISCARD static const dom::QualifiedName &IdAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &MediaAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &OffsetAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &PathLengthAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &PatternContentUnitsAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &PatternTransformAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &PatternUnitsAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &PointsAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &PreserveAspectRatioAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &RAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &RelAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &RequiredExtensionsAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &RxAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &RyAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &SpreadMethodAttr() noexcept;
    /// @brief The `style` attribute, shared with HTML.
    KRYS_NODISCARD static const dom::QualifiedName &StyleAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &SystemLanguageAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &TargetAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &TitleAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &TransformAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &TypeAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &ViewBoxAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &WidthAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &X1Attr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &X2Attr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &XAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &Y1Attr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &Y2Attr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &YAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &ZoomAndPanAttr() noexcept;
  };

  /// @brief The names of the XLink attributes SVG uses, in the XLink namespace with the `xlink` prefix.
  struct XLinkNames
  {
    KRYS_NODISCARD static const dom::QualifiedName &HrefAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &ShowAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &TitleAttr() noexcept;
  };
}
