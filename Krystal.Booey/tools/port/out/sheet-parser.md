# Port graph

Entries: `css/parser/CSSParser.cpp`

- Files to port: **123**
- Boundary `cssjit`: 1
- Boundary `dom`: 5
- Boundary `external`: 2
- Boundary `html`: 1
- Boundary `loader`: 4
- Boundary `platform`: 11
- Boundary `rendering`: 1
- Boundary `wtf`: 54
- Unresolved: 3

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 6 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 5 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 5 | `style/values/StyleValueTypes.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 5 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 5 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 5 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 5 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryTypes.h |
| 4 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 4 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 4 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 4 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h |
| 4 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 4 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 3 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/CSSColorValue.h` |  | css/CSSValuePool.h |
| 3 | `css/CSSCounterStyleDescriptors.h` |  | css/CSSCounterStyle.h |
| 3 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 3 | `css/calc/CSSCalcSymbolsAllowed.h` |  | css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 3 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 3 | `css/parser/CSSPropertyParserState.h` |  | css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 3 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryParser.h, css/query/MediaQuery.h |
| 3 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 3 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h, style/values/color/StyleResolvedColor.h |
| 3 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 3 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/values/color/CSSColorDescriptors.h |
| 2 | `css/CSSCounterStyle.h` |  | css/CSSCounterStyleRule.h |
| 2 | `css/CSSPrimitiveValue.h` |  | css/CSSValuePool.h, css/StylePropertiesInlines.h, css/parser/CSSParserToken.h, css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h, css/parser/CSSPropertyParserConsumer+Ident.h |
| 2 | `css/CSSProperty.h` |  | css/StyleProperties.h, css/parser/CSSParser.h, css/parser/CSSPropertyParserState.h |
| 2 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 2 | `css/CSSRule.h` |  | css/CSSCounterStyleRule.h, css/CSSFontFeatureValuesRule.h, css/CSSKeyframeRule.h, css/CSSKeyframesRule.h, css/CSSPositionTryRule.h, css/CSSViewTransitionRule.h |
| 2 | `css/CSSRuleList.h` |  | css/CSSStyleSheet.h |
| 2 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/StyleRule.h, css/parser/MutableCSSSelector.h |
| 2 | `css/CSSSelectorList.h` |  | css/StyleRule.h, css/parser/CSSSelectorParser.h |
| 2 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSColorValue.h, css/CSSCustomPropertyValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/CSSValueList.h, css/CSSValuePair.h, css/CSSVariableReferenceValue.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 2 | `css/CSSValuePool.h` |  | css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSPropertyParserState.h |
| 2 | `css/CSSVariableData.h` |  | css/CSSCustomPropertyValue.h, css/CSSVariableReferenceValue.h, css/StyleRule.h, css/StyleRuleFunction.h |
| 2 | `css/CSSVariableReferenceValue.h` |  | css/CSSCustomPropertyValue.h |
| 2 | `css/CSSWideKeyword.h` |  | css/CSSCustomPropertyValue.h, css/parser/CSSParserIdioms.h |
| 2 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 2 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 2 | `css/StyleProperties.h` |  | css/CSSCounterStyleRule.h, css/CSSPositionTryRule.h, css/CSSViewTransitionRule.h, css/ImmutableStyleProperties.h, css/MutableStyleProperties.h |
| 2 | `css/StyleRuleType.h` |  | css/CSSRule.h, css/StyleRule.h, css/parser/CSSParserContext.h, css/parser/CSSParserObserver.h, css/parser/CSSPropertyParserState.h |
| 2 | `css/StyleSheet.h` |  | css/CSSStyleSheet.h |
| 2 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h, css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h |
| 2 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/MediaQueryParserContext.h, css/StyleSheetContents.h, css/parser/CSSParserIdioms.h, css/parser/CSSSelectorParser.h, css/query/ContainerQueryParser.h, css/query/GenericMediaQueryParser.h |
| 2 | `css/parser/CSSParserMode.h` |  | css/StyleSheet.h, css/parser/CSSParserContext.h, css/parser/CSSPropertyParserConsumer+CounterStyles.h, css/parser/CSSPropertyParserOptions.h, css/parser/CSSSelectorParserContext.h |
| 2 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h, css/CSSVariableReferenceValue.h, css/parser/CSSParser.h, css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h, css/parser/CSSPropertyParserConsumer+MetaResolver.h, css/parser/CSSSelectorParser.h, css/parser/CSSVariableParser.h, css/query/GenericMediaQueryParser.h |
| 2 | `css/parser/CSSPropertyParserConsumer+MetaConsumer.h` |  | css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h |
| 2 | `css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h` |  | css/parser/CSSPropertyParserConsumer+IntegerDefinitions.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaResolver.h |
| 2 | `css/parser/CSSPropertyParserConsumer+MetaResolver.h` |  | css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h |
| 2 | `css/parser/CSSPropertyParserOptions.h` |  | css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h, css/parser/CSSPropertyParserConsumer+MetaResolver.h |
| 2 | `css/parser/CSSSelectorParserContext.h` |  | css/parser/CSSSelectorParser.h |
| 2 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 2 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, css/query/ContainerQueryParser.h |
| 2 | `css/query/GenericMediaQueryParser.h` |  | css/query/ContainerQueryParser.h, css/query/MediaQueryParser.h |
| 2 | `css/query/MediaQuery.h` |  | css/CSSStyleSheet.h, css/MediaList.h, css/StyleRule.h, css/StyleRuleImport.h, css/query/MediaQueryParser.h |
| 2 | `css/values/color/CSSColor.h` |  | css/CSSColorValue.h, style/values/color/StyleColor.h |
| 2 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h, style/values/color/StyleColor.h |
| 2 | `css/values/color/CSSColorType.h` |  | style/values/color/StyleColor.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h, css/parser/CSSPropertyParserConsumer+Font.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h, css/parser/CSSPropertyParserConsumer+MetaResolver.h, css/values/color/CSSColorDescriptors.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h, css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 2 | `style/values/color/StyleColorOptions.h` |  | style/values/color/StyleColor.h |
| 2 | `style/values/color/StyleCurrentColor.h` |  | style/values/color/StyleColor.h |
| 2 | `style/values/color/StyleResolvedColor.h` |  | style/values/color/StyleColor.h |
| 1 | `css/CSSCounterStyleRule.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/CSSCustomPropertyValue.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/CSSFontFeatureValuesRule.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/CSSKeyframeRule.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/CSSKeyframesRule.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/CSSPositionTryRule.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/CSSStyleSheet.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/CSSValueList.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/CSSValuePair.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/CSSViewTransitionRule.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/ComputedStyleDependencies.h` |  | css/parser/CSSParser.cpp, css/values/CSSValueTypes.h |
| 1 | `css/MediaList.h` |  | css/CSSStyleSheet.h, css/parser/CSSParser.cpp |
| 1 | `css/MediaQueryParserContext.h` |  | css/MediaList.h, css/parser/CSSParser.cpp, css/query/GenericMediaQueryParser.h, css/query/MediaQueryParser.h |
| 1 | `css/StylePropertiesInlines.h` |  | css/CSSPositionTryRule.h, css/parser/CSSParser.cpp |
| 1 | `css/StyleRule.h` |  | css/CSSCounterStyleRule.h, css/CSSFontFeatureValuesRule.h, css/CSSKeyframeRule.h, css/CSSKeyframesRule.h, css/CSSPositionTryRule.h, css/CSSViewTransitionRule.h, css/StyleRuleFunction.h, css/StyleRuleImport.h, css/parser/CSSParser.cpp, css/parser/CSSParser.h |
| 1 | `css/StyleRuleFunction.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/StyleRuleImport.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/StyleSheetContents.h` |  | css/parser/CSSParser.cpp, css/parser/CSSSelectorParser.h |
| 1 | `css/parser/CSSAtRuleID.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/parser/CSSCustomPropertySyntax.h` |  | css/StyleRuleFunction.h, css/parser/CSSParser.cpp |
| 1 | `css/parser/CSSParser.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/parser/CSSParserEnum.h` |  | css/CSSRule.h, css/parser/CSSParser.cpp, css/parser/CSSParser.h, css/parser/CSSSelectorParser.h, css/parser/CSSSupportsParser.h |
| 1 | `css/parser/CSSParserFastPaths.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/parser/CSSParserIdioms.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/parser/CSSParserObserver.h` |  | css/parser/CSSParser.cpp, css/parser/CSSParserObserverWrapper.h |
| 1 | `css/parser/CSSParserObserverWrapper.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParser.cpp, css/parser/CSSParserTokenRange.h, css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSPropertyParserConsumer+MetaConsumer.h, css/parser/CSSPropertyParserConsumer+MetaResolver.h, css/parser/CSSSupportsParser.h, css/parser/CSSTokenizer.h, css/query/ContainerQueryParser.h |
| 1 | `css/parser/CSSPropertyParser.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Animations.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+CSSPrimitiveValueResolver.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+CounterStyles.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Font.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Ident.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+IntegerDefinitions.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Primitives.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Timeline.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/parser/CSSSelectorParser.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/parser/CSSSupportsParser.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParser.cpp, css/parser/CSSParserTokenRange.h |
| 1 | `css/parser/CSSVariableParser.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/parser/MutableCSSSelector.h` |  | css/parser/CSSParser.cpp, css/parser/CSSSelectorParser.h |
| 1 | `css/query/ContainerQueryParser.h` |  | css/parser/CSSParser.cpp |
| 1 | `css/query/MediaQueryParser.h` |  | css/parser/CSSParser.cpp |
| 1 | `style/values/color/StyleColor.h` |  | css/parser/CSSParser.cpp |
| 0 | `css/parser/CSSParser.cpp` |  | (entry) |

