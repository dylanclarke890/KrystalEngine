# Port graph

Entries: `style/StyleCustomProperty.cpp`

- Files to port: **116**
- Boundary `animation`: 1
- Boundary `cssjit`: 2
- Boundary `dom`: 5
- Boundary `external`: 2
- Boundary `loader`: 1
- Boundary `platform`: 19
- Boundary `rendering`: 4
- Boundary `wtf`: 51
- Unresolved: 3

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 10 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 9 | `css/CSSProperty.h` |  | css/StyleProperties.h |
| 8 | `css/StyleProperties.h` |  | css/ImmutableStyleProperties.h, css/MutableStyleProperties.h |
| 8 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 7 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 7 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 7 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 7 | `css/query/GenericMediaQueryEvaluator.h` |  | style/ContainerQueryEvaluator.h |
| 7 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h |
| 7 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 6 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 6 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 6 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 6 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 6 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 6 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 6 | `style/PropertyAllowlist.h` |  | style/MatchResult.h, style/RuleData.h |
| 6 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 6 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 5 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, style/HasSelectorFilter.h |
| 5 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.h, style/RuleSet.h |
| 5 | `css/StyleRule.h` |  | style/RuleData.h, style/RuleSet.h |
| 5 | `css/query/MediaQuery.h` |  | css/StyleRule.h, style/RuleSet.h |
| 5 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 5 | `style/AnchorPositionEvaluator.h` |  | style/TreeResolutionState.h |
| 5 | `style/MatchResult.h` |  | style/PropertyCascade.h |
| 5 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h |
| 5 | `style/RuleData.h` |  | style/RuleSet.h |
| 5 | `style/RuleFeature.h` |  | style/RuleSet.h |
| 5 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 5 | `style/StyleRelations.h` |  | css/SelectorChecker.h |
| 5 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/MatchResult.h, style/ScopedName.h |
| 5 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h |
| 4 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/SelectorChecker.h` |  | style/MatchResult.h, style/StyleBuilderState.h |
| 4 | `css/StyleRuleType.h` |  | css/StyleRule.h, css/parser/CSSParserContext.h |
| 4 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
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
| 4 | `style/PropertyCascade.h` |  | style/StyleBuilderState.h |
| 4 | `style/RuleSet.h` |  | style/MatchResult.h, style/StyleBuilderState.h |
| 4 | `style/StyleForVisitedLink.h` |  | style/StyleBuilderState.h |
| 4 | `style/TreeResolutionState.h` |  | style/StyleBuilderState.h |
| 3 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h, style/StyleBuilderState.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 3 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/calc/CSSCalcOperator.h` |  | style/calc/StyleCalculationTree.h |
| 3 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 3 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 3 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h |
| 3 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 3 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 3 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericTypes+CSSValueCreation.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSURLModifiers.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 3 | `css/values/color/CSSColor.h` |  | css/CSSColorValue.h, style/values/color/StyleColor.h |
| 3 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h, style/values/color/StyleColor.h |
| 3 | `css/values/color/CSSColorType.h` |  | style/values/color/StyleColor.h |
| 3 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericTypes+Canonicalization.h` |  | style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.h` |  | style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, css/values/primitives/CSSPrimitiveNumericTypes+CSSValueCreation.h, css/values/primitives/CSSPrimitiveNumericTypes+Canonicalization.h, css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 3 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 3 | `css/values/primitives/CSSURL.h` |  | style/values/primitives/StyleURL.h |
| 3 | `style/StyleBuilderState.h` |  | style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
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
| 2 | `css/CSSColorValue.h` |  | css/CSSValuePool.h |
| 2 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSColorValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/CSSValueList.h, css/CSSVariableReferenceValue.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h, style/StyleCustomProperty.h |
| 2 | `css/CSSVariableData.h` |  | css/CSSVariableReferenceValue.h, css/StyleRule.h, style/StyleCustomProperty.h |
| 2 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 2 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 2 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h, css/CSSVariableReferenceValue.h |
| 2 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 2 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h, style/StyleCustomProperty.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/calc/StyleCalculationValue.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes+CSSValueCreation.h` |  | style/values/primitives/StylePrimitiveNumericTypes+CSSValueCreation.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h` |  | style/values/primitives/StylePrimitiveNumericTypes+Serialization.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 2 | `style/calc/StyleCalculationTree.h` |  | style/calc/StyleCalculationValue.h |
| 2 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h, style/values/images/StyleImageWrapper.h, style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StylePrimitiveNumericTypes+CSSValueCreation.h, style/values/primitives/StylePrimitiveNumericTypes+Serialization.h, style/values/primitives/StyleURL.h, style/values/transforms/StyleTransformFunction.h, style/values/transforms/functions/StyleTransformFunctionBase.h |
| 2 | `style/values/color/StyleColor.h` |  | style/StyleCustomProperty.h |
| 2 | `style/values/images/StyleImageWrapper.h` |  | style/StyleCustomProperty.h |
| 2 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/StyleCustomProperty.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `style/values/primitives/StylePrimitiveNumericTypes+Conversions.h` |  | style/values/primitives/StylePrimitiveNumericTypes+CSSValueCreation.h, style/values/primitives/StylePrimitiveNumericTypes+Serialization.h |
| 2 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, style/values/primitives/StylePrimitiveNumericTypes+CSSValueCreation.h, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h, style/values/primitives/StylePrimitiveNumericTypes+Serialization.h |
| 2 | `style/values/primitives/StyleURL.h` |  | style/StyleCustomProperty.h |
| 2 | `style/values/transforms/StyleTransformFunction.h` |  | style/StyleCustomProperty.h |
| 1 | `css/CSSPrimitiveValue.h` |  | css/CSSValuePool.h, css/StylePropertiesInlines.h, css/parser/CSSParserToken.h, css/values/primitives/CSSPrimitiveNumericTypes+CSSValueCreation.h, style/StyleCustomProperty.cpp |
| 1 | `css/CSSValueList.h` |  | style/StyleCustomProperty.cpp |
| 1 | `css/CSSValuePool.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+CSSValueCreation.h, style/StyleCustomProperty.cpp |
| 1 | `css/CSSVariableReferenceValue.h` |  | style/StyleCustomProperty.cpp |
| 1 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h, style/StyleCustomProperty.cpp, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 1 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h, style/StyleCustomProperty.cpp |
| 1 | `css/values/CSSSerializationContext.h` |  | style/StyleCustomProperty.cpp, style/values/primitives/StylePrimitiveNumericTypes+Serialization.h |
| 1 | `style/StyleCustomProperty.h` |  | style/StyleCustomProperty.cpp |
| 1 | `style/calc/StyleCalculationValue.h` |  | style/StyleCustomProperty.cpp, style/values/primitives/StylePrimitiveNumericTypes+Conversions.h |
| 1 | `style/values/primitives/StylePrimitiveNumericTypes+CSSValueCreation.h` |  | style/StyleCustomProperty.cpp |
| 1 | `style/values/primitives/StylePrimitiveNumericTypes+Serialization.h` |  | style/StyleCustomProperty.cpp |
| 0 | `style/StyleCustomProperty.cpp` |  | (entry) |

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

### external (2)

- `pal/text/TextEncoding.h` ← 1 file(s)
- `unicode/umachine.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 2 file(s)

