#pragma once

#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class RenderTableRow;

  /// @brief A table row group.
  // FIXME: The grid of rows and cells, collapsed borders and section layout come with the table builder and
  // layout.
  class RenderTableSection final : public RenderBox
  {
    KRYS_NON_COPY_MOVABLE(RenderTableSection);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTableSection);

  public:
    RenderTableSection(dom::Element &element, RenderStyle &&style) noexcept;
    RenderTableSection(dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderTableSection() noexcept override;

    KRYS_NODISCARD RenderTableRow *FirstRow() const noexcept;
    KRYS_NODISCARD RenderTableRow *LastRow() const noexcept;

    KRYS_NODISCARD bool CanHaveChildren() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return (IsAnonymous() || IsPseudoElement()) ? u8"RenderTableSection (anonymous)"
                                                  : u8"RenderTableSection";
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTableSection)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTableSection();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