## Boundaries

### cssjit (1)

- `cssjit/CompiledSelector.h` ← 1 file(s)

### dom (5)

- `dom/Document.h` ← 1 file(s)
- `dom/Element.h` ← 2 file(s)
- `dom/ExceptionOr.h` ← 1 file(s)
- `dom/NodeDocument.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)

### external (2)

- `pal/text/TextEncoding.h` ← 1 file(s)
- `unicode/umachine.h` ← 1 file(s)

### html (1)

- `html/parser/NestingLevelIncrementer.h` ← 1 file(s)

### loader (4)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)
- `loader/cache/CachedCSSStyleSheet.h` ← 1 file(s)
- `loader/cache/CachedResourceHandle.h` ← 1 file(s)
- `loader/cache/CachedStyleSheetClient.h` ← 1 file(s)

### platform (11)

- `platform/CommonAtomStrings.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/graphics/Color.h` ← 5 file(s)
- `platform/graphics/ColorHash.h` ← 1 file(s)
- `platform/graphics/ColorTypes.h` ← 3 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 2 file(s)
- `platform/graphics/SystemFontDatabase.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 2 file(s)

### rendering (1)

- `rendering/style/RenderStyleConstants.h` ← 1 file(s)

### wtf (54)

- `wtf/ASCIICType.h` ← 1 file(s)
- `wtf/AbstractRefCounted.h` ← 1 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 1 file(s)
- `wtf/Brigand.h` ← 8 file(s)
- `wtf/CheckedPtr.h` ← 2 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 5 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 24 file(s)
- `wtf/Function.h` ← 2 file(s)
- `wtf/HashFunctions.h` ← 2 file(s)
- `wtf/HashMap.h` ← 4 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/MallocSpan.h` ← 1 file(s)
- `wtf/Markable.h` ← 3 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NeverDestroyed.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 2 file(s)
- `wtf/OptionSet.h` ← 5 file(s)
- `wtf/PointerComparison.h` ← 1 file(s)
- `wtf/Ref.h` ← 4 file(s)
- `wtf/RefCounted.h` ← 3 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 4 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 8 file(s)
- `wtf/RobinHoodHashMap.h` ← 1 file(s)
- `wtf/SetForScope.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 9 file(s)
- `wtf/TZoneMalloc.h` ← 4 file(s)
- `wtf/TZoneMallocInlines.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 6 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 1 file(s)
- `wtf/Vector.h` ← 10 file(s)
- `wtf/WeakHashSet.h` ← 1 file(s)
- `wtf/WeakPtr.h` ← 4 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 12 file(s)
- `wtf/text/AtomStringHash.h` ← 5 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringParsingBuffer.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 4 file(s)
- `wtf/text/TextPosition.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 2 file(s)
- `wtf/text/WTFString.h` ← 5 file(s)

## Problems

### unresolved (3)

- `CSSPropertyNames.h` ← 8 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 14 file(s)
