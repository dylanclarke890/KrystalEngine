#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Web/Dom/Types/QualifiedName.hpp"

namespace krys::web
{
  /// @brief The names of MathML's attributes, in the null namespace.
  /// @note Only the attributes a ported element uses are here.
  struct MathMLNames
  {
    KRYS_NODISCARD static const QualifiedName &AccentAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &AccentunderAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ColumnspanAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &DepthAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &DirAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &DisplaystyleAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FenceAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FormAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &HeightAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &LargeopAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &LinethicknessAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &LspaceAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MathbackgroundAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MathcolorAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MathsizeAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MathvariantAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MaxsizeAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MinsizeAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MovablelimitsAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &NotationAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &RowspanAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &RspaceAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ScriptlevelAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &SeparatorAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &StretchyAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &SymmetricAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &VoffsetAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &WidthAttr() noexcept;
  };
}
