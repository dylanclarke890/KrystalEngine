# Port graph

Entries: `css/parser/CSSSelectorParser.cpp`, `css/CSSSelectorList.cpp`, `css/CSSSelector.cpp`, `css/parser/MutableCSSSelector.cpp`, `css/parser/CSSSelectorParserContext.cpp`

- Files to port: **68**
- Boundary `dom`: 3
- Boundary `external`: 1
- Boundary `loader`: 1
- Boundary `platform`: 7
- Boundary `rendering`: 1
- Boundary `wtf`: 46
- Unresolved: 7

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 9 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 8 | `style/values/StyleValueTypes.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 8 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 8 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 8 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 8 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 7 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 7 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 7 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h |
| 7 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 7 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 6 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 6 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 6 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h |
| 5 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 5 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 5 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 5 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h |
| 5 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 5 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 5 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h |
| 5 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 4 | `css/values/color/CSSColor.h` |  | css/CSSColorValue.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 3 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/CSSColorValue.h` |  | css/CSSValuePool.h |
| 3 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSColorValue.h, css/CSSPrimitiveValue.h, css/values/CSSValueTypes.h |
| 3 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 2 | `css/CSSPrimitiveValue.h` |  | css/CSSValuePool.h, css/parser/CSSParserToken.h, css/parser/CSSPropertyParserConsumer+Ident.h |
| 2 | `css/CSSValuePool.h` |  | css/parser/CSSPropertyParserConsumer+Ident.h |
| 2 | `css/CSSWideKeyword.h` |  | css/parser/CSSParserIdioms.h |
| 2 | `css/StyleRuleType.h` |  | css/parser/CSSParserContext.h |
| 2 | `css/StyleSheetContents.h` |  | css/parser/CSSSelectorParser.h |
| 2 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h, css/parser/CSSSelectorParserContext.h |
| 2 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSTokenizer.h |
| 2 | `css/parser/CSSParserTokenRange.h` |  | css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSSelectorParser.h |
| 2 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 1 | `css/CSSMarkup.h` |  | css/CSSSelector.cpp |
| 1 | `css/CSSSelector.h` |  | css/CSSSelector.cpp, css/CSSSelectorList.h, css/SelectorPseudoTypeMap.h, css/parser/MutableCSSSelector.cpp, css/parser/MutableCSSSelector.h |
| 1 | `css/CSSSelectorList.h` |  | css/CSSSelector.cpp, css/CSSSelectorList.cpp, css/parser/CSSSelectorParser.h, css/parser/MutableCSSSelector.cpp |
| 1 | `css/SelectorPseudoTypeMap.h` |  | css/CSSSelector.cpp, css/parser/CSSSelectorParser.cpp, css/parser/MutableCSSSelector.cpp |
| 1 | `css/parser/CSSParserContext.h` |  | css/StyleSheetContents.h, css/parser/CSSParserIdioms.h, css/parser/CSSSelectorParser.h, css/parser/CSSSelectorParserContext.cpp |
| 1 | `css/parser/CSSParserEnum.h` |  | css/parser/CSSSelectorParser.cpp, css/parser/CSSSelectorParser.h |
| 1 | `css/parser/CSSParserIdioms.h` |  | css/parser/CSSSelectorParser.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Ident.h` |  | css/parser/CSSSelectorParser.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Primitives.h` |  | css/parser/CSSSelectorParser.cpp |
| 1 | `css/parser/CSSSelectorParser.h` |  | css/parser/CSSSelectorParser.cpp |
| 1 | `css/parser/CSSSelectorParserContext.h` |  | css/parser/CSSSelectorParser.h, css/parser/CSSSelectorParserContext.cpp |
| 1 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSSelectorParser.cpp |
| 1 | `css/parser/MutableCSSSelector.h` |  | css/CSSSelector.cpp, css/CSSSelectorList.cpp, css/parser/CSSSelectorParser.cpp, css/parser/CSSSelectorParser.h, css/parser/MutableCSSSelector.cpp |
| 1 | `style/PseudoElementIdentifier.h` |  | css/parser/CSSSelectorParser.cpp |
| 0 | `css/CSSSelector.cpp` |  | (entry) |
| 0 | `css/CSSSelectorList.cpp` |  | (entry) |
| 0 | `css/parser/CSSSelectorParser.cpp` |  | (entry) |
| 0 | `css/parser/CSSSelectorParserContext.cpp` |  | (entry) |
| 0 | `css/parser/MutableCSSSelector.cpp` |  | (entry) |

## Boundaries

### dom (3)

- `dom/Document.h` ← 1 file(s)
- `dom/DocumentQuirks.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)

### platform (7)

- `platform/CommonAtomStrings.h` ← 3 file(s)
- `platform/LayoutUnit.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/graphics/Color.h` ← 3 file(s)
- `platform/graphics/ColorHash.h` ← 1 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)

### rendering (1)

- `rendering/style/RenderStyleConstants.h` ← 2 file(s)

### wtf (46)

- `wtf/ASCIICType.h` ← 1 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/Brigand.h` ← 6 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 5 file(s)
- `wtf/FixedVector.h` ← 2 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 9 file(s)
- `wtf/Function.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 2 file(s)
- `wtf/HashMap.h` ← 3 file(s)
- `wtf/Hasher.h` ← 4 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/Markable.h` ← 2 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NeverDestroyed.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 2 file(s)
- `wtf/Ref.h` ← 3 file(s)
- `wtf/RefCounted.h` ← 1 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 2 file(s)
- `wtf/SetForScope.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 7 file(s)
- `wtf/TZoneMalloc.h` ← 4 file(s)
- `wtf/TZoneMallocInlines.h` ← 3 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/Vector.h` ← 6 file(s)
- `wtf/WeakPtr.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 3 file(s)
- `wtf/text/AtomStringHash.h` ← 5 file(s)
- `wtf/text/MakeString.h` ← 1 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 4 file(s)
- `wtf/text/StringToIntegerConversion.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 3 file(s)
- `wtf/text/WTFString.h` ← 3 file(s)

## Problems

### unresolved (7)

- `CSSPropertyNames.h` ← 3 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSSelectorInlines.h` ← 3 file(s)
- `CSSValueKeywords.h` ← 7 file(s)
- `HTMLNames.h` ← 1 file(s)
- `Settings.h` ← 1 file(s)
- `UserAgentParts.h` ← 1 file(s)
