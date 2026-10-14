# Port graph

Entries: `style/RuleSetBuilder.cpp`

- Files to port: **130**
- Boundary `animation`: 1
- Boundary `cssjit`: 2
- Boundary `dom`: 8
- Boundary `external`: 1
- Boundary `loader`: 5
- Boundary `platform`: 22
- Boundary `rendering`: 4
- Boundary `wtf`: 62
- Unresolved: 5

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 7 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 7 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h |
| 7 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 7 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 6 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 6 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 6 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 6 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h, style/values/color/StyleResolvedColor.h |
| 6 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 6 | `css/values/primitives/CSSURLModifiers.h` |  | css/values/primitives/CSSURL.h |
| 6 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 6 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 6 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h |
| 5 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 5 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 5 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 5 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 5 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 5 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSURLModifiers.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 5 | `css/values/color/CSSColor.h` |  | style/values/color/StyleColor.h |
| 5 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h, style/values/color/StyleColor.h |
| 5 | `css/values/color/CSSColorType.h` |  | style/values/color/StyleColor.h |
| 5 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
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
| 4 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 4 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h, style/StyleCustomProperty.h |
| 4 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 4 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 4 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 4 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 4 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h, style/StyleCustomProperty.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 4 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleRelations.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h, style/PseudoElementRequest.h |
| 4 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h, style/values/images/StyleImageWrapper.h, style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleURL.h, style/values/transforms/StyleTransformFunction.h, style/values/transforms/functions/StyleTransformFunctionBase.h |
| 4 | `style/values/color/StyleColor.h` |  | style/StyleCustomProperty.h |
| 4 | `style/values/images/StyleImageWrapper.h` |  | style/StyleCustomProperty.h |
| 4 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/StyleCustomProperty.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 4 | `style/values/primitives/StyleURL.h` |  | style/StyleCustomProperty.h |
| 4 | `style/values/transforms/StyleTransformFunction.h` |  | style/StyleCustomProperty.h |
| 3 | `css/CSSCounterStyleDescriptors.h` |  | css/CSSCounterStyle.h |
| 3 | `css/CSSPrimitiveValue.h` |  | css/StylePropertiesInlines.h, css/parser/CSSParserToken.h |
| 3 | `css/CSSProperty.h` |  | css/StyleProperties.h |
| 3 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 3 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h |
| 3 | `css/CSSVariableData.h` |  | css/CSSRegisteredCustomProperty.h, css/StyleRule.h, style/StyleCustomProperty.h |
| 3 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 3 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 3 | `css/SelectorChecker.h` |  | style/ElementRuleCollector.h, style/MatchResult.h |
| 3 | `css/StyleRuleType.h` |  | css/CSSRule.h, css/StyleRule.h, css/parser/CSSParserContext.h |
| 3 | `css/parser/CSSCustomPropertySyntax.h` |  | css/CSSRegisteredCustomProperty.h |
| 3 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h, css/parser/CSSSelectorParserContext.h |
| 3 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 3 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 3 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 3 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 3 | `style/MatchResult.h` |  | style/ElementRuleCollector.h, style/MatchedDeclarationsCache.h, style/PropertyCascade.h |
| 3 | `style/PropertyAllowlist.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleData.h |
| 3 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h, style/PseudoElementRequest.h, style/Styleable.h |
| 3 | `style/PseudoElementRequest.h` |  | style/ElementRuleCollector.h |
| 3 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/RuleData.h` |  | style/RuleSet.h |
| 3 | `style/RuleFeature.h` |  | style/RuleSet.h, style/StyleScopeRuleSets.h |
| 3 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/StyleCustomProperty.h` |  | css/CSSRegisteredCustomProperty.h |
| 3 | `style/UserAgentStyle.h` |  | style/StyleScopeRuleSets.h |
| 2 | `css/CSSCounterStyle.h` |  | css/CSSCounterStyleRegistry.h, css/CSSCounterStyleRule.h |
| 2 | `css/CSSFontFace.h` |  | css/CSSFontFaceSet.h, css/CSSFontSelector.h |
| 2 | `css/CSSFontFaceSet.h` |  | css/CSSFontSelector.h |
| 2 | `css/CSSRegisteredCustomProperty.h` |  | style/StyleCustomPropertyRegistry.h |
| 2 | `css/CSSRule.h` |  | css/CSSCounterStyleRule.h, css/CSSKeyframesRule.h, css/CSSPositionTryRule.h, css/CSSViewTransitionRule.h |
| 2 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, css/parser/MutableCSSSelector.h, style/HasSelectorFilter.h, style/StyleResolver.h |
| 2 | `css/CSSSelectorList.h` |  | css/StyleRule.h, css/parser/CSSSelectorParser.h, style/RuleFeature.h, style/RuleSet.h |
| 2 | `css/StyleProperties.h` |  | css/CSSCounterStyleRule.h, css/CSSPositionTryRule.h, css/CSSViewTransitionRule.h, css/ImmutableStyleProperties.h, css/MutableStyleProperties.h |
| 2 | `css/StylePropertiesInlines.h` |  | css/CSSPositionTryRule.h, style/MatchResult.h |
| 2 | `css/StyleRule.h` |  | css/CSSCounterStyleRule.h, css/CSSFontSelector.h, css/CSSKeyframesRule.h, css/CSSPositionTryRule.h, css/CSSViewTransitionRule.h, css/StyleRuleImport.h, style/RuleData.h, style/RuleSet.h, style/StyleCustomPropertyRegistry.h |
| 2 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/StyleSheetContents.h, css/parser/CSSSelectorParser.h |
| 2 | `css/parser/CSSParserEnum.h` |  | css/CSSRule.h, css/parser/CSSSelectorParser.h, style/RuleSetBuilder.h |
| 2 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h, css/parser/CSSSelectorParser.h |
| 2 | `css/parser/CSSSelectorParserContext.h` |  | css/parser/CSSSelectorParser.h |
| 2 | `css/query/GenericMediaQueryEvaluator.h` |  | css/query/MediaQueryEvaluator.h, style/ContainerQueryEvaluator.h |
| 2 | `css/query/MediaQuery.h` |  | css/StyleRule.h, css/StyleRuleImport.h, css/query/MediaQueryEvaluator.h, style/RuleSet.h, style/RuleSetBuilder.h |
| 2 | `style/AnchorPositionEvaluator.h` |  | style/StyleScope.h, style/TreeResolutionState.h |
| 2 | `style/ElementRuleCollector.h` |  | style/StyleResolver.h |
| 2 | `style/InspectorCSSOMWrappers.h` |  | style/StyleResolver.h |
| 2 | `style/MatchedDeclarationsCache.h` |  | style/StyleResolver.h |
| 2 | `style/PropertyCascade.h` |  | style/StyleResolver.h |
| 2 | `style/RuleSet.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleSetBuilder.h, style/StyleResolver.h, style/StyleScopeRuleSets.h |
| 2 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h, style/StyleScope.h |
| 2 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/ElementRuleCollector.h, style/MatchResult.h, style/ScopedName.h, style/StyleScope.h |
| 2 | `style/StyleScopeRuleSets.h` |  | style/StyleResolver.h |
| 2 | `style/Styleable.h` |  | style/StyleScope.h |
| 2 | `style/TreeResolutionState.h` |  | style/StyleResolver.h |
| 1 | `css/CSSCounterStyleRegistry.h` |  | style/RuleSetBuilder.cpp |
| 1 | `css/CSSCounterStyleRule.h` |  | style/RuleSetBuilder.cpp |
| 1 | `css/CSSFontSelector.h` |  | style/RuleSetBuilder.cpp |
| 1 | `css/CSSKeyframesRule.h` |  | style/RuleSetBuilder.cpp |
| 1 | `css/CSSPositionTryRule.h` |  | style/RuleSetBuilder.cpp |
| 1 | `css/CSSViewTransitionRule.h` |  | style/RuleSetBuilder.cpp |
| 1 | `css/StyleRuleImport.h` |  | style/RuleSetBuilder.cpp |
| 1 | `css/StyleSheetContents.h` |  | css/parser/CSSSelectorParser.h, style/RuleSetBuilder.cpp |
| 1 | `css/parser/CSSSelectorParser.h` |  | style/RuleSetBuilder.cpp |
| 1 | `css/parser/MutableCSSSelector.h` |  | css/parser/CSSSelectorParser.h, style/RuleSetBuilder.cpp |
| 1 | `css/query/MediaQueryEvaluator.h` |  | style/ElementRuleCollector.h, style/RuleSetBuilder.cpp, style/StyleResolver.h |
| 1 | `style/RuleSetBuilder.h` |  | style/RuleSetBuilder.cpp |
| 1 | `style/StyleCustomPropertyRegistry.h` |  | style/RuleSetBuilder.cpp |
| 1 | `style/StyleResolver.h` |  | style/RuleSetBuilder.cpp |
| 1 | `style/StyleScope.h` |  | style/RuleSetBuilder.cpp |
| 0 | `style/RuleSetBuilder.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/WebAnimationTypes.h` ← 2 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (8)

