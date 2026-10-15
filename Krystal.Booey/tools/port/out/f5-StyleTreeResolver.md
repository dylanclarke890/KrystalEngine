# Port graph

Entries: `style/StyleTreeResolver.cpp`

- Files to port: **102**
- Boundary `accessibility`: 1
- Boundary `animation`: 4
- Boundary `cssjit`: 2
- Boundary `dom`: 19
- Boundary `external`: 1
- Boundary `html`: 5
- Boundary `loader`: 4
- Boundary `page`: 2
- Boundary `platform`: 19
- Boundary `rendering`: 4
- Boundary `wtf`: 55
- Unresolved: 9

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 8 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 7 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 7 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 6 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 6 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 6 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h |
| 5 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 5 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 5 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 5 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 5 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 5 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 5 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 4 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 4 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 4 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 4 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 4 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 4 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h |
| 4 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h |
| 4 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 4 | `css/values/CSSNoConversionDataRequiredToken.h` |  | style/values/StyleValueTypes.h |
| 4 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 3 | `css/CSSPrimitiveValue.h` |  | css/StylePropertiesInlines.h, css/parser/CSSParserToken.h |
| 3 | `css/CSSProperty.h` |  | css/StyleProperties.h |
| 3 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 3 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.h, style/RuleSet.h |
| 3 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h, style/StyleBuilderState.h |
| 3 | `css/CSSVariableData.h` |  | css/StyleRule.h |
| 3 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h, style/values/anchor-position/StylePositionTryFallback.h |
| 3 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 3 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 3 | `css/StyleRuleType.h` |  | css/CSSRule.h, css/StyleRule.h, css/parser/CSSParserContext.h |
| 3 | `css/parser/CSSParserEnum.h` |  | css/CSSRule.h |
| 3 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 3 | `css/query/GenericMediaQueryEvaluator.h` |  | css/query/MediaQueryEvaluator.h, style/ContainerQueryEvaluator.h |
| 3 | `css/query/MediaQuery.h` |  | css/StyleRule.h, css/query/MediaQueryEvaluator.h, style/RuleSet.h |
| 3 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 3 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 3 | `style/PropertyAllowlist.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleData.h |
| 3 | `style/PseudoElementRequest.h` |  | style/ElementRuleCollector.h |
| 3 | `style/RuleData.h` |  | style/RuleSet.h |
| 3 | `style/RuleFeature.h` |  | style/RuleSet.h, style/StyleScopeRuleSets.h |
| 3 | `style/StyleForVisitedLink.h` |  | style/StyleBuilderState.h |
| 3 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h, style/PseudoElementRequest.h |
| 3 | `style/UserAgentStyle.h` |  | style/StyleScopeRuleSets.h |
| 3 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h, style/values/anchor-position/StylePositionTryFallback.h |
| 2 | `css/CSSFontFace.h` |  | css/CSSFontFaceSet.h, css/CSSFontSelector.h |
| 2 | `css/CSSFontFaceSet.h` |  | css/CSSFontSelector.h |
| 2 | `css/CSSRule.h` |  | css/CSSPositionTryRule.h |
| 2 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, style/HasSelectorFilter.h, style/StyleResolver.h |
| 2 | `css/SelectorChecker.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/StyleBuilderState.h, style/StyleTreeResolver.h |
| 2 | `css/StyleProperties.h` |  | css/CSSPositionTryRule.h, css/ImmutableStyleProperties.h, css/MutableStyleProperties.h |
| 2 | `css/StylePropertiesInlines.h` |  | css/CSSPositionTryRule.h, style/MatchResult.h |
| 2 | `css/StyleRule.h` |  | css/CSSFontSelector.h, css/CSSPositionTryRule.h, style/RuleData.h, style/RuleSet.h |
| 2 | `css/query/MediaQueryEvaluator.h` |  | style/ElementRuleCollector.h, style/StyleResolver.h |
| 2 | `rendering/style/AutosizeStatus.h` |  | style/StyleAdjuster.h |
| 2 | `rendering/style/RenderStyle+GettersInlines.h` |  | rendering/style/RenderStyle+SettersInlines.h |
| 2 | `rendering/style/RenderStyle.h` |  | css/query/MediaQueryEvaluator.h, rendering/style/RenderStyle+GettersInlines.h, style/MatchedDeclarationsCache.h, style/ResolvedStyle.h, style/StyleBuilderState.h, style/StyleTreeResolver.h |
| 2 | `rendering/style/RenderStyleConstants.h` |  | css/CSSFontFace.h, css/CSSSelector.h, style/PropertyAllowlist.h, style/PseudoElementIdentifier.h, style/PseudoElementRequest.h, style/StyleAdjuster.h, style/Styleable.h |
| 2 | `style/ElementRuleCollector.h` |  | style/StyleResolver.h |
| 2 | `style/InspectorCSSOMWrappers.h` |  | style/StyleResolver.h |
| 2 | `style/MatchResult.h` |  | style/ElementRuleCollector.h, style/MatchedDeclarationsCache.h, style/PropertyCascade.h, style/ResolvedStyle.h |
| 2 | `style/MatchedDeclarationsCache.h` |  | style/StyleResolver.h |
| 2 | `style/PropertyCascade.h` |  | style/MatchResultCache.h, style/StyleBuilder.h, style/StyleBuilderState.h, style/StyleResolver.h, style/StyleTreeResolver.h |
| 2 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h, style/PseudoElementRequest.h, style/Styleable.h |
| 2 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 2 | `style/RuleSet.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/StyleBuilderState.h, style/StyleResolver.h, style/StyleScopeRuleSets.h |
| 2 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h, style/values/anchor-position/StylePositionTryFallback.h |
| 2 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h, style/StyleTreeResolver.h |
| 2 | `style/StyleBuilderState.h` |  | style/StyleBuilder.h |
| 2 | `style/StyleChange.h` |  | style/StyleTreeResolver.h, style/StyleUpdate.h |
| 2 | `style/StyleRelations.h` |  | css/SelectorChecker.h, style/ResolvedStyle.h |
| 2 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h, style/StyleScope.h |
| 2 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/ElementRuleCollector.h, style/MatchResult.h, style/ScopedName.h, style/StyleScope.h |
| 2 | `style/StyleScopeRuleSets.h` |  | style/StyleResolver.h |
| 2 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h, style/StyleTreeResolver.h |
| 2 | `style/Styleable.h` |  | style/StyleScope.h, style/StyleTreeResolver.h |
| 2 | `style/TreeResolutionState.h` |  | style/StyleBuilderState.h, style/StyleResolver.h, style/StyleTreeResolver.h |
| 2 | `style/values/anchor-position/StylePositionTryFallback.h` |  | style/StyleTreeResolver.h |
| 1 | `css/CSSFontSelector.h` |  | style/StyleTreeResolver.cpp |
| 1 | `css/CSSPositionTryRule.h` |  | style/StyleTreeResolver.cpp |
| 1 | `css/values/CSSSerializationContext.h` |  | style/StyleTreeResolver.cpp |
| 1 | `rendering/style/PositionTryOrder.h` |  | style/AnchorPositionEvaluator.h, style/StyleTreeResolver.cpp |
| 1 | `rendering/style/RenderStyle+SettersInlines.h` |  | style/StyleTreeResolver.cpp |
| 1 | `style/AnchorPositionEvaluator.h` |  | style/StyleScope.h, style/StyleTreeResolver.cpp, style/StyleTreeResolver.h, style/TreeResolutionState.h |
| 1 | `style/MatchResultCache.h` |  | style/StyleTreeResolver.cpp |
| 1 | `style/ResolvedStyle.h` |  | style/MatchResultCache.h, style/StyleTreeResolver.cpp, style/StyleTreeResolver.h |
| 1 | `style/StyleAdjuster.h` |  | style/StyleTreeResolver.cpp |
| 1 | `style/StyleBuilder.h` |  | style/StyleTreeResolver.cpp |
| 1 | `style/StyleFontSizeFunctions.h` |  | style/StyleTreeResolver.cpp |
| 1 | `style/StyleResolver.h` |  | style/StyleTreeResolver.cpp |
| 1 | `style/StyleScope.h` |  | style/StyleTreeResolver.cpp |
| 1 | `style/StyleTreeResolver.h` |  | style/StyleTreeResolver.cpp |
| 1 | `style/values/anchor-position/StylePositionTryFallbackTactic.h` |  | style/StyleTreeResolver.cpp, style/values/anchor-position/StylePositionTryFallback.h |
| 0 | `style/StyleTreeResolver.cpp` |  | (entry) |

## Boundaries

### accessibility (1)

- `accessibility/AXObjectCache.h` ← 1 file(s)

### animation (4)

- `animation/DocumentTimeline.h` ← 1 file(s)
- `animation/StyleOriginatedTimelinesController.h` ← 1 file(s)
- `animation/WebAnimationTypes.h` ← 3 file(s)
- `animation/WebAnimationUtilities.h` ← 1 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (19)

- `dom/ActiveDOMObject.h` ← 1 file(s)
- `dom/ComposedTreeAncestorIterator.h` ← 1 file(s)
- `dom/ComposedTreeIterator.h` ← 1 file(s)
- `dom/Document.h` ← 3 file(s)
- `dom/DocumentPage.h` ← 1 file(s)
- `dom/DocumentQuirks.h` ← 1 file(s)
- `dom/DocumentView.h` ← 1 file(s)
- `dom/Element.h` ← 4 file(s)
- `dom/EventTarget.h` ← 2 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/NodeInlines.h` ← 1 file(s)
- `dom/NodeRenderStyle.h` ← 1 file(s)
- `dom/PseudoElement.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)
- `dom/ScriptExecutionContext.h` ← 1 file(s)
- `dom/ShadowRoot.h` ← 1 file(s)
- `dom/Text.h` ← 1 file(s)
- `dom/TypedElementDescendantIteratorInlines.h` ← 1 file(s)
- `dom/ViewTransition.h` ← 1 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### html (5)

- `html/HTMLBodyElement.h` ← 1 file(s)
- `html/HTMLInputElement.h` ← 1 file(s)
- `html/HTMLMeterElement.h` ← 1 file(s)
- `html/HTMLProgressElement.h` ← 1 file(s)
- `html/HTMLSlotElement.h` ← 1 file(s)

### loader (4)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)
- `loader/LoaderStrategy.h` ← 1 file(s)
- `loader/cache/CachedResourceHandle.h` ← 1 file(s)
- `loader/cache/TrustedFonts.h` ← 1 file(s)

### page (2)

- `page/LocalFrame.h` ← 1 file(s)
- `page/Page.h` ← 1 file(s)

### platform (19)

- `platform/BoxSides.h` ← 1 file(s)
- `platform/CommonAtomStrings.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 3 file(s)
- `platform/PlatformStrategies.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 1 file(s)
- `platform/Timer.h` ← 3 file(s)
- `platform/graphics/Font.h` ← 1 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/FontSelectionAlgorithm.h` ← 1 file(s)
- `platform/graphics/FontSelector.h` ← 1 file(s)
- `platform/graphics/FontTaggedSettings.h` ← 2 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutRect.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 2 file(s)
- `platform/text/TextFlags.h` ← 2 file(s)
- `platform/text/WritingMode.h` ← 2 file(s)

