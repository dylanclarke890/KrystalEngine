# Port graph

Entries: `css/values/images/CSSGradient.cpp`

- Files to port: **169**
- Boundary `animation`: 3
- Boundary `cssjit`: 2
- Boundary `dom`: 5
- Boundary `external`: 6
- Boundary `loader`: 3
- Boundary `platform`: 16
- Boundary `rendering`: 5
- Boundary `wtf`: 61
- Unresolved: 3

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 9 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 8 | `css/CSSProperty.h` |  | css/StyleProperties.h |
| 8 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 8 | `platform/graphics/GradientColorStop.h` |  | platform/graphics/GradientColorStops.h |
| 8 | `platform/graphics/RenderingResourceIdentifier.h` |  | platform/graphics/RenderingResource.h |
| 7 | `css/StyleProperties.h` |  | css/ImmutableStyleProperties.h, css/MutableStyleProperties.h |
| 7 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 7 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 7 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 7 | `platform/graphics/GradientColorStops.h` |  | platform/graphics/Gradient.h |
| 7 | `platform/graphics/RenderingResource.h` |  | platform/graphics/Gradient.h |
| 7 | `platform/graphics/cg/GradientRendererCG.h` | yes | platform/graphics/Gradient.h |
| 7 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 6 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 6 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 6 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 6 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h |
| 6 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h |
| 6 | `css/query/GenericMediaQueryEvaluator.h` |  | style/ContainerQueryEvaluator.h |
| 6 | `platform/graphics/Gradient.h` |  | platform/graphics/FontPaletteValues.h |
| 6 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 5 | `css/CSSVariableData.h` |  | css/StyleRule.h |
| 5 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 5 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 5 | `css/StyleRuleType.h` |  | css/StyleRule.h, css/parser/CSSParserContext.h |
| 5 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 5 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 5 | `platform/graphics/FloatRect.h` |  | platform/graphics/controls/PlatformControl.h |
| 5 | `platform/graphics/FontFeatureValues.h` |  | css/StyleRule.h |
| 5 | `platform/graphics/FontPaletteValues.h` |  | css/StyleRule.h |
| 5 | `platform/graphics/PlatformColorSpace.h` |  | platform/graphics/DestinationColorSpace.h |
| 5 | `platform/graphics/controls/ControlStyle.h` |  | platform/graphics/controls/PlatformControl.h |
| 5 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 5 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 5 | `style/PropertyAllowlist.h` |  | style/MatchResult.h, style/RuleData.h |
| 4 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, style/HasSelectorFilter.h |
| 4 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.h, style/RuleSet.h |
| 4 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/StyleRule.h` |  | style/RuleData.h, style/RuleSet.h |
| 4 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 4 | `css/query/MediaQuery.h` |  | css/StyleRule.h, style/RuleSet.h |
| 4 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 4 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 4 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/values/primitives/CSSURLModifiers.h` |  | css/values/primitives/CSSURL.h |
| 4 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 4 | `platform/graphics/ColorConversion.h` |  | platform/graphics/Color.h |
| 4 | `platform/graphics/ColorSpace.h` |  | platform/graphics/Color.h |
| 4 | `platform/graphics/ColorUtilities.h` |  | platform/graphics/Color.h |
| 4 | `platform/graphics/DestinationColorSpace.h` |  | platform/graphics/Color.h, platform/graphics/cg/GradientRendererCG.h |
| 4 | `platform/graphics/DoubleSize.h` |  | platform/graphics/DoublePoint.h, platform/graphics/FloatSize.h |
| 4 | `platform/graphics/FontBaseline.h` |  | platform/graphics/FontMetrics.h |
| 4 | `platform/graphics/IntSize.h` |  | platform/graphics/DoubleSize.h, platform/graphics/IntPoint.h, platform/graphics/LayoutSize.h |
| 4 | `platform/graphics/controls/ControlFactory.h` |  | platform/graphics/controls/ControlPart.h |
| 4 | `platform/graphics/controls/PlatformControl.h` |  | platform/graphics/controls/ControlPart.h |
| 4 | `style/MatchResult.h` |  | style/PropertyCascade.h |
| 4 | `style/RuleData.h` |  | style/RuleSet.h |
| 4 | `style/RuleFeature.h` |  | style/RuleSet.h |
| 4 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleRelations.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h |
| 4 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/MatchResult.h, style/ScopedName.h |
| 4 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h |
| 4 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 4 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StyleLengthWrapperData.h, style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSFontFaceSrcValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 3 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 3 | `css/SelectorChecker.h` |  | style/MatchResult.h, style/StyleBuilderState.h |
| 3 | `css/calc/CSSCalcOperator.h` |  | style/calc/StyleCalculationTree.h |
| 3 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 3 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 3 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h, style/values/primitives/StyleLengthWrapper.h |
| 3 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 3 | `css/values/primitives/CSSURL.h` |  | css/CSSFontFaceSrcValue.h |
| 3 | `platform/graphics/Color.h` |  | css/values/color/CSSColorDescriptors.h, css/values/color/CSSHexColor.h, css/values/color/CSSResolvedColor.h, platform/graphics/FontPaletteValues.h, platform/graphics/Gradient.h, platform/graphics/GradientColorStop.h, platform/graphics/GraphicsTypes.h, platform/graphics/controls/ControlPart.h, platform/graphics/controls/ControlStyle.h |
| 3 | `platform/graphics/ColorComponents.h` |  | platform/graphics/ColorTypes.h, platform/graphics/ColorUtilities.h, platform/graphics/cg/GradientRendererCG.h |
| 3 | `platform/graphics/ColorMatrix.h` |  | platform/graphics/ColorTypes.h |
| 3 | `platform/graphics/ColorModels.h` |  | platform/graphics/ColorTypes.h |
| 3 | `platform/graphics/ColorTransferFunctions.h` |  | platform/graphics/ColorTypes.h |
| 3 | `platform/graphics/DoublePoint.h` |  | platform/graphics/FloatPoint.h, platform/graphics/LayoutPoint.h |
| 3 | `platform/graphics/FloatSize.h` |  | platform/graphics/FloatPoint.h, platform/graphics/GraphicsTypes.h, platform/graphics/LayoutSize.h |
| 3 | `platform/graphics/FontMetrics.h` |  | platform/graphics/FontSizeAdjust.h |
| 3 | `platform/graphics/FontTaggedSettings.h` |  | style/StyleBuilderState.h |
| 3 | `platform/graphics/IntPoint.h` |  | platform/graphics/DoublePoint.h, platform/graphics/FloatPoint.h, platform/graphics/FloatSize.h |
| 3 | `platform/graphics/LayoutPoint.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `platform/graphics/LayoutSize.h` |  | platform/graphics/LayoutPoint.h, style/AnchorPositionEvaluator.h |
| 3 | `platform/graphics/WindRule.h` |  | platform/graphics/GraphicsTypes.h |
| 3 | `platform/graphics/controls/ControlPart.h` | yes | platform/graphics/controls/ApplePayButtonPart.h |
| 3 | `style/PropertyCascade.h` |  | style/StyleBuilderState.h |
| 3 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h |
| 3 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/RuleSet.h` |  | style/MatchResult.h, style/StyleBuilderState.h |
| 3 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/StyleForVisitedLink.h` |  | style/StyleBuilderState.h |
| 3 | `style/TreeResolutionState.h` |  | style/StyleBuilderState.h |
| 3 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h, style/values/anchor-position/StylePositionVisibility.h, style/values/box/StyleMarginTrim.h, style/values/contain/StyleContain.h, style/values/images/StyleImageOrientation.h, style/values/masking/StyleMaskMode.h, style/values/non-standard/StyleWebKitLineBoxContain.h, style/values/pointerevents/StyleTouchAction.h, style/values/primitives/StylePrimitiveNumeric.h, style/values/scrollbars/StyleScrollbarWidth.h, style/values/speech/StyleSpeakAs.h, style/values/text-decoration/StyleTextDecorationLine.h, style/values/text-decoration/StyleTextEmphasisPosition.h, style/values/text-decoration/StyleTextUnderlinePosition.h, style/values/text/StyleHangingPunctuation.h, style/values/text/StyleTextAlign.h, style/values/text/StyleTextAlignLast.h, style/values/text/StyleTextTransform.h, style/values/ui/StyleResize.h |
| 3 | `style/values/primitives/StyleLengthWrapperData.h` |  | style/values/primitives/StyleLengthWrapper.h |
| 3 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 3 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `css/CSSFontFaceSrcValue.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `css/CSSPrimitiveValue.h` |  | css/CSSPrimitiveValueMappings.h, css/StylePropertiesInlines.h, css/parser/CSSParserToken.h |
| 2 | `css/CSSToLengthConversionData.h` |  | css/CSSPrimitiveValueMappings.h, css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h, style/StyleBuilderState.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `css/calc/CSSCalcSymbolTable.h` |  | css/CSSPrimitiveValueMappings.h, style/values/StyleValueTypes.h |
| 2 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/images/CSSGradient.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSURLModifiers.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 2 | `css/values/color/CSSColor.h` |  | css/values/images/CSSGradient.h |
| 2 | `css/values/primitives/CSSPosition.h` |  | css/values/images/CSSGradient.h, style/values/primitives/StylePosition.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/calc/StyleCalculationValue.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes+Canonicalization.h` |  | style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, css/values/primitives/CSSPosition.h, css/values/primitives/CSSPrimitiveNumericTypes+CSSValueVisitation.h, css/values/primitives/CSSPrimitiveNumericTypes+Canonicalization.h, css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `platform/graphics/AlphaPremultiplication.h` |  | platform/graphics/ColorInterpolation.h, platform/graphics/ColorInterpolationMethod.h |
| 2 | `platform/graphics/ColorInterpolationMethod.h` |  | css/values/images/CSSGradient.h, platform/graphics/ColorInterpolation.h, platform/graphics/Gradient.h, platform/graphics/cg/GradientRendererCG.h |
| 2 | `platform/graphics/ColorNormalization.h` |  | platform/graphics/ColorInterpolation.h |
| 2 | `platform/graphics/ColorTypes.h` |  | css/values/color/CSSColorDescriptors.h, css/values/color/CSSHexColor.h, platform/graphics/ColorConversion.h, platform/graphics/ColorInterpolation.h, platform/graphics/ColorInterpolationMethod.h, platform/graphics/ColorNormalization.h, platform/graphics/ColorSpace.h, platform/graphics/ColorUtilities.h |
| 2 | `platform/graphics/FloatPoint.h` |  | platform/graphics/FloatRect.h, platform/graphics/Gradient.h, platform/graphics/LayoutPoint.h, style/values/primitives/StylePosition.h |
| 2 | `platform/graphics/FontSizeAdjust.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `platform/graphics/GraphicsTypes.h` |  | css/CSSPrimitiveValueMappings.h, platform/graphics/Gradient.h |
| 2 | `platform/graphics/controls/ApplePayButtonPart.h` | yes | css/CSSPrimitiveValueMappings.h |
| 2 | `style/AnchorPositionEvaluator.h` |  | css/CSSPrimitiveValueMappings.h, style/TreeResolutionState.h |
| 2 | `style/StyleBuilderState.h` |  | css/CSSPrimitiveValueMappings.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `style/calc/StyleCalculationTree.h` |  | style/calc/StyleCalculationValue.h |
| 2 | `style/values/anchor-position/StylePositionTryFallbackTactic.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/anchor-position/StylePositionVisibility.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/box/StyleMarginTrim.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/contain/StyleContain.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/images/StyleImageOrientation.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/masking/StyleMaskMode.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/non-standard/StyleWebKitLineBoxContain.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/non-standard/StyleWebKitOverflowScrolling.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/non-standard/StyleWebKitTouchCallout.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/overflow/StyleScrollBehavior.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/pointerevents/StyleTouchAction.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/primitives/StyleLengthWrapper.h` |  | style/values/primitives/StylePosition.h |
| 2 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, style/values/primitives/StyleLengthWrapper.h, style/values/primitives/StylePosition.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `style/values/scrollbars/StyleScrollbarWidth.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/speech/StyleSpeakAs.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text-decoration/StyleTextDecorationLine.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text-decoration/StyleTextEmphasisPosition.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text-decoration/StyleTextUnderlinePosition.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text/StyleHangingPunctuation.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text/StyleTextAlign.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text/StyleTextAlignLast.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text/StyleTextTransform.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/ui/StyleResize.h` |  | css/CSSPrimitiveValueMappings.h |
| 1 | `css/CSSPrimitiveValueMappings.h` |  | css/values/images/CSSGradient.cpp |
| 1 | `css/values/images/CSSGradient.h` |  | css/values/images/CSSGradient.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes+CSSValueVisitation.h` |  | css/values/images/CSSGradient.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.h` |  | css/values/images/CSSGradient.cpp, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h` |  | css/values/images/CSSGradient.cpp |
| 1 | `platform/graphics/ColorInterpolation.h` |  | css/values/images/CSSGradient.cpp |
| 1 | `style/calc/StyleCalculationValue.h` |  | css/values/images/CSSGradient.cpp, style/values/primitives/StyleLengthWrapperData.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 1 | `style/values/primitives/StylePosition.h` |  | css/values/images/CSSGradient.cpp |
| 1 | `style/values/primitives/StylePrimitiveNumericTypes+Conversions.h` |  | css/values/images/CSSGradient.cpp |
| 0 | `css/values/images/CSSGradient.cpp` |  | (entry) |

## Boundaries

### animation (3)

- `animation/CompositeOperation.h` ← 1 file(s)
- `animation/ScrollAxis.h` ← 1 file(s)
- `animation/WebAnimationTypes.h` ← 1 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (5)

- `dom/Document.h` ← 1 file(s)
- `dom/Element.h` ← 3 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)

### external (6)

- `CoreGraphics/CoreGraphics.h` ← 2 file(s)
- `pal/text/TextEncoding.h` ← 1 file(s)
- `skia/core/SkColor.h` ← 1 file(s)
- `skia/core/SkColorSpace.h` ← 1 file(s)
- `skia/core/SkData.h` ← 1 file(s)
- `skia/core/SkShader.h` ← 1 file(s)

### loader (3)

- `loader/LoadedFromOpaqueSource.h` ← 2 file(s)
- `loader/ResourceLoaderOptions.h` ← 1 file(s)
- `loader/cache/CachedResourceHandle.h` ← 1 file(s)

### platform (16)

- `platform/BoxExtents.h` ← 2 file(s)
- `platform/CommonAtomStrings.h` ← 1 file(s)
- `platform/FloatConversion.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 5 file(s)
- `platform/PlatformExportMacros.h` ← 3 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 3 file(s)
- `platform/StyleAppearance.h` ← 1 file(s)
- `platform/ThemeTypes.h` ← 1 file(s)
- `platform/TouchAction.h` ← 1 file(s)
- `platform/cocoa/AppleVisualEffect.h` ← 1 file(s)
- `platform/text/TextFlags.h` ← 2 file(s)
- `platform/text/TextSpacing.h` ← 1 file(s)
- `platform/text/UnicodeBidi.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 3 file(s)

### rendering (5)

- `rendering/style/PositionTryOrder.h` ← 1 file(s)
- `rendering/style/RenderStyle.h` ← 1 file(s)
- `rendering/style/RenderStyleConstants.h` ← 5 file(s)
- `rendering/style/SVGRenderStyleDefs.h` ← 1 file(s)
- `rendering/style/StyleGradientImage.h` ← 1 file(s)

### wtf (61)

- `wtf/AbstractCanMakeCheckedPtr.h` ← 1 file(s)
- `wtf/ArgumentCoder.h` ← 1 file(s)
- `wtf/Assertions.h` ← 4 file(s)
- `wtf/BitSet.h` ← 3 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 6 file(s)
- `wtf/CheckedArithmetic.h` ← 1 file(s)
- `wtf/CheckedPtr.h` ← 2 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/Compiler.h` ← 1 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 6 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 28 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/GetPtr.h` ← 1 file(s)
- `wtf/HashCountedSet.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 3 file(s)
- `wtf/HashMap.h` ← 7 file(s)
- `wtf/HashSet.h` ← 1 file(s)
- `wtf/HashTraits.h` ← 1 file(s)
- `wtf/Hasher.h` ← 8 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/JSONValues.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 2 file(s)
- `wtf/Markable.h` ← 4 file(s)
- `wtf/MathExtras.h` ← 13 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/ObjectIdentifier.h` ← 2 file(s)
- `wtf/OptionSet.h` ← 6 file(s)
- `wtf/Platform.h` ← 8 file(s)
- `wtf/Ref.h` ← 8 file(s)
- `wtf/RefCounted.h` ← 3 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 4 file(s)
- `wtf/RetainPtr.h` ← 2 file(s)
- `wtf/StdLibExtras.h` ← 9 file(s)
- `wtf/TZoneMalloc.h` ← 13 file(s)
- `wtf/TZoneMallocInlines.h` ← 1 file(s)
- `wtf/ThreadSafeRefCounted.h` ← 4 file(s)
- `wtf/ThreadSafeWeakPtr.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 2 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/Variant.h` ← 1 file(s)
- `wtf/Vector.h` ← 13 file(s)
- `wtf/WeakHashMap.h` ← 1 file(s)
- `wtf/WeakHashSet.h` ← 2 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 10 file(s)
- `wtf/text/AtomStringHash.h` ← 4 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 2 file(s)
- `wtf/text/StringHash.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 7 file(s)
- `wtf/text/WTFString.h` ← 6 file(s)

## Problems

### unresolved (3)

- `CSSPropertyNames.h` ← 6 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 11 file(s)
