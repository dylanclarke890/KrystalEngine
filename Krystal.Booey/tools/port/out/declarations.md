# Port graph

Entries: `css/StyleProperties.cpp`, `css/MutableStyleProperties.cpp`, `css/ImmutableStyleProperties.cpp`, `css/ShorthandSerializer.cpp`

- Files to port: **106**
- Boundary `animation`: 1
- Boundary `bindings`: 1
- Boundary `cssjit`: 1
- Boundary `dom`: 3
- Boundary `external`: 2
- Boundary `loader`: 1
- Boundary `platform`: 11
- Boundary `rendering`: 2
- Boundary `wtf`: 50
- Unresolved: 3

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 6 | `css/calc/CSSCalcOperator.h` |  | style/calc/StyleCalculationTree.h |
| 5 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryTypes.h |
| 5 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h |
| 5 | `style/calc/StyleCalculationTree.h` |  | style/calc/StyleCalculationValue.h |
| 5 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 5 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 5 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 4 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 4 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/MediaQuery.h |
| 4 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 4 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 4 | `style/calc/StyleCalculationValue.h` |  | style/values/primitives/StyleLengthWrapperData.h |
| 4 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 4 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 4 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StyleLengthWrapperData.h, style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/StyleRule.h |
| 3 | `css/CSSSelectorList.h` |  | css/StyleRule.h |
| 3 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 3 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 3 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 3 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 3 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 3 | `css/query/ContainerQuery.h` |  | css/StyleRule.h |
| 3 | `css/query/MediaQuery.h` |  | css/StyleRule.h |
| 3 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 3 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/grid/CSSGridNamedAreaMap.h, css/values/grid/CSSGridTemplateAreas.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 3 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/grid/CSSGridNamedAreaMap.h` |  | css/values/grid/CSSGridTemplateAreas.h |
| 3 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h, style/values/primitives/StyleLengthWrapper.h |
| 3 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/calc/StyleCalculationValue.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 3 | `style/values/primitives/StyleLengthWrapperData.h` |  | style/values/primitives/StyleLengthWrapper.h |
| 3 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, style/values/primitives/StyleLengthWrapper.h |
| 2 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 2 | `css/CSSProperty.h` |  | css/CSSStyleDeclaration.h, css/CSSStyleProperties.h, css/StyleProperties.h, css/parser/CSSParser.h |
| 2 | `css/CSSStyleDeclaration.h` |  | css/CSSStyleProperties.h |
| 2 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSBorderImageWidthValue.h, css/CSSColorValue.h, css/CSSCustomPropertyValue.h, css/CSSGridLineNamesValue.h, css/CSSGridTemplateAreasValue.h, css/CSSPendingSubstitutionValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/CSSValueList.h, css/CSSValuePair.h, css/CSSVariableReferenceValue.h, css/RectBase.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 2 | `css/CSSVariableData.h` |  | css/CSSCustomPropertyValue.h, css/CSSVariableReferenceValue.h, css/StyleRule.h |
| 2 | `css/CSSWideKeyword.h` |  | css/CSSCustomPropertyValue.h, css/parser/CSSParserIdioms.h |
| 2 | `css/RectBase.h` |  | css/Quad.h |
| 2 | `css/StyleRule.h` |  | css/parser/CSSParser.h |
| 2 | `css/StyleRuleType.h` |  | css/CSSStyleProperties.h, css/StyleRule.h, css/parser/CSSParserContext.h |
| 2 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 2 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/parser/CSSParserIdioms.h |
| 2 | `css/parser/CSSParserEnum.h` |  | css/parser/CSSParser.h |
| 2 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSPropertyParserConsumer+Ident.h, css/parser/CSSTokenizer.h |
| 2 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h, css/CSSVariableReferenceValue.h, css/parser/CSSParser.h, css/parser/CSSPropertyParserConsumer+Ident.h |
| 2 | `css/values/color/CSSColor.h` |  | css/CSSColorValue.h |
| 2 | `css/values/color/CSSColorType.h` |  | css/parser/CSSPropertyParserConsumer+Color.h |
| 2 | `css/values/grid/CSSGridTemplateAreas.h` |  | css/CSSGridTemplateAreasValue.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/parser/CSSPropertyParserConsumer+Font.h, css/values/color/CSSColorDescriptors.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 2 | `style/PseudoElementIdentifier.h` |  | style/StyleExtractor.h, style/StyleExtractorState.h |
| 2 | `style/StyleExtractorState.h` |  | style/StyleExtractor.h |
| 2 | `style/values/StyleValueTypes.h` |  | style/values/animations/StyleSingleAnimationRange.h, style/values/primitives/StylePrimitiveNumeric.h |
| 2 | `style/values/animations/StyleSingleAnimationRangeName.h` |  | style/values/animations/StyleSingleAnimationRange.h |
| 2 | `style/values/primitives/StyleLengthWrapper.h` |  | style/values/animations/StyleSingleAnimationRange.h |
| 1 | `css/CSSBorderImageWidthValue.h` |  | css/ShorthandSerializer.cpp |
| 1 | `css/CSSColorValue.h` |  | css/CSSValuePool.h, css/StyleProperties.cpp |
| 1 | `css/CSSCustomPropertyValue.h` |  | css/ImmutableStyleProperties.cpp, css/MutableStyleProperties.cpp, css/StyleProperties.cpp |
| 1 | `css/CSSGridLineNamesValue.h` |  | css/ShorthandSerializer.cpp |
| 1 | `css/CSSGridTemplateAreasValue.h` |  | css/ShorthandSerializer.cpp |
| 1 | `css/CSSPendingSubstitutionValue.h` |  | css/ShorthandSerializer.cpp, css/StyleProperties.cpp |
| 1 | `css/CSSPrimitiveValue.h` |  | css/CSSValuePool.h, css/StyleProperties.cpp, css/StylePropertiesInlines.h, css/parser/CSSParserToken.h, css/parser/CSSPropertyParserConsumer+Ident.h |
| 1 | `css/CSSPropertyInitialValues.h` |  | css/ShorthandSerializer.cpp, css/StyleProperties.cpp, css/StylePropertiesInlines.h |
| 1 | `css/CSSStyleProperties.h` |  | css/MutableStyleProperties.cpp, css/StyleProperties.cpp |
| 1 | `css/CSSValueList.h` |  | css/ShorthandSerializer.cpp, css/StyleProperties.cpp |
| 1 | `css/CSSValuePair.h` |  | css/ShorthandSerializer.cpp |
| 1 | `css/CSSValuePool.h` |  | css/MutableStyleProperties.cpp, css/parser/CSSPropertyParserConsumer+Ident.h |
| 1 | `css/CSSVariableReferenceValue.h` |  | css/CSSCustomPropertyValue.h, css/CSSPendingSubstitutionValue.h, css/ShorthandSerializer.cpp |
| 1 | `css/FontSelectionValueInlines.h` |  | css/ShorthandSerializer.cpp |
| 1 | `css/ImmutableStyleProperties.h` |  | css/ImmutableStyleProperties.cpp, css/MutableStyleProperties.cpp, css/StylePropertiesInlines.h |
| 1 | `css/MutableStyleProperties.h` |  | css/MutableStyleProperties.cpp, css/StylePropertiesInlines.h |
| 1 | `css/Quad.h` |  | css/CSSBorderImageWidthValue.h, css/ShorthandSerializer.cpp |
| 1 | `css/ShorthandSerializer.h` |  | css/ShorthandSerializer.cpp, css/StyleProperties.cpp |
| 1 | `css/StyleProperties.h` |  | css/ImmutableStyleProperties.h, css/MutableStyleProperties.h, css/StyleProperties.cpp |
| 1 | `css/StylePropertiesInlines.h` |  | css/ImmutableStyleProperties.cpp, css/MutableStyleProperties.cpp, css/ShorthandSerializer.cpp, css/StyleProperties.cpp |
| 1 | `css/StylePropertyShorthand.h` |  | css/MutableStyleProperties.cpp, css/ShorthandSerializer.cpp, css/StyleProperties.cpp |
| 1 | `css/parser/CSSParser.h` |  | css/MutableStyleProperties.cpp |
| 1 | `css/parser/CSSParserIdioms.h` |  | css/ShorthandSerializer.cpp |
| 1 | `css/parser/CSSPropertyParser.h` |  | css/ShorthandSerializer.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Color.h` |  | css/StyleProperties.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Font.h` |  | css/ShorthandSerializer.cpp, css/StyleProperties.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Grid.h` |  | css/ShorthandSerializer.cpp |
| 1 | `css/parser/CSSPropertyParserConsumer+Ident.h` |  | css/ShorthandSerializer.cpp |
| 1 | `css/values/CSSSerializationContext.h` |  | css/MutableStyleProperties.cpp, css/ShorthandSerializer.cpp, css/StyleProperties.cpp |
| 1 | `style/StyleExtractor.h` |  | css/ShorthandSerializer.cpp |
| 1 | `style/values/animations/StyleSingleAnimationRange.h` |  | css/ShorthandSerializer.cpp |
| 0 | `css/ImmutableStyleProperties.cpp` |  | (entry) |
| 0 | `css/MutableStyleProperties.cpp` |  | (entry) |
| 0 | `css/ShorthandSerializer.cpp` |  | (entry) |
| 0 | `css/StyleProperties.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/TimelineRangeValue.h` ← 1 file(s)