### rendering (4)

- `rendering/PositionedLayoutConstraints.h` ← 1 file(s)
- `rendering/RenderBoxInlines.h` ← 1 file(s)
- `rendering/RenderElement.h` ← 1 file(s)
- `rendering/RenderView.h` ← 1 file(s)

### wtf (55)

- `wtf/AbstractRefCountedAndCanMakeWeakPtr.h` ← 2 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 3 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 4 file(s)
- `wtf/CanMakeWeakPtr.h` ← 1 file(s)
- `wtf/CheckedPtr.h` ← 3 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/EnumSet.h` ← 2 file(s)
- `wtf/EnumTraits.h` ← 7 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/Forward.h` ← 14 file(s)
- `wtf/Function.h` ← 1 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 12 file(s)
- `wtf/HashSet.h` ← 5 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/Identified.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 3 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/Observer.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 5 file(s)
- `wtf/Ref.h` ← 5 file(s)
- `wtf/RefCounted.h` ← 1 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 2 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 10 file(s)
- `wtf/StdLibExtras.h` ← 4 file(s)
- `wtf/TZoneMalloc.h` ← 10 file(s)
- `wtf/TypeCasts.h` ← 4 file(s)
- `wtf/URL.h` ← 1 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 1 file(s)
- `wtf/Vector.h` ← 12 file(s)
- `wtf/WeakHashMap.h` ← 3 file(s)
- `wtf/WeakHashSet.h` ← 4 file(s)
- `wtf/WeakListHashSet.h` ← 1 file(s)
- `wtf/WeakPtr.h` ← 2 file(s)
- `wtf/WeakRef.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 10 file(s)
- `wtf/text/AtomStringHash.h` ← 6 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringHash.h` ← 3 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 4 file(s)
- `wtf/text/WTFString.h` ← 5 file(s)

## Problems

### unresolved (9)

- `CSSPropertyNames.h` ← 6 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 8 file(s)
- `HTMLNames.h` ← 1 file(s)
- `RenderStyleProperties+GettersInlines.h` ← 1 file(s)
- `RenderStyleProperties+SettersInlines.h` ← 1 file(s)
- `RenderStyleProperties.h` ← 1 file(s)
- `Settings.h` ← 2 file(s)
- `WebKitFontFamilyNames.h` ← 1 file(s)
