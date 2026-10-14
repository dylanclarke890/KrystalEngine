# Port graph

Entries: `css/CSSConditionRule.cpp`, `css/CSSContainerRule.cpp`, `css/CSSCounterStyleRule.cpp`, `css/CSSFontFaceRule.cpp`, `css/CSSFontFeatureValuesRule.cpp`, `css/CSSFontPaletteValuesRule.cpp`, `css/CSSFunctionDeclarations.cpp`, `css/CSSFunctionRule.cpp`, `css/CSSGroupingRule.cpp`, `css/CSSImportRule.cpp`, `css/CSSInternalBaseAppearanceRule.cpp`, `css/CSSKeyframeRule.cpp`, `css/CSSKeyframesRule.cpp`, `css/CSSLayerBlockRule.cpp`, `css/CSSLayerStatementRule.cpp`, `css/CSSMediaRule.cpp`, `css/CSSNamespaceRule.cpp`, `css/CSSNestedDeclarations.cpp`, `css/CSSPageRule.cpp`, `css/CSSPositionTryRule.cpp`, `css/CSSPropertyRule.cpp`, `css/CSSRule.cpp`, `css/CSSRuleList.cpp`, `css/CSSScopeRule.cpp`, `css/CSSStartingStyleRule.cpp`, `css/CSSStyleDeclaration.cpp`, `css/CSSStyleProperties.cpp`, `css/CSSStyleRule.cpp`, `css/CSSStyleSheet.cpp`, `css/CSSSupportsRule.cpp`, `css/CSSViewTransitionRule.cpp`, `css/MediaList.cpp`, `css/PropertySetCSSDescriptors.cpp`, `css/StyleSheet.cpp`, `css/StyleSheetList.cpp`

