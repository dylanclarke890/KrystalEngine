#pragma once

#include "Krystal.Booey/Style/Values/Masking/MaskBorderOutset.hpp"
#include "Krystal.Booey/Style/Values/Masking/MaskBorderRepeat.hpp"
#include "Krystal.Booey/Style/Values/Masking/MaskBorderSlice.hpp"
#include "Krystal.Booey/Style/Values/Masking/MaskBorderSource.hpp"
#include "Krystal.Booey/Style/Values/Masking/MaskBorderWidth.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // <'mask-border'> = <'mask-border-source'> || <'mask-border-slice'> [ / <'mask-border-width'>? [ /
  // <'mask-border-outset'> ]? ]? || <'mask-border-repeat'> || <'mask-border-mode'>
  // FIXME: Add support for `mask-border-mode`.
  // https://drafts.fxtf.org/css-masking-1/#propdef-mask-border
  struct MaskBorder
  {
    MaskBorderSource Source;
    MaskBorderSlice Slice;
    MaskBorderWidth Width;
    MaskBorderOutset Outset;
    MaskBorderRepeat Repeat;

    MaskBorder() noexcept;
    MaskBorder(MaskBorderSource &&, MaskBorderSlice &&, MaskBorderWidth &&, MaskBorderOutset &&,
               MaskBorderRepeat &&) noexcept;

    KRYS_NODISCARD bool operator==(const MaskBorder &) const noexcept = default;
  };

  // MARK: - Conversion

  enum class MaskBorderSliceOverride : bool
  {
    None,
    AlwaysFill
  };

  template <>
  struct CSSValueConversion<MaskBorder>
  {
    KRYS_NODISCARD MaskBorder operator()(BuilderState &, const css::Value &,
                                         MaskBorderSliceOverride = MaskBorderSliceOverride::None) noexcept;
  };

  template <>
  struct CSSValueCreation<MaskBorder>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const MaskBorder &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<MaskBorder>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const MaskBorder &) noexcept;
  };
}
