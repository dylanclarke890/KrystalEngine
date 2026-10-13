# Port graph

Entries: `css/parser/CSSPropertyParserConsumer+Timeline.cpp`

- Files to port: **76**
- Boundary `animation`: 1
- Boundary `external`: 2
- Boundary `loader`: 1
- Boundary `platform`: 7
- Boundary `rendering`: 1
- Boundary `wtf`: 38
- Unresolved: 3

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 7 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h |
| 6 | `css/calc/CSSCalcOperator.h` |  | style/calc/StyleCalculationTree.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 5 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h |
| 5 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 5 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 5 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h |
| 5 | `style/calc/StyleCalculationTree.h` |  | style/calc/StyleCalculationValue.h |
| 5 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 5 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 5 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 4 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 4 | `css/values/color/CSSColor.h` |  | css/CSSColorValue.h |
| 4 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/calc/StyleCalculationValue.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `style/calc/StyleCalculationValue.h` |  | style/values/primitives/StyleLengthWrapperData.h |
| 4 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 4 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 4 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StyleLengthWrapperData.h, style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/CSSColorValue.h` |  | css/CSSValuePool.h |
| 3 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 3 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 3 | `css/calc/CSSCalcSymbolsAllowed.h` |  | css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 3 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 3 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 3 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 3 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 3 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h, style/values/primitives/StyleLengthWrapper.h |
| 3 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 3 | `style/values/primitives/StyleLengthWrapperData.h` |  | style/values/primitives/StyleLengthWrapper.h |
| 3 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/values/color/CSSColorDescriptors.h, style/values/primitives/StyleLengthWrapper.h |
| 2 | `css/CSSPrimitiveValue.h` |  | css/CSSScrollValue.h, css/CSSValuePool.h, css/CSSViewValue.h, css/parser/CSSParserToken.h, css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h, css/parser/CSSPropertyParserConsumer+Ident.h |
| 2 | `css/CSSProperty.h` |  | css/parser/CSSPropertyParserConsumer+LengthDefinitions.h, css/parser/CSSPropertyParserState.h |
| 2 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSColorValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/CSSValueList.h, css/CSSValuePair.h, css/values/CSSValueTypes.h |
| 2 | `css/CSSValueList.h` |  | css/parser/CSSPropertyParserConsumer+List.h |
| 2 | `css/CSSValuePool.h` |  | css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSPropertyParserState.h |
| 2 | `css/StyleRuleType.h` |  | css/parser/CSSParserContext.h, css/parser/CSSPropertyParserState.h |
| 2 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h, css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 2 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h, css/parser/CSSPropertyParserOptions.h |
| 2 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaResolver.h, css/parser/CSSTokenizer.h |
| 2 | `css/parser/CSSPropertyParserConsumer+LengthDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+LengthPercentageDefinitions.h |
| 2 | `css/parser/CSSPropertyParserConsumer+MetaConsumer.h` |  | css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h |
| 2 | `css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+LengthDefinitions.h, css/parser/CSSPropertyParserConsumer+LengthPercentageDefinitions.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaResolver.h |
| 2 | `css/parser/CSSPropertyParserConsumer+MetaResolver.h` |  | css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h |
| 2 | `css/parser/CSSPropertyParserConsumer+Primitives.h` |  | css/parser/CSSPropertyParserConsumer+List.h |
| 2 | `css/parser/CSSPropertyParserOptions.h` |  | css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h, css/parser/CSSPropertyParserConsumer+MetaResolver.h |
| 2 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h, css/parser/CSSPropertyParserConsumer+MetaResolver.h, css/values/color/CSSColorDescriptors.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h, css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 2 | `style/values/StyleValueTypes.h` |  | style/values/animations/StyleSingleAnimationRange.h, style/values/primitives/StylePrimitiveNumeric.h |
| 2 | `style/values/animations/StyleSingleAnimationRangeName.h` |  | style/values/animations/StyleSingleAnimationRange.h |
| 2 | `style/values/primitives/StyleLengthWrapper.h` |  | style/values/animations/StyleSingleAnimationRange.h |
| 1 | `css/CSSScrollValue.h` |  | css/parser/CSSPropertyParserConsumer+Timeline.cpp |
| 1 | `css/CSSValuePair.h` |  | css/parser/CSSPropertyParserConsumer+Timeline.cpp |
| 1 | `css/CSSViewValue.h` |  | css/parser/CSSPropertyParserConsumer+Timeline.cpp |
| 1 | `css/parser/CSSParserContext.h` |  | css/parser/CSSPropertyParserConsumer+LengthDefinitions.h, css/parser/CSSPropertyParserConsumer+Timeline.cpp |
| 1 | `css/parser/CSSParserTokenRange.h` |  | css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSPropertyParserConsumer+List.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h, css/parser/CSSPropertyParserConsumer+MetaResolver.h, css/parser/CSSPropertyParserConsumer+Timeline.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h` |  | css/parser/CSSPropertyParserConsumer+Timeline.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Ident.h` |  | css/parser/CSSPropertyParserConsumer+Timeline.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+LengthPercentageDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+Timeline.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+List.h` |  | css/parser/CSSPropertyParserConsumer+Timeline.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Timeline.h` |  | css/parser/CSSPropertyParserConsumer+Timeline.cpp |
| 1 | `css/parser/CSSPropertyParserState.h` |  | css/parser/CSSPropertyParserConsumer+LengthDefinitions.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h, css/parser/CSSPropertyParserConsumer+Timeline.cpp |
| 1 | `style/values/animations/StyleSingleAnimationRange.h` |  | css/parser/CSSPropertyParserConsumer+Timeline.cpp |
| 0 | `css/parser/CSSPropertyParserConsumer+Timeline.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/TimelineRangeValue.h` ← 1 file(s)

### external (2)

- `pal/text/TextEncoding.h` ← 1 file(s)
- `unicode/umachine.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)

### platform (7)

- `platform/LayoutUnit.h` ← 2 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/graphics/Color.h` ← 3 file(s)
- `platform/graphics/ColorHash.h` ← 1 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)
- `platform/text/WritingMode.h` ← 1 file(s)

### rendering (1)

- `rendering/style/RenderStyleConstants.h` ← 2 file(s)

### wtf (38)

- `wtf/BitSet.h` ← 1 file(s)
- `wtf/Brigand.h` ← 8 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 4 file(s)
- `wtf/FixedVector.h` ← 1 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 13 file(s)
- `wtf/Function.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 3 file(s)
- `wtf/Hasher.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/MallocSpan.h` ← 1 file(s)
- `wtf/Markable.h` ← 2 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NeverDestroyed.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 1 file(s)
- `wtf/Ref.h` ← 5 file(s)
- `wtf/RefCounted.h` ← 2 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 4 file(s)
- `wtf/StdLibExtras.h` ← 9 file(s)
- `wtf/TZoneMalloc.h` ← 2 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/URL.h` ← 1 file(s)
- `wtf/Vector.h` ← 5 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 2 file(s)
- `wtf/text/AtomStringHash.h` ← 1 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 2 file(s)
- `wtf/text/WTFString.h` ← 2 file(s)

## Problems

### unresolved (3)

- `CSSPropertyNames.h` ← 5 file(s)
- `CSSPropertyParsing.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 8 file(s)
