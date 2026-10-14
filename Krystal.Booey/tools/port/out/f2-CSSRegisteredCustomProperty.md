# Port graph

Entries: `css/CSSRegisteredCustomProperty.cpp`

- Files to port: **67**
- Boundary `external`: 1
- Boundary `loader`: 1
- Boundary `platform`: 8
- Boundary `rendering`: 1
- Boundary `wtf`: 37
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 8 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 7 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 7 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 6 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 6 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 6 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 6 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h |
| 6 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 5 | `css/CSSPrimitiveValue.h` |  | css/parser/CSSParserToken.h |
| 5 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 5 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 5 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 5 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSURLModifiers.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 5 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 5 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 5 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h, style/values/color/StyleResolvedColor.h |
| 5 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 5 | `css/values/primitives/CSSURLModifiers.h` |  | css/values/primitives/CSSURL.h |
| 5 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 5 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/StyleRuleType.h` |  | css/parser/CSSParserContext.h |
| 4 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 4 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 4 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 4 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 4 | `css/values/color/CSSColor.h` |  | style/values/color/StyleColor.h |
| 4 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h, style/values/color/StyleColor.h |
| 4 | `css/values/color/CSSColorType.h` |  | style/values/color/StyleColor.h |
| 4 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSURL.h` |  | style/values/primitives/StyleURL.h |
| 4 | `style/values/StyleValueTypes.h` |  | style/values/images/StyleImageWrapper.h, style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleURL.h, style/values/transforms/StyleTransformFunction.h, style/values/transforms/functions/StyleTransformFunctionBase.h |
| 4 | `style/values/color/StyleColorOptions.h` |  | style/values/color/StyleColor.h |
| 4 | `style/values/color/StyleCurrentColor.h` |  | style/values/color/StyleColor.h |
| 4 | `style/values/color/StyleResolvedColor.h` |  | style/values/color/StyleColor.h |
| 4 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 4 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `style/values/transforms/functions/StyleTransformFunctionBase.h` |  | style/values/transforms/StyleTransformFunction.h |
| 4 | `style/values/transforms/functions/StyleTransformFunctionWrapper.h` |  | style/values/transforms/StyleTransformFunction.h |
| 4 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h |
| 3 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h, css/CSSVariableReferenceValue.h |
| 3 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h, style/StyleCustomProperty.h |
| 3 | `style/values/color/StyleColor.h` |  | style/StyleCustomProperty.h |
| 3 | `style/values/images/StyleImageWrapper.h` |  | style/StyleCustomProperty.h |
| 3 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/StyleCustomProperty.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 3 | `style/values/primitives/StyleURL.h` |  | style/StyleCustomProperty.h |
| 3 | `style/values/transforms/StyleTransformFunction.h` |  | style/StyleCustomProperty.h |
| 2 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSCustomPropertyValue.h, css/CSSPrimitiveValue.h, css/CSSVariableReferenceValue.h, css/values/CSSValueTypes.h, style/StyleCustomProperty.h |
| 2 | `css/CSSVariableData.h` |  | css/CSSCustomPropertyValue.h, css/CSSRegisteredCustomProperty.h, css/CSSVariableReferenceValue.h, style/StyleCustomProperty.h |
| 2 | `css/CSSVariableReferenceValue.h` |  | css/CSSCustomPropertyValue.h |
| 2 | `css/CSSWideKeyword.h` |  | css/CSSCustomPropertyValue.h |
| 2 | `css/parser/CSSCustomPropertySyntax.h` |  | css/CSSRegisteredCustomProperty.h |
| 2 | `style/StyleCustomProperty.h` |  | css/CSSRegisteredCustomProperty.h |
| 1 | `css/CSSCustomPropertyValue.h` |  | css/CSSRegisteredCustomProperty.cpp |
| 1 | `css/CSSRegisteredCustomProperty.h` |  | css/CSSRegisteredCustomProperty.cpp |
| 0 | `css/CSSRegisteredCustomProperty.cpp` |  | (entry) |

## Boundaries

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 2 file(s)

### platform (8)

- `platform/LayoutUnit.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/graphics/Color.h` ← 5 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)
- `platform/graphics/FloatSize.h` ← 1 file(s)
- `platform/graphics/transforms/TransformOperation.h` ← 2 file(s)
- `platform/graphics/transforms/TransformationMatrix.h` ← 1 file(s)

### rendering (1)

- `rendering/style/StyleImage.h` ← 1 file(s)

### wtf (37)

- `wtf/Brigand.h` ← 6 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 4 file(s)
- `wtf/FixedVector.h` ← 1 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 13 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 1 file(s)
- `wtf/Hasher.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/Markable.h` ← 3 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 2 file(s)
- `wtf/PointerComparison.h` ← 2 file(s)
- `wtf/Ref.h` ← 4 file(s)
- `wtf/RefCounted.h` ← 3 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 6 file(s)
- `wtf/TZoneMalloc.h` ← 2 file(s)
- `wtf/TypeCasts.h` ← 2 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/UniqueRef.h` ← 1 file(s)
- `wtf/Variant.h` ← 1 file(s)
- `wtf/Vector.h` ← 5 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 5 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringParsingBuffer.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 1 file(s)
- `wtf/text/WTFString.h` ← 5 file(s)

## Problems

### unresolved (2)

- `CSSPropertyNames.h` ← 4 file(s)
- `CSSValueKeywords.h` ← 8 file(s)
