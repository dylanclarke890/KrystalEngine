# Port graph

Entries: `css/query/MediaQueryParser.cpp`, `css/query/ContainerQueryParser.cpp`, `css/query/GenericMediaQuerySerialization.cpp`, `css/query/MediaQueryFeatures.cpp`, `css/query/ContainerQueryFeatures.cpp`, `css/query/ContainerQuery.cpp`, `css/MediaQueryParserContext.cpp`, `css/MediaList.cpp`, `css/parser/CSSSupportsParser.cpp`

- Files to port: **146**
- Boundary `animation`: 1
- Boundary `bindings`: 1
- Boundary `cssjit`: 2
- Boundary `dom`: 9
- Boundary `external`: 1
- Boundary `loader`: 2
- Boundary `page`: 5
- Boundary `platform`: 23
- Boundary `rendering`: 10
- Boundary `wtf`: 59
- Unresolved: 5

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 6 | `css/StyleProperties.h` |  | css/ImmutableStyleProperties.h, css/MutableStyleProperties.h |
| 6 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h |
| 5 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 5 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 5 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h |
| 5 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 5 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 5 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 5 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 4 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 4 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 4 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 4 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 4 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSURLModifiers.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 4 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h, style/values/color/StyleResolvedColor.h |
| 4 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/values/primitives/CSSURLModifiers.h` |  | css/values/primitives/CSSURL.h |
| 4 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 4 | `style/AnchorPositionEvaluator.h` |  | style/TreeResolutionState.h |
| 4 | `style/PropertyAllowlist.h` |  | style/MatchResult.h, style/RuleData.h |
| 4 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h |
| 4 | `style/RuleData.h` |  | style/RuleSet.h |
| 4 | `style/RuleFeature.h` |  | style/RuleSet.h |
| 4 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleRelations.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h |
| 4 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h |
| 3 | `css/CSSColorValue.h` |  | css/CSSValuePool.h |
| 3 | `css/CSSGroupingRule.h` |  | css/CSSConditionRule.h |
| 3 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h, style/StyleBuilderState.h |
| 3 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 3 | `css/SelectorChecker.h` |  | style/MatchResult.h, style/StyleBuilderState.h |
| 3 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 3 | `css/parser/CSSCustomPropertySyntax.h` |  | css/CSSRegisteredCustomProperty.h |
| 3 | `css/parser/CSSParserMode.h` |  | css/StyleSheet.h, css/parser/CSSParserContext.h, css/parser/CSSSelectorParserContext.h |
| 3 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 3 | `css/values/color/CSSColor.h` |  | css/CSSColorValue.h, style/values/color/StyleColor.h |
| 3 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h, style/values/color/StyleColor.h |
| 3 | `css/values/color/CSSColorType.h` |  | style/values/color/StyleColor.h |
| 3 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 3 | `css/values/primitives/CSSURL.h` |  | style/values/primitives/StyleURL.h |
| 3 | `style/MatchResult.h` |  | style/PropertyCascade.h |
| 3 | `style/RuleSet.h` |  | style/MatchResult.h, style/StyleBuilderState.h |
| 3 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 3 | `style/StyleForVisitedLink.h` |  | style/StyleBuilderState.h |
| 3 | `style/TreeResolutionState.h` |  | style/StyleBuilderState.h |
| 3 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h, style/values/images/StyleImageWrapper.h, style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleURL.h, style/values/transforms/StyleTransformFunction.h, style/values/transforms/functions/StyleTransformFunctionBase.h |
| 3 | `style/values/color/StyleColorOptions.h` |  | style/values/color/StyleColor.h |
| 3 | `style/values/color/StyleCurrentColor.h` |  | style/values/color/StyleColor.h |
| 3 | `style/values/color/StyleResolvedColor.h` |  | style/values/color/StyleColor.h |
| 3 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 3 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `style/values/transforms/functions/StyleTransformFunctionBase.h` |  | style/values/transforms/StyleTransformFunction.h |
| 3 | `style/values/transforms/functions/StyleTransformFunctionWrapper.h` |  | style/values/transforms/StyleTransformFunction.h |
| 3 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 2 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 2 | `css/CSSConditionRule.h` |  | css/CSSMediaRule.h |
| 2 | `css/CSSProperty.h` |  | css/StyleProperties.h, css/parser/CSSParser.h, css/parser/CSSPropertyParserState.h |
| 2 | `css/CSSRegisteredCustomProperty.h` |  | style/StyleCustomPropertyRegistry.h |
| 2 | `css/CSSRule.h` |  | css/CSSGroupingRule.h, css/CSSImportRule.h |
| 2 | `css/CSSRuleList.h` |  | css/CSSStyleSheet.h |
| 2 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, css/parser/MutableCSSSelector.h, style/HasSelectorFilter.h |
| 2 | `css/CSSSelectorList.h` |  | css/StyleRule.h, css/parser/CSSSelectorParser.h, style/RuleFeature.h, style/RuleSet.h |
| 2 | `css/CSSValuePool.h` |  | css/parser/CSSPropertyParserState.h |
| 2 | `css/CSSVariableData.h` |  | css/CSSCustomPropertyValue.h, css/CSSRegisteredCustomProperty.h, css/CSSVariableReferenceValue.h, css/StyleRule.h, style/StyleCustomProperty.h |
| 2 | `css/CSSVariableReferenceValue.h` |  | css/CSSCustomPropertyValue.h |
| 2 | `css/CSSWideKeyword.h` |  | css/CSSCustomPropertyValue.h |
| 2 | `css/StyleRuleType.h` |  | css/CSSRule.h, css/StyleRule.h, css/parser/CSSParserContext.h, css/parser/CSSPropertyParserState.h |
| 2 | `css/StyleSheet.h` |  | css/CSSStyleSheet.h |
| 2 | `css/StyleSheetContents.h` |  | css/parser/CSSSelectorParser.h |
| 2 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 2 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/MediaQueryParserContext.h, css/StyleSheetContents.h, css/parser/CSSSelectorParser.h, css/query/ContainerQueryParser.h, css/query/GenericMediaQueryParser.h |
| 2 | `css/parser/CSSParserEnum.h` |  | css/CSSRule.h, css/parser/CSSParser.h, css/parser/CSSSelectorParser.h, css/parser/CSSSupportsParser.h |
| 2 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSSupportsParser.h, css/parser/CSSTokenizer.h, css/query/ContainerQueryParser.h |
| 2 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h, css/CSSVariableReferenceValue.h, css/parser/CSSParser.h, css/parser/CSSSelectorParser.h, css/query/GenericMediaQueryParser.h |
| 2 | `css/parser/CSSSelectorParserContext.h` |  | css/parser/CSSSelectorParser.h |
| 2 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 2 | `css/parser/MutableCSSSelector.h` |  | css/parser/CSSSelectorParser.h |
| 2 | `css/query/GenericMediaQueryEvaluator.h` |  | css/query/MediaQueryEvaluator.h, style/ContainerQueryEvaluator.h |
| 2 | `css/query/GenericMediaQueryParser.h` |  | css/query/ContainerQueryParser.h, css/query/MediaQueryParser.h |
| 2 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/ContainerQueryFeatures.h, css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryParser.h, css/query/GenericMediaQuerySerialization.h, css/query/MediaQuery.h |
| 2 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h, style/StyleCustomProperty.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/parser/CSSPropertyParserConsumer+Font.h, css/values/color/CSSColorDescriptors.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 2 | `style/PropertyCascade.h` |  | style/StyleBuilder.h, style/StyleBuilderState.h |
| 2 | `style/StyleBuilderState.h` |  | style/StyleBuilder.h |
| 2 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/MatchResult.h, style/ScopedName.h |
| 2 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 2 | `style/values/color/StyleColor.h` |  | style/StyleCustomProperty.h |
| 2 | `style/values/images/StyleImageWrapper.h` |  | style/StyleCustomProperty.h |
| 2 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/StyleCustomProperty.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `style/values/primitives/StyleURL.h` |  | style/StyleCustomProperty.h |
| 2 | `style/values/transforms/StyleTransformFunction.h` |  | style/StyleCustomProperty.h |
| 1 | `css/CSSCustomPropertyValue.h` |  | css/query/ContainerQueryFeatures.cpp |
| 1 | `css/CSSImportRule.h` |  | css/MediaList.cpp |
| 1 | `css/CSSMarkup.h` |  | css/query/ContainerQuery.cpp, css/query/GenericMediaQuerySerialization.cpp, css/query/MediaQueryParser.cpp |
| 1 | `css/CSSMediaRule.h` |  | css/MediaList.cpp |
| 1 | `css/CSSPrimitiveValue.h` |  | css/CSSValuePool.h, css/StylePropertiesInlines.h, css/parser/CSSParserToken.h, css/query/ContainerQueryParser.cpp |
| 1 | `css/CSSStyleSheet.h` |  | css/MediaList.cpp |
| 1 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSColorValue.h, css/CSSCustomPropertyValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/CSSVariableReferenceValue.h, css/query/ContainerQuery.cpp, css/query/GenericMediaQuerySerialization.cpp, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h, style/StyleCustomProperty.h |
| 1 | `css/ComputedStyleDependencies.h` |  | css/query/ContainerQueryFeatures.cpp, css/query/MediaQueryFeatures.cpp, css/values/CSSValueTypes.h |
| 1 | `css/MediaList.h` |  | css/CSSStyleSheet.h, css/MediaList.cpp |
| 1 | `css/MediaQueryParserContext.h` |  | css/MediaList.h, css/MediaQueryParserContext.cpp, css/query/ContainerQueryParser.cpp, css/query/GenericMediaQueryParser.h, css/query/MediaQueryParser.h |
| 1 | `css/StyleRule.h` |  | css/parser/CSSParser.h, css/parser/CSSSupportsParser.cpp, style/RuleData.h, style/RuleSet.h, style/StyleCustomPropertyRegistry.h |
| 1 | `css/parser/CSSParser.h` |  | css/parser/CSSSupportsParser.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Font.h` |  | css/parser/CSSSupportsParser.cpp |
| 1 | `css/parser/CSSPropertyParserState.h` |  | css/parser/CSSSupportsParser.cpp |
| 1 | `css/parser/CSSSelectorParser.h` |  | css/parser/CSSSupportsParser.cpp |
| 1 | `css/parser/CSSSupportsParser.h` |  | css/parser/CSSSupportsParser.cpp |
| 1 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSSupportsParser.cpp, css/query/MediaQueryParser.cpp |
| 1 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, css/query/ContainerQuery.cpp, css/query/ContainerQueryParser.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 1 | `css/query/ContainerQueryFeatures.h` |  | css/query/ContainerQuery.cpp, css/query/ContainerQueryFeatures.cpp, css/query/ContainerQueryParser.cpp |
| 1 | `css/query/ContainerQueryParser.h` |  | css/query/ContainerQueryParser.cpp |
| 1 | `css/query/GenericMediaQuerySerialization.h` |  | css/query/ContainerQuery.cpp, css/query/GenericMediaQuerySerialization.cpp, css/query/MediaQueryParser.cpp |
| 1 | `css/query/MediaQuery.h` |  | css/CSSStyleSheet.h, css/MediaList.cpp, css/MediaList.h, css/StyleRule.h, css/query/MediaQueryEvaluator.h, css/query/MediaQueryFeatures.h, css/query/MediaQueryParser.h, style/RuleSet.h |
| 1 | `css/query/MediaQueryEvaluator.h` |  | css/query/MediaQueryFeatures.cpp |
| 1 | `css/query/MediaQueryFeatures.h` |  | css/query/MediaQueryFeatures.cpp, css/query/MediaQueryParser.cpp |
| 1 | `css/query/MediaQueryParser.h` |  | css/MediaList.cpp, css/query/MediaQueryParser.cpp |
| 1 | `css/values/CSSSerializationContext.h` |  | css/query/GenericMediaQuerySerialization.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/query/ContainerQueryFeatures.cpp, css/query/MediaQueryFeatures.cpp, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 1 | `style/ContainerQueryEvaluator.h` |  | css/query/ContainerQueryFeatures.cpp, style/SelectorMatchingState.h |
| 1 | `style/StyleBuilder.h` |  | css/query/ContainerQueryFeatures.cpp |
| 1 | `style/StyleCustomProperty.h` |  | css/CSSRegisteredCustomProperty.h, css/query/ContainerQueryFeatures.cpp |
| 1 | `style/StyleCustomPropertyRegistry.h` |  | css/query/ContainerQueryFeatures.cpp |
| 0 | `css/MediaList.cpp` |  | (entry) |
| 0 | `css/MediaQueryParserContext.cpp` |  | (entry) |
| 0 | `css/parser/CSSSupportsParser.cpp` |  | (entry) |
| 0 | `css/query/ContainerQuery.cpp` |  | (entry) |
| 0 | `css/query/ContainerQueryFeatures.cpp` |  | (entry) |
| 0 | `css/query/ContainerQueryParser.cpp` |  | (entry) |
| 0 | `css/query/GenericMediaQuerySerialization.cpp` |  | (entry) |
| 0 | `css/query/MediaQueryFeatures.cpp` |  | (entry) |
| 0 | `css/query/MediaQueryParser.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/WebAnimationTypes.h` ← 1 file(s)

