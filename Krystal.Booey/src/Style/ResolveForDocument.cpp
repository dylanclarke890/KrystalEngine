#include "Krystal.Booey/Style/ResolveForDocument.hpp"
#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/Adjuster.hpp"
#include "Krystal.Booey/Style/FontSizeFunctions.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/SVG/SVGDocument.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::style
{
  RenderStyle ResolveForDocument(const dom::Document &document) noexcept
  {
    auto documentStyle = RenderStyle::Create();

    documentStyle.SetDisplay(DisplayType::Block);
    // FIXME: Visually ordered documents (legacy encodings) set `Order::Visual`; there are no encodings.
    documentStyle.SetRTLOrdering(boo::Order::Logical);
    // FIXME: The frame's page zoom and scale factors (`setZoom`, `setPageScaleTransform`) come with frames.

    // This overrides any -webkit-user-modify inherited from the parent iframe.
    // FIXME: Documents in design mode are `read-write`; there's no design mode.
    documentStyle.SetUserModify(UserModify::ReadOnly);

    Adjuster::AdjustEventListenerRegionTypesForRootStyle(documentStyle, document);

    // FIXME: Paged frame views set the column styles from their pagination mode; there are no frame views.

    auto fontDescription = [&]()
    {
      auto &settings = DocumentSettingsValues(document);

      FontCascadeDescription fontDescription;
      // FIXME: The document's content language (`Content-Language`, `<meta http-equiv>`) isn't tracked.
      fontDescription.SetOneFamily(css::WebKitFontFamilyNames::StandardFamily());
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

    // FIXME: Upstream gives the font cascade the document's font selector here; there's no font selector.
    documentStyle.SetFontCascade(krys::move(fontCascade));

    documentStyle.SetEvaluationTimeZoomEnabled(DocumentSettingsValues(document).EvaluationTimeZoomEnabled);

    // FIXME: The device scale factor comes with pages; it's 1.
    documentStyle.SetDeviceScaleFactor(1);

    return documentStyle;
  }
}
