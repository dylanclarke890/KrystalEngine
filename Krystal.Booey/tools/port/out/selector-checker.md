# Port graph

Entries: `css/SelectorChecker.cpp`

- Files to port: **59**
- Boundary `animation`: 1
- Boundary `cssjit`: 1
- Boundary `dom`: 18
- Boundary `editing`: 1
- Boundary `external`: 1
- Boundary `html`: 12
- Boundary `inspector`: 1
- Boundary `loader`: 1
- Boundary `page`: 5
- Boundary `platform`: 12
- Boundary `rendering`: 3
- Boundary `wtf`: 45
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
| 5 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 4 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h |
| 4 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSPrimitiveValue.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 4 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 4 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 4 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 4 | `css/query/GenericMediaQueryEvaluator.h` |  | style/ContainerQueryEvaluator.h |
| 4 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 4 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h |
| 3 | `css/SelectorFilter.h` |  | style/SelectorMatchingState.h |
| 3 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h |
| 3 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h |
| 3 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 3 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 3 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 3 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 2 | `css/CSSVariableData.h` |  | css/StyleRule.h |
| 2 | `css/StyleRuleType.h` |  | css/StyleRule.h, css/parser/CSSParserContext.h |
| 2 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 2 | `css/query/MediaQuery.h` |  | css/StyleRule.h |
| 2 | `style/AnchorPositionEvaluator.h` |  | style/StyleScope.h |
| 2 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h, style/Styleable.h |
| 2 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 2 | `style/StyleRelations.h` |  | css/SelectorChecker.h |
| 2 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h, style/StyleScope.h |
| 2 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/ScopedName.h, style/StyleScope.h |
| 2 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h |
| 2 | `style/Styleable.h` |  | style/StyleScope.h |
| 1 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.cpp, css/SelectorChecker.h, css/StyleRule.h, style/HasSelectorFilter.h |
| 1 | `css/CSSSelectorList.h` |  | css/SelectorChecker.cpp, css/StyleRule.h, style/RuleFeature.h |
| 1 | `css/SelectorChecker.h` |  | css/SelectorChecker.cpp, css/SelectorCheckerTestFunctions.h |
| 1 | `css/SelectorCheckerTestFunctions.h` |  | css/SelectorChecker.cpp |
| 1 | `css/StyleRule.h` |  | css/SelectorChecker.cpp |
| 1 | `style/RuleFeature.h` |  | css/SelectorChecker.cpp |
| 1 | `style/StyleScope.h` |  | css/SelectorChecker.cpp |
| 0 | `css/SelectorChecker.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/WebAnimationTypes.h` ← 1 file(s)

### cssjit (1)

- `cssjit/CompiledSelector.h` ← 1 file(s)

### dom (18)

- `dom/Document.h` ← 2 file(s)
- `dom/DocumentFullscreen.h` ← 1 file(s)
- `dom/DocumentImmersive.h` ← 1 file(s)
- `dom/DocumentOrShadowRootFullscreen.h` ← 1 file(s)
- `dom/DocumentPage.h` ← 1 file(s)
- `dom/Element.h` ← 4 file(s)
- `dom/ElementChildIteratorInlines.h` ← 1 file(s)
- `dom/ElementRareData.h` ← 1 file(s)
- `dom/ElementTraversal.h` ← 1 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/PseudoElement.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)
- `dom/ShadowRoot.h` ← 2 file(s)
- `dom/Text.h` ← 1 file(s)
- `dom/TypedElementDescendantIteratorInlines.h` ← 1 file(s)
- `dom/ViewTransition.h` ← 2 file(s)
- `dom/ViewTransitionTypeSet.h` ← 1 file(s)

### editing (1)

- `editing/FrameSelection.h` ← 2 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### html (12)

- `html/HTMLAttachmentElement.h` ← 1 file(s)
- `html/HTMLDialogElement.h` ← 1 file(s)
- `html/HTMLDocument.h` ← 1 file(s)
- `html/HTMLFrameElement.h` ← 1 file(s)
- `html/HTMLIFrameElement.h` ← 1 file(s)
- `html/HTMLImageElement.h` ← 1 file(s)
- `html/HTMLInputElement.h` ← 1 file(s)
- `html/HTMLMediaElement.h` ← 1 file(s)
- `html/HTMLOptionElement.h` ← 1 file(s)
- `html/HTMLSlotElement.h` ← 1 file(s)
- `html/HTMLVideoElement.h` ← 1 file(s)
- `html/track/WebVTTElement.h` ← 1 file(s)

### inspector (1)

- `inspector/InspectorInstrumentation.h` ← 2 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)

### page (5)

- `page/FocusController.h` ← 1 file(s)
- `page/FrameDestructionObserverInlines.h` ← 1 file(s)
- `page/LocalFrame.h` ← 1 file(s)
- `page/LocalFrameInlines.h` ← 1 file(s)
- `page/Page.h` ← 2 file(s)

### platform (12)

- `platform/CommonAtomStrings.h` ← 2 file(s)
- `platform/LayoutUnit.h` ← 3 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 1 file(s)
- `platform/Timer.h` ← 1 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutRect.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 2 file(s)
- `platform/text/WritingMode.h` ← 1 file(s)

### rendering (3)

- `rendering/RenderElement.h` ← 1 file(s)
- `rendering/style/PositionTryOrder.h` ← 1 file(s)
- `rendering/style/RenderStyleConstants.h` ← 3 file(s)

### wtf (45)

- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 4 file(s)
- `wtf/CheckedPtr.h` ← 2 file(s)
- `wtf/Compiler.h` ← 1 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 6 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/Forward.h` ← 9 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 6 file(s)
- `wtf/HashSet.h` ← 2 file(s)
- `wtf/Hasher.h` ← 1 file(s)
- `wtf/Identified.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 3 file(s)
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
- `wtf/TZoneMalloc.h` ← 7 file(s)
- `wtf/TypeCasts.h` ← 2 file(s)
- `wtf/URL.h` ← 1 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 1 file(s)
- `wtf/Vector.h` ← 7 file(s)
- `wtf/WeakHashMap.h` ← 2 file(s)
- `wtf/WeakHashSet.h` ← 2 file(s)
- `wtf/WeakListHashSet.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 8 file(s)
- `wtf/text/AtomStringHash.h` ← 4 file(s)
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
- `HTMLNames.h` ← 1 file(s)
- `Settings.h` ← 1 file(s)
