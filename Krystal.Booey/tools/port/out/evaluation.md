# Port graph

Entries: `css/query/GenericMediaQueryEvaluator.cpp`, `css/query/MediaQueryEvaluator.cpp`, `css/query/MediaQueryFeatures.cpp`, `css/query/ContainerQueryFeatures.cpp`, `css/MediaQueryMatcher.cpp`, `css/MediaQueryList.cpp`, `style/ContainerQueryEvaluator.cpp`

- Files to port: **145**
- Boundary `animation`: 1
- Boundary `bindings`: 1
- Boundary `cssjit`: 2
- Boundary `dom`: 18
- Boundary `external`: 2
- Boundary `html`: 1
- Boundary `loader`: 2
- Boundary `page`: 6
- Boundary `platform`: 25
- Boundary `rendering`: 12
- Boundary `wtf`: 61
- Unresolved: 6

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 7 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 6 | `css/CSSProperty.h` |  | css/StyleProperties.h |
| 5 | `css/StyleProperties.h` |  | css/ImmutableStyleProperties.h, css/MutableStyleProperties.h |
| 5 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 5 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 5 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 4 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 4 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 4 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 4 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 4 | `css/calc/CSSCalcOperator.h` |  | style/calc/StyleCalculationTree.h |
| 4 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 4 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 4 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 4 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 4 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 4 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h, style/values/color/StyleResolvedColor.h |
| 4 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/values/primitives/CSSURLModifiers.h` |  | css/values/primitives/CSSURL.h |
| 4 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 4 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h, style/PseudoElementRequest.h |
| 3 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/SelectorChecker.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/StyleBuilderState.h |
| 3 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 3 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 3 | `css/parser/CSSCustomPropertySyntax.h` |  | css/CSSRegisteredCustomProperty.h |
| 3 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 3 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h, css/CSSVariableReferenceValue.h, css/query/GenericMediaQueryParser.h |
| 3 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 3 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSURLModifiers.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 3 | `css/values/color/CSSColor.h` |  | style/values/color/StyleColor.h |
| 3 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h, style/values/color/StyleColor.h |
| 3 | `css/values/color/CSSColorType.h` |  | style/values/color/StyleColor.h |
| 3 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, css/values/primitives/CSSPrimitiveNumericTypes+Canonicalization.h, css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, css/values/primitives/CSSRatio.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 3 | `css/values/primitives/CSSURL.h` |  | style/values/primitives/StyleURL.h |
| 3 | `style/PropertyAllowlist.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleData.h |
| 3 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h, style/PseudoElementRequest.h, style/Styleable.h |
| 3 | `style/PseudoElementRequest.h` |  | style/ElementRuleCollector.h |
| 3 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/RuleData.h` |  | style/RuleSet.h |
| 3 | `style/RuleFeature.h` |  | style/RuleSet.h, style/StyleScopeRuleSets.h |
| 3 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 3 | `style/StyleForVisitedLink.h` |  | style/StyleBuilderState.h |
| 3 | `style/UserAgentStyle.h` |  | style/StyleScopeRuleSets.h |
| 3 | `style/calc/StyleCalculationTree.h` |  | style/calc/StyleCalculationValue.h |
| 3 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h, style/values/images/StyleImageWrapper.h, style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleURL.h, style/values/transforms/StyleTransformFunction.h, style/values/transforms/functions/StyleTransformFunctionBase.h |
| 3 | `style/values/color/StyleColorOptions.h` |  | style/values/color/StyleColor.h |
| 3 | `style/values/color/StyleCurrentColor.h` |  | style/values/color/StyleColor.h |
| 3 | `style/values/color/StyleResolvedColor.h` |  | style/values/color/StyleColor.h |
| 3 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 3 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 3 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `style/values/transforms/functions/StyleTransformFunctionBase.h` |  | style/values/transforms/StyleTransformFunction.h |
| 3 | `style/values/transforms/functions/StyleTransformFunctionWrapper.h` |  | style/values/transforms/StyleTransformFunction.h |
| 3 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 2 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 2 | `css/CSSRegisteredCustomProperty.h` |  | style/StyleCustomPropertyRegistry.h |
| 2 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, style/HasSelectorFilter.h, style/StyleResolver.h |
| 2 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.h, style/RuleSet.h |
| 2 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSCustomPropertyValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/CSSRatioValue.h, css/CSSValueList.h, css/CSSVariableReferenceValue.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h, style/StyleCustomProperty.h |
| 2 | `css/CSSVariableData.h` |  | css/CSSCustomPropertyValue.h, css/CSSRegisteredCustomProperty.h, css/CSSVariableReferenceValue.h, css/StyleRule.h, style/StyleCustomProperty.h |
| 2 | `css/CSSVariableReferenceValue.h` |  | css/CSSCustomPropertyValue.h |
| 2 | `css/CSSWideKeyword.h` |  | css/CSSCustomPropertyValue.h |
| 2 | `css/StyleRuleType.h` |  | css/StyleRule.h, css/parser/CSSParserContext.h |
| 2 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/MediaQueryParserContext.h, css/query/GenericMediaQueryParser.h |
| 2 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 2 | `css/query/GenericMediaQueryParser.h` |  | css/query/MediaQueryParser.h |
| 2 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/ContainerQueryFeatures.h, css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryParser.h, css/query/MediaQuery.h |
| 2 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h, style/StyleCustomProperty.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/calc/StyleCalculationValue.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes+Canonicalization.h` |  | style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.h` |  | style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 2 | `css/values/primitives/CSSRatio.h` |  | css/CSSRatioValue.h, style/values/primitives/StyleRatio.h |
| 2 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `style/AnchorPositionEvaluator.h` |  | style/StyleScope.h, style/TreeResolutionState.h |
| 2 | `style/ElementRuleCollector.h` |  | style/StyleResolver.h |
| 2 | `style/InspectorCSSOMWrappers.h` |  | style/StyleResolver.h |
| 2 | `style/MatchResult.h` |  | style/ElementRuleCollector.h, style/MatchedDeclarationsCache.h, style/PropertyCascade.h, style/ResolvedStyle.h |
| 2 | `style/MatchedDeclarationsCache.h` |  | style/StyleResolver.h |
| 2 | `style/PropertyCascade.h` |  | style/StyleBuilder.h, style/StyleBuilderState.h, style/StyleResolver.h |
| 2 | `style/RuleSet.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/StyleBuilderState.h, style/StyleResolver.h, style/StyleScopeRuleSets.h |
| 2 | `style/StyleBuilderState.h` |  | style/StyleBuilder.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `style/StyleRelations.h` |  | css/SelectorChecker.h, style/ResolvedStyle.h |
| 2 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h, style/StyleScope.h |
| 2 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/ElementRuleCollector.h, style/MatchResult.h, style/ScopedName.h, style/StyleScope.h |
| 2 | `style/StyleScopeRuleSets.h` |  | style/StyleResolver.h |
| 2 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 2 | `style/Styleable.h` |  | style/StyleScope.h |
| 2 | `style/TreeResolutionState.h` |  | style/StyleBuilderState.h, style/StyleResolver.h |
| 2 | `style/calc/StyleCalculationValue.h` |  | style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 2 | `style/values/color/StyleColor.h` |  | style/StyleCustomProperty.h |
| 2 | `style/values/images/StyleImageWrapper.h` |  | style/StyleCustomProperty.h |
| 2 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/StyleCustomProperty.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h, style/values/primitives/StyleRatio.h |
| 2 | `style/values/primitives/StyleURL.h` |  | style/StyleCustomProperty.h |
| 2 | `style/values/transforms/StyleTransformFunction.h` |  | style/StyleCustomProperty.h |
| 1 | `css/CSSCustomPropertyValue.h` |  | css/query/ContainerQueryFeatures.cpp |
| 1 | `css/CSSPrimitiveValue.h` |  | css/StylePropertiesInlines.h, css/parser/CSSParserToken.h, css/query/GenericMediaQueryEvaluator.cpp, style/ContainerQueryEvaluator.cpp |
| 1 | `css/CSSRatioValue.h` |  | css/query/GenericMediaQueryEvaluator.cpp |
| 1 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryEvaluator.cpp, css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h, css/query/MediaQueryEvaluator.cpp, style/ContainerQueryEvaluator.cpp, style/StyleBuilderState.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 1 | `css/CSSValueList.h` |  | style/ContainerQueryEvaluator.cpp |
| 1 | `css/ComputedStyleDependencies.h` |  | css/query/ContainerQueryFeatures.cpp, css/query/MediaQueryFeatures.cpp, css/values/CSSValueTypes.h |
| 1 | `css/MediaList.h` |  | style/ContainerQueryEvaluator.cpp |
| 1 | `css/MediaQueryList.h` |  | css/MediaQueryList.cpp, css/MediaQueryMatcher.cpp |
| 1 | `css/MediaQueryListEvent.h` |  | css/MediaQueryList.cpp |
| 1 | `css/MediaQueryMatcher.h` |  | css/MediaQueryList.h, css/MediaQueryMatcher.cpp |
| 1 | `css/MediaQueryParserContext.h` |  | css/MediaList.h, css/MediaQueryMatcher.cpp, css/query/GenericMediaQueryParser.h, css/query/MediaQueryParser.h |
| 1 | `css/StyleRule.h` |  | style/ContainerQueryEvaluator.cpp, style/RuleData.h, style/RuleSet.h, style/StyleCustomPropertyRegistry.h |
| 1 | `css/query/ContainerQueryFeatures.h` |  | css/query/ContainerQueryFeatures.cpp, style/ContainerQueryEvaluator.cpp |
| 1 | `css/query/GenericMediaQueryEvaluator.h` |  | css/query/GenericMediaQueryEvaluator.cpp, css/query/MediaQueryEvaluator.h, style/ContainerQueryEvaluator.h |
| 1 | `css/query/MediaQuery.h` |  | css/MediaList.h, css/MediaQueryList.h, css/StyleRule.h, css/query/MediaQueryEvaluator.cpp, css/query/MediaQueryEvaluator.h, css/query/MediaQueryFeatures.h, css/query/MediaQueryParser.h, style/RuleSet.h |
| 1 | `css/query/MediaQueryEvaluator.h` |  | css/MediaQueryList.cpp, css/MediaQueryMatcher.cpp, css/query/MediaQueryEvaluator.cpp, css/query/MediaQueryFeatures.cpp, style/ElementRuleCollector.h, style/StyleResolver.h |
| 1 | `css/query/MediaQueryFeatures.h` |  | css/query/MediaQueryEvaluator.cpp, css/query/MediaQueryFeatures.cpp |
| 1 | `css/query/MediaQueryParser.h` |  | css/MediaQueryList.cpp, css/MediaQueryMatcher.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/query/ContainerQueryFeatures.cpp, css/query/MediaQueryFeatures.cpp, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 1 | `style/ContainerQueryEvaluator.h` |  | css/query/ContainerQueryFeatures.cpp, style/ContainerQueryEvaluator.cpp, style/SelectorMatchingState.h |
| 1 | `style/ResolvedStyle.h` |  | css/MediaQueryMatcher.cpp |
| 1 | `style/StyleBuilder.h` |  | css/query/ContainerQueryFeatures.cpp |
| 1 | `style/StyleCustomProperty.h` |  | css/CSSRegisteredCustomProperty.h, css/query/ContainerQueryFeatures.cpp |
| 1 | `style/StyleCustomPropertyRegistry.h` |  | css/query/ContainerQueryFeatures.cpp |
| 1 | `style/StyleFontSizeFunctions.h` |  | css/query/MediaQueryEvaluator.cpp |
| 1 | `style/StyleResolver.h` |  | css/MediaQueryMatcher.cpp |
| 1 | `style/StyleScope.h` |  | css/MediaQueryMatcher.cpp, style/ContainerQueryEvaluator.cpp |
| 1 | `style/values/primitives/StylePrimitiveNumericTypes+Conversions.h` |  | css/query/GenericMediaQueryEvaluator.cpp |
| 1 | `style/values/primitives/StyleRatio.h` |  | css/query/GenericMediaQueryEvaluator.cpp |
| 0 | `css/MediaQueryList.cpp` |  | (entry) |
| 0 | `css/MediaQueryMatcher.cpp` |  | (entry) |
| 0 | `css/query/ContainerQueryFeatures.cpp` |  | (entry) |
| 0 | `css/query/GenericMediaQueryEvaluator.cpp` |  | (entry) |
| 0 | `css/query/MediaQueryEvaluator.cpp` |  | (entry) |
| 0 | `css/query/MediaQueryFeatures.cpp` |  | (entry) |
| 0 | `style/ContainerQueryEvaluator.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/WebAnimationTypes.h` ← 2 file(s)

