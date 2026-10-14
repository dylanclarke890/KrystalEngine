# Port graph

Entries: `dom/InlineStyleSheetOwner.cpp`, `html/HTMLStyleElement.cpp`, `css/StyleSheetList.cpp`

- Files to port: **56**
- Boundary `animation`: 1
- Boundary `dom`: 13
- Boundary `external`: 1
- Boundary `html`: 3
- Boundary `loader`: 2
- Boundary `page`: 2
- Boundary `platform`: 10
- Boundary `rendering`: 2
- Boundary `wtf`: 51
- Unresolved: 5

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 10 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 9 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 9 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 9 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 8 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 8 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h |
| 7 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 7 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 7 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 6 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 6 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 6 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 5 | `css/CSSPrimitiveValue.h` |  | css/parser/CSSParserToken.h |
| 5 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 5 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 5 | `css/values/CSSNoConversionDataRequiredToken.h` |  | style/values/StyleValueTypes.h |
| 5 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryTypes.h |
| 4 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSPrimitiveValue.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 4 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 4 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 4 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h |
| 3 | `css/parser/CSSParserMode.h` |  | css/StyleSheet.h, css/parser/CSSParserContext.h |
| 3 | `css/parser/CSSParserTokenRange.h` |  | css/query/GenericMediaQueryParser.h |
| 3 | `css/query/GenericMediaQueryTypes.h` |  | css/query/GenericMediaQueryParser.h, css/query/MediaQuery.h |
| 3 | `style/PseudoElementIdentifier.h` |  | style/AnchorPositionEvaluator.h, style/Styleable.h |
| 3 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 2 | `css/CSSRuleList.h` |  | css/CSSStyleSheet.h |
| 2 | `css/StyleRuleType.h` |  | css/CSSRule.h, css/parser/CSSParserContext.h |
| 2 | `css/StyleSheet.h` |  | css/CSSStyleSheet.h |
| 2 | `css/parser/CSSParserContext.h` |  | css/MediaQueryParserContext.h, css/StyleSheetContents.h, css/query/GenericMediaQueryParser.h, style/StyleSheetContentsCache.h |
| 2 | `css/parser/CSSParserEnum.h` |  | css/CSSRule.h |
| 2 | `css/query/GenericMediaQueryParser.h` |  | css/query/MediaQueryParser.h |
| 2 | `css/query/MediaQuery.h` |  | css/CSSStyleSheet.h, css/MediaList.h, css/query/MediaQueryParser.h |
| 2 | `style/AnchorPositionEvaluator.h` |  | style/StyleScope.h |
| 2 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h, style/StyleScope.h |
| 2 | `style/StyleScopeOrdinal.h` |  | style/ScopedName.h, style/StyleScope.h |
| 2 | `style/Styleable.h` |  | style/StyleScope.h |
| 1 | `css/CSSRule.h` |  | html/HTMLStyleElement.cpp |
| 1 | `css/CSSStyleSheet.h` |  | css/StyleSheetList.cpp |
| 1 | `css/MediaList.h` |  | css/CSSStyleSheet.h, dom/InlineStyleSheetOwner.cpp |
| 1 | `css/MediaQueryParserContext.h` |  | css/MediaList.h, css/query/GenericMediaQueryParser.h, css/query/MediaQueryParser.h, dom/InlineStyleSheetOwner.cpp, html/HTMLStyleElement.cpp |
| 1 | `css/StyleSheetContents.h` |  | dom/InlineStyleSheetOwner.cpp, html/HTMLStyleElement.cpp |
| 1 | `css/StyleSheetList.h` |  | css/StyleSheetList.cpp |
| 1 | `css/query/MediaQueryParser.h` |  | dom/InlineStyleSheetOwner.cpp, html/HTMLStyleElement.cpp |
| 1 | `style/StyleScope.h` |  | css/StyleSheetList.cpp, dom/InlineStyleSheetOwner.cpp, html/HTMLStyleElement.cpp |
| 1 | `style/StyleSheetContentsCache.h` |  | dom/InlineStyleSheetOwner.cpp |
| 0 | `css/StyleSheetList.cpp` |  | (entry) |
| 0 | `dom/InlineStyleSheetOwner.cpp` |  | (entry) |
| 0 | `html/HTMLStyleElement.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/WebAnimationTypes.h` ← 1 file(s)

