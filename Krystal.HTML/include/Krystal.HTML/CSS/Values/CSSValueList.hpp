#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  class CSSValueList : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSValueList> Create(CSSValueListBuilder &&values) noexcept;
    KRYS_NODISCARD static Ref<CSSValueList> CreateCommaSeparated(CSSValueListBuilder &&values) noexcept;
    KRYS_NODISCARD static Ref<CSSValueList> CreateSpaceSeparated(CSSValueListBuilder &&values) noexcept;
    KRYS_NODISCARD static Ref<CSSValueList> CreateSlashSeparated(CSSValueListBuilder &&values) noexcept;
  };
}