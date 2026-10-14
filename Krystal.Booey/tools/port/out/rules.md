# Port graph

Entries: `css/StyleRule.cpp`, `css/StyleRuleImport.cpp`, `css/StyleRuleFunction.cpp`, `css/parser/CSSAtRuleID.cpp`, `css/CSSCounterStyleDescriptors.cpp`, `css/StyleSheet.cpp`, `css/StyleSheetContents.cpp`

- Files to port: **99**
- Boundary `contentextensions`: 1
- Boundary `cssjit`: 2
- Boundary `dom`: 6
- Boundary `external`: 2
- Boundary `loader`: 9
- Boundary `page`: 7
- Boundary `platform`: 7
- Boundary `rendering`: 1
- Boundary `wtf`: 54
- Unresolved: 3

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 7 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 7 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 6 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 5 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 5 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryTypes.h |
| 4 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 4 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 4 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 4 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h |
| 3 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 3 | `css/SelectorFilter.h` |  | style/RuleData.h |
| 3 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 3 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 3 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 3 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/MediaQuery.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `style/PropertyAllowlist.h` |  | style/RuleData.h |
| 2 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 2 | `css/CSSConditionRule.h` |  | css/CSSContainerRule.h, css/CSSMediaRule.h, css/CSSSupportsRule.h |
| 2 | `css/CSSCounterStyle.h` |  | css/CSSCounterStyleRule.h |
| 2 | `css/CSSProperty.h` |  | css/StyleProperties.h, css/parser/CSSParser.h |
| 2 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 2 | `css/CSSRule.h` |  | css/CSSCounterStyleRule.h, css/CSSFontFaceRule.h, css/CSSFontFeatureValuesRule.h, css/CSSFontPaletteValuesRule.h, css/CSSFunctionDeclarations.h, css/CSSGroupingRule.h, css/CSSImportRule.h, css/CSSKeyframeRule.h, css/CSSKeyframesRule.h, css/CSSLayerStatementRule.h, css/CSSNamespaceRule.h, css/CSSNestedDeclarations.h, css/CSSPageRule.h, css/CSSPositionTryRule.h, css/CSSPropertyRule.h, css/CSSStyleRule.h, css/CSSViewTransitionRule.h |
| 2 | `css/CSSRuleList.h` |  | css/CSSStyleSheet.h |
| 2 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/StyleRule.h |
| 2 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.h, style/RuleSet.h |
| 2 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/CSSValueList.h, css/CSSValuePair.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 2 | `css/CSSVariableData.h` |  | css/StyleRule.h, css/StyleRuleFunction.h |
| 2 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 2 | `css/StyleRuleType.h` |  | css/CSSLayerBlockRule.h, css/CSSRule.h, css/StyleRule.h, css/parser/CSSParserContext.h |
| 2 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 2 | `css/parser/CSSCustomPropertySyntax.h` |  | css/StyleRuleFunction.h |
| 2 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/MediaQueryParserContext.h, css/StyleSheetContents.h |
| 2 | `css/parser/CSSParserEnum.h` |  | css/CSSRule.h, css/parser/CSSParser.h |
| 2 | `css/parser/CSSParserMode.h` |  | css/StyleSheet.h, css/parser/CSSParserContext.h |
| 2 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h, css/parser/CSSParser.h |
| 2 | `css/query/ContainerQuery.h` |  | css/StyleRule.h |
| 2 | `css/query/MediaQuery.h` |  | css/CSSStyleSheet.h, css/MediaList.h, css/StyleRule.h, css/StyleRuleImport.h, style/RuleSet.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 2 | `style/RuleData.h` |  | style/RuleSet.h |
| 2 | `style/RuleFeature.h` |  | style/RuleSet.h |
| 1 | `css/CSSContainerRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSCounterStyleDescriptors.h` |  | css/CSSCounterStyle.h, css/CSSCounterStyleDescriptors.cpp |
| 1 | `css/CSSCounterStyleRule.h` |  | css/CSSCounterStyleDescriptors.cpp, css/StyleRule.cpp |
| 1 | `css/CSSFontFaceRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSFontFeatureValuesRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSFontPaletteValuesRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSFunctionDeclarations.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSFunctionRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSGroupingRule.h` |  | css/CSSConditionRule.h, css/CSSFunctionRule.h, css/CSSInternalBaseAppearanceRule.h, css/CSSLayerBlockRule.h, css/CSSScopeRule.h, css/CSSStartingStyleRule.h, css/StyleRule.cpp |
| 1 | `css/CSSImportRule.h` |  | css/StyleRule.cpp, css/StyleSheetContents.cpp |
| 1 | `css/CSSInternalBaseAppearanceRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSKeyframeRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSKeyframesRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSLayerBlockRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSLayerStatementRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSMarkup.h` |  | css/CSSCounterStyleDescriptors.cpp |
| 1 | `css/CSSMediaRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSNamespaceRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSNestedDeclarations.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSPageRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSPositionTryRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSPrimitiveValue.h` |  | css/CSSCounterStyleDescriptors.cpp, css/StylePropertiesInlines.h, css/parser/CSSParserToken.h |
| 1 | `css/CSSPropertyRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSScopeRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSStartingStyleRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSStyleRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSStyleSheet.h` |  | css/StyleRuleImport.cpp, css/StyleSheetContents.cpp |
| 1 | `css/CSSSupportsRule.h` |  | css/StyleRule.cpp |
| 1 | `css/CSSValueList.h` |  | css/CSSCounterStyleDescriptors.cpp |
| 1 | `css/CSSValuePair.h` |  | css/CSSCounterStyleDescriptors.cpp |
| 1 | `css/CSSViewTransitionRule.h` |  | css/StyleRule.cpp |
| 1 | `css/MediaList.h` |  | css/CSSStyleSheet.h, css/StyleRule.cpp, css/StyleRuleImport.cpp, css/StyleSheetContents.cpp |
| 1 | `css/MediaQueryParserContext.h` |  | css/MediaList.h, css/StyleRuleImport.cpp |
| 1 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h, css/StyleRule.cpp, css/StyleRuleFunction.cpp |
| 1 | `css/StyleProperties.h` |  | css/CSSCounterStyleRule.h, css/CSSPositionTryRule.h, css/CSSViewTransitionRule.h, css/ImmutableStyleProperties.h, css/MutableStyleProperties.h, css/StyleRule.cpp, css/StyleSheetContents.cpp |
| 1 | `css/StylePropertiesInlines.h` |  | css/CSSPositionTryRule.h, css/StyleRule.cpp, css/StyleRuleFunction.cpp |
| 1 | `css/StyleRule.h` |  | css/CSSCounterStyleRule.h, css/CSSFontFeatureValuesRule.h, css/CSSKeyframeRule.h, css/CSSKeyframesRule.h, css/CSSPositionTryRule.h, css/CSSViewTransitionRule.h, css/StyleRule.cpp, css/StyleRuleFunction.h, css/StyleRuleImport.h, css/StyleSheetContents.cpp, css/parser/CSSParser.h, style/RuleData.h, style/RuleSet.h |
| 1 | `css/StyleRuleFunction.h` |  | css/StyleRule.cpp, css/StyleRuleFunction.cpp |
| 1 | `css/StyleRuleImport.h` |  | css/StyleRule.cpp, css/StyleRuleImport.cpp, css/StyleSheetContents.cpp |
| 1 | `css/StyleSheet.h` |  | css/CSSStyleSheet.h, css/StyleSheet.cpp |
| 1 | `css/StyleSheetContents.h` |  | css/StyleRule.cpp, css/StyleRuleImport.cpp, css/StyleSheetContents.cpp |
| 1 | `css/parser/CSSAtRuleID.h` |  | css/parser/CSSAtRuleID.cpp |
| 1 | `css/parser/CSSParser.h` |  | css/StyleSheetContents.cpp |
| 1 | `css/values/CSSSerializationContext.h` |  | css/StyleRule.cpp |
| 1 | `style/RuleSet.h` |  | css/StyleSheetContents.cpp |
| 0 | `css/CSSCounterStyleDescriptors.cpp` |  | (entry) |
| 0 | `css/StyleRule.cpp` |  | (entry) |
| 0 | `css/StyleRuleFunction.cpp` |  | (entry) |
| 0 | `css/StyleRuleImport.cpp` |  | (entry) |
| 0 | `css/StyleSheet.cpp` |  | (entry) |
| 0 | `css/StyleSheetContents.cpp` |  | (entry) |
| 0 | `css/parser/CSSAtRuleID.cpp` |  | (entry) |

