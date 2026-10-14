# Port graph

Entries: `dom/SelectorQuery.cpp`

- Files to port: **51**
- Boundary `cssjit`: 1
- Boundary `dom`: 10
- Boundary `external`: 1
- Boundary `html`: 2
- Boundary `loader`: 1
- Boundary `page`: 1
- Boundary `platform`: 5
- Boundary `rendering`: 1
- Boundary `svg`: 1
- Boundary `wtf`: 41
- Unresolved: 4

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 9 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 9 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 8 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 8 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 7 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 7 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 6 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 6 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h |
| 5 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h |
| 5 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 5 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 5 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 4 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 4 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSPrimitiveValue.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 4 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 4 | `css/query/GenericMediaQueryEvaluator.h` |  | style/ContainerQueryEvaluator.h |
| 4 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 4 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 3 | `css/CSSPrimitiveValue.h` |  | css/parser/CSSParserToken.h |
| 3 | `css/SelectorFilter.h` |  | style/SelectorMatchingState.h |
| 3 | `css/StyleRuleType.h` |  | css/parser/CSSParserContext.h |
| 3 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h, css/parser/CSSSelectorParserContext.h |
| 3 | `css/query/ContainerQuery.h` |  | style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 3 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 3 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 2 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/parser/MutableCSSSelector.h, style/HasSelectorFilter.h |
| 2 | `css/CSSSelectorList.h` |  | css/parser/CSSSelectorParser.h |
| 2 | `css/StyleSheetContents.h` |  | css/parser/CSSSelectorParser.h |
| 2 | `css/parser/CSSParserContext.h` |  | css/StyleSheetContents.h, css/parser/CSSSelectorParser.h |
| 2 | `css/parser/CSSParserEnum.h` |  | css/parser/CSSSelectorParser.h |
| 2 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 2 | `css/parser/CSSParserTokenRange.h` |  | css/parser/CSSSelectorParser.h |
| 2 | `css/parser/CSSSelectorParserContext.h` |  | css/parser/CSSSelectorParser.h |
| 2 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 2 | `css/parser/MutableCSSSelector.h` |  | css/parser/CSSSelectorParser.h |
| 2 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h |
| 2 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 2 | `style/StyleRelations.h` |  | css/SelectorChecker.h |
| 2 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h |
| 2 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h |
| 1 | `css/SelectorChecker.h` |  | dom/SelectorQuery.cpp |
| 1 | `css/parser/CSSSelectorParser.h` |  | dom/SelectorQuery.cpp, dom/SelectorQuery.h |
| 1 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h, dom/SelectorQuery.cpp |
| 1 | `dom/SelectorQuery.h` |  | dom/SelectorQuery.cpp |
| 0 | `dom/SelectorQuery.cpp` |  | (entry) |

## Boundaries

### cssjit (1)

- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (10)

- `dom/Element.h` ← 3 file(s)
- `dom/ElementAncestorIteratorInlines.h` ← 1 file(s)
- `dom/ExceptionOr.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/NodeList.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)
- `dom/StaticNodeList.h` ← 1 file(s)
- `dom/StyledElement.h` ← 1 file(s)
- `dom/TreeScopeInlines.h` ← 1 file(s)
- `dom/TypedElementDescendantIteratorInlines.h` ← 1 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### html (2)

- `html/HTMLBaseElement.h` ← 1 file(s)
- `html/HTMLDocument.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)

### page (1)

- `page/SecurityOriginData.h` ← 1 file(s)

### platform (5)

- `platform/CommonAtomStrings.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 2 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 1 file(s)

### rendering (1)

- `rendering/style/RenderStyleConstants.h` ← 2 file(s)

### svg (1)

- `svg/SVGElement.h` ← 1 file(s)

### wtf (41)

- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 4 file(s)
- `wtf/CheckedPtr.h` ← 1 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 6 file(s)
- `wtf/FixedVector.h` ← 2 file(s)
- `wtf/Forward.h` ← 7 file(s)
- `wtf/Function.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 2 file(s)
- `wtf/HashMap.h` ← 4 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 2 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 3 file(s)
- `wtf/Ref.h` ← 3 file(s)
- `wtf/RefCounted.h` ← 1 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 3 file(s)
- `wtf/TZoneMalloc.h` ← 7 file(s)
- `wtf/TZoneMallocInlines.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/Vector.h` ← 7 file(s)
- `wtf/WeakPtr.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 5 file(s)
- `wtf/text/AtomStringHash.h` ← 4 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 3 file(s)
- `wtf/text/WTFString.h` ← 2 file(s)

## Problems

### unresolved (4)

- `CSSPropertyNames.h` ← 4 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 5 file(s)
- `HTMLNames.h` ← 1 file(s)
