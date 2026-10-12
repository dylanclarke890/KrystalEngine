# Port graph

Entries: `css/calc/CSSCalcTree+Evaluation.cpp`, `css/calc/CSSCalcTree+Simplification.cpp`, `css/calc/CSSCalcTree+Parser.cpp`, `css/calc/CSSCalcValue.cpp`

- Files to port: **123**
- Boundary `animation`: 1
- Boundary `cssjit`: 2
- Boundary `dom`: 5
- Boundary `external`: 1
- Boundary `loader`: 1
- Boundary `platform`: 16
- Boundary `rendering`: 4
- Boundary `wtf`: 49
- Unresolved: 4

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 7 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h |
| 6 | `css/StyleProperties.h` |  | css/ImmutableStyleProperties.h, css/MutableStyleProperties.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 6 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 5 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/query/GenericMediaQueryEvaluator.h` |  | style/ContainerQueryEvaluator.h |
| 5 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h |
| 5 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 5 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 5 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h |
| 5 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 4 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 4 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 4 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 4 | `css/values/color/CSSColor.h` |  | css/CSSColorValue.h |
| 4 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 4 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 4 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 4 | `style/PropertyAllowlist.h` |  | style/MatchResult.h, style/RuleData.h |
| 3 | `css/CSSColorValue.h` |  | css/CSSValuePool.h |
| 3 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, style/HasSelectorFilter.h |
| 3 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.h, style/RuleSet.h |
| 3 | `css/CSSVariableData.h` |  | css/StyleRule.h |
| 3 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 3 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 3 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 3 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 3 | `css/query/MediaQuery.h` |  | css/StyleRule.h, style/RuleSet.h |
| 3 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 3 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 3 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 3 | `style/MatchResult.h` |  | style/PropertyCascade.h |
| 3 | `style/RuleData.h` |  | style/RuleSet.h |
| 3 | `style/RuleFeature.h` |  | style/RuleSet.h |
| 3 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 3 | `style/StyleRelations.h` |  | css/SelectorChecker.h |
| 3 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h |
| 3 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/MatchResult.h, style/ScopedName.h |
| 3 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h |
| 3 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h, style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 3 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 2 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 2 | `css/CSSProperty.h` |  | css/StyleProperties.h, css/parser/CSSParser.h, css/parser/CSSPropertyParserState.h |
| 2 | `css/CSSToLengthConversionData.h` |  | css/calc/CSSCalcTree+Evaluation.h, css/calc/CSSCalcTree+Simplification.h, css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h, style/StyleBuilderState.h, style/calc/StyleCalculationTree+Conversion.h |
| 2 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSColorValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 2 | `css/CSSValuePool.h` |  | css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSPropertyParserState.h |
| 2 | `css/CSSWideKeyword.h` |  | css/parser/CSSParserIdioms.h |
| 2 | `css/SelectorChecker.h` |  | style/MatchResult.h, style/StyleBuilderState.h |
| 2 | `css/StyleRule.h` |  | css/parser/CSSParser.h, style/RuleData.h, style/RuleSet.h |
| 2 | `css/StyleRuleType.h` |  | css/StyleRule.h, css/parser/CSSParserContext.h, css/parser/CSSPropertyParserState.h |
| 2 | `css/calc/CSSCalcSymbolsAllowed.h` |  | css/calc/CSSCalcTree+Parser.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 2 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 2 | `css/parser/CSSParserEnum.h` |  | css/parser/CSSParser.h |
| 2 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h, css/parser/CSSPropertyParserOptions.h |
| 2 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSTokenizer.h |
| 2 | `css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+NumberDefinitions.h |
| 2 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 2 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 2 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree+Evaluation.h, css/calc/CSSCalcTree+Parser.h, css/calc/CSSCalcTree+Serialization.h, css/calc/CSSCalcTree+Simplification.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/calc/StyleCalculationTree+Conversion.h, style/calc/StyleCalculationValue.h, style/values/primitives/StyleLengthResolution.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h, css/values/color/CSSColorDescriptors.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 2 | `style/PropertyCascade.h` |  | style/StyleBuilderState.h |
| 2 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h |
| 2 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 2 | `style/RuleSet.h` |  | style/MatchResult.h, style/StyleBuilderState.h |
| 2 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 2 | `style/StyleForVisitedLink.h` |  | style/StyleBuilderState.h |
| 2 | `style/TreeResolutionState.h` |  | style/StyleBuilderState.h |
| 2 | `style/calc/StyleCalculationTree.h` |  | css/calc/CSSCalcTree+Mappings.h, style/calc/StyleCalculationValue.h |
| 2 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 1 | `css/CSSPrimitiveValue.h` |  | css/CSSValuePool.h, css/StylePropertiesInlines.h, css/calc/CSSCalcTree+Simplification.cpp, css/parser/CSSParserToken.h, css/parser/CSSPropertyParserConsumer+Ident.h |
| 1 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree+NumericIdentity.h, css/calc/CSSCalcTree+Parser.cpp, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 1 | `css/calc/CSSCalcExecutor.h` |  | css/calc/CSSCalcTree+Evaluation.cpp, css/calc/CSSCalcTree+Simplification.cpp |
| 1 | `css/calc/CSSCalcOperator.h` |  | css/calc/CSSCalcExecutor.h, css/calc/CSSCalcTree+Parser.cpp, style/calc/StyleCalculationTree.h |
| 1 | `css/calc/CSSCalcRandomCachingKey.h` |  | css/calc/CSSCalcTree+Evaluation.cpp, css/calc/CSSCalcTree+Simplification.cpp |
| 1 | `css/calc/CSSCalcSymbolTable.h` |  | css/calc/CSSCalcTree+Evaluation.cpp, css/calc/CSSCalcTree+Parser.cpp, css/calc/CSSCalcTree+Simplification.cpp, css/calc/CSSCalcTree+Simplification.h, css/calc/CSSCalcValue.cpp, style/calc/StyleCalculationTree+Conversion.h, style/values/StyleValueTypes.h |
| 1 | `css/calc/CSSCalcTree+ComputedStyleDependencies.h` |  | css/calc/CSSCalcValue.cpp |
| 1 | `css/calc/CSSCalcTree+Copy.h` |  | css/calc/CSSCalcTree+Simplification.cpp |
| 1 | `css/calc/CSSCalcTree+Evaluation.h` |  | css/calc/CSSCalcTree+Evaluation.cpp, css/calc/CSSCalcTree+Simplification.cpp, css/calc/CSSCalcValue.cpp |
| 1 | `css/calc/CSSCalcTree+Mappings.h` |  | css/calc/CSSCalcTree+Evaluation.cpp, css/calc/CSSCalcTree+Simplification.cpp |
| 1 | `css/calc/CSSCalcTree+NumericIdentity.h` |  | css/calc/CSSCalcTree+Simplification.cpp |
| 1 | `css/calc/CSSCalcTree+Parser.h` |  | css/calc/CSSCalcTree+Parser.cpp, css/calc/CSSCalcValue.cpp |
| 1 | `css/calc/CSSCalcTree+Serialization.h` |  | css/calc/CSSCalcTree+Parser.cpp, css/calc/CSSCalcValue.cpp |
| 1 | `css/calc/CSSCalcTree+Simplification.h` |  | css/calc/CSSCalcTree+Evaluation.cpp, css/calc/CSSCalcTree+Parser.cpp, css/calc/CSSCalcTree+Simplification.cpp, css/calc/CSSCalcValue.cpp |
| 1 | `css/calc/CSSCalcTree+Traversal.h` |  | css/calc/CSSCalcTree+Simplification.cpp |
| 1 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcRandomCachingKey.h, css/calc/CSSCalcTree+Evaluation.cpp, css/calc/CSSCalcTree+Mappings.h, css/calc/CSSCalcTree+NumericIdentity.h, css/calc/CSSCalcTree+Parser.cpp, css/calc/CSSCalcTree+Simplification.cpp, css/calc/CSSCalcTree+Traversal.h, css/calc/CSSCalcValue.h |
| 1 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h, css/calc/CSSCalcValue.cpp, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 1 | `css/parser/CSSParser.h` |  | css/calc/CSSCalcValue.cpp |
| 1 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/calc/CSSCalcTree+Parser.cpp, css/parser/CSSParserIdioms.h |
| 1 | `css/parser/CSSParserIdioms.h` |  | css/calc/CSSCalcTree+Parser.cpp |
| 1 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h, css/calc/CSSCalcTree+Parser.cpp, css/calc/CSSCalcValue.cpp, css/parser/CSSParser.h, css/parser/CSSParserTokenRangeGuard.h, css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 1 | `css/parser/CSSParserTokenRangeGuard.h` |  | css/calc/CSSCalcTree+Parser.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Ident.h` |  | css/calc/CSSCalcTree+Parser.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+MetaConsumer.h` |  | css/calc/CSSCalcTree+Parser.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+NumberDefinitions.h` |  | css/calc/CSSCalcTree+Parser.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Primitives.h` |  | css/calc/CSSCalcTree+Parser.cpp |
| 1 | `css/parser/CSSPropertyParserOptions.h` |  | css/calc/CSSCalcTree+Parser.h, css/calc/CSSCalcValue.cpp, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 1 | `css/parser/CSSPropertyParserState.h` |  | css/calc/CSSCalcTree+Parser.cpp, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 1 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/calc/CSSCalcValue.cpp, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 1 | `css/values/CSSSerializationContext.h` |  | css/calc/CSSCalcTree+Parser.cpp, css/calc/CSSCalcValue.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/calc/CSSCalcTree+Parser.cpp, css/calc/CSSCalcTree+Simplification.cpp, css/calc/CSSCalcValue.cpp, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 1 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/calc/CSSCalcTree+Evaluation.cpp, css/calc/CSSCalcTree+Simplification.cpp, css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 1 | `style/AnchorPositionEvaluator.h` |  | css/calc/CSSCalcTree+Evaluation.cpp, css/calc/CSSCalcTree+Parser.cpp, css/calc/CSSCalcTree+Simplification.cpp, style/TreeResolutionState.h |
| 1 | `style/StyleBuilderState.h` |  | css/calc/CSSCalcTree+Evaluation.cpp, css/calc/CSSCalcTree+Simplification.cpp |
| 1 | `style/calc/StyleCalculationTree+Conversion.h` |  | css/calc/CSSCalcValue.cpp |
| 1 | `style/calc/StyleCalculationValue.h` |  | css/calc/CSSCalcValue.cpp |
| 1 | `style/values/primitives/StyleLengthResolution.h` |  | css/calc/CSSCalcTree+Simplification.cpp, css/calc/CSSCalcValue.cpp |
| 1 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/calc/CSSCalcValue.cpp, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/values/color/CSSColorDescriptors.h |
| 0 | `css/calc/CSSCalcTree+Evaluation.cpp` |  | (entry) |
| 0 | `css/calc/CSSCalcTree+Parser.cpp` |  | (entry) |
| 0 | `css/calc/CSSCalcTree+Simplification.cpp` |  | (entry) |
| 0 | `css/calc/CSSCalcValue.cpp` |  | (entry) |

