#pragma once

#include "Krystal.Booey/Core/StyleAppearance.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/Values/Align/AlignContent.hpp"
#include "Krystal.Booey/Style/Values/Align/AlignItems.hpp"
#include "Krystal.Booey/Style/Values/Align/AlignSelf.hpp"
#include "Krystal.Booey/Style/Values/Align/JustifyContent.hpp"
#include "Krystal.Booey/Style/Values/Align/JustifyItems.hpp"
#include "Krystal.Booey/Style/Values/Align/JustifySelf.hpp"
#include "Krystal.Booey/Style/Values/Animations/Animations.hpp"
#include "Krystal.Booey/Style/Values/Borders/BoxShadow.hpp"
#include "Krystal.Booey/Style/Values/Color/Opacity.hpp"
#include "Krystal.Booey/Style/Values/Content/Content.hpp"
#include "Krystal.Booey/Style/Values/Display/Order.hpp"
#include "Krystal.Booey/Style/Values/Images/ObjectPosition.hpp"
#include "Krystal.Booey/Style/Values/Masking/MaskLayers.hpp"
#include "Krystal.Booey/Style/Values/Sizing/AspectRatio.hpp"
#include "Krystal.Booey/Style/Values/Transitions/Transitions.hpp"
#include "Krystal.Booey/Style/Values/UI/Resize.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/DataRef.hpp"
#include "Krystal.Core/Types/FixedVector.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <memory>

namespace krys::boo
{
  class StyleFilterData;
  class StyleFlexibleBoxData;
  class StyleMultiColData;
  class StyleTransformData;
  class StyleVisitedLinkColorData;
  class StyleMiscNonInheritedData : public RefCounted<StyleMiscNonInheritedData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleMiscNonInheritedData> Create() noexcept
    {
      return AdoptRef(*new StyleMiscNonInheritedData);
    }
    KRYS_NODISCARD Ref<StyleMiscNonInheritedData> Copy() const noexcept;
    ~StyleMiscNonInheritedData() noexcept;

    bool operator==(const StyleMiscNonInheritedData &) const noexcept;

    KRYS_NODISCARD bool HasFilters() const noexcept;

    // This is here to pack in with _refCount.
    style::Opacity Opacity;

    DataRef<StyleFlexibleBoxData> FlexibleBox;
    DataRef<StyleMultiColData> MultiCol;   // CSS3 multicol properties
    DataRef<StyleFilterData> Filter;       // Filter operations (url, sepia, blur, etc.)
    DataRef<StyleTransformData> Transform; // Transform properties (rotate, scale, skew, etc.)
    DataRef<StyleVisitedLinkColorData> VisitedLinkColor;

    style::MaskLayers Mask;
    style::Animations Animations;
    style::Transitions Transitions;
    style::Content Content;
    style::BoxShadows BoxShadow;
    style::AspectRatio AspectRatio;

    style::AlignContent AlignContent;
    style::AlignItems AlignItems;
    style::AlignSelf AlignSelf;
    style::JustifyContent JustifyContent;
    style::JustifyItems JustifyItems;
    style::JustifySelf JustifySelf;

    style::ObjectPosition ObjectPosition;
    style::Order Order;

    unsigned HasAttrContent : 1 {false};
    unsigned HasDisplayAffectedByAnimations : 1 {false};
    unsigned HasExplicitlySetColorScheme : 1 {false};
    unsigned HasExplicitlySetDirection : 1 {false};
    unsigned HasExplicitlySetWritingMode : 1 {false};
    unsigned TableLayout : 1;
    unsigned Appearance : AppearanceBitWidth;
    unsigned UsedAppearance : AppearanceBitWidth;
    unsigned TextOverflow : 1; // Whether or not lines that spill out should be truncated with "..."
    unsigned UserDrag : 2;
    unsigned ObjectFit : 3;
    unsigned Resize : 3;

  private:
    StyleMiscNonInheritedData() noexcept;
    StyleMiscNonInheritedData(const StyleMiscNonInheritedData &) noexcept;
  };
}