- `dom/ActiveDOMObject.h` ← 1 file(s)
- `dom/Document.h` ← 2 file(s)
- `dom/Element.h` ← 4 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/PseudoElement.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)
- `dom/ScriptExecutionContext.h` ← 1 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### loader (5)

- `loader/LoadedFromOpaqueSource.h` ← 2 file(s)
- `loader/cache/CachedCSSStyleSheet.h` ← 1 file(s)
- `loader/cache/CachedResourceHandle.h` ← 2 file(s)
- `loader/cache/CachedStyleSheetClient.h` ← 1 file(s)
- `loader/cache/TrustedFonts.h` ← 1 file(s)

### platform (22)

- `platform/CommonAtomStrings.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 3 file(s)
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
- `platform/graphics/FontTaggedSettings.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutRect.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 2 file(s)
- `platform/graphics/transforms/TransformOperation.h` ← 2 file(s)
- `platform/graphics/transforms/TransformationMatrix.h` ← 1 file(s)
- `platform/text/TextFlags.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 3 file(s)

### rendering (4)

- `rendering/style/PositionTryOrder.h` ← 1 file(s)
- `rendering/style/RenderStyle.h` ← 2 file(s)
- `rendering/style/RenderStyleConstants.h` ← 6 file(s)
- `rendering/style/StyleImage.h` ← 1 file(s)

### wtf (62)

- `wtf/AbstractRefCountedAndCanMakeWeakPtr.h` ← 2 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 2 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 6 file(s)
- `wtf/CanMakeWeakPtr.h` ← 1 file(s)
- `wtf/CheckedPtr.h` ← 3 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/CryptographicallyRandomNumber.h` ← 1 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 6 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 26 file(s)
- `wtf/Function.h` ← 1 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 2 file(s)
- `wtf/HashMap.h` ← 14 file(s)
- `wtf/HashSet.h` ← 5 file(s)
- `wtf/Hasher.h` ← 3 file(s)
- `wtf/Identified.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 3 file(s)
- `wtf/Markable.h` ← 3 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/Observer.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 6 file(s)
- `wtf/PointerComparison.h` ← 1 file(s)
- `wtf/Ref.h` ← 6 file(s)
- `wtf/RefCounted.h` ← 3 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 4 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 9 file(s)
- `wtf/SetForScope.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 6 file(s)
- `wtf/TZoneMalloc.h` ← 12 file(s)
- `wtf/TypeCasts.h` ← 6 file(s)
- `wtf/URL.h` ← 3 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 2 file(s)
- `wtf/Variant.h` ← 1 file(s)
- `wtf/Vector.h` ← 13 file(s)
- `wtf/WeakHashMap.h` ← 2 file(s)
- `wtf/WeakHashSet.h` ← 4 file(s)
- `wtf/WeakListHashSet.h` ← 1 file(s)
- `wtf/WeakPtr.h` ← 4 file(s)
- `wtf/WeakRef.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 18 file(s)
- `wtf/text/AtomStringHash.h` ← 9 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringHash.h` ← 3 file(s)
- `wtf/text/StringParsingBuffer.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 4 file(s)
- `wtf/text/WTFString.h` ← 5 file(s)

## Problems

### unresolved (5)

- `CSSPropertyNames.h` ← 6 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 10 file(s)
- `Settings.h` ← 1 file(s)
- `WebKitFontFamilyNames.h` ← 1 file(s)
