# Port graph

Entries: `css/CSSValue.cpp`

- Files to port: **160**
- Boundary `bindings`: 1
- Boundary `dom`: 3
- Boundary `external`: 2
- Boundary `loader`: 4
- Boundary `platform`: 9
- Boundary `rendering`: 3
- Boundary `svg`: 1
- Boundary `wtf`: 45
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 7 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 6 | `style/values/StyleValueTypes.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 6 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 6 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 6 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 6 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 5 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 5 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 5 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h |
| 5 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 5 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 4 | `css/StyleRuleType.h` |  | css/parser/CSSParserContext.h |
| 4 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 4 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 4 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 4 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 4 | `css/values/borders/CSSBorderRadius.h` |  | css/values/shapes/CSSInsetFunction.h, css/values/shapes/CSSRectFunction.h, css/values/shapes/CSSXywhFunction.h |
| 4 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 4 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h |
| 3 | `css/CSSStyleDeclaration.h` |  | css/DeprecatedCSSOMValue.h |
| 3 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/RectBase.h` |  | css/Quad.h, css/Rect.h |
| 3 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 3 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/parser/CSSParserIdioms.h |
| 3 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 3 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 3 | `css/values/CSSValueTypes.h` |  | css/values/color-adjust/CSSColorScheme.h, css/values/color/CSSColor.h, css/values/color/CSSDynamicRangeLimit.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/filter-effects/CSSAppleInvertLightnessFunction.h, css/values/grid/CSSGridNamedAreaMap.h, css/values/grid/CSSGridTemplateAreas.h, css/values/images/CSSGradient.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSURLModifiers.h, css/values/primitives/CSSUnevaluatedCalc.h, css/values/shapes/CSSFillRule.h, style/values/StyleValueTypes.h |
| 3 | `css/values/borders/CSSBoxShadow.h` |  | css/values/borders/CSSBoxShadowProperty.h |
| 3 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h |
| 3 | `css/values/easing/CSSCubicBezierEasingFunction.h` |  | css/values/easing/CSSEasingFunction.h |
| 3 | `css/values/easing/CSSLinearEasingFunction.h` |  | css/values/easing/CSSEasingFunction.h |
| 3 | `css/values/easing/CSSSpringEasingFunction.h` |  | css/values/easing/CSSEasingFunction.h |
| 3 | `css/values/easing/CSSStepsEasingFunction.h` |  | css/values/easing/CSSEasingFunction.h |
| 3 | `css/values/filter-effects/CSSAppleInvertLightnessFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h |
| 3 | `css/values/filter-effects/CSSBlurFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSBrightnessFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSContrastFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSDropShadowFunction.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSFilterReference.h` |  | css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSGrayscaleFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSHueRotateFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSInvertFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSOpacityFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSSaturateFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/filter-effects/CSSSepiaFunction.h` |  | css/values/filter-effects/CSSAppleColorFilter.h, css/values/filter-effects/CSSFilter.h |
| 3 | `css/values/grid/CSSGridNamedAreaMap.h` |  | css/values/grid/CSSGridTemplateAreas.h |
| 3 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 3 | `css/values/primitives/CSSURLModifiers.h` |  | css/values/primitives/CSSURL.h |
| 3 | `css/values/shapes/CSSCircleFunction.h` |  | css/values/shapes/CSSBasicShape.h |
| 3 | `css/values/shapes/CSSEllipseFunction.h` |  | css/values/shapes/CSSBasicShape.h |
| 3 | `css/values/shapes/CSSFillRule.h` |  | css/values/shapes/CSSPathFunction.h, css/values/shapes/CSSPolygonFunction.h, css/values/shapes/CSSShapeFunction.h |
| 3 | `css/values/shapes/CSSInsetFunction.h` |  | css/values/shapes/CSSBasicShape.h |
| 3 | `css/values/shapes/CSSPolygonFunction.h` |  | css/values/shapes/CSSBasicShape.h |
| 3 | `css/values/shapes/CSSRectFunction.h` |  | css/values/shapes/CSSBasicShape.h |
| 3 | `css/values/shapes/CSSShapeFunction.h` |  | css/values/shapes/CSSBasicShape.h |
| 3 | `css/values/shapes/CSSXywhFunction.h` |  | css/values/shapes/CSSBasicShape.h |
| 3 | `css/values/text-decoration/CSSTextShadow.h` |  | css/values/text-decoration/CSSTextShadowProperty.h |
| 2 | `css/CSSVariableData.h` |  | css/CSSCustomPropertyValue.h, css/CSSVariableReferenceValue.h |
| 2 | `css/CSSWideKeyword.h` |  | css/CSSCustomPropertyValue.h, css/parser/CSSParserIdioms.h |
| 2 | `css/DeprecatedCSSOMValue.h` |  | css/DeprecatedCSSOMPrimitiveValue.h, css/DeprecatedCSSOMValueList.h |
| 2 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 2 | `css/Quad.h` |  | css/CSSBorderImageSliceValue.h, css/CSSBorderImageWidthValue.h, css/CSSQuadValue.h |
| 2 | `css/Rect.h` |  | css/CSSRectValue.h |
| 2 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 2 | `css/parser/CSSParserIdioms.h` |  | css/DeprecatedCSSOMPrimitiveValue.h |
| 2 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h, css/CSSVariableReferenceValue.h |
| 2 | `css/values/borders/CSSBoxShadowProperty.h` |  | css/CSSBoxShadowPropertyValue.h |
| 2 | `css/values/color-adjust/CSSColorScheme.h` |  | css/CSSColorSchemeValue.h |
| 2 | `css/values/color/CSSColor.h` |  | css/CSSColorValue.h, css/values/borders/CSSBoxShadow.h, css/values/filter-effects/CSSDropShadowFunction.h, css/values/filter-effects/CSSHueRotateFunction.h, css/values/images/CSSGradient.h, css/values/text-decoration/CSSTextShadow.h |
| 2 | `css/values/color/CSSDynamicRangeLimit.h` |  | css/CSSDynamicRangeLimitValue.h |
| 2 | `css/values/easing/CSSEasingFunction.h` |  | css/CSSEasingFunctionValue.h |
| 2 | `css/values/filter-effects/CSSAppleColorFilter.h` |  | css/CSSAppleColorFilterValue.h |
| 2 | `css/values/filter-effects/CSSFilter.h` |  | css/CSSFilterImageValue.h, css/CSSFilterValue.h |
| 2 | `css/values/grid/CSSGridTemplateAreas.h` |  | css/CSSGridTemplateAreasValue.h |
| 2 | `css/values/images/CSSGradient.h` |  | css/CSSGradientValue.h |
| 2 | `css/values/motion/CSSRayFunction.h` |  | css/CSSRayValue.h |
| 2 | `css/values/primitives/CSSPosition.h` |  | css/CSSPositionValue.h, css/values/images/CSSGradient.h, css/values/motion/CSSRayFunction.h, css/values/shapes/CSSCircleFunction.h, css/values/shapes/CSSEllipseFunction.h, css/values/shapes/CSSShapeFunction.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/CSSFontStyleWithAngleValue.h, css/values/borders/CSSBorderRadius.h, css/values/borders/CSSBoxShadow.h, css/values/color/CSSColorDescriptors.h, css/values/easing/CSSCubicBezierEasingFunction.h, css/values/easing/CSSLinearEasingFunction.h, css/values/easing/CSSSpringEasingFunction.h, css/values/easing/CSSStepsEasingFunction.h, css/values/filter-effects/CSSBlurFunction.h, css/values/filter-effects/CSSBrightnessFunction.h, css/values/filter-effects/CSSContrastFunction.h, css/values/filter-effects/CSSDropShadowFunction.h, css/values/filter-effects/CSSGrayscaleFunction.h, css/values/filter-effects/CSSHueRotateFunction.h, css/values/filter-effects/CSSInvertFunction.h, css/values/filter-effects/CSSOpacityFunction.h, css/values/filter-effects/CSSSaturateFunction.h, css/values/filter-effects/CSSSepiaFunction.h, css/values/motion/CSSRayFunction.h, css/values/primitives/CSSPosition.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, css/values/primitives/CSSRatio.h, css/values/shapes/CSSCircleFunction.h, css/values/shapes/CSSEllipseFunction.h, css/values/shapes/CSSInsetFunction.h, css/values/shapes/CSSPathFunction.h, css/values/shapes/CSSPolygonFunction.h, css/values/shapes/CSSRectFunction.h, css/values/shapes/CSSShapeFunction.h, css/values/shapes/CSSXywhFunction.h, css/values/text-decoration/CSSTextShadow.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 2 | `css/values/primitives/CSSRatio.h` |  | css/CSSRatioValue.h |
| 2 | `css/values/primitives/CSSURL.h` |  | css/CSSCursorImageValue.h, css/CSSFontFaceSrcValue.h, css/CSSImageValue.h, css/CSSURLValue.h, css/values/filter-effects/CSSFilterReference.h |
| 2 | `css/values/shapes/CSSBasicShape.h` |  | css/CSSBasicShapeValue.h |
| 2 | `css/values/shapes/CSSPathFunction.h` |  | css/CSSPathValue.h, css/values/shapes/CSSBasicShape.h |
| 2 | `css/values/text-decoration/CSSTextShadowProperty.h` |  | css/CSSTextShadowPropertyValue.h |
| 1 | `css/CSSAppleColorFilterValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h, css/CSSValue.cpp |
| 1 | `css/CSSBackgroundRepeatValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSBasicShapeValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSBorderImageSliceValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSBorderImageWidthValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSBoxShadowPropertyValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSCanvasValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSColorSchemeValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSColorValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSCounterValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSCrossfadeValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSCursorImageValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSCustomPropertyValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSDynamicRangeLimitValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSEasingFunctionValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSFilterImageValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSFilterValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSFontFaceSrcValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSFontFeatureValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSFontStyleRangeValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSFontStyleWithAngleValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSFontValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSFontVariationValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSFunctionValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSGradientValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSGridAutoRepeatValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSGridIntegerRepeatValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSGridLineNamesValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSGridLineValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSGridTemplateAreasValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSImageSetOptionValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSImageSetValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSImageValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSNamedImageValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSOffsetRotateValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSPaintImageValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSPathValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSPendingSubstitutionValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSPositionValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSPrimitiveValue.h` |  | css/CSSCrossfadeValue.h, css/CSSFontStyleRangeValue.h, css/CSSGridIntegerRepeatValue.h, css/CSSGridLineValue.h, css/CSSImageSetOptionValue.h, css/CSSOffsetRotateValue.h, css/CSSReflectValue.h, css/CSSScrollValue.h, css/CSSValue.cpp, css/CSSViewValue.h, css/DeprecatedCSSOMPrimitiveValue.h, css/parser/CSSParserToken.h |
| 1 | `css/CSSProperty.h` |  | css/CSSStyleDeclaration.h, css/CSSValue.cpp |
| 1 | `css/CSSQuadValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSRatioValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSRayValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSRectValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSReflectValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSScrollValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSSubgridValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSTextShadowPropertyValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSToLengthConversionData.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSTransformListValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSURLValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSUnicodeRangeValue.h` |  | css/CSSValue.cpp |
| 1 | `css/CSSValue.h` |  | css/CSSAppleColorFilterValue.h, css/CSSAttrValue.h, css/CSSBackgroundRepeatValue.h, css/CSSBasicShapeValue.h, css/CSSBorderImageSliceValue.h, css/CSSBorderImageWidthValue.h, css/CSSBoxShadowPropertyValue.h, css/CSSCanvasValue.h, css/CSSColorSchemeValue.h, css/CSSColorValue.h, css/CSSCounterValue.h, css/CSSCrossfadeValue.h, css/CSSCursorImageValue.h, css/CSSCustomPropertyValue.h, css/CSSDynamicRangeLimitValue.h, css/CSSEasingFunctionValue.h, css/CSSFilterImageValue.h, css/CSSFilterValue.h, css/CSSFontFaceSrcValue.h, css/CSSFontFeatureValue.h, css/CSSFontStyleRangeValue.h, css/CSSFontStyleWithAngleValue.h, css/CSSFontValue.h, css/CSSFontVariationValue.h, css/CSSGridLineNamesValue.h, css/CSSGridLineValue.h, css/CSSGridTemplateAreasValue.h, css/CSSImageSetOptionValue.h, css/CSSImageValue.h, css/CSSNamedImageValue.h, css/CSSOffsetRotateValue.h, css/CSSPaintImageValue.h, css/CSSPathValue.h, css/CSSPendingSubstitutionValue.h, css/CSSPositionValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/CSSRatioValue.h, css/CSSRayValue.h, css/CSSReflectValue.h, css/CSSTextShadowPropertyValue.h, css/CSSURLValue.h, css/CSSUnicodeRangeValue.h, css/CSSValue.cpp, css/CSSValueList.h, css/CSSValuePair.h, css/CSSVariableReferenceValue.h, css/DeprecatedCSSOMValue.h, css/RectBase.h, css/values/CSSValueTypes.h |
| 1 | `css/CSSValueList.h` |  | css/CSSFontStyleRangeValue.h, css/CSSFunctionValue.h, css/CSSGridAutoRepeatValue.h, css/CSSGridIntegerRepeatValue.h, css/CSSImageSetValue.h, css/CSSSubgridValue.h, css/CSSTransformListValue.h, css/CSSValue.cpp, css/DeprecatedCSSOMValueList.h |
| 1 | `css/CSSValuePair.h` |  | css/CSSCursorImageValue.h, css/CSSValue.cpp |
| 1 | `css/CSSVariableReferenceValue.h` |  | css/CSSCustomPropertyValue.h, css/CSSPendingSubstitutionValue.h, css/CSSValue.cpp |
| 1 | `css/CSSViewValue.h` |  | css/CSSValue.cpp |
| 1 | `css/ComputedStyleDependencies.h` |  | css/CSSValue.cpp, css/values/CSSValueTypes.h |
| 1 | `css/DeprecatedCSSOMPrimitiveValue.h` |  | css/CSSValue.cpp |
| 1 | `css/DeprecatedCSSOMValueList.h` |  | css/CSSValue.cpp |
| 1 | `css/values/CSSSerializationContext.h` |  | css/CSSValue.cpp |
| 0 | `css/CSSValue.cpp` |  | (entry) |

## Boundaries

### bindings (1)

- `bindings/js/ScriptWrappable.h` ← 1 file(s)

### dom (3)

- `dom/Element.h` ← 1 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/ExceptionOr.h` ← 1 file(s)

