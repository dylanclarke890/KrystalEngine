/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Rendering/Style/StyleMiscNonInheritedData.hpp"

#include "Krystal.Booey/Rendering/Style/StyleFilterData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleFlexibleBoxData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleMultiColData.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

#include "Krystal.Booey/Rendering/Style/StyleTransformData.hpp"

namespace krys::boo
{

  StyleMiscNonInheritedData::StyleMiscNonInheritedData() noexcept
      : Opacity(style::ComputedStyle::InitialOpacity()), FlexibleBox(StyleFlexibleBoxData::Create()),
        MultiCol(StyleMultiColData::Create()), Filter(StyleFilterData::Create()),
        Transform(StyleTransformData::Create()), Mask(css::keywords::None {}),
        Animations(css::keywords::None {}), Transitions(css::keywords::All {}),
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
        MultiCol(o.MultiCol), Filter(o.Filter), Transform(o.Transform), Mask(o.Mask),
        Animations(o.Animations), Transitions(o.Transitions), Content(o.Content), BoxShadow(o.BoxShadow),
        AspectRatio(o.AspectRatio), AlignContent(o.AlignContent), AlignItems(o.AlignItems),
        AlignSelf(o.AlignSelf), JustifyContent(o.JustifyContent), JustifyItems(o.JustifyItems),
        JustifySelf(o.JustifySelf), ObjectPosition(o.ObjectPosition), Order(o.Order),
        HasAttrContent(o.HasAttrContent), HasDisplayAffectedByAnimations(o.HasDisplayAffectedByAnimations),
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
           && Filter == o.Filter && Transform == o.Transform && Mask == o.Mask && Animations == o.Animations
           && Transitions == o.Transitions && Content == o.Content && BoxShadow == o.BoxShadow
           && AspectRatio == o.AspectRatio && AlignContent == o.AlignContent && AlignItems == o.AlignItems
           && AlignSelf == o.AlignSelf && JustifyContent == o.JustifyContent && JustifyItems == o.JustifyItems
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
