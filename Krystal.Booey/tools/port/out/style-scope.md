# Port graph

Entries: `style/StyleScope.cpp`

- Files to port: **131**
- Boundary `animation`: 1
- Boundary `cssjit`: 2
- Boundary `dom`: 18
- Boundary `external`: 1
- Boundary `html`: 5
- Boundary `inspector`: 1
- Boundary `loader`: 3
- Boundary `page`: 3
- Boundary `platform`: 23
- Boundary `rendering`: 9
- Boundary `svg`: 1
- Boundary `wtf`: 64
- Unresolved: 6

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 8 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 8 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 7 | `css/CSSProperty.h` |  | css/StyleProperties.h |
| 7 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 7 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 7 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 7 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h |
| 7 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 7 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 6 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 6 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/StyleProperties.h` |  | css/ImmutableStyleProperties.h, css/MutableStyleProperties.h |
| 6 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 6 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 6 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 6 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 6 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h, style/values/color/StyleResolvedColor.h |
| 6 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 6 | `css/values/primitives/CSSURLModifiers.h` |  | css/values/primitives/CSSURL.h |
| 6 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 6 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 6 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h |
| 5 | `css/CSSPrimitiveValue.h` |  | css/StylePropertiesInlines.h, css/parser/CSSParserToken.h |
| 5 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 5 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 5 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 5 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 5 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 5 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSURLModifiers.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 5 | `css/values/color/CSSColor.h` |  | style/values/color/StyleColor.h |
| 5 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h, style/values/color/StyleColor.h |
| 5 | `css/values/color/CSSColorType.h` |  | style/values/color/StyleColor.h |
| 5 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 5 | `css/values/primitives/CSSURL.h` |  | style/values/primitives/StyleURL.h |
| 5 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 5 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 5 | `style/values/color/StyleColorOptions.h` |  | style/values/color/StyleColor.h |
| 5 | `style/values/color/StyleCurrentColor.h` |  | style/values/color/StyleColor.h |
| 5 | `style/values/color/StyleResolvedColor.h` |  | style/values/color/StyleColor.h |
| 5 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 5 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 5 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 5 | `style/values/transforms/functions/StyleTransformFunctionBase.h` |  | style/values/transforms/StyleTransformFunction.h |
| 5 | `style/values/transforms/functions/StyleTransformFunctionWrapper.h` |  | style/values/transforms/StyleTransformFunction.h |
| 5 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h, style/StyleCustomProperty.h |
| 4 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 4 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h |
| 4 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h, style/StyleCustomProperty.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 4 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h, style/PseudoElementRequest.h |
| 4 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h, style/values/images/StyleImageWrapper.h, style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleURL.h, style/values/transforms/StyleTransformFunction.h, style/values/transforms/functions/StyleTransformFunctionBase.h |
| 4 | `style/values/color/StyleColor.h` |  | style/StyleCustomProperty.h |
| 4 | `style/values/images/StyleImageWrapper.h` |  | style/StyleCustomProperty.h |
| 4 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/StyleCustomProperty.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 4 | `style/values/primitives/StyleURL.h` |  | style/StyleCustomProperty.h |
| 4 | `style/values/transforms/StyleTransformFunction.h` |  | style/StyleCustomProperty.h |
| 3 | `css/CSSCounterStyleDescriptors.h` |  | css/CSSCounterStyle.h |
| 3 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h, style/StyleBuilderState.h |
| 3 | `css/CSSVariableData.h` |  | css/CSSRegisteredCustomProperty.h, css/StyleRule.h, style/StyleCustomProperty.h |
| 3 | `css/MediaQueryParserContext.h` |  | css/MediaList.h |
| 3 | `css/SelectorChecker.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/StyleBuilderState.h |
| 3 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 3 | `css/StyleRuleType.h` |  | css/StyleRule.h, css/parser/CSSParserContext.h |
| 3 | `css/parser/CSSCustomPropertySyntax.h` |  | css/CSSRegisteredCustomProperty.h |
| 3 | `css/parser/CSSParserMode.h` |  | css/StyleSheet.h, css/parser/CSSParserContext.h |
| 3 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 3 | `css/query/GenericMediaQueryEvaluator.h` |  | css/query/MediaQueryEvaluator.h, style/ContainerQueryEvaluator.h |
| 3 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 3 | `style/MatchResult.h` |  | style/ElementRuleCollector.h, style/MatchedDeclarationsCache.h, style/PropertyCascade.h, style/ResolvedStyle.h |
| 3 | `style/PropertyAllowlist.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleData.h |
| 3 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h, style/PseudoElementRequest.h, style/Styleable.h |
| 3 | `style/PseudoElementRequest.h` |  | style/ElementRuleCollector.h |
| 3 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/StyleCustomProperty.h` |  | css/CSSRegisteredCustomProperty.h |
| 3 | `style/StyleForVisitedLink.h` |  | style/StyleBuilderState.h |
| 3 | `style/StyleRelations.h` |  | css/SelectorChecker.h, style/ResolvedStyle.h |
| 3 | `style/UserAgentStyle.h` |  | style/StyleScopeRuleSets.h |
| 2 | `css/CSSCounterStyle.h` |  | css/CSSCounterStyleRegistry.h |
| 2 | `css/CSSFontFace.h` |  | css/CSSFontFaceSet.h, css/CSSFontSelector.h |
| 2 | `css/CSSFontFaceSet.h` |  | css/CSSFontSelector.h |
| 2 | `css/CSSRegisteredCustomProperty.h` |  | style/StyleCustomPropertyRegistry.h |
| 2 | `css/CSSRuleList.h` |  | css/CSSStyleSheet.h |
| 2 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, style/HasSelectorFilter.h, style/StyleResolver.h |
| 2 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.h, style/RuleSet.h |
| 2 | `css/MediaList.h` |  | css/CSSStyleSheet.h |
| 2 | `css/StyleRule.h` |  | css/CSSFontSelector.h, style/RuleData.h, style/RuleSet.h, style/StyleCustomPropertyRegistry.h |
| 2 | `css/StyleSheet.h` |  | css/CSSStyleSheet.h |
| 2 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/MediaQueryParserContext.h, css/StyleSheetContents.h |
| 2 | `css/query/MediaQuery.h` |  | css/CSSStyleSheet.h, css/MediaList.h, css/StyleRule.h, css/query/MediaQueryEvaluator.h, style/RuleSet.h |
| 2 | `css/query/MediaQueryEvaluator.h` |  | style/ElementRuleCollector.h, style/StyleResolver.h |
| 2 | `style/AnchorPositionEvaluator.h` |  | style/StyleScope.h, style/TreeResolutionState.h |
| 2 | `style/ElementRuleCollector.h` |  | style/StyleResolver.h |
| 2 | `style/InspectorCSSOMWrappers.h` |  | style/StyleResolver.h |
| 2 | `style/MatchedDeclarationsCache.h` |  | style/StyleResolver.h |
| 2 | `style/PropertyCascade.h` |  | style/MatchResultCache.h, style/StyleBuilder.h, style/StyleBuilderState.h, style/StyleResolver.h |
| 2 | `style/ResolvedStyle.h` |  | style/MatchResultCache.h |
| 2 | `style/RuleData.h` |  | style/RuleSet.h |
| 2 | `style/RuleFeature.h` |  | style/RuleSet.h, style/StyleInvalidator.h, style/StyleScopeRuleSets.h |
| 2 | `style/StyleBuilderState.h` |  | style/StyleBuilder.h |
| 2 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h, style/StyleScope.h |
| 2 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/ElementRuleCollector.h, style/MatchResult.h, style/ScopedName.h, style/StyleScope.h |
| 2 | `style/StyleScopeRuleSets.h` |  | style/StyleResolver.h |
| 2 | `style/Styleable.h` |  | style/StyleScope.h |
| 2 | `style/TreeResolutionState.h` |  | style/StyleBuilderState.h, style/StyleResolver.h |
| 1 | `css/CSSCounterStyleRegistry.h` |  | style/StyleScope.cpp |
| 1 | `css/CSSFontSelector.h` |  | style/StyleScope.cpp |
| 1 | `css/CSSStyleSheet.h` |  | style/StyleScope.cpp |
| 1 | `css/StyleSheetContents.h` |  | style/StyleScope.cpp |
| 1 | `css/StyleSheetList.h` |  | style/StyleScope.cpp |
| 1 | `style/MatchResultCache.h` |  | style/StyleScope.cpp |
| 1 | `style/RuleSet.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/StyleBuilderState.h, style/StyleInvalidator.h, style/StyleResolver.h, style/StyleScope.cpp, style/StyleScopeRuleSets.h |
| 1 | `style/StyleBuilder.h` |  | style/StyleScope.cpp |
| 1 | `style/StyleCustomPropertyRegistry.h` |  | style/StyleScope.cpp |
| 1 | `style/StyleInvalidator.h` |  | style/StyleScope.cpp |
| 1 | `style/StyleResolver.h` |  | style/StyleScope.cpp |
| 1 | `style/StyleScope.h` |  | style/StyleScope.cpp |
| 0 | `style/StyleScope.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/WebAnimationTypes.h` ← 2 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (18)

- `dom/ActiveDOMObject.h` ← 1 file(s)
- `dom/ContainerNodeInlines.h` ← 1 file(s)
- `dom/Document.h` ← 2 file(s)
- `dom/DocumentInlines.h` ← 1 file(s)
- `dom/DocumentView.h` ← 1 file(s)
- `dom/Element.h` ← 5 file(s)
- `dom/ElementAncestorIteratorInlines.h` ← 1 file(s)
- `dom/ElementChildIteratorInlines.h` ← 1 file(s)
- `dom/ElementRareData.h` ← 1 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/ExceptionOr.h` ← 1 file(s)
- `dom/ExtensionStyleSheets.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/ProcessingInstruction.h` ← 1 file(s)
- `dom/PseudoElement.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)
- `dom/ScriptExecutionContext.h` ← 1 file(s)
- `dom/ShadowRoot.h` ← 1 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### html (5)

- `html/HTMLHeadElement.h` ← 1 file(s)
- `html/HTMLIFrameElement.h` ← 1 file(s)
- `html/HTMLLinkElement.h` ← 1 file(s)
- `html/HTMLSlotElement.h` ← 1 file(s)
- `html/HTMLStyleElement.h` ← 1 file(s)

### inspector (1)

- `inspector/InspectorInstrumentation.h` ← 1 file(s)

### loader (3)

- `loader/LoadedFromOpaqueSource.h` ← 2 file(s)
- `loader/cache/CachedResourceHandle.h` ← 1 file(s)
- `loader/cache/TrustedFonts.h` ← 1 file(s)

### page (3)

- `page/UserContentController.h` ← 1 file(s)
- `page/UserContentURLPattern.h` ← 1 file(s)
- `page/UserStyleSheet.h` ← 1 file(s)

### platform (23)

- `platform/CommonAtomStrings.h` ← 2 file(s)
- `platform/LayoutUnit.h` ← 3 file(s)
- `platform/Logging.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 1 file(s)
- `platform/Timer.h` ← 3 file(s)
- `platform/graphics/Color.h` ← 5 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)
- `platform/graphics/FloatSize.h` ← 1 file(s)
- `platform/graphics/Font.h` ← 1 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/FontSelectionAlgorithm.h` ← 1 file(s)
- `platform/graphics/FontSelector.h` ← 1 file(s)
- `platform/graphics/FontTaggedSettings.h` ← 2 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutRect.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 2 file(s)
- `platform/graphics/transforms/TransformOperation.h` ← 2 file(s)
- `platform/graphics/transforms/TransformationMatrix.h` ← 1 file(s)
- `platform/text/TextFlags.h` ← 2 file(s)
- `platform/text/WritingMode.h` ← 3 file(s)

### rendering (9)

- `rendering/RenderBoxInlines.h` ← 1 file(s)
- `rendering/RenderElementStyleInlines.h` ← 1 file(s)
- `rendering/RenderLayer.h` ← 1 file(s)
- `rendering/RenderObjectInlines.h` ← 1 file(s)
- `rendering/RenderView.h` ← 1 file(s)
- `rendering/style/PositionTryOrder.h` ← 1 file(s)
- `rendering/style/RenderStyle.h` ← 4 file(s)
- `rendering/style/RenderStyleConstants.h` ← 6 file(s)
- `rendering/style/StyleImage.h` ← 1 file(s)

### svg (1)

- `svg/SVGStyleElement.h` ← 1 file(s)

### wtf (64)

- `wtf/AbstractRefCounted.h` ← 1 file(s)
- `wtf/AbstractRefCountedAndCanMakeWeakPtr.h` ← 2 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 3 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 6 file(s)
- `wtf/CanMakeWeakPtr.h` ← 1 file(s)
- `wtf/CheckedPtr.h` ← 4 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 6 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 29 file(s)
- `wtf/Function.h` ← 1 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 15 file(s)
- `wtf/HashSet.h` ← 5 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/Identified.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 3 file(s)
- `wtf/Markable.h` ← 3 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 2 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/Observer.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 6 file(s)
- `wtf/PointerComparison.h` ← 1 file(s)
- `wtf/Ref.h` ← 6 file(s)
- `wtf/RefCounted.h` ← 6 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 3 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 10 file(s)
- `wtf/SetForScope.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 6 file(s)
- `wtf/TZoneMalloc.h` ← 12 file(s)
- `wtf/TZoneMallocInlines.h` ← 2 file(s)
- `wtf/TypeCasts.h` ← 4 file(s)
- `wtf/URL.h` ← 3 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 2 file(s)
- `wtf/Variant.h` ← 1 file(s)
- `wtf/Vector.h` ← 16 file(s)
- `wtf/WeakHashMap.h` ← 3 file(s)
- `wtf/WeakHashSet.h` ← 5 file(s)
- `wtf/WeakListHashSet.h` ← 1 file(s)
- `wtf/WeakPtr.h` ← 7 file(s)
- `wtf/WeakRef.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 15 file(s)
- `wtf/text/AtomStringHash.h` ← 9 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringHash.h` ← 3 file(s)
- `wtf/text/StringParsingBuffer.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextPosition.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 4 file(s)
- `wtf/text/WTFString.h` ← 5 file(s)

## Problems

### unresolved (6)

- `CSSPropertyNames.h` ← 6 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 10 file(s)
- `SVGElementTypeHelpers.h` ← 1 file(s)
- `Settings.h` ← 2 file(s)
- `WebKitFontFamilyNames.h` ← 1 file(s)
