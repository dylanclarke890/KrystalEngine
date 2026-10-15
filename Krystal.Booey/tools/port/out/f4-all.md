# Port graph

Entries: `style/StyleBuilder.cpp`, `style/StyleBuilderState.cpp`, `style/StyleResolveForFont.cpp`, `style/StyleFontSizeFunctions.cpp`, `style/StyleCustomPropertyRegistry.cpp`, `css/CSSRegisteredCustomProperty.cpp`, `css/CSSVariableReferenceValue.cpp`, `css/CSSPendingSubstitutionValue.cpp`

- Files to port: **252**
- Boundary `animation`: 5
- Boundary `cssjit`: 2
- Boundary `dom`: 15
- Boundary `external`: 2
- Boundary `html`: 3
- Boundary `loader`: 6
- Boundary `page`: 2
- Boundary `platform`: 48
- Boundary `rendering`: 2
- Boundary `svg`: 3
- Boundary `worklets`: 1
- Boundary `wtf`: 66
- Unresolved: 11

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 6 | `css/StyleProperties.h` |  | css/ImmutableStyleProperties.h, css/MutableStyleProperties.h |
| 6 | `css/values/shapes/CSSFillRule.h` |  | css/values/shapes/CSSPathFunction.h, style/values/shapes/StyleFillRule.h |
| 6 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 5 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h, style/values/primitives/StyleLengthWrapper.h |
| 5 | `css/values/shapes/CSSPathFunction.h` |  | style/values/shapes/StylePathFunction.h |
| 5 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 5 | `style/values/primitives/StyleLengthWrapperData.h` |  | style/values/primitives/StyleLengthWrapper.h |
| 5 | `style/values/shapes/StyleFillRule.h` |  | style/values/shapes/StylePathFunction.h |
| 5 | `style/values/shapes/StylePathComputation.h` |  | style/values/shapes/StylePathFunction.h |
| 5 | `style/values/shapes/StyleWindRuleComputation.h` |  | style/values/shapes/StylePathFunction.h |
| 4 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 4 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 4 | `css/calc/CSSCalcOperator.h` |  | style/calc/StyleCalculationTree.h |
| 4 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 4 | `css/values/borders/CSSBoxShadow.h` |  | style/values/borders/StyleBoxShadow.h |
| 4 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 4 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 4 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 4 | `style/values/borders/StyleShadow.h` |  | style/values/borders/StyleBoxShadow.h |
| 4 | `style/values/primitives/StyleLengthWrapper.h` |  | style/values/primitives/StylePosition.h, style/values/svg/StyleSVGBaselineShift.h, style/values/svg/StyleSVGCenterCoordinateComponent.h, style/values/svg/StyleSVGCoordinateComponent.h, style/values/svg/StyleSVGRadius.h, style/values/svg/StyleSVGRadiusComponent.h, style/values/svg/StyleSVGStrokeDasharray.h, style/values/svg/StyleSVGStrokeDashoffset.h |
| 4 | `style/values/shapes/StylePathFunction.h` |  | style/values/svg/StyleSVGPathData.h |
| 3 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.h, style/RuleSet.h |
| 3 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/StyleRuleType.h` |  | css/StyleRule.h, css/parser/CSSParserContext.h |
| 3 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 3 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 3 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 3 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 3 | `css/query/GenericMediaQueryEvaluator.h` |  | css/query/MediaQueryEvaluator.h, style/ContainerQueryEvaluator.h |
| 3 | `css/query/MediaQuery.h` |  | css/StyleRule.h, css/query/MediaQueryEvaluator.h, style/RuleSet.h |
| 3 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 3 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 3 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/images/CSSGradient.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSURLModifiers.h, css/values/primitives/CSSUnevaluatedCalc.h, css/values/shapes/CSSFillRule.h, style/values/StyleValueTypes.h |
| 3 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h, style/values/color/StyleResolvedColor.h |
| 3 | `css/values/filter-effects/CSSBlurFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSBrightnessFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSContrastFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSDropShadowFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSFilterReference.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSGrayscaleFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSHueRotateFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSInvertFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSOpacityFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSSaturateFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSSepiaFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/primitives/CSSPosition.h` |  | css/values/images/CSSGradient.h, style/values/primitives/StylePosition.h |
| 3 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/borders/StyleBoxShadow.h, style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 3 | `css/values/primitives/CSSURLModifiers.h` |  | css/values/primitives/CSSURL.h |
| 3 | `rendering/style/PositionTryOrder.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `rendering/style/StyleInvalidImage.h` |  | rendering/style/StyleMultiImage.h |
| 3 | `style/MatchResult.h` |  | style/ElementRuleCollector.h, style/MatchedDeclarationsCache.h, style/PropertyCascade.h |
| 3 | `style/PropertyAllowlist.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleData.h |
| 3 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h, style/PseudoElementRequest.h, style/Styleable.h |
| 3 | `style/PseudoElementRequest.h` |  | style/ElementRuleCollector.h |
| 3 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/RuleData.h` |  | style/RuleSet.h |
| 3 | `style/RuleFeature.h` |  | style/RuleSet.h, style/StyleScopeRuleSets.h |
| 3 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 3 | `style/StyleRelations.h` |  | css/SelectorChecker.h |
| 3 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h, style/PseudoElementRequest.h |
| 3 | `style/UserAgentStyle.h` |  | style/StyleScopeRuleSets.h |
| 3 | `style/calc/StyleCalculationTree.h` |  | style/calc/StyleCalculationValue.h |
| 3 | `style/values/borders/StyleBoxShadow.h` |  | rendering/style/SVGRenderStyleDefs.h |
| 3 | `style/values/color/StyleOpacity.h` |  | rendering/style/SVGRenderStyleDefs.h |
| 3 | `style/values/primitives/StylePosition.h` |  | style/values/images/StyleGradient.h |
| 3 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 3 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 3 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `style/values/svg/StyleSVGBaselineShift.h` |  | rendering/style/SVGRenderStyleDefs.h |
| 3 | `style/values/svg/StyleSVGCenterCoordinateComponent.h` |  | rendering/style/SVGRenderStyleDefs.h |
| 3 | `style/values/svg/StyleSVGCoordinateComponent.h` |  | rendering/style/SVGRenderStyleDefs.h |
| 3 | `style/values/svg/StyleSVGMarkerResource.h` |  | rendering/style/SVGRenderStyleDefs.h |
| 3 | `style/values/svg/StyleSVGPaint.h` |  | rendering/style/SVGRenderStyleDefs.h |
| 3 | `style/values/svg/StyleSVGPathData.h` |  | rendering/style/SVGRenderStyleDefs.h |
| 3 | `style/values/svg/StyleSVGRadius.h` |  | rendering/style/SVGRenderStyleDefs.h |
| 3 | `style/values/svg/StyleSVGRadiusComponent.h` |  | rendering/style/SVGRenderStyleDefs.h |
| 3 | `style/values/svg/StyleSVGStrokeDasharray.h` |  | rendering/style/SVGRenderStyleDefs.h |
| 3 | `style/values/svg/StyleSVGStrokeDashoffset.h` |  | rendering/style/SVGRenderStyleDefs.h |
| 3 | `style/values/transforms/functions/StyleTransformFunctionBase.h` |  | style/values/transforms/StyleTransformFunction.h |
| 3 | `style/values/transforms/functions/StyleTransformFunctionWrapper.h` |  | style/values/transforms/StyleTransformFunction.h |
| 3 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StyleLengthWrapperData.h, style/values/primitives/StylePrimitiveNumeric.h |
| 2 | `css/CSSFontFace.h` |  | css/CSSFontFaceSet.h, css/CSSFontSelector.h |
| 2 | `css/CSSFontFaceSet.h` |  | css/CSSFontSelector.h |
| 2 | `css/CSSFontFaceSrcValue.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `css/CSSPrimitiveValue.h` |  | css/CSSCrossfadeValue.h, css/CSSPrimitiveValueMappings.h, css/CSSValuePool.h, css/StylePropertiesInlines.h, css/parser/CSSParserToken.h |
| 2 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, style/HasSelectorFilter.h, style/StyleResolver.h |
| 2 | `css/CSSToLengthConversionData.h` |  | css/CSSPrimitiveValueMappings.h, css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h, style/StyleBuilderState.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSCanvasValue.h, css/CSSColorValue.h, css/CSSCrossfadeValue.h, css/CSSCursorImageValue.h, css/CSSCustomPropertyValue.h, css/CSSFilterImageValue.h, css/CSSFontFaceSrcValue.h, css/CSSFontFeatureValue.h, css/CSSFontStyleWithAngleValue.h, css/CSSFontVariationValue.h, css/CSSImageValue.h, css/CSSNamedImageValue.h, css/CSSPaintImageValue.h, css/CSSPendingSubstitutionValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/CSSValueList.h, css/CSSValuePair.h, css/CSSVariableReferenceValue.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h, rendering/style/StyleImage.h, style/StyleCustomProperty.h |
| 2 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 2 | `css/SelectorChecker.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/StyleBuilderState.h |
| 2 | `css/StyleRule.h` |  | css/CSSFontSelector.h, style/RuleData.h, style/RuleSet.h, style/StyleCustomPropertyRegistry.h |
| 2 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcRandomCachingKey.h, css/calc/CSSCalcValue.h |
| 2 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `css/parser/CSSCustomPropertySyntax.h` |  | css/CSSRegisteredCustomProperty.h |
| 2 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h |
| 2 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 2 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 2 | `css/query/MediaQueryEvaluator.h` |  | style/ElementRuleCollector.h, style/StyleResolver.h |
| 2 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h, style/StyleCustomProperty.h |
| 2 | `css/values/color/CSSColor.h` |  | css/CSSColorValue.h, css/values/borders/CSSBoxShadow.h, css/values/filter-effects/CSSDropShadowFunction.h, css/values/filter-effects/CSSHueRotateFunction.h, css/values/images/CSSGradient.h, style/values/color/StyleColor.h |
| 2 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h, style/values/color/StyleColor.h |
| 2 | `css/values/color/CSSColorType.h` |  | style/values/color/StyleColor.h |
| 2 | `css/values/filter-effects/CSSFilter.h` |  | css/CSSFilterImageValue.h |
| 2 | `css/values/images/CSSGradient.h` |  | css/CSSGradientValue.h, style/values/images/StyleGradient.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/calc/StyleCalculationValue.h, style/values/primitives/StyleLengthResolution.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes+Canonicalization.h` |  | style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.h` |  | style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/CSSFontStyleWithAngleValue.h, css/parser/CSSPropertyParserConsumer+Font.h, css/values/borders/CSSBoxShadow.h, css/values/color/CSSColorDescriptors.h, css/values/filter-effects/CSSBlurFunction.h, css/values/filter-effects/CSSBrightnessFunction.h, css/values/filter-effects/CSSContrastFunction.h, css/values/filter-effects/CSSDropShadowFunction.h, css/values/filter-effects/CSSGrayscaleFunction.h, css/values/filter-effects/CSSHueRotateFunction.h, css/values/filter-effects/CSSInvertFunction.h, css/values/filter-effects/CSSOpacityFunction.h, css/values/filter-effects/CSSSaturateFunction.h, css/values/filter-effects/CSSSepiaFunction.h, css/values/primitives/CSSPosition.h, css/values/primitives/CSSPrimitiveNumericTypes+Canonicalization.h, css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, css/values/shapes/CSSPathFunction.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `css/values/primitives/CSSURL.h` |  | css/CSSCursorImageValue.h, css/CSSFontFaceSrcValue.h, css/CSSImageValue.h, css/values/filter-effects/CSSFilterReference.h, style/values/primitives/StyleURL.h |
| 2 | `rendering/style/AutosizeStatus.h` |  | style/StyleAdjuster.h |
| 2 | `rendering/style/RenderStyleConstants.h` |  | css/CSSFontFace.h, css/CSSPrimitiveValueMappings.h, css/CSSSelector.h, rendering/style/SVGRenderStyleDefs.h, style/PropertyAllowlist.h, style/PseudoElementIdentifier.h, style/PseudoElementRequest.h, style/StyleAdjuster.h, style/Styleable.h, style/values/text-decoration/StyleTextDecorationLine.h |
| 2 | `rendering/style/SVGRenderStyleDefs.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `rendering/style/StyleImage.h` |  | css/CSSGradientValue.h, rendering/style/StyleCachedImage.h, rendering/style/StyleGeneratedImage.h, rendering/style/StyleMultiImage.h, style/values/images/StyleImageWrapper.h |
| 2 | `rendering/style/StyleMultiImage.h` |  | rendering/style/StyleCursorImage.h, rendering/style/StyleImageSet.h |
| 2 | `style/AnchorPositionEvaluator.h` |  | css/CSSPrimitiveValueMappings.h, style/StyleScope.h, style/TreeResolutionState.h |
| 2 | `style/ElementRuleCollector.h` |  | style/StyleResolver.h |
| 2 | `style/InspectorCSSOMWrappers.h` |  | style/StyleResolver.h |
| 2 | `style/MatchedDeclarationsCache.h` |  | style/StyleResolver.h |
| 2 | `style/PropertyCascade.h` |  | style/StyleBuilder.h, style/StyleBuilderState.h, style/StyleResolver.h |
| 2 | `style/RuleSet.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/StyleBuilderState.h, style/StyleResolver.h, style/StyleScopeRuleSets.h |
| 2 | `style/StyleForVisitedLink.h` |  | style/StyleBuilderState.h |
| 2 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h, style/StyleScope.h |
| 2 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/ElementRuleCollector.h, style/MatchResult.h, style/ScopedName.h, style/StyleScope.h |
| 2 | `style/StyleScopeRuleSets.h` |  | style/StyleResolver.h |
| 2 | `style/Styleable.h` |  | style/StyleScope.h |
| 2 | `style/TreeResolutionState.h` |  | style/StyleBuilderState.h, style/StyleResolver.h |
| 2 | `style/calc/StyleCalculationValue.h` |  | style/values/primitives/StyleLengthWrapperData.h, style/values/primitives/StylePrimitiveNumericTypes+Calculation.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `style/computed/StyleComputedStyle.h` |  | style/computed/StyleComputedStyle+InitialInlines.h |
| 2 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h, style/values/anchor-position/StylePositionVisibility.h, style/values/box/StyleMarginTrim.h, style/values/color/StyleOpacity.h, style/values/contain/StyleContain.h, style/values/filter-effects/StyleFilter.h, style/values/images/StyleImageOrientation.h, style/values/images/StyleImageWrapper.h, style/values/masking/StyleMaskMode.h, style/values/non-standard/StyleWebKitLineBoxContain.h, style/values/pointerevents/StyleTouchAction.h, style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StylePrimitiveNumericTypes+Evaluation.h, style/values/primitives/StyleURL.h, style/values/scrollbars/StyleScrollbarWidth.h, style/values/shapes/StylePathComputation.h, style/values/shapes/StyleWindRuleComputation.h, style/values/speech/StyleSpeakAs.h, style/values/text-decoration/StyleTextDecorationLine.h, style/values/text-decoration/StyleTextEmphasisPosition.h, style/values/text-decoration/StyleTextUnderlinePosition.h, style/values/text/StyleHangingPunctuation.h, style/values/text/StyleTextAlign.h, style/values/text/StyleTextAlignLast.h, style/values/text/StyleTextTransform.h, style/values/transforms/StyleTransformFunction.h, style/values/transforms/functions/StyleTransformFunctionBase.h, style/values/ui/StyleResize.h |
| 2 | `style/values/anchor-position/StylePositionTryFallbackTactic.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/anchor-position/StylePositionVisibility.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/box/StyleMarginTrim.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/color/StyleColorOptions.h` |  | style/values/color/StyleColor.h |
| 2 | `style/values/color/StyleCurrentColor.h` |  | style/values/color/StyleColor.h |
| 2 | `style/values/color/StyleResolvedColor.h` |  | style/values/color/StyleColor.h |
| 2 | `style/values/contain/StyleContain.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/filter-effects/StyleFilter.h` |  | rendering/style/StyleFilterImage.h |
| 2 | `style/values/images/StyleGradient.h` |  | rendering/style/StyleGradientImage.h |
| 2 | `style/values/images/StyleImageOrientation.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/images/StyleImageWrapper.h` |  | style/StyleCustomProperty.h |
| 2 | `style/values/masking/StyleMaskMode.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/non-standard/StyleWebKitLineBoxContain.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/non-standard/StyleWebKitOverflowScrolling.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/non-standard/StyleWebKitTouchCallout.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/overflow/StyleScrollBehavior.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/pointerevents/StyleTouchAction.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/StyleCustomProperty.h, style/values/color/StyleOpacity.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `style/values/primitives/StylePrimitiveNumericTypes+Calculation.h` |  | style/values/primitives/StylePrimitiveNumericTypes+Evaluation.h |
| 2 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, style/values/borders/StyleBoxShadow.h, style/values/images/StyleGradient.h, style/values/primitives/StyleLengthWrapper.h, style/values/primitives/StylePosition.h, style/values/primitives/StylePrimitiveNumericTypes+Calculation.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h, style/values/primitives/StylePrimitiveNumericTypes+Evaluation.h, style/values/shapes/StylePathFunction.h, style/values/viewport/StyleZoom.h |
| 2 | `style/values/primitives/StyleURL.h` |  | rendering/style/StyleImage.h, style/StyleCustomProperty.h, style/values/svg/StyleSVGMarkerResource.h, style/values/svg/StyleSVGPaint.h |
| 2 | `style/values/scrollbars/StyleScrollbarWidth.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/speech/StyleSpeakAs.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text-decoration/StyleTextDecorationLine.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text-decoration/StyleTextEmphasisPosition.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text-decoration/StyleTextUnderlinePosition.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text/StyleHangingPunctuation.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text/StyleTextAlign.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text/StyleTextAlignLast.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text/StyleTextTransform.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/transforms/StyleTransformFunction.h` |  | style/StyleCustomProperty.h |
| 2 | `style/values/ui/StyleResize.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/viewport/StyleZoom.h` |  | style/StyleBuilderStateInlines.h |
| 1 | `css/CSSCanvasValue.h` |  | style/StyleBuilderState.cpp |
| 1 | `css/CSSColorValue.h` |  | css/CSSValuePool.h, style/StyleBuilderState.cpp |
| 1 | `css/CSSCrossfadeValue.h` |  | style/StyleBuilderState.cpp |
| 1 | `css/CSSCursorImageValue.h` |  | style/StyleBuilderState.cpp |
| 1 | `css/CSSCustomPropertyValue.h` |  | css/CSSRegisteredCustomProperty.cpp, css/CSSVariableReferenceValue.cpp, style/StyleBuilder.cpp |
| 1 | `css/CSSFilterImageValue.h` |  | style/StyleBuilderState.cpp |
| 1 | `css/CSSFontFeatureValue.h` |  | style/StyleResolveForFont.cpp |
| 1 | `css/CSSFontSelector.h` |  | style/StyleBuilder.cpp, style/StyleBuilderState.cpp, style/StyleResolveForFont.cpp |
| 1 | `css/CSSFontStyleWithAngleValue.h` |  | style/StyleResolveForFont.cpp |
| 1 | `css/CSSFontVariationValue.h` |  | style/StyleResolveForFont.cpp |
| 1 | `css/CSSFunctionValue.h` |  | style/StyleBuilderChecking.h, style/StyleBuilderState.cpp |
| 1 | `css/CSSGradientValue.h` |  | style/StyleBuilderState.cpp |
| 1 | `css/CSSImageSetValue.h` |  | style/StyleBuilderState.cpp |
| 1 | `css/CSSImageValue.h` |  | style/StyleBuilderState.cpp |
| 1 | `css/CSSNamedImageValue.h` |  | style/StyleBuilderState.cpp |
| 1 | `css/CSSPaintImageValue.h` |  | style/StyleBuilder.cpp, style/StyleBuilderState.cpp |
| 1 | `css/CSSPendingSubstitutionValue.h` |  | css/CSSPendingSubstitutionValue.cpp, style/StyleBuilder.cpp |
| 1 | `css/CSSPrimitiveValueMappings.h` |  | style/StyleResolveForFont.cpp |
| 1 | `css/CSSProperty.h` |  | css/CSSPendingSubstitutionValue.cpp, css/StyleProperties.h |
| 1 | `css/CSSRegisteredCustomProperty.h` |  | css/CSSRegisteredCustomProperty.cpp, css/CSSVariableReferenceValue.cpp, style/StyleBuilder.cpp, style/StyleCustomPropertyRegistry.cpp, style/StyleCustomPropertyRegistry.h |
| 1 | `css/CSSValueList.h` |  | css/CSSFunctionValue.h, css/CSSImageSetValue.h, style/StyleBuilderChecking.h, style/StyleResolveForFont.cpp |
| 1 | `css/CSSValuePair.h` |  | css/CSSCursorImageValue.h, style/StyleBuilder.cpp, style/StyleBuilderChecking.h, style/StyleResolveForFont.cpp |
| 1 | `css/CSSValuePool.h` |  | style/StyleBuilder.cpp |
| 1 | `css/CSSVariableData.h` |  | css/CSSCustomPropertyValue.h, css/CSSRegisteredCustomProperty.h, css/CSSVariableReferenceValue.cpp, css/CSSVariableReferenceValue.h, css/StyleRule.h, style/StyleCustomProperty.h |
| 1 | `css/CSSVariableReferenceValue.h` |  | css/CSSCustomPropertyValue.h, css/CSSPendingSubstitutionValue.h, css/CSSVariableReferenceValue.cpp |
| 1 | `css/CSSWideKeyword.h` |  | css/CSSCustomPropertyValue.h, style/StyleBuilder.cpp |
| 1 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h, style/StyleBuilder.cpp, style/StyleCustomPropertyRegistry.cpp |
| 1 | `css/FontSelectionValueInlines.h` |  | style/StyleResolveForFont.cpp |
| 1 | `css/StylePropertyShorthand.h` |  | style/StyleBuilder.cpp |
| 1 | `css/calc/CSSCalcRandomCachingKey.h` |  | style/StyleBuilderState.cpp |
| 1 | `css/calc/CSSCalcSymbolTable.h` |  | css/CSSPrimitiveValueMappings.h, style/StyleResolveForFont.cpp, style/values/StyleValueTypes.h |
| 1 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h, css/CSSVariableReferenceValue.cpp, css/CSSVariableReferenceValue.h |
| 1 | `css/parser/CSSPropertyParser.h` |  | css/CSSPendingSubstitutionValue.cpp, css/CSSVariableReferenceValue.cpp, style/StyleBuilder.cpp, style/StyleCustomPropertyRegistry.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Font.h` |  | style/StyleResolveForFont.cpp |
| 1 | `rendering/style/RenderStyle+GettersInlines.h` |  | css/CSSVariableReferenceValue.cpp, rendering/style/RenderStyle+SettersInlines.h, style/StyleBuilder.cpp, style/StyleFontSizeFunctions.cpp |
| 1 | `rendering/style/RenderStyle+SettersInlines.h` |  | style/StyleBuilder.cpp, style/StyleBuilderState.cpp, style/StyleBuilderStateInlines.h, style/StyleCustomPropertyRegistry.cpp |
| 1 | `rendering/style/RenderStyle.h` |  | css/query/MediaQueryEvaluator.h, rendering/style/RenderStyle+GettersInlines.h, style/MatchedDeclarationsCache.h, style/StyleBuilderState.h, style/StyleResolveForFont.cpp |
| 1 | `rendering/style/StyleCachedImage.h` |  | style/StyleBuilderState.cpp |
| 1 | `rendering/style/StyleCanvasImage.h` |  | style/StyleBuilderState.cpp |
| 1 | `rendering/style/StyleCrossfadeImage.h` |  | style/StyleBuilderState.cpp |
| 1 | `rendering/style/StyleCursorImage.h` |  | style/StyleBuilderState.cpp |
| 1 | `rendering/style/StyleCustomPropertyData.h` |  | style/StyleBuilder.cpp |
| 1 | `rendering/style/StyleFilterImage.h` |  | style/StyleBuilderState.cpp |
| 1 | `rendering/style/StyleGeneratedImage.h` |  | rendering/style/StyleCanvasImage.h, rendering/style/StyleCrossfadeImage.h, rendering/style/StyleFilterImage.h, rendering/style/StyleGradientImage.h, rendering/style/StyleInvalidImage.h, rendering/style/StyleNamedImage.h, rendering/style/StylePaintImage.h, style/StyleBuilderState.cpp |
| 1 | `rendering/style/StyleGradientImage.h` |  | style/StyleBuilderState.cpp |
| 1 | `rendering/style/StyleImageSet.h` |  | style/StyleBuilderState.cpp |
| 1 | `rendering/style/StyleNamedImage.h` |  | style/StyleBuilderState.cpp |
| 1 | `rendering/style/StylePaintImage.h` |  | style/StyleBuilderState.cpp |
| 1 | `style/StyleAdjuster.h` |  | style/StyleBuilder.cpp |
| 1 | `style/StyleBuilder.h` |  | css/CSSVariableReferenceValue.cpp, style/StyleBuilder.cpp, style/StyleBuilderState.cpp, style/StyleCustomPropertyRegistry.cpp |
| 1 | `style/StyleBuilderChecking.h` |  | style/StyleResolveForFont.cpp |
| 1 | `style/StyleBuilderGenerated.h` |  | style/StyleBuilder.cpp |
| 1 | `style/StyleBuilderState.h` |  | css/CSSPrimitiveValueMappings.h, style/StyleBuilder.h, style/StyleBuilderChecking.h, style/StyleBuilderState.cpp, style/StyleBuilderStateInlines.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 1 | `style/StyleBuilderStateInlines.h` |  | style/StyleBuilderState.cpp |
| 1 | `style/StyleCustomProperty.h` |  | css/CSSRegisteredCustomProperty.h, rendering/style/StyleCustomPropertyData.h, style/StyleBuilder.cpp, style/StyleCustomPropertyRegistry.cpp |
| 1 | `style/StyleCustomPropertyRegistry.h` |  | css/CSSVariableReferenceValue.cpp, style/StyleBuilder.cpp, style/StyleCustomPropertyRegistry.cpp |
| 1 | `style/StyleFontSizeFunctions.h` |  | style/StyleBuilder.cpp, style/StyleBuilderState.cpp, style/StyleBuilderStateInlines.h, style/StyleFontSizeFunctions.cpp, style/StyleResolveForFont.cpp |
| 1 | `style/StyleResolveForFont.h` |  | style/StyleResolveForFont.cpp |
| 1 | `style/StyleResolver.h` |  | css/CSSVariableReferenceValue.cpp, style/StyleCustomPropertyRegistry.cpp |
| 1 | `style/StyleScope.h` |  | style/StyleCustomPropertyRegistry.cpp |
| 1 | `style/computed/StyleComputedStyle+InitialInlines.h` |  | style/StyleBuilder.cpp |
| 1 | `style/values/color/StyleColor.h` |  | rendering/style/SVGRenderStyleDefs.h, style/StyleBuilderState.cpp, style/StyleCustomProperty.h, style/values/borders/StyleBoxShadow.h, style/values/images/StyleGradient.h, style/values/svg/StyleSVGPaint.h |
| 1 | `style/values/primitives/StyleLengthResolution.h` |  | style/StyleResolveForFont.cpp |
| 1 | `style/values/primitives/StylePrimitiveNumericTypes+Conversions.h` |  | style/StyleBuilderState.cpp, style/StyleResolveForFont.cpp |
| 1 | `style/values/primitives/StylePrimitiveNumericTypes+Evaluation.h` |  | style/StyleBuilderState.cpp, style/StyleResolveForFont.cpp |
| 0 | `css/CSSPendingSubstitutionValue.cpp` |  | (entry) |
| 0 | `css/CSSRegisteredCustomProperty.cpp` |  | (entry) |
| 0 | `css/CSSVariableReferenceValue.cpp` |  | (entry) |
| 0 | `style/StyleBuilder.cpp` |  | (entry) |
| 0 | `style/StyleBuilderState.cpp` |  | (entry) |
| 0 | `style/StyleCustomPropertyRegistry.cpp` |  | (entry) |
| 0 | `style/StyleFontSizeFunctions.cpp` |  | (entry) |
| 0 | `style/StyleResolveForFont.cpp` |  | (entry) |

