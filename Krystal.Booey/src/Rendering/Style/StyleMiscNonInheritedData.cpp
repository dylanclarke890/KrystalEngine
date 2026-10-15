#include "Krystal.Booey/Rendering/Style/StyleMiscNonInheritedData.hpp"

#include "Krystal.Booey/Rendering/Style/StyleFilterData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleFlexibleBoxData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleMultiColData.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

#include "Krystal.Booey/Rendering/Style/StyleTransformData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleVisitedLinkColorData.hpp"

namespace krys::boo
{

  StyleMiscNonInheritedData::StyleMiscNonInheritedData() noexcept
      : Opacity(style::ComputedStyle::InitialOpacity()), FlexibleBox(StyleFlexibleBoxData::Create()),
        MultiCol(StyleMultiColData::Create()), Filter(StyleFilterData::Create()),
        Transform(StyleTransformData::Create()), VisitedLinkColor(StyleVisitedLinkColorData::Create()),
        Mask(css::keywords::None {}), Animations(css::keywords::None {}), Transitions(css::keywords::All {}),
        Content(style::ComputedStyle::InitialContent()), BoxShadow(style::ComputedStyle::InitialBoxShadow()),
        AspectRatio(style::ComputedStyle::InitialAspectRatio()),
        AlignContent(style::ComputedStyle::InitialAlignContent()),
        AlignItems(style::ComputedStyle::InitialAlignItems()),
        AlignSelf(style::ComputedStyle::InitialAlignSelf()),
        JustifyContent(style::ComputedStyle::InitialJustifyContent()),
        JustifyItems(style::ComputedStyle::InitialJustifyItems()),
        JustifySelf(style::ComputedStyle::InitialJustifySelf()),
        ObjectPosition(style::ComputedStyle::InitialObjectPosition()),
        Order(style::ComputedStyle::InitialOrder()),
        TableLayout(static_cast<unsigned>(style::ComputedStyle::InitialTableLayout())),
        Appearance(static_cast<unsigned>(style::ComputedStyle::InitialAppearance())),
        UsedAppearance(static_cast<unsigned>(style::ComputedStyle::InitialAppearance())),
        TextOverflow(static_cast<unsigned>(style::ComputedStyle::InitialTextOverflow())),
        UserDrag(static_cast<unsigned>(style::ComputedStyle::InitialUserDrag())),
        ObjectFit(static_cast<unsigned>(style::ComputedStyle::InitialObjectFit())),
        Resize(static_cast<unsigned>(style::ComputedStyle::InitialResize()))
  {
  }

  StyleMiscNonInheritedData::StyleMiscNonInheritedData(const StyleMiscNonInheritedData &o) noexcept
      : RefCounted<StyleMiscNonInheritedData>(), Opacity(o.Opacity), FlexibleBox(o.FlexibleBox),
        MultiCol(o.MultiCol), Filter(o.Filter), Transform(o.Transform), VisitedLinkColor(o.VisitedLinkColor),
        Mask(o.Mask), Animations(o.Animations), Transitions(o.Transitions), Content(o.Content),
        BoxShadow(o.BoxShadow), AspectRatio(o.AspectRatio), AlignContent(o.AlignContent),
        AlignItems(o.AlignItems), AlignSelf(o.AlignSelf), JustifyContent(o.JustifyContent),
        JustifyItems(o.JustifyItems), JustifySelf(o.JustifySelf), ObjectPosition(o.ObjectPosition),
        Order(o.Order), HasAttrContent(o.HasAttrContent),
        HasDisplayAffectedByAnimations(o.HasDisplayAffectedByAnimations),
        HasExplicitlySetColorScheme(o.HasExplicitlySetColorScheme),
        HasExplicitlySetDirection(o.HasExplicitlySetDirection),
        HasExplicitlySetWritingMode(o.HasExplicitlySetWritingMode), TableLayout(o.TableLayout),
        Appearance(o.Appearance), UsedAppearance(o.UsedAppearance), TextOverflow(o.TextOverflow),
        UserDrag(o.UserDrag), ObjectFit(o.ObjectFit), Resize(o.Resize)
  {
  }

  StyleMiscNonInheritedData::~StyleMiscNonInheritedData() noexcept = default;

  Ref<StyleMiscNonInheritedData> StyleMiscNonInheritedData::Copy() const noexcept
  {
    return AdoptRef(*new StyleMiscNonInheritedData(*this));
  }

  bool StyleMiscNonInheritedData::operator==(const StyleMiscNonInheritedData &o) const noexcept
  {
    return Opacity == o.Opacity && FlexibleBox == o.FlexibleBox && MultiCol == o.MultiCol
           && Filter == o.Filter && Transform == o.Transform && VisitedLinkColor == o.VisitedLinkColor
           && Mask == o.Mask && Animations == o.Animations && Transitions == o.Transitions
           && Content == o.Content && BoxShadow == o.BoxShadow && AspectRatio == o.AspectRatio
           && AlignContent == o.AlignContent && AlignItems == o.AlignItems && AlignSelf == o.AlignSelf
           && JustifyContent == o.JustifyContent && JustifyItems == o.JustifyItems
           && JustifySelf == o.JustifySelf && ObjectPosition == o.ObjectPosition && Order == o.Order
           && HasAttrContent == o.HasAttrContent
           && HasDisplayAffectedByAnimations == o.HasDisplayAffectedByAnimations
           && HasExplicitlySetColorScheme == o.HasExplicitlySetColorScheme
           && HasExplicitlySetDirection == o.HasExplicitlySetDirection
           && HasExplicitlySetWritingMode == o.HasExplicitlySetWritingMode && TableLayout == o.TableLayout
           && Appearance == o.Appearance && UsedAppearance == o.UsedAppearance
           && TextOverflow == o.TextOverflow && UserDrag == o.UserDrag && ObjectFit == o.ObjectFit
           && Resize == o.Resize;
  }

  bool StyleMiscNonInheritedData::HasFilters() const noexcept
  {
    return !Filter->Filter.IsNone();
  }
}