## Boundaries

### animation (1)

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

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)

### platform (16)

- `platform/CommonAtomStrings.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 3 file(s)
- `platform/Logging.h` ← 2 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 1 file(s)
- `platform/graphics/Color.h` ← 3 file(s)
- `platform/graphics/ColorHash.h` ← 1 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/FontTaggedSettings.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 1 file(s)
- `platform/text/TextFlags.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 2 file(s)

### rendering (4)

- `rendering/style/PositionTryOrder.h` ← 1 file(s)
- `rendering/style/RenderStyle+GettersInlines.h` ← 1 file(s)
- `rendering/style/RenderStyle.h` ← 3 file(s)
- `rendering/style/RenderStyleConstants.h` ← 3 file(s)

### wtf (49)

- `wtf/ASCIICType.h` ← 1 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 3 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 7 file(s)
- `wtf/CheckedPtr.h` ← 2 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 6 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 19 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 2 file(s)
- `wtf/HashMap.h` ← 9 file(s)
- `wtf/HashSet.h` ← 1 file(s)
- `wtf/HashTraits.h` ← 1 file(s)
- `wtf/Hasher.h` ← 3 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 2 file(s)
- `wtf/Markable.h` ← 2 file(s)
- `wtf/MathExtras.h` ← 5 file(s)
- `wtf/NeverDestroyed.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 4 file(s)
- `wtf/Ref.h` ← 7 file(s)
- `wtf/RefCounted.h` ← 2 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 4 file(s)
- `wtf/SortedArrayMap.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 9 file(s)
- `wtf/TZoneMalloc.h` ← 8 file(s)
- `wtf/TypeCasts.h` ← 2 file(s)
- `wtf/URL.h` ← 1 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/Vector.h` ← 8 file(s)
- `wtf/WeakHashMap.h` ← 1 file(s)
- `wtf/WeakHashSet.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 10 file(s)
- `wtf/text/AtomStringHash.h` ← 5 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 2 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 5 file(s)
- `wtf/text/WTFString.h` ← 5 file(s)

## Problems

### unresolved (4)

- `CSSPropertyNames.h` ← 8 file(s)
- `CSSPropertyParsing.h` ← 1 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 14 file(s)
