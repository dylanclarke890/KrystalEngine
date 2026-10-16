#pragma once

#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief A table.
  // FIXME: The table structure (sections, columns, captions), collapsed borders and table layout come with
  // the table builder and layout.
  class RenderTable : public RenderBlock
  {
    KRYS_NON_COPY_MOVABLE(RenderTable);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTable);

  public:
    RenderTable(Type type, dom::Element &element, RenderStyle &&style) noexcept;
    RenderTable(Type type, dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderTable() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderTable";
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTable)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTable();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
