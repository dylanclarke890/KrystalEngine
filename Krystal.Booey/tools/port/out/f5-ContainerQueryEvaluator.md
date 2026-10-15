# Port graph

Entries: `style/ContainerQueryEvaluator.cpp`

- Files to port: **59**
- Boundary `animation`: 1
- Boundary `cssjit`: 1
- Boundary `dom`: 10
- Boundary `external`: 2
- Boundary `loader`: 1
- Boundary `platform`: 11
- Boundary `rendering`: 1
- Boundary `wtf`: 44
- Unresolved: 5

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 6 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 5 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 5 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 5 | `css/values/CSSNoConversionDataRequiredToken.h` |  | style/values/StyleValueTypes.h |
| 5 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 5 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 5 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 4 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 4 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 4 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 4 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 4 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h |
| 4 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h |
| 3 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 3 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/MediaQueryParserContext.h |
| 3 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 3 | `rendering/style/PositionTryOrder.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `rendering/style/RenderStyleConstants.h` |  | css/CSSSelector.h, style/PseudoElementIdentifier.h, style/Styleable.h |
| 3 | `style/PseudoElementIdentifier.h` |  | style/AnchorPositionEvaluator.h, style/Styleable.h |
| 3 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 2 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 2 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/StyleRule.h |
| 2 | `css/CSSSelectorList.h` |  | css/StyleRule.h |
| 2 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSPrimitiveValue.h, css/CSSValueList.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 2 | `css/CSSVariableData.h` |  | css/StyleRule.h |
| 2 | `css/MediaQueryParserContext.h` |  | css/MediaList.h |
| 2 | `css/StyleRuleType.h` |  | css/StyleRule.h, css/parser/CSSParserContext.h |
| 2 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 2 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h |
| 2 | `css/query/GenericMediaQueryEvaluator.h` |  | style/ContainerQueryEvaluator.h |
| 2 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/ContainerQueryFeatures.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 2 | `css/query/MediaQuery.h` |  | css/MediaList.h, css/StyleRule.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 2 | `rendering/style/RenderStyle.h` |  | rendering/style/RenderStyle+GettersInlines.h |
| 2 | `style/AnchorPositionEvaluator.h` |  | style/StyleScope.h |
| 2 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h, style/StyleScope.h |
| 2 | `style/StyleScopeOrdinal.h` |  | style/ContainerQueryEvaluator.h, style/ScopedName.h, style/StyleScope.h |
| 2 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 2 | `style/Styleable.h` |  | style/StyleScope.h |
| 1 | `css/CSSPrimitiveValue.h` |  | css/parser/CSSParserToken.h, style/ContainerQueryEvaluator.cpp |
| 1 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h, style/ContainerQueryEvaluator.cpp |
| 1 | `css/CSSValueList.h` |  | style/ContainerQueryEvaluator.cpp |
| 1 | `css/MediaList.h` |  | style/ContainerQueryEvaluator.cpp |
| 1 | `css/StyleRule.h` |  | style/ContainerQueryEvaluator.cpp |
| 1 | `css/query/ContainerQueryFeatures.h` |  | style/ContainerQueryEvaluator.cpp |
| 1 | `rendering/style/RenderStyle+GettersInlines.h` |  | style/ContainerQueryEvaluator.cpp |
| 1 | `style/ContainerQueryEvaluator.h` |  | style/ContainerQueryEvaluator.cpp |
| 1 | `style/StyleScope.h` |  | style/ContainerQueryEvaluator.cpp |
| 0 | `style/ContainerQueryEvaluator.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/WebAnimationTypes.h` ← 1 file(s)

### cssjit (1)

- `cssjit/CompiledSelector.h` ← 1 file(s)

### dom (10)

- `dom/ComposedTreeAncestorIterator.h` ← 1 file(s)
- `dom/ContainerNodeInlines.h` ← 1 file(s)
- `dom/Document.h` ← 2 file(s)
- `dom/Element.h` ← 2 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/NodeDocument.h` ← 1 file(s)
- `dom/NodeRenderStyle.h` ← 1 file(s)
- `dom/PseudoElement.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)

### external (2)

- `pal/text/TextEncoding.h` ← 1 file(s)
- `unicode/umachine.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)

### platform (11)

- `platform/BoxSides.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 3 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/Timer.h` ← 1 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutRect.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 2 file(s)
- `platform/text/WritingMode.h` ← 1 file(s)

### rendering (1)

- `rendering/RenderView.h` ← 1 file(s)

### wtf (44)

- `wtf/Assertions.h` ← 1 file(s)
- `wtf/Brigand.h` ← 4 file(s)
- `wtf/CheckedPtr.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 2 file(s)
- `wtf/EnumTraits.h` ← 7 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/Forward.h` ← 8 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 4 file(s)
- `wtf/HashSet.h` ← 1 file(s)
- `wtf/Hasher.h` ← 1 file(s)
- `wtf/Identified.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 3 file(s)
- `wtf/MallocSpan.h` ← 1 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 3 file(s)
- `wtf/Ref.h` ← 4 file(s)
- `wtf/RefCounted.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 3 file(s)
- `wtf/StdLibExtras.h` ← 4 file(s)
- `wtf/TZoneMalloc.h` ← 6 file(s)
- `wtf/TypeCasts.h` ← 2 file(s)
- `wtf/URL.h` ← 1 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 1 file(s)
- `wtf/Vector.h` ← 7 file(s)
- `wtf/WeakHashMap.h` ← 2 file(s)
- `wtf/WeakHashSet.h` ← 2 file(s)
- `wtf/WeakListHashSet.h` ← 1 file(s)
- `wtf/WeakPtr.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 7 file(s)
- `wtf/text/AtomStringHash.h` ← 3 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 4 file(s)
- `wtf/text/WTFString.h` ← 4 file(s)

## Problems

### unresolved (5)

- `CSSPropertyNames.h` ← 4 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 7 file(s)
- `RenderStyleProperties+GettersInlines.h` ← 1 file(s)
- `RenderStyleProperties.h` ← 1 file(s)