### platform (19)

- `platform/CommonAtomStrings.h` ← 1 file(s)
- `platform/FloatConversion.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 3 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 1 file(s)
- `platform/graphics/Color.h` ← 5 file(s)
- `platform/graphics/ColorHash.h` ← 1 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)
- `platform/graphics/FloatSize.h` ← 1 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/FontTaggedSettings.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 1 file(s)
- `platform/graphics/transforms/TransformOperation.h` ← 2 file(s)
- `platform/graphics/transforms/TransformationMatrix.h` ← 1 file(s)
- `platform/text/TextFlags.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 2 file(s)

### rendering (4)

- `rendering/style/PositionTryOrder.h` ← 1 file(s)
- `rendering/style/RenderStyle.h` ← 2 file(s)
- `rendering/style/RenderStyleConstants.h` ← 3 file(s)
- `rendering/style/StyleImage.h` ← 1 file(s)

### wtf (51)

- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 3 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 6 file(s)
- `wtf/CheckedPtr.h` ← 2 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 6 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 21 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 8 file(s)
- `wtf/HashSet.h` ← 1 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 2 file(s)
- `wtf/MallocSpan.h` ← 1 file(s)
- `wtf/Markable.h` ← 3 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NeverDestroyed.h` ← 2 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 5 file(s)
- `wtf/PointerComparison.h` ← 2 file(s)
- `wtf/Ref.h` ← 8 file(s)
- `wtf/RefCounted.h` ← 4 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 3 file(s)
- `wtf/StdLibExtras.h` ← 6 file(s)
- `wtf/TZoneMalloc.h` ← 9 file(s)
- `wtf/TZoneMallocInlines.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 3 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 1 file(s)
- `wtf/Variant.h` ← 1 file(s)
- `wtf/Vector.h` ← 8 file(s)
- `wtf/WeakHashMap.h` ← 1 file(s)
- `wtf/WeakHashSet.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 10 file(s)
- `wtf/text/AtomStringHash.h` ← 5 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 4 file(s)
- `wtf/text/WTFString.h` ← 6 file(s)

## Problems

### unresolved (3)

- `CSSPropertyNames.h` ← 7 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 12 file(s)