### bindings (1)

- `bindings/js/ScriptController.h` ← 1 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (18)

- `dom/ActiveDOMObject.h` ← 1 file(s)
- `dom/AddEventListenerOptionsInlines.h` ← 1 file(s)
- `dom/ComposedTreeAncestorIterator.h` ← 1 file(s)
- `dom/ContainerNodeInlines.h` ← 1 file(s)
- `dom/ContextDestructionObserverInlines.h` ← 1 file(s)
- `dom/Document.h` ← 4 file(s)
- `dom/DocumentPage.h` ← 1 file(s)
- `dom/DocumentQuirks.h` ← 2 file(s)
- `dom/DocumentView.h` ← 3 file(s)
- `dom/Element.h` ← 4 file(s)
- `dom/Event.h` ← 1 file(s)
- `dom/EventTarget.h` ← 2 file(s)
- `dom/EventTargetInlines.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/NodeDocument.h` ← 2 file(s)
- `dom/NodeRenderStyle.h` ← 2 file(s)
- `dom/PseudoElement.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)

### external (2)

- `pal/text/TextEncoding.h` ← 1 file(s)
- `unicode/umachine.h` ← 1 file(s)

### html (1)

- `html/HTMLFrameOwnerElement.h` ← 1 file(s)

### loader (2)

- `loader/DocumentLoader.h` ← 1 file(s)
- `loader/LoadedFromOpaqueSource.h` ← 2 file(s)

### page (6)

- `page/Chrome.h` ← 1 file(s)
- `page/FrameDestructionObserverInlines.h` ← 2 file(s)
- `page/LocalFrame.h` ← 1 file(s)
- `page/LocalFrameInlines.h` ← 1 file(s)
- `page/LocalFrameView.h` ← 2 file(s)
- `page/Quirks.h` ← 1 file(s)

### platform (25)

- `platform/BoxSides.h` ← 1 file(s)
- `platform/CommonAtomStrings.h` ← 1 file(s)
- `platform/FloatConversion.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 3 file(s)
- `platform/Logging.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScreenProperties.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 1 file(s)
- `platform/Theme.h` ← 1 file(s)
- `platform/Timer.h` ← 2 file(s)
- `platform/graphics/Color.h` ← 5 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)
- `platform/graphics/FloatSize.h` ← 1 file(s)
- `platform/graphics/FontCascade.h` ← 1 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/FontTaggedSettings.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutRect.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 2 file(s)
- `platform/graphics/transforms/TransformOperation.h` ← 2 file(s)
- `platform/graphics/transforms/TransformationMatrix.h` ← 1 file(s)
- `platform/text/TextFlags.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 2 file(s)

