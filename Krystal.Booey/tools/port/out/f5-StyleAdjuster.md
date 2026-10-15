# Port graph

Entries: `style/StyleAdjuster.cpp`

- Files to port: **71**
- Boundary `animation`: 1
- Boundary `cssjit`: 1
- Boundary `dom`: 20
- Boundary `external`: 1
- Boundary `html`: 10
- Boundary `loader`: 3
- Boundary `mathml`: 1
- Boundary `page`: 3
- Boundary `platform`: 17
- Boundary `rendering`: 4
- Boundary `svg`: 4
- Boundary `wtf`: 47
- Unresolved: 15

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 9 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 8 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 7 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 7 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 7 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 6 | `css/CSSPrimitiveValue.h` |  | css/parser/CSSParserToken.h |
| 6 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 6 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 5 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryTypes.h |
| 5 | `css/calc/CSSCalcOperator.h` |  | style/calc/StyleCalculationTree.h |
| 5 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 5 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 5 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 5 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 5 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 5 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSPrimitiveValue.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 4 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h |
| 4 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h |
| 4 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/MediaQuery.h |
| 4 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/calc/StyleCalculationValue.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 4 | `style/calc/StyleCalculationTree.h` |  | style/calc/StyleCalculationValue.h |
| 4 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 4 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 4 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/StyleRule.h |
| 3 | `css/CSSSelectorList.h` |  | css/StyleRule.h |
| 3 | `css/CSSVariableData.h` |  | css/StyleRule.h |
| 3 | `css/StyleRuleType.h` |  | css/StyleRule.h, css/parser/CSSParserContext.h |
| 3 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 3 | `css/query/ContainerQuery.h` |  | css/StyleRule.h |
| 3 | `css/query/MediaQuery.h` |  | css/StyleRule.h |
| 3 | `css/values/CSSNoConversionDataRequiredToken.h` |  | style/values/StyleValueTypes.h |
| 3 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 3 | `style/calc/StyleCalculationValue.h` |  | style/values/primitives/StylePrimitiveNumericTypes+Calculation.h |
| 3 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 3 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `css/CSSFontFace.h` |  | css/CSSFontFaceSet.h, css/CSSFontSelector.h |
| 2 | `css/CSSFontFaceSet.h` |  | css/CSSFontSelector.h |
| 2 | `css/StyleRule.h` |  | css/CSSFontSelector.h |
| 2 | `rendering/style/AutosizeStatus.h` |  | style/StyleAdjuster.h |
| 2 | `rendering/style/RenderStyle.h` |  | rendering/style/RenderStyle+GettersInlines.h |
| 2 | `rendering/style/RenderStyleConstants.h` |  | css/CSSFontFace.h, css/CSSSelector.h, rendering/style/StyleSelfAlignmentData.h, style/PseudoElementIdentifier.h, style/StyleAdjuster.h, style/Styleable.h, style/values/text-decoration/StyleTextDecorationLine.h |
| 2 | `style/PseudoElementIdentifier.h` |  | style/Styleable.h |
| 2 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 2 | `style/computed/StyleComputedStyle.h` |  | style/computed/StyleComputedStyle+InitialInlines.h |
| 2 | `style/values/StyleValueTypes.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StylePrimitiveNumericTypes+Evaluation.h, style/values/text-decoration/StyleTextDecorationLine.h |
| 2 | `style/values/primitives/StylePrimitiveNumericTypes+Calculation.h` |  | style/values/primitives/StylePrimitiveNumericTypes+Evaluation.h |
| 2 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | style/values/primitives/StylePrimitiveNumericTypes+Calculation.h, style/values/primitives/StylePrimitiveNumericTypes+Evaluation.h |
| 1 | `css/CSSFontSelector.h` |  | style/StyleAdjuster.cpp |
| 1 | `rendering/style/RenderStyle+GettersInlines.h` |  | rendering/style/RenderStyle+SettersInlines.h, style/StyleAdjuster.cpp |
| 1 | `rendering/style/RenderStyle+SettersInlines.h` |  | style/StyleAdjuster.cpp |
| 1 | `rendering/style/StyleSelfAlignmentData.h` |  | style/StyleAdjuster.cpp |
| 1 | `style/StylableInlines.h` |  | style/StyleAdjuster.cpp |
| 1 | `style/StyleAdjuster.h` |  | style/StyleAdjuster.cpp |
| 1 | `style/StyleUpdate.h` |  | style/StyleAdjuster.cpp |
| 1 | `style/Styleable.h` |  | style/StylableInlines.h, style/StyleAdjuster.cpp |
| 1 | `style/computed/StyleComputedStyle+InitialInlines.h` |  | style/StyleAdjuster.cpp |
| 1 | `style/values/primitives/StylePrimitiveNumericTypes+Evaluation.h` |  | style/StyleAdjuster.cpp |
| 1 | `style/values/text-decoration/StyleTextDecorationLine.h` |  | style/StyleAdjuster.cpp |
| 0 | `style/StyleAdjuster.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/WebAnimationTypes.h` ← 2 file(s)

### cssjit (1)

- `cssjit/CompiledSelector.h` ← 1 file(s)

### dom (20)

- `dom/ActiveDOMObject.h` ← 1 file(s)
- `dom/ContainerNodeInlines.h` ← 1 file(s)
- `dom/DocumentFullscreen.h` ← 1 file(s)
- `dom/DocumentPage.h` ← 1 file(s)
- `dom/DocumentQuirks.h` ← 1 file(s)
- `dom/DocumentView.h` ← 1 file(s)
- `dom/Element.h` ← 2 file(s)
- `dom/ElementAncestorIterator.h` ← 1 file(s)
- `dom/ElementAncestorIteratorInlines.h` ← 1 file(s)
- `dom/ElementInlines.h` ← 2 file(s)
- `dom/EventTargetInlines.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/PseudoElement.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)
- `dom/ScriptExecutionContext.h` ← 1 file(s)
- `dom/ShadowRoot.h` ← 1 file(s)
- `dom/Text.h` ← 1 file(s)
- `dom/TypedElementDescendantIterator.h` ← 1 file(s)
- `dom/TypedElementDescendantIteratorInlines.h` ← 1 file(s)
- `dom/VisibilityAdjustment.h` ← 1 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### html (10)

