/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 2004-2005 Allan Sandfeld Jensen (kde@carewolf.com)
 * Copyright (C) 2006, 2007 Nicholas Shanks (webkit@nickshanks.com)
 * Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alexey Proskuryakov <ap@webkit.org>
 * Copyright (C) 2007, 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/Style/ResolveForDocument.hpp"
#include "Krystal.Booey/CSS/Font/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/Font/GenericFontFamilyNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/Adjuster.hpp"
#include "Krystal.Booey/Style/Builder/FontSizeFunctions.hpp"
#include "Krystal.Booey/SVG/SVGDocument.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::style
{
  RenderStyle ResolveForDocument(const dom::Document &document) noexcept
  {
    auto documentStyle = RenderStyle::Create();

    documentStyle.SetDisplay(DisplayType::Block);
    if (auto *frame = document.Frame())
    {
      documentStyle.SetZoom(!document.Printing() ? frame->PageZoomFactor() : 1);
      documentStyle.SetPageScaleTransform(frame->FrameScaleFactor());
    }

    // This overrides any -webkit-user-modify inherited from the parent iframe.
    documentStyle.SetUserModify(document.InDesignMode() ? UserModify::ReadWrite : UserModify::ReadOnly);

    Adjuster::AdjustEventListenerRegionTypesForRootStyle(documentStyle, document);

    auto fontDescription = [&]()
    {
      auto &settings = document.GetSettings().Values;

      FontCascadeDescription fontDescription;
      fontDescription.SetSpecifiedLocale(StringAtom(document.ContentLanguage()));
      fontDescription.SetOneFamily(css::GenericFontFamilyNames::StandardFamily());
      // FIXME: The settings' shouldAllowUserInstalledFonts comes with Settings (its default, Yes, is the
      // description's).
      // FIXME: We need evaluationTimeZoomEnabled to be accessible from FontDescription, not only from
      // RenderStyle. Would it be weird to move it to FontDescription (which is already accessible from
      // RenderStyle)?
      fontDescription.SetEvaluationTimeZoomEnabled(settings.EvaluationTimeZoomEnabled);

      fontDescription.SetKeywordSizeFromIdentifier(css::ValueId::Medium);
      int32 size =
        static_cast<int32>(FontSizeForKeyword(static_cast<uint32>(css::ValueId::Medium), false, document));
      fontDescription.SetSpecifiedSize(static_cast<float>(size));
      bool useSVGZoomRules = Is<svg::SVGDocument>(document);
      auto computedFontSize =
        ComputedFontSizeFromSpecifiedSize(static_cast<float>(size), fontDescription.IsAbsoluteSize(),
                                          useSVGZoomRules, &documentStyle, document);
      fontDescription.SetComputedSize(computedFontSize.Size, computedFontSize.UsedZoomFactor);

      auto [fontOrientation, glyphOrientation] = documentStyle.FontAndGlyphOrientation();
      fontDescription.SetOrientation(fontOrientation);
      fontDescription.SetNonCJKGlyphOrientation(glyphOrientation);
      return fontDescription;
    }();

    auto fontCascade = FontCascade {krys::move(fontDescription), documentStyle.GetFontCascade()};

    // We don't just call setFontDescription() because we need to provide the fontSelector to the FontCascade.
    fontCascade.Update(ShareRefPtr<FontSelector>(&document.GetFontSelector()));
    documentStyle.SetFontCascade(krys::move(fontCascade));

    documentStyle.SetEvaluationTimeZoomEnabled(document.GetSettings().Values.EvaluationTimeZoomEnabled);

    documentStyle.SetDeviceScaleFactor(document.DeviceScaleFactor());

    return documentStyle;
  }
}