## Boundaries

### contentextensions (1)

- `contentextensions/ContentRuleListResults.h` ← 1 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (6)

- `dom/DocumentPage.h` ← 2 file(s)
- `dom/DocumentResourceLoader.h` ← 1 file(s)
- `dom/Element.h` ← 2 file(s)
- `dom/ExceptionOr.h` ← 1 file(s)
- `dom/NodeDocument.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)

### external (2)

- `pal/text/TextEncoding.h` ← 1 file(s)
- `unicode/umachine.h` ← 1 file(s)

### loader (9)

- `loader/FrameLoader.h` ← 1 file(s)
- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)
- `loader/ResourceLoadInfo.h` ← 1 file(s)
- `loader/cache/CachePolicy.h` ← 1 file(s)
- `loader/cache/CachedCSSStyleSheet.h` ← 3 file(s)
- `loader/cache/CachedResourceHandle.h` ← 1 file(s)
- `loader/cache/CachedResourceRequest.h` ← 1 file(s)
- `loader/cache/CachedResourceRequestInitiatorTypes.h` ← 1 file(s)
- `loader/cache/CachedStyleSheetClient.h` ← 1 file(s)

### page (7)

- `page/FrameConsoleClient.h` ← 1 file(s)
- `page/FrameDestructionObserverInlines.h` ← 1 file(s)
- `page/LocalFrame.h` ← 1 file(s)
- `page/OriginAccessPatterns.h` ← 1 file(s)
- `page/SecurityOrigin.h` ← 2 file(s)
- `page/UserContentController.h` ← 1 file(s)
- `page/UserContentProvider.h` ← 1 file(s)

### platform (7)

- `platform/CommonAtomStrings.h` ← 3 file(s)
- `platform/LayoutUnit.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 2 file(s)