- Files to port: **183**
- Boundary `animation`: 1
- Boundary `bindings`: 6
- Boundary `cssjit`: 2
- Boundary `dom`: 15
- Boundary `external`: 2
- Boundary `html`: 2
- Boundary `inspector`: 1
- Boundary `loader`: 4
- Boundary `page`: 4
- Boundary `platform`: 14
- Boundary `rendering`: 3
- Boundary `svg`: 1
- Boundary `wtf`: 65
- Unresolved: 7

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 7 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 7 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 6 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 5 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 5 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 5 | `css/typedom/CSSStyleValue.h` |  | css/typedom/StylePropertyMapReadOnly.h |
| 5 | `css/values/CSSNoConversionDataRequiredToken.h` |  | style/values/StyleValueTypes.h |
| 5 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 5 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 5 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h |
| 5 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 5 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 4 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 4 | `css/typedom/StylePropertyMapReadOnly.h` |  | css/typedom/MainThreadStylePropertyMapReadOnly.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleRelations.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h, style/PseudoElementRequest.h |
| 4 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h |
| 3 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h |
| 3 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 3 | `css/SelectorChecker.h` |  | style/ElementRuleCollector.h, style/MatchResult.h |
| 3 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 3 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/query/GenericMediaQueryEvaluator.h` |  | css/query/MediaQueryEvaluator.h, style/ContainerQueryEvaluator.h |
| 3 | `css/typedom/MainThreadStylePropertyMapReadOnly.h` |  | css/typedom/StylePropertyMap.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 3 | `style/MatchResult.h` |  | style/ElementRuleCollector.h, style/MatchedDeclarationsCache.h, style/PropertyCascade.h |
| 3 | `style/PropertyAllowlist.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleData.h |
| 3 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h, style/PseudoElementRequest.h, style/Styleable.h |
| 3 | `style/PseudoElementRequest.h` |  | style/ElementRuleCollector.h |
| 3 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/UserAgentStyle.h` |  | style/StyleScopeRuleSets.h |
| 2 | `css/CSSCounterStyle.h` |  | css/CSSCounterStyleRule.h |
| 2 | `css/CSSPrimitiveValue.h` |  | css/StylePropertiesInlines.h, css/parser/CSSParserToken.h |
| 2 | `css/CSSProperty.h` |  | css/CSSStyleDeclaration.h, css/CSSStyleProperties.h, css/PropertySetCSSDescriptors.h, css/StyleProperties.h, css/parser/CSSParser.h |
| 2 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 2 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, css/parser/MutableCSSSelector.h, style/HasSelectorFilter.h, style/StyleResolver.h |
| 2 | `css/CSSSelectorList.h` |  | css/StyleRule.h, css/parser/CSSSelectorParser.h, style/RuleFeature.h, style/RuleSet.h |
| 2 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/CSSValueList.h, css/CSSValuePair.h, css/DeprecatedCSSOMValue.h, css/query/GenericMediaQueryTypes.h, css/typedom/CSSStyleValue.h, css/typedom/StylePropertyMapReadOnly.h, css/values/CSSValueTypes.h |
| 2 | `css/CSSVariableData.h` |  | css/StyleRule.h, css/StyleRuleFunction.h |
| 2 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 2 | `css/MediaQueryParserContext.h` |  | css/MediaList.h, css/query/GenericMediaQueryParser.h, css/query/MediaQueryParser.h |
| 2 | `css/StyleRuleType.h` |  | css/CSSLayerBlockRule.h, css/CSSRule.h, css/CSSStyleProperties.h, css/StyleRule.h, css/parser/CSSParserContext.h |
| 2 | `css/parser/CSSCustomPropertySyntax.h` |  | css/StyleRuleFunction.h |
| 2 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/MediaQueryParserContext.h, css/StyleSheetContents.h, css/parser/CSSSelectorParser.h, css/query/GenericMediaQueryParser.h, style/StyleSheetContentsCache.h |
| 2 | `css/parser/CSSParserMode.h` |  | css/StyleSheet.h, css/parser/CSSParserContext.h, css/parser/CSSPropertyParserConsumer+CounterStyles.h, css/parser/CSSSelectorParserContext.h |
| 2 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 2 | `css/parser/CSSSelectorParserContext.h` |  | css/parser/CSSSelectorParser.h |
| 2 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 2 | `css/parser/MutableCSSSelector.h` |  | css/parser/CSSSelectorParser.h |
| 2 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 2 | `css/query/GenericMediaQueryParser.h` |  | css/query/MediaQueryParser.h |
| 2 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryParser.h, css/query/GenericMediaQuerySerialization.h, css/query/MediaQuery.h |
| 2 | `css/query/MediaQueryEvaluator.h` |  | style/ElementRuleCollector.h, style/StyleResolver.h |
| 2 | `css/typedom/StylePropertyMap.h` |  | css/typedom/DeclaredStylePropertyMap.h |
| 2 | `style/AnchorPositionEvaluator.h` |  | style/StyleScope.h, style/TreeResolutionState.h |
| 2 | `style/ElementRuleCollector.h` |  | style/StyleResolver.h |
| 2 | `style/InspectorCSSOMWrappers.h` |  | style/StyleResolver.h |
| 2 | `style/MatchedDeclarationsCache.h` |  | style/StyleResolver.h |
| 2 | `style/PropertyCascade.h` |  | style/StyleResolver.h |
| 2 | `style/RuleData.h` |  | style/RuleSet.h |
| 2 | `style/RuleFeature.h` |  | style/RuleSet.h, style/StyleScopeRuleSets.h |
| 2 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h, style/StyleScope.h |
| 2 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/ElementRuleCollector.h, style/MatchResult.h, style/ScopedName.h, style/StyleScope.h |
| 2 | `style/StyleScopeRuleSets.h` |  | style/StyleResolver.h |
| 2 | `style/Styleable.h` |  | style/StyleScope.h |
| 2 | `style/TreeResolutionState.h` |  | style/StyleResolver.h |
| 1 | `css/CSSConditionRule.h` |  | css/CSSConditionRule.cpp, css/CSSContainerRule.h, css/CSSMediaRule.h, css/CSSSupportsRule.h |
| 1 | `css/CSSContainerRule.h` |  | css/CSSContainerRule.cpp |
| 1 | `css/CSSCounterStyleDescriptors.h` |  | css/CSSCounterStyle.h, css/CSSCounterStyleRule.cpp |
| 1 | `css/CSSCounterStyleRule.h` |  | css/CSSCounterStyleRule.cpp |
| 1 | `css/CSSFontFaceDescriptors.h` |  | css/CSSFontFaceRule.cpp |
| 1 | `css/CSSFontFaceRule.h` |  | css/CSSFontFaceRule.cpp |
| 1 | `css/CSSFontFeatureValuesRule.h` |  | css/CSSFontFeatureValuesRule.cpp |
| 1 | `css/CSSFontPaletteValuesRule.h` |  | css/CSSFontPaletteValuesRule.cpp |
| 1 | `css/CSSFunctionDeclarations.h` |  | css/CSSFunctionDeclarations.cpp |
| 1 | `css/CSSFunctionDescriptors.h` |  | css/CSSFunctionDeclarations.cpp |
| 1 | `css/CSSFunctionRule.h` |  | css/CSSFunctionRule.cpp |
| 1 | `css/CSSGroupingRule.h` |  | css/CSSConditionRule.h, css/CSSFunctionRule.h, css/CSSGroupingRule.cpp, css/CSSInternalBaseAppearanceRule.h, css/CSSLayerBlockRule.h, css/CSSScopeRule.h, css/CSSStartingStyleRule.h, css/CSSStyleRule.cpp |
| 1 | `css/CSSImportRule.h` |  | css/CSSImportRule.cpp, css/CSSStyleSheet.cpp, css/MediaList.cpp |
| 1 | `css/CSSInternalBaseAppearanceRule.h` |  | css/CSSInternalBaseAppearanceRule.cpp |
| 1 | `css/CSSKeyframeRule.h` |  | css/CSSKeyframeRule.cpp, css/CSSKeyframesRule.cpp |
| 1 | `css/CSSKeyframesRule.h` |  | css/CSSKeyframeRule.cpp, css/CSSKeyframesRule.cpp, css/CSSStyleSheet.cpp |
| 1 | `css/CSSLayerBlockRule.h` |  | css/CSSImportRule.cpp, css/CSSLayerBlockRule.cpp, css/CSSLayerStatementRule.cpp |
| 1 | `css/CSSLayerStatementRule.h` |  | css/CSSLayerStatementRule.cpp |
| 1 | `css/CSSMarkup.h` |  | css/CSSContainerRule.cpp, css/CSSFontFeatureValuesRule.cpp, css/CSSFontPaletteValuesRule.cpp, css/CSSFunctionRule.cpp, css/CSSImportRule.cpp, css/CSSLayerBlockRule.cpp, css/CSSNamespaceRule.cpp, css/CSSPropertyRule.cpp |
| 1 | `css/CSSMediaRule.h` |  | css/CSSMediaRule.cpp, css/MediaList.cpp |
| 1 | `css/CSSNamespaceRule.h` |  | css/CSSNamespaceRule.cpp |
| 1 | `css/CSSNestedDeclarations.h` |  | css/CSSNestedDeclarations.cpp |
| 1 | `css/CSSPageDescriptors.h` |  | css/CSSPageRule.cpp |
| 1 | `css/CSSPageRule.h` |  | css/CSSPageRule.cpp |
| 1 | `css/CSSPositionTryDescriptors.h` |  | css/CSSPositionTryRule.cpp |
| 1 | `css/CSSPositionTryRule.h` |  | css/CSSPositionTryRule.cpp |
| 1 | `css/CSSPropertyRule.h` |  | css/CSSPropertyRule.cpp |
| 1 | `css/CSSRule.h` |  | css/CSSCounterStyleRule.h, css/CSSFontFaceRule.h, css/CSSFontFeatureValuesRule.h, css/CSSFontPaletteValuesRule.h, css/CSSFunctionDeclarations.h, css/CSSGroupingRule.h, css/CSSImportRule.h, css/CSSKeyframeRule.h, css/CSSKeyframesRule.h, css/CSSLayerStatementRule.h, css/CSSNamespaceRule.h, css/CSSNestedDeclarations.h, css/CSSPageRule.h, css/CSSPositionTryRule.h, css/CSSPropertyRule.h, css/CSSRule.cpp, css/CSSRuleList.cpp, css/CSSStyleProperties.cpp, css/CSSStyleRule.h, css/CSSViewTransitionRule.h, css/PropertySetCSSDescriptors.cpp |
| 1 | `css/CSSRuleList.h` |  | css/CSSGroupingRule.cpp, css/CSSKeyframesRule.cpp, css/CSSRuleList.cpp, css/CSSStyleRule.cpp, css/CSSStyleSheet.cpp, css/CSSStyleSheet.h |
| 1 | `css/CSSScopeRule.h` |  | css/CSSRule.cpp, css/CSSScopeRule.cpp |
| 1 | `css/CSSStartingStyleRule.h` |  | css/CSSStartingStyleRule.cpp |
| 1 | `css/CSSStyleDeclaration.h` |  | css/CSSStyleDeclaration.cpp, css/CSSStyleProperties.h, css/DeprecatedCSSOMValue.h, css/PropertySetCSSDescriptors.h |
| 1 | `css/CSSStyleProperties.h` |  | css/CSSKeyframeRule.cpp, css/CSSNestedDeclarations.cpp, css/CSSStyleProperties.cpp, css/CSSStyleRule.cpp |
| 1 | `css/CSSStyleRule.h` |  | css/CSSRule.cpp, css/CSSStyleRule.cpp |
| 1 | `css/CSSStyleSheet.h` |  | css/CSSContainerRule.cpp, css/CSSCounterStyleRule.cpp, css/CSSGroupingRule.cpp, css/CSSImportRule.cpp, css/CSSKeyframesRule.cpp, css/CSSLayerBlockRule.cpp, css/CSSLayerStatementRule.cpp, css/CSSMediaRule.cpp, css/CSSPageRule.cpp, css/CSSPropertyRule.cpp, css/CSSRule.cpp, css/CSSStyleProperties.cpp, css/CSSStyleRule.cpp, css/CSSStyleSheet.cpp, css/CSSSupportsRule.cpp, css/CSSViewTransitionRule.cpp, css/MediaList.cpp, css/PropertySetCSSDescriptors.cpp, css/StyleSheetList.cpp |
| 1 | `css/CSSSupportsRule.h` |  | css/CSSSupportsRule.cpp |
| 1 | `css/CSSValueList.h` |  | css/CSSViewTransitionRule.cpp, css/DeprecatedCSSOMValueList.h |
| 1 | `css/CSSValuePair.h` |  | css/CSSCounterStyleRule.cpp, css/CSSViewTransitionRule.cpp |
| 1 | `css/CSSViewTransitionRule.h` |  | css/CSSViewTransitionRule.cpp |
| 1 | `css/DeprecatedCSSOMValue.h` |  | css/CSSStyleProperties.cpp, css/DeprecatedCSSOMValueList.h, css/PropertySetCSSDescriptors.cpp |
| 1 | `css/DeprecatedCSSOMValueList.h` |  | css/PropertySetCSSDescriptors.cpp |
| 1 | `css/MediaList.h` |  | css/CSSImportRule.cpp, css/CSSMediaRule.cpp, css/CSSStyleSheet.cpp, css/CSSStyleSheet.h, css/MediaList.cpp |
| 1 | `css/MutableStyleProperties.h` |  | css/CSSCounterStyleRule.cpp, css/CSSFontFaceRule.cpp, css/CSSFunctionDeclarations.cpp, css/CSSKeyframeRule.cpp, css/CSSNestedDeclarations.cpp, css/CSSStyleProperties.cpp, css/CSSStyleRule.cpp, css/CSSViewTransitionRule.cpp, css/PropertySetCSSDescriptors.cpp, css/StylePropertiesInlines.h |
| 1 | `css/PropertySetCSSDescriptors.h` |  | css/CSSFontFaceDescriptors.h, css/CSSFunctionDescriptors.h, css/CSSPageDescriptors.h, css/CSSPositionTryDescriptors.h, css/PropertySetCSSDescriptors.cpp |
| 1 | `css/StyleAttributeMutationScope.h` |  | css/CSSStyleProperties.cpp, css/PropertySetCSSDescriptors.cpp |
| 1 | `css/StyleProperties.h` |  | css/CSSCounterStyleRule.cpp, css/CSSCounterStyleRule.h, css/CSSFontFaceRule.cpp, css/CSSFontPaletteValuesRule.cpp, css/CSSKeyframeRule.cpp, css/CSSNestedDeclarations.cpp, css/CSSPageRule.cpp, css/CSSPositionTryRule.h, css/CSSStyleProperties.cpp, css/CSSStyleRule.cpp, css/CSSSupportsRule.cpp, css/CSSViewTransitionRule.cpp, css/CSSViewTransitionRule.h, css/ImmutableStyleProperties.h, css/MutableStyleProperties.h |
| 1 | `css/StylePropertiesInlines.h` |  | css/CSSCounterStyleRule.cpp, css/CSSFunctionDeclarations.cpp, css/CSSGroupingRule.cpp, css/CSSKeyframeRule.cpp, css/CSSPositionTryRule.h, css/CSSViewTransitionRule.cpp, style/MatchResult.h |
| 1 | `css/StyleRule.h` |  | css/CSSContainerRule.cpp, css/CSSCounterStyleRule.h, css/CSSFontFaceRule.cpp, css/CSSFontFeatureValuesRule.h, css/CSSFontPaletteValuesRule.cpp, css/CSSGroupingRule.cpp, css/CSSInternalBaseAppearanceRule.cpp, css/CSSKeyframeRule.h, css/CSSKeyframesRule.h, css/CSSLayerBlockRule.cpp, css/CSSLayerStatementRule.cpp, css/CSSMediaRule.cpp, css/CSSNamespaceRule.cpp, css/CSSNestedDeclarations.cpp, css/CSSPageRule.cpp, css/CSSPositionTryRule.h, css/CSSPropertyRule.cpp, css/CSSRule.cpp, css/CSSScopeRule.cpp, css/CSSStartingStyleRule.cpp, css/CSSStyleRule.cpp, css/CSSStyleSheet.cpp, css/CSSSupportsRule.cpp, css/CSSViewTransitionRule.h, css/StyleRuleFunction.h, css/StyleRuleImport.h, css/parser/CSSParser.h, style/RuleData.h, style/RuleSet.h |
| 1 | `css/StyleRuleFunction.h` |  | css/CSSFunctionDeclarations.cpp, css/CSSFunctionRule.cpp |
| 1 | `css/StyleRuleImport.h` |  | css/CSSImportRule.cpp |
| 1 | `css/StyleSheet.h` |  | css/CSSStyleSheet.h, css/StyleSheet.cpp |
| 1 | `css/StyleSheetContents.h` |  | css/CSSImportRule.cpp, css/CSSPageRule.cpp, css/CSSRule.cpp, css/CSSStyleProperties.cpp, css/CSSStyleRule.cpp, css/CSSStyleSheet.cpp, css/PropertySetCSSDescriptors.cpp, css/parser/CSSSelectorParser.h |
| 1 | `css/StyleSheetList.h` |  | css/StyleSheetList.cpp |
| 1 | `css/parser/CSSParser.h` |  | css/CSSGroupingRule.cpp, css/CSSKeyframesRule.cpp, css/CSSStyleRule.cpp, css/CSSStyleSheet.cpp |
| 1 | `css/parser/CSSParserEnum.h` |  | css/CSSRule.cpp, css/CSSRule.h, css/CSSStyleRule.cpp, css/parser/CSSParser.h, css/parser/CSSSelectorParser.h |
| 1 | `css/parser/CSSParserTokenRange.h` |  | css/CSSPropertyRule.cpp, css/CSSVariableData.h, css/parser/CSSParser.h, css/parser/CSSSelectorParser.h, css/query/GenericMediaQueryParser.h |
| 1 | `css/parser/CSSPropertyParser.h` |  | css/CSSCounterStyleRule.cpp, css/CSSStyleProperties.cpp, css/CSSViewTransitionRule.cpp, css/PropertySetCSSDescriptors.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Animations.h` |  | css/CSSKeyframeRule.cpp, css/CSSKeyframesRule.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+CounterStyles.h` |  | css/CSSCounterStyleRule.cpp |
| 1 | `css/parser/CSSSelectorParser.h` |  | css/CSSPageRule.cpp, css/CSSStyleRule.cpp |
| 1 | `css/parser/CSSTokenizer.h` |  | css/CSSCounterStyleRule.cpp, css/CSSViewTransitionRule.cpp, css/parser/CSSParserTokenRange.h |
| 1 | `css/query/GenericMediaQuerySerialization.h` |  | css/CSSContainerRule.cpp |
| 1 | `css/query/MediaQuery.h` |  | css/CSSStyleSheet.h, css/MediaList.cpp, css/MediaList.h, css/StyleRule.h, css/StyleRuleImport.h, css/query/MediaQueryEvaluator.h, css/query/MediaQueryParser.h, style/RuleSet.h |
| 1 | `css/query/MediaQueryParser.h` |  | css/CSSImportRule.cpp, css/CSSMediaRule.cpp, css/CSSStyleSheet.cpp, css/MediaList.cpp |
| 1 | `css/typedom/DeclaredStylePropertyMap.h` |  | css/CSSNestedDeclarations.cpp, css/CSSStyleRule.cpp |
| 1 | `css/values/CSSSerializationContext.h` |  | css/CSSFontFaceRule.cpp, css/CSSFunctionDeclarations.cpp, css/CSSImportRule.cpp, css/CSSKeyframeRule.cpp, css/CSSNestedDeclarations.cpp, css/CSSPageRule.cpp, css/CSSPositionTryRule.cpp, css/CSSStyleProperties.cpp, css/CSSStyleRule.cpp, css/PropertySetCSSDescriptors.cpp |
| 1 | `style/RuleSet.h` |  | css/CSSStyleRule.cpp, style/ElementRuleCollector.h, style/MatchResult.h, style/StyleResolver.h, style/StyleScopeRuleSets.h |
| 1 | `style/StyleResolver.h` |  | css/CSSStyleSheet.cpp |
| 1 | `style/StyleScope.h` |  | css/CSSStyleSheet.cpp, css/StyleSheetList.cpp |
| 1 | `style/StyleSheetContentsCache.h` |  | css/CSSStyleSheet.cpp |
| 0 | `css/CSSConditionRule.cpp` |  | (entry) |
| 0 | `css/CSSContainerRule.cpp` |  | (entry) |
| 0 | `css/CSSCounterStyleRule.cpp` |  | (entry) |
| 0 | `css/CSSFontFaceRule.cpp` |  | (entry) |
| 0 | `css/CSSFontFeatureValuesRule.cpp` |  | (entry) |
| 0 | `css/CSSFontPaletteValuesRule.cpp` |  | (entry) |
| 0 | `css/CSSFunctionDeclarations.cpp` |  | (entry) |
| 0 | `css/CSSFunctionRule.cpp` |  | (entry) |
| 0 | `css/CSSGroupingRule.cpp` |  | (entry) |
| 0 | `css/CSSImportRule.cpp` |  | (entry) |
| 0 | `css/CSSInternalBaseAppearanceRule.cpp` |  | (entry) |
| 0 | `css/CSSKeyframeRule.cpp` |  | (entry) |
| 0 | `css/CSSKeyframesRule.cpp` |  | (entry) |
| 0 | `css/CSSLayerBlockRule.cpp` |  | (entry) |
| 0 | `css/CSSLayerStatementRule.cpp` |  | (entry) |
| 0 | `css/CSSMediaRule.cpp` |  | (entry) |
| 0 | `css/CSSNamespaceRule.cpp` |  | (entry) |
| 0 | `css/CSSNestedDeclarations.cpp` |  | (entry) |
| 0 | `css/CSSPageRule.cpp` |  | (entry) |
| 0 | `css/CSSPositionTryRule.cpp` |  | (entry) |
| 0 | `css/CSSPropertyRule.cpp` |  | (entry) |
| 0 | `css/CSSRule.cpp` |  | (entry) |
| 0 | `css/CSSRuleList.cpp` |  | (entry) |
| 0 | `css/CSSScopeRule.cpp` |  | (entry) |
| 0 | `css/CSSStartingStyleRule.cpp` |  | (entry) |
| 0 | `css/CSSStyleDeclaration.cpp` |  | (entry) |
| 0 | `css/CSSStyleProperties.cpp` |  | (entry) |
| 0 | `css/CSSStyleRule.cpp` |  | (entry) |
| 0 | `css/CSSStyleSheet.cpp` |  | (entry) |
| 0 | `css/CSSSupportsRule.cpp` |  | (entry) |
| 0 | `css/CSSViewTransitionRule.cpp` |  | (entry) |
| 0 | `css/MediaList.cpp` |  | (entry) |
| 0 | `css/PropertySetCSSDescriptors.cpp` |  | (entry) |
| 0 | `css/StyleSheet.cpp` |  | (entry) |
| 0 | `css/StyleSheetList.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/WebAnimationTypes.h` ← 2 file(s)

### bindings (6)

- `bindings/js/JSDOMGlobalObject.h` ← 1 file(s)
- `bindings/js/JSDOMMapLike.h` ← 1 file(s)
- `bindings/js/JSDOMPromiseDeferred.h` ← 1 file(s)
- `bindings/js/JSDOMWindowBase.h` ← 1 file(s)
- `bindings/js/ScriptWrappable.h` ← 2 file(s)
- `bindings/js/ScriptWrappableInlines.h` ← 1 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (15)

- `dom/CustomElementReactionQueue.h` ← 1 file(s)
- `dom/Document.h` ← 8 file(s)
- `dom/DocumentSecurityOrigin.h` ← 1 file(s)
- `dom/Element.h` ← 5 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/ExceptionOr.h` ← 2 file(s)
- `dom/MutationObserverInterestGroup.h` ← 1 file(s)
- `dom/MutationRecord.h` ← 1 file(s)
- `dom/Node.h` ← 2 file(s)
- `dom/NodeDocument.h` ← 1 file(s)
- `dom/NodeInlines.h` ← 2 file(s)
- `dom/PseudoElement.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)
- `dom/ShadowRoot.h` ← 2 file(s)
- `dom/StyledElement.h` ← 4 file(s)

### external (2)

- `pal/text/TextEncoding.h` ← 1 file(s)
- `unicode/umachine.h` ← 1 file(s)

### html (2)

- `html/HTMLLinkElement.h` ← 1 file(s)
- `html/HTMLStyleElement.h` ← 2 file(s)

### inspector (1)

- `inspector/InspectorInstrumentation.h` ← 1 file(s)

### loader (4)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)
- `loader/cache/CachedCSSStyleSheet.h` ← 2 file(s)
- `loader/cache/CachedResourceHandle.h` ← 1 file(s)
- `loader/cache/CachedStyleSheetClient.h` ← 1 file(s)

### page (4)

- `page/LocalDOMWindow.h` ← 2 file(s)
- `page/OriginAccessPatterns.h` ← 1 file(s)
- `page/Quirks.h` ← 1 file(s)
- `page/SecurityOrigin.h` ← 1 file(s)

### platform (14)

- `platform/CommonAtomStrings.h` ← 3 file(s)
- `platform/LayoutUnit.h` ← 3 file(s)
- `platform/Logging.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 1 file(s)
- `platform/Timer.h` ← 2 file(s)
- `platform/graphics/ColorSerialization.h` ← 1 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutRect.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 2 file(s)
- `platform/text/WritingMode.h` ← 3 file(s)