## Boundaries

### animation (5)

- `animation/CompositeOperation.h` ← 1 file(s)
- `animation/KeyframeEffect.h` ← 1 file(s)
- `animation/ScrollAxis.h` ← 1 file(s)
- `animation/WebAnimation.h` ← 1 file(s)
- `animation/WebAnimationTypes.h` ← 2 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (15)

- `dom/ActiveDOMObject.h` ← 1 file(s)
- `dom/ConstantPropertyMap.h` ← 1 file(s)
- `dom/Document.h` ← 6 file(s)
- `dom/DocumentInlines.h` ← 1 file(s)
- `dom/DocumentSettingsValues.h` ← 1 file(s)
- `dom/DocumentView.h` ← 2 file(s)
- `dom/Element.h` ← 5 file(s)
- `dom/ElementInlines.h` ← 1 file(s)
- `dom/ElementTraversal.h` ← 1 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/NodeDocument.h` ← 1 file(s)
- `dom/PseudoElement.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)
- `dom/ScriptExecutionContext.h` ← 2 file(s)

### external (2)

- `pal/text/TextEncoding.h` ← 1 file(s)
- `unicode/umachine.h` ← 1 file(s)

### html (3)

- `html/CanvasObserver.h` ← 1 file(s)
- `html/HTMLCanvasElement.h` ← 1 file(s)
- `html/HTMLElement.h` ← 2 file(s)