### bindings (1)

- `bindings/js/ScriptWrappable.h` ← 1 file(s)

### cssjit (1)

- `cssjit/CompiledSelector.h` ← 1 file(s)

### dom (3)

- `dom/Element.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)
- `dom/StyledElement.h` ← 1 file(s)

### external (2)

- `pal/text/TextEncoding.h` ← 1 file(s)
- `unicode/umachine.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)

### platform (11)

- `platform/LayoutUnit.h` ← 2 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/graphics/Color.h` ← 4 file(s)
- `platform/graphics/ColorHash.h` ← 1 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/FontSelectionAlgorithm.h` ← 1 file(s)
- `platform/graphics/SystemFontDatabase.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 1 file(s)

### rendering (2)

- `rendering/style/GridArea.h` ← 1 file(s)
- `rendering/style/RenderStyleConstants.h` ← 2 file(s)

### wtf (50)

- `wtf/ASCIICType.h` ← 1 file(s)
- `wtf/AbstractRefCountedAndCanMakeWeakPtr.h` ← 1 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 1 file(s)
- `wtf/Brigand.h` ← 6 file(s)
- `wtf/CheckedPtr.h` ← 1 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 5 file(s)
- `wtf/FixedVector.h` ← 4 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 17 file(s)
- `wtf/Function.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 6 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/IndexedRange.h` ← 2 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/MallocSpan.h` ← 1 file(s)
- `wtf/Markable.h` ← 2 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NeverDestroyed.h` ← 2 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 4 file(s)
- `wtf/OptionalOrReference.h` ← 1 file(s)
- `wtf/PointerComparison.h` ← 1 file(s)
- `wtf/Ref.h` ← 7 file(s)
- `wtf/RefCounted.h` ← 3 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 8 file(s)
- `wtf/StdLibExtras.h` ← 7 file(s)
- `wtf/TZoneMalloc.h` ← 4 file(s)
- `wtf/TypeCasts.h` ← 2 file(s)
- `wtf/URL.h` ← 1 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/Vector.h` ← 8 file(s)
- `wtf/WeakPtr.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 6 file(s)
- `wtf/text/AtomStringHash.h` ← 2 file(s)
- `wtf/text/CString.h` ← 1 file(s)
- `wtf/text/MakeString.h` ← 3 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 2 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 4 file(s)
- `wtf/text/WTFString.h` ← 8 file(s)

## Problems

### unresolved (3)

- `CSSPropertyNames.h` ← 12 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 16 file(s)
