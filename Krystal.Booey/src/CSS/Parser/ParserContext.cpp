#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::css
{
  namespace
  {
    void ApplyUASheetBehaviorsToContext(ParserContext &context) noexcept
    {
      // FIXME: We should turn all of the features on from their WebCore Settings defaults.
      context.cssAppearanceBaseEnabled = true;
      context.cssTextTransformMathAutoEnabled = true;
      context.popoverAttributeEnabled = true;
      context.PropertySettings.cssInputSecurityEnabled = true;
      context.PropertySettings.supportHDRDisplayEnabled = true;
      context.PropertySettings.viewTransitionsEnabled = true;
      context.PropertySettings.cssFieldSizingEnabled = true;
      context.cssMathDepthEnabled = true;
      context.PropertySettings.cssMathDepthEnabled = true;
      context.thumbAndTrackPseudoElementsEnabled = true;
      context.cssInternalAutoBaseParsingEnabled = true;
    }
  }

  const ParserContext &StrictParserContext() noexcept
  {
    static MainThreadNeverDestroyed<ParserContext> strictContext(ParserMode::HTMLStandard);
    return strictContext;
  }

  ParserContext::ParserContext(ParserMode mode, const url::URL &baseURL) noexcept
      : BaseURL(baseURL), Mode(mode)
  {
    if (IsUASheetBehavior(mode))
    {
      ApplyUASheetBehaviorsToContext(*this);
    }

    StaticValuePool::Init();
  }

  void ParserContext::SetUASheetMode() noexcept
  {
    Mode = ParserMode::UASheet;
    ApplyUASheetBehaviorsToContext(*this);
  }

  size_t ParserContext::Hash() const noexcept
  {
    uint32 bits =
      static_cast<uint32>(IsHTMLDocument) << 0 | static_cast<uint32>(HasDocumentSecurityOrigin) << 1
      | static_cast<uint32>(static_cast<bool>(LoadedFromOpaqueSource)) << 2
      | static_cast<uint32>(useSystemAppearance) << 3 | static_cast<uint32>(springTimingFunctionEnabled) << 4
      | static_cast<uint32>(cssTransformStyleSeparatedEnabled) << 5
      | static_cast<uint32>(gridLanesEnabled) << 6 | static_cast<uint32>(cssAppearanceBaseEnabled) << 7
      | static_cast<uint32>(cssPaintingAPIEnabled) << 8 | static_cast<uint32>(cssShapeFunctionEnabled) << 9
      | static_cast<uint32>(cssBackgroundClipBorderAreaEnabled) << 10
      | static_cast<uint32>(cssWordBreakAutoPhraseEnabled) << 11
      | static_cast<uint32>(popoverAttributeEnabled) << 12
      | static_cast<uint32>(sidewaysWritingModesEnabled) << 13
      | static_cast<uint32>(cssTextWrapPrettyEnabled) << 14
      | static_cast<uint32>(thumbAndTrackPseudoElementsEnabled) << 15
      | static_cast<uint32>(imageControlsEnabled) << 16 | static_cast<uint32>(colorLayersEnabled) << 17
      | static_cast<uint32>(contrastColorEnabled) << 18
      | static_cast<uint32>(targetTextPseudoElementEnabled) << 19
      | static_cast<uint32>(cssProgressFunctionEnabled) << 20
      | static_cast<uint32>(cssRandomFunctionEnabled) << 21
      | static_cast<uint32>(cssTreeCountingFunctionsEnabled) << 22
      | static_cast<uint32>(cssURLModifiersEnabled) << 23
      | static_cast<uint32>(cssURLIntegrityModifierEnabled) << 24
      | static_cast<uint32>(cssAxisRelativePositionKeywordsEnabled) << 25
      | static_cast<uint32>(cssDynamicRangeLimitMixEnabled) << 26
      | static_cast<uint32>(cssConstrainedDynamicRangeLimitEnabled) << 27
      | static_cast<uint32>(cssTextDecorationLineErrorValues) << 28
      | static_cast<uint32>(cssTextTransformMathAutoEnabled) << 29
      | static_cast<uint32>(cssInternalAutoBaseParsingEnabled) << 30
      | static_cast<uint32>(cssTextTransformMathAutoEnabled) << 31;
    uint32 bits2 = static_cast<uint32>(cssMathDepthEnabled) << 0;
    return krys::Hash::Combine(BaseURL, Charset, PropertySettings, Mode, bits, bits2);
  }
}