### rendering (12)

- `rendering/RenderBoxInlines.h` ← 1 file(s)
- `rendering/RenderElement.h` ← 1 file(s)
- `rendering/RenderElementInlines.h` ← 1 file(s)
- `rendering/RenderElementStyleInlines.h` ← 1 file(s)
- `rendering/RenderLayerCompositor.h` ← 1 file(s)
- `rendering/RenderObjectInlines.h` ← 1 file(s)
- `rendering/RenderView.h` ← 3 file(s)
- `rendering/style/PositionTryOrder.h` ← 1 file(s)
- `rendering/style/RenderStyle+GettersInlines.h` ← 1 file(s)
- `rendering/style/RenderStyle.h` ← 4 file(s)
- `rendering/style/RenderStyleConstants.h` ← 5 file(s)
- `rendering/style/StyleImage.h` ← 1 file(s)

### wtf (61)

- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 3 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 6 file(s)
- `wtf/CanMakeWeakPtr.h` ← 1 file(s)
- `wtf/CheckedPtr.h` ← 3 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 6 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 24 file(s)
- `wtf/Function.h` ← 1 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 11 file(s)
- `wtf/HashSet.h` ← 3 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/Identified.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 3 file(s)
- `wtf/MallocSpan.h` ← 1 file(s)
- `wtf/Markable.h` ← 3 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NeverDestroyed.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 5 file(s)
- `wtf/PointerComparison.h` ← 2 file(s)
- `wtf/Ref.h` ← 8 file(s)
- `wtf/RefCounted.h` ← 5 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 8 file(s)
- `wtf/RobinHoodHashMap.h` ← 1 file(s)
- `wtf/SetForScope.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 6 file(s)
- `wtf/TZoneMalloc.h` ← 13 file(s)
- `wtf/TZoneMallocInlines.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 3 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 2 file(s)
- `wtf/Variant.h` ← 1 file(s)
- `wtf/Vector.h` ← 14 file(s)
- `wtf/WeakHashMap.h` ← 2 file(s)
- `wtf/WeakHashSet.h` ← 2 file(s)
- `wtf/WeakListHashSet.h` ← 1 file(s)
- `wtf/WeakPtr.h` ← 3 file(s)
- `wtf/WeakRef.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 13 file(s)
- `wtf/text/AtomStringHash.h` ← 7 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 2 file(s)
- `wtf/text/StringHash.h` ← 1 file(s)
- `wtf/text/StringParsingBuffer.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 5 file(s)
- `wtf/text/WTFString.h` ← 6 file(s)

## Problems

### unresolved (6)

- `CSSPropertyNames.h` ← 7 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 13 file(s)
- `EventNames.h` ← 2 file(s)
- `EventTargetInterfaces.h` ← 1 file(s)
- `Settings.h` ← 1 file(s)