- `html/HTMLBodyElement.h` ← 1 file(s)
- `html/HTMLDialogElement.h` ← 1 file(s)
- `html/HTMLDivElement.h` ← 1 file(s)
- `html/HTMLInputElement.h` ← 1 file(s)
- `html/HTMLLabelElement.h` ← 1 file(s)
- `html/HTMLMarqueeElement.h` ← 1 file(s)
- `html/HTMLSlotElement.h` ← 1 file(s)
- `html/HTMLTableElement.h` ← 1 file(s)
- `html/HTMLTextAreaElement.h` ← 1 file(s)
- `html/HTMLVideoElement.h` ← 1 file(s)

### loader (3)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)
- `loader/cache/CachedResourceHandle.h` ← 1 file(s)
- `loader/cache/TrustedFonts.h` ← 1 file(s)

### mathml (1)

- `mathml/MathMLElement.h` ← 1 file(s)

### page (3)

- `page/LocalDOMWindow.h` ← 1 file(s)
- `page/LocalFrameView.h` ← 1 file(s)
- `page/Page.h` ← 1 file(s)

### platform (17)

- `platform/FloatConversion.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 2 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/Timer.h` ← 1 file(s)
- `platform/TouchAction.h` ← 1 file(s)
- `platform/graphics/FloatPoint.h` ← 1 file(s)
- `platform/graphics/FloatSize.h` ← 1 file(s)
- `platform/graphics/Font.h` ← 1 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/FontSelectionAlgorithm.h` ← 1 file(s)
- `platform/graphics/FontSelector.h` ← 1 file(s)
- `platform/graphics/FontTaggedSettings.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 1 file(s)
- `platform/text/TextFlags.h` ← 1 file(s)

### rendering (4)

- `rendering/PathOperation.h` ← 1 file(s)
- `rendering/RenderBox.h` ← 1 file(s)
- `rendering/RenderTheme.h` ← 1 file(s)
- `rendering/RenderView.h` ← 1 file(s)

### svg (4)

- `svg/SVGElement.h` ← 1 file(s)
- `svg/SVGGraphicsElement.h` ← 1 file(s)
- `svg/SVGSVGElement.h` ← 1 file(s)
- `svg/SVGURIReference.h` ← 1 file(s)

### wtf (47)

- `wtf/AbstractRefCountedAndCanMakeWeakPtr.h` ← 2 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/Brigand.h` ← 4 file(s)
- `wtf/CheckedPtr.h` ← 1 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 2 file(s)
- `wtf/EnumTraits.h` ← 7 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 13 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 3 file(s)
- `wtf/HashSet.h` ← 2 file(s)
- `wtf/Hasher.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 2 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/Observer.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 6 file(s)
- `wtf/Ref.h` ← 6 file(s)
- `wtf/RefCounted.h` ← 2 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 3 file(s)
- `wtf/RobinHoodHashSet.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 4 file(s)
- `wtf/TZoneMalloc.h` ← 5 file(s)
- `wtf/TypeCasts.h` ← 2 file(s)
- `wtf/URL.h` ← 1 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/Vector.h` ← 6 file(s)
- `wtf/WeakHashSet.h` ← 2 file(s)
- `wtf/WeakPtr.h` ← 1 file(s)
- `wtf/cocoa/RuntimeApplicationChecksCocoa.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 5 file(s)
- `wtf/text/AtomStringHash.h` ← 1 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringHash.h` ← 2 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 4 file(s)
- `wtf/text/WTFString.h` ← 3 file(s)

## Problems

### unresolved (15)

- `CSSPropertyNames.h` ← 4 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 6 file(s)
- `EventNames.h` ← 1 file(s)
- `HTMLNames.h` ← 1 file(s)
- `NodeName.h` ← 1 file(s)
- `RenderStyleProperties+GettersInlines.h` ← 1 file(s)
- `RenderStyleProperties+SettersInlines.h` ← 1 file(s)
- `RenderStyleProperties.h` ← 1 file(s)
- `SVGNames.h` ← 1 file(s)
- `Settings.h` ← 2 file(s)
- `StyleComputedStyleProperties+InitialInlines.h` ← 1 file(s)
- `StyleComputedStyleProperties.h` ← 1 file(s)
- `UserAgentParts.h` ← 1 file(s)
- `WebKitFontFamilyNames.h` ← 1 file(s)
