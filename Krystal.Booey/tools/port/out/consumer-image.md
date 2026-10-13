# Port graph

Entries: `css/parser/CSSPropertyParserConsumer+Image.cpp`

- Files to port: **107**
- Boundary `external`: 2
- Boundary `loader`: 4
- Boundary `platform`: 9
- Boundary `rendering`: 1
- Boundary `wtf`: 40
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 4 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 4 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 4 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 4 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 4 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h |
| 4 | `style/values/StyleValueTypes.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 4 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 3 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 3 | `css/calc/CSSCalcSymbolsAllowed.h` |  | css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 3 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 3 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 3 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 3 | `css/values/filter-effects/CSSBlurFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSBrightnessFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSContrastFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSDropShadowFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSFilterReference.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSGrayscaleFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSHueRotateFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSInvertFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSOpacityFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSSaturateFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSSepiaFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 3 | `css/values/primitives/CSSURLModifiers.h` |  | css/values/primitives/CSSURL.h |
| 3 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 3 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `css/CSSColorValue.h` |  | css/CSSValuePool.h |
| 2 | `css/CSSPrimitiveValue.h` |  | css/CSSCrossfadeValue.h, css/CSSImageSetOptionValue.h, css/CSSValuePool.h, css/parser/CSSParserToken.h, css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h, css/parser/CSSPropertyParserConsumer+Ident.h |
| 2 | `css/CSSProperty.h` |  | css/parser/CSSPropertyParserConsumer+LengthDefinitions.h, css/parser/CSSPropertyParserState.h |
| 2 | `css/CSSValuePair.h` |  | css/CSSCursorImageValue.h |
| 2 | `css/StyleRuleType.h` |  | css/parser/CSSParserContext.h, css/parser/CSSPropertyParserState.h |
| 2 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h, css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 2 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h, css/parser/CSSPropertyParserOptions.h |
| 2 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaResolver.h, css/parser/CSSTokenizer.h |
| 2 | `css/parser/CSSPropertyParserConsumer+LengthDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+LengthPercentageDefinitions.h |
| 2 | `css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+AngleDefinitions.h, css/parser/CSSPropertyParserConsumer+AnglePercentageDefinitions.h, css/parser/CSSPropertyParserConsumer+LengthDefinitions.h, css/parser/CSSPropertyParserConsumer+LengthPercentageDefinitions.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaResolver.h, css/parser/CSSPropertyParserConsumer+NumberDefinitions.h, css/parser/CSSPropertyParserConsumer+PercentageDefinitions.h, css/parser/CSSPropertyParserConsumer+ResolutionDefinitions.h |
| 2 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 2 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/images/CSSGradient.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSURLModifiers.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 2 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h |
| 2 | `css/values/color/CSSColorType.h` |  | css/parser/CSSPropertyParserConsumer+Color.h |
| 2 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 2 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 2 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h |
| 2 | `css/values/filter-effects/CSSFilter.h` |  | css/CSSFilterImageValue.h |
| 2 | `css/values/images/CSSGradient.h` |  | css/CSSGradientValue.h |
| 2 | `css/values/primitives/CSSPosition.h` |  | css/parser/CSSPropertyParserConsumer+Position.h, css/values/images/CSSGradient.h |
| 2 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes+Canonicalization.h` |  | css/parser/CSSPropertyParserConsumer+AngleDefinitions.h, css/parser/CSSPropertyParserConsumer+ResolutionDefinitions.h |
| 2 | `css/values/primitives/CSSURL.h` |  | css/CSSCursorImageValue.h, css/CSSImageValue.h, css/values/filter-effects/CSSFilterReference.h |
| 2 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h, css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 2 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/values/color/CSSColorDescriptors.h |
| 1 | `css/CSSCanvasValue.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/CSSCrossfadeValue.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/CSSCursorImageValue.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/CSSFilterImageValue.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/CSSGradientValue.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/CSSImageSetOptionValue.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/CSSImageSetValue.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/CSSImageValue.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/CSSNamedImageValue.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/CSSPaintImageValue.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSCanvasValue.h, css/CSSColorValue.h, css/CSSCrossfadeValue.h, css/CSSCursorImageValue.h, css/CSSFilterImageValue.h, css/CSSImageSetOptionValue.h, css/CSSImageValue.h, css/CSSNamedImageValue.h, css/CSSPaintImageValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/CSSValueList.h, css/CSSValuePair.h, css/parser/CSSPropertyParserConsumer+Image.cpp, css/values/CSSValueTypes.h |
| 1 | `css/CSSValueList.h` |  | css/CSSImageSetValue.h, css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/CSSValuePool.h` |  | css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSPropertyParserConsumer+Image.cpp, css/parser/CSSPropertyParserState.h |
| 1 | `css/CSSVariableData.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/calc/CSSCalcSymbolTable.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp, style/values/StyleValueTypes.h |
| 1 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/parser/CSSPropertyParserConsumer+AngleDefinitions.h, css/parser/CSSPropertyParserConsumer+Image.cpp, css/parser/CSSPropertyParserConsumer+LengthDefinitions.h |
| 1 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h, css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSPropertyParserConsumer+Image.cpp, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h, css/parser/CSSPropertyParserConsumer+MetaResolver.h |
| 1 | `css/parser/CSSPropertyParserConsumer+AngleDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+AnglePercentageDefinitions.h, css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+AnglePercentageDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Color.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+ColorInterpolationMethod.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Filter.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Ident.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Image.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+LengthPercentageDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+MetaConsumer.h` |  | css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h, css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+MetaResolver.h` |  | css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h, css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+NumberDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Percentage.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+PercentageDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Position.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Primitives.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+ResolutionDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+String.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+URL.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp |
| 1 | `css/parser/CSSPropertyParserOptions.h` |  | css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h, css/parser/CSSPropertyParserConsumer+Image.cpp, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h, css/parser/CSSPropertyParserConsumer+MetaResolver.h, css/parser/CSSPropertyParserConsumer+Percentage.h, css/parser/CSSPropertyParserConsumer+Position.h |
| 1 | `css/parser/CSSPropertyParserState.h` |  | css/parser/CSSPropertyParserConsumer+Image.cpp, css/parser/CSSPropertyParserConsumer+LengthDefinitions.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 1 | `css/values/color/CSSColor.h` |  | css/CSSColorValue.h, css/parser/CSSPropertyParserConsumer+Image.cpp, css/values/filter-effects/CSSDropShadowFunction.h, css/values/filter-effects/CSSHueRotateFunction.h, css/values/images/CSSGradient.h |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h, css/parser/CSSPropertyParserConsumer+Image.cpp, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h, css/parser/CSSPropertyParserConsumer+MetaResolver.h, css/values/color/CSSColorDescriptors.h, css/values/filter-effects/CSSBlurFunction.h, css/values/filter-effects/CSSBrightnessFunction.h, css/values/filter-effects/CSSContrastFunction.h, css/values/filter-effects/CSSDropShadowFunction.h, css/values/filter-effects/CSSGrayscaleFunction.h, css/values/filter-effects/CSSHueRotateFunction.h, css/values/filter-effects/CSSInvertFunction.h, css/values/filter-effects/CSSOpacityFunction.h, css/values/filter-effects/CSSSaturateFunction.h, css/values/filter-effects/CSSSepiaFunction.h, css/values/primitives/CSSPosition.h, css/values/primitives/CSSPrimitiveNumericTypes+Canonicalization.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 0 | `css/parser/CSSPropertyParserConsumer+Image.cpp` |  | (entry) |

## Boundaries

### external (2)

- `pal/text/TextEncoding.h` ← 1 file(s)
- `unicode/umachine.h` ← 1 file(s)

### loader (4)

- `loader/LoadedFromOpaqueSource.h` ← 2 file(s)
- `loader/ResourceLoaderOptions.h` ← 1 file(s)
- `loader/cache/CachedImage.h` ← 1 file(s)
- `loader/cache/CachedResourceHandle.h` ← 1 file(s)

### platform (9)

- `platform/LayoutUnit.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/graphics/Color.h` ← 3 file(s)
- `platform/graphics/ColorHash.h` ← 1 file(s)
- `platform/graphics/ColorInterpolationMethod.h` ← 1 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)
- `platform/graphics/IntPoint.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 1 file(s)

### rendering (1)

- `rendering/style/StyleImage.h` ← 2 file(s)

### wtf (40)

- `wtf/BitSet.h` ← 1 file(s)
- `wtf/Brigand.h` ← 8 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 4 file(s)
- `wtf/FixedVector.h` ← 1 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 13 file(s)
- `wtf/Function.h` ← 5 file(s)
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
- `wtf/OptionSet.h` ← 4 file(s)
- `wtf/Ref.h` ← 4 file(s)
- `wtf/RefCounted.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 6 file(s)
- `wtf/SortedArrayMap.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 9 file(s)
- `wtf/TZoneMalloc.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/Vector.h` ← 4 file(s)
- `wtf/WeakHashSet.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 2 file(s)
- `wtf/text/AtomStringHash.h` ← 1 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 1 file(s)
- `wtf/text/WTFString.h` ← 8 file(s)

## Problems

### unresolved (2)

- `CSSPropertyNames.h` ← 5 file(s)
- `CSSValueKeywords.h` ← 7 file(s)
