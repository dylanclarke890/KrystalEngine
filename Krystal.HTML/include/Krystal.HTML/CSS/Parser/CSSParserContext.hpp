#pragma once

#include "Krystal.HTML/CSS/Parser/Enums/CSSParserMode.hpp"
#include "Krystal.HTML/CSS/Properties/Enums/CSSPropertyId.hpp"

namespace Krys::HTML
{
  struct CSSParserContext
  {
    CSSParserMode Mode;
    CSSPropertySettings PropertySettings;
    bool IsHTMLDocument : 1 {false};
    bool useSystemAppearance : 1 {false};
    bool shouldIgnoreImportRules : 1 {false};

    // Settings, excluding those affecting properties.
    bool counterStyleAtRuleImageSymbolsEnabled : 1 {false};
    bool springTimingFunctionEnabled : 1 {false};
    bool cssTransformStyleSeparatedEnabled : 1 {false};
    bool gridLanesEnabled : 1 {false};
    bool cssAppearanceBaseEnabled : 1 {false};
    bool cssPaintingAPIEnabled : 1 {false};
    bool cssShapeFunctionEnabled : 1 {false};
    bool cssTextDecorationLineErrorValues : 1 {false};
    bool cssBackgroundClipBorderAreaEnabled : 1 {false};
    bool cssWordBreakAutoPhraseEnabled : 1 {false};
    bool popoverAttributeEnabled : 1 {false};
    bool sidewaysWritingModesEnabled : 1 {false};
    bool cssTextWrapPrettyEnabled : 1 {true};
    bool thumbAndTrackPseudoElementsEnabled : 1 {false};
    bool imageControlsEnabled : 1 {false};
    bool colorLayersEnabled : 1 {false};
    bool contrastColorEnabled : 1 {false};
    bool targetTextPseudoElementEnabled : 1 {false};
    bool cssProgressFunctionEnabled : 1 {false};
    bool cssRandomFunctionEnabled : 1 {false};
    bool cssTreeCountingFunctionsEnabled : 1 {false};
    bool cssURLModifiersEnabled : 1 {false};
    bool cssURLIntegrityModifierEnabled : 1 {false};
    bool cssAxisRelativePositionKeywordsEnabled : 1 {false};
    bool cssDynamicRangeLimitMixEnabled : 1 {false};
    bool cssConstrainedDynamicRangeLimitEnabled : 1 {false};
    bool cssTextTransformMathAutoEnabled : 1 {false};
    bool cssInternalAutoBaseParsingEnabled : 1 {false};
    bool webkitMediaTextTrackDisplayQuirkEnabled : 1 {false};
    bool cssMathDepthEnabled : 1 {false};
  };
}