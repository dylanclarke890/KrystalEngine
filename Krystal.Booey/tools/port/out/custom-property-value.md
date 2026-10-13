# Port graph

Entries: `css/CSSCustomPropertyValue.cpp`, `css/CSSVariableData.cpp`

- Files to port: **56**
- Boundary `external`: 1
- Boundary `loader`: 1
- Boundary `platform`: 6
- Boundary `rendering`: 1
- Boundary `wtf`: 36
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 8 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 7 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 7 | `style/values/StyleValueTypes.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 7 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 7 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 7 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 7 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 6 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 6 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h |
| 6 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 6 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 6 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 5 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 5 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 5 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 5 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 5 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 5 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 4 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 4 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 3 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/StyleRuleType.h` |  | css/parser/CSSParserContext.h |
| 3 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 3 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 3 | `css/values/color/CSSColor.h` |  | css/CSSColorValue.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 2 | `css/CSSColorValue.h` |  | css/CSSValuePool.h |
| 2 | `css/CSSPrimitiveValue.h` |  | css/CSSValuePool.h, css/parser/CSSParserToken.h |
| 2 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSColorValue.h, css/CSSCustomPropertyValue.h, css/CSSPrimitiveValue.h, css/CSSVariableReferenceValue.h, css/values/CSSValueTypes.h |
| 2 | `css/CSSVariableReferenceValue.h` |  | css/CSSCustomPropertyValue.h |
| 2 | `css/CSSWideKeyword.h` |  | css/CSSCustomPropertyValue.h |
| 2 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h |
| 2 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 2 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 1 | `css/CSSCustomPropertyValue.h` |  | css/CSSCustomPropertyValue.cpp, css/CSSVariableData.cpp |
| 1 | `css/CSSValuePool.h` |  | css/CSSVariableData.cpp |
| 1 | `css/CSSVariableData.h` |  | css/CSSCustomPropertyValue.h, css/CSSVariableData.cpp, css/CSSVariableReferenceValue.h |
| 1 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.cpp, css/CSSVariableData.h, css/CSSVariableReferenceValue.h |
| 1 | `css/values/CSSSerializationContext.h` |  | css/CSSCustomPropertyValue.cpp |
| 0 | `css/CSSCustomPropertyValue.cpp` |  | (entry) |
| 0 | `css/CSSVariableData.cpp` |  | (entry) |

## Boundaries

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)

### platform (6)

- `platform/LayoutUnit.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/graphics/Color.h` ← 3 file(s)
- `platform/graphics/ColorHash.h` ← 1 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)

### rendering (1)

- `rendering/style/RenderStyle.h` ← 1 file(s)

### wtf (36)

- `wtf/Brigand.h` ← 6 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 4 file(s)
- `wtf/FixedVector.h` ← 1 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 9 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 3 file(s)
- `wtf/Hasher.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/Markable.h` ← 2 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NeverDestroyed.h` ← 2 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 1 file(s)
- `wtf/PointerComparison.h` ← 1 file(s)
- `wtf/Ref.h` ← 3 file(s)
- `wtf/RefCounted.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 6 file(s)
- `wtf/TZoneMalloc.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/URL.h` ← 1 file(s)
- `wtf/Vector.h` ← 4 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 2 file(s)
- `wtf/text/AtomStringHash.h` ← 2 file(s)
- `wtf/text/ParsingUtilities.h` ← 2 file(s)
- `wtf/text/StringBuilder.h` ← 2 file(s)
- `wtf/text/StringView.h` ← 4 file(s)
- `wtf/text/TextStream.h` ← 1 file(s)
- `wtf/text/WTFString.h` ← 5 file(s)

## Problems

### unresolved (2)

- `CSSPropertyNames.h` ← 4 file(s)
- `CSSValueKeywords.h` ← 7 file(s)
