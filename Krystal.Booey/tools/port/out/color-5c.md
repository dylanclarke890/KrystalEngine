# Port graph

Entries: `css/CSSValuePool.cpp`, `css/CSSColorValue.cpp`, `css/CSSDynamicRangeLimitValue.cpp`, `css/CSSColorSchemeValue.cpp`

- Files to port: **128**
- Boundary `animation`: 3
- Boundary `cssjit`: 2
- Boundary `dom`: 5
- Boundary `external`: 2
- Boundary `loader`: 3
- Boundary `platform`: 23
- Boundary `rendering`: 4
- Boundary `wtf`: 47
- Unresolved: 3

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 9 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 8 | `css/CSSProperty.h` |  | css/StyleProperties.h |
| 8 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 7 | `css/StyleProperties.h` |  | css/ImmutableStyleProperties.h, css/MutableStyleProperties.h |
| 7 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 7 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 7 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 7 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 6 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 6 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 6 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 6 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h |
| 6 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h |
| 6 | `css/query/GenericMediaQueryEvaluator.h` |  | style/ContainerQueryEvaluator.h |
| 6 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 6 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 6 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 6 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 6 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 5 | `css/CSSVariableData.h` |  | css/StyleRule.h |
| 5 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 5 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 5 | `css/StyleRuleType.h` |  | css/StyleRule.h, css/parser/CSSParserContext.h |
| 5 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 5 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 5 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 5 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h |
| 5 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 5 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 5 | `style/PropertyAllowlist.h` |  | style/MatchResult.h, style/RuleData.h |
| 5 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 5 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 4 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, style/HasSelectorFilter.h |
| 4 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.h, style/RuleSet.h |
| 4 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/StyleRule.h` |  | style/RuleData.h, style/RuleSet.h |
| 4 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 4 | `css/query/MediaQuery.h` |  | css/StyleRule.h, style/RuleSet.h |
| 4 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 4 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 4 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/values/primitives/CSSURLModifiers.h` |  | css/values/primitives/CSSURL.h |
| 4 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 4 | `style/MatchResult.h` |  | style/PropertyCascade.h |
| 4 | `style/RuleData.h` |  | style/RuleSet.h |
| 4 | `style/RuleFeature.h` |  | style/RuleSet.h |
| 4 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleRelations.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h |
| 4 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/MatchResult.h, style/ScopedName.h |
| 4 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h |
| 4 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h |
| 3 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/SelectorChecker.h` |  | style/MatchResult.h, style/StyleBuilderState.h |
| 3 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 3 | `css/values/CSSValueTypes.h` |  | css/values/color-adjust/CSSColorScheme.h, css/values/color/CSSColor.h, css/values/color/CSSDynamicRangeLimit.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSURLModifiers.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 3 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 3 | `css/values/primitives/CSSURL.h` |  | css/CSSFontFaceSrcValue.h |
| 3 | `style/PropertyCascade.h` |  | style/StyleBuilderState.h |
| 3 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h |
| 3 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/RuleSet.h` |  | style/MatchResult.h, style/StyleBuilderState.h |
| 3 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/StyleForVisitedLink.h` |  | style/StyleBuilderState.h |
| 3 | `style/TreeResolutionState.h` |  | style/StyleBuilderState.h |
| 3 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h, style/values/anchor-position/StylePositionVisibility.h, style/values/box/StyleMarginTrim.h, style/values/contain/StyleContain.h, style/values/images/StyleImageOrientation.h, style/values/masking/StyleMaskMode.h, style/values/non-standard/StyleWebKitLineBoxContain.h, style/values/pointerevents/StyleTouchAction.h, style/values/primitives/StylePrimitiveNumeric.h, style/values/scrollbars/StyleScrollbarWidth.h, style/values/speech/StyleSpeakAs.h, style/values/text-decoration/StyleTextDecorationLine.h, style/values/text-decoration/StyleTextEmphasisPosition.h, style/values/text-decoration/StyleTextUnderlinePosition.h, style/values/text/StyleHangingPunctuation.h, style/values/text/StyleTextAlign.h, style/values/text/StyleTextAlignLast.h, style/values/text/StyleTextTransform.h, style/values/ui/StyleResize.h |
| 2 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 2 | `css/CSSFontFaceSrcValue.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `css/CSSToLengthConversionData.h` |  | css/CSSPrimitiveValueMappings.h, css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h, style/StyleBuilderState.h |
| 2 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSColorSchemeValue.h, css/CSSColorValue.h, css/CSSDynamicRangeLimitValue.h, css/CSSFontFaceSrcValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/CSSValueList.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 2 | `css/calc/CSSCalcSymbolTable.h` |  | css/CSSPrimitiveValueMappings.h, style/values/StyleValueTypes.h |
| 2 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 2 | `css/values/color-adjust/CSSColorScheme.h` |  | css/CSSColorSchemeValue.h |
| 2 | `css/values/color/CSSColor.h` |  | css/CSSColorValue.h |
| 2 | `css/values/color/CSSDynamicRangeLimit.h` |  | css/CSSDynamicRangeLimitValue.h, css/values/color/CSSDynamicRangeLimitMix.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, css/values/color/CSSDynamicRangeLimitMix.h, css/values/primitives/CSSPrimitiveNumericTypes+CSSValueVisitation.h, css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 2 | `style/AnchorPositionEvaluator.h` |  | css/CSSPrimitiveValueMappings.h, style/TreeResolutionState.h |
| 2 | `style/StyleBuilderState.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/anchor-position/StylePositionTryFallbackTactic.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/anchor-position/StylePositionVisibility.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/box/StyleMarginTrim.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/contain/StyleContain.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/images/StyleImageOrientation.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/masking/StyleMaskMode.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/non-standard/StyleWebKitLineBoxContain.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/non-standard/StyleWebKitOverflowScrolling.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/non-standard/StyleWebKitTouchCallout.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/overflow/StyleScrollBehavior.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/pointerevents/StyleTouchAction.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/scrollbars/StyleScrollbarWidth.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/speech/StyleSpeakAs.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text-decoration/StyleTextDecorationLine.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text-decoration/StyleTextEmphasisPosition.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text-decoration/StyleTextUnderlinePosition.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text/StyleHangingPunctuation.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text/StyleTextAlign.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text/StyleTextAlignLast.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/text/StyleTextTransform.h` |  | css/CSSPrimitiveValueMappings.h |
| 2 | `style/values/ui/StyleResize.h` |  | css/CSSPrimitiveValueMappings.h |
| 1 | `css/CSSColorSchemeValue.h` |  | css/CSSColorSchemeValue.cpp |
| 1 | `css/CSSColorValue.h` |  | css/CSSColorValue.cpp, css/CSSValuePool.h |
| 1 | `css/CSSDynamicRangeLimitValue.h` |  | css/CSSDynamicRangeLimitValue.cpp |
| 1 | `css/CSSPrimitiveValue.h` |  | css/CSSColorValue.cpp, css/CSSPrimitiveValueMappings.h, css/CSSValuePool.h, css/StylePropertiesInlines.h, css/parser/CSSParserToken.h |
| 1 | `css/CSSPrimitiveValueMappings.h` |  | css/CSSValuePool.cpp |
| 1 | `css/CSSValueList.h` |  | css/CSSValuePool.cpp |
| 1 | `css/CSSValuePool.h` |  | css/CSSValuePool.cpp |
| 1 | `css/parser/CSSPropertyParser.h` |  | css/CSSValuePool.cpp |
| 1 | `css/values/color/CSSDynamicRangeLimitMix.h` |  | css/CSSDynamicRangeLimitValue.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes+CSSValueVisitation.h` |  | css/CSSDynamicRangeLimitValue.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.h` |  | css/CSSDynamicRangeLimitValue.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h` |  | css/CSSDynamicRangeLimitValue.cpp |
| 0 | `css/CSSColorSchemeValue.cpp` |  | (entry) |
| 0 | `css/CSSColorValue.cpp` |  | (entry) |
| 0 | `css/CSSDynamicRangeLimitValue.cpp` |  | (entry) |
| 0 | `css/CSSValuePool.cpp` |  | (entry) |

## Boundaries

### animation (3)

- `animation/CompositeOperation.h` ← 1 file(s)
- `animation/ScrollAxis.h` ← 1 file(s)
- `animation/WebAnimationTypes.h` ← 1 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (5)

- `dom/Document.h` ← 1 file(s)
- `dom/Element.h` ← 3 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)

### external (2)

- `pal/text/TextEncoding.h` ← 1 file(s)
- `unicode/umachine.h` ← 1 file(s)

### loader (3)

- `loader/LoadedFromOpaqueSource.h` ← 2 file(s)
- `loader/ResourceLoaderOptions.h` ← 1 file(s)
- `loader/cache/CachedResourceHandle.h` ← 1 file(s)

### platform (23)

- `platform/CommonAtomStrings.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 3 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 3 file(s)
- `platform/ThemeTypes.h` ← 1 file(s)
- `platform/TouchAction.h` ← 1 file(s)
- `platform/cocoa/AppleVisualEffect.h` ← 1 file(s)
- `platform/graphics/Color.h` ← 3 file(s)
- `platform/graphics/ColorHash.h` ← 1 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/FontSizeAdjust.h` ← 1 file(s)
- `platform/graphics/FontTaggedSettings.h` ← 1 file(s)
- `platform/graphics/GraphicsTypes.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 1 file(s)
- `platform/graphics/controls/ApplePayButtonPart.h` ← 1 file(s)
- `platform/text/TextFlags.h` ← 2 file(s)
- `platform/text/TextSpacing.h` ← 1 file(s)
- `platform/text/UnicodeBidi.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 3 file(s)

### rendering (4)

- `rendering/style/PositionTryOrder.h` ← 1 file(s)
- `rendering/style/RenderStyle.h` ← 1 file(s)
- `rendering/style/RenderStyleConstants.h` ← 5 file(s)
- `rendering/style/SVGRenderStyleDefs.h` ← 1 file(s)

### wtf (47)

- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 3 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 6 file(s)
- `wtf/CheckedPtr.h` ← 2 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/CompactVariant.h` ← 3 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 6 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 19 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 7 file(s)
- `wtf/HashSet.h` ← 1 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 2 file(s)
- `wtf/MallocSpan.h` ← 1 file(s)
- `wtf/Markable.h` ← 2 file(s)
- `wtf/MathExtras.h` ← 4 file(s)
- `wtf/NeverDestroyed.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 5 file(s)
- `wtf/Ref.h` ← 5 file(s)
- `wtf/RefCounted.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 3 file(s)
- `wtf/StdLibExtras.h` ← 6 file(s)
- `wtf/TZoneMalloc.h` ← 7 file(s)
- `wtf/TypeCasts.h` ← 2 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/Vector.h` ← 8 file(s)
- `wtf/WeakHashMap.h` ← 1 file(s)
- `wtf/WeakHashSet.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 10 file(s)
- `wtf/text/AtomStringHash.h` ← 5 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 5 file(s)
- `wtf/text/WTFString.h` ← 4 file(s)

## Problems

### unresolved (3)

- `CSSPropertyNames.h` ← 6 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 11 file(s)
