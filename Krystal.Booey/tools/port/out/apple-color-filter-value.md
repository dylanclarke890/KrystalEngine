# Port graph

Entries: `css/CSSAppleColorFilterValue.cpp`

- Files to port: **72**
- Boundary `bindings`: 1
- Boundary `dom`: 1
- Boundary `external`: 2
- Boundary `loader`: 1
- Boundary `platform`: 7
- Boundary `wtf`: 40
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 8 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 7 | `style/values/StyleValueTypes.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 7 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 7 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 7 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 7 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 6 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 6 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h |
| 6 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 6 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 5 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 5 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 5 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 5 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 5 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 5 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 5 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/CSSProperty.h` |  | css/CSSStyleDeclaration.h |
| 4 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/StyleRuleType.h` |  | css/parser/CSSParserContext.h |
| 4 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 4 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 4 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/filter-effects/CSSAppleInvertLightnessFunction.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 4 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 3 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/CSSStyleDeclaration.h` |  | css/DeprecatedCSSOMValue.h |
| 3 | `css/CSSWideKeyword.h` |  | css/parser/CSSParserIdioms.h |
| 3 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/parser/CSSParserContext.h` |  | css/parser/CSSParserIdioms.h |
| 3 | `css/values/color/CSSColor.h` |  | css/CSSColorValue.h, css/values/filter-effects/CSSDropShadowFunction.h, css/values/filter-effects/CSSHueRotateFunction.h |
| 3 | `css/values/filter-effects/CSSAppleInvertLightnessFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilterFunction.h |
| 3 | `css/values/filter-effects/CSSBlurFunction.h` |  | css/values/filter-effects/CSSFilterFunction.h |
| 3 | `css/values/filter-effects/CSSBrightnessFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilterFunction.h |
| 3 | `css/values/filter-effects/CSSContrastFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilterFunction.h |
| 3 | `css/values/filter-effects/CSSDropShadowFunction.h` |  | css/values/filter-effects/CSSFilterFunction.h |
| 3 | `css/values/filter-effects/CSSGrayscaleFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilterFunction.h |
| 3 | `css/values/filter-effects/CSSHueRotateFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilterFunction.h |
| 3 | `css/values/filter-effects/CSSInvertFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilterFunction.h |
| 3 | `css/values/filter-effects/CSSOpacityFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilterFunction.h |
| 3 | `css/values/filter-effects/CSSSaturateFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilterFunction.h |
| 3 | `css/values/filter-effects/CSSSepiaFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilterFunction.h |
| 3 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 2 | `css/CSSColorValue.h` |  | css/CSSValuePool.h |
| 2 | `css/CSSPrimitiveValue.h` |  | css/CSSValuePool.h, css/DeprecatedCSSOMPrimitiveValue.h |
| 2 | `css/CSSValue.h` |  | css/CSSAppleColorFilterValue.h, css/CSSAttrValue.h, css/CSSColorValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/CSSValueList.h, css/DeprecatedCSSOMValue.h, css/values/CSSValueTypes.h |
| 2 | `css/CSSValueList.h` |  | css/DeprecatedCSSOMValueList.h |
| 2 | `css/DeprecatedCSSOMValue.h` |  | css/DeprecatedCSSOMFilterFunctionValue.h, css/DeprecatedCSSOMPrimitiveValue.h, css/DeprecatedCSSOMValueList.h |
| 2 | `css/parser/CSSParserIdioms.h` |  | css/DeprecatedCSSOMPrimitiveValue.h |
| 2 | `css/values/filter-effects/CSSAppleColorFilter.h` |  | css/CSSAppleColorFilterValue.h |
| 2 | `css/values/filter-effects/CSSFilterFunction.h` |  | css/DeprecatedCSSOMFilterFunctionValue.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, css/values/filter-effects/CSSBlurFunction.h, css/values/filter-effects/CSSBrightnessFunction.h, css/values/filter-effects/CSSContrastFunction.h, css/values/filter-effects/CSSDropShadowFunction.h, css/values/filter-effects/CSSGrayscaleFunction.h, css/values/filter-effects/CSSHueRotateFunction.h, css/values/filter-effects/CSSInvertFunction.h, css/values/filter-effects/CSSOpacityFunction.h, css/values/filter-effects/CSSSaturateFunction.h, css/values/filter-effects/CSSSepiaFunction.h, css/values/primitives/CSSPrimitiveNumericTypes+CSSValueVisitation.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 1 | `css/CSSAppleColorFilterValue.h` |  | css/CSSAppleColorFilterValue.cpp |
| 1 | `css/CSSValuePool.h` |  | css/CSSAppleColorFilterValue.cpp |
| 1 | `css/DeprecatedCSSOMFilterFunctionValue.h` |  | css/CSSAppleColorFilterValue.cpp |
| 1 | `css/DeprecatedCSSOMPrimitiveValue.h` |  | css/CSSAppleColorFilterValue.cpp |
| 1 | `css/DeprecatedCSSOMValueList.h` |  | css/CSSAppleColorFilterValue.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes+CSSValueVisitation.h` |  | css/CSSAppleColorFilterValue.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h` |  | css/CSSAppleColorFilterValue.cpp |
| 0 | `css/CSSAppleColorFilterValue.cpp` |  | (entry) |

## Boundaries

### bindings (1)

- `bindings/js/ScriptWrappable.h` ← 1 file(s)

### dom (1)

- `dom/ExceptionOr.h` ← 1 file(s)

### external (2)

- `pal/text/TextEncoding.h` ← 1 file(s)
- `unicode/umachine.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)

### platform (7)

- `platform/LayoutUnit.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/graphics/Color.h` ← 3 file(s)
- `platform/graphics/ColorHash.h` ← 1 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)
- `platform/text/WritingMode.h` ← 1 file(s)

### wtf (40)

- `wtf/ASCIICType.h` ← 1 file(s)
- `wtf/AbstractRefCountedAndCanMakeWeakPtr.h` ← 1 file(s)
- `wtf/BitSet.h` ← 1 file(s)
- `wtf/Brigand.h` ← 6 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 4 file(s)
- `wtf/FixedVector.h` ← 1 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 8 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 2 file(s)
- `wtf/Hasher.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/MallocSpan.h` ← 1 file(s)
- `wtf/Markable.h` ← 2 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NeverDestroyed.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 1 file(s)
- `wtf/Ref.h` ← 3 file(s)
- `wtf/RefCounted.h` ← 1 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 2 file(s)
- `wtf/StdLibExtras.h` ← 7 file(s)
- `wtf/TZoneMalloc.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/URL.h` ← 1 file(s)
- `wtf/Vector.h` ← 4 file(s)
- `wtf/WeakPtr.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 2 file(s)
- `wtf/text/AtomStringHash.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 1 file(s)
- `wtf/text/WTFString.h` ← 2 file(s)

## Problems

### unresolved (2)

- `CSSPropertyNames.h` ← 5 file(s)
- `CSSValueKeywords.h` ← 8 file(s)
