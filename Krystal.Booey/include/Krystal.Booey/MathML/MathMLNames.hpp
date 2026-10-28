#pragma once

#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::mathml
{
  /// @brief The names of MathML's attributes, in the null namespace.
  /// @note Only the attributes a ported element uses are here.
  struct MathMLNames
  {
    KRYS_NODISCARD static const dom::QualifiedName &AccentAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &AccentunderAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &ColumnspanAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &DepthAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &DirAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &DisplaystyleAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &FenceAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &FormAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &HeightAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &LargeopAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &LinethicknessAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &LspaceAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &MathbackgroundAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &MathcolorAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &MathsizeAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &MathvariantAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &MaxsizeAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &MinsizeAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &MovablelimitsAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &NotationAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &RowspanAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &RspaceAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &ScriptlevelAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &SeparatorAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &StretchyAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &SymmetricAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &VoffsetAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &WidthAttr() noexcept;
  };
}