### rendering (3)

- `rendering/style/PositionTryOrder.h` ← 1 file(s)
- `rendering/style/RenderStyle.h` ← 2 file(s)
- `rendering/style/RenderStyleConstants.h` ← 5 file(s)

### svg (1)

- `svg/SVGStyleElement.h` ← 1 file(s)

### wtf (65)

- `wtf/AbstractRefCounted.h` ← 1 file(s)
- `wtf/AbstractRefCountedAndCanMakeWeakPtr.h` ← 1 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 2 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 4 file(s)
- `wtf/CanMakeWeakPtr.h` ← 1 file(s)
- `wtf/CheckedPtr.h` ← 4 file(s)
- `wtf/CheckedRef.h` ← 2 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 6 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/Forward.h` ← 21 file(s)
- `wtf/Function.h` ← 2 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 2 file(s)
- `wtf/HashMap.h` ← 15 file(s)
- `wtf/HashSet.h` ← 3 file(s)
- `wtf/Hasher.h` ← 3 file(s)
- `wtf/HexNumber.h` ← 1 file(s)
- `wtf/Identified.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 3 file(s)
- `wtf/MallocSpan.h` ← 1 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NeverDestroyed.h` ← 3 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 3 file(s)
- `wtf/Noncopyable.h` ← 2 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 5 file(s)
- `wtf/OptionalOrReference.h` ← 1 file(s)
- `wtf/Ref.h` ← 4 file(s)
- `wtf/RefCounted.h` ← 8 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 4 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 11 file(s)
- `wtf/RobinHoodHashMap.h` ← 1 file(s)
- `wtf/SetForScope.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 5 file(s)
- `wtf/TZoneMalloc.h` ← 10 file(s)
- `wtf/TZoneMallocInlines.h` ← 4 file(s)
- `wtf/TypeCasts.h` ← 6 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 2 file(s)
- `wtf/Vector.h` ← 16 file(s)
- `wtf/WeakHashMap.h` ← 2 file(s)
- `wtf/WeakHashSet.h` ← 3 file(s)
- `wtf/WeakListHashSet.h` ← 1 file(s)
- `wtf/WeakPtr.h` ← 11 file(s)
- `wtf/WeakRef.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 16 file(s)
- `wtf/text/AtomStringHash.h` ← 10 file(s)
- `wtf/text/MakeString.h` ← 6 file(s)
- `wtf/text/ParsingUtilities.h` ← 2 file(s)
- `wtf/text/StringBuilder.h` ← 22 file(s)
- `wtf/text/StringHash.h` ← 1 file(s)
- `wtf/text/StringParsingBuffer.h` ← 2 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextPosition.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 6 file(s)
- `wtf/text/WTFString.h` ← 13 file(s)

## Problems

### unresolved (7)

- `CSSPropertyNames.h` ← 13 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 8 file(s)
- `HTMLNames.h` ← 3 file(s)
- `JSCSSStyleSheet.h` ← 1 file(s)
- `SVGElementTypeHelpers.h` ← 1 file(s)
- `Settings.h` ← 3 file(s)
