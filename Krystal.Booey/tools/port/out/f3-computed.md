# Port graph

Entries: `style/computed/StyleComputedStyle.cpp`, `style/computed/StyleComputedStyleBase.cpp`, `rendering/style/RenderStyle.cpp`

- Files to port: **211**
- Boundary `animation`: 3
- Boundary `cssjit`: 2
- Boundary `dom`: 6
- Boundary `external`: 2
- Boundary `layout`: 3
- Boundary `loader`: 3
- Boundary `platform`: 45
- Boundary `rendering`: 7
- Boundary `svg`: 2
- Boundary `wtf`: 58
- Unresolved: 10

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 6 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 6 | `css/values/shapes/CSSFillRule.h` |  | css/values/shapes/CSSPathFunction.h, style/values/shapes/StyleFillRule.h |
| 5 | `css/CSSProperty.h` |  | css/StyleProperties.h |
| 5 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/calc/CSSCalcOperator.h` |  | style/calc/StyleCalculationTree.h |
| 5 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 5 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 5 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 5 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h |
| 5 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 5 | `css/values/shapes/CSSPathFunction.h` |  | style/values/shapes/StylePathFunction.h |
| 5 | `style/values/shapes/StyleFillRule.h` |  | style/values/shapes/StylePathFunction.h |
| 5 | `style/values/shapes/StylePathComputation.h` |  | style/values/shapes/StylePathFunction.h |
| 5 | `style/values/shapes/StyleWindRuleComputation.h` |  | style/values/shapes/StylePathFunction.h |
| 5 | `style/values/transforms/functions/StyleTransformFunctionWrapper.h` |  | style/values/transforms/StyleTransformFunction.h |
| 4 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/StyleProperties.h` |  | css/ImmutableStyleProperties.h, css/MutableStyleProperties.h |
| 4 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 4 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 4 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 4 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 4 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 4 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 4 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h, style/StyleCustomProperty.h |
| 4 | `css/values/borders/CSSBoxShadow.h` |  | style/values/borders/StyleBoxShadow.h |
| 4 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h, style/values/color/StyleResolvedColor.h |
| 4 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h, style/values/primitives/StyleLengthWrapper.h |
| 4 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/borders/StyleBoxShadow.h, style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/values/primitives/CSSURLModifiers.h` |  | css/values/primitives/CSSURL.h |
| 4 | `style/StyleForVisitedLink.h` |  | style/StyleBuilderState.h |
| 4 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h |
| 4 | `style/calc/StyleCalculationTree.h` |  | style/calc/StyleCalculationValue.h |
| 4 | `style/values/borders/StyleShadow.h` |  | style/values/borders/StyleBoxShadow.h |
| 4 | `style/values/images/StyleImageWrapper.h` |  | style/StyleCustomProperty.h |
| 4 | `style/values/primitives/StyleLengthWrapperData.h` |  | style/values/primitives/StyleLengthWrapper.h |
| 4 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 4 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `style/values/shapes/StylePathFunction.h` |  | style/values/svg/StyleSVGPathData.h |
| 4 | `style/values/transforms/StyleTransformFunction.h` |  | style/StyleCustomProperty.h |
| 4 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StyleLengthWrapperData.h, style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/CSSFontFaceSrcValue.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.h, style/RuleSet.h |
| 3 | `css/CSSToLengthConversionData.h` |  | css/CSSPrimitiveValueMappings.h, css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h, style/StyleBuilderState.h |
| 3 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h, style/values/anchor-position/StylePositionTryFallback.h |
| 3 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 3 | `css/StyleRuleType.h` |  | css/StyleRule.h, css/parser/CSSParserContext.h |
| 3 | `css/calc/CSSCalcSymbolTable.h` |  | css/CSSPrimitiveValueMappings.h, style/values/StyleValueTypes.h |
| 3 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/parser/CSSCustomPropertySyntax.h` |  | css/CSSRegisteredCustomProperty.h |
| 3 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h |
| 3 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h, css/CSSVariableReferenceValue.h |
| 3 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 3 | `css/query/GenericMediaQueryEvaluator.h` |  | css/query/MediaQueryEvaluator.h, style/ContainerQueryEvaluator.h |
| 3 | `css/query/MediaQuery.h` |  | css/StyleRule.h, css/query/MediaQueryEvaluator.h, style/RuleSet.h |
| 3 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 3 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, style/values/primitives/StylePrimitiveKeyword+ValueRepresentationNeeded.h |
| 3 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSURLModifiers.h, css/values/primitives/CSSUnevaluatedCalc.h, css/values/shapes/CSSFillRule.h, style/values/StyleValueTypes.h |
| 3 | `css/values/color/CSSColor.h` |  | css/CSSColorValue.h, css/values/borders/CSSBoxShadow.h, style/values/color/StyleColor.h |
| 3 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h, style/values/color/StyleColor.h |
| 3 | `css/values/color/CSSColorType.h` |  | style/values/color/StyleColor.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/calc/StyleCalculationValue.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/values/borders/CSSBoxShadow.h, css/values/color/CSSColorDescriptors.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, css/values/shapes/CSSPathFunction.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 3 | `css/values/primitives/CSSURL.h` |  | css/CSSFontFaceSrcValue.h, style/values/primitives/StyleURL.h |
| 3 | `rendering/style/PositionTryOrder.h` |  | style/AnchorPositionEvaluator.h, style/values/primitives/StylePrimitiveKeyword+ValueRepresentationNeeded.h |
| 3 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 3 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 3 | `style/MatchResult.h` |  | style/ElementRuleCollector.h, style/MatchedDeclarationsCache.h, style/PropertyCascade.h, style/ResolvedStyle.h |
| 3 | `style/PropertyAllowlist.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleData.h |
| 3 | `style/PseudoElementRequest.h` |  | style/ElementRuleCollector.h |
| 3 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/RuleData.h` |  | style/RuleSet.h |
| 3 | `style/RuleFeature.h` |  | style/RuleSet.h, style/StyleScopeRuleSets.h |
| 3 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h, style/values/anchor-position/StylePositionTryFallback.h |
| 3 | `style/StyleBuilderState.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `style/StyleCustomProperty.h` |  | css/CSSRegisteredCustomProperty.h |
| 3 | `style/StyleRelations.h` |  | css/SelectorChecker.h, style/ResolvedStyle.h |
| 3 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/ElementRuleCollector.h, style/MatchResult.h, style/ScopedName.h |
| 3 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h, style/PseudoElementRequest.h |
| 3 | `style/UserAgentStyle.h` |  | style/StyleScopeRuleSets.h |
| 3 | `style/calc/StyleCalculationValue.h` |  | style/values/primitives/StyleLengthWrapperData.h, style/values/primitives/StylePrimitiveNumericTypes+Calculation.h |
| 3 | `style/values/anchor-position/StylePositionTryFallbackTactic.h` |  | css/CSSPrimitiveValueMappings.h, style/values/anchor-position/StylePositionTryFallback.h |
| 3 | `style/values/anchor-position/StylePositionVisibility.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `style/values/borders/StyleBoxShadow.h` |  | rendering/style/SVGRenderStyleDefs.h |
| 3 | `style/values/box/StyleMarginTrim.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `style/values/color/StyleColorOptions.h` |  | style/values/color/StyleColor.h |
| 3 | `style/values/color/StyleCurrentColor.h` |  | style/values/color/StyleColor.h |
| 3 | `style/values/color/StyleOpacity.h` |  | rendering/style/SVGRenderStyleDefs.h |
| 3 | `style/values/color/StyleResolvedColor.h` |  | style/values/color/StyleColor.h |
| 3 | `style/values/contain/StyleContain.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `style/values/images/StyleImageOrientation.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `style/values/masking/StyleMaskMode.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `style/values/non-standard/StyleWebKitLineBoxContain.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `style/values/non-standard/StyleWebKitOverflowScrolling.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `style/values/non-standard/StyleWebKitTouchCallout.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `style/values/overflow/StyleScrollBehavior.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `style/values/pointerevents/StyleTouchAction.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `style/values/primitives/StyleLengthWrapper.h` |  | style/values/inline/StyleLineHeight.h, style/values/svg/StyleSVGBaselineShift.h, style/values/svg/StyleSVGCenterCoordinateComponent.h, style/values/svg/StyleSVGCoordinateComponent.h, style/values/svg/StyleSVGRadius.h, style/values/svg/StyleSVGRadiusComponent.h, style/values/svg/StyleSVGStrokeDasharray.h, style/values/svg/StyleSVGStrokeDashoffset.h, style/values/text/StyleLetterSpacing.h, style/values/text/StyleWordSpacing.h |
| 3 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/StyleCustomProperty.h, style/values/color/StyleOpacity.h, style/values/non-standard/StyleWebKitBorderSpacing.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 3 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 3 | `style/values/scrollbars/StyleScrollbarWidth.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `style/values/speech/StyleSpeakAs.h` |  | css/CSSPrimitiveValueMappings.h |
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
| 3 | `style/values/text-decoration/StyleTextEmphasisPosition.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `style/values/text-decoration/StyleTextUnderlinePosition.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `style/values/text/StyleHangingPunctuation.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `style/values/text/StyleLetterSpacing.h` |  | rendering/style/StyleFontData.h |
| 3 | `style/values/text/StyleTextAlign.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `style/values/text/StyleTextAlignLast.h` |  | css/CSSPrimitiveValueMappings.h |
| 3 | `style/values/text/StyleWordSpacing.h` |  | rendering/style/StyleFontData.h |
| 3 | `style/values/ui/StyleResize.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `css/CSSColorValue.h` |  | css/CSSValuePool.h |
| 2 | `css/CSSPrimitiveValue.h` |  | css/CSSPrimitiveValueMappings.h, css/CSSValuePool.h, css/StylePropertiesInlines.h, css/parser/CSSParserToken.h |
| 2 | `css/CSSPrimitiveValueMappings.h` |  | style/values/primitives/StylePrimitiveKeyword+Logging.h |
| 2 | `css/CSSRegisteredCustomProperty.h` |  | style/StyleCustomPropertyRegistry.h |
| 2 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, style/HasSelectorFilter.h, style/StyleResolver.h |
| 2 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSColorValue.h, css/CSSCustomPropertyValue.h, css/CSSFontFaceSrcValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/CSSVariableReferenceValue.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h, rendering/style/StyleImage.h, style/StyleCustomProperty.h |
| 2 | `css/CSSVariableData.h` |  | css/CSSCustomPropertyValue.h, css/CSSRegisteredCustomProperty.h, css/CSSVariableReferenceValue.h, css/StyleRule.h, style/StyleCustomProperty.h |
| 2 | `css/CSSVariableReferenceValue.h` |  | css/CSSCustomPropertyValue.h |
| 2 | `css/CSSWideKeyword.h` |  | css/CSSCustomPropertyValue.h |
| 2 | `css/SelectorChecker.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/StyleBuilderState.h, style/StyleTreeResolver.h |
| 2 | `css/StyleRule.h` |  | style/RuleData.h, style/RuleSet.h, style/StyleCustomPropertyRegistry.h |
| 2 | `css/query/MediaQueryEvaluator.h` |  | style/ElementRuleCollector.h, style/StyleResolver.h |
| 2 | `rendering/style/RenderStyle+GettersInlines.h` |  | rendering/style/RenderStyle+SettersInlines.h |
| 2 | `rendering/style/RenderStyleBase.h` |  | rendering/style/RenderStyleBase+ConstructionInlines.h |
| 2 | `rendering/style/RenderStyleConstants.h` |  | css/CSSPrimitiveValueMappings.h, css/CSSSelector.h, rendering/style/SVGRenderStyle.h, rendering/style/SVGRenderStyleDefs.h, rendering/style/StyleSelfAlignmentData.h, style/InlineTextBoxStyle.h, style/PropertyAllowlist.h, style/PseudoElementIdentifier.h, style/PseudoElementRequest.h, style/Styleable.h, style/values/primitives/StylePrimitiveKeyword+ValueRepresentationNeeded.h, style/values/text-decoration/StyleTextDecorationLine.h |
| 2 | `rendering/style/SVGRenderStyleDefs.h` |  | css/CSSPrimitiveValueMappings.h, rendering/style/SVGRenderStyle.h |
| 2 | `rendering/style/StyleFontData.h` |  | rendering/style/StyleInheritedData.h |
| 2 | `style/AnchorPositionEvaluator.h` |  | css/CSSPrimitiveValueMappings.h, style/StyleTreeResolver.h, style/TreeResolutionState.h |
| 2 | `style/ElementRuleCollector.h` |  | style/StyleResolver.h |
| 2 | `style/InspectorCSSOMWrappers.h` |  | style/StyleResolver.h |
| 2 | `style/MatchedDeclarationsCache.h` |  | style/StyleResolver.h |
| 2 | `style/PropertyCascade.h` |  | style/StyleBuilderState.h, style/StyleResolver.h, style/StyleTreeResolver.h |
| 2 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h, style/PseudoElementRequest.h, style/StyleExtractor.h, style/StyleExtractorState.h, style/Styleable.h, style/computed/StyleComputedStyleBase.h |
| 2 | `style/ResolvedStyle.h` |  | style/StyleTreeResolver.h |
| 2 | `style/RuleSet.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/StyleBuilderState.h, style/StyleResolver.h, style/StyleScopeRuleSets.h |
| 2 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h, style/StyleTreeResolver.h |
| 2 | `style/StyleChange.h` |  | style/StyleTreeResolver.h, style/StyleUpdate.h |
| 2 | `style/StyleExtractorState.h` |  | style/StyleExtractor.h |
| 2 | `style/StyleScopeRuleSets.h` |  | style/StyleResolver.h |
| 2 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h, style/StyleTreeResolver.h |
| 2 | `style/Styleable.h` |  | style/StyleTreeResolver.h |
| 2 | `style/TreeResolutionState.h` |  | style/StyleBuilderState.h, style/StyleResolver.h, style/StyleTreeResolver.h |
| 2 | `style/computed/StyleComputedStyle+ConstructionInlines.h` |  | rendering/style/RenderStyleBase+ConstructionInlines.h |
| 2 | `style/computed/StyleComputedStyle+GettersInlines.h` |  | style/computed/StyleComputedStyle+SettersInlines.h |
| 2 | `style/computed/StyleComputedStyle+InitialInlines.h` |  | style/computed/StyleComputedStyle+SettersInlines.h, style/computed/StyleComputedStyleBase+ConstructionInlines.h |
| 2 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h, style/values/anchor-position/StylePositionTryFallback.h, style/values/anchor-position/StylePositionVisibility.h, style/values/box/StyleMarginTrim.h, style/values/color/StyleOpacity.h, style/values/contain/StyleContain.h, style/values/grid/StyleGridAutoFlow.h, style/values/images/StyleImageOrientation.h, style/values/images/StyleImageWrapper.h, style/values/masking/StyleMaskMode.h, style/values/non-standard/StyleWebKitLineBoxContain.h, style/values/non-standard/StyleWebKitLocale.h, style/values/pointerevents/StyleTouchAction.h, style/values/primitives/StylePrimitiveKeyword+Logging.h, style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StylePrimitiveNumericTypes+Evaluation.h, style/values/primitives/StyleURL.h, style/values/scrollbars/StyleScrollbarWidth.h, style/values/shapes/StylePathComputation.h, style/values/shapes/StyleWindRuleComputation.h, style/values/speech/StyleSpeakAs.h, style/values/text-decoration/StyleTextDecorationLine.h, style/values/text-decoration/StyleTextEmphasisPosition.h, style/values/text-decoration/StyleTextUnderlinePosition.h, style/values/text/StyleHangingPunctuation.h, style/values/text/StyleLetterSpacing.h, style/values/text/StyleTextAlign.h, style/values/text/StyleTextAlignLast.h, style/values/text/StyleTextTransform.h, style/values/text/StyleWordSpacing.h, style/values/transforms/StyleTransformFunction.h, style/values/transforms/functions/StyleTransformFunctionBase.h, style/values/ui/StyleResize.h |
| 2 | `style/values/anchor-position/StylePositionTryFallback.h` |  | style/StyleTreeResolver.h |
| 2 | `style/values/color/StyleColor.h` |  | rendering/style/SVGRenderStyleDefs.h, rendering/style/StyleInheritedData.h, style/StyleCustomProperty.h, style/values/borders/StyleBoxShadow.h, style/values/svg/StyleSVGPaint.h |
| 2 | `style/values/grid/StyleGridAutoFlow.h` |  | style/computed/StyleComputedStyleBase.h |
| 2 | `style/values/inline/StyleLineHeight.h` |  | rendering/style/StyleInheritedData.h |
| 2 | `style/values/non-standard/StyleWebKitBorderSpacing.h` |  | rendering/style/StyleInheritedData.h |
| 2 | `style/values/primitives/StylePrimitiveKeyword+ValueRepresentationNeeded.h` |  | style/values/primitives/StylePrimitiveKeyword+Logging.h |
| 2 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/computed/StyleComputedStyleBase.h, style/values/primitives/StylePrimitiveNumeric.h |
| 2 | `style/values/primitives/StylePrimitiveNumericTypes+Calculation.h` |  | style/values/primitives/StylePrimitiveNumericTypes+Evaluation.h |
| 2 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, style/values/borders/StyleBoxShadow.h, style/values/primitives/StyleLengthWrapper.h, style/values/primitives/StylePrimitiveNumericTypes+Calculation.h, style/values/primitives/StylePrimitiveNumericTypes+Evaluation.h, style/values/shapes/StylePathFunction.h, style/values/svg/StyleSVGGlyphOrientationHorizontal.h, style/values/svg/StyleSVGGlyphOrientationVertical.h, style/values/transforms/functions/StyleScaleTransformFunction.h |
| 2 | `style/values/primitives/StyleURL.h` |  | rendering/style/StyleImage.h, style/StyleCustomProperty.h, style/values/svg/StyleSVGMarkerResource.h, style/values/svg/StyleSVGPaint.h |
| 2 | `style/values/svg/StyleSVGGlyphOrientationHorizontal.h` |  | rendering/style/SVGRenderStyle.h |
| 2 | `style/values/svg/StyleSVGGlyphOrientationVertical.h` |  | rendering/style/SVGRenderStyle.h |
| 2 | `style/values/transforms/functions/StyleTransformFunctionBase.h` |  | style/values/transforms/StyleTransformFunction.h, style/values/transforms/functions/StyleScaleTransformFunction.h |
| 1 | `css/CSSCustomPropertyValue.h` |  | rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyle.cpp, style/computed/StyleComputedStyleBase.cpp |
| 1 | `css/CSSValuePool.h` |  | rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyle.cpp, style/computed/StyleComputedStyleBase.cpp |
| 1 | `css/parser/CSSPropertyParser.h` |  | rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyle.cpp, style/computed/StyleComputedStyleBase.cpp |
| 1 | `rendering/style/AutosizeStatus.h` |  | rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyle.cpp, style/computed/StyleComputedStyleBase.cpp |
| 1 | `rendering/style/RenderStyle+SettersInlines.h` |  | rendering/style/RenderStyle.cpp |
| 1 | `rendering/style/RenderStyle.h` |  | css/query/MediaQueryEvaluator.h, rendering/style/RenderStyle+GettersInlines.h, rendering/style/RenderStyle.cpp, rendering/style/SVGRenderStyle.h, style/MatchedDeclarationsCache.h, style/ResolvedStyle.h, style/StyleBuilderState.h, style/StyleTreeResolver.h |
| 1 | `rendering/style/RenderStyleBase+ConstructionInlines.h` |  | rendering/style/RenderStyle.cpp |
| 1 | `rendering/style/RenderStyleDifference.h` |  | rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyleBase.cpp |
| 1 | `rendering/style/SVGRenderStyle.h` |  | rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyle.cpp, style/computed/StyleComputedStyleBase.cpp |
| 1 | `rendering/style/StyleImage.h` |  | rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyle.cpp, style/computed/StyleComputedStyleBase.cpp, style/values/images/StyleImageWrapper.h |
| 1 | `rendering/style/StyleInheritedData.h` |  | rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyle.cpp, style/computed/StyleComputedStyleBase.cpp |
| 1 | `rendering/style/StyleSelfAlignmentData.h` |  | rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyle.cpp, style/computed/StyleComputedStyleBase.cpp |
| 1 | `style/InlineTextBoxStyle.h` |  | rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyle.cpp, style/computed/StyleComputedStyleBase.cpp |
| 1 | `style/StyleCustomPropertyRegistry.h` |  | rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyle.cpp, style/computed/StyleComputedStyleBase.cpp |
| 1 | `style/StyleExtractor.h` |  | rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyle.cpp, style/computed/StyleComputedStyleBase.cpp |
| 1 | `style/StyleResolver.h` |  | rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyle.cpp, style/computed/StyleComputedStyleBase.cpp |
| 1 | `style/StyleTransformResolver.h` |  | rendering/style/RenderStyle.cpp |
| 1 | `style/StyleTreeResolver.h` |  | rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyle.cpp, style/computed/StyleComputedStyleBase.cpp |
| 1 | `style/computed/StyleComputedStyle+SettersInlines.h` |  | style/computed/StyleComputedStyle.cpp |
| 1 | `style/computed/StyleComputedStyle.h` |  | rendering/style/RenderStyleBase.h, style/StyleTransformResolver.h, style/computed/StyleComputedStyle+ConstructionInlines.h, style/computed/StyleComputedStyle+GettersInlines.h, style/computed/StyleComputedStyle+InitialInlines.h, style/computed/StyleComputedStyle.cpp |
| 1 | `style/computed/StyleComputedStyleBase+ConstructionInlines.h` |  | style/computed/StyleComputedStyle+ConstructionInlines.h, style/computed/StyleComputedStyle.cpp |
| 1 | `style/computed/StyleComputedStyleBase.h` |  | style/computed/StyleComputedStyleBase+ConstructionInlines.h, style/computed/StyleComputedStyleBase.cpp |
| 1 | `style/values/non-standard/StyleWebKitLocale.h` |  | style/computed/StyleComputedStyleBase.cpp |
| 1 | `style/values/primitives/StylePrimitiveKeyword+Logging.h` |  | style/computed/StyleComputedStyleBase.cpp |
| 1 | `style/values/primitives/StylePrimitiveNumericTypes+Evaluation.h` |  | rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyle+SettersInlines.h, style/computed/StyleComputedStyle.cpp, style/computed/StyleComputedStyleBase.cpp |
| 1 | `style/values/text-decoration/StyleTextDecorationLine.h` |  | css/CSSPrimitiveValueMappings.h, rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyle.cpp, style/computed/StyleComputedStyleBase.cpp |
| 1 | `style/values/text/StyleTextTransform.h` |  | css/CSSPrimitiveValueMappings.h, rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyle.cpp, style/computed/StyleComputedStyleBase.cpp |
| 1 | `style/values/transforms/functions/StyleScaleTransformFunction.h` |  | rendering/style/RenderStyle.cpp, style/computed/StyleComputedStyle.cpp, style/computed/StyleComputedStyleBase.cpp |
| 0 | `rendering/style/RenderStyle.cpp` |  | (entry) |
| 0 | `style/computed/StyleComputedStyle.cpp` |  | (entry) |
| 0 | `style/computed/StyleComputedStyleBase.cpp` |  | (entry) |

## Boundaries

### animation (3)

- `animation/CompositeOperation.h` ← 2 file(s)
- `animation/ScrollAxis.h` ← 5 file(s)
- `animation/WebAnimationTypes.h` ← 2 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (6)

- `dom/Document.h` ← 1 file(s)
- `dom/Element.h` ← 4 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/PseudoElement.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)

### external (2)

- `pal/text/TextEncoding.h` ← 1 file(s)
- `unicode/utypes.h` ← 1 file(s)

### layout (3)

- `layout/integration/inline/InlineIteratorInlineBox.h` ← 1 file(s)
- `layout/integration/inline/InlineIteratorLineBox.h` ← 1 file(s)
- `layout/integration/inline/InlineIteratorTextBox.h` ← 3 file(s)

### loader (3)

- `loader/LoadedFromOpaqueSource.h` ← 2 file(s)
- `loader/ResourceLoaderOptions.h` ← 1 file(s)
- `loader/cache/CachedResourceHandle.h` ← 1 file(s)

### platform (45)

- `platform/BoxExtents.h` ← 1 file(s)
- `platform/BoxSides.h` ← 1 file(s)
- `platform/CommonAtomStrings.h` ← 1 file(s)
- `platform/FloatConversion.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 5 file(s)
- `platform/Logging.h` ← 3 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 4 file(s)
- `platform/StyleAppearance.h` ← 1 file(s)
- `platform/ThemeTypes.h` ← 2 file(s)
- `platform/Timer.h` ← 1 file(s)
- `platform/TouchAction.h` ← 1 file(s)
- `platform/animation/values/AcceleratedEffectOpacity.h` ← 1 file(s)
- `platform/cocoa/AppleVisualEffect.h` ← 2 file(s)
- `platform/graphics/Color.h` ← 5 file(s)
- `platform/graphics/ColorBlending.h` ← 3 file(s)
- `platform/graphics/ColorHash.h` ← 1 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)
- `platform/graphics/FloatPoint.h` ← 1 file(s)
- `platform/graphics/FloatRect.h` ← 2 file(s)
- `platform/graphics/FloatRoundedRect.h` ← 3 file(s)
- `platform/graphics/FloatSize.h` ← 3 file(s)
- `platform/graphics/FontCascade.h` ← 5 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/FontSelector.h` ← 3 file(s)
- `platform/graphics/FontSizeAdjust.h` ← 2 file(s)
- `platform/graphics/FontTaggedSettings.h` ← 1 file(s)
- `platform/graphics/GraphicsTypes.h` ← 2 file(s)
- `platform/graphics/Image.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 2 file(s)
- `platform/graphics/LayoutRect.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 2 file(s)
- `platform/graphics/Path.h` ← 1 file(s)
- `platform/graphics/PathTraversalState.h` ← 3 file(s)
- `platform/graphics/WindRule.h` ← 3 file(s)
- `platform/graphics/controls/ApplePayButtonPart.h` ← 1 file(s)
- `platform/graphics/transforms/ScaleTransformOperation.h` ← 3 file(s)
- `platform/graphics/transforms/TransformOperation.h` ← 2 file(s)
- `platform/graphics/transforms/TransformationMatrix.h` ← 1 file(s)
- `platform/text/TextFlags.h` ← 3 file(s)
- `platform/text/TextSpacing.h` ← 2 file(s)
- `platform/text/UnicodeBidi.h` ← 2 file(s)
- `platform/text/WritingMode.h` ← 5 file(s)

### rendering (7)

- `rendering/MotionPath.h` ← 3 file(s)
- `rendering/Pagination.h` ← 3 file(s)
- `rendering/RenderBlock.h` ← 3 file(s)
- `rendering/RenderElement.h` ← 3 file(s)
- `rendering/RenderObject.h` ← 1 file(s)
- `rendering/RenderTheme.h` ← 3 file(s)
- `rendering/TransformOperationData.h` ← 3 file(s)

### svg (2)

- `svg/SVGLengthValue.h` ← 1 file(s)
- `svg/SVGPathByteStream.h` ← 1 file(s)

### wtf (58)

- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 3 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 6 file(s)
- `wtf/CanMakeWeakPtr.h` ← 1 file(s)
- `wtf/CheckedPtr.h` ← 2 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/DataRef.h` ← 3 file(s)
- `wtf/EnumSet.h` ← 2 file(s)
- `wtf/EnumTraits.h` ← 8 file(s)
- `wtf/FixedVector.h` ← 6 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 27 file(s)
- `wtf/Function.h` ← 1 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 11 file(s)
- `wtf/HashSet.h` ← 2 file(s)
- `wtf/Hasher.h` ← 3 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 2 file(s)
- `wtf/Markable.h` ← 3 file(s)
- `wtf/MathExtras.h` ← 7 file(s)
- `wtf/NeverDestroyed.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 9 file(s)
- `wtf/PointerComparison.h` ← 6 file(s)
- `wtf/Ref.h` ← 10 file(s)
- `wtf/RefCounted.h` ← 5 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 11 file(s)
- `wtf/StdLibExtras.h` ← 10 file(s)
- `wtf/TZoneMalloc.h` ← 11 file(s)
- `wtf/TypeCasts.h` ← 4 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 1 file(s)
- `wtf/Variant.h` ← 1 file(s)
- `wtf/Vector.h` ← 14 file(s)
- `wtf/WeakHashMap.h` ← 1 file(s)
- `wtf/WeakHashSet.h` ← 1 file(s)
- `wtf/WeakPtr.h` ← 2 file(s)
- `wtf/WeakRef.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 14 file(s)
- `wtf/text/AtomStringHash.h` ← 6 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringHash.h` ← 4 file(s)
- `wtf/text/StringParsingBuffer.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 10 file(s)
- `wtf/text/WTFString.h` ← 5 file(s)

## Problems

### unresolved (10)

- `CSSPropertyNames.h` ← 10 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 14 file(s)
- `RenderStyleProperties+GettersInlines.h` ← 1 file(s)
- `RenderStyleProperties+SettersInlines.h` ← 1 file(s)
- `RenderStyleProperties.h` ← 1 file(s)
- `StyleComputedStyleProperties+GettersInlines.h` ← 1 file(s)
- `StyleComputedStyleProperties+InitialInlines.h` ← 1 file(s)
- `StyleComputedStyleProperties+SettersInlines.h` ← 1 file(s)
- `StyleComputedStyleProperties.h` ← 1 file(s)