### external (2)

- `pal/text/TextEncoding.h` ← 1 file(s)
- `unicode/umachine.h` ← 2 file(s)

### loader (4)

- `loader/LoadedFromOpaqueSource.h` ← 2 file(s)
- `loader/ResourceLoaderOptions.h` ← 2 file(s)
- `loader/cache/CachedImage.h` ← 1 file(s)
- `loader/cache/CachedResourceHandle.h` ← 2 file(s)

### platform (9)

- `platform/LayoutUnit.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 2 file(s)
- `platform/graphics/Color.h` ← 3 file(s)
- `platform/graphics/ColorInterpolationMethod.h` ← 1 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)
- `platform/graphics/FontTaggedSettings.h` ← 2 file(s)
- `platform/graphics/IntPoint.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 1 file(s)

### rendering (3)

- `rendering/style/GridArea.h` ← 1 file(s)
- `rendering/style/RenderStyleConstants.h` ← 3 file(s)
- `rendering/style/StyleImage.h` ← 1 file(s)

### svg (1)

- `svg/SVGPathByteStream.h` ← 1 file(s)

### wtf (45)

- `wtf/ASCIICType.h` ← 1 file(s)
- `wtf/AbstractRefCountedAndCanMakeWeakPtr.h` ← 1 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 1 file(s)
- `wtf/Brigand.h` ← 6 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/CompactVariant.h` ← 3 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 4 file(s)
- `wtf/FixedVector.h` ← 2 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 9 file(s)
- `wtf/Function.h` ← 6 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 3 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/MallocSpan.h` ← 1 file(s)
- `wtf/Markable.h` ← 2 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 1 file(s)
- `wtf/PointerComparison.h` ← 1 file(s)
- `wtf/Ref.h` ← 4 file(s)
- `wtf/RefCounted.h` ← 1 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 2 file(s)
- `wtf/StdLibExtras.h` ← 8 file(s)
- `wtf/TZoneMalloc.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/Vector.h` ← 6 file(s)
- `wtf/WeakHashSet.h` ← 1 file(s)
- `wtf/WeakPtr.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 4 file(s)
- `wtf/text/MakeString.h` ← 2 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 1 file(s)
- `wtf/text/WTFString.h` ← 14 file(s)

## Problems

### unresolved (2)

- `CSSPropertyNames.h` ← 7 file(s)
- `CSSValueKeywords.h` ← 9 file(s)