### dom (13)

- `dom/Document.h` ← 3 file(s)
- `dom/Element.h` ← 2 file(s)
- `dom/ElementInlines.h` ← 1 file(s)
- `dom/Event.h` ← 1 file(s)
- `dom/EventSender.h` ← 1 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/ExceptionOr.h` ← 1 file(s)
- `dom/InlineStyleSheetOwner.h` ← 1 file(s)
- `dom/NodeDocument.h` ← 1 file(s)
- `dom/PseudoElement.h` ← 1 file(s)
- `dom/ScriptableDocumentParser.h` ← 2 file(s)
- `dom/ShadowRoot.h` ← 3 file(s)
- `dom/TextNodeTraversal.h` ← 2 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### html (3)

- `html/DOMTokenList.h` ← 1 file(s)
- `html/HTMLStyleElement.h` ← 2 file(s)
- `html/PluginDocument.h` ← 1 file(s)

### loader (2)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)
- `loader/cache/CachedResource.h` ← 1 file(s)

### page (2)

- `page/Page.h` ← 1 file(s)
- `page/csp/ContentSecurityPolicy.h` ← 1 file(s)

### platform (10)

- `platform/CommonAtomStrings.h` ← 2 file(s)
- `platform/LayoutUnit.h` ← 2 file(s)
- `platform/Logging.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/Timer.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutRect.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 2 file(s)
- `platform/text/WritingMode.h` ← 1 file(s)

### rendering (2)

- `rendering/style/PositionTryOrder.h` ← 1 file(s)
- `rendering/style/RenderStyleConstants.h` ← 2 file(s)

### wtf (51)

- `wtf/AbstractRefCounted.h` ← 1 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/Brigand.h` ← 4 file(s)
- `wtf/CheckedPtr.h` ← 3 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 5 file(s)
- `wtf/FixedVector.h` ← 1 file(s)
- `wtf/Forward.h` ← 9 file(s)
- `wtf/Function.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 6 file(s)
- `wtf/HashSet.h` ← 1 file(s)
- `wtf/Hasher.h` ← 1 file(s)
- `wtf/Identified.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 2 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NeverDestroyed.h` ← 3 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 2 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 1 file(s)
- `wtf/Ref.h` ← 2 file(s)
- `wtf/RefCounted.h` ← 4 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 2 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 3 file(s)
- `wtf/RobinHoodHashMap.h` ← 1 file(s)
- `wtf/SetForScope.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 4 file(s)
- `wtf/TZoneMalloc.h` ← 3 file(s)
- `wtf/TZoneMallocInlines.h` ← 2 file(s)
- `wtf/TypeCasts.h` ← 3 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/UniqueRef.h` ← 1 file(s)
- `wtf/Vector.h` ← 10 file(s)
- `wtf/WeakHashMap.h` ← 2 file(s)
- `wtf/WeakHashSet.h` ← 3 file(s)
- `wtf/WeakListHashSet.h` ← 1 file(s)
- `wtf/WeakPtr.h` ← 4 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 6 file(s)
- `wtf/text/AtomStringHash.h` ← 6 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextPosition.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 3 file(s)
- `wtf/text/WTFString.h` ← 5 file(s)

## Problems

### unresolved (5)

- `CSSPropertyNames.h` ← 4 file(s)
- `CSSValueKeywords.h` ← 6 file(s)
- `EventNames.h` ← 1 file(s)
- `HTMLNames.h` ← 2 file(s)
- `NodeName.h` ← 1 file(s)