### rendering (1)

- `rendering/style/RenderStyleConstants.h` ← 2 file(s)

### wtf (54)

- `wtf/AbstractRefCounted.h` ← 1 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 1 file(s)
- `wtf/BloomFilter.h` ← 1 file(s)
- `wtf/Brigand.h` ← 4 file(s)
- `wtf/CheckedPtr.h` ← 2 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/Deque.h` ← 1 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 5 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/Forward.h` ← 14 file(s)
- `wtf/Function.h` ← 2 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 4 file(s)
- `wtf/HashSet.h` ← 1 file(s)
- `wtf/Hasher.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/MallocSpan.h` ← 1 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NeverDestroyed.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 2 file(s)
- `wtf/OptionSet.h` ← 3 file(s)
- `wtf/Ref.h` ← 4 file(s)
- `wtf/RefCounted.h` ← 3 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 3 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 4 file(s)
- `wtf/SortedArrayMap.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 4 file(s)
- `wtf/TZoneMalloc.h` ← 3 file(s)
- `wtf/TZoneMallocInlines.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 6 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/Vector.h` ← 10 file(s)
- `wtf/WeakHashSet.h` ← 1 file(s)
- `wtf/WeakPtr.h` ← 5 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 12 file(s)
- `wtf/text/AtomStringHash.h` ← 4 file(s)
- `wtf/text/MakeString.h` ← 2 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 2 file(s)
- `wtf/text/StringParsingBuffer.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 4 file(s)
- `wtf/text/TextPosition.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 3 file(s)
- `wtf/text/WTFString.h` ← 6 file(s)

## Problems

### unresolved (3)

- `CSSPropertyNames.h` ← 7 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 5 file(s)