### loader (6)

- `loader/LoadedFromOpaqueSource.h` ← 2 file(s)
- `loader/ResourceLoaderOptions.h` ← 2 file(s)
- `loader/cache/CachedImage.h` ← 2 file(s)
- `loader/cache/CachedImageClient.h` ← 2 file(s)
- `loader/cache/CachedResourceHandle.h` ← 6 file(s)
- `loader/cache/TrustedFonts.h` ← 1 file(s)

### page (2)

- `page/FrameDestructionObserverInlines.h` ← 1 file(s)
- `page/LocalFrame.h` ← 2 file(s)

### platform (48)

- `platform/BoxSides.h` ← 1 file(s)
- `platform/CommonAtomStrings.h` ← 1 file(s)
- `platform/FloatConversion.h` ← 2 file(s)
- `platform/LayoutUnit.h` ← 5 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 3 file(s)
- `platform/ThemeTypes.h` ← 1 file(s)
- `platform/Timer.h` ← 3 file(s)
- `platform/TouchAction.h` ← 1 file(s)
- `platform/animation/values/AcceleratedEffectOpacity.h` ← 1 file(s)
- `platform/cocoa/AppleVisualEffect.h` ← 1 file(s)
- `platform/graphics/Color.h` ← 5 file(s)
- `platform/graphics/ColorHash.h` ← 1 file(s)
- `platform/graphics/ColorInterpolationMethod.h` ← 1 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)
- `platform/graphics/FloatPoint.h` ← 2 file(s)
- `platform/graphics/FloatRect.h` ← 2 file(s)
- `platform/graphics/FloatSize.h` ← 4 file(s)
- `platform/graphics/FloatSizeHash.h` ← 1 file(s)
- `platform/graphics/Font.h` ← 1 file(s)
- `platform/graphics/FontCache.h` ← 1 file(s)
- `platform/graphics/FontCascade.h` ← 1 file(s)
- `platform/graphics/FontCascadeDescription.h` ← 1 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontMetrics.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/FontSelectionAlgorithm.h` ← 2 file(s)
- `platform/graphics/FontSelector.h` ← 1 file(s)
- `platform/graphics/FontSizeAdjust.h` ← 2 file(s)
- `platform/graphics/FontTaggedSettings.h` ← 4 file(s)
- `platform/graphics/GraphicsTypes.h` ← 1 file(s)
- `platform/graphics/Image.h` ← 1 file(s)
- `platform/graphics/IntPoint.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 2 file(s)
- `platform/graphics/LayoutRect.h` ← 2 file(s)
- `platform/graphics/LayoutSize.h` ← 3 file(s)
- `platform/graphics/Path.h` ← 1 file(s)
- `platform/graphics/SystemFontDatabase.h` ← 1 file(s)
- `platform/graphics/WindRule.h` ← 1 file(s)
- `platform/graphics/controls/ApplePayButtonPart.h` ← 1 file(s)
- `platform/graphics/filters/FilterOperations.h` ← 1 file(s)
- `platform/graphics/transforms/TransformOperation.h` ← 2 file(s)
- `platform/graphics/transforms/TransformationMatrix.h` ← 1 file(s)
- `platform/text/TextFlags.h` ← 3 file(s)
- `platform/text/TextSpacing.h` ← 1 file(s)
- `platform/text/UnicodeBidi.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 4 file(s)

### rendering (2)

- `rendering/RenderObject.h` ← 1 file(s)
- `rendering/RenderTheme.h` ← 1 file(s)

### svg (3)

- `svg/SVGLengthValue.h` ← 1 file(s)
- `svg/SVGPathByteStream.h` ← 1 file(s)
- `svg/SVGSVGElement.h` ← 1 file(s)

### worklets (1)

- `worklets/PaintWorkletGlobalScope.h` ← 1 file(s)

### wtf (66)

- `wtf/AbstractRefCountedAndCanMakeWeakPtr.h` ← 2 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 3 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 6 file(s)
- `wtf/CanMakeWeakPtr.h` ← 1 file(s)
- `wtf/CheckedPtr.h` ← 3 file(s)
- `wtf/CheckedRef.h` ← 2 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 2 file(s)
- `wtf/EnumTraits.h` ← 7 file(s)
- `wtf/FixedVector.h` ← 5 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 29 file(s)
- `wtf/Function.h` ← 5 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 2 file(s)
- `wtf/HashMap.h` ← 15 file(s)
- `wtf/HashSet.h` ← 5 file(s)
- `wtf/HashTraits.h` ← 1 file(s)
- `wtf/Hasher.h` ← 3 file(s)
- `wtf/Identified.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 3 file(s)
- `wtf/ListHashSet.h` ← 3 file(s)
- `wtf/MallocSpan.h` ← 1 file(s)
- `wtf/Markable.h` ← 3 file(s)
- `wtf/MathExtras.h` ← 4 file(s)
- `wtf/NeverDestroyed.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/Observer.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 8 file(s)
- `wtf/PointerComparison.h` ← 2 file(s)
- `wtf/Ref.h` ← 9 file(s)
- `wtf/RefCounted.h` ← 6 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 2 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 13 file(s)
- `wtf/SetForScope.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 8 file(s)
- `wtf/TZoneMalloc.h` ← 18 file(s)
- `wtf/TZoneMallocInlines.h` ← 2 file(s)
- `wtf/TypeCasts.h` ← 4 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 2 file(s)
- `wtf/Variant.h` ← 1 file(s)
- `wtf/Vector.h` ← 16 file(s)
- `wtf/WeakHashCountedSet.h` ← 1 file(s)
- `wtf/WeakHashMap.h` ← 2 file(s)
- `wtf/WeakHashSet.h` ← 6 file(s)
- `wtf/WeakListHashSet.h` ← 1 file(s)
- `wtf/WeakPtr.h` ← 4 file(s)
- `wtf/WeakRef.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 15 file(s)
- `wtf/text/AtomStringHash.h` ← 8 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringHash.h` ← 3 file(s)
- `wtf/text/StringParsingBuffer.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 6 file(s)
- `wtf/text/WTFString.h` ← 12 file(s)

## Problems

### unresolved (11)

- `CSSPropertyNames.h` ← 8 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 18 file(s)
- `RenderStyleProperties+GettersInlines.h` ← 1 file(s)
- `RenderStyleProperties+SettersInlines.h` ← 1 file(s)
- `RenderStyleProperties.h` ← 1 file(s)
- `SVGElementTypeHelpers.h` ← 1 file(s)
- `Settings.h` ← 4 file(s)
- `StyleComputedStyleProperties+InitialInlines.h` ← 1 file(s)
- `StyleComputedStyleProperties.h` ← 1 file(s)
- `WebKitFontFamilyNames.h` ← 2 file(s)
