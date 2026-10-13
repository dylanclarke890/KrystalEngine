# Port graph

Entries: `css/parser/CSSPropertyParserConsumer+Filter.cpp`

- Files to port: **128**
- Boundary `animation`: 1
- Boundary `cssjit`: 2
- Boundary `dom`: 5
- Boundary `external`: 1
- Boundary `loader`: 1
- Boundary `platform`: 17
- Boundary `rendering`: 3
- Boundary `wtf`: 46
- Unresolved: 3

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
| 5 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 5 | `css/values/primitives/CSSURLModifiers.h` |  | css/values/primitives/CSSURL.h |
| 5 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h |
| 5 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 4 | `css/CSSVariableData.h` |  | css/StyleRule.h |
| 4 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 4 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 4 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 4 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 4 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 4 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 4 | `css/values/color/CSSColor.h` |  | css/CSSColorValue.h, css/values/filter-effects/CSSDropShadowFunction.h, css/values/filter-effects/CSSHueRotateFunction.h |
| 4 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSURL.h` |  | css/values/filter-effects/CSSFilterReference.h |
| 4 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 4 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 4 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 4 | `style/PropertyAllowlist.h` |  | style/MatchResult.h, style/RuleData.h |
| 4 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 4 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 4 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 4 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/CSSColorValue.h` |  | css/CSSValuePool.h |
| 3 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, style/HasSelectorFilter.h |
| 3 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.h, style/RuleSet.h |
| 3 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 3 | `css/StyleRule.h` |  | style/RuleData.h, style/RuleSet.h |
| 3 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 3 | `css/calc/CSSCalcSymbolsAllowed.h` |  | css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 3 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 3 | `css/query/MediaQuery.h` |  | css/StyleRule.h, style/RuleSet.h |
| 3 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 3 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/filter-effects/CSSAppleInvertLightnessFunction.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSURLModifiers.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 3 | `css/values/filter-effects/CSSAppleInvertLightnessFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h |
| 3 | `css/values/filter-effects/CSSBlurFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSBrightnessFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSContrastFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSDropShadowFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSFilterReference.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSGrayscaleFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSHueRotateFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSInvertFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSOpacityFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSSaturateFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSSepiaFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 3 | `style/AnchorPositionEvaluator.h` |  | style/TreeResolutionState.h |
| 3 | `style/MatchResult.h` |  | style/PropertyCascade.h |
| 3 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h |
| 3 | `style/RuleData.h` |  | style/RuleSet.h |
| 3 | `style/RuleFeature.h` |  | style/RuleSet.h |
| 3 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 3 | `style/StyleRelations.h` |  | css/SelectorChecker.h |
| 3 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/MatchResult.h, style/ScopedName.h |
| 3 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h |
| 3 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 3 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 2 | `css/CSSProperty.h` |  | css/StyleProperties.h, css/parser/CSSPropertyParserConsumer+LengthDefinitions.h, css/parser/CSSPropertyParserState.h |
| 2 | `css/CSSValue.h` |  | css/CSSAppleColorFilterValue.h, css/CSSAttrValue.h, css/CSSColorValue.h, css/CSSFilterValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 2 | `css/CSSValuePool.h` |  | css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSPropertyParserState.h |
| 2 | `css/SelectorChecker.h` |  | style/MatchResult.h, style/StyleBuilderState.h |
| 2 | `css/StyleRuleType.h` |  | css/StyleRule.h, css/parser/CSSParserContext.h, css/parser/CSSPropertyParserState.h |
| 2 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 2 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h, css/parser/CSSPropertyParserOptions.h |
| 2 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSTokenizer.h |
| 2 | `css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+AngleDefinitions.h, css/parser/CSSPropertyParserConsumer+LengthDefinitions.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+NumberDefinitions.h, css/parser/CSSPropertyParserConsumer+PercentageDefinitions.h |
| 2 | `css/parser/CSSPropertyParserOptions.h` |  | css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 2 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 2 | `css/values/color/CSSColorType.h` |  | css/parser/CSSPropertyParserConsumer+Color.h |
| 2 | `css/values/filter-effects/CSSAppleColorFilter.h` |  | css/CSSAppleColorFilterValue.h |
| 2 | `css/values/filter-effects/CSSFilter.h` |  | css/CSSFilterValue.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes+Canonicalization.h` |  | css/parser/CSSPropertyParserConsumer+AngleDefinitions.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h, css/values/color/CSSColorDescriptors.h, css/values/filter-effects/CSSBlurFunction.h, css/values/filter-effects/CSSBrightnessFunction.h, css/values/filter-effects/CSSContrastFunction.h, css/values/filter-effects/CSSDropShadowFunction.h, css/values/filter-effects/CSSFilterFunctionDescriptor.h, css/values/filter-effects/CSSGrayscaleFunction.h, css/values/filter-effects/CSSHueRotateFunction.h, css/values/filter-effects/CSSInvertFunction.h, css/values/filter-effects/CSSOpacityFunction.h, css/values/filter-effects/CSSSaturateFunction.h, css/values/filter-effects/CSSSepiaFunction.h, css/values/primitives/CSSPrimitiveNumericTypes+Canonicalization.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 2 | `style/PropertyCascade.h` |  | style/StyleBuilderState.h |
| 2 | `style/RuleSet.h` |  | style/MatchResult.h, style/StyleBuilderState.h |
| 2 | `style/StyleForVisitedLink.h` |  | style/StyleBuilderState.h |
| 2 | `style/TreeResolutionState.h` |  | style/StyleBuilderState.h |
| 2 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h, style/values/filter-effects/StyleFilter.h, style/values/primitives/StylePrimitiveNumeric.h |
| 2 | `style/values/color/StyleCurrentColor.h` |  | css/values/filter-effects/CSSFilterFunctionDescriptor.h |
| 2 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/values/color/CSSColorDescriptors.h |
| 1 | `css/CSSAppleColorFilterValue.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp |
| 1 | `css/CSSFilterValue.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp |
| 1 | `css/CSSPrimitiveValue.h` |  | css/CSSValuePool.h, css/StylePropertiesInlines.h, css/parser/CSSParserToken.h, css/parser/CSSPropertyParserConsumer+Filter.cpp, css/parser/CSSPropertyParserConsumer+Ident.h |
| 1 | `css/CSSToLengthConversionData.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp, css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h, style/StyleBuilderState.h |
| 1 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/parser/CSSPropertyParserConsumer+AngleDefinitions.h, css/parser/CSSPropertyParserConsumer+Filter.cpp, css/parser/CSSPropertyParserConsumer+LengthDefinitions.h |
| 1 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h, css/parser/CSSPropertyParserConsumer+Filter.cpp, css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 1 | `css/parser/CSSPropertyParserConsumer+AngleDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Background.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Color.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Filter.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Ident.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+LengthDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+MetaConsumer.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+NumberDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+PercentageDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Primitives.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+URL.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp |
| 1 | `css/parser/CSSPropertyParserState.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp, css/parser/CSSPropertyParserConsumer+LengthDefinitions.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 1 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSPropertyParserConsumer+Filter.cpp |
| 1 | `css/values/filter-effects/CSSFilterFunctionDescriptor.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp |
| 1 | `style/StyleBuilderState.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp |
| 1 | `style/values/filter-effects/StyleFilter.h` |  | css/parser/CSSPropertyParserConsumer+Filter.cpp |
| 0 | `css/parser/CSSPropertyParserConsumer+Filter.cpp` |  | (entry) |

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

- `loader/LoadedFromOpaqueSource.h` ← 2 file(s)

### platform (17)

- `platform/CommonAtomStrings.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 3 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 1 file(s)
- `platform/graphics/Color.h` ← 5 file(s)
- `platform/graphics/ColorHash.h` ← 1 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/FontTaggedSettings.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 1 file(s)
- `platform/graphics/filters/FilterOperation.h` ← 1 file(s)
- `platform/graphics/filters/FilterOperations.h` ← 2 file(s)
- `platform/text/TextFlags.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 2 file(s)

### rendering (3)

- `rendering/style/PositionTryOrder.h` ← 1 file(s)
- `rendering/style/RenderStyle.h` ← 1 file(s)
- `rendering/style/RenderStyleConstants.h` ← 3 file(s)

### wtf (46)

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
- `wtf/Forward.h` ← 18 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 8 file(s)
- `wtf/HashSet.h` ← 1 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 2 file(s)
- `wtf/Markable.h` ← 2 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NeverDestroyed.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 6 file(s)
- `wtf/Ref.h` ← 5 file(s)
- `wtf/RefCounted.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 5 file(s)
- `wtf/StdLibExtras.h` ← 9 file(s)
- `wtf/TZoneMalloc.h` ← 7 file(s)
- `wtf/TypeCasts.h` ← 2 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/Vector.h` ← 6 file(s)
- `wtf/WeakHashMap.h` ← 1 file(s)
- `wtf/WeakHashSet.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 9 file(s)
- `wtf/text/AtomStringHash.h` ← 5 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 4 file(s)
- `wtf/text/TextStream.h` ← 4 file(s)
- `wtf/text/WTFString.h` ← 4 file(s)

## Problems

### unresolved (3)

- `CSSPropertyNames.h` ← 7 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 12 file(s)