### bindings (1)

- `bindings/js/ScriptController.h` ← 1 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (9)

- `dom/Document.h` ← 2 file(s)
- `dom/DocumentPage.h` ← 1 file(s)
- `dom/DocumentQuirks.h` ← 1 file(s)
- `dom/DocumentView.h` ← 1 file(s)
- `dom/Element.h` ← 3 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/ExceptionOr.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### loader (2)

- `loader/DocumentLoader.h` ← 1 file(s)
- `loader/LoadedFromOpaqueSource.h` ← 2 file(s)

### page (5)

- `page/Chrome.h` ← 1 file(s)
- `page/FrameDestructionObserverInlines.h` ← 1 file(s)
- `page/LocalDOMWindow.h` ← 1 file(s)
- `page/LocalFrame.h` ← 1 file(s)
- `page/LocalFrameView.h` ← 1 file(s)

### platform (23)

- `platform/BoxSides.h` ← 1 file(s)
- `platform/CommonAtomStrings.h` ← 3 file(s)
- `platform/LayoutUnit.h` ← 3 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScreenProperties.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 1 file(s)
- `platform/Theme.h` ← 1 file(s)
- `platform/graphics/Color.h` ← 5 file(s)
- `platform/graphics/ColorHash.h` ← 1 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)
- `platform/graphics/FloatSize.h` ← 1 file(s)
- `platform/graphics/FontCustomPlatformData.h` ← 1 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/FontTaggedSettings.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 1 file(s)
- `platform/graphics/SystemFontDatabase.h` ← 1 file(s)
- `platform/graphics/transforms/TransformOperation.h` ← 2 file(s)
- `platform/graphics/transforms/TransformationMatrix.h` ← 1 file(s)
- `platform/text/TextFlags.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 2 file(s)

### rendering (10)

- `rendering/RenderBoxInlines.h` ← 1 file(s)
- `rendering/RenderElementInlines.h` ← 1 file(s)
- `rendering/RenderElementStyleInlines.h` ← 1 file(s)
- `rendering/RenderLayerCompositor.h` ← 1 file(s)
- `rendering/RenderObjectInlines.h` ← 1 file(s)
- `rendering/RenderView.h` ← 1 file(s)
- `rendering/style/PositionTryOrder.h` ← 1 file(s)
- `rendering/style/RenderStyle.h` ← 2 file(s)
- `rendering/style/RenderStyleConstants.h` ← 3 file(s)
- `rendering/style/StyleImage.h` ← 1 file(s)

### wtf (59)

- `wtf/AbstractRefCounted.h` ← 1 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 3 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 6 file(s)
- `wtf/CheckedPtr.h` ← 3 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 6 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 21 file(s)
- `wtf/Function.h` ← 2 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 2 file(s)
- `wtf/HashMap.h` ← 10 file(s)
- `wtf/HashSet.h` ← 1 file(s)
- `wtf/Hasher.h` ← 3 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 2 file(s)
- `wtf/Markable.h` ← 3 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NeverDestroyed.h` ← 4 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 2 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 5 file(s)
- `wtf/PointerComparison.h` ← 2 file(s)
- `wtf/Ref.h` ← 6 file(s)
- `wtf/RefCounted.h` ← 5 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 2 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 5 file(s)
- `wtf/RobinHoodHashMap.h` ← 1 file(s)
- `wtf/SetForScope.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 6 file(s)
- `wtf/TZoneMalloc.h` ← 11 file(s)
- `wtf/TZoneMallocInlines.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 5 file(s)
- `wtf/URL.h` ← 3 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 1 file(s)
- `wtf/Variant.h` ← 1 file(s)
- `wtf/Vector.h` ← 13 file(s)
- `wtf/WeakHashMap.h` ← 1 file(s)
- `wtf/WeakHashSet.h` ← 2 file(s)
- `wtf/WeakPtr.h` ← 3 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 13 file(s)
- `wtf/text/AtomStringHash.h` ← 9 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 4 file(s)
- `wtf/text/StringParsingBuffer.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextPosition.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 5 file(s)
- `wtf/text/WTFString.h` ← 8 file(s)

## Problems

### unresolved (5)

- `CSSPropertyNames.h` ← 9 file(s)
- `CSSPropertyParsing.h` ← 1 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 14 file(s)
- `Settings.h` ← 1 file(s)
