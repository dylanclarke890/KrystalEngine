# Port graph

Entries: `style/StyleResolver.cpp`

- Files to port: **189**
- Boundary `animation`: 4
- Boundary `cssjit`: 2
- Boundary `dom`: 15
- Boundary `editing`: 1
- Boundary `external`: 1
- Boundary `inspector`: 1
- Boundary `loader`: 4
- Boundary `page`: 2
- Boundary `platform`: 35
- Boundary `rendering`: 2
- Boundary `svg`: 6
- Boundary `wtf`: 61
- Unresolved: 10

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 8 | `css/calc/CSSCalcOperator.h` |  | style/calc/StyleCalculationTree.h |
| 7 | `style/calc/StyleCalculationTree.h` |  | style/calc/StyleCalculationValue.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 6 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h |
| 6 | `css/values/shapes/CSSFillRule.h` |  | css/values/shapes/CSSPathFunction.h, style/values/shapes/StyleFillRule.h |
| 6 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 6 | `style/calc/StyleCalculationValue.h` |  | style/values/primitives/StyleLengthWrapperData.h |
| 5 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 5 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 5 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 5 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 5 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 5 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h, style/values/color/StyleResolvedColor.h |
| 5 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 5 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h, style/values/primitives/StyleLengthWrapper.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/values/borders/CSSBoxShadow.h, css/values/color/CSSColorDescriptors.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, css/values/shapes/CSSPathFunction.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 5 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 5 | `css/values/shapes/CSSPathFunction.h` |  | style/values/shapes/StylePathFunction.h |
| 5 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 5 | `style/values/primitives/StyleLengthWrapperData.h` |  | style/values/primitives/StyleLengthWrapper.h |
| 5 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 5 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 5 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 5 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 5 | `style/values/shapes/StyleFillRule.h` |  | style/values/shapes/StylePathFunction.h |
| 5 | `style/values/shapes/StylePathComputation.h` |  | style/values/shapes/StylePathFunction.h |
| 5 | `style/values/shapes/StyleWindRuleComputation.h` |  | style/values/shapes/StylePathFunction.h |
| 5 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StyleLengthWrapperData.h, style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 4 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 4 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 4 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 4 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 4 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 4 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/values/borders/CSSBoxShadow.h` |  | style/values/borders/StyleBoxShadow.h |
| 4 | `css/values/color/CSSColor.h` |  | css/values/borders/CSSBoxShadow.h, style/values/color/StyleColor.h |
| 4 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h, style/values/color/StyleColor.h |
| 4 | `css/values/color/CSSColorType.h` |  | style/values/color/StyleColor.h |
| 4 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/borders/StyleBoxShadow.h, style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 4 | `css/values/primitives/CSSURLModifiers.h` |  | css/values/primitives/CSSURL.h |
| 4 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 4 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 4 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h |
| 4 | `style/values/borders/StyleShadow.h` |  | style/values/borders/StyleBoxShadow.h |
| 4 | `style/values/color/StyleColorOptions.h` |  | style/values/color/StyleColor.h |
| 4 | `style/values/color/StyleCurrentColor.h` |  | style/values/color/StyleColor.h |
| 4 | `style/values/color/StyleResolvedColor.h` |  | style/values/color/StyleColor.h |
| 4 | `style/values/primitives/StyleLengthWrapper.h` |  | style/values/svg/StyleSVGBaselineShift.h, style/values/svg/StyleSVGCenterCoordinateComponent.h, style/values/svg/StyleSVGCoordinateComponent.h, style/values/svg/StyleSVGRadius.h, style/values/svg/StyleSVGRadiusComponent.h, style/values/svg/StyleSVGStrokeDasharray.h, style/values/svg/StyleSVGStrokeDashoffset.h |
| 4 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/values/color/StyleOpacity.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 4 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, style/values/borders/StyleBoxShadow.h, style/values/primitives/StyleLengthWrapper.h, style/values/shapes/StylePathFunction.h |
| 4 | `style/values/primitives/StyleURL.h` |  | style/values/svg/StyleSVGMarkerResource.h, style/values/svg/StyleSVGPaint.h |
| 4 | `style/values/shapes/StylePathFunction.h` |  | style/values/svg/StyleSVGPathData.h |
| 3 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 3 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 3 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 3 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/parser/CSSParserMode.h` |  | css/StyleSheet.h, css/parser/CSSParserContext.h |
| 3 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h, css/CSSVariableReferenceValue.h |
| 3 | `css/query/GenericMediaQueryEvaluator.h` |  | css/query/MediaQueryEvaluator.h, style/ContainerQueryEvaluator.h |
| 3 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 3 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 3 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSURLModifiers.h, css/values/primitives/CSSUnevaluatedCalc.h, css/values/shapes/CSSFillRule.h, style/values/StyleValueTypes.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/calc/StyleCalculationValue.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 3 | `css/values/primitives/CSSURL.h` |  | css/CSSFontFaceSrcValue.h, style/values/primitives/StyleURL.h |
| 3 | `rendering/style/PositionTryOrder.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h, style/PseudoElementRequest.h |
| 3 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 3 | `style/StyleForVisitedLink.h` |  | style/StyleBuilderState.h |
| 3 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h, style/PseudoElementRequest.h |
| 3 | `style/values/borders/StyleBoxShadow.h` |  | rendering/style/SVGRenderStyleDefs.h |
| 3 | `style/values/color/StyleColor.h` |  | rendering/style/SVGRenderStyleDefs.h, style/values/borders/StyleBoxShadow.h, style/values/svg/StyleSVGPaint.h |
| 3 | `style/values/color/StyleOpacity.h` |  | rendering/style/SVGRenderStyleDefs.h |
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
| 2 | `css/CSSFontFace.h` |  | css/CSSFontFaceSet.h, css/CSSFontSelector.h |
| 2 | `css/CSSFontFaceSet.h` |  | css/CSSFontSelector.h |
| 2 | `css/CSSFontFaceSrcValue.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `css/CSSPrimitiveValue.h` |  | css/CSSPrimitiveValueMappings.h, css/StylePropertiesInlines.h, css/parser/CSSParserToken.h |
| 2 | `css/CSSProperty.h` |  | css/StyleProperties.h |
| 2 | `css/CSSRule.h` |  | css/CSSKeyframeRule.h, css/CSSKeyframesRule.h, css/CSSStyleRule.h, css/CSSViewTransitionRule.h |
| 2 | `css/CSSRuleList.h` |  | css/CSSStyleSheet.h |
| 2 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.h, style/RuleSet.h |
| 2 | `css/CSSToLengthConversionData.h` |  | css/CSSPrimitiveValueMappings.h, css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h, style/StyleBuilderState.h |
| 2 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSCustomPropertyValue.h, css/CSSFontFaceSrcValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/CSSVariableReferenceValue.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 2 | `css/CSSVariableData.h` |  | css/CSSCustomPropertyValue.h, css/CSSVariableReferenceValue.h, css/StyleRule.h |
| 2 | `css/CSSVariableReferenceValue.h` |  | css/CSSCustomPropertyValue.h |
| 2 | `css/CSSWideKeyword.h` |  | css/CSSCustomPropertyValue.h |
| 2 | `css/MediaQueryParserContext.h` |  | css/MediaList.h |
| 2 | `css/SelectorChecker.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/StyleBuilderState.h |
| 2 | `css/StyleRuleType.h` |  | css/CSSRule.h, css/StyleRule.h, css/parser/CSSParserContext.h |
| 2 | `css/StyleSheet.h` |  | css/CSSStyleSheet.h |
| 2 | `css/calc/CSSCalcSymbolTable.h` |  | css/CSSPrimitiveValueMappings.h, style/values/StyleValueTypes.h |
| 2 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/MediaQueryParserContext.h, css/StyleSheetContents.h |
| 2 | `css/parser/CSSParserEnum.h` |  | css/CSSRule.h, style/RuleSetBuilder.h |
| 2 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 2 | `css/query/MediaQuery.h` |  | css/CSSStyleSheet.h, css/MediaList.h, css/StyleRule.h, css/query/MediaQueryEvaluator.h, style/RuleSet.h, style/RuleSetBuilder.h |
| 2 | `css/query/MediaQueryEvaluator.h` |  | style/ElementRuleCollector.h, style/StyleResolver.h |
| 2 | `rendering/style/AutosizeStatus.h` |  | style/StyleAdjuster.h |
| 2 | `rendering/style/RenderStyle.h` |  | css/query/MediaQueryEvaluator.h, rendering/style/RenderStyle+GettersInlines.h, style/MatchedDeclarationsCache.h, style/ResolvedStyle.h, style/StyleBuilderState.h |
| 2 | `rendering/style/SVGRenderStyleDefs.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/AnchorPositionEvaluator.h` |  | css/CSSPrimitiveValueMappings.h, style/TreeResolutionState.h |
| 2 | `style/InspectorCSSOMWrappers.h` |  | style/StyleResolver.h |
| 2 | `style/MatchResult.h` |  | style/ElementRuleCollector.h, style/MatchedDeclarationsCache.h, style/PageRuleCollector.h, style/PropertyCascade.h, style/ResolvedStyle.h |
| 2 | `style/MatchedDeclarationsCache.h` |  | style/StyleResolver.h |
| 2 | `style/PropertyAllowlist.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleData.h |
| 2 | `style/PropertyCascade.h` |  | style/MatchResultCache.h, style/StyleBuilder.h, style/StyleBuilderState.h, style/StyleResolver.h |
| 2 | `style/PseudoElementRequest.h` |  | style/ElementRuleCollector.h |
| 2 | `style/RuleData.h` |  | style/RuleSet.h |
| 2 | `style/RuleFeature.h` |  | style/RuleSet.h, style/StyleScopeRuleSets.h |
| 2 | `style/StyleBuilderState.h` |  | css/CSSPrimitiveValueMappings.h, style/StyleBuilder.h |
| 2 | `style/StyleRelations.h` |  | css/SelectorChecker.h, style/ResolvedStyle.h |
| 2 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/ElementRuleCollector.h, style/MatchResult.h, style/ScopedName.h |
| 2 | `style/StyleScopeRuleSets.h` |  | style/PageRuleCollector.h, style/StyleResolver.h |
| 2 | `style/TreeResolutionState.h` |  | style/StyleBuilderState.h, style/StyleResolver.h |
| 2 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h, style/values/anchor-position/StylePositionVisibility.h, style/values/box/StyleMarginTrim.h, style/values/color/StyleOpacity.h, style/values/contain/StyleContain.h, style/values/easing/StyleEasingFunction.h, style/values/images/StyleImageOrientation.h, style/values/masking/StyleMaskMode.h, style/values/non-standard/StyleWebKitLineBoxContain.h, style/values/pointerevents/StyleTouchAction.h, style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleURL.h, style/values/scrollbars/StyleScrollbarWidth.h, style/values/shapes/StylePathComputation.h, style/values/shapes/StyleWindRuleComputation.h, style/values/speech/StyleSpeakAs.h, style/values/text-decoration/StyleTextDecorationLine.h, style/values/text-decoration/StyleTextEmphasisPosition.h, style/values/text-decoration/StyleTextUnderlinePosition.h, style/values/text/StyleHangingPunctuation.h, style/values/text/StyleTextAlign.h, style/values/text/StyleTextAlignLast.h, style/values/text/StyleTextTransform.h, style/values/ui/StyleResize.h |
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
| 1 | `css/CSSCustomPropertyValue.h` |  | style/StyleResolver.cpp |
| 1 | `css/CSSFontSelector.h` |  | style/StyleResolver.cpp |
| 1 | `css/CSSKeyframeRule.h` |  | style/StyleResolver.cpp |
| 1 | `css/CSSKeyframesRule.h` |  | style/StyleResolver.cpp |
| 1 | `css/CSSPrimitiveValueMappings.h` |  | style/StyleResolver.cpp |
| 1 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, css/parser/MutableCSSSelector.h, style/HasSelectorFilter.h, style/StyleResolver.cpp, style/StyleResolver.h |
| 1 | `css/CSSStyleRule.h` |  | style/StyleResolver.cpp |
| 1 | `css/CSSStyleSheet.h` |  | style/StyleResolver.cpp |
| 1 | `css/CSSViewTransitionRule.h` |  | style/StyleResolver.cpp |
| 1 | `css/MediaList.h` |  | css/CSSStyleSheet.h, style/StyleResolver.cpp |
| 1 | `css/StyleProperties.h` |  | css/CSSViewTransitionRule.h, css/ImmutableStyleProperties.h, css/MutableStyleProperties.h, style/StyleResolver.cpp |
| 1 | `css/StylePropertyShorthand.h` |  | style/StyleResolver.cpp |
| 1 | `css/StyleRule.h` |  | css/CSSFontSelector.h, css/CSSKeyframeRule.h, css/CSSKeyframesRule.h, css/CSSViewTransitionRule.h, style/RuleData.h, style/RuleSet.h, style/StyleResolver.cpp |
| 1 | `css/StyleSheetContents.h` |  | style/StyleResolver.cpp |
| 1 | `css/parser/MutableCSSSelector.h` |  | style/StyleResolver.cpp |
| 1 | `rendering/style/RenderStyle+GettersInlines.h` |  | rendering/style/RenderStyle+SettersInlines.h, style/StyleResolver.cpp |
| 1 | `rendering/style/RenderStyle+SettersInlines.h` |  | style/StyleResolver.cpp |
| 1 | `rendering/style/RenderStyleConstants.h` |  | css/CSSFontFace.h, css/CSSPrimitiveValueMappings.h, css/CSSSelector.h, rendering/style/SVGRenderStyleDefs.h, style/PropertyAllowlist.h, style/PseudoElementIdentifier.h, style/PseudoElementRequest.h, style/StyleAdjuster.h, style/StyleResolver.cpp, style/values/text-decoration/StyleTextDecorationLine.h |
| 1 | `style/ElementRuleCollector.h` |  | style/StyleResolver.cpp, style/StyleResolver.h |
| 1 | `style/MatchResultCache.h` |  | style/StyleResolver.cpp |
| 1 | `style/PageRuleCollector.h` |  | style/StyleResolver.cpp |
| 1 | `style/ResolvedStyle.h` |  | style/MatchResultCache.h, style/StyleResolver.cpp |
| 1 | `style/RuleSet.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleSetBuilder.h, style/StyleBuilderState.h, style/StyleResolver.cpp, style/StyleResolver.h, style/StyleScopeRuleSets.h |
| 1 | `style/RuleSetBuilder.h` |  | style/StyleResolver.cpp |
| 1 | `style/StyleAdjuster.h` |  | style/StyleResolver.cpp |
| 1 | `style/StyleBuilder.h` |  | style/StyleResolver.cpp |
| 1 | `style/StyleFontSizeFunctions.h` |  | style/StyleResolver.cpp |
| 1 | `style/StyleResolveForDocument.h` |  | style/StyleResolver.cpp |
| 1 | `style/StyleResolver.h` |  | style/StyleResolver.cpp |
| 1 | `style/UserAgentStyle.h` |  | style/StyleResolver.cpp, style/StyleScopeRuleSets.h |
| 1 | `style/values/animations/StyleSingleAnimationRangeName.h` |  | style/StyleResolver.cpp |
| 1 | `style/values/easing/StyleEasingFunction.h` |  | style/StyleResolver.cpp |
| 0 | `style/StyleResolver.cpp` |  | (entry) |

## Boundaries

### animation (4)

- `animation/BlendingKeyframes.h` ← 1 file(s)
- `animation/CompositeOperation.h` ← 2 file(s)
- `animation/ScrollAxis.h` ← 1 file(s)
- `animation/WebAnimationTypes.h` ← 2 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (15)

- `dom/ActiveDOMObject.h` ← 1 file(s)
- `dom/Document.h` ← 1 file(s)
- `dom/DocumentInlines.h` ← 1 file(s)
- `dom/DocumentResourceLoader.h` ← 1 file(s)
- `dom/DocumentView.h` ← 1 file(s)
- `dom/Element.h` ← 3 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/ExceptionOr.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/NodeRenderStyle.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)
- `dom/ScriptExecutionContext.h` ← 1 file(s)
- `dom/ShadowRoot.h` ← 1 file(s)
- `dom/VisibilityAdjustment.h` ← 1 file(s)
- `dom/VisitedLinkState.h` ← 1 file(s)

### editing (1)

- `editing/FrameSelection.h` ← 1 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### inspector (1)

- `inspector/InspectorInstrumentation.h` ← 1 file(s)

### loader (4)

- `loader/LoadedFromOpaqueSource.h` ← 2 file(s)
- `loader/ResourceLoaderOptions.h` ← 1 file(s)
- `loader/cache/CachedResourceHandle.h` ← 2 file(s)
- `loader/cache/TrustedFonts.h` ← 1 file(s)

### page (2)

- `page/LocalFrame.h` ← 1 file(s)
- `page/LocalFrameView.h` ← 1 file(s)

### platform (35)

- `platform/BoxSides.h` ← 1 file(s)
- `platform/CommonAtomStrings.h` ← 2 file(s)
- `platform/LayoutUnit.h` ← 4 file(s)
- `platform/Logging.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 3 file(s)
- `platform/SharedStringHash.h` ← 1 file(s)
- `platform/ThemeTypes.h` ← 1 file(s)
- `platform/Timer.h` ← 2 file(s)
- `platform/TouchAction.h` ← 1 file(s)
- `platform/animation/TimingFunction.h` ← 2 file(s)
- `platform/animation/values/AcceleratedEffectOpacity.h` ← 1 file(s)
- `platform/cocoa/AppleVisualEffect.h` ← 1 file(s)
- `platform/graphics/Color.h` ← 5 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)
- `platform/graphics/FloatRect.h` ← 2 file(s)
- `platform/graphics/Font.h` ← 1 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/FontSelectionAlgorithm.h` ← 1 file(s)
- `platform/graphics/FontSelector.h` ← 1 file(s)
- `platform/graphics/FontSizeAdjust.h` ← 1 file(s)
- `platform/graphics/FontTaggedSettings.h` ← 2 file(s)
- `platform/graphics/GraphicsTypes.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutRect.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 1 file(s)
- `platform/graphics/Path.h` ← 1 file(s)
- `platform/graphics/WindRule.h` ← 1 file(s)
- `platform/graphics/controls/ApplePayButtonPart.h` ← 1 file(s)
- `platform/text/TextFlags.h` ← 3 file(s)
- `platform/text/TextSpacing.h` ← 1 file(s)
- `platform/text/UnicodeBidi.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 4 file(s)

### rendering (2)

- `rendering/RenderScrollbar.h` ← 1 file(s)
- `rendering/RenderView.h` ← 1 file(s)

### svg (6)

- `svg/SVGDocumentExtensions.h` ← 1 file(s)
- `svg/SVGElement.h` ← 1 file(s)
- `svg/SVGFontFaceElement.h` ← 1 file(s)
- `svg/SVGLengthValue.h` ← 1 file(s)
- `svg/SVGPathByteStream.h` ← 1 file(s)
- `svg/SVGSVGElement.h` ← 1 file(s)

### wtf (61)

- `wtf/AbstractRefCounted.h` ← 1 file(s)
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
- `wtf/Forward.h` ← 31 file(s)
- `wtf/Function.h` ← 1 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 2 file(s)
- `wtf/HashMap.h` ← 11 file(s)
- `wtf/HashSet.h` ← 4 file(s)
- `wtf/HashTraits.h` ← 1 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 2 file(s)
- `wtf/Markable.h` ← 3 file(s)
- `wtf/MathExtras.h` ← 4 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 2 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/Observer.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 8 file(s)
- `wtf/PointerComparison.h` ← 1 file(s)
- `wtf/Ref.h` ← 7 file(s)
- `wtf/RefCounted.h` ← 5 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 3 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 10 file(s)
- `wtf/Seconds.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 8 file(s)
- `wtf/TZoneMalloc.h` ← 11 file(s)
- `wtf/TZoneMallocInlines.h` ← 2 file(s)
- `wtf/TypeCasts.h` ← 4 file(s)
- `wtf/URL.h` ← 3 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 1 file(s)
- `wtf/Vector.h` ← 17 file(s)
- `wtf/WeakHashMap.h` ← 2 file(s)
- `wtf/WeakHashSet.h` ← 4 file(s)
- `wtf/WeakPtr.h` ← 6 file(s)
- `wtf/WeakRef.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 12 file(s)
- `wtf/text/AtomStringHash.h` ← 9 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringHash.h` ← 3 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextPosition.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 6 file(s)
- `wtf/text/WTFString.h` ← 5 file(s)

## Problems

### unresolved (10)

- `CSSPropertyNames.h` ← 9 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 15 file(s)
- `RenderStyleProperties+GettersInlines.h` ← 1 file(s)
- `RenderStyleProperties+SettersInlines.h` ← 1 file(s)
- `RenderStyleProperties.h` ← 1 file(s)
- `SVGElementTypeHelpers.h` ← 1 file(s)
- `Settings.h` ← 2 file(s)
- `UserAgentParts.h` ← 1 file(s)
- `WebKitFontFamilyNames.h` ← 2 file(s)
