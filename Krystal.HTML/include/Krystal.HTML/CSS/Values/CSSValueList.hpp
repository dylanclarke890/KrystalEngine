#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  class CSSValueContainingVector : public CSSValue
  {
  };

  class CSSValueList final : public CSSValueContainingVector
  {
  public:
    KRYS_NODISCARD static Ref<CSSValueList> Create(char32 separator, CSSValueListBuilder &&values) noexcept;

    KRYS_NODISCARD static Ref<CSSValueList> CreateCommaSeparated(CSSValueListBuilder &&values) noexcept;

    KRYS_NODISCARD static Ref<CSSValueList> CreateSpaceSeparated(Ref<CSSValue> value) noexcept;

    KRYS_NODISCARD static Ref<CSSValueList> CreateSpaceSeparated(Ref<CSSValue> first,
                                                                 Ref<CSSValue> second) noexcept;

    KRYS_NODISCARD static Ref<CSSValueList> CreateSpaceSeparated(Ref<CSSValue> first,
                                                                 Ref<CSSValue> second,
                                                                 Ref<CSSValue> third) noexcept;

    KRYS_NODISCARD static Ref<CSSValueList> CreateSpaceSeparated(CSSValueListBuilder &&values) noexcept;

    KRYS_NODISCARD static Ref<CSSValueList> CreateSlashSeparated(CSSValueListBuilder &&values) noexcept;
  };
}