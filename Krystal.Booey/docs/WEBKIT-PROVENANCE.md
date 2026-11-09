# WebKit provenance

Which Krystal.Booey files were ported from WebKit, the WebKit file(s) each came from, and the licence of that source. Ported files carry their source's copyright notice verbatim at the top (`#pragma once` or the first `#include` follows it); hand-written files carry none. The sources are WebKit at `300e80c70fd3` (2025-12-26). Tests are out of scope.

Files that combine several WebKit files carry each source's notice. A `.hpp`/`.cpp` pair takes the notice of WebKit's `.h`/`.cpp` respectively, so the two can differ.

| | Files |
|---|---|
| Ported | 3526 |
| &nbsp;&nbsp;BSD-2-Clause | 2359 |
| &nbsp;&nbsp;LGPL / tri-licence | 906 |
| &nbsp;&nbsp;BSD-3-Clause | 255 |
| &nbsp;&nbsp;BSD-3-Clause AND BSD-2-Clause | 2 |
| &nbsp;&nbsp;BSD-2-Clause AND BSD-3-Clause | 2 |
| &nbsp;&nbsp;no notice in WebKit | 2 |
| Hand-written | 334 |

## LGPL and Mozilla tri-licensed

These come from WebKit files under the GNU LGPL (mostly LGPL-2.0-or-later, from KHTML) or Mozilla's MPL-1.1/GPL-2.0/LGPL-2.1 tri-licence, so they can't be relicensed under the repository's MIT licence. Replacing them means rewriting the unit (from the specifications) rather than porting it.

| Booey unit | Files | WebKit source | Licence |
|---|---|---|---|
| `Animation/Keyframes/BlendingKeyframes` | .cpp, .hpp | `animation/BlendingKeyframes.cpp`<br>`animation/BlendingKeyframes.h` | LGPL-2.0-or-later |
| `CSS/CSSOM/CSSComputedStyleDeclaration` | .cpp, .hpp | `css/CSSComputedStyleDeclaration.cpp`<br>`css/CSSComputedStyleDeclaration.h` | LGPL-2.1-or-later |
| `CSS/CSSOM/CSSFontFaceRule` | .cpp, .hpp | `css/CSSFontFaceRule.cpp`<br>`css/CSSFontFaceRule.h` | LGPL-2.0-or-later |
| `CSS/CSSOM/CSSGroupingRule` | .hpp | `css/CSSGroupingRule.h` | LGPL-2.0-or-later |
| `CSS/CSSOM/CSSImportRule` | .cpp, .hpp | `css/CSSImportRule.cpp`<br>`css/CSSImportRule.h` | LGPL-2.0-or-later |
| `CSS/CSSOM/CSSMediaRule` | .cpp, .hpp | `css/CSSMediaRule.cpp`<br>`css/CSSMediaRule.h` | LGPL-2.0-or-later |
| `CSS/CSSOM/CSSNestedDeclarations` | .cpp, .hpp | `css/CSSNestedDeclarations.cpp`<br>`css/CSSNestedDeclarations.h` | LGPL-2.0-or-later |
| `CSS/CSSOM/CSSRule` | .cpp, .hpp | `css/CSSRule.cpp`<br>`css/CSSRule.h` | LGPL-2.0-or-later |
| `CSS/CSSOM/CSSRuleList` | .cpp, .hpp | `css/CSSRuleList.cpp`<br>`css/CSSRuleList.h` | LGPL-2.0-or-later |
| `CSS/CSSOM/CSSStyleDeclaration` | .hpp | `css/CSSStyleDeclaration.h` | LGPL-2.0-or-later |
| `CSS/CSSOM/CSSStyleRule` | .cpp, .hpp | `css/CSSStyleRule.cpp`<br>`css/CSSStyleRule.h` | LGPL-2.0-or-later |
| `CSS/CSSOM/Markup` | .cpp, .hpp | `css/CSSMarkup.cpp`<br>`css/CSSMarkup.h` | LGPL-2.0-or-later |
| `CSS/Media/MediaList` | .cpp, .hpp | `css/MediaList.cpp`<br>`css/MediaList.h` | LGPL-2.0-or-later |
| `CSS/Media/MediaQueryList` | .cpp, .hpp | `css/MediaQueryList.cpp`<br>`css/MediaQueryList.h` | LGPL-2.0-or-later |
| `CSS/Properties/Property` | .cpp, .hpp | `css/CSSProperty.cpp`<br>`css/CSSProperty.h` | LGPL-2.0-or-later |
| `CSS/Properties/PropertyParser` | .hpp | `css/parser/CSSPropertyParser.h` | LGPL-2.0-or-later |
| `CSS/Properties/PropertyShorthand` | .cpp, .hpp | `css/StylePropertyShorthand.cpp`<br>`css/StylePropertyShorthand.h` | LGPL-2.0-or-later |
| `CSS/Properties/StyleProperties` | .cpp, .hpp | `css/StyleProperties.cpp`<br>`css/StyleProperties.h` | LGPL-2.0-or-later |
| `CSS/Queries/MediaQueryMatcher` | .cpp, .hpp | `css/MediaQueryMatcher.cpp`<br>`css/MediaQueryMatcher.h` | LGPL-2.0-or-later |
| `CSS/Rules/ImportRule` | .cpp, .hpp | `css/StyleRuleImport.cpp`<br>`css/StyleRuleImport.h` | LGPL-2.0-or-later |
| `CSS/Rules/Rule` | .cpp, .hpp | `css/StyleRule.cpp`<br>`css/StyleRule.h` | LGPL-2.0-or-later |
| `CSS/Selectors/MutableSelector` | .cpp, .hpp | `css/parser/MutableCSSSelector.cpp`<br>`css/parser/MutableCSSSelector.h` | LGPL-2.0-or-later |
| `CSS/Selectors/Selector` | .cpp, .hpp | `css/CSSSelector.cpp`<br>`css/CSSSelector.h` | LGPL-2.0-or-later |
| `CSS/Selectors/SelectorChecker` | .cpp, .hpp | `css/SelectorChecker.cpp`<br>`css/SelectorChecker.h` | LGPL-2.0-or-later |
| `CSS/Selectors/SelectorFilter` | .cpp, .hpp | `css/SelectorFilter.cpp`<br>`css/SelectorFilter.h` | LGPL-2.0-or-later |
| `CSS/StyleSheets/CSSStyleSheet` | .cpp, .hpp | `css/CSSStyleSheet.cpp`<br>`css/CSSStyleSheet.h` | LGPL-2.0-or-later |
| `CSS/StyleSheets/StyleSheet` | .hpp | `css/StyleSheet.h` | LGPL-2.0-or-later |
| `CSS/StyleSheets/StyleSheetContents` | .cpp, .hpp | `css/StyleSheetContents.cpp`<br>`css/StyleSheetContents.h` | LGPL-2.0-or-later |
| `CSS/StyleSheets/StyleSheetList` | .cpp, .hpp | `css/StyleSheetList.cpp`<br>`css/StyleSheetList.h` | LGPL-2.0-or-later |
| `CSS/Values/Borders/BorderImage` | .cpp, .hpp | `css/CSSBorderImage.cpp`<br>`css/CSSBorderImage.h` | LGPL-2.0-or-later |
| `CSS/Values/ComputedStyleDependencies` | .hpp | `css/ComputedStyleDependencies.h` | LGPL-2.0-or-later |
| `CSS/Values/CursorImageValue` | .cpp, .hpp | `css/CSSCursorImageValue.cpp`<br>`css/CSSCursorImageValue.h` | LGPL-2.0-or-later |
| `CSS/Values/FontValue` | .cpp, .hpp | `css/CSSFontValue.cpp`<br>`css/CSSFontValue.h` | LGPL-2.0-or-later |
| `CSS/Values/ImageValue` | .cpp, .hpp | `css/CSSImageValue.cpp`<br>`css/CSSImageValue.h` | LGPL-2.0-or-later |
| `CSS/Values/PrimitiveValue` | .cpp, .hpp | `css/CSSPrimitiveValue.cpp`<br>`css/CSSPrimitiveValue.h` | LGPL-2.0-or-later |
| `CSS/Values/Quad` | .hpp | `css/Quad.h` | LGPL-2.0-or-later |
| `CSS/Values/Rect` | .hpp | `css/Rect.h` | LGPL-2.0-or-later |
| `CSS/Values/RectBase` | .hpp | `css/RectBase.h` | LGPL-2.0-or-later |
| `CSS/Values/UnitCategory` | .hpp | `css/CSSUnits.h` | LGPL-2.0-or-later |
| `CSS/Values/UnitType` | .hpp | `css/CSSUnits.h` | LGPL-2.0-or-later |
| `CSS/Values/Units` | .hpp | `css/CSSUnits.h` | LGPL-2.0-or-later |
| `CSS/Values/Value` | .hpp | `css/CSSValue.h` | LGPL-2.0-or-later |
| `CSS/Values/ValueList` | .cpp, .hpp | `css/CSSValueList.cpp`<br>`css/CSSValueList.h` | LGPL-2.0-or-later |
| `CSS/Values/ValueSeparator` | .hpp | `css/CSSValue.h` | LGPL-2.0-or-later |
| `CSS/Values/ValueType` | .hpp | `css/CSSValue.h` | LGPL-2.0-or-later |
| `DOM/Algorithms/EventDispatcher` | .cpp, .hpp | `dom/EventDispatcher.cpp`<br>`dom/EventDispatcher.h` | LGPL-2.0-or-later |
| `DOM/Algorithms/ExtensibilityHooks` | .cpp, .hpp | `dom/Element.cpp`<br>`dom/Element.h` | LGPL-2.0-or-later |
| `DOM/Algorithms/TreeQueries` | .cpp, .hpp | `dom/Node.cpp`<br>`dom/Node.h` | LGPL-2.0-or-later |
| `DOM/ContentVisibility/ContentRelevancy` | .hpp | `dom/Element.h` | LGPL-2.0-or-later |
| `DOM/DataTransfer/DataTransfer` | .hpp | `dom/DataTransfer.h` | LGPL-2.0-or-later |
| `DOM/Events/Event` | .cpp, .hpp | `dom/Event.cpp`<br>`dom/Event.h` | LGPL-2.0-or-later |
| `DOM/Events/KeyboardEvent` | .cpp, .hpp | `dom/KeyboardEvent.cpp`<br>`dom/KeyboardEvent.h` | LGPL-2.0-or-later |
| `DOM/Events/MouseEvent` | .cpp, .hpp | `dom/MouseEvent.cpp`<br>`dom/MouseEvent.h` | LGPL-2.0-or-later |
| `DOM/Events/MouseRelatedEvent` | .cpp, .hpp | `dom/MouseRelatedEvent.cpp`<br>`dom/MouseRelatedEvent.h` | LGPL-2.0-or-later |
| `DOM/Events/UIEventWithKeyState` | .hpp | `dom/UIEventWithKeyState.h` | LGPL-2.0-or-later |
| `DOM/Events/WheelEvent` | .cpp, .hpp | `dom/WheelEvent.cpp`<br>`dom/WheelEvent.h` | LGPL-2.0-or-later |
| `DOM/Internals/ElementRareData` | .hpp | `dom/ElementRareData.h` | LGPL-2.0-or-later |
| `DOM/Internals/NodeRareData` | .hpp | `dom/NodeRareData.h` | LGPL-2.0-or-later |
| `DOM/Nodes/ContainerNode` | .cpp, .hpp | `dom/ContainerNode.cpp`<br>`dom/ContainerNode.h` | LGPL-2.0-or-later |
| `DOM/Nodes/Document` | .cpp, .hpp | `dom/Document.cpp`<br>`dom/Document.h` | LGPL-2.0-or-later |
| `DOM/Nodes/Element` | .cpp, .hpp | `dom/Element.cpp`<br>`dom/Element.h` | LGPL-2.0-or-later |
| `DOM/Nodes/HTMLDocument` | .cpp, .hpp | `html/HTMLDocument.cpp`<br>`html/HTMLDocument.h` | MPL-1.1/GPL-2.0/LGPL-2.1 tri-licence / LGPL-2.0-or-later |
| `DOM/Nodes/Node` | .cpp, .hpp | `dom/Node.cpp`<br>`dom/Node.h` | LGPL-2.0-or-later |
| `DOM/Nodes/ProcessingInstruction` | .cpp, .hpp | `dom/ProcessingInstruction.cpp`<br>`dom/ProcessingInstruction.h` | LGPL-2.0-or-later |
| `DOM/Nodes/StyledElement` | .cpp, .hpp | `dom/StyledElement.cpp`<br>`dom/StyledElement.h` | LGPL-2.0-or-later |
| `DOM/Nodes/Text` | .cpp, .hpp | `dom/Text.cpp`<br>`dom/Text.h` | LGPL-2.0-or-later |
| `DOM/Parsing/DocumentParser` | .hpp | `dom/DocumentParser.h` | LGPL-2.0-or-later |
| `DOM/Ranges/Range` | .cpp, .hpp | `dom/Range.cpp`<br>`dom/Range.h` | LGPL-2.0-or-later |
| `DOM/Types/Exception` | .hpp | `dom/ExceptionCode.h` | LGPL-2.1-or-later |
| `DOM/Types/QualifiedName` | .cpp, .hpp | `dom/QualifiedName.cpp`<br>`dom/QualifiedName.h` | LGPL-2.0-or-later |
| `Editing/EditingBehavior` | .hpp | `editing/EditingBehavior.h` | LGPL-2.0-or-later |
| `Graphics/Animation/TimingFunction` | .hpp | `platform/animation/TimingFunction.h` | LGPL-2.0-or-later |
| `Graphics/Filters/DistantLightSource` | .hpp | `platform/graphics/filters/DistantLightSource.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FEBlend` | .cpp, .hpp | `platform/graphics/filters/FEBlend.cpp`<br>`platform/graphics/filters/FEBlend.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FEColorMatrix` | .cpp, .hpp | `platform/graphics/filters/FEColorMatrix.cpp`<br>`platform/graphics/filters/FEColorMatrix.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FEComponentTransfer` | .cpp, .hpp | `platform/graphics/filters/FEComponentTransfer.cpp`<br>`platform/graphics/filters/FEComponentTransfer.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FEComposite` | .cpp, .hpp | `platform/graphics/filters/FEComposite.cpp`<br>`platform/graphics/filters/FEComposite.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FEConvolveMatrix` | .cpp, .hpp | `platform/graphics/filters/FEConvolveMatrix.cpp`<br>`platform/graphics/filters/FEConvolveMatrix.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FEDiffuseLighting` | .cpp, .hpp | `platform/graphics/filters/FEDiffuseLighting.cpp`<br>`platform/graphics/filters/FEDiffuseLighting.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FEDisplacementMap` | .cpp, .hpp | `platform/graphics/filters/FEDisplacementMap.cpp`<br>`platform/graphics/filters/FEDisplacementMap.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FEDropShadow` | .cpp, .hpp | `platform/graphics/filters/FEDropShadow.cpp`<br>`platform/graphics/filters/FEDropShadow.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FEFlood` | .cpp, .hpp | `platform/graphics/filters/FEFlood.cpp`<br>`platform/graphics/filters/FEFlood.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FEGaussianBlur` | .cpp, .hpp | `platform/graphics/filters/FEGaussianBlur.cpp`<br>`platform/graphics/filters/FEGaussianBlur.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FEImage` | .cpp, .hpp | `platform/graphics/filters/FEImage.cpp`<br>`platform/graphics/filters/FEImage.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FEMerge` | .cpp, .hpp | `platform/graphics/filters/FEMerge.cpp`<br>`platform/graphics/filters/FEMerge.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FEMorphology` | .cpp, .hpp | `platform/graphics/filters/FEMorphology.cpp`<br>`platform/graphics/filters/FEMorphology.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FEOffset` | .cpp, .hpp | `platform/graphics/filters/FEOffset.cpp`<br>`platform/graphics/filters/FEOffset.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FESpecularLighting` | .cpp, .hpp | `platform/graphics/filters/FESpecularLighting.cpp`<br>`platform/graphics/filters/FESpecularLighting.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FETile` | .cpp, .hpp | `platform/graphics/filters/FETile.cpp`<br>`platform/graphics/filters/FETile.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FETurbulence` | .cpp, .hpp | `platform/graphics/filters/FETurbulence.cpp`<br>`platform/graphics/filters/FETurbulence.h` | LGPL-2.0-or-later |
| `Graphics/Filters/Filter` | .hpp | `platform/graphics/filters/Filter.h` | LGPL-2.0-or-later |
| `Graphics/Filters/FilterEffect` | .cpp, .hpp | `platform/graphics/filters/FilterEffect.cpp`<br>`platform/graphics/filters/FilterEffect.h` | LGPL-2.0-or-later |
| `Graphics/Filters/LightSource` | .hpp | `platform/graphics/filters/LightSource.h` | LGPL-2.0-or-later |
| `Graphics/Filters/PointLightSource` | .hpp | `platform/graphics/filters/PointLightSource.h` | LGPL-2.0-or-later |
| `Graphics/Filters/SourceAlpha` | .cpp, .hpp | `platform/graphics/filters/SourceAlpha.cpp`<br>`platform/graphics/filters/SourceAlpha.h` | LGPL-2.0-or-later |
| `Graphics/Filters/SourceGraphic` | .cpp, .hpp | `platform/graphics/filters/SourceGraphic.cpp`<br>`platform/graphics/filters/SourceGraphic.h` | LGPL-2.0-or-later |
| `Graphics/Filters/SpotLightSource` | .hpp | `platform/graphics/filters/SpotLightSource.h` | LGPL-2.0-or-later |
| `Graphics/Fonts/Font` | .hpp | `platform/graphics/Font.h` | LGPL-2.0-or-later |
| `Graphics/Fonts/FontCascade` | .cpp, .hpp | `platform/graphics/FontCascade.cpp`<br>`platform/graphics/FontCascade.h` | LGPL-2.0-or-later |
| `Graphics/Fonts/FontCascadeDescription` | .hpp | `platform/graphics/FontCascadeDescription.h` | LGPL-2.0-or-later |
| `Graphics/Fonts/FontCascadeFonts` | .hpp | `platform/graphics/FontCascadeFonts.h` | LGPL-2.0-or-later |
| `Graphics/Fonts/FontDescription` | .hpp | `platform/graphics/FontDescription.h` | LGPL-2.0-or-later |
| `Graphics/Fonts/FontMetrics` | .hpp | `platform/graphics/FontMetrics.h` | LGPL-2.0-or-later |
| `Graphics/Fonts/FontPlatformData` | .cpp, .hpp | `platform/graphics/FontPlatformData.cpp`<br>`platform/graphics/FontPlatformData.h` | LGPL-2.0-or-later |
| `Graphics/Geometry/FloatPoint3D` | .cpp, .hpp | `platform/graphics/FloatPoint3D.cpp`<br>`platform/graphics/FloatPoint3D.h` | LGPL-2.0-or-later |
| `Graphics/Layers/GraphicsLayerAnimation` | .cpp, .hpp | `platform/graphics/GraphicsLayerAnimation.cpp`<br>`platform/graphics/GraphicsLayerAnimation.h` | LGPL-2.0-or-later |
| `Graphics/Layers/GraphicsLayerTransform` | .cpp, .hpp | `platform/graphics/GraphicsLayerTransform.cpp`<br>`platform/graphics/GraphicsLayerTransform.h` | LGPL-2.0-or-later |
| `Graphics/Paths/PathTraversalState` | .cpp | `platform/graphics/PathTraversalState.cpp` | LGPL-2.0-or-later |
| `Graphics/TextRuns/TextRun` | .hpp | `platform/graphics/TextRun.h` | LGPL-2.0-or-later |
| `Graphics/TextRuns/WidthIterator` | .cpp, .hpp | `platform/graphics/WidthIterator.cpp`<br>`platform/graphics/WidthIterator.h` | LGPL-2.0-or-later |
| `Graphics/Transforms/IdentityTransformOperation` | .hpp | `platform/graphics/transforms/IdentityTransformOperation.h` | LGPL-2.0-or-later |
| `Graphics/Transforms/MatrixTransformOperation` | .cpp, .hpp | `platform/graphics/transforms/MatrixTransformOperation.cpp`<br>`platform/graphics/transforms/MatrixTransformOperation.h` | LGPL-2.0-or-later |
| `Graphics/Transforms/RotateTransformOperation` | .cpp, .hpp | `platform/graphics/transforms/RotateTransformOperation.cpp`<br>`platform/graphics/transforms/RotateTransformOperation.h` | LGPL-2.0-or-later |
| `Graphics/Transforms/ScaleTransformOperation` | .cpp, .hpp | `platform/graphics/transforms/ScaleTransformOperation.cpp`<br>`platform/graphics/transforms/ScaleTransformOperation.h` | LGPL-2.0-or-later |
| `Graphics/Transforms/SkewTransformOperation` | .cpp, .hpp | `platform/graphics/transforms/SkewTransformOperation.cpp`<br>`platform/graphics/transforms/SkewTransformOperation.h` | LGPL-2.0-or-later |
| `Graphics/Transforms/TransformOperation` | .hpp | `platform/graphics/transforms/TransformOperation.h` | LGPL-2.0-or-later |
| `Graphics/Transforms/TransformOperations` | .cpp, .hpp | `platform/graphics/transforms/TransformOperations.cpp`<br>`platform/graphics/transforms/TransformOperations.h` | LGPL-2.0-or-later |
| `Graphics/Transforms/TranslateTransformOperation` | .cpp, .hpp | `platform/graphics/transforms/TranslateTransformOperation.cpp`<br>`platform/graphics/transforms/TranslateTransformOperation.h` | LGPL-2.0-or-later |
| `HTML/Collections/HTMLOptionsCollection` | .cpp, .hpp | `html/HTMLOptionsCollection.cpp`<br>`html/HTMLOptionsCollection.h` | LGPL-2.0-or-later |
| `HTML/Collections/LabelsNodeList` | .cpp, .hpp | `html/LabelsNodeList.cpp`<br>`html/LabelsNodeList.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLAnchorElement` | .cpp, .hpp | `html/HTMLAnchorElement.cpp`<br>`html/HTMLAnchorElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLAreaElement` | .cpp, .hpp | `html/HTMLAreaElement.cpp`<br>`html/HTMLAreaElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLBRElement` | .cpp, .hpp | `html/HTMLBRElement.cpp`<br>`html/HTMLBRElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLBodyElement` | .cpp, .hpp | `html/HTMLBodyElement.cpp`<br>`html/HTMLBodyElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLButtonElement` | .cpp, .hpp | `html/HTMLButtonElement.cpp`<br>`html/HTMLButtonElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLDetailsElement` | .cpp, .hpp | `html/HTMLDetailsElement.cpp`<br>`html/HTMLDetailsElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLElement` | .cpp, .hpp | `html/HTMLElement.cpp`<br>`html/HTMLElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLFieldSetElement` | .cpp, .hpp | `html/HTMLFieldSetElement.cpp`<br>`html/HTMLFieldSetElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLFormControlElement` | .cpp, .hpp | `html/HTMLFormControlElement.cpp`<br>`html/HTMLFormControlElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLFormElement` | .cpp, .hpp | `html/HTMLFormElement.cpp`<br>`html/HTMLFormElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLFrameSetElement` | .cpp, .hpp | `html/HTMLFrameSetElement.cpp`<br>`html/HTMLFrameSetElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLHRElement` | .cpp, .hpp | `html/HTMLHRElement.cpp`<br>`html/HTMLHRElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLIFrameElement` | .cpp, .hpp | `html/HTMLIFrameElement.cpp`<br>`html/HTMLIFrameElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLImageElement` | .cpp, .hpp | `html/HTMLImageElement.cpp`<br>`html/HTMLImageElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLInputElement` | .cpp, .hpp | `html/HTMLInputElement.cpp`<br>`html/HTMLInputElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLLIElement` | .cpp, .hpp | `html/HTMLLIElement.cpp`<br>`html/HTMLLIElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLLabelElement` | .cpp, .hpp | `html/HTMLLabelElement.cpp`<br>`html/HTMLLabelElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLLegendElement` | .cpp, .hpp | `html/HTMLLegendElement.cpp`<br>`html/HTMLLegendElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLLinkElement` | .cpp, .hpp | `html/HTMLLinkElement.cpp`<br>`html/HTMLLinkElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLMapElement` | .cpp, .hpp | `html/HTMLMapElement.cpp`<br>`html/HTMLMapElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLMetaElement` | .cpp, .hpp | `html/HTMLMetaElement.cpp`<br>`html/HTMLMetaElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLMeterElement` | .cpp, .hpp | `html/HTMLMeterElement.cpp`<br>`html/HTMLMeterElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLOListElement` | .cpp, .hpp | `html/HTMLOListElement.cpp`<br>`html/HTMLOListElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLObjectElement` | .cpp, .hpp | `html/HTMLObjectElement.cpp`<br>`html/HTMLObjectElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLOptGroupElement` | .cpp, .hpp | `html/HTMLOptGroupElement.cpp`<br>`html/HTMLOptGroupElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLOptionElement` | .cpp, .hpp | `html/HTMLOptionElement.cpp`<br>`html/HTMLOptionElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLPlugInElement` | .cpp, .hpp | `html/HTMLPlugInElement.cpp`<br>`html/HTMLPlugInElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLProgressElement` | .cpp, .hpp | `html/HTMLProgressElement.cpp`<br>`html/HTMLProgressElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLScriptElement` | .cpp, .hpp | `html/HTMLScriptElement.cpp`<br>`html/HTMLScriptElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLSelectElement` | .cpp, .hpp | `html/HTMLSelectElement.cpp`<br>`html/HTMLSelectElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLStyleElement` | .cpp, .hpp | `html/HTMLStyleElement.cpp`<br>`html/HTMLStyleElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLSummaryElement` | .cpp, .hpp | `html/HTMLSummaryElement.cpp`<br>`html/HTMLSummaryElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLTableCellElement` | .cpp, .hpp | `html/HTMLTableCellElement.cpp`<br>`html/HTMLTableCellElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLTableColElement` | .cpp, .hpp | `html/HTMLTableColElement.cpp`<br>`html/HTMLTableColElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLTableElement` | .cpp, .hpp | `html/HTMLTableElement.cpp`<br>`html/HTMLTableElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLTablePartElement` | .cpp, .hpp | `html/HTMLTablePartElement.cpp`<br>`html/HTMLTablePartElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLTableSectionElement` | .cpp, .hpp | `html/HTMLTableSectionElement.cpp`<br>`html/HTMLTableSectionElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLTextAreaElement` | .cpp, .hpp | `html/HTMLTextAreaElement.cpp`<br>`html/HTMLTextAreaElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLTextFormControlElement` | .cpp, .hpp | `html/HTMLTextFormControlElement.cpp`<br>`html/HTMLTextFormControlElement.h` | LGPL-2.0-or-later |
| `HTML/Elements/HTMLUListElement` | .cpp, .hpp | `html/HTMLUListElement.cpp`<br>`html/HTMLUListElement.h` | LGPL-2.0-or-later |
| `HTML/Forms/FormAssociatedElement` | .hpp | `html/FormAssociatedElement.h` | LGPL-2.0-or-later |
| `HTML/Forms/FormListedElement` | .cpp, .hpp | `html/FormListedElement.cpp`<br>`html/FormListedElement.h` | LGPL-2.0-or-later |
| `HTML/Forms/RadioButtonGroups` | .cpp, .hpp | `dom/RadioButtonGroups.cpp`<br>`dom/RadioButtonGroups.h` | LGPL-2.0-or-later |
| `HTML/Forms/TypeAhead` | .cpp | `html/TypeAhead.cpp` | LGPL-2.0-or-later |
| `HTML/Images/HTMLImageLoader` | .cpp, .hpp | `html/HTMLImageLoader.cpp`<br>`html/HTMLImageLoader.h` | LGPL-2.0-or-later |
| `HTML/InputTypes/BaseTextInputType` | .cpp | `html/BaseTextInputType.cpp` | LGPL-2.0-or-later |
| `HTML/InputTypes/EmailInputType` | .cpp | `html/EmailInputType.cpp` | LGPL-2.0-or-later |
| `HTML/InputTypes/FileInputType` | .cpp | `html/FileInputType.cpp` | LGPL-2.0-or-later |
| `HTML/InputTypes/ImageInputType` | .cpp | `html/ImageInputType.cpp` | LGPL-2.0-or-later |
| `HTML/InputTypes/InputType` | .cpp | `html/InputType.cpp` | LGPL-2.0-or-later |
| `HTML/InputTypes/InputTypeNames` | .hpp | `html/InputTypeNames.h` | LGPL-2.0-or-later |
| `HTML/InputTypes/RadioInputType` | .cpp | `html/RadioInputType.cpp` | LGPL-2.0-or-later |
| `HTML/InputTypes/StepRange` | .cpp, .hpp | `html/StepRange.cpp`<br>`html/StepRange.h` | LGPL-2.0-or-later |
| `HTML/Internals/InlineStyleSheetOwner` | .cpp, .hpp | `dom/InlineStyleSheetOwner.cpp`<br>`dom/InlineStyleSheetOwner.h` | LGPL-2.0-or-later |
| `HTML/Mixins/HTMLOrSVGOrMathMLElement` | .cpp, .hpp | `dom/Element.cpp`<br>`dom/Element.h` | LGPL-2.0-or-later |
| `Loader/Cache/CachedCSSStyleSheet` | .cpp, .hpp | `loader/cache/CachedCSSStyleSheet.cpp`<br>`loader/cache/CachedCSSStyleSheet.h` | LGPL-2.0-or-later |
| `Loader/Cache/CachedImage` | .cpp, .hpp | `loader/cache/CachedImage.cpp`<br>`loader/cache/CachedImage.h` | LGPL-2.0-or-later |
| `Loader/Cache/CachedImageClient` | .hpp | `loader/cache/CachedImageClient.h` | LGPL-2.0-or-later |
| `Loader/Cache/CachedRawResource` | .hpp | `loader/cache/CachedRawResource.h` | LGPL-2.0-or-later |
| `Loader/Cache/CachedResource` | .cpp, .hpp | `loader/cache/CachedResource.cpp`<br>`loader/cache/CachedResource.h` | LGPL-2.0-or-later |
| `Loader/Cache/CachedResourceClient` | .hpp | `loader/cache/CachedResourceClient.h` | LGPL-2.0-or-later |
| `Loader/Cache/CachedResourceClientWalker` | .hpp | `loader/cache/CachedResourceClientWalker.h` | LGPL-2.0-or-later |
| `Loader/Cache/CachedResourceLoader` | .cpp, .hpp | `loader/cache/CachedResourceLoader.cpp`<br>`loader/cache/CachedResourceLoader.h` | LGPL-2.0-or-later |
| `Loader/Cache/CachedSVGDocument` | .cpp, .hpp | `loader/cache/CachedSVGDocument.cpp`<br>`loader/cache/CachedSVGDocument.h` | LGPL-2.0-or-later |
| `Loader/Cache/CachedSVGDocumentClient` | .hpp | `loader/cache/CachedSVGDocumentClient.h` | LGPL-2.0-or-later |
| `Loader/Cache/CachedStyleSheetClient` | .hpp | `loader/cache/CachedStyleSheetClient.h` | LGPL-2.0-or-later |
| `Loader/Cache/MemoryCache` | .cpp, .hpp | `loader/cache/MemoryCache.cpp`<br>`loader/cache/MemoryCache.h` | LGPL-2.0-or-later |
| `Loader/ImageLoader` | .cpp, .hpp | `loader/ImageLoader.cpp`<br>`loader/ImageLoader.h` | LGPL-2.0-or-later |
| `Page/ChromeClient` | .hpp | `page/ChromeClient.h` | LGPL-2.0-or-later |
| `Page/LocalFrame` | .cpp, .hpp | `page/LocalFrame.cpp`<br>`page/LocalFrame.h` | LGPL-2.0-or-later |
| `Page/LocalFrameView` | .cpp, .hpp | `page/LocalFrameView.cpp`<br>`page/LocalFrameView.h` | LGPL-2.0-or-later |
| `Page/MouseEventWithHitTestResults` | .hpp | `page/MouseEventWithHitTestResults.h` | LGPL-2.0-or-later |
| `Page/Page` | .cpp, .hpp | `page/Page.cpp`<br>`page/Page.h` | LGPL-2.0-or-later |
| `Page/SpatialNavigation` | .hpp | `page/SpatialNavigation.h` | LGPL-2.0-or-later |
| `Platform/Choosers/PopupMenu` | .hpp | `platform/PopupMenu.h` | LGPL-2.0-or-later |
| `Platform/Choosers/PopupMenuClient` | .hpp | `platform/PopupMenuClient.h` | LGPL-2.0-or-later |
| `Platform/Events/PlatformTouchEvent` | .hpp | `platform/PlatformTouchEvent.h` | LGPL-2.0-or-later |
| `Platform/Events/PlatformTouchPoint` | .hpp | `platform/PlatformTouchPoint.h` | LGPL-2.0-or-later |
| `Platform/ImageDecoders/GIFImageReader` | .cpp, .hpp | `platform/image-decoders/gif/GIFImageReader.cpp`<br>`platform/image-decoders/gif/GIFImageReader.h` | MPL-1.1/GPL-2.0/LGPL-2.1 tri-licence |
| `Platform/ImageDecoders/JPEGImageDecoder` | .cpp | `platform/image-decoders/jpeg/JPEGImageDecoder.cpp` | MPL-1.1/GPL-2.0/LGPL-2.1 tri-licence |
| `Platform/ImageDecoders/PNGImageDecoder` | .cpp | `platform/image-decoders/png/PNGImageDecoder.cpp` | MPL-1.1/GPL-2.0/LGPL-2.1 tri-licence |
| `Platform/ImageDecoders/ScalableImageDecoder` | .cpp | `platform/image-decoders/ScalableImageDecoder.cpp` | LGPL-2.0-or-later |
| `Platform/Pasteboard/SelectionData` | .cpp, .hpp | `platform/glib/SelectionData.cpp`<br>`platform/glib/SelectionData.h` | LGPL-2.1-or-later |
| `Platform/Scrolling/ScrollAlignment` | .cpp, .hpp | `platform/ScrollAlignment.cpp`<br>`platform/ScrollAlignment.h` | MPL-1.1/GPL-2.0/LGPL-2.1 tri-licence |
| `Rendering/FloatingObjects` | .cpp, .hpp | `rendering/FloatingObjects.cpp`<br>`rendering/FloatingObjects.h` | LGPL-2.0-or-later |
| `Rendering/FormControls/RenderButton` | .cpp, .hpp | `rendering/RenderButton.cpp`<br>`rendering/RenderButton.h` | LGPL-2.0-or-later |
| `Rendering/FormControls/RenderFileUploadControl` | .cpp, .hpp | `rendering/RenderFileUploadControl.cpp`<br>`rendering/RenderFileUploadControl.h` | LGPL-2.0-or-later |
| `Rendering/FormControls/RenderMenuList` | .cpp, .hpp | `rendering/RenderMenuList.cpp`<br>`rendering/RenderMenuList.h` | LGPL-2.0-or-later |
| `Rendering/FormControls/RenderMeter` | .cpp, .hpp | `rendering/RenderMeter.cpp`<br>`rendering/RenderMeter.h` | LGPL-2.0-or-later |
| `Rendering/FormControls/RenderProgress` | .cpp, .hpp | `rendering/RenderProgress.cpp`<br>`rendering/RenderProgress.h` | LGPL-2.0-or-later |
| `Rendering/FormControls/RenderSlider` | .cpp, .hpp | `rendering/RenderSlider.cpp`<br>`rendering/RenderSlider.h` | LGPL-2.0-or-later |
| `Rendering/FormControls/RenderTextControl` | .cpp, .hpp | `rendering/RenderTextControl.cpp`<br>`rendering/RenderTextControl.h` | LGPL-2.0-or-later |
| `Rendering/FormControls/RenderTextControlMultiLine` | .cpp, .hpp | `rendering/RenderTextControlMultiLine.cpp`<br>`rendering/RenderTextControlMultiLine.h` | LGPL-2.0-or-later |
| `Rendering/FormControls/RenderTextControlSingleLine` | .cpp, .hpp | `rendering/RenderTextControlSingleLine.cpp`<br>`rendering/RenderTextControlSingleLine.h` | LGPL-2.0-or-later |
| `Rendering/GeneratedContent/CounterNode` | .cpp, .hpp | `rendering/CounterNode.cpp`<br>`rendering/CounterNode.h` | LGPL-2.0-or-later |
| `Rendering/GeneratedContent/RenderCounter` | .cpp, .hpp | `rendering/RenderCounter.cpp`<br>`rendering/RenderCounter.h` | LGPL-2.0-or-later |
| `Rendering/GeneratedContent/RenderListItem` | .cpp, .hpp | `rendering/RenderListItem.cpp`<br>`rendering/RenderListItem.h` | LGPL-2.0-or-later |
| `Rendering/GeneratedContent/RenderListMarker` | .cpp, .hpp | `rendering/RenderListMarker.cpp`<br>`rendering/RenderListMarker.h` | LGPL-2.0-or-later |
| `Rendering/GeneratedContent/RenderQuote` | .cpp, .hpp | `rendering/RenderQuote.cpp`<br>`rendering/RenderQuote.h` | LGPL-2.0-or-later |
| `Rendering/HitTesting/HitTestLocation` | .cpp, .hpp | `rendering/HitTestLocation.cpp`<br>`rendering/HitTestLocation.h` | LGPL-2.0-or-later |
| `Rendering/HitTesting/HitTestRequest` | .hpp | `rendering/HitTestRequest.h` | LGPL-2.0-or-later |
| `Rendering/HitTesting/HitTestResult` | .cpp, .hpp | `rendering/HitTestResult.cpp`<br>`rendering/HitTestResult.h` | LGPL-2.0-or-later |
| `Rendering/HitTesting/PointerEventsHitRules` | .cpp, .hpp | `rendering/PointerEventsHitRules.cpp`<br>`rendering/PointerEventsHitRules.h` | LGPL-2.0-or-later |
| `Rendering/Layers/RenderLayer` | .cpp, .hpp | `rendering/RenderLayer.cpp`<br>`rendering/RenderLayer.h` | MPL-1.1/GPL-2.0/LGPL-2.1 tri-licence |
| `Rendering/Layers/RenderLayerScrollableArea` | .cpp, .hpp | `rendering/RenderLayerScrollableArea.cpp`<br>`rendering/RenderLayerScrollableArea.h` | MPL-1.1/GPL-2.0/LGPL-2.1 tri-licence |
| `Rendering/LayoutRepainter` | .cpp, .hpp | `rendering/LayoutRepainter.cpp`<br>`rendering/LayoutRepainter.h` | LGPL-2.0-or-later |
| `Rendering/Line/BidiRun` | .cpp, .hpp | `rendering/BidiRun.cpp`<br>`rendering/BidiRun.h` | LGPL-2.0-or-later |
| `Rendering/Line/BreakablePositions` | .hpp | `rendering/BreakablePositions.h` | LGPL-2.0-or-later |
| `Rendering/Line/BreakingContext` | .cpp, .hpp | `rendering/line/BreakingContext.h` | LGPL-2.0-or-later |
| `Rendering/Line/LegacyInlineBox` | .cpp, .hpp | `rendering/LegacyInlineBox.cpp`<br>`rendering/LegacyInlineBox.h` | LGPL-2.0-or-later |
| `Rendering/Line/LegacyInlineFlowBox` | .cpp, .hpp | `rendering/LegacyInlineFlowBox.cpp`<br>`rendering/LegacyInlineFlowBox.h` | LGPL-2.0-or-later |
| `Rendering/Line/LegacyInlineIterator` | .hpp | `rendering/LegacyInlineIterator.h` | LGPL-2.0-or-later |
| `Rendering/Line/LegacyInlineTextBox` | .cpp, .hpp | `rendering/LegacyInlineTextBox.cpp`<br>`rendering/LegacyInlineTextBox.h` | LGPL-2.0-or-later |
| `Rendering/Line/LegacyLineLayout` | .cpp | `rendering/LegacyLineLayout.cpp` | LGPL-2.0-or-later |
| `Rendering/Line/LegacyRootInlineBox` | .cpp, .hpp | `rendering/LegacyRootInlineBox.cpp`<br>`rendering/LegacyRootInlineBox.h` | LGPL-2.0-or-later |
| `Rendering/Line/LineBreaker` | .cpp, .hpp | `rendering/line/LineBreaker.cpp`<br>`rendering/line/LineBreaker.h` | LGPL-2.0-or-later |
| `Rendering/Line/LineInlineHeaders` | .hpp | `rendering/line/LineInlineHeaders.h` | LGPL-2.0-or-later |
| `Rendering/Line/TrailingObjects` | .cpp, .hpp | `rendering/line/TrailingObjects.cpp`<br>`rendering/line/TrailingObjects.h` | LGPL-2.0-or-later |
| `Rendering/Line/WordTrailingSpace` | .hpp | `rendering/line/WordTrailingSpace.h` | LGPL-2.0-or-later |
| `Rendering/Painting/BackgroundPainter` | .cpp, .hpp | `rendering/BackgroundPainter.cpp`<br>`rendering/BackgroundPainter.h` | LGPL-2.0-or-later |
| `Rendering/Painting/BorderPainter` | .cpp, .hpp | `rendering/BorderPainter.cpp`<br>`rendering/BorderPainter.h` | LGPL-2.0-or-later |
| `Rendering/Painting/GapRects` | .hpp | `rendering/GapRects.h` | LGPL-2.0-or-later |
| `Rendering/Painting/NinePieceImagePainter` | .cpp, .hpp | `rendering/NinePieceImagePainter.cpp`<br>`rendering/NinePieceImagePainter.h` | LGPL-2.0-or-later |
| `Rendering/Painting/OutlinePainter` | .cpp, .hpp | `rendering/OutlinePainter.cpp`<br>`rendering/OutlinePainter.h` | LGPL-2.0-or-later |
| `Rendering/Painting/PaintInfo` | .hpp | `rendering/PaintInfo.h` | LGPL-2.0-or-later |
| `Rendering/Painting/PaintPhase` | .hpp | `rendering/PaintPhase.h` | LGPL-2.0-or-later |
| `Rendering/Painting/TextDecorationPainter` | .cpp, .hpp | `rendering/TextDecorationPainter.cpp`<br>`rendering/TextDecorationPainter.h` | LGPL-2.0-or-later |
| `Rendering/Painting/TextPainter` | .cpp, .hpp | `rendering/TextPainter.cpp`<br>`rendering/TextPainter.h` | LGPL-2.0-or-later |
| `Rendering/RenderBlock` | .cpp, .hpp | `rendering/RenderBlock.cpp`<br>`rendering/RenderBlock.h` | LGPL-2.0-or-later |
| `Rendering/RenderBlockFlow` | .cpp, .hpp | `rendering/RenderBlockFlow.cpp`<br>`rendering/RenderBlockFlow.h` | LGPL-2.0-or-later |
| `Rendering/RenderBox` | .cpp, .hpp | `rendering/RenderBox.cpp`<br>`rendering/RenderBox.h` | LGPL-2.0-or-later |
| `Rendering/RenderBoxModelObject` | .cpp, .hpp | `rendering/RenderBoxModelObject.cpp`<br>`rendering/RenderBoxModelObject.h` | LGPL-2.0-or-later |
| `Rendering/RenderElement` | .cpp, .hpp | `rendering/RenderElement.cpp`<br>`rendering/RenderElement.h` | LGPL-2.0-or-later |
| `Rendering/RenderInline` | .cpp, .hpp | `rendering/RenderInline.cpp`<br>`rendering/RenderInline.h` | LGPL-2.0-or-later |
| `Rendering/RenderLayerModelObject` | .cpp, .hpp | `rendering/RenderLayerModelObject.cpp`<br>`rendering/RenderLayerModelObject.h` | LGPL-2.0-or-later |
| `Rendering/RenderLineBreak` | .cpp, .hpp | `rendering/RenderLineBreak.cpp`<br>`rendering/RenderLineBreak.h` | LGPL-2.0-or-later |
| `Rendering/RenderObject` | .cpp, .hpp | `rendering/RenderObject.cpp`<br>`rendering/RenderObject.h` | LGPL-2.0-or-later |
| `Rendering/RenderOverflow` | .hpp | `rendering/RenderOverflow.h` | LGPL-2.0-or-later |
| `Rendering/RenderText` | .cpp, .hpp | `rendering/RenderText.cpp`<br>`rendering/RenderText.h` | LGPL-2.0-or-later |
| `Rendering/RenderTextFragment` | .cpp, .hpp | `rendering/RenderTextFragment.cpp`<br>`rendering/RenderTextFragment.h` | LGPL-2.0-or-later |
| `Rendering/RenderTheme` | .cpp, .hpp | `rendering/RenderTheme.cpp`<br>`rendering/RenderTheme.h` | LGPL-2.0-or-later |
| `Rendering/RenderView` | .cpp, .hpp | `rendering/RenderView.cpp`<br>`rendering/RenderView.h` | LGPL-2.0-or-later |
| `Rendering/Replaced/RenderImage` | .cpp, .hpp | `rendering/RenderImage.cpp`<br>`rendering/RenderImage.h` | LGPL-2.0-or-later |
| `Rendering/Replaced/RenderImageResource` | .cpp, .hpp | `rendering/RenderImageResource.cpp`<br>`rendering/RenderImageResource.h` | LGPL-2.0-or-later |
| `Rendering/Replaced/RenderImageResourceStyleImage` | .cpp, .hpp | `rendering/RenderImageResourceStyleImage.cpp`<br>`rendering/RenderImageResourceStyleImage.h` | LGPL-2.0-or-later |
| `Rendering/Replaced/RenderReplaced` | .cpp, .hpp | `rendering/RenderReplaced.cpp`<br>`rendering/RenderReplaced.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGContainer` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGContainer.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGContainer.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGForeignObject` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGForeignObject.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGForeignObject.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGHiddenContainer` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGHiddenContainer.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGHiddenContainer.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGImage` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGImage.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGImage.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGPath` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGPath.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGPath.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGResource` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGResource.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGResource.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGResourceClipper` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGResourceClipper.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGResourceClipper.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGResourceContainer.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGResourceContainer.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGResourceFilter` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGResourceFilter.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGResourceFilter.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGResourceGradient` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGResourceGradient.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGResourceGradient.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGResourceLinearGradient` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGResourceLinearGradient.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGResourceLinearGradient.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGResourceMarker` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGResourceMarker.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGResourceMarker.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGResourceMasker` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGResourceMasker.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGResourceMasker.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGResourcePattern` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGResourcePattern.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGResourcePattern.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGResourceRadialGradient` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGResourceRadialGradient.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGResourceRadialGradient.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGResourceSolidColor` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGResourceSolidColor.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGResourceSolidColor.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGRoot` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGRoot.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGRoot.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGShape` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGShape.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGShape.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGTransformableContainer` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGTransformableContainer.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGTransformableContainer.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/LegacyRenderSVGViewportContainer` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGViewportContainer.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGViewportContainer.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/SVGResources` | .cpp, .hpp | `rendering/svg/legacy/SVGResources.cpp`<br>`rendering/svg/legacy/SVGResources.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/SVGResourcesCache` | .cpp, .hpp | `rendering/svg/legacy/SVGResourcesCache.cpp`<br>`rendering/svg/legacy/SVGResourcesCache.h` | LGPL-2.0-or-later |
| `Rendering/SVG/Legacy/SVGResourcesCycleSolver` | .cpp, .hpp | `rendering/svg/legacy/SVGResourcesCycleSolver.cpp`<br>`rendering/svg/legacy/SVGResourcesCycleSolver.h` | LGPL-2.0-or-later |
| `Rendering/SVG/RenderSVGBlock` | .cpp, .hpp | `rendering/svg/RenderSVGBlock.cpp`<br>`rendering/svg/RenderSVGBlock.h` | LGPL-2.0-or-later |
| `Rendering/SVG/RenderSVGGradientStop` | .cpp, .hpp | `rendering/svg/RenderSVGGradientStop.cpp`<br>`rendering/svg/RenderSVGGradientStop.h` | LGPL-2.0-or-later |
| `Rendering/SVG/RenderSVGInline` | .cpp, .hpp | `rendering/svg/RenderSVGInline.cpp`<br>`rendering/svg/RenderSVGInline.h` | LGPL-2.0-or-later |
| `Rendering/SVG/RenderSVGInlineText` | .cpp, .hpp | `rendering/svg/RenderSVGInlineText.cpp`<br>`rendering/svg/RenderSVGInlineText.h` | LGPL-2.0-or-later |
| `Rendering/SVG/RenderSVGTSpan` | .hpp | `rendering/svg/RenderSVGTSpan.h` | LGPL-2.0-or-later |
| `Rendering/SVG/RenderSVGText` | .cpp, .hpp | `rendering/svg/RenderSVGText.cpp`<br>`rendering/svg/RenderSVGText.h` | LGPL-2.0-or-later |
| `Rendering/SVG/RenderSVGTextPath` | .cpp, .hpp | `rendering/svg/RenderSVGTextPath.cpp`<br>`rendering/svg/RenderSVGTextPath.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGInlineFlowBox` | .cpp, .hpp | `rendering/svg/SVGInlineFlowBox.cpp`<br>`rendering/svg/SVGInlineFlowBox.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGInlineTextBox` | .cpp, .hpp | `rendering/svg/SVGInlineTextBox.cpp`<br>`rendering/svg/SVGInlineTextBox.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGMarkerData` | .hpp | `rendering/svg/SVGMarkerData.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGPathData` | .cpp, .hpp | `rendering/svg/SVGPathData.cpp`<br>`rendering/svg/SVGPathData.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGRenderSupport` | .cpp, .hpp | `rendering/svg/SVGRenderSupport.cpp`<br>`rendering/svg/SVGRenderSupport.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGRenderingContext` | .cpp, .hpp | `rendering/svg/SVGRenderingContext.cpp`<br>`rendering/svg/SVGRenderingContext.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGRootInlineBox` | .cpp, .hpp | `rendering/svg/SVGRootInlineBox.cpp`<br>`rendering/svg/SVGRootInlineBox.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGSubpathData` | .hpp | `rendering/svg/SVGSubpathData.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGTextChunk` | .cpp, .hpp | `rendering/svg/SVGTextChunk.cpp`<br>`rendering/svg/SVGTextChunk.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGTextChunkBuilder` | .cpp, .hpp | `rendering/svg/SVGTextChunkBuilder.cpp`<br>`rendering/svg/SVGTextChunkBuilder.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGTextFragment` | .hpp | `rendering/svg/SVGTextFragment.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGTextLayoutAttributes` | .cpp, .hpp | `rendering/svg/SVGTextLayoutAttributes.cpp`<br>`rendering/svg/SVGTextLayoutAttributes.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGTextLayoutAttributesBuilder` | .cpp, .hpp | `rendering/svg/SVGTextLayoutAttributesBuilder.cpp`<br>`rendering/svg/SVGTextLayoutAttributesBuilder.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGTextLayoutEngine` | .cpp, .hpp | `rendering/svg/SVGTextLayoutEngine.cpp`<br>`rendering/svg/SVGTextLayoutEngine.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGTextLayoutEngineBaseline` | .cpp, .hpp | `rendering/svg/SVGTextLayoutEngineBaseline.cpp`<br>`rendering/svg/SVGTextLayoutEngineBaseline.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGTextLayoutEngineSpacing` | .cpp, .hpp | `rendering/svg/SVGTextLayoutEngineSpacing.cpp`<br>`rendering/svg/SVGTextLayoutEngineSpacing.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGTextMetrics` | .cpp, .hpp | `rendering/svg/SVGTextMetrics.cpp`<br>`rendering/svg/SVGTextMetrics.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGTextMetricsBuilder` | .cpp, .hpp | `rendering/svg/SVGTextMetricsBuilder.cpp`<br>`rendering/svg/SVGTextMetricsBuilder.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGTextQuery` | .cpp, .hpp | `rendering/svg/SVGTextQuery.cpp`<br>`rendering/svg/SVGTextQuery.h` | LGPL-2.0-or-later |
| `Rendering/SVG/SVGVisitedRendererTracking` | .hpp | `rendering/svg/SVGVisitedRendererTracking.h` | LGPL-2.0-or-later |
| `Rendering/Selection/LogicalSelectionOffsetCaches` | .hpp | `rendering/LogicalSelectionOffsetCaches.h` | LGPL-2.0-or-later |
| `Rendering/Selection/LogicalSelectionOffsetCachesInlines` | .hpp | `rendering/LogicalSelectionOffsetCachesInlines.h` | LGPL-2.0-or-later |
| `Rendering/Selection/RenderObjectHighlightState` | .hpp | `rendering/RenderObject.h` | LGPL-2.0-or-later |
| `Rendering/Selection/RenderSelectionGeometry` | .hpp | `rendering/RenderSelectionGeometry.h` | LGPL-2.0-or-later |
| `Rendering/Style/BorderData` | .hpp | `rendering/style/BorderData.h` | LGPL-2.0-or-later |
| `Rendering/Style/BorderValue` | .cpp, .hpp | `rendering/style/BorderValue.cpp`<br>`rendering/style/BorderValue.h` | LGPL-2.0-or-later |
| `Rendering/Style/CollapsedBorderValue` | .hpp | `rendering/style/CollapsedBorderValue.h` | LGPL-2.0-or-later |
| `Rendering/Style/CounterDirectives` | .hpp | `rendering/style/CounterDirectives.h` | LGPL-2.0-or-later |
| `Rendering/Style/OutlineValue` | .cpp, .hpp | `rendering/style/OutlineValue.cpp`<br>`rendering/style/OutlineValue.h` | LGPL-2.0-or-later |
| `Rendering/Style/RenderStyle+ContainmentInlines` | .hpp | `rendering/style/RenderStyle+GettersInlines.h` | LGPL-2.0-or-later |
| `Rendering/Style/RenderStyle+GettersInlines` | .hpp | `rendering/style/RenderStyle+GettersInlines.h` | LGPL-2.0-or-later |
| `Rendering/Style/RenderStyle+SettersInlines` | .hpp | `rendering/style/RenderStyle+SettersInlines.h` | LGPL-2.0-or-later |
| `Rendering/Style/RenderStyle` | .cpp, .hpp | `rendering/style/RenderStyle.cpp`<br>`rendering/style/RenderStyle.h` | LGPL-2.0-or-later |
| `Rendering/Style/RenderStyleBase+SettersInlines` | .hpp | `rendering/style/RenderStyleBase+SettersInlines.h` | LGPL-2.0-or-later |
| `Rendering/Style/RenderStyleBase` | .hpp | `rendering/style/RenderStyleBase.h` | LGPL-2.0-or-later |
| `Rendering/Style/RenderStyleConstants` | .hpp | `rendering/style/RenderStyleConstants.h` | LGPL-2.0-or-later |
| `Rendering/Style/SVGRenderStyle` | .cpp, .hpp | `rendering/style/SVGRenderStyle.cpp`<br>`rendering/style/SVGRenderStyle.h` | LGPL-2.0-or-later |
| `Rendering/Style/SVGRenderStyleDefs` | .cpp, .hpp | `rendering/style/SVGRenderStyleDefs.cpp`<br>`rendering/style/SVGRenderStyleDefs.h` | LGPL-2.0-or-later |
| `Rendering/Style/StyleBackgroundData` | .cpp, .hpp | `rendering/style/StyleBackgroundData.cpp`<br>`rendering/style/StyleBackgroundData.h` | LGPL-2.0-or-later |
| `Rendering/Style/StyleBoxData` | .cpp, .hpp | `rendering/style/StyleBoxData.cpp`<br>`rendering/style/StyleBoxData.h` | LGPL-2.0-or-later |
| `Rendering/Style/StyleCachedImage` | .cpp, .hpp | `rendering/style/StyleCachedImage.cpp`<br>`rendering/style/StyleCachedImage.h` | LGPL-2.0-or-later |
| `Rendering/Style/StyleCursorImage` | .cpp, .hpp | `rendering/style/StyleCursorImage.cpp`<br>`rendering/style/StyleCursorImage.h` | LGPL-2.0-or-later |
| `Rendering/Style/StyleCustomPropertyData` | .hpp | `rendering/style/StyleCustomPropertyData.h` | LGPL-2.0-or-later |
| `Rendering/Style/StyleGeneratedImage` | .cpp, .hpp | `rendering/style/StyleGeneratedImage.cpp`<br>`rendering/style/StyleGeneratedImage.h` | LGPL-2.0-or-later |
| `Rendering/Style/StyleImage` | .hpp | `rendering/style/StyleImage.h` | LGPL-2.0-or-later |
| `Rendering/Style/StyleImageSet` | .cpp, .hpp | `rendering/style/StyleImageSet.cpp`<br>`rendering/style/StyleImageSet.h` | LGPL-2.0-or-later |
| `Rendering/Style/StyleInheritedData` | .cpp, .hpp | `rendering/style/StyleInheritedData.cpp`<br>`rendering/style/StyleInheritedData.h` | LGPL-2.0-or-later |
| `Rendering/Style/StyleMultiColData` | .cpp, .hpp | `rendering/style/StyleMultiColData.cpp`<br>`rendering/style/StyleMultiColData.h` | LGPL-2.0-or-later |
| `Rendering/Style/StyleMultiImage` | .cpp, .hpp | `rendering/style/StyleMultiImage.cpp`<br>`rendering/style/StyleMultiImage.h` | LGPL-2.0-or-later |
| `Rendering/Style/StyleRareInheritedData` | .cpp, .hpp | `rendering/style/StyleRareInheritedData.cpp`<br>`rendering/style/StyleRareInheritedData.h` | LGPL-2.0-or-later |
| `Rendering/Style/StyleRareNonInheritedData` | .cpp, .hpp | `rendering/style/StyleRareNonInheritedData.cpp`<br>`rendering/style/StyleRareNonInheritedData.h` | LGPL-2.0-or-later |
| `Rendering/Style/StyleSurroundData` | .cpp, .hpp | `rendering/style/StyleSurroundData.cpp`<br>`rendering/style/StyleSurroundData.h` | LGPL-2.0-or-later |
| `Rendering/Style/StyleTransformData` | .cpp, .hpp | `rendering/style/StyleTransformData.cpp`<br>`rendering/style/StyleTransformData.h` | LGPL-2.0-or-later |
| `Rendering/Tables/AutoTableLayout` | .cpp, .hpp | `rendering/AutoTableLayout.cpp`<br>`rendering/AutoTableLayout.h` | LGPL-2.0-or-later |
| `Rendering/Tables/FixedTableLayout` | .cpp, .hpp | `rendering/FixedTableLayout.cpp`<br>`rendering/FixedTableLayout.h` | LGPL-2.0-or-later |
| `Rendering/Tables/RenderTable` | .cpp, .hpp | `rendering/RenderTable.cpp`<br>`rendering/RenderTable.h` | LGPL-2.0-or-later |
| `Rendering/Tables/RenderTableCaption` | .cpp, .hpp | `rendering/RenderTableCaption.cpp`<br>`rendering/RenderTableCaption.h` | LGPL-2.0-or-later |
| `Rendering/Tables/RenderTableCell` | .cpp, .hpp | `rendering/RenderTableCell.cpp`<br>`rendering/RenderTableCell.h` | LGPL-2.0-or-later |
| `Rendering/Tables/RenderTableCol` | .cpp, .hpp | `rendering/RenderTableCol.cpp`<br>`rendering/RenderTableCol.h` | LGPL-2.0-or-later |
| `Rendering/Tables/RenderTableRow` | .cpp, .hpp | `rendering/RenderTableRow.cpp`<br>`rendering/RenderTableRow.h` | LGPL-2.0-or-later |
| `Rendering/Tables/RenderTableSection` | .cpp, .hpp | `rendering/RenderTableSection.cpp`<br>`rendering/RenderTableSection.h` | LGPL-2.0-or-later |
| `Rendering/Tables/TableLayout` | .hpp | `rendering/TableLayout.h` | LGPL-2.0-or-later |
| `Rendering/Updating/RenderTreeBuilderFirstLetter` | .cpp | `rendering/updating/RenderTreeBuilderFirstLetter.cpp` | LGPL-2.0-or-later |
| `Rendering/Updating/RenderTreeBuilderList` | .cpp | `rendering/updating/RenderTreeBuilderList.cpp` | LGPL-2.0-or-later |
| `SVG/Elements/SVGAElement` | .cpp, .hpp | `svg/SVGAElement.cpp`<br>`svg/SVGAElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGAnimateMotionElement` | .cpp, .hpp | `svg/SVGAnimateMotionElement.cpp`<br>`svg/SVGAnimateMotionElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGAnimateTransformElement` | .cpp, .hpp | `svg/SVGAnimateTransformElement.cpp`<br>`svg/SVGAnimateTransformElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGAnimationElement` | .cpp, .hpp | `svg/SVGAnimationElement.cpp`<br>`svg/SVGAnimationElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGCircleElement` | .cpp, .hpp | `svg/SVGCircleElement.cpp`<br>`svg/SVGCircleElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGClipPathElement` | .cpp, .hpp | `svg/SVGClipPathElement.cpp`<br>`svg/SVGClipPathElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGComponentTransferFunctionElement` | .cpp, .hpp | `svg/SVGComponentTransferFunctionElement.cpp`<br>`svg/SVGComponentTransferFunctionElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGDefsElement` | .cpp, .hpp | `svg/SVGDefsElement.cpp`<br>`svg/SVGDefsElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGDescElement` | .cpp, .hpp | `svg/SVGDescElement.cpp`<br>`svg/SVGDescElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGElement` | .cpp, .hpp | `svg/SVGElement.cpp`<br>`svg/SVGElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGEllipseElement` | .cpp, .hpp | `svg/SVGEllipseElement.cpp`<br>`svg/SVGEllipseElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEBlendElement` | .cpp, .hpp | `svg/SVGFEBlendElement.cpp`<br>`svg/SVGFEBlendElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEColorMatrixElement` | .cpp, .hpp | `svg/SVGFEColorMatrixElement.cpp`<br>`svg/SVGFEColorMatrixElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEComponentTransferElement` | .cpp, .hpp | `svg/SVGFEComponentTransferElement.cpp`<br>`svg/SVGFEComponentTransferElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFECompositeElement` | .cpp, .hpp | `svg/SVGFECompositeElement.cpp`<br>`svg/SVGFECompositeElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEConvolveMatrixElement` | .cpp, .hpp | `svg/SVGFEConvolveMatrixElement.cpp`<br>`svg/SVGFEConvolveMatrixElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEDiffuseLightingElement` | .cpp, .hpp | `svg/SVGFEDiffuseLightingElement.cpp`<br>`svg/SVGFEDiffuseLightingElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEDisplacementMapElement` | .cpp, .hpp | `svg/SVGFEDisplacementMapElement.cpp`<br>`svg/SVGFEDisplacementMapElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEDistantLightElement` | .cpp, .hpp | `svg/SVGFEDistantLightElement.cpp`<br>`svg/SVGFEDistantLightElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEDropShadowElement` | .cpp, .hpp | `svg/SVGFEDropShadowElement.cpp`<br>`svg/SVGFEDropShadowElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEFloodElement` | .cpp, .hpp | `svg/SVGFEFloodElement.cpp`<br>`svg/SVGFEFloodElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEFuncAElement` | .hpp | `svg/SVGFEFuncAElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEFuncBElement` | .hpp | `svg/SVGFEFuncBElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEFuncGElement` | .hpp | `svg/SVGFEFuncGElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEFuncRElement` | .hpp | `svg/SVGFEFuncRElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEGaussianBlurElement` | .cpp, .hpp | `svg/SVGFEGaussianBlurElement.cpp`<br>`svg/SVGFEGaussianBlurElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEImageElement` | .cpp, .hpp | `svg/SVGFEImageElement.cpp`<br>`svg/SVGFEImageElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFELightElement` | .cpp, .hpp | `svg/SVGFELightElement.cpp`<br>`svg/SVGFELightElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEMergeElement` | .cpp, .hpp | `svg/SVGFEMergeElement.cpp`<br>`svg/SVGFEMergeElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEMergeNodeElement` | .cpp, .hpp | `svg/SVGFEMergeNodeElement.cpp`<br>`svg/SVGFEMergeNodeElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEMorphologyElement` | .cpp, .hpp | `svg/SVGFEMorphologyElement.cpp`<br>`svg/SVGFEMorphologyElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEOffsetElement` | .cpp, .hpp | `svg/SVGFEOffsetElement.cpp`<br>`svg/SVGFEOffsetElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFEPointLightElement` | .cpp, .hpp | `svg/SVGFEPointLightElement.cpp`<br>`svg/SVGFEPointLightElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFESpecularLightingElement` | .cpp, .hpp | `svg/SVGFESpecularLightingElement.cpp`<br>`svg/SVGFESpecularLightingElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFESpotLightElement` | .cpp, .hpp | `svg/SVGFESpotLightElement.cpp`<br>`svg/SVGFESpotLightElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFETileElement` | .cpp, .hpp | `svg/SVGFETileElement.cpp`<br>`svg/SVGFETileElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFETurbulenceElement` | .cpp, .hpp | `svg/SVGFETurbulenceElement.cpp`<br>`svg/SVGFETurbulenceElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGFilterElement` | .cpp, .hpp | `svg/SVGFilterElement.cpp`<br>`svg/SVGFilterElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGForeignObjectElement` | .cpp, .hpp | `svg/SVGForeignObjectElement.cpp`<br>`svg/SVGForeignObjectElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGGElement` | .cpp, .hpp | `svg/SVGGElement.cpp`<br>`svg/SVGGElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGGeometryElement` | .cpp, .hpp | `svg/SVGGeometryElement.cpp`<br>`svg/SVGGeometryElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGGradientElement` | .cpp, .hpp | `svg/SVGGradientElement.cpp`<br>`svg/SVGGradientElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGGraphicsElement` | .cpp, .hpp | `svg/SVGGraphicsElement.cpp`<br>`svg/SVGGraphicsElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGImageElement` | .cpp, .hpp | `svg/SVGImageElement.cpp`<br>`svg/SVGImageElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGLineElement` | .cpp, .hpp | `svg/SVGLineElement.cpp`<br>`svg/SVGLineElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGLinearGradientElement` | .cpp, .hpp | `svg/SVGLinearGradientElement.cpp`<br>`svg/SVGLinearGradientElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGMPathElement` | .cpp, .hpp | `svg/SVGMPathElement.cpp`<br>`svg/SVGMPathElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGMarkerElement` | .cpp, .hpp | `svg/SVGMarkerElement.cpp`<br>`svg/SVGMarkerElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGMaskElement` | .cpp, .hpp | `svg/SVGMaskElement.cpp`<br>`svg/SVGMaskElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGMetadataElement` | .cpp, .hpp | `svg/SVGMetadataElement.cpp`<br>`svg/SVGMetadataElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGPathElement` | .cpp, .hpp | `svg/SVGPathElement.cpp`<br>`svg/SVGPathElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGPatternElement` | .cpp, .hpp | `svg/SVGPatternElement.cpp`<br>`svg/SVGPatternElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGPolyElement` | .cpp, .hpp | `svg/SVGPolyElement.cpp`<br>`svg/SVGPolyElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGPolygonElement` | .cpp, .hpp | `svg/SVGPolygonElement.cpp`<br>`svg/SVGPolygonElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGPolylineElement` | .cpp, .hpp | `svg/SVGPolylineElement.cpp`<br>`svg/SVGPolylineElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGRadialGradientElement` | .cpp, .hpp | `svg/SVGRadialGradientElement.cpp`<br>`svg/SVGRadialGradientElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGRectElement` | .cpp, .hpp | `svg/SVGRectElement.cpp`<br>`svg/SVGRectElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGScriptElement` | .cpp, .hpp | `svg/SVGScriptElement.cpp`<br>`svg/SVGScriptElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGSetElement` | .hpp | `svg/SVGSetElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGStopElement` | .cpp, .hpp | `svg/SVGStopElement.cpp`<br>`svg/SVGStopElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGStyleElement` | .cpp, .hpp | `svg/SVGStyleElement.cpp`<br>`svg/SVGStyleElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGSvgElement` | .cpp, .hpp | `svg/SVGSVGElement.cpp`<br>`svg/SVGSVGElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGSwitchElement` | .cpp, .hpp | `svg/SVGSwitchElement.cpp`<br>`svg/SVGSwitchElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGSymbolElement` | .cpp, .hpp | `svg/SVGSymbolElement.cpp`<br>`svg/SVGSymbolElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGTSpanElement` | .cpp, .hpp | `svg/SVGTSpanElement.cpp`<br>`svg/SVGTSpanElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGTextContentElement` | .cpp, .hpp | `svg/SVGTextContentElement.cpp`<br>`svg/SVGTextContentElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGTextElement` | .cpp, .hpp | `svg/SVGTextElement.cpp`<br>`svg/SVGTextElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGTextPathElement` | .cpp, .hpp | `svg/SVGTextPathElement.cpp`<br>`svg/SVGTextPathElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGTextPositioningElement` | .cpp, .hpp | `svg/SVGTextPositioningElement.cpp`<br>`svg/SVGTextPositioningElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGTitleElement` | .cpp, .hpp | `svg/SVGTitleElement.cpp`<br>`svg/SVGTitleElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGUseElement` | .cpp, .hpp | `svg/SVGUseElement.cpp`<br>`svg/SVGUseElement.h` | LGPL-2.0-or-later |
| `SVG/Elements/SVGViewElement` | .cpp, .hpp | `svg/SVGViewElement.cpp`<br>`svg/SVGViewElement.h` | LGPL-2.0-or-later |
| `SVG/GradientAttributes` | .hpp | `svg/GradientAttributes.h` | LGPL-2.0-or-later |
| `SVG/Graphics/Filters/SVGFilterRenderer` | .cpp, .hpp | `svg/graphics/filters/SVGFilterRenderer.cpp`<br>`svg/graphics/filters/SVGFilterRenderer.h` | LGPL-2.0-or-later |
| `SVG/Graphics/SVGImageCache` | .cpp, .hpp | `svg/graphics/SVGImageCache.cpp`<br>`svg/graphics/SVGImageCache.h` | LGPL-2.0-or-later |
| `SVG/Graphics/SVGImageForContainer` | .cpp | `svg/graphics/SVGImageForContainer.cpp` | LGPL-2.0-or-later |
| `SVG/Internals/SVGParserUtilities` | .cpp, .hpp | `svg/SVGParserUtilities.cpp`<br>`svg/SVGParserUtilities.h` | LGPL-2.0-or-later |
| `SVG/Internals/SVGPathBlender` | .cpp, .hpp | `svg/SVGPathBlender.cpp`<br>`svg/SVGPathBlender.h` | LGPL-2.0-or-later |
| `SVG/Internals/SVGPathBuilder` | .cpp, .hpp | `svg/SVGPathBuilder.cpp`<br>`svg/SVGPathBuilder.h` | LGPL-2.0-or-later |
| `SVG/Internals/SVGPathByteStream` | .cpp, .hpp | `svg/SVGPathByteStream.h` | LGPL-2.0-or-later |
| `SVG/Internals/SVGPathByteStreamBuilder` | .cpp, .hpp | `svg/SVGPathByteStreamBuilder.cpp`<br>`svg/SVGPathByteStreamBuilder.h` | LGPL-2.0-or-later |
| `SVG/Internals/SVGPathByteStreamSource` | .cpp, .hpp | `svg/SVGPathByteStreamSource.cpp`<br>`svg/SVGPathByteStreamSource.h` | LGPL-2.0-or-later |
| `SVG/Internals/SVGPathConsumer` | .hpp | `svg/SVGPathConsumer.h` | LGPL-2.0-or-later |
| `SVG/Internals/SVGPathParser` | .cpp, .hpp | `svg/SVGPathParser.cpp`<br>`svg/SVGPathParser.h` | LGPL-2.0-or-later |
| `SVG/Internals/SVGPathSegType` | .hpp | `svg/SVGPathSeg.h` | LGPL-2.0-or-later |
| `SVG/Internals/SVGPathSource` | .hpp | `svg/SVGPathSource.h` | LGPL-2.0-or-later |
| `SVG/Internals/SVGPathStringBuilder` | .cpp, .hpp | `svg/SVGPathStringBuilder.cpp`<br>`svg/SVGPathStringBuilder.h` | LGPL-2.0-or-later |
| `SVG/Internals/SVGPathStringViewSource` | .cpp, .hpp | `svg/SVGPathStringViewSource.cpp`<br>`svg/SVGPathStringViewSource.h` | LGPL-2.0-or-later |
| `SVG/Internals/SVGPathTraversalStateBuilder` | .cpp, .hpp | `svg/SVGPathTraversalStateBuilder.cpp`<br>`svg/SVGPathTraversalStateBuilder.h` | LGPL-2.0-or-later |
| `SVG/Internals/SVGPathUtilities` | .cpp, .hpp | `svg/SVGPathUtilities.cpp`<br>`svg/SVGPathUtilities.h` | LGPL-2.0-or-later |
| `SVG/LinearGradientAttributes` | .hpp | `svg/LinearGradientAttributes.h` | LGPL-2.0-or-later |
| `SVG/PatternAttributes` | .hpp | `svg/PatternAttributes.h` | LGPL-2.0-or-later |
| `SVG/Properties/SVGProperty` | .hpp | `svg/properties/SVGProperty.h` | LGPL-2.0-or-later |
| `SVG/Properties/SVGPropertyTraits` | .hpp | `svg/properties/SVGPropertyTraits.h` | LGPL-2.0-or-later |
| `SVG/RadialGradientAttributes` | .hpp | `svg/RadialGradientAttributes.h` | LGPL-2.0-or-later |
| `SVG/SVGAngleValue` | .cpp, .hpp | `svg/SVGAngleValue.cpp`<br>`svg/SVGAngleValue.h` | LGPL-2.0-or-later |
| `SVG/SVGAnimateElementBase` | .cpp, .hpp | `svg/SVGAnimateElementBase.cpp`<br>`svg/SVGAnimateElementBase.h` | LGPL-2.0-or-later |
| `SVG/SVGDocument` | .hpp | `svg/SVGDocument.h` | LGPL-2.0-or-later |
| `SVG/SVGDocumentExtensions` | .cpp, .hpp | `svg/SVGDocumentExtensions.cpp`<br>`svg/SVGDocumentExtensions.h` | LGPL-2.0-or-later |
| `SVG/SVGElementRareData` | .hpp | `svg/SVGElementRareData.h` | LGPL-2.0-or-later |
| `SVG/SVGFilterPrimitiveStandardAttributes` | .cpp, .hpp | `svg/SVGFilterPrimitiveStandardAttributes.cpp`<br>`svg/SVGFilterPrimitiveStandardAttributes.h` | LGPL-2.0-or-later |
| `SVG/SVGFitToViewBox` | .cpp, .hpp | `svg/SVGFitToViewBox.cpp`<br>`svg/SVGFitToViewBox.h` | LGPL-2.0-or-later |
| `SVG/SVGImageLoader` | .cpp, .hpp | `svg/SVGImageLoader.cpp`<br>`svg/SVGImageLoader.h` | LGPL-2.0-or-later |
| `SVG/SVGLengthContext` | .cpp, .hpp | `svg/SVGLengthContext.cpp`<br>`svg/SVGLengthContext.h` | LGPL-2.0-or-later |
| `SVG/SVGLengthValue` | .cpp, .hpp | `svg/SVGLengthValue.cpp`<br>`svg/SVGLengthValue.h` | LGPL-2.0-or-later |
| `SVG/SVGLocatable` | .cpp, .hpp | `svg/SVGLocatable.cpp`<br>`svg/SVGLocatable.h` | LGPL-2.0-or-later |
| `SVG/SVGPathSegList` | .hpp | `svg/SVGPathSegList.h` | LGPL-2.0-or-later |
| `SVG/SVGPreserveAspectRatioValue` | .cpp, .hpp | `svg/SVGPreserveAspectRatioValue.cpp`<br>`svg/SVGPreserveAspectRatioValue.h` | LGPL-2.0-or-later |
| `SVG/SVGRect` | .hpp | `svg/SVGRect.h` | LGPL-2.0-or-later |
| `SVG/SVGTests` | .cpp, .hpp | `svg/SVGTests.cpp`<br>`svg/SVGTests.h` | LGPL-2.0-or-later |
| `SVG/SVGTransformDistance` | .cpp, .hpp | `svg/SVGTransformDistance.cpp`<br>`svg/SVGTransformDistance.h` | LGPL-2.0-or-later |
| `SVG/SVGTransformList` | .hpp | `svg/SVGTransformList.h` | LGPL-2.0-or-later |
| `SVG/SVGTransformValue` | .cpp, .hpp | `svg/SVGTransformValue.h` | LGPL-2.0-or-later |
| `SVG/SVGTransformable` | .cpp, .hpp | `svg/SVGTransformableInlines.h` | LGPL-2.0-or-later |
| `SVG/SVGURIReference` | .cpp, .hpp | `svg/SVGURIReference.cpp`<br>`svg/SVGURIReference.h` | LGPL-2.0-or-later |
| `SVG/SVGUnitTypes` | .hpp | `svg/SVGUnitTypes.h` | LGPL-2.0-or-later |
| `SVG/SVGViewSpec` | .cpp, .hpp | `svg/SVGViewSpec.cpp`<br>`svg/SVGViewSpec.h` | LGPL-2.0-or-later |
| `Style/Adjuster` | .cpp | `style/StyleAdjuster.cpp` | LGPL-2.0-or-later |
| `Style/Builder/Builder` | .cpp | `style/StyleBuilder.cpp` | LGPL-2.0-or-later |
| `Style/Builder/BuilderState` | .cpp | `style/StyleBuilderState.cpp` | LGPL-2.0-or-later |
| `Style/Builder/FontSizeFunctions` | .cpp | `style/StyleFontSizeFunctions.cpp` | LGPL-2.0-or-later |
| `Style/Computed/ComputedStyle+ConstructionInlines` | .hpp | `style/computed/StyleComputedStyle+ConstructionInlines.h` | LGPL-2.0-or-later |
| `Style/Computed/ComputedStyle+GettersInlines` | .hpp | `style/computed/StyleComputedStyle+GettersInlines.h` | LGPL-2.0-or-later |
| `Style/Computed/ComputedStyle+InitialInlines` | .hpp | `style/computed/StyleComputedStyle+InitialInlines.h` | LGPL-2.0-or-later |
| `Style/Computed/ComputedStyle+SettersInlines` | .hpp | `style/computed/StyleComputedStyle+SettersInlines.h` | LGPL-2.0-or-later |
| `Style/Computed/ComputedStyle` | .cpp, .hpp | `style/computed/StyleComputedStyle.cpp`<br>`style/computed/StyleComputedStyle.h` | LGPL-2.0-or-later |
| `Style/Computed/ComputedStyleBase+ConstructionInlines` | .hpp | `style/computed/StyleComputedStyleBase+ConstructionInlines.h` | LGPL-2.0-or-later |
| `Style/Computed/ComputedStyleBase+SettersInlines` | .hpp | `style/computed/StyleComputedStyleBase+SettersInlines.h` | LGPL-2.0-or-later |
| `Style/Computed/ComputedStyleBase` | .hpp | `style/computed/StyleComputedStyleBase.h` | LGPL-2.0-or-later |
| `Style/Extractor/Extractor` | .cpp | `style/StyleExtractor.cpp` | LGPL-2.1-or-later |
| `Style/ResolveForDocument` | .cpp | `style/StyleResolveForDocument.cpp` | LGPL-2.0-or-later |
| `Style/ResolvedStyle` | .hpp | `style/ResolvedStyle.h` | LGPL-2.0-or-later |
| `Style/Resolver` | .cpp, .hpp | `style/StyleResolver.cpp`<br>`style/StyleResolver.h` | LGPL-2.0-or-later |
| `Style/Rules/ElementRuleCollector` | .cpp, .hpp | `style/ElementRuleCollector.cpp`<br>`style/ElementRuleCollector.h` | LGPL-2.0-or-later |
| `Style/Rules/MatchedDeclarationsCache` | .cpp | `style/MatchedDeclarationsCache.cpp` | LGPL-2.0-or-later |
| `Style/Rules/RuleData` | .cpp, .hpp | `style/RuleData.cpp`<br>`style/RuleData.h` | LGPL-2.0-or-later |
| `Style/Rules/RuleFeature` | .cpp, .hpp | `style/RuleFeature.cpp`<br>`style/RuleFeature.h` | LGPL-2.0-or-later |
| `Style/Rules/RuleSet` | .cpp, .hpp | `style/RuleSet.cpp`<br>`style/RuleSet.h` | LGPL-2.0-or-later |
| `Style/Rules/RuleSetBuilder` | .cpp, .hpp | `style/RuleSetBuilder.cpp`<br>`style/RuleSetBuilder.h` | LGPL-2.0-or-later |
| `Style/Rules/ScopeRuleSets` | .cpp, .hpp | `style/StyleScopeRuleSets.cpp`<br>`style/StyleScopeRuleSets.h` | LGPL-2.0-or-later |
| `Style/Rules/UserAgentStyle` | .cpp, .hpp | `style/UserAgentStyle.cpp`<br>`style/UserAgentStyle.h` | LGPL-2.0-or-later |
| `Style/Scope/Scope` | .cpp, .hpp | `style/StyleScope.cpp`<br>`style/StyleScope.h` | LGPL-2.0-or-later |
| `Style/TreeResolver` | .cpp | `style/StyleTreeResolver.cpp` | LGPL-2.0-or-later |
| `Style/Values/Backgrounds/BackgroundLayer` | .cpp, .hpp | `style/values/backgrounds/StyleBackgroundLayer.cpp`<br>`style/values/backgrounds/StyleBackgroundLayer.h` | LGPL-2.0-or-later |
| `Style/Values/Backgrounds/BorderImage` | .hpp | `style/values/backgrounds/StyleBorderImage.h` | LGPL-2.0-or-later |
| `Style/Values/Backgrounds/FillLayers` | .hpp | `style/values/backgrounds/StyleFillLayers.h` | LGPL-2.0-or-later |
| `Style/Values/Content/Quotes` | .cpp, .hpp | `style/values/content/StyleQuotes.cpp`<br>`style/values/content/StyleQuotes.h` | LGPL-2.0-or-later |
| `Style/Values/Masking/MaskBorder` | .hpp | `style/values/masking/StyleMaskBorder.h` | LGPL-2.0-or-later |
| `Style/Values/Masking/MaskLayer` | .cpp, .hpp | `style/values/masking/StyleMaskLayer.cpp`<br>`style/values/masking/StyleMaskLayer.h` | LGPL-2.0-or-later |
| `Style/Values/Transforms/Functions/MatrixTransformFunction` | .cpp, .hpp | `style/values/transforms/functions/StyleMatrixTransformFunction.cpp`<br>`style/values/transforms/functions/StyleMatrixTransformFunction.h` | LGPL-2.0-or-later |
| `Style/Values/Transforms/Functions/RotateTransformFunction` | .cpp, .hpp | `style/values/transforms/functions/StyleRotateTransformFunction.cpp`<br>`style/values/transforms/functions/StyleRotateTransformFunction.h` | LGPL-2.0-or-later |
| `Style/Values/Transforms/Functions/ScaleTransformFunction` | .cpp, .hpp | `style/values/transforms/functions/StyleScaleTransformFunction.cpp`<br>`style/values/transforms/functions/StyleScaleTransformFunction.h` | LGPL-2.0-or-later |
| `Style/Values/Transforms/Functions/SkewTransformFunction` | .cpp, .hpp | `style/values/transforms/functions/StyleSkewTransformFunction.cpp`<br>`style/values/transforms/functions/StyleSkewTransformFunction.h` | LGPL-2.0-or-later |
| `Style/Values/Transforms/Functions/TransformFunctionBase` | .hpp | `style/values/transforms/functions/StyleTransformFunctionBase.h` | LGPL-2.0-or-later |
| `Style/Values/Transforms/Functions/TranslateTransformFunction` | .cpp, .hpp | `style/values/transforms/functions/StyleTranslateTransformFunction.cpp`<br>`style/values/transforms/functions/StyleTranslateTransformFunction.h` | LGPL-2.0-or-later |
| `Text/BidiContext` | .cpp, .hpp | `platform/text/BidiContext.cpp`<br>`platform/text/BidiContext.h` | LGPL-2.0-or-later |
| `Text/BidiResolver` | .hpp | `platform/text/BidiResolver.h` | LGPL-2.0-or-later |
| `Text/BidiRunList` | .hpp | `platform/text/BidiRunList.h` | LGPL-2.0-or-later |
| `Text/TextBreakIterator` | .cpp, .hpp | `WTF/wtf/text/TextBreakIterator.cpp`<br>`WTF/wtf/text/TextBreakIterator.h` | LGPL-2.0-or-later |
| `XML/XMLDocumentParser` | .cpp, .hpp | `xml/parser/XMLDocumentParser.cpp`<br>`xml/parser/XMLDocumentParser.h` | LGPL-2.0-or-later |
| `XML/XMLDocumentParserLibxml2` | .cpp | `xml/parser/XMLDocumentParserLibxml2.cpp` | LGPL-2.0-or-later |
| `codegen/user-agent-style-sheets/html` | .css | `css/html.css` | LGPL-2.0-or-later |
| `codegen/user-agent-style-sheets/quirks` | .css | `css/quirks.css` | LGPL-2.0-or-later |
| `codegen/user-agent-style-sheets/themeAdwaita` | .css | `css/themeAdwaita.css` | LGPL-2.0-or-later |

## BSD-licensed

| Booey unit | Files | WebKit source | Licence |
|---|---|---|---|
| `Animation/AnimationEffect` | .cpp, .hpp | `animation/AnimationEffect.cpp`<br>`animation/AnimationEffect.h` | BSD-2-Clause |
| `Animation/CSSAnimation` | .cpp, .hpp | `animation/CSSAnimation.cpp`<br>`animation/CSSAnimation.h` | BSD-2-Clause |
| `Animation/CSSTransition` | .cpp, .hpp | `animation/CSSTransition.cpp`<br>`animation/CSSTransition.h` | BSD-2-Clause |
| `Animation/ElementAnimationRareData` | .cpp, .hpp | `animation/ElementAnimationRareData.cpp`<br>`animation/ElementAnimationRareData.h` | BSD-2-Clause |
| `Animation/Events/AnimationEventBase` | .cpp, .hpp | `animation/AnimationEventBase.cpp`<br>`animation/AnimationEventBase.h` | BSD-2-Clause |
| `Animation/Events/AnimationPlaybackEvent` | .cpp, .hpp | `animation/AnimationPlaybackEvent.cpp`<br>`animation/AnimationPlaybackEvent.h` | BSD-2-Clause |
| `Animation/Events/CSSAnimationEvent` | .cpp, .hpp | `animation/CSSAnimationEvent.cpp`<br>`animation/CSSAnimationEvent.h` | BSD-2-Clause |
| `Animation/Events/CSSTransitionEvent` | .cpp, .hpp | `animation/CSSTransitionEvent.cpp`<br>`animation/CSSTransitionEvent.h` | BSD-2-Clause |
| `Animation/Events/StyleOriginatedAnimationEvent` | .cpp, .hpp | `animation/StyleOriginatedAnimationEvent.cpp`<br>`animation/StyleOriginatedAnimationEvent.h` | BSD-2-Clause |
| `Animation/KeyframeEffect` | .cpp, .hpp | `animation/KeyframeEffect.cpp`<br>`animation/KeyframeEffect.h` | BSD-2-Clause |
| `Animation/KeyframeEffectStack` | .cpp, .hpp | `animation/KeyframeEffectStack.cpp`<br>`animation/KeyframeEffectStack.h` | BSD-2-Clause |
| `Animation/Keyframes/CompositeOperationOrAuto` | .hpp | `animation/CompositeOperationOrAuto.h` | BSD-2-Clause |
| `Animation/Keyframes/KeyframeInterpolation` | .cpp, .hpp | `animation/KeyframeInterpolation.cpp`<br>`animation/KeyframeInterpolation.h` | BSD-2-Clause |
| `Animation/StyleOriginatedAnimation` | .cpp, .hpp | `animation/StyleOriginatedAnimation.cpp`<br>`animation/StyleOriginatedAnimation.h` | BSD-2-Clause |
| `Animation/Timelines/AnimationTimeline` | .cpp, .hpp | `animation/AnimationTimeline.cpp`<br>`animation/AnimationTimeline.h` | BSD-2-Clause |
| `Animation/Timelines/AnimationTimelinesController` | .cpp, .hpp | `animation/AnimationTimelinesController.cpp`<br>`animation/AnimationTimelinesController.h` | BSD-2-Clause |
| `Animation/Timelines/DocumentTimeline` | .cpp, .hpp | `animation/DocumentTimeline.cpp`<br>`animation/DocumentTimeline.h` | BSD-2-Clause |
| `Animation/Timelines/ScrollTimeline` | .cpp, .hpp | `animation/ScrollTimeline.cpp`<br>`animation/ScrollTimeline.h` | BSD-2-Clause |
| `Animation/Timelines/StyleOriginatedTimelinesController` | .cpp, .hpp | `animation/StyleOriginatedTimelinesController.cpp`<br>`animation/StyleOriginatedTimelinesController.h` | BSD-2-Clause |
| `Animation/Timelines/TimelineRangeValue` | .cpp, .hpp | `animation/TimelineRangeValue.cpp`<br>`animation/TimelineRangeValue.h` | BSD-2-Clause |
| `Animation/Timelines/ViewTimeline` | .cpp, .hpp | `animation/ViewTimeline.cpp`<br>`animation/ViewTimeline.h` | BSD-2-Clause |
| `Animation/Timing/AnimationEffectTiming` | .cpp, .hpp | `animation/AnimationEffectTiming.cpp`<br>`animation/AnimationEffectTiming.h` | BSD-2-Clause |
| `Animation/Timing/ComputedEffectTiming` | .hpp | `animation/ComputedEffectTiming.h` | BSD-2-Clause |
| `Animation/Timing/EffectTiming` | .hpp | `animation/EffectTiming.h` | BSD-2-Clause |
| `Animation/Timing/WebAnimationTime` | .cpp, .hpp | `animation/WebAnimationTime.cpp`<br>`animation/WebAnimationTime.h` | BSD-2-Clause |
| `Animation/WebAnimation` | .cpp, .hpp | `animation/WebAnimation.cpp`<br>`animation/WebAnimation.h` | BSD-2-Clause |
| `Animation/WebAnimationTypes` | .hpp | `animation/WebAnimationTypes.h` | BSD-2-Clause |
| `Animation/WebAnimationUtilities` | .cpp, .hpp | `animation/WebAnimationUtilities.cpp`<br>`animation/WebAnimationUtilities.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSConditionRule` | .cpp, .hpp | `css/CSSConditionRule.cpp`<br>`css/CSSConditionRule.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSContainerRule` | .cpp, .hpp | `css/CSSContainerRule.cpp`<br>`css/CSSContainerRule.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSCounterStyleRule` | .cpp, .hpp | `css/CSSCounterStyleRule.cpp`<br>`css/CSSCounterStyleRule.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSFontFeatureValuesRule` | .cpp, .hpp | `css/CSSFontFeatureValuesRule.cpp`<br>`css/CSSFontFeatureValuesRule.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSFontPaletteValuesRule` | .cpp, .hpp | `css/CSSFontPaletteValuesRule.cpp`<br>`css/CSSFontPaletteValuesRule.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSFunctionDeclarations` | .cpp, .hpp | `css/CSSFunctionDeclarations.cpp`<br>`css/CSSFunctionDeclarations.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSFunctionRule` | .cpp, .hpp | `css/CSSFunctionRule.cpp`<br>`css/CSSFunctionRule.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSGroupingRule` | .cpp | `css/CSSGroupingRule.cpp` | BSD-2-Clause |
| `CSS/CSSOM/CSSKeyframeRule` | .cpp, .hpp | `css/CSSKeyframeRule.cpp`<br>`css/CSSKeyframeRule.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSKeyframesRule` | .cpp, .hpp | `css/CSSKeyframesRule.cpp`<br>`css/CSSKeyframesRule.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSLayerBlockRule` | .cpp, .hpp | `css/CSSLayerBlockRule.cpp`<br>`css/CSSLayerBlockRule.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSLayerStatementRule` | .cpp, .hpp | `css/CSSLayerStatementRule.cpp`<br>`css/CSSLayerStatementRule.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSNamespaceRule` | .cpp, .hpp | `css/CSSNamespaceRule.cpp`<br>`css/CSSNamespaceRule.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSPositionTryRule` | .cpp, .hpp | `css/CSSPositionTryRule.cpp`<br>`css/CSSPositionTryRule.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSPropertyRule` | .cpp, .hpp | `css/CSSPropertyRule.cpp`<br>`css/CSSPropertyRule.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSScopeRule` | .cpp, .hpp | `css/CSSScopeRule.cpp`<br>`css/CSSScopeRule.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSStartingStyleRule` | .cpp, .hpp | `css/CSSStartingStyleRule.cpp`<br>`css/CSSStartingStyleRule.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSStyleDeclaration` | .cpp | `css/CSSStyleDeclaration.cpp` | BSD-2-Clause |
| `CSS/CSSOM/CSSStyleProperties` | .cpp, .hpp | `css/CSSStyleProperties.cpp`<br>`css/CSSStyleProperties.h` | BSD-2-Clause |
| `CSS/CSSOM/CSSSupportsRule` | .cpp, .hpp | `css/CSSSupportsRule.cpp`<br>`css/CSSSupportsRule.h` | BSD-3-Clause |
| `CSS/CSSOM/DOMCSSNamespace` | .cpp, .hpp | `css/DOMCSSNamespace.cpp`<br>`css/DOMCSSRegisterCustomProperty.cpp`<br>`css/DOMCSSNamespace.h`<br>`css/DOMCSSRegisterCustomProperty.h` | BSD-3-Clause AND BSD-2-Clause |
| `CSS/CSSOM/StyleAttributeMutationScope` | .cpp, .hpp | `css/StyleAttributeMutationScope.cpp`<br>`css/StyleAttributeMutationScope.h` | BSD-2-Clause |
| `CSS/Calc/CalcType` | .cpp, .hpp | `css/calc/CSSCalcType.cpp`<br>`css/calc/CSSCalcType.h` | BSD-2-Clause |
| `CSS/Calc/Executor` | .hpp | `css/calc/CSSCalcExecutor.h` | BSD-2-Clause |
| `CSS/Calc/Operator` | .hpp | `css/calc/CSSCalcOperator.h` | BSD-2-Clause |
| `CSS/Calc/RandomCachingKey` | .hpp | `css/calc/CSSCalcRandomCachingKey.h` | BSD-2-Clause |
| `CSS/Calc/RandomCachingKeyMap` | .hpp | `css/calc/CSSCalcRandomCachingKeyMap.h` | BSD-2-Clause |
| `CSS/Calc/SymbolTable` | .hpp | `css/calc/CSSCalcSymbolTable.h` | BSD-2-Clause |
| `CSS/Calc/Tree` | .cpp, .hpp | `css/calc/CSSCalcTree.cpp`<br>`css/calc/CSSCalcTree.h` | BSD-2-Clause |
| `CSS/Calc/TreeComputedStyleDependencies` | .cpp, .hpp | `css/calc/CSSCalcTree+ComputedStyleDependencies.cpp`<br>`css/calc/CSSCalcTree+ComputedStyleDependencies.h` | BSD-2-Clause |
| `CSS/Calc/TreeCopy` | .cpp, .hpp | `css/calc/CSSCalcTree+Copy.cpp`<br>`css/calc/CSSCalcTree+Copy.h` | BSD-2-Clause |
| `CSS/Calc/TreeEvaluation` | .cpp, .hpp | `css/calc/CSSCalcTree+Evaluation.cpp`<br>`css/calc/CSSCalcTree+Evaluation.h` | BSD-2-Clause |
| `CSS/Calc/TreeMappings` | .hpp | `css/calc/CSSCalcTree+Mappings.h` | BSD-2-Clause |
| `CSS/Calc/TreeNumericIdentity` | .hpp | `css/calc/CSSCalcTree+NumericIdentity.h` | BSD-2-Clause |
| `CSS/Calc/TreeParser` | .cpp, .hpp | `css/calc/CSSCalcTree+Parser.cpp`<br>`css/calc/CSSCalcTree+Parser.h` | BSD-2-Clause |
| `CSS/Calc/TreeSerialisation` | .cpp, .hpp | `css/calc/CSSCalcTree+Serialization.cpp`<br>`css/calc/CSSCalcTree+Serialization.h` | BSD-2-Clause |
| `CSS/Calc/TreeSimplification` | .cpp, .hpp | `css/calc/CSSCalcTree+Simplification.cpp`<br>`css/calc/CSSCalcTree+Simplification.h` | BSD-2-Clause |
| `CSS/Calc/TreeTraversal` | .hpp | `css/calc/CSSCalcTree+Traversal.h` | BSD-2-Clause |
| `CSS/Calc/Value` | .cpp, .hpp | `css/calc/CSSCalcValue.cpp`<br>`css/calc/CSSCalcValue.h` | BSD-3-Clause |
| `CSS/Calc/ValueEvaluation` | .cpp | `css/calc/CSSCalcValue.cpp` | BSD-3-Clause |
| `CSS/Calc/ValueParsing` | .cpp | `css/calc/CSSCalcValue.cpp` | BSD-3-Clause |
| `CSS/Counter/CounterStyle` | .cpp, .hpp | `css/CSSCounterStyle.cpp`<br>`css/CSSCounterStyle.h` | BSD-2-Clause |
| `CSS/Counter/CounterStyleDescriptors` | .cpp, .hpp | `css/CSSCounterStyleDescriptors.cpp`<br>`css/CSSCounterStyleDescriptors.h` | BSD-2-Clause |
| `CSS/Counter/CounterStyleRegistry` | .cpp, .hpp | `css/CSSCounterStyleRegistry.cpp`<br>`css/CSSCounterStyleRegistry.h` | BSD-2-Clause |
| `CSS/DOMMatrix/DOMMatrix` | .cpp, .hpp | `css/DOMMatrix.cpp`<br>`css/DOMMatrix.h` | BSD-2-Clause |
| `CSS/DOMMatrix/DOMMatrix2DInit` | .hpp | `css/DOMMatrix2DInit.h` | BSD-2-Clause |
| `CSS/DOMMatrix/DOMMatrixInit` | .hpp | `css/DOMMatrixInit.h` | BSD-2-Clause |
| `CSS/DOMMatrix/DOMMatrixReadOnly` | .cpp, .hpp | `css/DOMMatrixReadOnly.cpp`<br>`css/DOMMatrixReadOnly.h` | BSD-2-Clause |
| `CSS/Descriptors/CSSFontFaceDescriptors` | .cpp, .hpp | `css/CSSFontFaceDescriptors.cpp`<br>`css/CSSFontFaceDescriptors.h` | BSD-2-Clause |
| `CSS/Descriptors/CSSFunctionDescriptors` | .cpp, .hpp | `css/CSSFunctionDescriptors.cpp`<br>`css/CSSFunctionDescriptors.h` | BSD-2-Clause |
| `CSS/Descriptors/CSSPositionTryDescriptors` | .cpp, .hpp | `css/CSSPositionTryDescriptors.cpp`<br>`css/CSSPositionTryDescriptors.h` | BSD-2-Clause |
| `CSS/Descriptors/PropertySetCSSDescriptors` | .cpp, .hpp | `css/PropertySetCSSDescriptors.cpp`<br>`css/PropertySetCSSDescriptors.h` | BSD-2-Clause |
| `CSS/Font/CSSFontSelector` | .cpp, .hpp | `css/CSSFontSelector.cpp`<br>`css/CSSFontSelector.h` | BSD-2-Clause |
| `CSS/Font/FontFace` | .cpp, .hpp | `css/CSSFontFace.cpp`<br>`css/CSSFontFace.h` | BSD-2-Clause |
| `CSS/Font/FontFaceSet` | .cpp, .hpp | `css/CSSFontFaceSet.cpp`<br>`css/CSSFontFaceSet.h` | BSD-2-Clause |
| `CSS/Font/FontFaceSource` | .cpp, .hpp | `css/CSSFontFaceSource.cpp`<br>`css/CSSFontFaceSource.h` | BSD-2-Clause |
| `CSS/Font/FontSelectionValueInlines` | .hpp | `css/FontSelectionValueInlines.h` | BSD-2-Clause |
| `CSS/Font/SegmentedFontFace` | .cpp, .hpp | `css/CSSSegmentedFontFace.cpp`<br>`css/CSSSegmentedFontFace.h` | BSD-2-Clause |
| `CSS/Parser/AtRuleId` | .cpp, .hpp | `css/parser/CSSAtRuleID.cpp`<br>`css/parser/CSSAtRuleID.h` | BSD-3-Clause |
| `CSS/Parser/CustomPropertySyntax` | .cpp, .hpp | `css/parser/CSSCustomPropertySyntax.cpp`<br>`css/parser/CSSCustomPropertySyntax.h` | BSD-2-Clause |
| `CSS/Parser/Parser` | .cpp, .hpp | `css/parser/CSSParser.cpp`<br>`css/parser/CSSParser.h` | BSD-3-Clause |
| `CSS/Parser/ParserContext` | .cpp, .hpp | `css/parser/CSSParserContext.cpp`<br>`css/parser/CSSParserContext.h` | BSD-2-Clause |
| `CSS/Parser/ParserEnum` | .hpp | `css/parser/CSSParserEnum.h` | BSD-3-Clause |
| `CSS/Parser/ParserFastPaths` | .cpp, .hpp | `css/parser/CSSParserFastPaths.cpp`<br>`css/parser/CSSParserFastPaths.h` | BSD-3-Clause |
| `CSS/Parser/ParserIdioms` | .hpp | `css/parser/CSSParserIdioms.h` | BSD-3-Clause |
| `CSS/Parser/ParserMode` | .hpp | `css/parser/CSSParserMode.h` | BSD-2-Clause |
| `CSS/Parser/SizesAttributeParser` | .cpp, .hpp | `css/parser/SizesAttributeParser.cpp`<br>`css/parser/SizesAttributeParser.h` | BSD-3-Clause |
| `CSS/Parser/SizesCalcParser` | .cpp, .hpp | `css/parser/SizesCalcParser.cpp`<br>`css/parser/SizesCalcParser.h` | BSD-3-Clause |
| `CSS/Parser/SupportsParser` | .cpp, .hpp | `css/parser/CSSSupportsParser.cpp`<br>`css/parser/CSSSupportsParser.h` | BSD-3-Clause |
| `CSS/Parser/Token` | .cpp, .hpp | `css/parser/CSSParserToken.cpp`<br>`css/parser/CSSParserToken.h` | BSD-3-Clause |
| `CSS/Parser/TokenRange` | .hpp | `css/parser/CSSParserTokenRange.h` | BSD-3-Clause |
| `CSS/Parser/TokenRangeGuard` | .hpp | `css/parser/CSSParserTokenRangeGuard.h` | BSD-2-Clause |
| `CSS/Parser/TokenSerialisationMode` | .hpp | `css/parser/CSSParserToken.h` | BSD-3-Clause |
| `CSS/Parser/Tokenizer` | .cpp, .hpp | `css/parser/CSSTokenizer.cpp`<br>`css/parser/CSSTokenizer.h` | BSD-3-Clause |
| `CSS/Parser/VariableParser` | .cpp, .hpp | `css/parser/CSSVariableParser.cpp`<br>`css/parser/CSSVariableParser.h` | BSD-3-Clause |
| `CSS/Properties/Consumers/Align` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Align.cpp`<br>`css/parser/CSSPropertyParserConsumer+Align.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Anchor` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Anchor.cpp`<br>`css/parser/CSSPropertyParserConsumer+Anchor.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/AngleDefinitions` | .hpp | `css/parser/CSSPropertyParserConsumer+AngleDefinitions.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/AnglePercentageDefinitions` | .hpp | `css/parser/CSSPropertyParserConsumer+AnglePercentageDefinitions.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Animations` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Animations.cpp`<br>`css/parser/CSSPropertyParserConsumer+Animations.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Attr` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Attr.cpp`<br>`css/parser/CSSPropertyParserConsumer+Attr.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Background` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Background.cpp`<br>`css/parser/CSSPropertyParserConsumer+Background.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Box` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Box.cpp`<br>`css/parser/CSSPropertyParserConsumer+Box.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Color` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Color.cpp`<br>`css/parser/CSSPropertyParserConsumer+Color.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/ColorAdjust` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+ColorAdjust.cpp`<br>`css/parser/CSSPropertyParserConsumer+ColorAdjust.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/ColorInlines` | .hpp | `css/parser/CSSPropertyParserConsumer+ColorInlines.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/ColorInterpolationMethod` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+ColorInterpolationMethod.cpp`<br>`css/parser/CSSPropertyParserConsumer+ColorInterpolationMethod.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Content` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Content.cpp`<br>`css/parser/CSSPropertyParserConsumer+Content.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/CounterStyles` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+CounterStyles.cpp`<br>`css/parser/CSSPropertyParserConsumer+CounterStyles.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Display` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Display.cpp`<br>`css/parser/CSSPropertyParserConsumer+Display.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Easing` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Easing.cpp`<br>`css/parser/CSSPropertyParserConsumer+Easing.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Filter` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Filter.cpp`<br>`css/parser/CSSPropertyParserConsumer+Filter.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/FlexDefinitions` | .hpp | `css/parser/CSSPropertyParserConsumer+FlexDefinitions.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Font` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Font.cpp`<br>`css/parser/CSSPropertyParserConsumer+Font.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/FrequencyDefinitions` | .hpp | `css/parser/CSSPropertyParserConsumer+FrequencyDefinitions.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Grid` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Grid.cpp`<br>`css/parser/CSSPropertyParserConsumer+Grid.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Ident` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Ident.cpp`<br>`css/parser/CSSPropertyParserConsumer+Ident.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Image` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Image.cpp`<br>`css/parser/CSSPropertyParserConsumer+Image.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Inline` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Inline.cpp`<br>`css/parser/CSSPropertyParserConsumer+Inline.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/IntegerDefinitions` | .hpp | `css/parser/CSSPropertyParserConsumer+IntegerDefinitions.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/KeywordDefinitions` | .hpp | `css/parser/CSSPropertyParserConsumer+KeywordDefinitions.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/LengthDefinitions` | .hpp | `css/parser/CSSPropertyParserConsumer+LengthDefinitions.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/LengthPercentageDefinitions` | .hpp | `css/parser/CSSPropertyParserConsumer+LengthPercentageDefinitions.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/List` | .hpp | `css/parser/CSSPropertyParserConsumer+List.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Lists` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Lists.cpp`<br>`css/parser/CSSPropertyParserConsumer+Lists.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Masking` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Masking.cpp`<br>`css/parser/CSSPropertyParserConsumer+Masking.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/MetaConsumer` | .hpp | `css/parser/CSSPropertyParserConsumer+MetaConsumer.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/MetaConsumerDefinitions` | .hpp | `css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/MetaResolver` | .hpp | `css/parser/CSSPropertyParserConsumer+MetaResolver.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Motion` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Motion.cpp`<br>`css/parser/CSSPropertyParserConsumer+Motion.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/NumberDefinitions` | .hpp | `css/parser/CSSPropertyParserConsumer+NumberDefinitions.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Percentage` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Percentage.cpp`<br>`css/parser/CSSPropertyParserConsumer+Percentage.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/PercentageDefinitions` | .hpp | `css/parser/CSSPropertyParserConsumer+PercentageDefinitions.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Position` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Position.cpp`<br>`css/parser/CSSPropertyParserConsumer+Position.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/PositionTry` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+PositionTry.cpp`<br>`css/parser/CSSPropertyParserConsumer+PositionTry.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Primitives` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Primitives.cpp`<br>`css/parser/CSSPropertyParserConsumer+Primitives.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Ratio` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Ratio.cpp`<br>`css/parser/CSSPropertyParserConsumer+Ratio.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/ResolutionDefinitions` | .hpp | `css/parser/CSSPropertyParserConsumer+ResolutionDefinitions.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/SVG` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+SVG.cpp`<br>`css/parser/CSSPropertyParserConsumer+SVG.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/ScrollSnap` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+ScrollSnap.cpp`<br>`css/parser/CSSPropertyParserConsumer+ScrollSnap.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Scrollbars` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Scrollbars.cpp`<br>`css/parser/CSSPropertyParserConsumer+Scrollbars.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Shapes` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Shapes.cpp`<br>`css/parser/CSSPropertyParserConsumer+Shapes.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/String` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+String.cpp`<br>`css/parser/CSSPropertyParserConsumer+String.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Symbol` | .cpp | `css/parser/CSSPropertyParserConsumer+Symbol.cpp` | BSD-2-Clause |
| `CSS/Properties/Consumers/SymbolDefinitions` | .hpp | `css/parser/CSSPropertyParserConsumer+SymbolDefinitions.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Syntax` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Syntax.cpp`<br>`css/parser/CSSPropertyParserConsumer+Syntax.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/TextDecoration` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+TextDecoration.cpp`<br>`css/parser/CSSPropertyParserConsumer+TextDecoration.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/TimeDefinitions` | .hpp | `css/parser/CSSPropertyParserConsumer+TimeDefinitions.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Timeline` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Timeline.cpp`<br>`css/parser/CSSPropertyParserConsumer+Timeline.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Transform` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Transform.cpp`<br>`css/parser/CSSPropertyParserConsumer+Transform.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/Transitions` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+Transitions.cpp`<br>`css/parser/CSSPropertyParserConsumer+Transitions.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/UI` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+UI.cpp`<br>`css/parser/CSSPropertyParserConsumer+UI.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/URL` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+URL.cpp`<br>`css/parser/CSSPropertyParserConsumer+URL.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/UnicodeRange` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+UnicodeRange.cpp`<br>`css/parser/CSSPropertyParserConsumer+UnicodeRange.h` | BSD-2-Clause |
| `CSS/Properties/Consumers/WillChange` | .cpp, .hpp | `css/parser/CSSPropertyParserConsumer+WillChange.cpp`<br>`css/parser/CSSPropertyParserConsumer+WillChange.h` | BSD-2-Clause |
| `CSS/Properties/FontVariantLigaturesParser` | .hpp | `css/parser/CSSFontVariantLigaturesParser.h` | BSD-2-Clause |
| `CSS/Properties/FontVariantNumericParser` | .hpp | `css/parser/CSSFontVariantNumericParser.h` | BSD-2-Clause |
| `CSS/Properties/ImmutableStyleProperties` | .cpp, .hpp | `css/ImmutableStyleProperties.cpp`<br>`css/ImmutableStyleProperties.h` | BSD-2-Clause |
| `CSS/Properties/MutableStyleProperties` | .cpp, .hpp | `css/MutableStyleProperties.cpp`<br>`css/MutableStyleProperties.h` | BSD-2-Clause |
| `CSS/Properties/PropertyInitialValues` | .cpp, .hpp | `css/CSSPropertyInitialValues.cpp`<br>`css/CSSPropertyInitialValues.h` | BSD-2-Clause |
| `CSS/Properties/PropertyParser` | .cpp | `css/parser/CSSPropertyParser.cpp` | BSD-3-Clause |
| `CSS/Properties/PropertyParserCustom` | .hpp | `css/parser/CSSPropertyParserCustom.h` | BSD-3-Clause |
| `CSS/Properties/PropertyParserOptions` | .hpp | `css/parser/CSSPropertyParserOptions.h` | BSD-2-Clause |
| `CSS/Properties/PropertyParserResult` | .cpp, .hpp | `css/parser/CSSPropertyParserResult.cpp`<br>`css/parser/CSSPropertyParserResult.h` | BSD-2-Clause |
| `CSS/Properties/PropertyParserState` | .hpp | `css/parser/CSSPropertyParserState.h` | BSD-2-Clause |
| `CSS/Properties/RegisteredCustomProperty` | .hpp | `css/CSSRegisteredCustomProperty.h` | BSD-2-Clause |
| `CSS/Properties/ShorthandSerialiser` | .cpp, .hpp | `css/ShorthandSerializer.cpp`<br>`css/ShorthandSerializer.h` | BSD-2-Clause |
| `CSS/Properties/StylePropertiesInlines` | .hpp | `css/StylePropertiesInlines.h` | BSD-2-Clause |
| `CSS/Queries/ContainerQuery` | .cpp, .hpp | `css/query/ContainerQuery.cpp`<br>`css/query/ContainerQuery.h` | BSD-2-Clause |
| `CSS/Queries/ContainerQueryFeatures` | .cpp, .hpp | `css/query/ContainerQueryFeatures.cpp`<br>`css/query/ContainerQueryFeatures.h` | BSD-2-Clause |
| `CSS/Queries/ContainerQueryParser` | .cpp, .hpp | `css/query/ContainerQueryParser.cpp`<br>`css/query/ContainerQueryParser.h` | BSD-2-Clause |
| `CSS/Queries/GenericMediaQueryEvaluator` | .cpp, .hpp | `css/query/GenericMediaQueryEvaluator.cpp`<br>`css/query/GenericMediaQueryEvaluator.h` | BSD-2-Clause |
| `CSS/Queries/GenericMediaQueryParser` | .cpp, .hpp | `css/query/GenericMediaQueryParser.cpp`<br>`css/query/GenericMediaQueryParser.h` | BSD-2-Clause |
| `CSS/Queries/GenericMediaQuerySerialisation` | .cpp, .hpp | `css/query/GenericMediaQuerySerialization.cpp`<br>`css/query/GenericMediaQuerySerialization.h` | BSD-2-Clause |
| `CSS/Queries/GenericMediaQueryTypes` | .hpp | `css/query/GenericMediaQueryTypes.h` | BSD-2-Clause |
| `CSS/Queries/MediaQuery` | .hpp | `css/query/MediaQuery.h` | BSD-2-Clause |
| `CSS/Queries/MediaQueryEvaluator` | .cpp, .hpp | `css/query/MediaQueryEvaluator.cpp`<br>`css/query/MediaQueryEvaluator.h` | BSD-2-Clause |
| `CSS/Queries/MediaQueryFeatures` | .cpp, .hpp | `css/query/MediaQueryFeatures.cpp`<br>`css/query/MediaQueryFeatures.h` | BSD-2-Clause |
| `CSS/Queries/MediaQueryParser` | .cpp, .hpp | `css/query/MediaQueryParser.cpp`<br>`css/query/MediaQueryParser.h` | BSD-2-Clause |
| `CSS/Queries/MediaQueryParserContext` | .cpp, .hpp | `css/MediaQueryParserContext.cpp`<br>`css/MediaQueryParserContext.h` | BSD-2-Clause |
| `CSS/Rules/CounterStyleRule` | .cpp, .hpp | `css/CSSCounterStyleRule.cpp`<br>`css/CSSCounterStyleRule.h` | BSD-2-Clause |
| `CSS/Rules/FunctionRule` | .cpp, .hpp | `css/StyleRuleFunction.cpp`<br>`css/StyleRuleFunction.h` | BSD-2-Clause |
| `CSS/Rules/KeyframeRule` | .cpp, .hpp | `css/CSSKeyframeRule.cpp`<br>`css/CSSKeyframeRule.h` | BSD-2-Clause |
| `CSS/Rules/KeyframesRule` | .cpp, .hpp | `css/CSSKeyframesRule.cpp`<br>`css/CSSKeyframesRule.h` | BSD-2-Clause |
| `CSS/Rules/PositionTryRule` | .cpp, .hpp | `css/CSSPositionTryRule.cpp`<br>`css/CSSPositionTryRule.h` | BSD-2-Clause |
| `CSS/Rules/RuleType` | .hpp | `css/StyleRuleType.h` | BSD-2-Clause |
| `CSS/Selectors/SelectorCheckerTestFunctions` | .cpp, .hpp | `css/SelectorCheckerTestFunctions.h` | BSD-2-Clause |
| `CSS/Selectors/SelectorList` | .cpp, .hpp | `css/CSSSelectorList.cpp`<br>`css/CSSSelectorList.h` | BSD-2-Clause |
| `CSS/Selectors/SelectorParser` | .cpp, .hpp | `css/parser/CSSSelectorParser.cpp`<br>`css/parser/CSSSelectorParser.h` | BSD-3-Clause |
| `CSS/Selectors/SelectorParserContext` | .cpp, .hpp | `css/parser/CSSSelectorParserContext.cpp`<br>`css/parser/CSSSelectorParserContext.h` | BSD-2-Clause |
| `CSS/Selectors/SelectorPseudoTypeMap` | .hpp | `css/SelectorPseudoTypeMap.h` | BSD-2-Clause |
| `CSS/StyleSheets/CSSStyleSheetObservableArray` | .cpp, .hpp | `css/CSSStyleSheetObservableArray.cpp`<br>`css/CSSStyleSheetObservableArray.h` | BSD-2-Clause |
| `CSS/TypedOM/CSSKeywordValue` | .cpp, .hpp | `css/typedom/CSSKeywordValue.cpp`<br>`css/typedom/CSSKeywordValue.h` | BSD-2-Clause |
| `CSS/TypedOM/CSSNumericFactory` | .hpp | `css/typedom/CSSNumericFactory.h` | BSD-2-Clause |
| `CSS/TypedOM/CSSNumericValue` | .cpp, .hpp | `css/typedom/CSSNumericValue.cpp`<br>`css/typedom/CSSNumericValue.h` | BSD-2-Clause |
| `CSS/TypedOM/CSSStyleValue` | .cpp, .hpp | `css/typedom/CSSStyleValue.cpp`<br>`css/typedom/CSSStyleValue.h` | BSD-2-Clause |
| `CSS/TypedOM/CSSUnitValue` | .cpp, .hpp | `css/typedom/CSSUnitValue.cpp`<br>`css/typedom/CSSUnitValue.h` | BSD-2-Clause |
| `CSS/TypedOM/Numeric/CSSMathClamp` | .cpp, .hpp | `css/typedom/numeric/CSSMathClamp.cpp`<br>`css/typedom/numeric/CSSMathClamp.h` | BSD-2-Clause |
| `CSS/TypedOM/Numeric/CSSMathInvert` | .cpp, .hpp | `css/typedom/numeric/CSSMathInvert.cpp`<br>`css/typedom/numeric/CSSMathInvert.h` | BSD-2-Clause |
| `CSS/TypedOM/Numeric/CSSMathMax` | .cpp, .hpp | `css/typedom/numeric/CSSMathMax.cpp`<br>`css/typedom/numeric/CSSMathMax.h` | BSD-2-Clause |
| `CSS/TypedOM/Numeric/CSSMathMin` | .cpp, .hpp | `css/typedom/numeric/CSSMathMin.cpp`<br>`css/typedom/numeric/CSSMathMin.h` | BSD-2-Clause |
| `CSS/TypedOM/Numeric/CSSMathNegate` | .cpp, .hpp | `css/typedom/numeric/CSSMathNegate.cpp`<br>`css/typedom/numeric/CSSMathNegate.h` | BSD-2-Clause |
| `CSS/TypedOM/Numeric/CSSMathOperator` | .hpp | `css/typedom/numeric/CSSMathOperator.h` | BSD-2-Clause |
| `CSS/TypedOM/Numeric/CSSMathProduct` | .cpp, .hpp | `css/typedom/numeric/CSSMathProduct.cpp`<br>`css/typedom/numeric/CSSMathProduct.h` | BSD-2-Clause |
| `CSS/TypedOM/Numeric/CSSMathSum` | .cpp, .hpp | `css/typedom/numeric/CSSMathSum.cpp`<br>`css/typedom/numeric/CSSMathSum.h` | BSD-2-Clause |
| `CSS/TypedOM/Numeric/CSSMathValue` | .cpp, .hpp | `css/typedom/numeric/CSSMathValue.cpp`<br>`css/typedom/numeric/CSSMathValue.h` | BSD-2-Clause |
| `CSS/TypedOM/Numeric/CSSNumericArray` | .cpp, .hpp | `css/typedom/numeric/CSSNumericArray.cpp`<br>`css/typedom/numeric/CSSNumericArray.h` | BSD-2-Clause |
| `CSS/TypedOM/Numeric/CSSNumericBaseType` | .hpp | `css/typedom/numeric/CSSNumericBaseType.h` | BSD-2-Clause |
| `CSS/TypedOM/Numeric/CSSNumericType` | .cpp, .hpp | `css/typedom/numeric/CSSNumericType.cpp`<br>`css/typedom/numeric/CSSNumericType.h` | BSD-2-Clause |
| `CSS/Values/Aggregates` | .hpp | `css/values/CSSValueAggregates.h` | BSD-2-Clause |
| `CSS/Values/AttrValue` | .cpp, .hpp | `css/CSSAttrValue.cpp`<br>`css/CSSAttrValue.h` | BSD-2-Clause |
| `CSS/Values/BackgroundRepeatValue` | .cpp, .hpp | `css/CSSBackgroundRepeatValue.cpp`<br>`css/CSSBackgroundRepeatValue.h` | BSD-2-Clause |
| `CSS/Values/BasicShapeValue` | .cpp, .hpp | `css/CSSBasicShapeValue.cpp`<br>`css/CSSBasicShapeValue.h` | BSD-2-Clause |
| `CSS/Values/BorderImageSliceValue` | .cpp, .hpp | `css/CSSBorderImageSliceValue.cpp`<br>`css/CSSBorderImageSliceValue.h` | BSD-2-Clause |
| `CSS/Values/BorderImageWidthValue` | .cpp, .hpp | `css/CSSBorderImageWidthValue.cpp`<br>`css/CSSBorderImageWidthValue.h` | BSD-2-Clause |
| `CSS/Values/Borders/BorderRadius` | .cpp, .hpp | `css/values/borders/CSSBorderRadius.cpp`<br>`css/values/borders/CSSBorderRadius.h` | BSD-2-Clause |
| `CSS/Values/Borders/BoxShadow` | .hpp | `css/values/borders/CSSBoxShadow.h` | BSD-2-Clause |
| `CSS/Values/Borders/BoxShadowProperty` | .hpp | `css/values/borders/CSSBoxShadowProperty.h` | BSD-2-Clause |
| `CSS/Values/BoxShadowPropertyValue` | .cpp, .hpp | `css/CSSBoxShadowPropertyValue.cpp`<br>`css/CSSBoxShadowPropertyValue.h` | BSD-2-Clause |
| `CSS/Values/Color/AbsoluteColor` | .hpp | `css/values/color/CSSAbsoluteColor.h` | BSD-2-Clause |
| `CSS/Values/Color/AbsoluteColorResolver` | .hpp | `css/values/color/CSSAbsoluteColorResolver.h` | BSD-2-Clause |
| `CSS/Values/Color/Color` | .cpp, .hpp | `css/values/color/CSSColor.cpp`<br>`css/values/color/CSSColor.h` | BSD-2-Clause |
| `CSS/Values/Color/ColorDescriptors` | .hpp | `css/values/color/CSSColorDescriptors.h` | BSD-2-Clause |
| `CSS/Values/Color/ColorLayers` | .cpp, .hpp | `css/values/color/CSSColorLayers.cpp`<br>`css/values/color/CSSColorLayers.h` | BSD-2-Clause |
| `CSS/Values/Color/ColorLayersResolver` | .cpp, .hpp | `css/values/color/CSSColorLayersResolver.cpp`<br>`css/values/color/CSSColorLayersResolver.h` | BSD-2-Clause |
| `CSS/Values/Color/ColorMix` | .cpp, .hpp | `css/values/color/CSSColorMix.cpp`<br>`css/values/color/CSSColorMix.h` | BSD-2-Clause |
| `CSS/Values/Color/ColorMixResolver` | .cpp, .hpp | `css/values/color/CSSColorMixResolver.cpp`<br>`css/values/color/CSSColorMixResolver.h` | BSD-2-Clause |
| `CSS/Values/Color/ColorType` | .hpp | `css/values/color/CSSColorType.h` | BSD-2-Clause |
| `CSS/Values/Color/ContrastColor` | .cpp, .hpp | `css/values/color/CSSContrastColor.cpp`<br>`css/values/color/CSSContrastColor.h` | BSD-2-Clause |
| `CSS/Values/Color/ContrastColorResolver` | .cpp, .hpp | `css/values/color/CSSContrastColorResolver.cpp`<br>`css/values/color/CSSContrastColorResolver.h` | BSD-2-Clause |
| `CSS/Values/Color/Convert/Normalise` | .hpp | `css/values/color/CSSColorConversion+Normalize.h` | BSD-2-Clause |
| `CSS/Values/Color/Convert/ToColor` | .hpp | `css/values/color/CSSColorConversion+ToColor.h` | BSD-2-Clause |
| `CSS/Values/Color/Convert/ToTypedColor` | .hpp | `css/values/color/CSSColorConversion+ToTypedColor.h` | BSD-2-Clause |
| `CSS/Values/Color/CurrentColor` | .hpp | `style/values/color/StyleCurrentColor.h` | BSD-2-Clause |
| `CSS/Values/Color/DynamicRangeLimit` | .cpp, .hpp | `css/values/color/CSSDynamicRangeLimit.cpp`<br>`css/values/color/CSSDynamicRangeLimit.h` | BSD-2-Clause |
| `CSS/Values/Color/DynamicRangeLimitMix` | .hpp | `css/values/color/CSSDynamicRangeLimitMix.h` | BSD-2-Clause |
| `CSS/Values/Color/HexColor` | .cpp, .hpp | `css/values/color/CSSHexColor.cpp`<br>`css/values/color/CSSHexColor.h` | BSD-2-Clause |
| `CSS/Values/Color/KeywordColor` | .cpp, .hpp | `css/values/color/CSSKeywordColor.cpp`<br>`css/values/color/CSSKeywordColor.h` | BSD-2-Clause |
| `CSS/Values/Color/LightDarkColor` | .cpp, .hpp | `css/values/color/CSSLightDarkColor.cpp`<br>`css/values/color/CSSLightDarkColor.h` | BSD-2-Clause |
| `CSS/Values/Color/PlatformColorResolutionState` | .cpp, .hpp | `css/values/color/CSSPlatformColorResolutionState.cpp`<br>`css/values/color/CSSPlatformColorResolutionState.h` | BSD-2-Clause |
| `CSS/Values/Color/RelativeColor` | .hpp | `css/values/color/CSSRelativeColor.h` | BSD-2-Clause |
| `CSS/Values/Color/RelativeColorResolver` | .hpp | `css/values/color/CSSRelativeColorResolver.h` | BSD-2-Clause |
| `CSS/Values/Color/ResolvedColor` | .cpp, .hpp | `css/values/color/CSSResolvedColor.cpp`<br>`css/values/color/CSSResolvedColor.h` | BSD-2-Clause |
| `CSS/Values/ColorAdjust/ColorScheme` | .cpp, .hpp | `css/values/color-adjust/CSSColorScheme.cpp`<br>`css/values/color-adjust/CSSColorScheme.h` | BSD-2-Clause |
| `CSS/Values/ColorSchemeValue` | .cpp, .hpp | `css/CSSColorSchemeValue.cpp`<br>`css/CSSColorSchemeValue.h` | BSD-2-Clause |
| `CSS/Values/ColorValue` | .cpp, .hpp | `css/CSSColorValue.cpp`<br>`css/CSSColorValue.h` | BSD-2-Clause |
| `CSS/Values/ComputedStyleDependencies` | .cpp | `css/ComputedStyleDependencies.cpp` | BSD-2-Clause |
| `CSS/Values/Concepts` | .hpp | `css/values/CSSValueConcepts.h` | BSD-2-Clause |
| `CSS/Values/CounterValue` | .cpp, .hpp | `css/CSSCounterValue.cpp`<br>`css/CSSCounterValue.h` | BSD-2-Clause |
| `CSS/Values/CrossFadeValue` | .cpp, .hpp | `css/CSSCrossfadeValue.cpp`<br>`css/CSSCrossfadeValue.h` | BSD-2-Clause |
| `CSS/Values/CustomPropertyValue` | .cpp, .hpp | `css/CSSCustomPropertyValue.cpp`<br>`css/CSSCustomPropertyValue.h` | BSD-2-Clause |
| `CSS/Values/DynamicRangeLimitValue` | .cpp, .hpp | `css/CSSDynamicRangeLimitValue.cpp`<br>`css/CSSDynamicRangeLimitValue.h` | BSD-2-Clause |
| `CSS/Values/Easing/CubicBezierEasingFunction` | .hpp | `css/values/easing/CSSCubicBezierEasingFunction.h` | BSD-2-Clause |
| `CSS/Values/Easing/EasingFunction` | .hpp | `css/values/easing/CSSEasingFunction.h` | BSD-2-Clause |
| `CSS/Values/Easing/LinearEasingFunction` | .hpp | `css/values/easing/CSSLinearEasingFunction.h` | BSD-2-Clause |
| `CSS/Values/Easing/SpringEasingFunction` | .hpp | `css/values/easing/CSSSpringEasingFunction.h` | BSD-2-Clause |
| `CSS/Values/Easing/StepsEasingFunction` | .hpp | `css/values/easing/CSSStepsEasingFunction.h` | BSD-2-Clause |
| `CSS/Values/EasingFunctionValue` | .cpp, .hpp | `css/CSSEasingFunctionValue.cpp`<br>`css/CSSEasingFunctionValue.h` | BSD-2-Clause |
| `CSS/Values/FilterEffects/BlurFunction` | .hpp | `css/values/filter-effects/CSSBlurFunction.h` | BSD-2-Clause |
| `CSS/Values/FilterEffects/BrightnessFunction` | .hpp | `css/values/filter-effects/CSSBrightnessFunction.h` | BSD-2-Clause |
| `CSS/Values/FilterEffects/ContrastFunction` | .hpp | `css/values/filter-effects/CSSContrastFunction.h` | BSD-2-Clause |
| `CSS/Values/FilterEffects/DropShadowFunction` | .hpp | `css/values/filter-effects/CSSDropShadowFunction.h` | BSD-2-Clause |
| `CSS/Values/FilterEffects/Filter` | .hpp | `css/values/filter-effects/CSSFilter.h` | BSD-2-Clause |
| `CSS/Values/FilterEffects/FilterFunctionDescriptor` | .hpp | `css/values/filter-effects/CSSFilterFunctionDescriptor.h` | BSD-2-Clause |
| `CSS/Values/FilterEffects/FilterReference` | .hpp | `css/values/filter-effects/CSSFilterReference.h` | BSD-2-Clause |
| `CSS/Values/FilterEffects/GrayscaleFunction` | .hpp | `css/values/filter-effects/CSSGrayscaleFunction.h` | BSD-2-Clause |
| `CSS/Values/FilterEffects/HueRotateFunction` | .hpp | `css/values/filter-effects/CSSHueRotateFunction.h` | BSD-2-Clause |
| `CSS/Values/FilterEffects/InvertFunction` | .hpp | `css/values/filter-effects/CSSInvertFunction.h` | BSD-2-Clause |
| `CSS/Values/FilterEffects/OpacityFunction` | .hpp | `css/values/filter-effects/CSSOpacityFunction.h` | BSD-2-Clause |
| `CSS/Values/FilterEffects/SaturateFunction` | .hpp | `css/values/filter-effects/CSSSaturateFunction.h` | BSD-2-Clause |
| `CSS/Values/FilterEffects/SepiaFunction` | .hpp | `css/values/filter-effects/CSSSepiaFunction.h` | BSD-2-Clause |
| `CSS/Values/FilterImageValue` | .cpp, .hpp | `css/CSSFilterImageValue.cpp`<br>`css/CSSFilterImageValue.h` | BSD-2-Clause |
| `CSS/Values/FilterValue` | .cpp, .hpp | `css/CSSFilterValue.cpp`<br>`css/CSSFilterValue.h` | BSD-2-Clause |
| `CSS/Values/FontFaceSrcValue` | .cpp, .hpp | `css/CSSFontFaceSrcValue.cpp`<br>`css/CSSFontFaceSrcValue.h` | BSD-2-Clause |
| `CSS/Values/FontFeatureValue` | .cpp, .hpp | `css/CSSFontFeatureValue.cpp`<br>`css/CSSFontFeatureValue.h` | BSD-2-Clause |
| `CSS/Values/FontStyleRangeValue` | .cpp, .hpp | `css/CSSFontStyleRangeValue.cpp`<br>`css/CSSFontStyleRangeValue.h` | BSD-2-Clause |
| `CSS/Values/FontStyleWithAngleValue` | .cpp, .hpp | `css/CSSFontStyleWithAngleValue.cpp`<br>`css/CSSFontStyleWithAngleValue.h` | BSD-2-Clause |
| `CSS/Values/FontVariationValue` | .cpp, .hpp | `css/CSSFontVariationValue.cpp`<br>`css/CSSFontVariationValue.h` | BSD-2-Clause |
| `CSS/Values/FunctionValue` | .cpp, .hpp | `css/CSSFunctionValue.cpp`<br>`css/CSSFunctionValue.h` | BSD-2-Clause |
| `CSS/Values/GlobalKeyword` | .hpp | `css/CSSWideKeyword.h` | BSD-3-Clause |
| `CSS/Values/GradientValue` | .cpp, .hpp | `css/CSSGradientValue.cpp`<br>`css/CSSGradientValue.h` | BSD-2-Clause |
| `CSS/Values/Grid/GridNamedAreaMap` | .cpp, .hpp | `css/values/grid/CSSGridNamedAreaMap.cpp`<br>`css/values/grid/CSSGridNamedAreaMap.h` | BSD-2-Clause |
| `CSS/Values/Grid/GridTemplateAreas` | .hpp | `css/values/grid/CSSGridTemplateAreas.h` | BSD-2-Clause |
| `CSS/Values/GridAutoRepeatValue` | .cpp, .hpp | `css/CSSGridAutoRepeatValue.cpp`<br>`css/CSSGridAutoRepeatValue.h` | BSD-3-Clause |
| `CSS/Values/GridIntegerRepeatValue` | .cpp, .hpp | `css/CSSGridIntegerRepeatValue.cpp`<br>`css/CSSGridIntegerRepeatValue.h` | BSD-3-Clause |
| `CSS/Values/GridLineNamesValue` | .cpp, .hpp | `css/CSSGridLineNamesValue.cpp`<br>`css/CSSGridLineNamesValue.h` | BSD-3-Clause |
| `CSS/Values/GridLineValue` | .cpp, .hpp | `css/CSSGridLineValue.cpp`<br>`css/CSSGridLineValue.h` | BSD-2-Clause |
| `CSS/Values/GridTemplateAreasValue` | .cpp, .hpp | `css/CSSGridTemplateAreasValue.cpp`<br>`css/CSSGridTemplateAreasValue.h` | BSD-3-Clause |
| `CSS/Values/ImageSetOptionValue` | .cpp, .hpp | `css/CSSImageSetOptionValue.cpp`<br>`css/CSSImageSetOptionValue.h` | BSD-2-Clause |
| `CSS/Values/ImageSetValue` | .cpp, .hpp | `css/CSSImageSetValue.cpp`<br>`css/CSSImageSetValue.h` | BSD-2-Clause |
| `CSS/Values/Images/Gradient` | .cpp, .hpp | `css/values/images/CSSGradient.cpp`<br>`css/values/images/CSSGradient.h` | BSD-2-Clause |
| `CSS/Values/Motion/RayFunction` | .cpp, .hpp | `css/values/motion/CSSRayFunction.cpp`<br>`css/values/motion/CSSRayFunction.h` | BSD-2-Clause |
| `CSS/Values/OffsetRotateValue` | .cpp, .hpp | `css/CSSOffsetRotateValue.cpp`<br>`css/CSSOffsetRotateValue.h` | BSD-2-Clause |
| `CSS/Values/PaintImageValue` | .cpp, .hpp | `css/CSSPaintImageValue.cpp`<br>`css/CSSPaintImageValue.h` | BSD-2-Clause |
| `CSS/Values/PathValue` | .cpp, .hpp | `css/CSSPathValue.cpp`<br>`css/CSSPathValue.h` | BSD-2-Clause |
| `CSS/Values/PendingSubstitutionValue` | .cpp, .hpp | `css/CSSPendingSubstitutionValue.cpp`<br>`css/CSSPendingSubstitutionValue.h` | BSD-2-Clause / BSD-3-Clause |
| `CSS/Values/PositionValue` | .cpp, .hpp | `css/CSSPositionValue.cpp`<br>`css/CSSPositionValue.h` | BSD-2-Clause |
| `CSS/Values/PrimitiveValueMappings` | .hpp | `css/CSSPrimitiveValueMappings.h` | BSD-2-Clause |
| `CSS/Values/Primitives/Concepts` | .hpp | `css/values/primitives/CSSPrimitiveNumericConcepts.h` | BSD-2-Clause |
| `CSS/Values/Primitives/NumericCategory` | .hpp | `css/values/primitives/CSSPrimitiveNumericCategory.h` | BSD-2-Clause |
| `CSS/Values/Primitives/NumericRaw` | .hpp | `css/values/primitives/CSSPrimitiveNumericRaw.h` | BSD-2-Clause |
| `CSS/Values/Primitives/NumericTypes` | .hpp | `css/values/primitives/CSSPrimitiveNumericTypes.h` | BSD-2-Clause |
| `CSS/Values/Primitives/NumericTypesCanonicalization` | .hpp | `css/values/primitives/CSSPrimitiveNumericTypes+Canonicalization.h` | BSD-2-Clause |
| `CSS/Values/Primitives/NumericTypesComputedStyleDependencies` | .cpp, .hpp | `css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.cpp`<br>`css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.h` | BSD-2-Clause |
| `CSS/Values/Primitives/NumericTypesEvaluateCalc` | .hpp | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` | BSD-2-Clause |
| `CSS/Values/Primitives/NumericTypesSerialisation` | .cpp, .hpp | `css/values/primitives/CSSPrimitiveNumericTypes+Serialization.cpp`<br>`css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h` | BSD-2-Clause |
| `CSS/Values/Primitives/NumericTypesSymbolReplacement` | .cpp, .hpp | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.cpp`<br>`css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` | BSD-2-Clause |
| `CSS/Values/Primitives/NumericTypesValueCreation` | .hpp | `css/values/primitives/CSSPrimitiveNumericTypes+CSSValueCreation.h` | BSD-2-Clause |
| `CSS/Values/Primitives/NumericTypesValueVisitation` | .hpp | `css/values/primitives/CSSPrimitiveNumericTypes+CSSValueVisitation.h` | BSD-2-Clause |
| `CSS/Values/Primitives/Position` | .cpp, .hpp | `css/values/primitives/CSSPosition.cpp`<br>`css/values/primitives/CSSPosition.h` | BSD-2-Clause |
| `CSS/Values/Primitives/PrimitiveData` | .hpp | `css/values/primitives/CSSPrimitiveData.h` | BSD-2-Clause |
| `CSS/Values/Primitives/PrimitiveKeywordList` | .hpp | `css/values/primitives/CSSPrimitiveKeywordList.h` | BSD-2-Clause |
| `CSS/Values/Primitives/PrimitiveNumeric` | .hpp | `css/values/primitives/CSSPrimitiveNumeric.h` | BSD-2-Clause |
| `CSS/Values/Primitives/PrimitiveNumericOrKeyword` | .hpp | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` | BSD-2-Clause |
| `CSS/Values/Primitives/Range` | .hpp | `css/values/primitives/CSSPrimitiveNumericRange.h` | BSD-2-Clause |
| `CSS/Values/Primitives/Ratio` | .cpp, .hpp | `css/values/primitives/CSSRatio.cpp`<br>`css/values/primitives/CSSRatio.h` | BSD-2-Clause |
| `CSS/Values/Primitives/Symbol` | .cpp, .hpp | `css/values/primitives/CSSSymbol.cpp`<br>`css/values/primitives/CSSSymbol.h` | BSD-2-Clause |
| `CSS/Values/Primitives/URL` | .cpp, .hpp | `css/values/primitives/CSSURL.cpp`<br>`css/values/primitives/CSSURL.h` | BSD-2-Clause |
| `CSS/Values/Primitives/URLModifiers` | .cpp, .hpp | `css/values/primitives/CSSURLModifiers.cpp`<br>`css/values/primitives/CSSURLModifiers.h` | BSD-2-Clause |
| `CSS/Values/Primitives/UnevaluatedCalc` | .cpp, .hpp | `css/values/primitives/CSSUnevaluatedCalc.cpp`<br>`css/values/primitives/CSSUnevaluatedCalc.h` | BSD-2-Clause |
| `CSS/Values/Primitives/UnevaluatedCalcEvaluation` | .cpp | `css/values/primitives/CSSUnevaluatedCalc.cpp` | BSD-2-Clause |
| `CSS/Values/Primitives/Units` | .cpp, .hpp | `css/values/primitives/CSSPrimitiveNumericUnits.cpp`<br>`css/values/primitives/CSSPrimitiveNumericUnits.h` | BSD-2-Clause |
| `CSS/Values/QuadValue` | .cpp, .hpp | `css/CSSQuadValue.cpp`<br>`css/CSSQuadValue.h` | BSD-2-Clause |
| `CSS/Values/RatioValue` | .cpp, .hpp | `css/CSSRatioValue.cpp`<br>`css/CSSRatioValue.h` | BSD-2-Clause |
| `CSS/Values/RayValue` | .cpp, .hpp | `css/CSSRayValue.cpp`<br>`css/CSSRayValue.h` | BSD-3-Clause |
| `CSS/Values/RectValue` | .cpp, .hpp | `css/CSSRectValue.cpp`<br>`css/CSSRectValue.h` | BSD-2-Clause |
| `CSS/Values/ReflectValue` | .cpp, .hpp | `css/CSSReflectValue.cpp`<br>`css/CSSReflectValue.h` | BSD-2-Clause |
| `CSS/Values/ScrollValue` | .cpp, .hpp | `css/CSSScrollValue.cpp`<br>`css/CSSScrollValue.h` | BSD-3-Clause |
| `CSS/Values/Shapes/BasicShape` | .hpp | `css/values/shapes/CSSBasicShape.h` | BSD-2-Clause |
| `CSS/Values/Shapes/CircleFunction` | .cpp, .hpp | `css/values/shapes/CSSCircleFunction.cpp`<br>`css/values/shapes/CSSCircleFunction.h` | BSD-2-Clause |
| `CSS/Values/Shapes/EllipseFunction` | .cpp, .hpp | `css/values/shapes/CSSEllipseFunction.cpp`<br>`css/values/shapes/CSSEllipseFunction.h` | BSD-2-Clause |
| `CSS/Values/Shapes/InsetFunction` | .cpp, .hpp | `css/values/shapes/CSSInsetFunction.cpp`<br>`css/values/shapes/CSSInsetFunction.h` | BSD-2-Clause |
| `CSS/Values/Shapes/PathFunction` | .cpp, .hpp | `css/values/shapes/CSSPathFunction.cpp`<br>`css/values/shapes/CSSPathFunction.h` | BSD-2-Clause |
| `CSS/Values/Shapes/PolygonFunction` | .cpp, .hpp | `css/values/shapes/CSSPolygonFunction.cpp`<br>`css/values/shapes/CSSPolygonFunction.h` | BSD-2-Clause |
| `CSS/Values/Shapes/RectFunction` | .cpp, .hpp | `css/values/shapes/CSSRectFunction.cpp`<br>`css/values/shapes/CSSRectFunction.h` | BSD-2-Clause |
| `CSS/Values/Shapes/ShapeFunction` | .cpp, .hpp | `css/values/shapes/CSSShapeFunction.cpp`<br>`css/values/shapes/CSSShapeFunction.h` | BSD-2-Clause |
| `CSS/Values/Shapes/XywhFunction` | .cpp, .hpp | `css/values/shapes/CSSXywhFunction.cpp`<br>`css/values/shapes/CSSXywhFunction.h` | BSD-2-Clause |
| `CSS/Values/SubgridValue` | .cpp, .hpp | `css/CSSSubgridValue.cpp`<br>`css/CSSSubgridValue.h` | BSD-3-Clause |
| `CSS/Values/TextDecoration/TextShadow` | .hpp | `css/values/text-decoration/CSSTextShadow.h` | BSD-2-Clause |
| `CSS/Values/TextDecoration/TextShadowProperty` | .hpp | `css/values/text-decoration/CSSTextShadowProperty.h` | BSD-2-Clause |
| `CSS/Values/TextShadowPropertyValue` | .cpp, .hpp | `css/CSSTextShadowPropertyValue.cpp`<br>`css/CSSTextShadowPropertyValue.h` | BSD-2-Clause |
| `CSS/Values/ToLengthConversionData` | .cpp, .hpp | `css/CSSToLengthConversionData.cpp`<br>`css/CSSToLengthConversionData.h` | BSD-3-Clause |
| `CSS/Values/TransformListValue` | .cpp, .hpp | `css/CSSTransformListValue.cpp`<br>`css/CSSTransformListValue.h` | BSD-3-Clause |
| `CSS/Values/Types` | .cpp, .hpp | `css/values/CSSValueTypes.cpp`<br>`css/values/CSSValueTypes.h` | BSD-2-Clause |
| `CSS/Values/TypesValueCreation` | .cpp | `css/values/CSSValueTypes.cpp` | BSD-2-Clause |
| `CSS/Values/UnicodeRangeValue` | .cpp, .hpp | `css/CSSUnicodeRangeValue.cpp`<br>`css/CSSUnicodeRangeValue.h` | BSD-2-Clause |
| `CSS/Values/UrlValue` | .cpp, .hpp | `css/CSSURLValue.cpp`<br>`css/CSSURLValue.h` | BSD-2-Clause |
| `CSS/Values/Value` | .cpp | `css/CSSValue.cpp` | BSD-2-Clause |
| `CSS/Values/ValuePair` | .cpp, .hpp | `css/CSSValuePair.cpp`<br>`css/CSSValuePair.h` | BSD-2-Clause |
| `CSS/Values/ValuePool` | .cpp, .hpp | `css/CSSValuePool.cpp`<br>`css/CSSValuePool.h` | BSD-2-Clause |
| `CSS/Values/VariableData` | .cpp, .hpp | `css/CSSVariableData.cpp`<br>`css/CSSVariableData.h` | BSD-3-Clause |
| `CSS/Values/VariableReferenceValue` | .cpp, .hpp | `css/CSSVariableReferenceValue.cpp`<br>`css/CSSVariableReferenceValue.h` | BSD-3-Clause |
| `CSS/Values/ViewValue` | .cpp, .hpp | `css/CSSViewValue.cpp`<br>`css/CSSViewValue.h` | BSD-3-Clause |
| `DOM/Algorithms/ElementTextDirection` | .cpp, .hpp | `dom/ElementTextDirection.cpp`<br>`dom/ElementTextDirection.h` | BSD-2-Clause |
| `DOM/Algorithms/FindRevealAlgorithms` | .cpp, .hpp | `dom/FindRevealAlgorithms.cpp`<br>`dom/FindRevealAlgorithms.h` | BSD-2-Clause |
| `DOM/Algorithms/SelectorQuery` | .cpp, .hpp | `dom/SelectorQuery.cpp`<br>`dom/SelectorQuery.h` | BSD-2-Clause |
| `DOM/Collections/CollectionIndexCache` | .hpp | `dom/CollectionIndexCache.h` | BSD-2-Clause |
| `DOM/Collections/DOMTokenList` | .cpp, .hpp | `html/DOMTokenList.cpp`<br>`html/DOMTokenList.h` | BSD-2-Clause |
| `DOM/ContentVisibility/ContentVisibilityDocumentState` | .cpp, .hpp | `dom/ContentVisibilityDocumentState.cpp`<br>`dom/ContentVisibilityDocumentState.h` | BSD-2-Clause |
| `DOM/DataTransfer/DataTransfer` | .cpp | `dom/DataTransfer.cpp` | BSD-2-Clause |
| `DOM/DataTransfer/DataTransferItem` | .cpp, .hpp | `dom/DataTransferItem.cpp`<br>`dom/DataTransferItem.h` | BSD-3-Clause |
| `DOM/DataTransfer/DataTransferItemList` | .cpp, .hpp | `dom/DataTransferItemList.cpp`<br>`dom/DataTransferItemList.h` | BSD-2-Clause / BSD-3-Clause |
| `DOM/Events/BeforeTextInsertedEvent` | .cpp, .hpp | `dom/BeforeTextInsertedEvent.cpp`<br>`dom/BeforeTextInsertedEvent.h` | BSD-2-Clause |
| `DOM/Events/EventSender` | .hpp | `dom/EventSender.h` | BSD-2-Clause |
| `DOM/Events/InputEvent` | .cpp, .hpp | `dom/InputEvent.cpp`<br>`dom/InputEvent.h` | BSD-2-Clause |
| `DOM/Events/PointerEvent` | .cpp, .hpp | `dom/PointerEvent.cpp`<br>`dom/PointerEvent.h` | BSD-2-Clause |
| `DOM/Events/PointerEventTypeNames` | .cpp, .hpp | `dom/PointerEventTypeNames.cpp`<br>`dom/PointerEventTypeNames.h` | BSD-2-Clause |
| `DOM/Events/SimulatedClick` | .cpp, .hpp | `dom/SimulatedClick.cpp`<br>`dom/SimulatedClick.h` | BSD-2-Clause |
| `DOM/Events/TextEvent` | .cpp, .hpp | `dom/TextEvent.cpp`<br>`dom/TextEvent.h` | BSD-2-Clause |
| `DOM/Events/ToggleEventTask` | .cpp, .hpp | `dom/ToggleEventTask.cpp`<br>`dom/ToggleEventTask.h` | BSD-2-Clause |
| `DOM/Events/Touch` | .cpp, .hpp | `dom/Touch.cpp`<br>`dom/Touch.h` | BSD-2-Clause |
| `DOM/Events/TouchEvent` | .cpp, .hpp | `dom/TouchEvent.cpp`<br>`dom/TouchEvent.h` | BSD-2-Clause |
| `DOM/Events/TouchList` | .cpp, .hpp | `dom/TouchList.cpp`<br>`dom/TouchList.h` | BSD-2-Clause |
| `DOM/Geometry/DOMPoint` | .hpp | `dom/DOMPoint.h` | BSD-2-Clause |
| `DOM/Geometry/DOMPointReadOnly` | .cpp, .hpp | `dom/DOMPointReadOnly.cpp`<br>`dom/DOMPointReadOnly.h` | BSD-2-Clause |
| `DOM/Geometry/DOMRect` | .hpp | `dom/DOMRect.h` | BSD-2-Clause |
| `DOM/Geometry/DOMRectList` | .cpp, .hpp | `dom/DOMRectList.cpp`<br>`dom/DOMRectList.h` | BSD-2-Clause |
| `DOM/Geometry/DOMRectReadOnly` | .hpp | `dom/DOMRectReadOnly.h` | BSD-2-Clause |
| `DOM/Highlights/Highlight` | .cpp, .hpp | `Modules/highlight/Highlight.cpp`<br>`Modules/highlight/Highlight.h` | BSD-2-Clause |
| `DOM/Highlights/HighlightRegistry` | .cpp, .hpp | `Modules/highlight/HighlightRegistry.cpp`<br>`Modules/highlight/HighlightRegistry.h` | BSD-2-Clause |
| `DOM/Internals/DocumentFontLoader` | .cpp, .hpp | `dom/DocumentFontLoader.cpp`<br>`dom/DocumentFontLoader.h` | BSD-2-Clause |
| `DOM/Internals/ElementRareData` | .cpp | `dom/ElementRareData.cpp` | BSD-3-Clause |
| `DOM/Internals/IdTargetObserver` | .cpp, .hpp | `dom/IdTargetObserver.cpp`<br>`dom/IdTargetObserver.h` | BSD-2-Clause |
| `DOM/Internals/IdTargetObserverRegistry` | .cpp, .hpp | `dom/IdTargetObserverRegistry.cpp`<br>`dom/IdTargetObserverRegistry.h` | BSD-2-Clause |
| `DOM/Internals/NodeRareData` | .cpp | `dom/NodeRareData.cpp` | BSD-3-Clause |
| `DOM/Internals/PopoverData` | .cpp, .hpp | `dom/PopoverData.cpp`<br>`dom/PopoverData.h` | BSD-2-Clause |
| `DOM/Internals/QualifiedNameCache` | .cpp, .hpp | `dom/QualifiedNameCache.cpp`<br>`dom/QualifiedNameCache.h` | BSD-2-Clause |
| `DOM/Internals/TreeScopeOrderedMap` | .cpp, .hpp | `dom/TreeScopeOrderedMap.cpp`<br>`dom/TreeScopeOrderedMap.h` | BSD-3-Clause |
| `DOM/Internals/UserActionElementSet` | .cpp, .hpp | `dom/UserActionElementSet.cpp`<br>`dom/UserActionElementSet.h` | BSD-3-Clause |
| `DOM/Internals/VisibilityChangeClient` | .hpp | `dom/VisibilityChangeClient.h` | BSD-2-Clause |
| `DOM/Iterators/ComposedTreeAncestorIterator` | .hpp | `dom/ComposedTreeAncestorIterator.h` | BSD-2-Clause |
| `DOM/Iterators/ComposedTreeIterator` | .cpp, .hpp | `dom/ComposedTreeIterator.cpp`<br>`dom/ComposedTreeIterator.h` | BSD-2-Clause |
| `DOM/Iterators/ElementAndTextDescendantIterator` | .hpp | `dom/ElementAndTextDescendantIterator.h` | BSD-2-Clause |
| `DOM/Mixins/DocumentOrShadowRoot` | .cpp, .hpp | `dom/TreeScope.cpp`<br>`dom/TreeScope.h` | BSD-2-Clause |
| `DOM/MutationObservers/ChildListMutationRecord` | .cpp, .hpp | `dom/MutationRecord.cpp`<br>`dom/MutationRecord.h` | BSD-3-Clause |
| `DOM/MutationObservers/EmptyNodeMutationRecord` | .cpp, .hpp | `dom/MutationRecord.cpp`<br>`dom/MutationRecord.h` | BSD-3-Clause |
| `DOM/Nodes/FocusOptions` | .hpp | `dom/FocusOptions.h` | BSD-2-Clause |
| `DOM/Nodes/PseudoElement` | .cpp, .hpp | `dom/PseudoElement.cpp`<br>`dom/PseudoElement.h` | BSD-3-Clause |
| `DOM/Nodes/ShadowRoot` | .cpp, .hpp | `dom/ShadowRoot.cpp`<br>`dom/ShadowRoot.h` | BSD-3-Clause |
| `DOM/Nodes/TextDirectionState` | .cpp, .hpp | `dom/ElementTextDirection.cpp`<br>`dom/ElementTextDirection.h` | BSD-2-Clause |
| `DOM/Parsing/DocumentParser` | .cpp | `dom/DocumentParser.cpp` | BSD-2-Clause |
| `DOM/Parsing/ParserContentPolicy` | .hpp | `dom/ParserContentPolicy.h` | BSD-2-Clause |
| `DOM/Ranges/BoundaryPoint` | .cpp, .hpp | `dom/BoundaryPoint.cpp`<br>`dom/BoundaryPoint.h` | BSD-2-Clause |
| `DOM/Ranges/Position` | .cpp, .hpp | `dom/Position.cpp`<br>`dom/Position.h` | BSD-2-Clause |
| `DOM/Ranges/PositionIterator` | .cpp, .hpp | `dom/PositionIterator.cpp`<br>`dom/PositionIterator.h` | BSD-2-Clause |
| `DOM/Ranges/SimpleRange` | .cpp, .hpp | `dom/SimpleRange.cpp`<br>`dom/SimpleRange.h` | BSD-2-Clause |
| `DOM/Scheduling/ScriptedAnimationController` | .cpp, .hpp | `dom/ScriptedAnimationController.cpp`<br>`dom/ScriptedAnimationController.h` | BSD-2-Clause |
| `DOM/Types/ExceptionOr` | .hpp | `dom/ExceptionOr.h` | BSD-2-Clause |
| `Editing/CharacterRange` | .hpp | `editing/CharacterRange.h` | BSD-2-Clause |
| `Editing/Commands/AppendNodeCommand` | .cpp, .hpp | `editing/AppendNodeCommand.cpp`<br>`editing/AppendNodeCommand.h` | BSD-2-Clause |
| `Editing/Commands/ApplyStyleCommand` | .cpp, .hpp | `editing/ApplyStyleCommand.cpp`<br>`editing/ApplyStyleCommand.h` | BSD-2-Clause |
| `Editing/Commands/CompositeEditCommand` | .cpp, .hpp | `editing/CompositeEditCommand.cpp`<br>`editing/CompositeEditCommand.h` | BSD-2-Clause |
| `Editing/Commands/DeleteFromTextNodeCommand` | .cpp, .hpp | `editing/DeleteFromTextNodeCommand.cpp`<br>`editing/DeleteFromTextNodeCommand.h` | BSD-2-Clause |
| `Editing/Commands/DeleteSelectionCommand` | .cpp, .hpp | `editing/DeleteSelectionCommand.cpp`<br>`editing/DeleteSelectionCommand.h` | BSD-2-Clause |
| `Editing/Commands/EditCommand` | .cpp, .hpp | `editing/EditCommand.cpp`<br>`editing/EditCommand.h` | BSD-2-Clause |
| `Editing/Commands/InsertIntoTextNodeCommand` | .cpp, .hpp | `editing/InsertIntoTextNodeCommand.cpp`<br>`editing/InsertIntoTextNodeCommand.h` | BSD-2-Clause |
| `Editing/Commands/InsertLineBreakCommand` | .cpp, .hpp | `editing/InsertLineBreakCommand.cpp`<br>`editing/InsertLineBreakCommand.h` | BSD-2-Clause |
| `Editing/Commands/InsertNodeBeforeCommand` | .cpp, .hpp | `editing/InsertNodeBeforeCommand.cpp`<br>`editing/InsertNodeBeforeCommand.h` | BSD-2-Clause |
| `Editing/Commands/InsertParagraphSeparatorCommand` | .cpp, .hpp | `editing/InsertParagraphSeparatorCommand.cpp`<br>`editing/InsertParagraphSeparatorCommand.h` | BSD-2-Clause |
| `Editing/Commands/InsertTextCommand` | .cpp, .hpp | `editing/InsertTextCommand.cpp`<br>`editing/InsertTextCommand.h` | BSD-2-Clause |
| `Editing/Commands/MergeIdenticalElementsCommand` | .cpp, .hpp | `editing/MergeIdenticalElementsCommand.cpp`<br>`editing/MergeIdenticalElementsCommand.h` | BSD-2-Clause |
| `Editing/Commands/MoveSelectionCommand` | .cpp, .hpp | `editing/MoveSelectionCommand.cpp`<br>`editing/MoveSelectionCommand.h` | BSD-2-Clause |
| `Editing/Commands/RemoveNodeCommand` | .cpp, .hpp | `editing/RemoveNodeCommand.cpp`<br>`editing/RemoveNodeCommand.h` | BSD-2-Clause |
| `Editing/Commands/RemoveNodePreservingChildrenCommand` | .cpp, .hpp | `editing/RemoveNodePreservingChildrenCommand.cpp`<br>`editing/RemoveNodePreservingChildrenCommand.h` | BSD-2-Clause |
| `Editing/Commands/ReplaceNodeWithSpanCommand` | .cpp, .hpp | `editing/ReplaceNodeWithSpanCommand.cpp`<br>`editing/ReplaceNodeWithSpanCommand.h` | BSD-3-Clause |
| `Editing/Commands/ReplaceSelectionCommand` | .cpp, .hpp | `editing/ReplaceSelectionCommand.cpp`<br>`editing/ReplaceSelectionCommand.h` | BSD-2-Clause |
| `Editing/Commands/SetNodeAttributeCommand` | .cpp, .hpp | `editing/SetNodeAttributeCommand.cpp`<br>`editing/SetNodeAttributeCommand.h` | BSD-2-Clause |
| `Editing/Commands/SimplifyMarkupCommand` | .cpp, .hpp | `editing/SimplifyMarkupCommand.cpp`<br>`editing/SimplifyMarkupCommand.h` | BSD-2-Clause |
| `Editing/Commands/SplitElementCommand` | .cpp, .hpp | `editing/SplitElementCommand.cpp`<br>`editing/SplitElementCommand.h` | BSD-2-Clause |
| `Editing/Commands/SplitTextNodeCommand` | .cpp, .hpp | `editing/SplitTextNodeCommand.cpp`<br>`editing/SplitTextNodeCommand.h` | BSD-2-Clause |
| `Editing/Commands/SplitTextNodeContainingElementCommand` | .cpp, .hpp | `editing/SplitTextNodeContainingElementCommand.cpp`<br>`editing/SplitTextNodeContainingElementCommand.h` | BSD-2-Clause |
| `Editing/Commands/TextInsertionBaseCommand` | .cpp, .hpp | `editing/TextInsertionBaseCommand.cpp`<br>`editing/TextInsertionBaseCommand.h` | BSD-2-Clause |
| `Editing/Commands/TypingCommand` | .cpp, .hpp | `editing/TypingCommand.cpp`<br>`editing/TypingCommand.h` | BSD-2-Clause |
| `Editing/Commands/UndoStep` | .hpp | `editing/UndoStep.h` | BSD-3-Clause |
| `Editing/Commands/WrapContentsInDummySpanCommand` | .cpp, .hpp | `editing/WrapContentsInDummySpanCommand.cpp`<br>`editing/WrapContentsInDummySpanCommand.h` | BSD-2-Clause |
| `Editing/CompositionHighlight` | .hpp | `editing/CompositionHighlight.h` | BSD-2-Clause |
| `Editing/CompositionUnderline` | .hpp | `editing/CompositionUnderline.h` | BSD-2-Clause |
| `Editing/EditAction` | .cpp, .hpp | `editing/EditAction.cpp`<br>`editing/EditAction.h` | BSD-2-Clause |
| `Editing/Editing` | .cpp, .hpp | `editing/Editing.cpp`<br>`editing/Editing.h` | BSD-2-Clause |
| `Editing/EditingStyle` | .cpp, .hpp | `editing/EditingStyle.cpp`<br>`editing/EditingStyle.h` | BSD-2-Clause / BSD-3-Clause |
| `Editing/Editor` | .cpp, .hpp | `editing/Editor.cpp`<br>`editing/Editor.h` | BSD-2-Clause |
| `Editing/EditorCommand` | .cpp | `editing/EditorCommand.cpp` | BSD-2-Clause |
| `Editing/Markup/HTMLInterchange` | .cpp, .hpp | `editing/HTMLInterchange.cpp`<br>`editing/HTMLInterchange.h` | BSD-2-Clause |
| `Editing/Markup/Markup` | .cpp, .hpp | `editing/markup.cpp`<br>`editing/markup.h` | BSD-2-Clause |
| `Editing/Markup/MarkupAccumulator` | .cpp, .hpp | `editing/MarkupAccumulator.cpp`<br>`editing/MarkupAccumulator.h` | BSD-2-Clause |
| `Editing/Markup/PasteboardFileListReader` | .cpp, .hpp | `editing/WebCorePasteboardFileReader.cpp`<br>`editing/WebCorePasteboardFileReader.h` | BSD-2-Clause |
| `Editing/Markup/WebContentReader` | .cpp, .hpp | `editing/WebContentReader.cpp`<br>`editing/WebContentReader.h` | BSD-2-Clause |
| `Editing/Selection/EditingBoundary` | .hpp | `editing/EditingBoundary.h` | BSD-2-Clause |
| `Editing/Selection/FrameSelection` | .cpp, .hpp | `editing/FrameSelection.cpp`<br>`editing/FrameSelection.h` | BSD-2-Clause |
| `Editing/Selection/InlineRunAndOffset` | .hpp | `editing/InlineRunAndOffset.h` | BSD-2-Clause |
| `Editing/Selection/RenderedPosition` | .cpp, .hpp | `editing/RenderedPosition.cpp`<br>`editing/RenderedPosition.h` | BSD-3-Clause |
| `Editing/Selection/TemporarySelectionChange` | .cpp, .hpp | `editing/Editor.cpp`<br>`editing/Editor.h` | BSD-2-Clause |
| `Editing/Selection/TextAffinity` | .hpp | `editing/TextAffinity.h` | BSD-2-Clause |
| `Editing/Selection/TextGranularity` | .hpp | `editing/TextGranularity.h` | BSD-2-Clause |
| `Editing/Selection/VisiblePosition` | .cpp, .hpp | `editing/VisiblePosition.cpp`<br>`editing/VisiblePosition.h` | BSD-2-Clause |
| `Editing/Selection/VisibleSelection` | .cpp, .hpp | `editing/VisibleSelection.cpp`<br>`editing/VisibleSelection.h` | BSD-2-Clause |
| `Editing/Selection/VisibleUnits` | .cpp, .hpp | `editing/VisibleUnits.cpp`<br>`editing/VisibleUnits.h` | BSD-2-Clause |
| `Editing/SmartReplace` | .cpp, .hpp | `editing/SmartReplace.cpp`<br>`editing/SmartReplace.h` | BSD-3-Clause |
| `Editing/TextIterator` | .cpp, .hpp | `editing/TextIterator.cpp`<br>`editing/TextIterator.h` | BSD-2-Clause |
| `Editing/TextIteratorBehavior` | .hpp | `editing/TextIteratorBehavior.h` | BSD-2-Clause |
| `Editing/WritingDirection` | .hpp | `editing/WritingDirection.h` | BSD-2-Clause |
| `FileAPI/Blob` | .cpp, .hpp | `fileapi/Blob.cpp`<br>`fileapi/Blob.h` | BSD-3-Clause |
| `FileAPI/File` | .cpp, .hpp | `fileapi/File.cpp`<br>`fileapi/File.h` | BSD-2-Clause |
| `FileAPI/FileList` | .cpp, .hpp | `fileapi/FileList.cpp`<br>`fileapi/FileList.h` | BSD-2-Clause |
| `Graphics/Adwaita/Adwaita` | .cpp, .hpp | `platform/graphics/adwaita/Adwaita.cpp`<br>`platform/graphics/adwaita/Adwaita.h` | BSD-2-Clause |
| `Graphics/Adwaita/ButtonControlAdwaita` | .cpp, .hpp | `platform/graphics/adwaita/ButtonControlAdwaita.cpp`<br>`platform/graphics/adwaita/ButtonControlAdwaita.h` | BSD-2-Clause |
| `Graphics/Adwaita/ControlAdwaita` | .cpp, .hpp | `platform/graphics/adwaita/ControlAdwaita.cpp`<br>`platform/graphics/adwaita/ControlAdwaita.h` | BSD-2-Clause |
| `Graphics/Adwaita/ControlFactoryAdwaita` | .cpp, .hpp | `platform/graphics/adwaita/ControlFactoryAdwaita.cpp`<br>`platform/graphics/adwaita/ControlFactoryAdwaita.h` | BSD-2-Clause |
| `Graphics/Adwaita/InnerSpinButtonAdwaita` | .cpp, .hpp | `platform/graphics/adwaita/InnerSpinButtonAdwaita.cpp`<br>`platform/graphics/adwaita/InnerSpinButtonAdwaita.h` | BSD-2-Clause |
| `Graphics/Adwaita/MenuListAdwaita` | .cpp, .hpp | `platform/graphics/adwaita/MenuListAdwaita.cpp`<br>`platform/graphics/adwaita/MenuListAdwaita.h` | BSD-2-Clause |
| `Graphics/Adwaita/ProgressBarAdwaita` | .cpp, .hpp | `platform/graphics/adwaita/ProgressBarAdwaita.cpp`<br>`platform/graphics/adwaita/ProgressBarAdwaita.h` | BSD-2-Clause |
| `Graphics/Adwaita/SliderThumbAdwaita` | .cpp, .hpp | `platform/graphics/adwaita/SliderThumbAdwaita.cpp`<br>`platform/graphics/adwaita/SliderThumbAdwaita.h` | BSD-2-Clause |
| `Graphics/Adwaita/SliderTrackAdwaita` | .cpp, .hpp | `platform/graphics/adwaita/SliderTrackAdwaita.cpp`<br>`platform/graphics/adwaita/SliderTrackAdwaita.h` | BSD-2-Clause |
| `Graphics/Adwaita/TextFieldAdwaita` | .cpp, .hpp | `platform/graphics/adwaita/TextFieldAdwaita.cpp`<br>`platform/graphics/adwaita/TextFieldAdwaita.h` | BSD-2-Clause |
| `Graphics/Adwaita/ToggleButtonAdwaita` | .cpp, .hpp | `platform/graphics/adwaita/ToggleButtonAdwaita.cpp`<br>`platform/graphics/adwaita/ToggleButtonAdwaita.h` | BSD-2-Clause |
| `Graphics/Animation/AnimationUtilities` | .hpp | `platform/animation/AnimationUtilities.h` | BSD-2-Clause |
| `Graphics/Animation/CompositeOperation` | .hpp | `animation/CompositeOperation.h` | BSD-2-Clause |
| `Graphics/Animation/IterationCompositeOperation` | .hpp | `animation/IterationCompositeOperation.h` | BSD-2-Clause |
| `Graphics/Animation/SpringSolver` | .hpp | `platform/graphics/SpringSolver.h` | BSD-2-Clause |
| `Graphics/Animation/TimingFunction` | .cpp | `platform/animation/TimingFunction.cpp` | BSD-2-Clause |
| `Graphics/Animation/UnitBezier` | .hpp | `platform/graphics/UnitBezier.h` | BSD-2-Clause |
| `Graphics/Color/Color` | .cpp, .hpp | `platform/graphics/Color.cpp`<br>`platform/graphics/Color.h` | BSD-2-Clause |
| `Graphics/Color/ColorBlending` | .cpp, .hpp | `platform/graphics/ColorBlending.cpp`<br>`platform/graphics/ColorBlending.h` | BSD-2-Clause |
| `Graphics/Color/ColorFlags` | .hpp | `platform/graphics/Color.h` | BSD-2-Clause |
| `Graphics/Color/ColorHash` | .hpp | `platform/graphics/ColorHash.h` | BSD-2-Clause |
| `Graphics/Color/ColorInterpolation` | .cpp, .hpp | `platform/graphics/ColorInterpolation.cpp`<br>`platform/graphics/ColorInterpolation.h` | BSD-2-Clause |
| `Graphics/Color/ColorInterpolationMethod` | .cpp, .hpp | `platform/graphics/ColorInterpolationMethod.cpp`<br>`platform/graphics/ColorInterpolationMethod.h` | BSD-2-Clause |
| `Graphics/Color/ColorLuminance` | .cpp, .hpp | `platform/graphics/ColorLuminance.cpp`<br>`platform/graphics/ColorLuminance.h` | BSD-2-Clause |
| `Graphics/Color/ColorSerialisation` | .cpp, .hpp | `platform/graphics/ColorSerialization.cpp`<br>`platform/graphics/ColorSerialization.h` | BSD-2-Clause |
| `Graphics/Color/DestinationColorSpace` | .cpp, .hpp | `platform/graphics/DestinationColorSpace.cpp`<br>`platform/graphics/DestinationColorSpace.h` | BSD-2-Clause |
| `Graphics/Controls/ButtonPart` | .hpp | `platform/graphics/controls/ButtonPart.h` | BSD-2-Clause |
| `Graphics/Controls/ColorWellPart` | .hpp | `platform/graphics/controls/ColorWellPart.h` | BSD-2-Clause |
| `Graphics/Controls/ControlFactory` | .cpp, .hpp | `platform/graphics/controls/ControlFactory.cpp`<br>`platform/graphics/controls/ControlFactory.h` | BSD-2-Clause |
| `Graphics/Controls/ControlPart` | .cpp, .hpp | `platform/graphics/controls/ControlPart.cpp`<br>`platform/graphics/controls/ControlPart.h` | BSD-2-Clause |
| `Graphics/Controls/ControlStyle` | .cpp, .hpp | `platform/graphics/controls/ControlStyle.cpp`<br>`platform/graphics/controls/ControlStyle.h` | BSD-2-Clause |
| `Graphics/Controls/InnerSpinButtonPart` | .hpp | `platform/graphics/controls/InnerSpinButtonPart.h` | BSD-2-Clause |
| `Graphics/Controls/MenuListButtonPart` | .hpp | `platform/graphics/controls/MenuListButtonPart.h` | BSD-2-Clause |
| `Graphics/Controls/MenuListPart` | .hpp | `platform/graphics/controls/MenuListPart.h` | BSD-2-Clause |
| `Graphics/Controls/MeterPart` | .cpp, .hpp | `platform/graphics/controls/MeterPart.cpp`<br>`platform/graphics/controls/MeterPart.h` | BSD-2-Clause |
| `Graphics/Controls/PlatformControl` | .hpp | `platform/graphics/controls/PlatformControl.h` | BSD-2-Clause |
| `Graphics/Controls/ProgressBarPart` | .cpp, .hpp | `platform/graphics/controls/ProgressBarPart.cpp`<br>`platform/graphics/controls/ProgressBarPart.h` | BSD-2-Clause |
| `Graphics/Controls/SearchFieldCancelButtonPart` | .hpp | `platform/graphics/controls/SearchFieldCancelButtonPart.h` | BSD-2-Clause |
| `Graphics/Controls/SearchFieldPart` | .hpp | `platform/graphics/controls/SearchFieldPart.h` | BSD-2-Clause |
| `Graphics/Controls/SliderThumbPart` | .hpp | `platform/graphics/controls/SliderThumbPart.h` | BSD-2-Clause |
| `Graphics/Controls/SliderTrackPart` | .cpp, .hpp | `platform/graphics/controls/SliderTrackPart.cpp`<br>`platform/graphics/controls/SliderTrackPart.h` | BSD-2-Clause |
| `Graphics/Controls/TextAreaPart` | .hpp | `platform/graphics/controls/TextAreaPart.h` | BSD-2-Clause |
| `Graphics/Controls/TextFieldPart` | .hpp | `platform/graphics/controls/TextFieldPart.h` | BSD-2-Clause |
| `Graphics/Controls/ToggleButtonPart` | .hpp | `platform/graphics/controls/ToggleButtonPart.h` | BSD-2-Clause |
| `Graphics/DisplayLists/DisplayList` | .cpp, .hpp | `platform/graphics/displaylists/DisplayList.cpp`<br>`platform/graphics/displaylists/DisplayList.h` | BSD-2-Clause |
| `Graphics/DisplayLists/DisplayListItem` | .cpp, .hpp | `platform/graphics/displaylists/DisplayListItem.cpp`<br>`platform/graphics/displaylists/DisplayListItem.h` | BSD-2-Clause |
| `Graphics/DisplayLists/DisplayListItems` | .cpp, .hpp | `platform/graphics/displaylists/DisplayListItems.cpp`<br>`platform/graphics/displaylists/DisplayListItems.h` | BSD-2-Clause |
| `Graphics/DisplayLists/DisplayListRecorder` | .cpp, .hpp | `platform/graphics/displaylists/DisplayListRecorder.cpp`<br>`platform/graphics/displaylists/DisplayListRecorder.h` | BSD-2-Clause |
| `Graphics/DisplayLists/DisplayListRecorderImpl` | .cpp, .hpp | `platform/graphics/displaylists/DisplayListRecorderImpl.cpp`<br>`platform/graphics/displaylists/DisplayListRecorderImpl.h` | BSD-2-Clause |
| `Graphics/Filters/DistantLightSource` | .cpp | `platform/graphics/filters/DistantLightSource.cpp` | BSD-2-Clause |
| `Graphics/Filters/FELighting` | .cpp, .hpp | `platform/graphics/filters/FELighting.cpp`<br>`platform/graphics/filters/FELighting.h` | BSD-2-Clause |
| `Graphics/Filters/Filter` | .cpp | `platform/graphics/filters/Filter.cpp` | BSD-2-Clause |
| `Graphics/Filters/FilterEffectApplier` | .hpp | `platform/graphics/filters/FilterEffectApplier.h` | BSD-2-Clause |
| `Graphics/Filters/FilterEffectGeometry` | .hpp | `platform/graphics/filters/FilterEffectGeometry.h` | BSD-2-Clause |
| `Graphics/Filters/FilterFunction` | .cpp, .hpp | `platform/graphics/filters/FilterFunction.cpp`<br>`platform/graphics/filters/FilterFunction.h` | BSD-2-Clause |
| `Graphics/Filters/FilterImage` | .cpp, .hpp | `platform/graphics/filters/FilterImage.cpp`<br>`platform/graphics/filters/FilterImage.h` | BSD-2-Clause |
| `Graphics/Filters/FilterOperation` | .cpp, .hpp | `platform/graphics/filters/FilterOperation.cpp`<br>`platform/graphics/filters/FilterOperation.h` | BSD-2-Clause |
| `Graphics/Filters/FilterOperations` | .cpp, .hpp | `platform/graphics/filters/FilterOperations.cpp`<br>`platform/graphics/filters/FilterOperations.h` | BSD-2-Clause |
| `Graphics/Filters/FilterRenderingMode` | .hpp | `platform/graphics/filters/FilterRenderingMode.h` | BSD-2-Clause |
| `Graphics/Filters/FilterResults` | .cpp, .hpp | `platform/graphics/filters/FilterResults.cpp`<br>`platform/graphics/filters/FilterResults.h` | BSD-2-Clause |
| `Graphics/Filters/FilterStyle` | .hpp | `platform/graphics/filters/FilterStyle.h` | BSD-2-Clause |
| `Graphics/Filters/PointLightSource` | .cpp | `platform/graphics/filters/PointLightSource.cpp` | BSD-2-Clause |
| `Graphics/Filters/SpotLightSource` | .cpp | `platform/graphics/filters/SpotLightSource.cpp` | BSD-2-Clause |
| `Graphics/Fonts/Font` | .cpp | `platform/graphics/Font.cpp` | BSD-3-Clause |
| `Graphics/Fonts/FontBaseline` | .hpp | `platform/graphics/FontBaseline.h` | BSD-2-Clause |
| `Graphics/Fonts/FontCache` | .cpp, .hpp | `platform/graphics/FontCache.cpp`<br>`platform/graphics/FontCache.h` | BSD-3-Clause |
| `Graphics/Fonts/FontCascadeCache` | .cpp, .hpp | `platform/graphics/FontCascadeCache.cpp`<br>`platform/graphics/FontCascadeCache.h` | BSD-3-Clause |
| `Graphics/Fonts/FontCascadeDescription` | .cpp | `platform/graphics/FontCascadeDescription.cpp` | BSD-3-Clause |
| `Graphics/Fonts/FontCascadeFonts` | .cpp | `platform/graphics/FontCascadeFonts.cpp` | BSD-3-Clause |
| `Graphics/Fonts/FontCreationContext` | .hpp | `platform/graphics/FontCreationContext.h` | BSD-2-Clause |
| `Graphics/Fonts/FontCustomPlatformData` | .cpp, .hpp | `platform/graphics/skia/FontCustomPlatformDataSkia.cpp`<br>`platform/graphics/FontCustomPlatformData.h` | BSD-2-Clause |
| `Graphics/Fonts/FontDescription` | .cpp | `platform/graphics/FontDescription.cpp` | BSD-3-Clause |
| `Graphics/Fonts/FontFeatureValues` | .cpp, .hpp | `platform/graphics/FontFeatureValues.cpp`<br>`platform/graphics/FontFeatureValues.h` | BSD-2-Clause |
| `Graphics/Fonts/FontGenericFamilies` | .cpp, .hpp | `platform/graphics/FontGenericFamilies.cpp`<br>`platform/graphics/FontGenericFamilies.h` | BSD-2-Clause |
| `Graphics/Fonts/FontHarfBuzz` | .cpp | `platform/graphics/skia/SkiaHarfBuzzFont.cpp` | BSD-2-Clause |
| `Graphics/Fonts/FontPalette` | .hpp | `platform/graphics/FontPalette.h` | BSD-2-Clause |
| `Graphics/Fonts/FontPaletteValues` | .hpp | `platform/graphics/FontPaletteValues.h` | BSD-2-Clause |
| `Graphics/Fonts/FontRanges` | .cpp, .hpp | `platform/graphics/FontRanges.cpp`<br>`platform/graphics/FontRanges.h` | BSD-2-Clause |
| `Graphics/Fonts/FontSelectionAlgorithm` | .cpp, .hpp | `platform/graphics/FontSelectionAlgorithm.cpp`<br>`platform/graphics/FontSelectionAlgorithm.h` | BSD-2-Clause |
| `Graphics/Fonts/FontSelector` | .hpp | `platform/graphics/FontSelector.h` | BSD-2-Clause |
| `Graphics/Fonts/FontSizeAdjust` | .hpp | `platform/graphics/FontSizeAdjust.h` | BSD-2-Clause |
| `Graphics/Fonts/FontTaggedSettings` | .hpp | `platform/graphics/FontTaggedSettings.h` | BSD-2-Clause |
| `Graphics/Fonts/Glyph` | .hpp | `platform/graphics/Glyph.h` | BSD-3-Clause |
| `Graphics/Fonts/GlyphBuffer` | .hpp | `platform/graphics/GlyphBuffer.h` | BSD-3-Clause |
| `Graphics/Fonts/GlyphBufferMembers` | .hpp | `platform/graphics/GlyphBufferMembers.h` | BSD-3-Clause |
| `Graphics/Fonts/GlyphMetricsMap` | .hpp | `platform/graphics/GlyphMetricsMap.h` | BSD-3-Clause |
| `Graphics/Fonts/GlyphPage` | .hpp | `platform/graphics/GlyphPage.h` | BSD-3-Clause |
| `Graphics/Fonts/OpenTypeMathData` | .cpp, .hpp | `platform/graphics/opentype/OpenTypeMathData.cpp`<br>`platform/graphics/opentype/OpenTypeMathData.h` | BSD-2-Clause |
| `Graphics/Fonts/SystemFallbackFontCache` | .cpp, .hpp | `platform/graphics/SystemFallbackFontCache.cpp`<br>`platform/graphics/SystemFallbackFontCache.h` | BSD-3-Clause |
| `Graphics/Fonts/SystemFonts` | .hpp | `platform/graphics/SystemFontDatabase.h` | BSD-2-Clause |
| `Graphics/Fonts/WOFFFileFormat` | .cpp, .hpp | `platform/graphics/WOFFFileFormat.cpp`<br>`platform/graphics/WOFFFileFormat.h` | BSD-2-Clause |
| `Graphics/Geometry/BoxExtents` | .hpp | `platform/BoxExtents.h` | BSD-2-Clause |
| `Graphics/Geometry/BoxSides` | .hpp | `platform/BoxSides.h` | BSD-3-Clause |
| `Graphics/Geometry/DoublePoint` | .hpp | `platform/graphics/DoublePoint.h` | BSD-2-Clause |
| `Graphics/Geometry/DoubleSize` | .cpp, .hpp | `platform/graphics/DoubleSize.cpp`<br>`platform/graphics/DoubleSize.h` | BSD-2-Clause |
| `Graphics/Geometry/FloatPoint` | .cpp, .hpp | `platform/graphics/FloatPoint.cpp`<br>`platform/graphics/FloatPoint.h` | BSD-2-Clause |
| `Graphics/Geometry/FloatPointGraph` | .cpp, .hpp | `platform/graphics/FloatPointGraph.cpp`<br>`platform/graphics/FloatPointGraph.h` | BSD-2-Clause |
| `Graphics/Geometry/FloatPolygon` | .cpp, .hpp | `platform/graphics/FloatPolygon.cpp`<br>`platform/graphics/FloatPolygon.h` | BSD-2-Clause |
| `Graphics/Geometry/FloatQuad` | .cpp, .hpp | `platform/graphics/FloatQuad.cpp`<br>`platform/graphics/FloatQuad.h` | BSD-3-Clause |
| `Graphics/Geometry/FloatRect` | .cpp, .hpp | `platform/graphics/FloatRect.cpp`<br>`platform/graphics/FloatRect.h` | BSD-2-Clause |
| `Graphics/Geometry/FloatRoundedRect` | .cpp, .hpp | `platform/graphics/FloatRoundedRect.cpp`<br>`platform/graphics/FloatRoundedRect.h` | BSD-2-Clause |
| `Graphics/Geometry/FloatSegment` | .cpp, .hpp | `platform/graphics/FloatSegment.cpp`<br>`platform/graphics/FloatSegment.h` | BSD-2-Clause |
| `Graphics/Geometry/FloatSize` | .cpp, .hpp | `platform/graphics/FloatSize.cpp`<br>`platform/graphics/FloatSize.h` | BSD-2-Clause |
| `Graphics/Geometry/GeometryUtilities` | .cpp, .hpp | `platform/graphics/GeometryUtilities.cpp`<br>`platform/graphics/GeometryUtilities.h` | BSD-2-Clause |
| `Graphics/Geometry/IntPoint` | .cpp, .hpp | `platform/graphics/IntPoint.cpp`<br>`platform/graphics/IntPoint.h` | BSD-2-Clause |
| `Graphics/Geometry/IntRect` | .cpp, .hpp | `platform/graphics/IntRect.cpp`<br>`platform/graphics/IntRect.h` | BSD-2-Clause |
| `Graphics/Geometry/IntRectHash` | .hpp | `platform/graphics/IntRectHash.h` | BSD-2-Clause |
| `Graphics/Geometry/IntSize` | .cpp, .hpp | `platform/graphics/IntSize.cpp`<br>`platform/graphics/IntSize.h` | BSD-2-Clause |
| `Graphics/Geometry/LayoutPoint` | .cpp, .hpp | `platform/graphics/LayoutPoint.cpp`<br>`platform/graphics/LayoutPoint.h` | BSD-2-Clause / BSD-3-Clause |
| `Graphics/Geometry/LayoutRange` | .cpp, .hpp | `platform/graphics/LayoutRange.h` | BSD-2-Clause |
| `Graphics/Geometry/LayoutRect` | .cpp, .hpp | `platform/graphics/LayoutRect.cpp`<br>`platform/graphics/LayoutRect.h` | BSD-3-Clause |
| `Graphics/Geometry/LayoutRoundedRect` | .cpp, .hpp | `platform/graphics/LayoutRoundedRect.cpp`<br>`platform/graphics/LayoutRoundedRect.h` | BSD-2-Clause |
| `Graphics/Geometry/LayoutSize` | .cpp, .hpp | `platform/graphics/LayoutSize.cpp`<br>`platform/graphics/LayoutSize.h` | BSD-2-Clause / BSD-3-Clause |
| `Graphics/Geometry/LayoutUnit` | .cpp, .hpp | `platform/LayoutUnit.cpp`<br>`platform/LayoutUnit.h` | BSD-2-Clause / BSD-3-Clause |
| `Graphics/Geometry/RectCorners` | .hpp | `platform/RectCorners.h` | BSD-2-Clause |
| `Graphics/Geometry/RectEdges` | .hpp | `platform/RectEdges.h` | BSD-2-Clause |
| `Graphics/Geometry/Region` | .cpp, .hpp | `platform/graphics/Region.cpp`<br>`platform/graphics/Region.h` | BSD-2-Clause |
| `Graphics/Gradient` | .cpp, .hpp | `platform/graphics/Gradient.cpp`<br>`platform/graphics/Gradient.h` | BSD-2-Clause |
| `Graphics/GradientColorStop` | .cpp, .hpp | `platform/graphics/GradientColorStop.cpp`<br>`platform/graphics/GradientColorStop.h` | BSD-2-Clause |
| `Graphics/GradientColorStops` | .cpp, .hpp | `platform/graphics/GradientColorStops.cpp`<br>`platform/graphics/GradientColorStops.h` | BSD-2-Clause |
| `Graphics/GraphicsContext` | .cpp, .hpp | `platform/graphics/GraphicsContext.cpp`<br>`platform/graphics/GraphicsContext.h` | BSD-2-Clause |
| `Graphics/GraphicsContextState` | .cpp, .hpp | `platform/graphics/GraphicsContextState.cpp`<br>`platform/graphics/GraphicsContextState.h` | BSD-2-Clause |
| `Graphics/GraphicsContextStateSaver` | .hpp | `platform/graphics/GraphicsContextStateSaver.h` | BSD-2-Clause |
| `Graphics/GraphicsContextSwitcher` | .cpp, .hpp | `platform/graphics/GraphicsContextSwitcher.cpp`<br>`platform/graphics/GraphicsContextSwitcher.h` | BSD-2-Clause |
| `Graphics/GraphicsStyle` | .cpp, .hpp | `platform/graphics/GraphicsStyle.cpp`<br>`platform/graphics/GraphicsStyle.h` | BSD-2-Clause |
| `Graphics/GraphicsTypes` | .cpp, .hpp | `platform/graphics/GraphicsTypes.cpp`<br>`platform/graphics/GraphicsTypes.h` | BSD-2-Clause |
| `Graphics/ImageBuffers/ByteArrayPixelBuffer` | .cpp, .hpp | `platform/graphics/ByteArrayPixelBuffer.cpp`<br>`platform/graphics/ByteArrayPixelBuffer.h` | BSD-2-Clause |
| `Graphics/ImageBuffers/ImageBuffer` | .cpp, .hpp | `platform/graphics/ImageBuffer.cpp`<br>`platform/graphics/ImageBuffer.h` | BSD-2-Clause |
| `Graphics/ImageBuffers/ImageBufferBackend` | .cpp, .hpp | `platform/graphics/ImageBufferBackend.cpp`<br>`platform/graphics/ImageBufferBackend.h` | BSD-2-Clause |
| `Graphics/ImageBuffers/ImageBufferContextSwitcher` | .cpp, .hpp | `platform/graphics/ImageBufferContextSwitcher.cpp`<br>`platform/graphics/ImageBufferContextSwitcher.h` | BSD-2-Clause |
| `Graphics/ImageBuffers/ImageBufferDisplayListBackend` | .cpp, .hpp | `platform/graphics/ImageBufferDisplayListBackend.cpp`<br>`platform/graphics/ImageBufferDisplayListBackend.h` | BSD-2-Clause |
| `Graphics/ImageBuffers/PixelBuffer` | .cpp, .hpp | `platform/graphics/PixelBuffer.cpp`<br>`platform/graphics/PixelBuffer.h` | BSD-2-Clause |
| `Graphics/ImageBuffers/PixelBufferConversion` | .cpp, .hpp | `platform/graphics/PixelBufferConversion.cpp`<br>`platform/graphics/PixelBufferConversion.h` | BSD-2-Clause |
| `Graphics/ImageBuffers/PixelBufferFormat` | .cpp, .hpp | `platform/graphics/PixelBufferFormat.cpp`<br>`platform/graphics/PixelBufferFormat.h` | BSD-2-Clause |
| `Graphics/ImageBuffers/PixelFormat` | .cpp, .hpp | `platform/graphics/PixelFormat.cpp`<br>`platform/graphics/PixelFormat.h` | BSD-2-Clause |
| `Graphics/Images/BitmapImage` | .cpp, .hpp | `platform/graphics/BitmapImage.cpp`<br>`platform/graphics/BitmapImage.h` | BSD-2-Clause |
| `Graphics/Images/BitmapImageDescriptor` | .cpp, .hpp | `platform/graphics/BitmapImageDescriptor.cpp`<br>`platform/graphics/BitmapImageDescriptor.h` | BSD-2-Clause |
| `Graphics/Images/BitmapImageSource` | .cpp, .hpp | `platform/graphics/BitmapImageSource.cpp`<br>`platform/graphics/BitmapImageSource.h` | BSD-2-Clause |
| `Graphics/Images/DecodingOptions` | .hpp | `platform/graphics/DecodingOptions.h` | BSD-2-Clause |
| `Graphics/Images/GeneratedImage` | .cpp, .hpp | `platform/graphics/GeneratedImage.cpp`<br>`platform/graphics/GeneratedImage.h` | BSD-3-Clause / BSD-2-Clause |
| `Graphics/Images/GradientImage` | .cpp, .hpp | `platform/graphics/GradientImage.cpp`<br>`platform/graphics/GradientImage.h` | BSD-2-Clause |
| `Graphics/Images/Image` | .cpp, .hpp | `platform/graphics/Image.cpp`<br>`platform/graphics/Image.h` | BSD-2-Clause |
| `Graphics/Images/ImageBackingStore` | .hpp | `platform/graphics/ImageBackingStore.h` | BSD-2-Clause |
| `Graphics/Images/ImageDecoder` | .cpp, .hpp | `platform/graphics/ImageDecoder.cpp`<br>`platform/graphics/ImageDecoder.h` | BSD-2-Clause |
| `Graphics/Images/ImageFrame` | .cpp, .hpp | `platform/graphics/ImageFrame.cpp`<br>`platform/graphics/ImageFrame.h` | BSD-2-Clause |
| `Graphics/Images/ImageFrameAnimator` | .cpp, .hpp | `platform/graphics/ImageFrameAnimator.cpp`<br>`platform/graphics/ImageFrameAnimator.h` | BSD-2-Clause |
| `Graphics/Images/ImageFrameWorkQueue` | .cpp, .hpp | `platform/graphics/ImageFrameWorkQueue.cpp`<br>`platform/graphics/ImageFrameWorkQueue.h` | BSD-2-Clause |
| `Graphics/Images/ImageObserver` | .hpp | `platform/graphics/ImageObserver.h` | BSD-2-Clause |
| `Graphics/Images/ImageOrientation` | .hpp | `platform/graphics/ImageOrientation.h` | BSD-2-Clause |
| `Graphics/Images/ImagePaintingOptions` | .hpp | `platform/graphics/ImagePaintingOptions.h` | BSD-2-Clause |
| `Graphics/Images/ImageSource` | .cpp, .hpp | `platform/graphics/ImageSource.cpp`<br>`platform/graphics/ImageSource.h` | BSD-2-Clause |
| `Graphics/Images/ImageTypes` | .hpp | `platform/graphics/ImageTypes.h` | BSD-2-Clause |
| `Graphics/Images/NativeImage` | .cpp, .hpp | `platform/graphics/NativeImage.cpp`<br>`platform/graphics/NativeImage.h` | BSD-2-Clause |
| `Graphics/Images/NativeImageSource` | .cpp, .hpp | `platform/graphics/NativeImageSource.cpp`<br>`platform/graphics/NativeImageSource.h` | BSD-2-Clause |
| `Graphics/Images/PlatformImage` | .hpp | `platform/graphics/PlatformImage.h` | BSD-2-Clause |
| `Graphics/Images/SourceImage` | .cpp, .hpp | `platform/graphics/SourceImage.cpp`<br>`platform/graphics/SourceImage.h` | BSD-2-Clause |
| `Graphics/Layers/ContentsFormat` | .cpp, .hpp | `platform/graphics/ContentsFormat.cpp`<br>`platform/graphics/ContentsFormat.h` | BSD-2-Clause |
| `Graphics/Layers/GraphicsLayer` | .cpp, .hpp | `platform/graphics/GraphicsLayer.cpp`<br>`platform/graphics/GraphicsLayer.h` | BSD-2-Clause |
| `Graphics/Layers/GraphicsLayerAnimationValue` | .hpp | `platform/graphics/GraphicsLayerAnimationValue.h` | BSD-2-Clause |
| `Graphics/Layers/GraphicsLayerClient` | .hpp | `platform/graphics/GraphicsLayerClient.h` | BSD-2-Clause |
| `Graphics/Layers/GraphicsLayerContentsDisplayDelegate` | .hpp | `platform/graphics/GraphicsLayerContentsDisplayDelegate.h` | BSD-2-Clause |
| `Graphics/Layers/GraphicsLayerEnums` | .hpp | `platform/graphics/GraphicsLayerEnums.h` | BSD-2-Clause |
| `Graphics/Layers/GraphicsLayerFactory` | .hpp | `platform/graphics/GraphicsLayerFactory.h` | BSD-2-Clause |
| `Graphics/Layers/GraphicsLayerFilterAnimationValue` | .hpp | `platform/graphics/GraphicsLayerFilterAnimationValue.h` | BSD-2-Clause |
| `Graphics/Layers/GraphicsLayerFloatAnimationValue` | .hpp | `platform/graphics/GraphicsLayerFloatAnimationValue.h` | BSD-2-Clause |
| `Graphics/Layers/GraphicsLayerKeyframeValueList` | .cpp, .hpp | `platform/graphics/GraphicsLayerKeyframeValueList.cpp`<br>`platform/graphics/GraphicsLayerKeyframeValueList.h` | BSD-2-Clause |
| `Graphics/Layers/GraphicsLayerTransformAnimationValue` | .hpp | `platform/graphics/GraphicsLayerTransformAnimationValue.h` | BSD-2-Clause |
| `Graphics/Layers/LayerTreeAsTextOptions` | .hpp | `platform/graphics/LayerTreeAsTextOptions.h` | BSD-2-Clause |
| `Graphics/Layers/PlatformLayerIdentifier` | .hpp | `platform/graphics/PlatformLayerIdentifier.h` | BSD-2-Clause |
| `Graphics/NullGraphicsContext` | .hpp | `platform/graphics/NullGraphicsContext.h` | BSD-2-Clause |
| `Graphics/Paths/DashArray` | .hpp | `platform/graphics/DashArray.h` | BSD-2-Clause |
| `Graphics/Paths/Path` | .cpp, .hpp | `platform/graphics/Path.cpp`<br>`platform/graphics/Path.h` | BSD-2-Clause |
| `Graphics/Paths/PathElement` | .hpp | `platform/graphics/PathElement.h` | BSD-2-Clause |
| `Graphics/Paths/PathImpl` | .cpp, .hpp | `platform/graphics/PathImpl.cpp`<br>`platform/graphics/PathImpl.h` | BSD-2-Clause |
| `Graphics/Paths/PathSegment` | .cpp, .hpp | `platform/graphics/PathSegment.cpp`<br>`platform/graphics/PathSegment.h` | BSD-2-Clause |
| `Graphics/Paths/PathSegmentData` | .cpp, .hpp | `platform/graphics/PathSegmentData.cpp`<br>`platform/graphics/PathSegmentData.h` | BSD-2-Clause |
| `Graphics/Paths/PathStream` | .cpp, .hpp | `platform/graphics/PathStream.cpp`<br>`platform/graphics/PathStream.h` | BSD-2-Clause |
| `Graphics/Paths/PathTraversalState` | .hpp | `platform/graphics/PathTraversalState.h` | BSD-2-Clause |
| `Graphics/Paths/RotationDirection` | .cpp, .hpp | `platform/graphics/RotationDirection.cpp`<br>`platform/graphics/RotationDirection.h` | BSD-2-Clause |
| `Graphics/Paths/WindRule` | .hpp | `platform/graphics/WindRule.h` | BSD-2-Clause |
| `Graphics/Pattern` | .cpp, .hpp | `platform/graphics/Pattern.cpp`<br>`platform/graphics/Pattern.h` | BSD-2-Clause |
| `Graphics/PlatformDynamicRangeLimit` | .cpp, .hpp | `platform/graphics/PlatformDynamicRangeLimit.cpp`<br>`platform/graphics/PlatformDynamicRangeLimit.h` | BSD-2-Clause |
| `Graphics/RenderingMode` | .cpp, .hpp | `platform/graphics/RenderingMode.cpp`<br>`platform/graphics/RenderingMode.h` | BSD-2-Clause |
| `Graphics/RenderingResourceIdentifier` | .hpp | `platform/graphics/RenderingResourceIdentifier.h` | BSD-2-Clause |
| `Graphics/SourceBrush` | .cpp, .hpp | `platform/graphics/SourceBrush.cpp`<br>`platform/graphics/SourceBrush.h` | BSD-2-Clause |
| `Graphics/SourceBrushLogicalGradient` | .hpp | `platform/graphics/SourceBrushLogicalGradient.h` | BSD-2-Clause |
| `Graphics/TextRuns/ComplexTextController` | .cpp, .hpp | `platform/graphics/ComplexTextController.cpp`<br>`platform/graphics/ComplexTextController.h` | BSD-2-Clause |
| `Graphics/TextRuns/ComplexTextControllerHarfBuzz` | .cpp | `platform/graphics/harfbuzz/ComplexTextControllerHarfBuzz.cpp` | BSD-2-Clause |
| `Graphics/TextRuns/StringTruncator` | .cpp, .hpp | `platform/graphics/StringTruncator.cpp`<br>`platform/graphics/StringTruncator.h` | BSD-3-Clause |
| `Graphics/TextRuns/TabSize` | .hpp | `platform/graphics/TabSize.h` | BSD-3-Clause |
| `Graphics/TextRuns/TextRunIterator` | .hpp | `platform/graphics/TextRunIterator.h` | BSD-2-Clause |
| `Graphics/TextRuns/WidthCache` | .hpp | `platform/graphics/WidthCache.h` | BSD-2-Clause |
| `Graphics/Transforms/AffineTransform` | .cpp, .hpp | `platform/graphics/transforms/AffineTransform.cpp`<br>`platform/graphics/transforms/AffineTransform.h` | BSD-2-Clause |
| `Graphics/Transforms/IdentityTransformOperation` | .cpp | `platform/graphics/transforms/IdentityTransformOperation.cpp` | BSD-2-Clause |
| `Graphics/Transforms/Matrix3DTransformOperation` | .cpp, .hpp | `platform/graphics/transforms/Matrix3DTransformOperation.cpp`<br>`platform/graphics/transforms/Matrix3DTransformOperation.h` | BSD-2-Clause |
| `Graphics/Transforms/PerspectiveTransformOperation` | .cpp, .hpp | `platform/graphics/transforms/PerspectiveTransformOperation.cpp`<br>`platform/graphics/transforms/PerspectiveTransformOperation.h` | BSD-2-Clause |
| `Graphics/Transforms/Quaternion` | .cpp, .hpp | `platform/graphics/transforms/Quaternion.cpp`<br>`platform/graphics/transforms/Quaternion.h` | BSD-2-Clause |
| `Graphics/Transforms/TransformOperation` | .cpp | `platform/graphics/transforms/TransformOperation.cpp` | BSD-2-Clause |
| `Graphics/Transforms/TransformOperationsSharedPrimitivesPrefix` | .hpp | `platform/graphics/transforms/TransformOperationsSharedPrimitivesPrefix.h` | BSD-2-Clause |
| `Graphics/Transforms/TransformState` | .cpp, .hpp | `platform/graphics/transforms/TransformState.cpp`<br>`platform/graphics/transforms/TransformState.h` | BSD-2-Clause |
| `Graphics/Transforms/TransformationMatrix` | .cpp, .hpp | `platform/graphics/transforms/TransformationMatrix.cpp`<br>`platform/graphics/transforms/TransformationMatrix.h` | BSD-2-Clause |
| `Graphics/TransparencyLayerContextSwitcher` | .cpp, .hpp | `platform/graphics/TransparencyLayerContextSwitcher.cpp`<br>`platform/graphics/TransparencyLayerContextSwitcher.h` | BSD-2-Clause |
| `HTML/Algorithms/FocusAlgorithms` | .cpp, .hpp | `page/FocusController.cpp`<br>`page/FocusController.h` | BSD-2-Clause |
| `HTML/Canvas/CanvasBase` | .cpp, .hpp | `html/CanvasBase.cpp`<br>`html/CanvasBase.h` | BSD-2-Clause |
| `HTML/Canvas/CanvasFilterContextSwitcher` | .cpp, .hpp | `html/canvas/CanvasFilterContextSwitcher.cpp`<br>`html/canvas/CanvasFilterContextSwitcher.h` | BSD-2-Clause |
| `HTML/Canvas/CanvasGradient` | .cpp, .hpp | `html/canvas/CanvasGradient.cpp`<br>`html/canvas/CanvasGradient.h` | BSD-2-Clause |
| `HTML/Canvas/CanvasLayerContextSwitcher` | .cpp, .hpp | `html/canvas/CanvasLayerContextSwitcher.cpp`<br>`html/canvas/CanvasLayerContextSwitcher.h` | BSD-2-Clause |
| `HTML/Canvas/CanvasPath` | .cpp, .hpp | `html/canvas/CanvasPath.cpp`<br>`html/canvas/CanvasPath.h` | BSD-2-Clause |
| `HTML/Canvas/CanvasPattern` | .cpp, .hpp | `html/canvas/CanvasPattern.cpp`<br>`html/canvas/CanvasPattern.h` | BSD-2-Clause |
| `HTML/Canvas/CanvasRenderingContext` | .cpp, .hpp | `html/canvas/CanvasRenderingContext.cpp`<br>`html/canvas/CanvasRenderingContext.h` | BSD-2-Clause |
| `HTML/Canvas/CanvasRenderingContext2D` | .cpp, .hpp | `html/canvas/CanvasRenderingContext2D.cpp`<br>`html/canvas/CanvasRenderingContext2D.h` | BSD-2-Clause |
| `HTML/Canvas/CanvasRenderingContext2DBase` | .cpp, .hpp | `html/canvas/CanvasRenderingContext2DBase.cpp`<br>`html/canvas/CanvasRenderingContext2DBase.h` | BSD-2-Clause |
| `HTML/Canvas/CanvasStyle` | .cpp, .hpp | `html/canvas/CanvasStyle.cpp`<br>`html/canvas/CanvasStyle.h` | BSD-2-Clause |
| `HTML/Canvas/CanvasTextAlign` | .hpp | `html/canvas/CanvasTextAlign.h` | BSD-2-Clause |
| `HTML/Canvas/CanvasTextBaseline` | .hpp | `html/canvas/CanvasTextBaseline.h` | BSD-2-Clause |
| `HTML/Canvas/GPUBasedCanvasRenderingContext` | .cpp, .hpp | `html/canvas/GPUBasedCanvasRenderingContext.cpp`<br>`html/canvas/GPUBasedCanvasRenderingContext.h` | BSD-2-Clause |
| `HTML/Canvas/Path2D` | .cpp, .hpp | `html/canvas/Path2D.cpp`<br>`html/canvas/Path2D.h` | BSD-2-Clause |
| `HTML/Canvas/PredefinedColorSpace` | .cpp, .hpp | `html/canvas/PredefinedColorSpace.cpp`<br>`html/canvas/PredefinedColorSpace.h` | BSD-2-Clause |
| `HTML/Canvas/TextMetrics` | .hpp | `html/TextMetrics.h` | BSD-2-Clause |
| `HTML/CloseWatcher/CloseWatcher` | .cpp, .hpp | `html/closewatcher/CloseWatcher.cpp`<br>`html/closewatcher/CloseWatcher.h` | BSD-2-Clause |
| `HTML/CloseWatcher/CloseWatcherManager` | .cpp, .hpp | `html/closewatcher/CloseWatcherManager.cpp`<br>`html/closewatcher/CloseWatcherManager.h` | BSD-2-Clause |
| `HTML/Collections/HTMLTableRowsCollection` | .cpp, .hpp | `html/HTMLTableRowsCollection.cpp`<br>`html/HTMLTableRowsCollection.h` | BSD-3-Clause |
| `HTML/Collections/RadioNodeList` | .cpp, .hpp | `html/RadioNodeList.cpp`<br>`html/RadioNodeList.h` | BSD-2-Clause |
| `HTML/CustomElement/CustomElementFormValue` | .hpp | `html/CustomElementFormValue.h` | BSD-2-Clause |
| `HTML/CustomElement/CustomElementInterface` | .cpp, .hpp | `bindings/js/JSCustomElementInterface.cpp`<br>`bindings/js/JSCustomElementInterface.h` | BSD-2-Clause |
| `HTML/CustomElement/CustomElementReactionQueue` | .cpp, .hpp | `dom/CustomElementReactionQueue.cpp`<br>`dom/CustomElementReactionQueue.h` | BSD-2-Clause |
| `HTML/CustomElement/CustomElementRegistry` | .cpp, .hpp | `dom/CustomElementRegistry.cpp`<br>`dom/CustomElementRegistry.h` | BSD-2-Clause |
| `HTML/CustomElement/CustomStateSet` | .cpp, .hpp | `dom/CustomStateSet.cpp`<br>`dom/CustomStateSet.h` | BSD-2-Clause |
| `HTML/CustomElement/ElementInternals` | .cpp, .hpp | `dom/ElementInternals.cpp`<br>`dom/ElementInternals.h` | BSD-2-Clause |
| `HTML/CustomElement/FormAssociatedCustomElement` | .cpp, .hpp | `html/FormAssociatedCustomElement.cpp`<br>`html/FormAssociatedCustomElement.h` | BSD-2-Clause |
| `HTML/CustomElement/ValidityStateFlags` | .hpp | `dom/ValidityStateFlags.h` | BSD-2-Clause |
| `HTML/Elements/HTMLCanvasElement` | .cpp, .hpp | `html/HTMLCanvasElement.cpp`<br>`html/HTMLCanvasElement.h` | BSD-2-Clause |
| `HTML/Elements/HTMLDialogElement` | .cpp, .hpp | `html/HTMLDialogElement.cpp`<br>`html/HTMLDialogElement.h` | BSD-2-Clause |
| `HTML/Elements/HTMLMaybeFormAssociatedCustomElement` | .cpp, .hpp | `html/HTMLMaybeFormAssociatedCustomElement.cpp`<br>`html/HTMLMaybeFormAssociatedCustomElement.h` | BSD-2-Clause |
| `HTML/Elements/HTMLMediaElement` | .cpp, .hpp | `html/HTMLMediaElement.cpp`<br>`html/HTMLMediaElement.h` | BSD-2-Clause |
| `HTML/Elements/HTMLOutputElement` | .cpp, .hpp | `html/HTMLOutputElement.cpp`<br>`html/HTMLOutputElement.h` | BSD-3-Clause |
| `HTML/Elements/HTMLPictureElement` | .cpp, .hpp | `html/HTMLPictureElement.cpp`<br>`html/HTMLPictureElement.h` | BSD-2-Clause |
| `HTML/Elements/HTMLSourceElement` | .cpp, .hpp | `html/HTMLSourceElement.cpp`<br>`html/HTMLSourceElement.h` | BSD-2-Clause |
| `HTML/Elements/HTMLVideoElement` | .cpp, .hpp | `html/HTMLVideoElement.cpp`<br>`html/HTMLVideoElement.h` | BSD-2-Clause |
| `HTML/Elements/HTMLWBRElement` | .cpp, .hpp | `html/HTMLWBRElement.cpp`<br>`html/HTMLWBRElement.h` | BSD-2-Clause |
| `HTML/EventLoop/TaskSource` | .hpp | `dom/TaskSource.h` | BSD-2-Clause |
| `HTML/Forms/Autofill` | .cpp, .hpp | `html/Autofill.cpp`<br>`html/Autofill.h` | BSD-2-Clause |
| `HTML/Forms/DOMFormData` | .cpp, .hpp | `html/DOMFormData.cpp`<br>`html/DOMFormData.h` | BSD-3-Clause |
| `HTML/Forms/FormAssociatedElement` | .cpp | `html/FormAssociatedElement.cpp` | BSD-2-Clause |
| `HTML/Forms/TypeAhead` | .hpp | `html/TypeAhead.h` | BSD-2-Clause |
| `HTML/Forms/ValidatedFormListedElement` | .cpp, .hpp | `html/ValidatedFormListedElement.cpp`<br>`html/ValidatedFormListedElement.h` | BSD-2-Clause |
| `HTML/ImageData/ImageData` | .cpp, .hpp | `html/ImageData.cpp`<br>`html/ImageData.h` | BSD-3-Clause |
| `HTML/ImageData/ImageDataArray` | .hpp | `html/ImageDataArray.h` | BSD-3-Clause |
| `HTML/ImageData/ImageDataPixelFormat` | .hpp | `html/ImageDataPixelFormat.h` | BSD-2-Clause |
| `HTML/ImageData/ImageDataSettings` | .hpp | `html/ImageDataSettings.h` | BSD-2-Clause |
| `HTML/Images/LazyLoadImageObserver` | .cpp, .hpp | `html/LazyLoadImageObserver.cpp`<br>`html/LazyLoadImageObserver.h` | BSD-2-Clause |
| `HTML/InputTypes/BaseButtonInputType` | .cpp, .hpp | `html/BaseButtonInputType.cpp`<br>`html/BaseButtonInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/BaseCheckableInputType` | .cpp, .hpp | `html/BaseCheckableInputType.cpp`<br>`html/BaseCheckableInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/BaseClickableWithKeyInputType` | .cpp, .hpp | `html/BaseClickableWithKeyInputType.cpp`<br>`html/BaseClickableWithKeyInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/BaseDateAndTimeInputType` | .cpp, .hpp | `html/BaseDateAndTimeInputType.cpp`<br>`html/BaseDateAndTimeInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/BaseTextInputType` | .hpp | `html/BaseTextInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/ButtonInputType` | .hpp | `html/ButtonInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/CheckboxInputType` | .cpp, .hpp | `html/CheckboxInputType.cpp`<br>`html/CheckboxInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/ColorInputType` | .cpp, .hpp | `html/ColorInputType.cpp`<br>`html/ColorInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/DateInputType` | .cpp, .hpp | `html/DateInputType.cpp`<br>`html/DateInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/DateTimeFieldsState` | .hpp | `html/DateTimeFieldsState.h` | BSD-2-Clause |
| `HTML/InputTypes/DateTimeLocalInputType` | .cpp, .hpp | `html/DateTimeLocalInputType.cpp`<br>`html/DateTimeLocalInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/EmailInputType` | .hpp | `html/EmailInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/FileInputType` | .hpp | `html/FileInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/HiddenInputType` | .cpp, .hpp | `html/HiddenInputType.cpp`<br>`html/HiddenInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/ImageInputType` | .hpp | `html/ImageInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/InputType` | .hpp | `html/InputType.h` | BSD-3-Clause |
| `HTML/InputTypes/MonthInputType` | .cpp, .hpp | `html/MonthInputType.cpp`<br>`html/MonthInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/NumberInputType` | .cpp, .hpp | `html/NumberInputType.cpp`<br>`html/NumberInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/PasswordInputType` | .hpp | `html/PasswordInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/RadioInputType` | .hpp | `html/RadioInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/RangeInputType` | .cpp, .hpp | `html/RangeInputType.cpp`<br>`html/RangeInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/ResetInputType` | .cpp, .hpp | `html/ResetInputType.cpp`<br>`html/ResetInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/SearchInputType` | .cpp, .hpp | `html/SearchInputType.cpp`<br>`html/SearchInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/SubmitInputType` | .cpp, .hpp | `html/SubmitInputType.cpp`<br>`html/SubmitInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/TelephoneInputType` | .hpp | `html/TelephoneInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/TextFieldInputType` | .cpp, .hpp | `html/TextFieldInputType.cpp`<br>`html/TextFieldInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/TextInputType` | .hpp | `html/TextInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/TimeInputType` | .cpp, .hpp | `html/TimeInputType.cpp`<br>`html/TimeInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/URLInputType` | .cpp, .hpp | `html/URLInputType.cpp`<br>`html/URLInputType.h` | BSD-3-Clause |
| `HTML/InputTypes/WeekInputType` | .cpp, .hpp | `html/WeekInputType.cpp`<br>`html/WeekInputType.h` | BSD-3-Clause |
| `HTML/MicroParsers/Dimensions` | .hpp | `html/parser/HTMLParserIdioms.h` | BSD-2-Clause |
| `HTML/Mixins/URLDecomposition` | .cpp, .hpp | `html/URLDecomposition.cpp`<br>`html/URLDecomposition.h` | BSD-2-Clause |
| `HTML/Parser/FormattingElementList` | .cpp, .hpp | `html/parser/HTMLFormattingElementList.cpp`<br>`html/parser/HTMLFormattingElementList.h` | BSD-2-Clause |
| `HTML/Parser/OpenElementStack` | .cpp, .hpp | `html/parser/HTMLElementStack.cpp`<br>`html/parser/HTMLElementStack.h` | BSD-2-Clause |
| `HTML/Parser/ParserIdioms` | .cpp, .hpp | `html/parser/HTMLParserIdioms.cpp`<br>`html/parser/HTMLParserIdioms.h` | BSD-2-Clause |
| `HTML/Parser/SrcsetParser` | .cpp, .hpp | `html/parser/HTMLSrcsetParser.cpp`<br>`html/parser/HTMLSrcsetParser.h` | BSD-3-Clause |
| `HTML/Parser/StackItem` | .hpp | `html/parser/HTMLStackItem.h` | BSD-2-Clause |
| `HTML/Parser/Token` | .hpp | `html/parser/HTMLToken.h` | BSD-2-Clause |
| `HTML/Parser/TokenAtom` | .hpp | `html/parser/AtomHTMLToken.h` | BSD-2-Clause |
| `HTML/Parser/Tokenizer` | .hpp | `html/parser/HTMLTokenizer.h` | BSD-2-Clause |
| `HTML/Parser/TreeBuilder` | .cpp, .hpp | `html/parser/HTMLTreeBuilder.cpp`<br>`html/parser/HTMLTreeBuilder.h` | BSD-2-Clause |
| `HTML/Shadow/DataListButtonElement` | .cpp, .hpp | `html/shadow/DataListButtonElement.cpp`<br>`html/shadow/DataListButtonElement.h` | BSD-2-Clause |
| `HTML/Shadow/DateTimeEditElement` | .cpp, .hpp | `html/shadow/DateTimeEditElement.cpp`<br>`html/shadow/DateTimeEditElement.h` | BSD-2-Clause |
| `HTML/Shadow/DateTimeFieldElement` | .cpp, .hpp | `html/shadow/DateTimeFieldElement.cpp`<br>`html/shadow/DateTimeFieldElement.h` | BSD-2-Clause |
| `HTML/Shadow/DateTimeFieldElements` | .cpp, .hpp | `html/shadow/DateTimeFieldElements.cpp`<br>`html/shadow/DateTimeFieldElements.h` | BSD-2-Clause |
| `HTML/Shadow/DateTimeNumericFieldElement` | .cpp, .hpp | `html/shadow/DateTimeNumericFieldElement.cpp`<br>`html/shadow/DateTimeNumericFieldElement.h` | BSD-2-Clause |
| `HTML/Shadow/DateTimeSymbolicFieldElement` | .cpp, .hpp | `html/shadow/DateTimeSymbolicFieldElement.cpp`<br>`html/shadow/DateTimeSymbolicFieldElement.h` | BSD-2-Clause |
| `HTML/Shadow/ProgressShadowElement` | .cpp, .hpp | `html/shadow/ProgressShadowElement.cpp`<br>`html/shadow/ProgressShadowElement.h` | BSD-3-Clause |
| `HTML/Shadow/SliderThumbElement` | .cpp, .hpp | `html/shadow/SliderThumbElement.cpp`<br>`html/shadow/SliderThumbElement.h` | BSD-3-Clause |
| `HTML/Shadow/SpinButtonElement` | .cpp, .hpp | `html/shadow/SpinButtonElement.cpp`<br>`html/shadow/SpinButtonElement.h` | BSD-2-Clause |
| `HTML/Shadow/TextControlInnerElements` | .cpp, .hpp | `html/shadow/TextControlInnerElements.cpp`<br>`html/shadow/TextControlInnerElements.h` | BSD-2-Clause |
| `Layout/Floats/FloatAvoider` | .cpp, .hpp | `layout/floats/FloatAvoider.cpp`<br>`layout/floats/FloatAvoider.h` | BSD-2-Clause |
| `Layout/Floats/FloatingContext` | .cpp, .hpp | `layout/floats/FloatingContext.cpp`<br>`layout/floats/FloatingContext.h` | BSD-2-Clause |
| `Layout/Floats/PlacedFloats` | .cpp, .hpp | `layout/floats/PlacedFloats.cpp`<br>`layout/floats/PlacedFloats.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Block/BlockFormattingState` | .cpp, .hpp | `layout/formattingContexts/block/BlockFormattingState.cpp`<br>`layout/formattingContexts/block/BlockFormattingState.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Block/BlockLayoutState` | .hpp | `layout/formattingContexts/block/BlockLayoutState.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Flex/FlexFormattingConstraints` | .hpp | `layout/formattingContexts/flex/FlexFormattingConstraints.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Flex/FlexFormattingContext` | .cpp, .hpp | `layout/formattingContexts/flex/FlexFormattingContext.cpp`<br>`layout/formattingContexts/flex/FlexFormattingContext.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Flex/FlexFormattingUtils` | .cpp, .hpp | `layout/formattingContexts/flex/FlexFormattingUtils.cpp`<br>`layout/formattingContexts/flex/FlexFormattingUtils.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Flex/FlexLayout` | .cpp, .hpp | `layout/formattingContexts/flex/FlexLayout.cpp`<br>`layout/formattingContexts/flex/FlexLayout.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Flex/FlexRect` | .hpp | `layout/formattingContexts/flex/FlexRect.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Flex/LogicalFlexItem` | .hpp | `layout/formattingContexts/flex/LogicalFlexItem.h` | BSD-2-Clause |
| `Layout/FormattingContexts/FormattingConstraints` | .hpp | `layout/formattingContexts/FormattingConstraints.h` | BSD-2-Clause |
| `Layout/FormattingContexts/FormattingContext` | .cpp, .hpp | `layout/formattingContexts/FormattingContext.cpp`<br>`layout/formattingContexts/FormattingContext.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Grid/GridFormattingContext` | .cpp, .hpp | `layout/formattingContexts/grid/GridFormattingContext.cpp`<br>`layout/formattingContexts/grid/GridFormattingContext.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Grid/GridLayout` | .cpp, .hpp | `layout/formattingContexts/grid/GridLayout.cpp`<br>`layout/formattingContexts/grid/GridLayout.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Grid/GridLayoutUtils` | .cpp | `layout/formattingContexts/grid/GridLayoutUtils.cpp` | BSD-2-Clause |
| `Layout/FormattingContexts/Grid/GridTypes` | .hpp | `layout/formattingContexts/grid/GridAreaLines.h`<br>`layout/formattingContexts/grid/GridItemRect.h`<br>`layout/formattingContexts/grid/TrackSizingFunctions.h`<br>`layout/formattingContexts/grid/UsedTrackSizes.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Grid/ImplicitGrid` | .cpp | `layout/formattingContexts/grid/ImplicitGrid.cpp` | BSD-2-Clause |
| `Layout/FormattingContexts/Grid/PlacedGridItem` | .cpp, .hpp | `layout/formattingContexts/grid/PlacedGridItem.cpp`<br>`layout/formattingContexts/grid/PlacedGridItem.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Grid/TrackSizingAlgorithm` | .cpp | `layout/formattingContexts/grid/TrackSizingAlgorithm.cpp` | BSD-2-Clause |
| `Layout/FormattingContexts/Grid/UnplacedGridItem` | .cpp, .hpp | `layout/formattingContexts/grid/UnplacedGridItem.cpp`<br>`layout/formattingContexts/grid/UnplacedGridItem.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/AbstractLineBuilder` | .cpp, .hpp | `layout/formattingContexts/inline/AbstractLineBuilder.cpp`<br>`layout/formattingContexts/inline/AbstractLineBuilder.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/AvailableLineWidthOverride` | .hpp | `layout/formattingContexts/inline/AvailableLineWidthOverride.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/Display/InlineDisplayBox` | .cpp, .hpp | `layout/formattingContexts/inline/display/InlineDisplayBox.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/Display/InlineDisplayContent` | .cpp, .hpp | `layout/formattingContexts/inline/display/InlineDisplayContent.cpp`<br>`layout/formattingContexts/inline/display/InlineDisplayContent.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/Display/InlineDisplayContentBuilder` | .cpp, .hpp | `layout/formattingContexts/inline/display/InlineDisplayContentBuilder.cpp`<br>`layout/formattingContexts/inline/display/InlineDisplayContentBuilder.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/Display/InlineDisplayLine` | .hpp | `layout/formattingContexts/inline/display/InlineDisplayLine.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/Display/InlineDisplayLineBuilder` | .cpp, .hpp | `layout/formattingContexts/inline/display/InlineDisplayLineBuilder.cpp`<br>`layout/formattingContexts/inline/display/InlineDisplayLineBuilder.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineContentAligner` | .cpp, .hpp | `layout/formattingContexts/inline/InlineContentAligner.cpp`<br>`layout/formattingContexts/inline/InlineContentAligner.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineContentBreaker` | .cpp, .hpp | `layout/formattingContexts/inline/InlineContentBreaker.cpp`<br>`layout/formattingContexts/inline/InlineContentBreaker.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineContentCache` | .hpp | `layout/formattingContexts/inline/InlineContentCache.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineContentConstrainer` | .cpp, .hpp | `layout/formattingContexts/inline/InlineContentConstrainer.cpp`<br>`layout/formattingContexts/inline/InlineContentConstrainer.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineFormattingConstraints` | .hpp | `layout/formattingContexts/inline/InlineFormattingConstraints.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineFormattingContext` | .cpp, .hpp | `layout/formattingContexts/inline/InlineFormattingContext.cpp`<br>`layout/formattingContexts/inline/InlineFormattingContext.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineFormattingUtils` | .cpp, .hpp | `layout/formattingContexts/inline/InlineFormattingUtils.cpp`<br>`layout/formattingContexts/inline/InlineFormattingUtils.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineItem` | .cpp, .hpp | `layout/formattingContexts/inline/InlineItem.cpp`<br>`layout/formattingContexts/inline/InlineItem.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineItemsBuilder` | .cpp, .hpp | `layout/formattingContexts/inline/InlineItemsBuilder.cpp`<br>`layout/formattingContexts/inline/InlineItemsBuilder.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineLayoutState` | .hpp | `layout/formattingContexts/inline/InlineLayoutState.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineLevelBox` | .cpp, .hpp | `layout/formattingContexts/inline/InlineLevelBox.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineLine` | .cpp, .hpp | `layout/formattingContexts/inline/InlineLine.cpp`<br>`layout/formattingContexts/inline/InlineLine.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineLineBox` | .cpp, .hpp | `layout/formattingContexts/inline/InlineLineBox.cpp`<br>`layout/formattingContexts/inline/InlineLineBox.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineLineBoxBuilder` | .cpp, .hpp | `layout/formattingContexts/inline/InlineLineBoxBuilder.cpp`<br>`layout/formattingContexts/inline/InlineLineBoxBuilder.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineLineBoxVerticalAligner` | .cpp, .hpp | `layout/formattingContexts/inline/InlineLineBoxVerticalAligner.cpp`<br>`layout/formattingContexts/inline/InlineLineBoxVerticalAligner.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineLineBuilder` | .cpp, .hpp | `layout/formattingContexts/inline/InlineLineBuilder.cpp`<br>`layout/formattingContexts/inline/InlineLineBuilder.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineLineTypes` | .hpp | `layout/formattingContexts/inline/InlineLineTypes.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineQuirks` | .cpp, .hpp | `layout/formattingContexts/inline/InlineQuirks.cpp`<br>`layout/formattingContexts/inline/InlineQuirks.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineRect` | .hpp | `layout/formattingContexts/inline/InlineRect.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineSoftLineBreakItem` | .hpp | `layout/formattingContexts/inline/InlineSoftLineBreakItem.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/InlineTextItem` | .cpp, .hpp | `layout/formattingContexts/inline/InlineTextItem.cpp`<br>`layout/formattingContexts/inline/InlineTextItem.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/IntrinsicWidthHandler` | .cpp, .hpp | `layout/formattingContexts/inline/IntrinsicWidthHandler.cpp`<br>`layout/formattingContexts/inline/IntrinsicWidthHandler.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/Invalidation/InlineDamage` | .hpp | `layout/formattingContexts/inline/invalidation/InlineDamage.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/Invalidation/InlineInvalidation` | .cpp, .hpp | `layout/formattingContexts/inline/invalidation/InlineInvalidation.cpp`<br>`layout/formattingContexts/inline/invalidation/InlineInvalidation.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/LineLayoutResult` | .hpp | `layout/formattingContexts/inline/LineLayoutResult.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/RangeBasedLineBuilder` | .cpp, .hpp | `layout/formattingContexts/inline/RangeBasedLineBuilder.cpp`<br>`layout/formattingContexts/inline/RangeBasedLineBuilder.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/Ruby/RubyFormattingContext` | .cpp, .hpp | `layout/formattingContexts/inline/ruby/RubyFormattingContext.cpp`<br>`layout/formattingContexts/inline/ruby/RubyFormattingContext.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/Text/TextBreakingPositionCache` | .cpp, .hpp | `layout/formattingContexts/inline/text/TextBreakingPositionCache.cpp`<br>`layout/formattingContexts/inline/text/TextBreakingPositionCache.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/Text/TextBreakingPositionContext` | .hpp | `layout/formattingContexts/inline/text/TextBreakingPositionContext.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/Text/TextUtil` | .cpp, .hpp | `layout/formattingContexts/inline/text/TextUtil.cpp`<br>`layout/formattingContexts/inline/text/TextUtil.h` | BSD-2-Clause |
| `Layout/FormattingContexts/Inline/TextOnlySimpleLineBuilder` | .cpp, .hpp | `layout/formattingContexts/inline/TextOnlySimpleLineBuilder.cpp`<br>`layout/formattingContexts/inline/TextOnlySimpleLineBuilder.h` | BSD-2-Clause |
| `Layout/FormattingState` | .cpp, .hpp | `layout/FormattingState.cpp`<br>`layout/FormattingState.h` | BSD-2-Clause |
| `Layout/Integration/Flex/LayoutIntegrationFlexLayout` | .cpp, .hpp | `layout/integration/flex/LayoutIntegrationFlexLayout.cpp`<br>`layout/integration/flex/LayoutIntegrationFlexLayout.h` | BSD-2-Clause |
| `Layout/Integration/Grid/LayoutIntegrationGridLayout` | .cpp, .hpp | `layout/integration/grid/LayoutIntegrationGridLayout.cpp`<br>`layout/integration/grid/LayoutIntegrationGridLayout.h` | BSD-2-Clause |
| `Layout/Integration/Inline/InlineIteratorBox` | .cpp, .hpp | `layout/integration/inline/InlineIteratorBox.cpp`<br>`layout/integration/inline/InlineIteratorBox.h` | BSD-2-Clause |
| `Layout/Integration/Inline/InlineIteratorBoxLegacyPath` | .hpp | `layout/integration/inline/InlineIteratorBoxLegacyPath.h` | BSD-2-Clause |
| `Layout/Integration/Inline/InlineIteratorBoxModernPath` | .hpp | `layout/integration/inline/InlineIteratorBoxModernPath.h` | BSD-2-Clause |
| `Layout/Integration/Inline/InlineIteratorInlineBox` | .cpp, .hpp | `layout/integration/inline/InlineIteratorInlineBox.cpp`<br>`layout/integration/inline/InlineIteratorInlineBox.h` | BSD-2-Clause |
| `Layout/Integration/Inline/InlineIteratorLineBox` | .cpp, .hpp | `layout/integration/inline/InlineIteratorLineBox.cpp`<br>`layout/integration/inline/InlineIteratorLineBox.h` | BSD-2-Clause |
| `Layout/Integration/Inline/InlineIteratorLineBoxLegacyPath` | .hpp | `layout/integration/inline/InlineIteratorLineBoxLegacyPath.h` | BSD-2-Clause |
| `Layout/Integration/Inline/InlineIteratorLogicalOrderTraversal` | .cpp, .hpp | `layout/integration/inline/InlineIteratorLogicalOrderTraversal.cpp`<br>`layout/integration/inline/InlineIteratorLogicalOrderTraversal.h` | BSD-2-Clause |
| `Layout/Integration/Inline/InlineIteratorSVGTextBox` | .cpp, .hpp | `layout/integration/inline/InlineIteratorSVGTextBox.cpp`<br>`layout/integration/inline/InlineIteratorSVGTextBox.h` | BSD-2-Clause |
| `Layout/Integration/Inline/InlineIteratorTextBox` | .cpp, .hpp | `layout/integration/inline/InlineIteratorTextBox.cpp`<br>`layout/integration/inline/InlineIteratorTextBox.h` | BSD-2-Clause |
| `Layout/Integration/Inline/LayoutIntegrationInlineContent` | .cpp, .hpp | `layout/integration/inline/LayoutIntegrationInlineContent.cpp`<br>`layout/integration/inline/LayoutIntegrationInlineContent.h` | BSD-2-Clause |
| `Layout/Integration/Inline/LayoutIntegrationInlineContentBuilder` | .cpp, .hpp | `layout/integration/inline/LayoutIntegrationInlineContentBuilder.cpp`<br>`layout/integration/inline/LayoutIntegrationInlineContentBuilder.h` | BSD-2-Clause |
| `Layout/Integration/Inline/LayoutIntegrationInlineContentPainter` | .cpp, .hpp | `layout/integration/inline/LayoutIntegrationInlineContentPainter.cpp`<br>`layout/integration/inline/LayoutIntegrationInlineContentPainter.h` | BSD-2-Clause |
| `Layout/Integration/Inline/LayoutIntegrationLineLayout` | .cpp, .hpp | `layout/integration/inline/LayoutIntegrationLineLayout.cpp`<br>`layout/integration/inline/LayoutIntegrationLineLayout.h` | BSD-2-Clause |
| `Layout/Integration/Inline/LineSelection` | .hpp | `layout/integration/inline/LineSelection.h` | BSD-2-Clause |
| `Layout/Integration/LayoutIntegrationBoxGeometryUpdater` | .cpp, .hpp | `layout/integration/LayoutIntegrationBoxGeometryUpdater.cpp`<br>`layout/integration/LayoutIntegrationBoxGeometryUpdater.h` | BSD-2-Clause |
| `Layout/Integration/LayoutIntegrationBoxTreeUpdater` | .cpp, .hpp | `layout/integration/LayoutIntegrationBoxTreeUpdater.cpp`<br>`layout/integration/LayoutIntegrationBoxTreeUpdater.h` | BSD-2-Clause |
| `Layout/Integration/LayoutIntegrationFormattingContextLayout` | .cpp, .hpp | `layout/integration/LayoutIntegrationFormattingContextLayout.cpp`<br>`layout/integration/LayoutIntegrationFormattingContextLayout.h` | BSD-2-Clause |
| `Layout/Integration/LayoutIntegrationUtils` | .cpp, .hpp | `layout/integration/LayoutIntegrationUtils.cpp`<br>`layout/integration/LayoutIntegrationUtils.h` | BSD-2-Clause |
| `Layout/LayoutState` | .cpp, .hpp | `layout/LayoutState.cpp`<br>`layout/LayoutState.h` | BSD-2-Clause |
| `Layout/LayoutTree/FormattingContextBoxIterator` | .hpp | `layout/layouttree/FormattingContextBoxIterator.h` | BSD-2-Clause |
| `Layout/LayoutTree/LayoutBox` | .cpp, .hpp | `layout/layouttree/LayoutBox.cpp`<br>`layout/layouttree/LayoutBox.h` | BSD-2-Clause |
| `Layout/LayoutTree/LayoutBoxGeometry` | .cpp, .hpp | `layout/layouttree/LayoutBoxGeometry.cpp`<br>`layout/layouttree/LayoutBoxGeometry.h` | BSD-2-Clause |
| `Layout/LayoutTree/LayoutBoxInlines` | .hpp | `layout/layouttree/LayoutBoxInlines.h` | BSD-2-Clause |
| `Layout/LayoutTree/LayoutChildIterator` | .hpp | `layout/layouttree/LayoutChildIterator.h` | BSD-2-Clause |
| `Layout/LayoutTree/LayoutContainingBlockChainIterator` | .hpp | `layout/layouttree/LayoutContainingBlockChainIterator.h` | BSD-2-Clause |
| `Layout/LayoutTree/LayoutDescendantIterator` | .hpp | `layout/layouttree/LayoutDescendantIterator.h` | BSD-2-Clause |
| `Layout/LayoutTree/LayoutElementBox` | .cpp, .hpp | `layout/layouttree/LayoutElementBox.cpp`<br>`layout/layouttree/LayoutElementBox.h` | BSD-2-Clause |
| `Layout/LayoutTree/LayoutGeometryRect` | .hpp | `layout/layouttree/LayoutGeometryRect.h` | BSD-2-Clause |
| `Layout/LayoutTree/LayoutInitialContainingBlock` | .cpp, .hpp | `layout/layouttree/LayoutInitialContainingBlock.cpp`<br>`layout/layouttree/LayoutInitialContainingBlock.h` | BSD-2-Clause |
| `Layout/LayoutTree/LayoutInlineTextBox` | .cpp, .hpp | `layout/layouttree/LayoutInlineTextBox.cpp`<br>`layout/layouttree/LayoutInlineTextBox.h` | BSD-2-Clause |
| `Layout/LayoutTree/LayoutIterator` | .hpp | `layout/layouttree/LayoutIterator.h` | BSD-2-Clause |
| `Layout/LayoutUnits` | .hpp | `layout/LayoutUnits.h` | BSD-2-Clause |
| `Layout/MarginTypes` | .hpp | `layout/MarginTypes.h` | BSD-2-Clause |
| `Loader/Cache/CachePolicy` | .hpp | `loader/cache/CachePolicy.h` | BSD-2-Clause |
| `Loader/Cache/CachedFont` | .cpp, .hpp | `loader/cache/CachedFont.cpp`<br>`loader/cache/CachedFont.h` | BSD-2-Clause |
| `Loader/Cache/CachedFontClient` | .hpp | `loader/cache/CachedFontClient.h` | BSD-2-Clause |
| `Loader/Cache/CachedFontLoadRequest` | .hpp | `loader/cache/CachedFontLoadRequest.h` | BSD-2-Clause |
| `Loader/Cache/CachedRawResource` | .cpp | `loader/cache/CachedRawResource.cpp` | BSD-2-Clause |
| `Loader/Cache/CachedResourceHandle` | .hpp | `loader/cache/CachedResourceHandle.h` | BSD-2-Clause |
| `Loader/Cache/CachedResourceRequest` | .hpp | `loader/cache/CachedResourceRequest.h` | BSD-2-Clause |
| `Loader/DocumentLoader` | .cpp, .hpp | `loader/DocumentLoader.cpp`<br>`loader/DocumentLoader.h` | BSD-3-Clause |
| `Loader/DocumentWriter` | .cpp, .hpp | `loader/DocumentWriter.cpp`<br>`loader/DocumentWriter.h` | BSD-3-Clause |
| `Loader/FetchOptions` | .hpp | `loader/FetchOptions.h` | BSD-3-Clause |
| `Loader/FontLoadRequest` | .hpp | `loader/FontLoadRequest.h` | BSD-2-Clause |
| `Loader/FormSubmission` | .cpp, .hpp | `loader/FormSubmission.cpp`<br>`loader/FormSubmission.h` | BSD-3-Clause |
| `Loader/FrameLoadRequest` | .cpp, .hpp | `loader/FrameLoadRequest.cpp`<br>`loader/FrameLoadRequest.h` | BSD-3-Clause / BSD-2-Clause |
| `Loader/FrameLoader` | .cpp, .hpp | `loader/FrameLoader.cpp`<br>`loader/FrameLoader.h` | BSD-3-Clause |
| `Loader/FrameLoaderTypes` | .hpp | `loader/FrameLoaderTypes.h` | BSD-3-Clause |
| `Loader/Icons/IconLoader` | .cpp, .hpp | `loader/icon/IconLoader.cpp`<br>`loader/icon/IconLoader.h` | BSD-2-Clause |
| `Loader/Icons/LinkIcon` | .hpp | `platform/LinkIcon.h` | BSD-2-Clause |
| `Loader/Icons/LinkIconCollector` | .cpp, .hpp | `html/LinkIconCollector.cpp`<br>`html/LinkIconCollector.h` | BSD-2-Clause |
| `Loader/LoaderStrategy` | .hpp | `loader/LoaderStrategy.h` | BSD-2-Clause |
| `Loader/LocalFrameLoaderClient` | .hpp | `loader/LocalFrameLoaderClient.h` | BSD-3-Clause |
| `Loader/NavigationAction` | .cpp, .hpp | `loader/NavigationAction.cpp`<br>`loader/NavigationAction.h` | BSD-3-Clause |
| `Loader/ResourceLoaderOptions` | .hpp | `loader/ResourceLoaderOptions.h` | BSD-3-Clause |
| `Loader/SubstituteData` | .hpp | `loader/SubstituteData.h` | BSD-2-Clause |
| `MathML/Elements/MathMLElement` | .cpp, .hpp | `mathml/MathMLElement.cpp`<br>`mathml/MathMLElement.h` | BSD-2-Clause |
| `MathML/Elements/MathMLFractionElement` | .cpp, .hpp | `mathml/MathMLFractionElement.cpp`<br>`mathml/MathMLFractionElement.h` | BSD-2-Clause |
| `MathML/Elements/MathMLMathElement` | .cpp, .hpp | `mathml/MathMLMathElement.cpp`<br>`mathml/MathMLMathElement.h` | BSD-2-Clause |
| `MathML/Elements/MathMLMencloseElement` | .cpp, .hpp | `mathml/MathMLMencloseElement.cpp`<br>`mathml/MathMLMencloseElement.h` | BSD-2-Clause |
| `MathML/Elements/MathMLOperatorElement` | .cpp, .hpp | `mathml/MathMLOperatorElement.cpp`<br>`mathml/MathMLOperatorElement.h` | BSD-2-Clause |
| `MathML/Elements/MathMLPaddedElement` | .cpp, .hpp | `mathml/MathMLPaddedElement.cpp`<br>`mathml/MathMLPaddedElement.h` | BSD-2-Clause |
| `MathML/Elements/MathMLPresentationElement` | .cpp, .hpp | `mathml/MathMLPresentationElement.cpp`<br>`mathml/MathMLPresentationElement.h` | BSD-2-Clause |
| `MathML/Elements/MathMLRootElement` | .cpp, .hpp | `mathml/MathMLRootElement.cpp`<br>`mathml/MathMLRootElement.h` | BSD-2-Clause |
| `MathML/Elements/MathMLRowElement` | .cpp, .hpp | `mathml/MathMLRowElement.cpp`<br>`mathml/MathMLRowElement.h` | BSD-2-Clause |
| `MathML/Elements/MathMLScriptsElement` | .cpp, .hpp | `mathml/MathMLScriptsElement.cpp`<br>`mathml/MathMLScriptsElement.h` | BSD-2-Clause |
| `MathML/Elements/MathMLSpaceElement` | .cpp, .hpp | `mathml/MathMLSpaceElement.cpp`<br>`mathml/MathMLSpaceElement.h` | BSD-2-Clause |
| `MathML/Elements/MathMLTokenElement` | .cpp, .hpp | `mathml/MathMLTokenElement.cpp`<br>`mathml/MathMLTokenElement.h` | BSD-2-Clause |
| `MathML/Elements/MathMLUnderOverElement` | .cpp, .hpp | `mathml/MathMLUnderOverElement.cpp`<br>`mathml/MathMLUnderOverElement.h` | BSD-2-Clause |
| `MathML/Elements/MathMLUnknownElement` | .hpp | `mathml/MathMLUnknownElement.h` | BSD-2-Clause |
| `MathML/MathMLOperatorDictionary` | .cpp, .hpp | `mathml/MathMLOperatorDictionary.cpp`<br>`mathml/MathMLOperatorDictionary.h` | BSD-2-Clause |
| `Page/AutoscrollController` | .cpp, .hpp | `page/AutoscrollController.cpp`<br>`page/AutoscrollController.h` | BSD-2-Clause |
| `Page/DOMSelection` | .cpp, .hpp | `page/DOMSelection.cpp`<br>`page/DOMSelection.h` | BSD-3-Clause |
| `Page/Drag/DragActions` | .hpp | `page/DragActions.h` | BSD-2-Clause |
| `Page/Drag/DragClient` | .hpp | `page/DragClient.h` | BSD-2-Clause |
| `Page/Drag/DragController` | .cpp, .hpp | `page/DragController.cpp`<br>`page/DragController.h` | BSD-2-Clause |
| `Page/Drag/DragItem` | .hpp | `platform/DragItem.h` | BSD-2-Clause |
| `Page/Drag/DragState` | .hpp | `page/DragState.h` | BSD-2-Clause |
| `Page/EditorClient` | .hpp | `page/EditorClient.h` | BSD-2-Clause |
| `Page/EventHandler` | .cpp, .hpp | `page/EventHandler.cpp`<br>`page/EventHandler.h` | BSD-2-Clause |
| `Page/FrameSnapshotting` | .cpp, .hpp | `page/FrameSnapshotting.cpp`<br>`page/FrameSnapshotting.h` | BSD-2-Clause |
| `Page/IntersectionObserver/IntersectionObserver` | .cpp, .hpp | `page/IntersectionObserver.cpp`<br>`page/IntersectionObserver.h` | BSD-2-Clause |
| `Page/IntersectionObserver/IntersectionObserverEntry` | .cpp, .hpp | `page/IntersectionObserverEntry.cpp`<br>`page/IntersectionObserverEntry.h` | BSD-2-Clause |
| `Page/IntersectionObserver/IntersectionObserverMarginBox` | .hpp | `page/IntersectionObserverMarginBox.h` | BSD-2-Clause |
| `Page/LocalDOMWindow` | .cpp, .hpp | `page/LocalDOMWindow.cpp`<br>`page/LocalDOMWindow.h` | BSD-2-Clause |
| `Page/LocalFrameViewLayoutContext` | .cpp, .hpp | `page/LocalFrameViewLayoutContext.cpp`<br>`page/LocalFrameViewLayoutContext.h` | BSD-2-Clause |
| `Page/MouseEventTypes` | .hpp | `dom/MouseEventTypes.h` | BSD-2-Clause |
| `Page/PointerCaptureController` | .cpp, .hpp | `page/PointerCaptureController.cpp`<br>`page/PointerCaptureController.h` | BSD-2-Clause |
| `Page/PointerLockController` | .cpp, .hpp | `page/PointerLockController.cpp`<br>`page/PointerLockController.h` | BSD-2-Clause |
| `Page/ResizeObserver/ResizeObservation` | .cpp, .hpp | `page/ResizeObservation.cpp`<br>`page/ResizeObservation.h` | BSD-2-Clause |
| `Page/ResizeObserver/ResizeObserver` | .cpp, .hpp | `page/ResizeObserver.cpp`<br>`page/ResizeObserver.h` | BSD-2-Clause |
| `Page/ResizeObserver/ResizeObserverEntry` | .hpp | `page/ResizeObserverEntry.h` | BSD-2-Clause |
| `Page/ResizeObserver/ResizeObserverSize` | .hpp | `page/ResizeObserverSize.h` | BSD-3-Clause |
| `Page/ScrollBehavior` | .cpp, .hpp | `page/ScrollBehavior.cpp`<br>`page/ScrollBehavior.h` | BSD-2-Clause |
| `Page/Scrolling/ScrollSnapOffsetsInfo` | .cpp, .hpp | `page/scrolling/ScrollSnapOffsetsInfo.cpp`<br>`page/scrolling/ScrollSnapOffsetsInfo.h` | BSD-2-Clause |
| `Page/Scrolling/ScrollingConstraints` | .cpp, .hpp | `page/scrolling/ScrollingConstraints.cpp`<br>`page/scrolling/ScrollingConstraints.h` | BSD-2-Clause |
| `Page/SpatialNavigation` | .cpp | `page/SpatialNavigation.cpp` | BSD-2-Clause |
| `Page/VisualViewport` | .cpp, .hpp | `page/VisualViewport.cpp`<br>`page/VisualViewport.h` | BSD-2-Clause |
| `Platform/CaretAnimator` | .cpp, .hpp | `platform/CaretAnimator.cpp`<br>`platform/CaretAnimator.h` | BSD-2-Clause |
| `Platform/Choosers/ColorChooser` | .hpp | `platform/ColorChooser.h` | BSD-3-Clause |
| `Platform/Choosers/ColorChooserClient` | .hpp | `platform/ColorChooserClient.h` | BSD-3-Clause |
| `Platform/Choosers/DataListSuggestionPicker` | .hpp | `platform/DataListSuggestionPicker.h` | BSD-2-Clause |
| `Platform/Choosers/DataListSuggestionsClient` | .hpp | `platform/DataListSuggestionsClient.h` | BSD-2-Clause |
| `Platform/Choosers/DateTimeChooser` | .hpp | `platform/DateTimeChooser.h` | BSD-3-Clause |
| `Platform/Choosers/DateTimeChooserClient` | .hpp | `platform/DateTimeChooserClient.h` | BSD-3-Clause |
| `Platform/Choosers/DateTimeChooserParameters` | .hpp | `platform/DateTimeChooserParameters.h` | BSD-2-Clause |
| `Platform/Choosers/FileChooser` | .hpp | `platform/FileChooser.h` | BSD-3-Clause |
| `Platform/Choosers/PopupMenuStyle` | .hpp | `platform/PopupMenuStyle.h` | BSD-2-Clause |
| `Platform/Cursor` | .cpp, .hpp | `platform/Cursor.cpp`<br>`platform/Cursor.h` | BSD-2-Clause |
| `Platform/DateComponents` | .cpp, .hpp | `platform/DateComponents.cpp`<br>`platform/DateComponents.h` | BSD-3-Clause |
| `Platform/Decimal` | .cpp, .hpp | `platform/Decimal.cpp`<br>`platform/Decimal.h` | BSD-3-Clause |
| `Platform/Events/PlatformEvent` | .hpp | `platform/PlatformEvent.h` | BSD-2-Clause |
| `Platform/Events/PlatformKeyboardEvent` | .hpp | `platform/PlatformKeyboardEvent.h` | BSD-2-Clause |
| `Platform/Events/PlatformMouseEvent` | .hpp | `platform/PlatformMouseEvent.h` | BSD-2-Clause |
| `Platform/Events/PlatformWheelEvent` | .hpp | `platform/PlatformWheelEvent.h` | BSD-2-Clause |
| `Platform/Events/PointerID` | .hpp | `platform/PointerID.h` | BSD-2-Clause |
| `Platform/Events/WindowsKeyboardCodes` | .hpp | `platform/WindowsKeyboardCodes.h` | BSD-2-Clause |
| `Platform/GraphicsClient` | .hpp | `platform/GraphicsClient.h` | BSD-2-Clause |
| `Platform/ImageDecoders/BMPImageDecoder` | .cpp, .hpp | `platform/image-decoders/bmp/BMPImageDecoder.cpp`<br>`platform/image-decoders/bmp/BMPImageDecoder.h` | BSD-3-Clause |
| `Platform/ImageDecoders/BMPImageReader` | .cpp, .hpp | `platform/image-decoders/bmp/BMPImageReader.cpp`<br>`platform/image-decoders/bmp/BMPImageReader.h` | BSD-3-Clause |
| `Platform/ImageDecoders/GIFImageDecoder` | .cpp, .hpp | `platform/image-decoders/gif/GIFImageDecoder.cpp`<br>`platform/image-decoders/gif/GIFImageDecoder.h` | BSD-2-Clause |
| `Platform/ImageDecoders/ICOImageDecoder` | .cpp, .hpp | `platform/image-decoders/ico/ICOImageDecoder.cpp`<br>`platform/image-decoders/ico/ICOImageDecoder.h` | BSD-3-Clause |
| `Platform/ImageDecoders/JPEGImageDecoder` | .hpp | `platform/image-decoders/jpeg/JPEGImageDecoder.h` | BSD-2-Clause |
| `Platform/ImageDecoders/PNGImageDecoder` | .hpp | `platform/image-decoders/png/PNGImageDecoder.h` | BSD-2-Clause |
| `Platform/ImageDecoders/ScalableImageDecoder` | .hpp | `platform/image-decoders/ScalableImageDecoder.h` | BSD-2-Clause |
| `Platform/ImageDecoders/ScalableImageDecoderFrame` | .cpp, .hpp | `platform/image-decoders/ScalableImageDecoderFrame.cpp`<br>`platform/image-decoders/ScalableImageDecoderFrame.h` | BSD-2-Clause |
| `Platform/ImageDecoders/WEBPImageDecoder` | .cpp, .hpp | `platform/image-decoders/webp/WEBPImageDecoder.cpp`<br>`platform/image-decoders/webp/WEBPImageDecoder.h` | BSD-3-Clause |
| `Platform/Locale/DateTimeFormat` | .cpp, .hpp | `platform/text/DateTimeFormat.cpp`<br>`platform/text/DateTimeFormat.h` | BSD-2-Clause |
| `Platform/Locale/Language` | .cpp, .hpp | `WTF/wtf/Language.cpp`<br>`WTF/wtf/Language.h` | BSD-2-Clause |
| `Platform/Locale/LocaleICU` | .cpp, .hpp | `platform/text/LocaleICU.cpp`<br>`platform/text/LocaleICU.h` | BSD-3-Clause |
| `Platform/Locale/LocalizedStrings` | .cpp, .hpp | `platform/LocalizedStrings.cpp`<br>`platform/LocalizedStrings.h` | BSD-2-Clause |
| `Platform/Locale/PlatformLocale` | .cpp, .hpp | `platform/text/PlatformLocale.cpp`<br>`platform/text/PlatformLocale.h` | BSD-3-Clause / BSD-2-Clause |
| `Platform/MIMETypeRegistry` | .cpp, .hpp | `platform/MIMETypeRegistry.cpp`<br>`platform/MIMETypeRegistry.h` | BSD-2-Clause |
| `Platform/Network/BlobData` | .cpp, .hpp | `platform/network/BlobData.cpp`<br>`platform/network/BlobData.h` | BSD-3-Clause |
| `Platform/Network/DataURLDecoder` | .cpp, .hpp | `platform/network/DataURLDecoder.cpp`<br>`platform/network/DataURLDecoder.h` | BSD-2-Clause |
| `Platform/Network/HTTPHeaderMap` | .cpp, .hpp | `platform/network/HTTPHeaderMap.cpp`<br>`platform/network/HTTPHeaderMap.h` | BSD-3-Clause / BSD-2-Clause |
| `Platform/Network/HTTPParsers` | .cpp, .hpp | `platform/network/HTTPParsers.cpp`<br>`platform/network/HTTPParsers.h` | BSD-3-Clause |
| `Platform/Network/ResourceError` | .hpp | `platform/network/cf/ResourceError.h` | BSD-2-Clause |
| `Platform/Network/ResourceRequest` | .hpp | `platform/network/cf/ResourceRequest.h` | BSD-2-Clause |
| `Platform/Network/ResourceResponse` | .cpp, .hpp | `platform/network/ResourceResponseBase.cpp`<br>`platform/network/ResourceResponseBase.h` | BSD-2-Clause |
| `Platform/Pasteboard/DragData` | .cpp, .hpp | `platform/DragData.cpp`<br>`platform/DragData.h` | BSD-2-Clause |
| `Platform/Pasteboard/DragImage` | .cpp, .hpp | `platform/DragImage.cpp`<br>`platform/DragImage.h` | BSD-2-Clause |
| `Platform/Pasteboard/Pasteboard` | .cpp, .hpp | `platform/Pasteboard.cpp`<br>`platform/Pasteboard.h` | BSD-2-Clause |
| `Platform/Pasteboard/PasteboardCustomData` | .cpp, .hpp | `platform/PasteboardCustomData.cpp`<br>`platform/PasteboardCustomData.h` | BSD-2-Clause |
| `Platform/Pasteboard/PasteboardStrategy` | .hpp | `platform/PasteboardStrategy.h` | BSD-2-Clause |
| `Platform/Pasteboard/StaticPasteboard` | .cpp, .hpp | `platform/StaticPasteboard.cpp`<br>`platform/StaticPasteboard.h` | BSD-2-Clause |
| `Platform/PlatformStrategies` | .cpp, .hpp | `platform/PlatformStrategies.cpp`<br>`platform/PlatformStrategies.h` | BSD-2-Clause |
| `Platform/Scrolling/ScrollAnimator` | .cpp, .hpp | `platform/ScrollAnimator.cpp`<br>`platform/ScrollAnimator.h` | BSD-3-Clause |
| `Platform/Scrolling/ScrollTypes` | .hpp | `platform/ScrollTypes.h` | BSD-2-Clause |
| `Platform/Scrolling/ScrollableArea` | .cpp, .hpp | `platform/ScrollableArea.cpp`<br>`platform/ScrollableArea.h` | BSD-3-Clause / BSD-2-Clause |
| `Platform/Scrolling/Scrollbar` | .cpp, .hpp | `platform/Scrollbar.cpp`<br>`platform/Scrollbar.h` | BSD-2-Clause |
| `Platform/Scrolling/ScrollbarTheme` | .cpp, .hpp | `platform/ScrollbarTheme.cpp`<br>`platform/ScrollbarTheme.h` | BSD-2-Clause |
| `Platform/Scrolling/ScrollbarThemeAdwaita` | .cpp, .hpp | `platform/adwaita/ScrollbarThemeAdwaita.cpp`<br>`platform/adwaita/ScrollbarThemeAdwaita.h` | BSD-2-Clause |
| `Platform/Scrolling/ScrollbarThemeComposite` | .cpp, .hpp | `platform/ScrollbarThemeComposite.cpp`<br>`platform/ScrollbarThemeComposite.h` | BSD-2-Clause |
| `Platform/Scrolling/ScrollbarsController` | .cpp, .hpp | `platform/ScrollbarsController.cpp`<br>`platform/generic/ScrollbarsControllerGeneric.cpp`<br>`platform/ScrollbarsController.h`<br>`platform/generic/ScrollbarsControllerGeneric.h` | BSD-2-Clause AND BSD-3-Clause |
| `Platform/SharedBuffer` | .hpp | `platform/SharedBuffer.h` | BSD-2-Clause |
| `Platform/SimpleCaretAnimator` | .cpp, .hpp | `platform/SimpleCaretAnimator.cpp`<br>`platform/SimpleCaretAnimator.h` | BSD-2-Clause |
| `Platform/Theme` | .hpp | `platform/Theme.h` | BSD-2-Clause |
| `Platform/ThemeAdwaita` | .cpp, .hpp | `platform/adwaita/ThemeAdwaita.cpp`<br>`platform/adwaita/ThemeAdwaita.h` | BSD-2-Clause |
| `Platform/ThemeTypes` | .hpp | `platform/ThemeTypes.h` | BSD-2-Clause |
| `Platform/ThreadTimers` | .cpp, .hpp | `platform/ThreadTimers.cpp`<br>`platform/ThreadTimers.h` | BSD-2-Clause |
| `Platform/Timer` | .cpp, .hpp | `platform/Timer.cpp`<br>`platform/Timer.h` | BSD-2-Clause |
| `Rendering/Adwaita/RenderThemeAdwaita` | .cpp, .hpp | `rendering/adwaita/RenderThemeAdwaita.cpp`<br>`rendering/adwaita/RenderThemeAdwaita.h` | BSD-2-Clause |
| `Rendering/BaselineAlignment` | .cpp, .hpp | `rendering/BaselineAlignment.cpp`<br>`rendering/BaselineAlignment.h` | BSD-2-Clause |
| `Rendering/BlockStepSizing` | .cpp, .hpp | `rendering/BlockStepSizing.cpp`<br>`rendering/BlockStepSizing.h` | BSD-2-Clause |
| `Rendering/FormControls/RenderListBox` | .cpp, .hpp | `rendering/RenderListBox.cpp`<br>`rendering/RenderListBox.h` | BSD-3-Clause |
| `Rendering/Grid/AncestorSubgridIterator` | .cpp, .hpp | `rendering/AncestorSubgridIterator.cpp`<br>`rendering/AncestorSubgridIterator.h` | BSD-2-Clause |
| `Rendering/Grid/Grid` | .cpp, .hpp | `rendering/Grid.cpp`<br>`rendering/Grid.h` | BSD-2-Clause |
| `Rendering/Grid/GridBaselineAlignment` | .cpp, .hpp | `rendering/GridBaselineAlignment.cpp`<br>`rendering/GridBaselineAlignment.h` | BSD-3-Clause / BSD-2-Clause |
| `Rendering/Grid/GridLayoutFunctions` | .cpp, .hpp | `rendering/GridLayoutFunctions.cpp`<br>`rendering/GridLayoutFunctions.h` | BSD-2-Clause |
| `Rendering/Grid/GridLayoutState` | .cpp, .hpp | `rendering/GridLayoutState.cpp`<br>`rendering/GridLayoutState.h` | BSD-2-Clause |
| `Rendering/Grid/GridMasonryLayout` | .cpp, .hpp | `rendering/GridMasonryLayout.cpp`<br>`rendering/GridMasonryLayout.h` | BSD-2-Clause |
| `Rendering/Grid/GridTrackSizingAlgorithm` | .cpp, .hpp | `rendering/GridTrackSizingAlgorithm.cpp`<br>`rendering/GridTrackSizingAlgorithm.h` | BSD-2-Clause |
| `Rendering/Grid/RenderGrid` | .cpp, .hpp | `rendering/RenderGrid.cpp`<br>`rendering/RenderGrid.h` | BSD-2-Clause |
| `Rendering/HitTesting/HitTestSource` | .hpp | `rendering/HitTestSource.h` | BSD-2-Clause |
| `Rendering/HitTesting/HitTestingTransformState` | .cpp, .hpp | `rendering/HitTestingTransformState.cpp`<br>`rendering/HitTestingTransformState.h` | BSD-2-Clause |
| `Rendering/Iterators/OrderIterator` | .cpp, .hpp | `rendering/OrderIterator.cpp`<br>`rendering/OrderIterator.h` | BSD-3-Clause |
| `Rendering/Iterators/RenderAncestorIterator` | .hpp | `rendering/RenderAncestorIterator.h` | BSD-2-Clause |
| `Rendering/Iterators/RenderChildIterator` | .hpp | `rendering/RenderChildIterator.h` | BSD-2-Clause |
| `Rendering/Iterators/RenderDescendantIterator` | .hpp | `rendering/RenderDescendantIterator.h` | BSD-2-Clause |
| `Rendering/Iterators/RenderIterator` | .hpp | `rendering/RenderIterator.h` | BSD-2-Clause |
| `Rendering/Layers/ClipRect` | .hpp | `rendering/ClipRect.h` | BSD-2-Clause |
| `Rendering/Layers/LayerAncestorClippingStack` | .cpp, .hpp | `rendering/LayerAncestorClippingStack.cpp`<br>`rendering/LayerAncestorClippingStack.h` | BSD-2-Clause |
| `Rendering/Layers/LayerFragment` | .hpp | `rendering/LayerFragment.h` | BSD-2-Clause |
| `Rendering/Layers/LayerOverlapMap` | .cpp, .hpp | `rendering/LayerOverlapMap.cpp`<br>`rendering/LayerOverlapMap.h` | BSD-2-Clause |
| `Rendering/Layers/RenderLayerBacking` | .cpp, .hpp | `rendering/RenderLayerBacking.cpp`<br>`rendering/RenderLayerBacking.h` | BSD-2-Clause |
| `Rendering/Layers/RenderLayerCompositor` | .cpp, .hpp | `rendering/RenderLayerCompositor.cpp`<br>`rendering/RenderLayerCompositor.h` | BSD-2-Clause |
| `Rendering/Layers/RenderLayerFilters` | .cpp, .hpp | `rendering/RenderLayerFilters.cpp`<br>`rendering/RenderLayerFilters.h` | BSD-2-Clause |
| `Rendering/LayoutScope` | .cpp, .hpp | `rendering/LayoutScope.cpp`<br>`rendering/LayoutScope.h` | BSD-2-Clause |
| `Rendering/Line/BreakablePositions` | .cpp | `rendering/BreakablePositions.cpp` | BSD-2-Clause |
| `Rendering/Line/InlineWalker` | .hpp | `rendering/InlineWalker.h` | BSD-2-Clause |
| `Rendering/Line/LegacyInlineIterator` | .cpp | `rendering/LegacyInlineIterator.cpp` | BSD-2-Clause |
| `Rendering/Line/LegacyLineLayout` | .hpp | `rendering/LegacyLineLayout.h` | BSD-2-Clause |
| `Rendering/Line/LineInfo` | .hpp | `rendering/line/LineInfo.h` | BSD-2-Clause |
| `Rendering/Line/LineWidth` | .cpp, .hpp | `rendering/line/LineWidth.cpp`<br>`rendering/line/LineWidth.h` | BSD-2-Clause |
| `Rendering/Line/RenderLineBoxList` | .cpp, .hpp | `rendering/RenderLineBoxList.cpp`<br>`rendering/RenderLineBoxList.h` | BSD-3-Clause |
| `Rendering/Line/RenderTextLineBoxes` | .cpp, .hpp | `rendering/RenderTextLineBoxes.cpp`<br>`rendering/RenderTextLineBoxes.h` | BSD-2-Clause |
| `Rendering/LineClampUpdater` | .cpp, .hpp | `rendering/LineClampUpdater.h` | BSD-2-Clause |
| `Rendering/MathML/MathOperator` | .cpp, .hpp | `rendering/mathml/MathOperator.cpp`<br>`rendering/mathml/MathOperator.h` | BSD-2-Clause |
| `Rendering/MathML/MathVariant` | .cpp, .hpp | `rendering/mathml/MathVariant.cpp`<br>`rendering/mathml/MathVariant.h` | BSD-2-Clause |
| `Rendering/MathML/RenderMathMLBlock` | .cpp, .hpp | `rendering/mathml/RenderMathMLBlock.cpp`<br>`rendering/mathml/RenderMathMLBlock.h` | BSD-2-Clause |
| `Rendering/MathML/RenderMathMLFraction` | .cpp, .hpp | `rendering/mathml/RenderMathMLFraction.cpp`<br>`rendering/mathml/RenderMathMLFraction.h` | BSD-2-Clause |
| `Rendering/MathML/RenderMathMLMath` | .cpp, .hpp | `rendering/mathml/RenderMathMLMath.cpp`<br>`rendering/mathml/RenderMathMLMath.h` | BSD-2-Clause |
| `Rendering/MathML/RenderMathMLMenclose` | .cpp, .hpp | `rendering/mathml/RenderMathMLMenclose.cpp`<br>`rendering/mathml/RenderMathMLMenclose.h` | BSD-2-Clause |
| `Rendering/MathML/RenderMathMLOperator` | .cpp, .hpp | `rendering/mathml/RenderMathMLOperator.cpp`<br>`rendering/mathml/RenderMathMLOperator.h` | BSD-2-Clause |
| `Rendering/MathML/RenderMathMLPadded` | .cpp, .hpp | `rendering/mathml/RenderMathMLPadded.cpp`<br>`rendering/mathml/RenderMathMLPadded.h` | BSD-2-Clause |
| `Rendering/MathML/RenderMathMLRoot` | .cpp, .hpp | `rendering/mathml/RenderMathMLRoot.cpp`<br>`rendering/mathml/RenderMathMLRoot.h` | BSD-2-Clause |
| `Rendering/MathML/RenderMathMLRow` | .cpp, .hpp | `rendering/mathml/RenderMathMLRow.cpp`<br>`rendering/mathml/RenderMathMLRow.h` | BSD-2-Clause |
| `Rendering/MathML/RenderMathMLScripts` | .cpp, .hpp | `rendering/mathml/RenderMathMLScripts.cpp`<br>`rendering/mathml/RenderMathMLScripts.h` | BSD-2-Clause |
| `Rendering/MathML/RenderMathMLSpace` | .cpp, .hpp | `rendering/mathml/RenderMathMLSpace.cpp`<br>`rendering/mathml/RenderMathMLSpace.h` | BSD-2-Clause |
| `Rendering/MathML/RenderMathMLToken` | .cpp, .hpp | `rendering/mathml/RenderMathMLToken.cpp`<br>`rendering/mathml/RenderMathMLToken.h` | BSD-2-Clause |
| `Rendering/MathML/RenderMathMLUnderOver` | .cpp, .hpp | `rendering/mathml/RenderMathMLUnderOver.cpp`<br>`rendering/mathml/RenderMathMLUnderOver.h` | BSD-2-Clause |
| `Rendering/MotionPath` | .cpp, .hpp | `rendering/MotionPath.cpp`<br>`rendering/MotionPath.h` | BSD-2-Clause |
| `Rendering/Painting/BorderEdge` | .cpp, .hpp | `rendering/BorderEdge.cpp`<br>`rendering/BorderEdge.h` | BSD-2-Clause |
| `Rendering/Painting/BorderShape` | .cpp, .hpp | `rendering/BorderShape.cpp`<br>`rendering/BorderShape.h` | BSD-2-Clause |
| `Rendering/Painting/CSSFilterRenderer` | .cpp, .hpp | `rendering/CSSFilterRenderer.cpp`<br>`rendering/CSSFilterRenderer.h` | BSD-2-Clause |
| `Rendering/Painting/EllipsisBoxPainter` | .cpp, .hpp | `rendering/EllipsisBoxPainter.cpp`<br>`rendering/EllipsisBoxPainter.h` | BSD-2-Clause |
| `Rendering/Painting/ImageQualityController` | .cpp, .hpp | `rendering/ImageQualityController.cpp`<br>`rendering/ImageQualityController.h` | BSD-2-Clause |
| `Rendering/Painting/InlineBoxPainter` | .cpp, .hpp | `rendering/InlineBoxPainter.cpp`<br>`rendering/InlineBoxPainter.h` | BSD-2-Clause |
| `Rendering/Painting/MarkedText` | .cpp, .hpp | `rendering/MarkedText.cpp`<br>`rendering/MarkedText.h` | BSD-2-Clause |
| `Rendering/Painting/StyledMarkedText` | .cpp, .hpp | `rendering/StyledMarkedText.cpp`<br>`rendering/StyledMarkedText.h` | BSD-2-Clause |
| `Rendering/Painting/TextBoxPainter` | .cpp, .hpp | `rendering/TextBoxPainter.cpp`<br>`rendering/TextBoxPainter.h` | BSD-2-Clause |
| `Rendering/Painting/TextPaintStyle` | .cpp, .hpp | `rendering/TextPaintStyle.cpp`<br>`rendering/TextPaintStyle.h` | BSD-2-Clause |
| `Rendering/PathOperation` | .cpp, .hpp | `rendering/PathOperation.cpp`<br>`rendering/PathOperation.h` | BSD-2-Clause |
| `Rendering/PositionedLayoutConstraints` | .cpp, .hpp | `rendering/PositionedLayoutConstraints.cpp`<br>`rendering/PositionedLayoutConstraints.h` | BSD-2-Clause |
| `Rendering/ReferencedSVGResources` | .cpp, .hpp | `rendering/ReferencedSVGResources.cpp`<br>`rendering/ReferencedSVGResources.h` | BSD-2-Clause |
| `Rendering/RenderFlexibleBox` | .cpp, .hpp | `rendering/RenderFlexibleBox.cpp`<br>`rendering/RenderFlexibleBox.h` | BSD-3-Clause |
| `Rendering/RenderGeometryMap` | .cpp, .hpp | `rendering/RenderGeometryMap.cpp`<br>`rendering/RenderGeometryMap.h` | BSD-2-Clause |
| `Rendering/RenderLayoutState` | .cpp, .hpp | `rendering/RenderLayoutState.cpp`<br>`rendering/RenderLayoutState.h` | BSD-2-Clause |
| `Rendering/RenderPtr` | .hpp | `rendering/RenderPtr.h` | BSD-2-Clause |
| `Rendering/RenderTreeAsText` | .cpp, .hpp | `rendering/RenderTreeAsText.cpp`<br>`rendering/RenderTreeAsText.h` | BSD-2-Clause |
| `Rendering/RepaintRectCalculation` | .hpp | `rendering/RepaintRectCalculation.h` | BSD-2-Clause |
| `Rendering/Replaced/RenderHTMLCanvas` | .cpp, .hpp | `rendering/RenderHTMLCanvas.cpp`<br>`rendering/RenderHTMLCanvas.h` | BSD-2-Clause |
| `Rendering/SVG/Legacy/LegacyRenderSVGEllipse` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGEllipse.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGEllipse.h` | BSD-2-Clause |
| `Rendering/SVG/Legacy/LegacyRenderSVGModelObject` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGModelObject.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGModelObject.h` | BSD-3-Clause |
| `Rendering/SVG/Legacy/LegacyRenderSVGRect` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGRect.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGRect.h` | BSD-2-Clause |
| `Rendering/SVG/Legacy/LegacyRenderSVGResourceFilterPrimitive` | .cpp, .hpp | `rendering/svg/legacy/LegacyRenderSVGResourceFilterPrimitive.cpp`<br>`rendering/svg/legacy/LegacyRenderSVGResourceFilterPrimitive.h` | BSD-2-Clause |
| `Rendering/SVG/SVGRenderTreeAsText` | .cpp, .hpp | `rendering/svg/SVGRenderTreeAsText.cpp`<br>`rendering/svg/SVGRenderTreeAsText.h` | BSD-2-Clause |
| `Rendering/SVG/SVGTextBoxPainter` | .cpp, .hpp | `rendering/svg/SVGTextBoxPainter.cpp`<br>`rendering/svg/SVGTextBoxPainter.h` | BSD-2-Clause |
| `Rendering/Selection/CaretRectComputation` | .cpp, .hpp | `rendering/CaretRectComputation.cpp`<br>`rendering/CaretRectComputation.h` | BSD-2-Clause |
| `Rendering/Selection/RenderHighlight` | .cpp, .hpp | `rendering/RenderHighlight.cpp`<br>`rendering/RenderHighlight.h` | BSD-2-Clause |
| `Rendering/Selection/RenderSelection` | .cpp, .hpp | `rendering/RenderSelection.cpp`<br>`rendering/RenderSelection.h` | BSD-2-Clause |
| `Rendering/Selection/RenderSelectionGeometry` | .cpp | `rendering/RenderSelectionGeometry.cpp` | BSD-2-Clause |
| `Rendering/Selection/TextBoxSelectableRange` | .hpp | `rendering/TextBoxSelectableRange.h` | BSD-2-Clause |
| `Rendering/Shapes/BoxLayoutShape` | .cpp, .hpp | `rendering/shapes/BoxLayoutShape.cpp`<br>`rendering/shapes/BoxLayoutShape.h` | BSD-2-Clause |
| `Rendering/Shapes/LayoutShape` | .cpp, .hpp | `rendering/shapes/LayoutShape.cpp`<br>`rendering/shapes/LayoutShape.h` | BSD-2-Clause |
| `Rendering/Shapes/PolygonLayoutShape` | .cpp, .hpp | `rendering/shapes/PolygonLayoutShape.cpp`<br>`rendering/shapes/PolygonLayoutShape.h` | BSD-2-Clause |
| `Rendering/Shapes/RectangleLayoutShape` | .cpp, .hpp | `rendering/shapes/RectangleLayoutShape.cpp`<br>`rendering/shapes/RectangleLayoutShape.h` | BSD-2-Clause |
| `Rendering/Shapes/ShapeInterval` | .hpp | `rendering/shapes/ShapeInterval.h` | BSD-2-Clause |
| `Rendering/Shapes/ShapeOutsideInfo` | .cpp, .hpp | `rendering/shapes/ShapeOutsideInfo.cpp`<br>`rendering/shapes/ShapeOutsideInfo.h` | BSD-2-Clause |
| `Rendering/Style/BorderData` | .cpp | `rendering/style/BorderData.cpp` | BSD-2-Clause |
| `Rendering/Style/DropShadowFilterOperationWithStyleColor` | .cpp, .hpp | `rendering/style/DropShadowFilterOperationWithStyleColor.cpp`<br>`rendering/style/DropShadowFilterOperationWithStyleColor.h` | BSD-2-Clause |
| `Rendering/Style/GridArea` | .hpp | `rendering/style/GridArea.h` | BSD-3-Clause |
| `Rendering/Style/GridSpan` | .cpp, .hpp | `rendering/style/GridSpan.cpp`<br>`rendering/style/GridSpan.h` | BSD-2-Clause |
| `Rendering/Style/PositionTryOrder` | .cpp, .hpp | `rendering/style/PositionTryOrder.cpp`<br>`rendering/style/PositionTryOrder.h` | BSD-2-Clause |
| `Rendering/Style/ReferenceFilterOperation` | .cpp, .hpp | `rendering/style/ReferenceFilterOperation.cpp`<br>`rendering/style/ReferenceFilterOperation.h` | BSD-2-Clause |
| `Rendering/Style/RenderStyleBase+ConstructionInlines` | .hpp | `rendering/style/RenderStyleBase+ConstructionInlines.h` | BSD-2-Clause |
| `Rendering/Style/RenderStyleBase+GettersInlines` | .hpp | `rendering/style/RenderStyleBase+GettersInlines.h` | BSD-2-Clause |
| `Rendering/Style/RenderStyleBase` | .cpp | `rendering/style/RenderStyleBase.cpp` | BSD-2-Clause |
| `Rendering/Style/RenderStyleConstants` | .cpp | `rendering/style/RenderStyleConstants.cpp` | BSD-2-Clause |
| `Rendering/Style/StyleBackdropFilterData` | .cpp, .hpp | `rendering/style/StyleBackdropFilterData.cpp`<br>`rendering/style/StyleBackdropFilterData.h` | BSD-2-Clause |
| `Rendering/Style/StyleBorderImageData` | .cpp, .hpp | `rendering/style/StyleBorderImageData.cpp`<br>`rendering/style/StyleBorderImageData.h` | BSD-2-Clause |
| `Rendering/Style/StyleContentAlignmentData` | .cpp, .hpp | `rendering/style/StyleContentAlignmentData.cpp`<br>`rendering/style/StyleContentAlignmentData.h` | BSD-2-Clause |
| `Rendering/Style/StyleCrossfadeImage` | .cpp, .hpp | `rendering/style/StyleCrossfadeImage.cpp`<br>`rendering/style/StyleCrossfadeImage.h` | BSD-2-Clause |
| `Rendering/Style/StyleCustomPropertyData` | .cpp | `rendering/style/StyleCustomPropertyData.cpp` | BSD-2-Clause |
| `Rendering/Style/StyleFilterData` | .cpp, .hpp | `rendering/style/StyleFilterData.cpp`<br>`rendering/style/StyleFilterData.h` | BSD-2-Clause |
| `Rendering/Style/StyleFilterImage` | .cpp, .hpp | `rendering/style/StyleFilterImage.cpp`<br>`rendering/style/StyleFilterImage.h` | BSD-2-Clause |
| `Rendering/Style/StyleFlexibleBoxData` | .cpp, .hpp | `rendering/style/StyleFlexibleBoxData.cpp`<br>`rendering/style/StyleFlexibleBoxData.h` | BSD-2-Clause |
| `Rendering/Style/StyleFontData` | .cpp, .hpp | `rendering/style/StyleFontData.cpp`<br>`rendering/style/StyleFontData.h` | BSD-2-Clause |
| `Rendering/Style/StyleGradientImage` | .cpp, .hpp | `rendering/style/StyleGradientImage.cpp`<br>`rendering/style/StyleGradientImage.h` | BSD-2-Clause |
| `Rendering/Style/StyleGridData` | .cpp, .hpp | `rendering/style/StyleGridData.cpp`<br>`rendering/style/StyleGridData.h` | BSD-2-Clause |
| `Rendering/Style/StyleGridItemData` | .cpp, .hpp | `rendering/style/StyleGridItemData.cpp`<br>`rendering/style/StyleGridItemData.h` | BSD-3-Clause |
| `Rendering/Style/StyleInvalidImage` | .cpp, .hpp | `rendering/style/StyleInvalidImage.cpp`<br>`rendering/style/StyleInvalidImage.h` | BSD-2-Clause |
| `Rendering/Style/StyleMaskBorderData` | .cpp, .hpp | `rendering/style/StyleMaskBorderData.cpp`<br>`rendering/style/StyleMaskBorderData.h` | BSD-2-Clause |
| `Rendering/Style/StyleMiscNonInheritedData` | .cpp, .hpp | `rendering/style/StyleMiscNonInheritedData.cpp`<br>`rendering/style/StyleMiscNonInheritedData.h` | BSD-2-Clause |
| `Rendering/Style/StyleNonInheritedData` | .cpp, .hpp | `rendering/style/StyleNonInheritedData.cpp`<br>`rendering/style/StyleNonInheritedData.h` | BSD-2-Clause |
| `Rendering/Style/StylePaintImage` | .cpp, .hpp | `rendering/style/StylePaintImage.cpp`<br>`rendering/style/StylePaintImage.h` | BSD-2-Clause |
| `Rendering/Style/StyleSelfAlignmentData` | .cpp, .hpp | `rendering/style/StyleSelfAlignmentData.cpp`<br>`rendering/style/StyleSelfAlignmentData.h` | BSD-2-Clause |
| `Rendering/TextBoxTrimmer` | .cpp, .hpp | `rendering/TextBoxTrimmer.cpp`<br>`rendering/TextBoxTrimmer.h` | BSD-2-Clause |
| `Rendering/TransformOperationData` | .hpp | `rendering/TransformOperationData.h` | BSD-2-Clause |
| `Rendering/Updating/RenderTreeBuilder` | .cpp, .hpp | `rendering/updating/RenderTreeBuilder.cpp`<br>`rendering/updating/RenderTreeBuilder.h` | BSD-2-Clause |
| `Rendering/Updating/RenderTreeBuilderBlock` | .cpp, .hpp | `rendering/updating/RenderTreeBuilderBlock.cpp`<br>`rendering/updating/RenderTreeBuilderBlock.h` | BSD-2-Clause |
| `Rendering/Updating/RenderTreeBuilderBlockFlow` | .cpp, .hpp | `rendering/updating/RenderTreeBuilderBlockFlow.cpp`<br>`rendering/updating/RenderTreeBuilderBlockFlow.h` | BSD-2-Clause |
| `Rendering/Updating/RenderTreeBuilderFirstLetter` | .hpp | `rendering/updating/RenderTreeBuilderFirstLetter.h` | BSD-2-Clause |
| `Rendering/Updating/RenderTreeBuilderFormControls` | .cpp, .hpp | `rendering/updating/RenderTreeBuilderFormControls.cpp`<br>`rendering/updating/RenderTreeBuilderFormControls.h` | BSD-2-Clause |
| `Rendering/Updating/RenderTreeBuilderInline` | .cpp, .hpp | `rendering/updating/RenderTreeBuilderInline.cpp`<br>`rendering/updating/RenderTreeBuilderInline.h` | BSD-2-Clause |
| `Rendering/Updating/RenderTreeBuilderList` | .hpp | `rendering/updating/RenderTreeBuilderList.h` | BSD-2-Clause |
| `Rendering/Updating/RenderTreeBuilderRuby` | .cpp, .hpp | `rendering/updating/RenderTreeBuilderRuby.cpp`<br>`rendering/updating/RenderTreeBuilderRuby.h` | BSD-2-Clause |
| `Rendering/Updating/RenderTreeBuilderSVG` | .cpp, .hpp | `rendering/updating/RenderTreeBuilderSVG.cpp`<br>`rendering/updating/RenderTreeBuilderSVG.h` | BSD-2-Clause |
| `Rendering/Updating/RenderTreeBuilderTable` | .cpp, .hpp | `rendering/updating/RenderTreeBuilderTable.cpp`<br>`rendering/updating/RenderTreeBuilderTable.h` | BSD-2-Clause |
| `Rendering/Updating/RenderTreePosition` | .cpp, .hpp | `rendering/updating/RenderTreePosition.cpp`<br>`rendering/updating/RenderTreePosition.h` | BSD-2-Clause |
| `Rendering/Updating/RenderTreeUpdater` | .cpp, .hpp | `rendering/updating/RenderTreeUpdater.cpp`<br>`rendering/updating/RenderTreeUpdater.h` | BSD-2-Clause |
| `Rendering/Updating/RenderTreeUpdaterGeneratedContent` | .cpp, .hpp | `rendering/updating/RenderTreeUpdaterGeneratedContent.cpp`<br>`rendering/updating/RenderTreeUpdaterGeneratedContent.h` | BSD-2-Clause |
| `Rendering/VisibleRectContext` | .hpp | `rendering/VisibleRectContext.h` | BSD-2-Clause |
| `SVG/Animation/SMILTime` | .cpp, .hpp | `svg/animation/SMILTime.cpp`<br>`svg/animation/SMILTime.h` | BSD-2-Clause |
| `SVG/Animation/SMILTimeContainer` | .cpp, .hpp | `svg/animation/SMILTimeContainer.cpp`<br>`svg/animation/SMILTimeContainer.h` | BSD-2-Clause |
| `SVG/Animation/SVGSMILElement` | .cpp, .hpp | `svg/animation/SVGSMILElement.cpp`<br>`svg/animation/SVGSMILElement.h` | BSD-2-Clause |
| `SVG/Elements/SVGAnimateElement` | .hpp | `svg/SVGAnimateElement.h` | BSD-2-Clause |
| `SVG/Elements/SVGUnknownElement` | .cpp, .hpp | `svg/SVGUnknownElement.cpp`<br>`svg/SVGUnknownElement.h` | BSD-2-Clause |
| `SVG/Graphics/Filters/SVGFilterEffectGraph` | .hpp | `svg/graphics/filters/SVGFilterEffectGraph.h` | BSD-2-Clause |
| `SVG/Graphics/Filters/SVGFilterExpression` | .hpp | `svg/graphics/filters/SVGFilterExpression.h` | BSD-2-Clause |
| `SVG/Graphics/Filters/SVGFilterGraph` | .hpp | `svg/graphics/filters/SVGFilterGraph.h` | BSD-2-Clause |
| `SVG/Graphics/Filters/SVGFilterPrimitiveGraph` | .hpp | `svg/graphics/filters/SVGFilterPrimitiveGraph.h` | BSD-2-Clause |
| `SVG/Graphics/SVGImage` | .cpp, .hpp | `svg/graphics/SVGImage.cpp`<br>`svg/graphics/SVGImage.h` | BSD-2-Clause |
| `SVG/Graphics/SVGImageClients` | .hpp | `svg/graphics/SVGImageClients.h` | BSD-3-Clause |
| `SVG/Graphics/SVGImageForContainer` | .hpp | `svg/graphics/SVGImageForContainer.h` | BSD-2-Clause |
| `SVG/Graphics/SVGResourceImage` | .cpp, .hpp | `svg/graphics/SVGResourceImage.cpp`<br>`svg/graphics/SVGResourceImage.h` | BSD-2-Clause |
| `SVG/Internals/SVGPathAbsoluteConverter` | .cpp, .hpp | `svg/SVGPathAbsoluteConverter.cpp`<br>`svg/SVGPathAbsoluteConverter.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimatedDecoratedProperty` | .hpp | `svg/properties/SVGAnimatedDecoratedProperty.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimatedPrimitiveProperty` | .hpp | `svg/properties/SVGAnimatedPrimitiveProperty.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimatedProperty` | .cpp, .hpp | `svg/properties/SVGAnimatedProperty.cpp`<br>`svg/properties/SVGAnimatedProperty.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimatedPropertyAccessor` | .hpp | `svg/properties/SVGAnimatedPropertyAccessor.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimatedPropertyAccessorImpl` | .hpp | `svg/properties/SVGAnimatedPropertyAccessorImpl.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimatedPropertyAnimator` | .hpp | `svg/properties/SVGAnimatedPropertyAnimator.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimatedPropertyAnimatorImpl` | .hpp | `svg/properties/SVGAnimatedPropertyAnimatorImpl.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimatedPropertyImpl` | .hpp | `svg/properties/SVGAnimatedPropertyImpl.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimatedPropertyList` | .hpp | `svg/properties/SVGAnimatedPropertyList.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimatedPropertyPairAccessor` | .hpp | `svg/properties/SVGAnimatedPropertyPairAccessor.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimatedPropertyPairAccessorImpl` | .hpp | `svg/properties/SVGAnimatedPropertyPairAccessorImpl.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimatedPropertyPairAnimator` | .hpp | `svg/properties/SVGAnimatedPropertyPairAnimator.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimatedPropertyPairAnimatorImpl` | .hpp | `svg/properties/SVGAnimatedPropertyPairAnimatorImpl.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimatedString` | .hpp | `svg/properties/SVGAnimatedString.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimatedValueProperty` | .hpp | `svg/properties/SVGAnimatedValueProperty.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimationAdditiveFunction` | .hpp | `svg/properties/SVGAnimationAdditiveFunction.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimationAdditiveListFunction` | .hpp | `svg/properties/SVGAnimationAdditiveListFunction.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimationAdditiveListFunctionImpl` | .hpp | `svg/properties/SVGAnimationAdditiveListFunctionImpl.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimationAdditiveValueFunction` | .hpp | `svg/properties/SVGAnimationAdditiveValueFunction.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimationAdditiveValueFunctionImpl` | .cpp, .hpp | `svg/properties/SVGAnimationAdditiveValueFunctionImpl.cpp`<br>`svg/properties/SVGAnimationAdditiveValueFunctionImpl.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimationDiscreteFunction` | .hpp | `svg/properties/SVGAnimationDiscreteFunction.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimationDiscreteFunctionImpl` | .hpp | `svg/properties/SVGAnimationDiscreteFunctionImpl.h` | BSD-2-Clause |
| `SVG/Properties/SVGAnimationFunction` | .hpp | `svg/properties/SVGAnimationFunction.h` | BSD-2-Clause |
| `SVG/Properties/SVGAttributeAnimator` | .cpp, .hpp | `svg/properties/SVGAttributeAnimator.cpp`<br>`svg/properties/SVGAttributeAnimator.h` | BSD-2-Clause |
| `SVG/Properties/SVGDecoratedEnumeration` | .hpp | `svg/properties/SVGDecoratedEnumeration.h` | BSD-2-Clause |
| `SVG/Properties/SVGDecoratedPrimitive` | .hpp | `svg/properties/SVGDecoratedPrimitive.h` | BSD-2-Clause |
| `SVG/Properties/SVGDecoratedProperty` | .hpp | `svg/properties/SVGDecoratedProperty.h` | BSD-2-Clause |
| `SVG/Properties/SVGList` | .hpp | `svg/properties/SVGList.h` | BSD-2-Clause |
| `SVG/Properties/SVGMemberAccessor` | .hpp | `svg/properties/SVGMemberAccessor.h` | BSD-2-Clause |
| `SVG/Properties/SVGPointerMemberAccessor` | .hpp | `svg/properties/SVGPointerMemberAccessor.h` | BSD-2-Clause |
| `SVG/Properties/SVGPrimitiveList` | .hpp | `svg/properties/SVGPrimitiveList.h` | BSD-2-Clause |
| `SVG/Properties/SVGPrimitivePropertyAnimator` | .hpp | `svg/properties/SVGPrimitivePropertyAnimator.h` | BSD-2-Clause |
| `SVG/Properties/SVGPrimitivePropertyAnimatorImpl` | .hpp | `svg/properties/SVGPrimitivePropertyAnimatorImpl.h` | BSD-2-Clause |
| `SVG/Properties/SVGPropertyAccessor` | .hpp | `svg/properties/SVGPropertyAccessor.h` | BSD-2-Clause |
| `SVG/Properties/SVGPropertyAccessorImpl` | .hpp | `svg/properties/SVGPropertyAccessorImpl.h` | BSD-2-Clause |
| `SVG/Properties/SVGPropertyAnimator` | .hpp | `svg/properties/SVGPropertyAnimator.h` | BSD-2-Clause |
| `SVG/Properties/SVGPropertyAnimatorFactory` | .hpp | `svg/properties/SVGPropertyAnimatorFactory.h` | BSD-2-Clause |
| `SVG/Properties/SVGPropertyList` | .hpp | `svg/properties/SVGPropertyList.h` | BSD-2-Clause |
| `SVG/Properties/SVGPropertyOwner` | .hpp | `svg/properties/SVGPropertyOwner.h` | BSD-2-Clause |
| `SVG/Properties/SVGPropertyOwnerRegistry` | .hpp | `svg/properties/SVGPropertyOwnerRegistry.h` | BSD-2-Clause |
| `SVG/Properties/SVGPropertyRegistry` | .hpp | `svg/properties/SVGPropertyRegistry.h` | BSD-2-Clause |
| `SVG/Properties/SVGPropertyTraits` | .cpp | `svg/properties/SVGPropertyTraits.cpp` | BSD-2-Clause |
| `SVG/Properties/SVGSharedPrimitiveProperty` | .hpp | `svg/properties/SVGSharedPrimitiveProperty.h` | BSD-2-Clause |
| `SVG/Properties/SVGValueProperty` | .hpp | `svg/properties/SVGValueProperty.h` | BSD-2-Clause |
| `SVG/Properties/SVGValuePropertyAnimator` | .hpp | `svg/properties/SVGValuePropertyAnimator.h` | BSD-2-Clause |
| `SVG/Properties/SVGValuePropertyAnimatorImpl` | .hpp | `svg/properties/SVGValuePropertyAnimatorImpl.h` | BSD-2-Clause |
| `SVG/Properties/SVGValuePropertyList` | .hpp | `svg/properties/SVGValuePropertyList.h` | BSD-2-Clause |
| `SVG/Properties/SVGValuePropertyListAnimator` | .hpp | `svg/properties/SVGValuePropertyListAnimator.h` | BSD-2-Clause |
| `SVG/Properties/SVGValuePropertyListAnimatorImpl` | .hpp | `svg/properties/SVGValuePropertyListAnimatorImpl.h` | BSD-2-Clause |
| `SVG/SVGAngle` | .hpp | `svg/SVGAngle.h` | BSD-2-Clause |
| `SVG/SVGLength` | .cpp, .hpp | `svg/SVGLength.cpp`<br>`svg/SVGLength.h` | BSD-2-Clause |
| `SVG/SVGLengthList` | .cpp, .hpp | `svg/SVGLengthList.cpp`<br>`svg/SVGLengthList.h` | BSD-2-Clause |
| `SVG/SVGMarkerTypes` | .hpp | `svg/SVGMarkerTypes.h` | BSD-2-Clause |
| `SVG/SVGMatrix` | .hpp | `svg/SVGMatrix.h` | BSD-2-Clause |
| `SVG/SVGNumber` | .hpp | `svg/SVGNumber.h` | BSD-2-Clause |
| `SVG/SVGNumberList` | .cpp, .hpp | `svg/SVGNumberList.cpp`<br>`svg/SVGNumberList.h` | BSD-2-Clause |
| `SVG/SVGParsingError` | .hpp | `svg/SVGParsingError.h` | BSD-2-Clause |
| `SVG/SVGPoint` | .hpp | `svg/SVGPoint.h` | BSD-2-Clause |
| `SVG/SVGPointList` | .cpp, .hpp | `svg/SVGPointList.cpp`<br>`svg/SVGPointList.h` | BSD-2-Clause |
| `SVG/SVGPreserveAspectRatio` | .hpp | `svg/SVGPreserveAspectRatio.h` | BSD-2-Clause |
| `SVG/SVGResourceElementClient` | .hpp | `svg/SVGResourceElementClient.h` | BSD-2-Clause |
| `SVG/SVGStringList` | .cpp, .hpp | `svg/SVGStringList.cpp`<br>`svg/SVGStringList.h` | BSD-2-Clause |
| `SVG/SVGTransform` | .hpp | `svg/SVGTransform.h` | BSD-2-Clause |
| `SVG/SVGTransformList` | .cpp | `svg/SVGTransformList.cpp` | BSD-2-Clause |
| `Style/Adjuster` | .hpp | `style/StyleAdjuster.h` | BSD-2-Clause |
| `Style/AnchorPositionEvaluator` | .cpp, .hpp | `style/AnchorPositionEvaluator.cpp`<br>`style/AnchorPositionEvaluator.h` | BSD-2-Clause |
| `Style/AnchorSizeDimension` | .hpp | `style/AnchorPositionEvaluator.h` | BSD-2-Clause |
| `Style/Builder/Builder` | .hpp | `style/StyleBuilder.h` | BSD-2-Clause |
| `Style/Builder/BuilderChecking` | .hpp | `style/StyleBuilderChecking.h` | BSD-2-Clause |
| `Style/Builder/BuilderCustom` | .hpp | `style/StyleBuilderCustom.h` | BSD-2-Clause |
| `Style/Builder/BuilderState` | .hpp | `style/StyleBuilderState.h` | BSD-2-Clause |
| `Style/Builder/BuilderStateInlines` | .hpp | `style/StyleBuilderStateInlines.h` | BSD-2-Clause |
| `Style/Builder/FontSizeFunctions` | .hpp | `style/StyleFontSizeFunctions.h` | BSD-2-Clause |
| `Style/Calc/Tree` | .cpp, .hpp | `css/calc/CSSCalcTree.cpp`<br>`css/calc/CSSCalcTree.h` | BSD-2-Clause |
| `Style/Calc/TreeConversion` | .cpp, .hpp | `style/calc/StyleCalculationTree+Conversion.cpp`<br>`style/calc/StyleCalculationTree+Conversion.h` | BSD-2-Clause |
| `Style/Calc/TreeCopy` | .cpp, .hpp | `css/calc/CSSCalcTree+Copy.cpp`<br>`css/calc/CSSCalcTree+Copy.h` | BSD-2-Clause |
| `Style/Calc/TreeEvaluation` | .cpp, .hpp | `css/calc/CSSCalcTree+Evaluation.cpp`<br>`css/calc/CSSCalcTree+Evaluation.h` | BSD-2-Clause |
| `Style/Calc/TreeTraversal` | .hpp | `css/calc/CSSCalcTree+Traversal.h` | BSD-2-Clause |
| `Style/Calc/Value` | .cpp, .hpp | `css/calc/CSSCalcValue.cpp`<br>`css/calc/CSSCalcValue.h` | BSD-3-Clause |
| `Style/Calc/ValueMap` | .cpp, .hpp | `style/calc/StyleCalculationValueMap.cpp`<br>`style/calc/StyleCalculationValueMap.h` | BSD-2-Clause |
| `Style/Change` | .cpp, .hpp | `style/StyleChange.cpp`<br>`style/StyleChange.h` | BSD-2-Clause |
| `Style/Computed/ComputedStyleBase+GettersInlines` | .hpp | `style/computed/StyleComputedStyleBase+GettersInlines.h` | BSD-2-Clause |
| `Style/Computed/ComputedStyleBase` | .cpp | `style/computed/StyleComputedStyleBase.cpp` | BSD-2-Clause |
| `Style/Computed/ComputedStyleProperties+GettersCustom` | .cpp | `style/computed/StyleComputedStyleProperties+GettersCustom.cpp` | BSD-2-Clause |
| `Style/Computed/ComputedStyleProperties+GettersCustomInlines` | .hpp | `style/computed/StyleComputedStyleProperties+GettersCustomInlines.h` | BSD-2-Clause |
| `Style/Computed/ComputedStyleProperties+InitialCustom` | .cpp | `style/computed/StyleComputedStyleProperties+InitialCustom.cpp` | BSD-2-Clause |
| `Style/Computed/ComputedStyleProperties+InitialCustomInlines` | .hpp | `style/computed/StyleComputedStyleProperties+InitialCustomInlines.h` | BSD-2-Clause |
| `Style/Computed/ComputedStyleProperties+SettersCustomInlines` | .hpp | `style/computed/StyleComputedStyleProperties+SettersCustomInlines.h` | BSD-2-Clause |
| `Style/ContainerQueryEvaluator` | .cpp, .hpp | `style/ContainerQueryEvaluator.cpp`<br>`style/ContainerQueryEvaluator.h` | BSD-2-Clause |
| `Style/CustomProperty` | .cpp, .hpp | `style/StyleCustomProperty.cpp`<br>`style/StyleCustomProperty.h` | BSD-2-Clause |
| `Style/CustomPropertyRegistry` | .cpp, .hpp | `style/StyleCustomPropertyRegistry.cpp`<br>`style/StyleCustomPropertyRegistry.h` | BSD-2-Clause |
| `Style/DeclarationOrigin` | .hpp | `style/DeclarationOrigin.h` | BSD-2-Clause |
| `Style/Difference` | .cpp, .hpp | `style/StyleDifference.cpp`<br>`style/StyleDifference.h` | BSD-2-Clause |
| `Style/Extractor/Extractor` | .hpp | `style/StyleExtractor.h` | BSD-2-Clause |
| `Style/Extractor/ExtractorCustom` | .hpp | `style/StyleExtractorCustom.h` | BSD-2-Clause |
| `Style/Extractor/ExtractorGenerated` | .hpp | `style/StyleExtractorGenerated.h` | BSD-2-Clause |
| `Style/Extractor/ExtractorState` | .hpp | `style/StyleExtractorState.h` | BSD-2-Clause |
| `Style/InlineTextBoxStyle` | .cpp, .hpp | `style/InlineTextBoxStyle.cpp`<br>`style/InlineTextBoxStyle.h` | BSD-2-Clause |
| `Style/Interpolation/ChangedAnimatableProperties` | .cpp, .hpp | `style/StyleChangedAnimatableProperties.cpp`<br>`style/StyleChangedAnimatableProperties.h` | BSD-2-Clause |
| `Style/Interpolation/ChangedAnimatablePropertiesCustom` | .hpp | `style/StyleChangedAnimatablePropertiesCustom.h` | BSD-2-Clause |
| `Style/Interpolation/Interpolation` | .cpp, .hpp | `style/StyleInterpolation.cpp`<br>`style/StyleInterpolation.h` | BSD-3-Clause |
| `Style/Interpolation/InterpolationClient` | .hpp | `style/StyleInterpolationClient.h` | BSD-2-Clause |
| `Style/Interpolation/InterpolationContext` | .hpp | `style/StyleInterpolationContext.h` | BSD-3-Clause |
| `Style/Interpolation/InterpolationFunctions` | .hpp | `style/StyleInterpolationFunctions.h` | BSD-3-Clause |
| `Style/Interpolation/InterpolationWrapperBase` | .hpp | `style/StyleInterpolationWrapperBase.h` | BSD-3-Clause |
| `Style/Interpolation/InterpolationWrappers` | .hpp | `style/StyleInterpolationWrappers.h` | BSD-3-Clause |
| `Style/Invalidation/AttributeChangeInvalidation` | .cpp, .hpp | `style/AttributeChangeInvalidation.cpp`<br>`style/AttributeChangeInvalidation.h` | BSD-2-Clause |
| `Style/Invalidation/ChildChangeInvalidation` | .cpp, .hpp | `style/ChildChangeInvalidation.cpp`<br>`style/ChildChangeInvalidation.h` | BSD-2-Clause |
| `Style/Invalidation/ClassChangeInvalidation` | .cpp, .hpp | `style/ClassChangeInvalidation.cpp`<br>`style/ClassChangeInvalidation.h` | BSD-2-Clause |
| `Style/Invalidation/HasSelectorFilter` | .cpp, .hpp | `style/HasSelectorFilter.cpp`<br>`style/HasSelectorFilter.h` | BSD-2-Clause |
| `Style/Invalidation/IdChangeInvalidation` | .cpp, .hpp | `style/IdChangeInvalidation.cpp`<br>`style/IdChangeInvalidation.h` | BSD-2-Clause |
| `Style/Invalidation/InvalidationFunctions` | .hpp | `style/StyleInvalidationFunctions.h` | BSD-2-Clause |
| `Style/Invalidation/Invalidator` | .cpp, .hpp | `style/StyleInvalidator.cpp`<br>`style/StyleInvalidator.h` | BSD-2-Clause |
| `Style/Invalidation/PseudoClassChangeInvalidation` | .cpp, .hpp | `style/PseudoClassChangeInvalidation.cpp`<br>`style/PseudoClassChangeInvalidation.h` | BSD-2-Clause |
| `Style/OrderedNamedLinesCollector` | .hpp | `style/StyleOrderedNamedLinesCollector.h` | BSD-2-Clause |
| `Style/PendingResources` | .cpp, .hpp | `style/StylePendingResources.cpp`<br>`style/StylePendingResources.h` | BSD-2-Clause |
| `Style/PropertyCascade` | .cpp, .hpp | `style/PropertyCascade.cpp`<br>`style/PropertyCascade.h` | BSD-2-Clause |
| `Style/PseudoElementIdentifier` | .hpp | `style/PseudoElementIdentifier.h` | BSD-2-Clause |
| `Style/PseudoElementRequest` | .hpp | `style/PseudoElementRequest.h` | BSD-2-Clause |
| `Style/Relations` | .cpp, .hpp | `style/StyleRelations.cpp`<br>`style/StyleRelations.h` | BSD-2-Clause |
| `Style/ResolveForDocument` | .hpp | `style/StyleResolveForDocument.h` | BSD-2-Clause |
| `Style/ResolveForFont` | .cpp, .hpp | `style/StyleResolveForFont.cpp`<br>`style/StyleResolveForFont.h` | BSD-2-Clause |
| `Style/Rules/MatchResult` | .hpp | `style/MatchResult.h` | BSD-2-Clause |
| `Style/Rules/MatchResultCache` | .cpp, .hpp | `style/MatchResultCache.cpp`<br>`style/MatchResultCache.h` | BSD-2-Clause |
| `Style/Rules/MatchedDeclarationsCache` | .hpp | `style/MatchedDeclarationsCache.h` | BSD-2-Clause |
| `Style/Rules/PropertyAllowlist` | .cpp, .hpp | `style/PropertyAllowlist.cpp`<br>`style/PropertyAllowlist.h` | BSD-2-Clause |
| `Style/Rules/SelectorMatchingState` | .hpp | `style/SelectorMatchingState.h` | BSD-2-Clause |
| `Style/Rules/StyleSheetContentsCache` | .cpp, .hpp | `style/StyleSheetContentsCache.cpp`<br>`style/StyleSheetContentsCache.h` | BSD-2-Clause |
| `Style/Scope/NameScope` | .cpp, .hpp | `style/StyleNameScope.cpp`<br>`style/StyleNameScope.h` | BSD-2-Clause |
| `Style/Scope/ResolvedScopedName` | .cpp, .hpp | `style/ResolvedScopedName.cpp`<br>`style/ResolvedScopedName.h` | BSD-2-Clause |
| `Style/Scope/ScopeIdentifier` | .hpp | `style/StyleScopeIdentifier.h` | BSD-2-Clause |
| `Style/Scope/ScopeOrdinal` | .hpp | `style/StyleScopeOrdinal.h` | BSD-2-Clause |
| `Style/Scope/ScopedName` | .cpp, .hpp | `style/ScopedName.cpp`<br>`style/ScopedName.h` | BSD-2-Clause |
| `Style/StyleAppearance` | .cpp, .hpp | `platform/StyleAppearance.cpp`<br>`platform/StyleAppearance.h` | BSD-2-Clause |
| `Style/Styleable` | .cpp, .hpp | `style/Styleable.cpp`<br>`style/Styleable.h` | BSD-2-Clause |
| `Style/TouchAction` | .hpp | `platform/TouchAction.h` | BSD-2-Clause |
| `Style/TransformResolver` | .cpp, .hpp | `style/StyleTransformResolver.cpp`<br>`style/StyleTransformResolver.h` | BSD-2-Clause |
| `Style/TreeResolutionState` | .hpp | `style/TreeResolutionState.h` | BSD-2-Clause |
| `Style/TreeResolver` | .hpp | `style/StyleTreeResolver.h` | BSD-2-Clause |
| `Style/Update` | .cpp, .hpp | `style/StyleUpdate.cpp`<br>`style/StyleUpdate.h` | BSD-2-Clause |
| `Style/Validity` | .hpp | `style/StyleValidity.h` | BSD-2-Clause |
| `Style/Values/Align/AlignContent` | .cpp, .hpp | `style/values/align/StyleAlignContent.cpp`<br>`style/values/align/StyleAlignContent.h` | BSD-2-Clause |
| `Style/Values/Align/AlignItems` | .cpp, .hpp | `style/values/align/StyleAlignItems.cpp`<br>`style/values/align/StyleAlignItems.h` | BSD-2-Clause |
| `Style/Values/Align/AlignSelf` | .cpp, .hpp | `style/values/align/StyleAlignSelf.cpp`<br>`style/values/align/StyleAlignSelf.h` | BSD-2-Clause |
| `Style/Values/Align/BaselineAlignmentPreference` | .hpp | `style/values/align/StyleBaselineAlignmentPreference.h` | BSD-2-Clause |
| `Style/Values/Align/GapGutter` | .hpp | `style/values/align/StyleGapGutter.h` | BSD-2-Clause |
| `Style/Values/Align/JustifyContent` | .cpp, .hpp | `style/values/align/StyleJustifyContent.cpp`<br>`style/values/align/StyleJustifyContent.h` | BSD-2-Clause |
| `Style/Values/Align/JustifyItems` | .cpp, .hpp | `style/values/align/StyleJustifyItems.cpp`<br>`style/values/align/StyleJustifyItems.h` | BSD-2-Clause |
| `Style/Values/Align/JustifySelf` | .cpp, .hpp | `style/values/align/StyleJustifySelf.cpp`<br>`style/values/align/StyleJustifySelf.h` | BSD-2-Clause |
| `Style/Values/Align/LegacyPosition` | .hpp | `style/values/align/StyleLegacyPosition.h` | BSD-2-Clause |
| `Style/Values/Align/OverflowPosition` | .hpp | `style/values/align/StyleOverflowPosition.h` | BSD-2-Clause |
| `Style/Values/AnchorPosition/AnchorName` | .hpp | `style/values/anchor-position/StyleAnchorName.h` | BSD-2-Clause |
| `Style/Values/AnchorPosition/PositionAnchor` | .hpp | `style/values/anchor-position/StylePositionAnchor.h` | BSD-2-Clause |
| `Style/Values/AnchorPosition/PositionArea` | .cpp, .hpp | `style/values/anchor-position/StylePositionArea.cpp`<br>`style/values/anchor-position/StylePositionArea.h` | BSD-2-Clause |
| `Style/Values/AnchorPosition/PositionAreaAxis` | .cpp, .hpp | `style/values/anchor-position/StylePositionAreaAxis.cpp`<br>`style/values/anchor-position/StylePositionAreaAxis.h` | BSD-2-Clause |
| `Style/Values/AnchorPosition/PositionAreaSelf` | .cpp, .hpp | `style/values/anchor-position/StylePositionAreaSelf.cpp`<br>`style/values/anchor-position/StylePositionAreaSelf.h` | BSD-2-Clause |
| `Style/Values/AnchorPosition/PositionAreaSpan` | .cpp, .hpp | `style/values/anchor-position/StylePositionAreaSpan.cpp`<br>`style/values/anchor-position/StylePositionAreaSpan.h` | BSD-2-Clause |
| `Style/Values/AnchorPosition/PositionAreaTrack` | .cpp, .hpp | `style/values/anchor-position/StylePositionAreaTrack.cpp`<br>`style/values/anchor-position/StylePositionAreaTrack.h` | BSD-2-Clause |
| `Style/Values/AnchorPosition/PositionTryFallback` | .cpp, .hpp | `style/values/anchor-position/StylePositionTryFallback.cpp`<br>`style/values/anchor-position/StylePositionTryFallback.h` | BSD-2-Clause |
| `Style/Values/AnchorPosition/PositionTryFallbackTactic` | .hpp | `style/values/anchor-position/StylePositionTryFallbackTactic.h` | BSD-2-Clause |
| `Style/Values/AnchorPosition/PositionTryFallbacks` | .hpp | `style/values/anchor-position/StylePositionTryFallbacks.h` | BSD-2-Clause |
| `Style/Values/AnchorPosition/PositionVisibility` | .hpp | `style/values/anchor-position/StylePositionVisibility.h` | BSD-2-Clause |
| `Style/Values/Animations/Animation` | .cpp, .hpp | `style/values/animations/StyleAnimation.cpp`<br>`style/values/animations/StyleAnimation.h` | BSD-2-Clause |
| `Style/Values/Animations/Animations` | .hpp | `style/values/animations/StyleAnimations.h` | BSD-2-Clause |
| `Style/Values/Animations/SingleAnimationDelay` | .hpp | `style/values/animations/StyleSingleAnimationDelay.h` | BSD-2-Clause |
| `Style/Values/Animations/SingleAnimationDuration` | .hpp | `style/values/animations/StyleSingleAnimationDuration.h` | BSD-2-Clause |
| `Style/Values/Animations/SingleAnimationIterationCount` | .hpp | `style/values/animations/StyleSingleAnimationIterationCount.h` | BSD-2-Clause |
| `Style/Values/Animations/SingleAnimationName` | .cpp, .hpp | `style/values/animations/StyleSingleAnimationName.cpp`<br>`style/values/animations/StyleSingleAnimationName.h` | BSD-2-Clause |
| `Style/Values/Animations/SingleAnimationRange` | .cpp, .hpp | `style/values/animations/StyleSingleAnimationRange.cpp`<br>`style/values/animations/StyleSingleAnimationRange.h` | BSD-2-Clause |
| `Style/Values/Animations/SingleAnimationRangeName` | .cpp, .hpp | `style/values/animations/StyleSingleAnimationRangeName.cpp`<br>`style/values/animations/StyleSingleAnimationRangeName.h` | BSD-2-Clause |
| `Style/Values/Animations/SingleAnimationTimeline` | .cpp, .hpp | `style/values/animations/StyleSingleAnimationTimeline.cpp`<br>`style/values/animations/StyleSingleAnimationTimeline.h` | BSD-2-Clause |
| `Style/Values/Backgrounds/BackgroundLayers` | .hpp | `style/values/backgrounds/StyleBackgroundLayers.h` | BSD-2-Clause |
| `Style/Values/Backgrounds/BackgroundSize` | .cpp, .hpp | `style/values/backgrounds/StyleBackgroundSize.cpp`<br>`style/values/backgrounds/StyleBackgroundSize.h` | BSD-2-Clause |
| `Style/Values/Backgrounds/BorderImage` | .cpp | `style/values/backgrounds/StyleBorderImage.cpp` | BSD-2-Clause |
| `Style/Values/Backgrounds/BorderImageOutset` | .cpp, .hpp | `style/values/backgrounds/StyleBorderImageOutset.cpp`<br>`style/values/backgrounds/StyleBorderImageOutset.h` | BSD-2-Clause |
| `Style/Values/Backgrounds/BorderImageRepeat` | .cpp, .hpp | `style/values/backgrounds/StyleBorderImageRepeat.cpp`<br>`style/values/backgrounds/StyleBorderImageRepeat.h` | BSD-2-Clause |
| `Style/Values/Backgrounds/BorderImageSlice` | .cpp, .hpp | `style/values/backgrounds/StyleBorderImageSlice.cpp`<br>`style/values/backgrounds/StyleBorderImageSlice.h` | BSD-2-Clause |
| `Style/Values/Backgrounds/BorderImageSource` | .cpp, .hpp | `style/values/backgrounds/StyleBorderImageSource.cpp`<br>`style/values/backgrounds/StyleBorderImageSource.h` | BSD-2-Clause |
| `Style/Values/Backgrounds/BorderImageWidth` | .cpp, .hpp | `style/values/backgrounds/StyleBorderImageWidth.cpp`<br>`style/values/backgrounds/StyleBorderImageWidth.h` | BSD-2-Clause |
| `Style/Values/Backgrounds/LineWidth` | .cpp, .hpp | `style/values/backgrounds/StyleLineWidth.cpp`<br>`style/values/backgrounds/StyleLineWidth.h` | BSD-2-Clause |
| `Style/Values/Backgrounds/RepeatStyle` | .cpp, .hpp | `style/values/backgrounds/StyleRepeatStyle.cpp`<br>`style/values/backgrounds/StyleRepeatStyle.h` | BSD-2-Clause |
| `Style/Values/Borders/BorderRadius` | .cpp, .hpp | `style/values/borders/StyleBorderRadius.cpp`<br>`style/values/borders/StyleBorderRadius.h` | BSD-2-Clause |
| `Style/Values/Borders/BoxShadow` | .cpp, .hpp | `style/values/borders/StyleBoxShadow.cpp`<br>`style/values/borders/StyleBoxShadow.h` | BSD-2-Clause |
| `Style/Values/Borders/CornerShapeValue` | .cpp, .hpp | `style/values/borders/StyleCornerShapeValue.cpp`<br>`style/values/borders/StyleCornerShapeValue.h` | BSD-2-Clause |
| `Style/Values/Borders/Shadow` | .hpp | `style/values/borders/StyleShadow.h` | BSD-2-Clause |
| `Style/Values/Borders/ShadowInterpolation` | .hpp | `style/values/borders/StyleShadowInterpolation.h` | BSD-2-Clause |
| `Style/Values/Box/Margin` | .cpp, .hpp | `style/values/box/StyleMargin.cpp`<br>`style/values/box/StyleMargin.h` | BSD-2-Clause |
| `Style/Values/Box/MarginTrim` | .cpp, .hpp | `style/values/box/StyleMarginTrim.cpp`<br>`style/values/box/StyleMarginTrim.h` | BSD-2-Clause |
| `Style/Values/Box/Padding` | .hpp | `style/values/box/StylePadding.h` | BSD-2-Clause |
| `Style/Values/Break/Orphans` | .cpp, .hpp | `style/values/break/StyleOrphans.cpp`<br>`style/values/break/StyleOrphans.h` | BSD-2-Clause |
| `Style/Values/Break/Widows` | .cpp, .hpp | `style/values/break/StyleWidows.cpp`<br>`style/values/break/StyleWidows.h` | BSD-2-Clause |
| `Style/Values/CSS2/ZIndex` | .cpp, .hpp | `style/values/css2/StyleZIndex.cpp`<br>`style/values/css2/StyleZIndex.h` | BSD-2-Clause |
| `Style/Values/CSSValueConversion` | .hpp | `style/values/StyleValueTypes+CSSValueConversion.h` | BSD-2-Clause |
| `Style/Values/Color/AbsoluteColor` | .hpp | `style/values/color/StyleAbsoluteColor.h` | BSD-2-Clause |
| `Style/Values/Color/Color` | .cpp, .hpp | `style/values/color/StyleColor.cpp`<br>`style/values/color/StyleColor.h` | BSD-3-Clause |
| `Style/Values/Color/ColorLayers` | .cpp, .hpp | `style/values/color/StyleColorLayers.cpp`<br>`style/values/color/StyleColorLayers.h` | BSD-2-Clause |
| `Style/Values/Color/ColorMix` | .cpp, .hpp | `style/values/color/StyleColorMix.cpp`<br>`style/values/color/StyleColorMix.h` | BSD-2-Clause |
| `Style/Values/Color/ColorOptions` | .hpp | `style/values/color/StyleColorOptions.h` | BSD-2-Clause |
| `Style/Values/Color/ColorResolutionState` | .hpp | `style/values/color/StyleColorResolutionState.h` | BSD-2-Clause |
| `Style/Values/Color/ContrastColor` | .cpp, .hpp | `style/values/color/StyleContrastColor.cpp`<br>`style/values/color/StyleContrastColor.h` | BSD-2-Clause |
| `Style/Values/Color/CurrentColor` | .cpp, .hpp | `style/values/color/StyleCurrentColor.cpp`<br>`style/values/color/StyleCurrentColor.h` | BSD-2-Clause |
| `Style/Values/Color/DynamicRangeLimit` | .cpp, .hpp | `style/values/color/StyleDynamicRangeLimit.cpp`<br>`style/values/color/StyleDynamicRangeLimit.h` | BSD-2-Clause |
| `Style/Values/Color/DynamicRangeLimitMix` | .cpp, .hpp | `style/values/color/StyleDynamicRangeLimitMix.cpp`<br>`style/values/color/StyleDynamicRangeLimitMix.h` | BSD-2-Clause |
| `Style/Values/Color/HexColor` | .cpp, .hpp | `style/values/color/StyleHexColor.cpp`<br>`style/values/color/StyleHexColor.h` | BSD-2-Clause |
| `Style/Values/Color/KeywordColor` | .cpp, .hpp | `style/values/color/StyleKeywordColor.cpp`<br>`style/values/color/StyleKeywordColor.h` | BSD-2-Clause |
| `Style/Values/Color/LightDarkColor` | .cpp, .hpp | `style/values/color/StyleLightDarkColor.cpp`<br>`style/values/color/StyleLightDarkColor.h` | BSD-2-Clause |
| `Style/Values/Color/Opacity` | .cpp, .hpp | `style/values/color/StyleOpacity.cpp`<br>`style/values/color/StyleOpacity.h` | BSD-2-Clause |
| `Style/Values/Color/RelativeColor` | .hpp | `style/values/color/StyleRelativeColor.h` | BSD-2-Clause |
| `Style/Values/Color/ResolvedColor` | .cpp, .hpp | `style/values/color/StyleResolvedColor.cpp`<br>`style/values/color/StyleResolvedColor.h` | BSD-2-Clause |
| `Style/Values/ColorAdjust/ColorScheme` | .cpp, .hpp | `style/values/color-adjust/StyleColorScheme.cpp`<br>`style/values/color-adjust/StyleColorScheme.h` | BSD-2-Clause |
| `Style/Values/Contain/Contain` | .cpp, .hpp | `style/values/contain/StyleContain.cpp`<br>`style/values/contain/StyleContain.h` | BSD-2-Clause |
| `Style/Values/Contain/ContainerName` | .hpp | `style/values/contain/StyleContainerName.h` | BSD-2-Clause |
| `Style/Values/Content/Content` | .cpp, .hpp | `style/values/content/StyleContent.cpp`<br>`style/values/content/StyleContent.h` | BSD-2-Clause |
| `Style/Values/CounterStyles/CounterStyle` | .cpp, .hpp | `style/values/counter-styles/StyleCounterStyle.cpp`<br>`style/values/counter-styles/StyleCounterStyle.h` | BSD-2-Clause |
| `Style/Values/Display/Order` | .hpp | `style/values/display/StyleOrder.h` | BSD-2-Clause |
| `Style/Values/Easing/CubicBezierEasingFunction` | .cpp, .hpp | `style/values/easing/StyleCubicBezierEasingFunction.cpp`<br>`style/values/easing/StyleCubicBezierEasingFunction.h` | BSD-2-Clause |
| `Style/Values/Easing/EasingFunction` | .cpp, .hpp | `style/values/easing/StyleEasingFunction.cpp`<br>`style/values/easing/StyleEasingFunction.h` | BSD-2-Clause |
| `Style/Values/Easing/LinearEasingFunction` | .cpp, .hpp | `style/values/easing/StyleLinearEasingFunction.cpp`<br>`style/values/easing/StyleLinearEasingFunction.h` | BSD-2-Clause |
| `Style/Values/Easing/SpringEasingFunction` | .cpp, .hpp | `style/values/easing/StyleSpringEasingFunction.cpp`<br>`style/values/easing/StyleSpringEasingFunction.h` | BSD-2-Clause |
| `Style/Values/Easing/StepsEasingFunction` | .cpp, .hpp | `style/values/easing/StyleStepsEasingFunction.cpp`<br>`style/values/easing/StyleStepsEasingFunction.h` | BSD-2-Clause |
| `Style/Values/FillStroke/StrokeMiterlimit` | .hpp | `style/values/fill-stroke/StyleStrokeMiterlimit.h` | BSD-2-Clause |
| `Style/Values/FillStroke/StrokeWidth` | .cpp, .hpp | `style/values/fill-stroke/StyleStrokeWidth.cpp`<br>`style/values/fill-stroke/StyleStrokeWidth.h` | BSD-2-Clause |
| `Style/Values/FilterEffects/BlurFunction` | .cpp, .hpp | `style/values/filter-effects/StyleBlurFunction.cpp`<br>`style/values/filter-effects/StyleBlurFunction.h` | BSD-2-Clause |
| `Style/Values/FilterEffects/BrightnessFunction` | .cpp, .hpp | `style/values/filter-effects/StyleBrightnessFunction.cpp`<br>`style/values/filter-effects/StyleBrightnessFunction.h` | BSD-2-Clause |
| `Style/Values/FilterEffects/ContrastFunction` | .cpp, .hpp | `style/values/filter-effects/StyleContrastFunction.cpp`<br>`style/values/filter-effects/StyleContrastFunction.h` | BSD-2-Clause |
| `Style/Values/FilterEffects/DropShadowFunction` | .cpp, .hpp | `style/values/filter-effects/StyleDropShadowFunction.cpp`<br>`style/values/filter-effects/StyleDropShadowFunction.h` | BSD-2-Clause |
| `Style/Values/FilterEffects/Filter` | .cpp, .hpp | `style/values/filter-effects/StyleFilter.cpp`<br>`style/values/filter-effects/StyleFilter.h` | BSD-2-Clause |
| `Style/Values/FilterEffects/FilterInterpolation` | .hpp | `style/values/filter-effects/StyleFilterInterpolation.h` | BSD-2-Clause |
| `Style/Values/FilterEffects/FilterReference` | .cpp, .hpp | `style/values/filter-effects/StyleFilterReference.cpp`<br>`style/values/filter-effects/StyleFilterReference.h` | BSD-2-Clause |
| `Style/Values/FilterEffects/GrayscaleFunction` | .cpp, .hpp | `style/values/filter-effects/StyleGrayscaleFunction.cpp`<br>`style/values/filter-effects/StyleGrayscaleFunction.h` | BSD-2-Clause |
| `Style/Values/FilterEffects/HueRotateFunction` | .cpp, .hpp | `style/values/filter-effects/StyleHueRotateFunction.cpp`<br>`style/values/filter-effects/StyleHueRotateFunction.h` | BSD-2-Clause |
| `Style/Values/FilterEffects/InvertFunction` | .cpp, .hpp | `style/values/filter-effects/StyleInvertFunction.cpp`<br>`style/values/filter-effects/StyleInvertFunction.h` | BSD-2-Clause |
| `Style/Values/FilterEffects/OpacityFunction` | .cpp, .hpp | `style/values/filter-effects/StyleOpacityFunction.cpp`<br>`style/values/filter-effects/StyleOpacityFunction.h` | BSD-2-Clause |
| `Style/Values/FilterEffects/SaturateFunction` | .cpp, .hpp | `style/values/filter-effects/StyleSaturateFunction.cpp`<br>`style/values/filter-effects/StyleSaturateFunction.h` | BSD-2-Clause |
| `Style/Values/FilterEffects/SepiaFunction` | .cpp, .hpp | `style/values/filter-effects/StyleSepiaFunction.cpp`<br>`style/values/filter-effects/StyleSepiaFunction.h` | BSD-2-Clause |
| `Style/Values/Flexbox/FlexBasis` | .cpp, .hpp | `style/values/flexbox/StyleFlexBasis.cpp`<br>`style/values/flexbox/StyleFlexBasis.h` | BSD-2-Clause |
| `Style/Values/Flexbox/FlexGrow` | .hpp | `style/values/flexbox/StyleFlexGrow.h` | BSD-2-Clause |
| `Style/Values/Flexbox/FlexShrink` | .hpp | `style/values/flexbox/StyleFlexShrink.h` | BSD-2-Clause |
| `Style/Values/Fonts/FontFamily` | .cpp, .hpp | `style/values/fonts/StyleFontFamily.cpp`<br>`style/values/fonts/StyleFontFamily.h` | BSD-2-Clause |
| `Style/Values/Fonts/FontFamilyName` | .cpp, .hpp | `style/values/fonts/StyleFontFamilyName.cpp`<br>`style/values/fonts/StyleFontFamilyName.h` | BSD-2-Clause |
| `Style/Values/Fonts/FontFeatureSettings` | .cpp, .hpp | `style/values/fonts/StyleFontFeatureSettings.cpp`<br>`style/values/fonts/StyleFontFeatureSettings.h` | BSD-2-Clause |
| `Style/Values/Fonts/FontOpentypeTag` | .hpp | `style/values/fonts/StyleFontOpentypeTag.h` | BSD-2-Clause |
| `Style/Values/Fonts/FontPalette` | .cpp, .hpp | `style/values/fonts/StyleFontPalette.cpp`<br>`style/values/fonts/StyleFontPalette.h` | BSD-2-Clause |
| `Style/Values/Fonts/FontSizeAdjust` | .cpp, .hpp | `style/values/fonts/StyleFontSizeAdjust.cpp`<br>`style/values/fonts/StyleFontSizeAdjust.h` | BSD-2-Clause |
| `Style/Values/Fonts/FontStyle` | .cpp, .hpp | `style/values/fonts/StyleFontStyle.cpp`<br>`style/values/fonts/StyleFontStyle.h` | BSD-2-Clause |
| `Style/Values/Fonts/FontVariantAlternates` | .cpp, .hpp | `style/values/fonts/StyleFontVariantAlternates.cpp`<br>`style/values/fonts/StyleFontVariantAlternates.h` | BSD-2-Clause |
| `Style/Values/Fonts/FontVariantEastAsian` | .cpp, .hpp | `style/values/fonts/StyleFontVariantEastAsian.cpp`<br>`style/values/fonts/StyleFontVariantEastAsian.h` | BSD-2-Clause |
| `Style/Values/Fonts/FontVariantLigatures` | .cpp, .hpp | `style/values/fonts/StyleFontVariantLigatures.cpp`<br>`style/values/fonts/StyleFontVariantLigatures.h` | BSD-2-Clause |
| `Style/Values/Fonts/FontVariantNumeric` | .cpp, .hpp | `style/values/fonts/StyleFontVariantNumeric.cpp`<br>`style/values/fonts/StyleFontVariantNumeric.h` | BSD-2-Clause |
| `Style/Values/Fonts/FontVariationSettings` | .cpp, .hpp | `style/values/fonts/StyleFontVariationSettings.cpp`<br>`style/values/fonts/StyleFontVariationSettings.h` | BSD-2-Clause |
| `Style/Values/Fonts/FontWeight` | .cpp, .hpp | `style/values/fonts/StyleFontWeight.cpp`<br>`style/values/fonts/StyleFontWeight.h` | BSD-2-Clause |
| `Style/Values/Fonts/FontWidth` | .cpp, .hpp | `style/values/fonts/StyleFontWidth.cpp`<br>`style/values/fonts/StyleFontWidth.h` | BSD-2-Clause |
| `Style/Values/Grid/GridAutoFlow` | .cpp, .hpp | `style/values/grid/StyleGridAutoFlow.cpp`<br>`style/values/grid/StyleGridAutoFlow.h` | BSD-2-Clause |
| `Style/Values/Grid/GridNamedAreaMap` | .hpp | `style/values/grid/StyleGridNamedAreaMap.h` | BSD-2-Clause |
| `Style/Values/Grid/GridNamedLinesMap` | .hpp | `style/values/grid/StyleGridNamedLinesMap.h` | BSD-2-Clause |
| `Style/Values/Grid/GridOrderedNamedLinesMap` | .hpp | `style/values/grid/StyleGridOrderedNamedLinesMap.h` | BSD-2-Clause |
| `Style/Values/Grid/GridPosition` | .cpp, .hpp | `style/values/grid/StyleGridPosition.cpp`<br>`style/values/grid/StyleGridPosition.h` | BSD-3-Clause |
| `Style/Values/Grid/GridPositionSide` | .hpp | `style/values/grid/StyleGridPositionSide.h` | BSD-2-Clause |
| `Style/Values/Grid/GridPositionsResolver` | .cpp, .hpp | `style/values/grid/StyleGridPositionsResolver.cpp`<br>`style/values/grid/StyleGridPositionsResolver.h` | BSD-3-Clause |
| `Style/Values/Grid/GridTemplateAreas` | .cpp, .hpp | `style/values/grid/StyleGridTemplateAreas.cpp`<br>`style/values/grid/StyleGridTemplateAreas.h` | BSD-2-Clause |
| `Style/Values/Grid/GridTemplateList` | .cpp, .hpp | `style/values/grid/StyleGridTemplateList.cpp`<br>`style/values/grid/StyleGridTemplateList.h` | BSD-2-Clause |
| `Style/Values/Grid/GridTrackBreadth` | .cpp, .hpp | `style/values/grid/StyleGridTrackBreadth.cpp`<br>`style/values/grid/StyleGridTrackBreadth.h` | BSD-2-Clause / BSD-3-Clause |
| `Style/Values/Grid/GridTrackSize` | .cpp, .hpp | `style/values/grid/StyleGridTrackSize.cpp`<br>`style/values/grid/StyleGridTrackSize.h` | BSD-2-Clause / BSD-3-Clause |
| `Style/Values/Grid/GridTrackSizes` | .cpp, .hpp | `style/values/grid/StyleGridTrackSizes.cpp`<br>`style/values/grid/StyleGridTrackSizes.h` | BSD-2-Clause |
| `Style/Values/Grid/GridTrackSizingDirection` | .hpp | `style/values/grid/StyleGridTrackSizingDirection.h` | BSD-2-Clause |
| `Style/Values/Grid/ItemTolerance` | .cpp, .hpp | `style/values/grid/StyleItemTolerance.cpp`<br>`style/values/grid/StyleItemTolerance.h` | BSD-2-Clause |
| `Style/Values/Images/Gradient` | .cpp, .hpp | `style/values/images/StyleGradient.cpp`<br>`style/values/images/StyleGradient.h` | BSD-2-Clause |
| `Style/Values/Images/ImageOrNone` | .cpp, .hpp | `style/values/images/StyleImageOrNone.cpp`<br>`style/values/images/StyleImageOrNone.h` | BSD-2-Clause |
| `Style/Values/Images/ImageOrientation` | .cpp, .hpp | `style/values/images/StyleImageOrientation.cpp`<br>`style/values/images/StyleImageOrientation.h` | BSD-2-Clause |
| `Style/Values/Images/ImageWrapper` | .cpp, .hpp | `style/values/images/StyleImageWrapper.cpp`<br>`style/values/images/StyleImageWrapper.h` | BSD-2-Clause |
| `Style/Values/Images/ObjectPosition` | .hpp | `style/values/images/StyleObjectPosition.h` | BSD-2-Clause |
| `Style/Values/Inline/LineFitEdge` | .hpp | `style/values/inline/StyleLineFitEdge.h` | BSD-2-Clause |
| `Style/Values/Inline/LineHeight` | .cpp, .hpp | `style/values/inline/StyleLineHeight.cpp`<br>`style/values/inline/StyleLineHeight.h` | BSD-2-Clause |
| `Style/Values/Inline/TextBoxEdge` | .hpp | `style/values/inline/StyleTextBoxEdge.h` | BSD-2-Clause |
| `Style/Values/Inline/TextEdge` | .hpp | `style/values/inline/StyleTextEdge.h` | BSD-2-Clause |
| `Style/Values/Inline/TextEdgeCSSValueConversion` | .hpp | `style/values/inline/StyleTextEdge+CSSValueConversion.h` | BSD-2-Clause |
| `Style/Values/Inline/VerticalAlign` | .cpp, .hpp | `style/values/inline/StyleVerticalAlign.cpp`<br>`style/values/inline/StyleVerticalAlign.h` | BSD-2-Clause |
| `Style/Values/Inline/WebKitInitialLetter` | .cpp, .hpp | `style/values/inline/StyleWebKitInitialLetter.cpp`<br>`style/values/inline/StyleWebKitInitialLetter.h` | BSD-2-Clause |
| `Style/Values/Lists/ListStyleType` | .cpp, .hpp | `style/values/lists/StyleListStyleType.cpp`<br>`style/values/lists/StyleListStyleType.h` | BSD-2-Clause |
| `Style/Values/Masking/Clip` | .cpp, .hpp | `style/values/masking/StyleClip.cpp`<br>`style/values/masking/StyleClip.h` | BSD-2-Clause |
| `Style/Values/Masking/ClipPath` | .cpp, .hpp | `style/values/masking/StyleClipPath.cpp`<br>`style/values/masking/StyleClipPath.h` | BSD-2-Clause |
| `Style/Values/Masking/MaskBorder` | .cpp | `style/values/masking/StyleMaskBorder.cpp` | BSD-2-Clause |
| `Style/Values/Masking/MaskBorderOutset` | .cpp, .hpp | `style/values/masking/StyleMaskBorderOutset.cpp`<br>`style/values/masking/StyleMaskBorderOutset.h` | BSD-2-Clause |
| `Style/Values/Masking/MaskBorderRepeat` | .cpp, .hpp | `style/values/masking/StyleMaskBorderRepeat.cpp`<br>`style/values/masking/StyleMaskBorderRepeat.h` | BSD-2-Clause |
| `Style/Values/Masking/MaskBorderSlice` | .cpp, .hpp | `style/values/masking/StyleMaskBorderSlice.cpp`<br>`style/values/masking/StyleMaskBorderSlice.h` | BSD-2-Clause |
| `Style/Values/Masking/MaskBorderSource` | .cpp, .hpp | `style/values/masking/StyleMaskBorderSource.cpp`<br>`style/values/masking/StyleMaskBorderSource.h` | BSD-2-Clause |
| `Style/Values/Masking/MaskBorderWidth` | .cpp, .hpp | `style/values/masking/StyleMaskBorderWidth.cpp`<br>`style/values/masking/StyleMaskBorderWidth.h` | BSD-2-Clause |
| `Style/Values/Masking/MaskLayers` | .hpp | `style/values/masking/StyleMaskLayers.h` | BSD-2-Clause |
| `Style/Values/Masking/MaskMode` | .cpp, .hpp | `style/values/masking/StyleMaskMode.cpp`<br>`style/values/masking/StyleMaskMode.h` | BSD-2-Clause |
| `Style/Values/Math/MathDepth` | .cpp, .hpp | `style/values/math/StyleMathDepth.cpp`<br>`style/values/math/StyleMathDepth.h` | BSD-2-Clause |
| `Style/Values/Motion/OffsetAnchor` | .cpp, .hpp | `style/values/motion/StyleOffsetAnchor.cpp`<br>`style/values/motion/StyleOffsetAnchor.h` | BSD-2-Clause |
| `Style/Values/Motion/OffsetDistance` | .hpp | `style/values/motion/StyleOffsetDistance.h` | BSD-2-Clause |
| `Style/Values/Motion/OffsetPath` | .cpp, .hpp | `style/values/motion/StyleOffsetPath.cpp`<br>`style/values/motion/StyleOffsetPath.h` | BSD-2-Clause |
| `Style/Values/Motion/OffsetPosition` | .cpp, .hpp | `style/values/motion/StyleOffsetPosition.cpp`<br>`style/values/motion/StyleOffsetPosition.h` | BSD-2-Clause |
| `Style/Values/Motion/OffsetRotate` | .cpp, .hpp | `style/values/motion/StyleOffsetRotate.cpp`<br>`style/values/motion/StyleOffsetRotate.h` | BSD-2-Clause |
| `Style/Values/Motion/RayFunction` | .cpp, .hpp | `style/values/motion/StyleRayFunction.cpp`<br>`style/values/motion/StyleRayFunction.h` | BSD-2-Clause |
| `Style/Values/Multicol/ColumnCount` | .cpp, .hpp | `style/values/multicol/StyleColumnCount.cpp`<br>`style/values/multicol/StyleColumnCount.h` | BSD-2-Clause |
| `Style/Values/Multicol/ColumnWidth` | .cpp, .hpp | `style/values/multicol/StyleColumnWidth.cpp`<br>`style/values/multicol/StyleColumnWidth.h` | BSD-2-Clause |
| `Style/Values/NonStandard/WebKitBorderSpacing` | .hpp | `style/values/non-standard/StyleWebKitBorderSpacing.h` | BSD-2-Clause |
| `Style/Values/NonStandard/WebKitLineBoxContain` | .hpp | `style/values/non-standard/StyleWebKitLineBoxContain.h` | BSD-2-Clause |
| `Style/Values/NonStandard/WebKitLocale` | .cpp, .hpp | `style/values/non-standard/StyleWebKitLocale.cpp`<br>`style/values/non-standard/StyleWebKitLocale.h` | BSD-2-Clause |
| `Style/Values/Overflow/BlockEllipsis` | .cpp, .hpp | `style/values/overflow/StyleBlockEllipsis.cpp`<br>`style/values/overflow/StyleBlockEllipsis.h` | BSD-2-Clause |
| `Style/Values/Overflow/MaximumLines` | .hpp | `style/values/overflow/StyleMaximumLines.h` | BSD-2-Clause |
| `Style/Values/Overflow/ScrollBehavior` | .hpp | `style/values/overflow/StyleScrollBehavior.h` | BSD-2-Clause |
| `Style/Values/Overflow/ScrollbarGutter` | .cpp, .hpp | `style/values/overflow/StyleScrollbarGutter.cpp`<br>`style/values/overflow/StyleScrollbarGutter.h` | BSD-2-Clause |
| `Style/Values/PointerEvents/TouchAction` | .cpp, .hpp | `style/values/pointerevents/StyleTouchAction.cpp`<br>`style/values/pointerevents/StyleTouchAction.h` | BSD-2-Clause |
| `Style/Values/Position/Inset` | .hpp | `style/values/position/StyleInset.h` | BSD-2-Clause |
| `Style/Values/Primitives/Concepts` | .hpp | `style/values/primitives/StylePrimitiveNumericConcepts.h` | BSD-2-Clause |
| `Style/Values/Primitives/CoordinatedValueList` | .hpp | `style/values/primitives/StyleCoordinatedValueList.h` | BSD-2-Clause |
| `Style/Values/Primitives/CoordinatedValueListValue` | .hpp | `style/values/primitives/StyleCoordinatedValueListValue.h` | BSD-2-Clause |
| `Style/Values/Primitives/KeywordCSSValueConversion` | .hpp | `style/values/primitives/StylePrimitiveKeyword+CSSValueConversion.h` | BSD-2-Clause |
| `Style/Values/Primitives/KeywordCSSValueCreation` | .hpp | `style/values/primitives/StylePrimitiveKeyword+CSSValueCreation.h` | BSD-2-Clause |
| `Style/Values/Primitives/KeywordSerialisation` | .hpp | `style/values/primitives/StylePrimitiveKeyword+Serialization.h` | BSD-2-Clause |
| `Style/Values/Primitives/KeywordValueRepresentationNeeded` | .hpp | `style/values/primitives/StylePrimitiveKeyword+ValueRepresentationNeeded.h` | BSD-2-Clause |
| `Style/Values/Primitives/LengthResolution` | .cpp, .hpp | `style/values/primitives/StyleLengthResolution.cpp`<br>`style/values/primitives/StyleLengthResolution.h` | BSD-2-Clause |
| `Style/Values/Primitives/LengthWrapper` | .hpp | `style/values/primitives/StyleLengthWrapper.h` | BSD-2-Clause |
| `Style/Values/Primitives/LengthWrapperBlending` | .hpp | `style/values/primitives/StyleLengthWrapper+Blending.h` | BSD-2-Clause |
| `Style/Values/Primitives/LengthWrapperCSSValueConversion` | .hpp | `style/values/primitives/StyleLengthWrapper+CSSValueConversion.h` | BSD-2-Clause |
| `Style/Values/Primitives/LengthWrapperData` | .cpp, .hpp | `style/values/primitives/StyleLengthWrapperData.cpp`<br>`style/values/primitives/StyleLengthWrapperData.h` | BSD-2-Clause |
| `Style/Values/Primitives/LengthWrapperDeprecatedCSSValueConversion` | .cpp, .hpp | `style/values/primitives/StyleLengthWrapper+DeprecatedCSSValueConversion.cpp`<br>`style/values/primitives/StyleLengthWrapper+DeprecatedCSSValueConversion.h` | BSD-2-Clause |
| `Style/Values/Primitives/NumericTypes` | .hpp | `style/values/primitives/StylePrimitiveNumericTypes.h` | BSD-2-Clause |
| `Style/Values/Primitives/NumericTypesBlending` | .hpp | `style/values/primitives/StylePrimitiveNumericTypes+Blending.h` | BSD-2-Clause |
| `Style/Values/Primitives/NumericTypesCSSValueConversion` | .hpp | `style/values/primitives/StylePrimitiveNumericTypes+CSSValueConversion.h` | BSD-2-Clause |
| `Style/Values/Primitives/NumericTypesCSSValueCreation` | .hpp | `style/values/primitives/StylePrimitiveNumericTypes+CSSValueCreation.h` | BSD-2-Clause |
| `Style/Values/Primitives/NumericTypesCalc` | .hpp | `style/values/primitives/StylePrimitiveNumericTypes+Calculation.h` | BSD-2-Clause |
| `Style/Values/Primitives/NumericTypesConversions` | .cpp, .hpp | `style/values/primitives/StylePrimitiveNumericTypes+Conversions.cpp`<br>`style/values/primitives/StylePrimitiveNumericTypes+Conversions.h` | BSD-2-Clause |
| `Style/Values/Primitives/NumericTypesEvaluation` | .hpp | `style/values/primitives/StylePrimitiveNumericTypes+Evaluation.h` | BSD-2-Clause |
| `Style/Values/Primitives/NumericTypesSerialisation` | .hpp | `style/values/primitives/StylePrimitiveNumericTypes+Serialization.h` | BSD-2-Clause |
| `Style/Values/Primitives/Position` | .cpp, .hpp | `style/values/primitives/StylePosition.cpp`<br>`style/values/primitives/StylePosition.h` | BSD-2-Clause |
| `Style/Values/Primitives/PrimitiveNumeric` | .hpp | `style/values/primitives/StylePrimitiveNumeric.h` | BSD-2-Clause |
| `Style/Values/Primitives/PrimitiveNumericForward` | .hpp | `style/values/primitives/StylePrimitiveNumeric+Forward.h` | BSD-2-Clause |
| `Style/Values/Primitives/PrimitiveNumericOrKeyword` | .hpp | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` | BSD-2-Clause |
| `Style/Values/Primitives/PrimitiveNumericOrKeywordConversions` | .hpp | `style/values/primitives/StylePrimitiveNumericOrKeyword+Conversions.h` | BSD-2-Clause |
| `Style/Values/Primitives/Ratio` | .cpp, .hpp | `style/values/primitives/StyleRatio.cpp`<br>`style/values/primitives/StyleRatio.h` | BSD-2-Clause |
| `Style/Values/Primitives/UnevaluatedCalc` | .cpp, .hpp | `css/values/primitives/CSSUnevaluatedCalc.cpp`<br>`css/values/primitives/CSSUnevaluatedCalc.h` | BSD-2-Clause |
| `Style/Values/Primitives/Url` | .cpp, .hpp | `style/values/primitives/StyleURL.cpp`<br>`style/values/primitives/StyleURL.h` | BSD-2-Clause |
| `Style/Values/Rhythm/BlockStepSize` | .cpp, .hpp | `style/values/rhythm/StyleBlockStepSize.cpp`<br>`style/values/rhythm/StyleBlockStepSize.h` | BSD-2-Clause |
| `Style/Values/SVG/SVGBaselineShift` | .cpp, .hpp | `style/values/svg/StyleSVGBaselineShift.cpp`<br>`style/values/svg/StyleSVGBaselineShift.h` | BSD-2-Clause |
| `Style/Values/SVG/SVGCenterCoordinateComponent` | .hpp | `style/values/svg/StyleSVGCenterCoordinateComponent.h` | BSD-2-Clause |
| `Style/Values/SVG/SVGCoordinateComponent` | .hpp | `style/values/svg/StyleSVGCoordinateComponent.h` | BSD-2-Clause |
| `Style/Values/SVG/SVGGlyphOrientationHorizontal` | .cpp, .hpp | `style/values/svg/StyleSVGGlyphOrientationHorizontal.cpp`<br>`style/values/svg/StyleSVGGlyphOrientationHorizontal.h` | BSD-2-Clause |
| `Style/Values/SVG/SVGGlyphOrientationVertical` | .cpp, .hpp | `style/values/svg/StyleSVGGlyphOrientationVertical.cpp`<br>`style/values/svg/StyleSVGGlyphOrientationVertical.h` | BSD-2-Clause |
| `Style/Values/SVG/SVGMarkerResource` | .hpp | `style/values/svg/StyleSVGMarkerResource.h` | BSD-2-Clause |
| `Style/Values/SVG/SVGPaint` | .cpp, .hpp | `style/values/svg/StyleSVGPaint.cpp`<br>`style/values/svg/StyleSVGPaint.h` | BSD-2-Clause |
| `Style/Values/SVG/SVGPaintOrder` | .cpp, .hpp | `style/values/svg/StyleSVGPaintOrder.cpp`<br>`style/values/svg/StyleSVGPaintOrder.h` | BSD-2-Clause |
| `Style/Values/SVG/SVGPathData` | .cpp, .hpp | `style/values/svg/StyleSVGPathData.cpp`<br>`style/values/svg/StyleSVGPathData.h` | BSD-2-Clause |
| `Style/Values/SVG/SVGRadius` | .hpp | `style/values/svg/StyleSVGRadius.h` | BSD-2-Clause |
| `Style/Values/SVG/SVGRadiusComponent` | .hpp | `style/values/svg/StyleSVGRadiusComponent.h` | BSD-2-Clause |
| `Style/Values/SVG/SVGStrokeDasharray` | .cpp, .hpp | `style/values/svg/StyleSVGStrokeDasharray.cpp`<br>`style/values/svg/StyleSVGStrokeDasharray.h` | BSD-2-Clause |
| `Style/Values/SVG/SVGStrokeDashoffset` | .cpp, .hpp | `style/values/svg/StyleSVGStrokeDashoffset.cpp`<br>`style/values/svg/StyleSVGStrokeDashoffset.h` | BSD-2-Clause |
| `Style/Values/ScrollAnimations/ProgressTimelineAxes` | .hpp | `style/values/scroll-animations/StyleProgressTimelineAxes.h` | BSD-2-Clause |
| `Style/Values/ScrollAnimations/ProgressTimelineName` | .cpp, .hpp | `style/values/scroll-animations/StyleProgressTimelineName.cpp`<br>`style/values/scroll-animations/StyleProgressTimelineName.h` | BSD-2-Clause |
| `Style/Values/ScrollAnimations/ScrollFunction` | .cpp, .hpp | `style/values/scroll-animations/StyleScrollFunction.cpp`<br>`style/values/scroll-animations/StyleScrollFunction.h` | BSD-2-Clause |
| `Style/Values/ScrollAnimations/ScrollTimelines` | .hpp | `style/values/scroll-animations/StyleScrollTimelines.h` | BSD-2-Clause |
| `Style/Values/ScrollAnimations/ViewFunction` | .cpp, .hpp | `style/values/scroll-animations/StyleViewFunction.cpp`<br>`style/values/scroll-animations/StyleViewFunction.h` | BSD-2-Clause |
| `Style/Values/ScrollAnimations/ViewTimelineInsets` | .cpp, .hpp | `style/values/scroll-animations/StyleViewTimelineInsets.cpp`<br>`style/values/scroll-animations/StyleViewTimelineInsets.h` | BSD-2-Clause |
| `Style/Values/ScrollAnimations/ViewTimelines` | .hpp | `style/values/scroll-animations/StyleViewTimelines.h` | BSD-2-Clause |
| `Style/Values/ScrollSnap/ScrollMargin` | .cpp, .hpp | `style/values/scroll-snap/StyleScrollMargin.cpp`<br>`style/values/scroll-snap/StyleScrollMargin.h` | BSD-2-Clause |
| `Style/Values/ScrollSnap/ScrollPadding` | .cpp, .hpp | `style/values/scroll-snap/StyleScrollPadding.cpp`<br>`style/values/scroll-snap/StyleScrollPadding.h` | BSD-2-Clause |
| `Style/Values/ScrollSnap/ScrollSnapAlign` | .cpp, .hpp | `style/values/scroll-snap/StyleScrollSnapAlign.cpp`<br>`style/values/scroll-snap/StyleScrollSnapAlign.h` | BSD-2-Clause |
| `Style/Values/ScrollSnap/ScrollSnapType` | .cpp, .hpp | `style/values/scroll-snap/StyleScrollSnapType.cpp`<br>`style/values/scroll-snap/StyleScrollSnapType.h` | BSD-2-Clause |
| `Style/Values/Scrollbars/ScrollbarColor` | .cpp, .hpp | `style/values/scrollbars/StyleScrollbarColor.cpp`<br>`style/values/scrollbars/StyleScrollbarColor.h` | BSD-2-Clause |
| `Style/Values/Scrollbars/ScrollbarWidth` | .cpp, .hpp | `style/values/scrollbars/StyleScrollbarWidth.cpp`<br>`style/values/scrollbars/StyleScrollbarWidth.h` | BSD-2-Clause |
| `Style/Values/Shapes/BasicShape` | .cpp, .hpp | `style/values/shapes/StyleBasicShape.cpp`<br>`style/values/shapes/StyleBasicShape.h` | BSD-2-Clause |
| `Style/Values/Shapes/CircleFunction` | .cpp, .hpp | `style/values/shapes/StyleCircleFunction.cpp`<br>`style/values/shapes/StyleCircleFunction.h` | BSD-2-Clause |
| `Style/Values/Shapes/EllipseFunction` | .cpp, .hpp | `style/values/shapes/StyleEllipseFunction.cpp`<br>`style/values/shapes/StyleEllipseFunction.h` | BSD-2-Clause |
| `Style/Values/Shapes/FillRule` | .hpp | `style/values/shapes/StyleFillRule.h` | BSD-2-Clause |
| `Style/Values/Shapes/InsetFunction` | .cpp, .hpp | `style/values/shapes/StyleInsetFunction.cpp`<br>`style/values/shapes/StyleInsetFunction.h` | BSD-2-Clause |
| `Style/Values/Shapes/PathComputation` | .hpp | `style/values/shapes/StylePathComputation.h` | BSD-2-Clause |
| `Style/Values/Shapes/PathFunction` | .cpp, .hpp | `style/values/shapes/StylePathFunction.cpp`<br>`style/values/shapes/StylePathFunction.h` | BSD-2-Clause |
| `Style/Values/Shapes/PathOperationWrappers` | .cpp, .hpp | `style/values/shapes/StylePathOperationWrappers.cpp`<br>`style/values/shapes/StylePathOperationWrappers.h` | BSD-2-Clause |
| `Style/Values/Shapes/PolygonFunction` | .cpp, .hpp | `style/values/shapes/StylePolygonFunction.cpp`<br>`style/values/shapes/StylePolygonFunction.h` | BSD-2-Clause |
| `Style/Values/Shapes/RectFunction` | .cpp, .hpp | `style/values/shapes/StyleRectFunction.cpp`<br>`style/values/shapes/StyleRectFunction.h` | BSD-2-Clause |
| `Style/Values/Shapes/ShapeFunction` | .cpp, .hpp | `style/values/shapes/StyleShapeFunction.cpp`<br>`style/values/shapes/StyleShapeFunction.h` | BSD-2-Clause |
| `Style/Values/Shapes/ShapeImageThreshold` | .hpp | `style/values/shapes/StyleShapeImageThreshold.h` | BSD-2-Clause |
| `Style/Values/Shapes/ShapeMargin` | .hpp | `style/values/shapes/StyleShapeMargin.h` | BSD-2-Clause |
| `Style/Values/Shapes/ShapeOutside` | .cpp, .hpp | `style/values/shapes/StyleShapeOutside.cpp`<br>`style/values/shapes/StyleShapeOutside.h` | BSD-2-Clause |
| `Style/Values/Shapes/WindRuleComputation` | .hpp | `style/values/shapes/StyleWindRuleComputation.h` | BSD-2-Clause |
| `Style/Values/Shapes/XywhFunction` | .cpp, .hpp | `style/values/shapes/StyleXywhFunction.cpp`<br>`style/values/shapes/StyleXywhFunction.h` | BSD-2-Clause |
| `Style/Values/Sizing/AspectRatio` | .cpp, .hpp | `style/values/sizing/StyleAspectRatio.cpp`<br>`style/values/sizing/StyleAspectRatio.h` | BSD-2-Clause |
| `Style/Values/Sizing/ContainIntrinsicSize` | .cpp, .hpp | `style/values/sizing/StyleContainIntrinsicSize.cpp`<br>`style/values/sizing/StyleContainIntrinsicSize.h` | BSD-2-Clause |
| `Style/Values/Sizing/MaximumSize` | .hpp | `style/values/sizing/StyleMaximumSize.h` | BSD-2-Clause |
| `Style/Values/Sizing/MinimumSize` | .hpp | `style/values/sizing/StyleMinimumSize.h` | BSD-2-Clause |
| `Style/Values/Sizing/PreferredSize` | .cpp, .hpp | `style/values/sizing/StylePreferredSize.cpp`<br>`style/values/sizing/StylePreferredSize.h` | BSD-2-Clause |
| `Style/Values/Speech/SpeakAs` | .cpp, .hpp | `style/values/speech/StyleSpeakAs.cpp`<br>`style/values/speech/StyleSpeakAs.h` | BSD-2-Clause |
| `Style/Values/Text/HangingPunctuation` | .cpp, .hpp | `style/values/text/StyleHangingPunctuation.cpp`<br>`style/values/text/StyleHangingPunctuation.h` | BSD-2-Clause |
| `Style/Values/Text/HyphenateCharacter` | .hpp | `style/values/text/StyleHyphenateCharacter.h` | BSD-2-Clause |
| `Style/Values/Text/HyphenateLimitEdge` | .hpp | `style/values/text/StyleHyphenateLimitEdge.h` | BSD-2-Clause |
| `Style/Values/Text/HyphenateLimitLines` | .hpp | `style/values/text/StyleHyphenateLimitLines.h` | BSD-2-Clause |
| `Style/Values/Text/LetterSpacing` | .cpp, .hpp | `style/values/text/StyleLetterSpacing.cpp`<br>`style/values/text/StyleLetterSpacing.h` | BSD-2-Clause |
| `Style/Values/Text/TabSize` | .cpp, .hpp | `style/values/text/StyleTabSize.cpp`<br>`style/values/text/StyleTabSize.h` | BSD-2-Clause |
| `Style/Values/Text/TextAlign` | .cpp, .hpp | `style/values/text/StyleTextAlign.cpp`<br>`style/values/text/StyleTextAlign.h` | BSD-2-Clause |
| `Style/Values/Text/TextAlignLast` | .cpp, .hpp | `style/values/text/StyleTextAlignLast.cpp`<br>`style/values/text/StyleTextAlignLast.h` | BSD-2-Clause |
| `Style/Values/Text/TextAutospace` | .cpp, .hpp | `style/values/text/StyleTextAutospace.cpp`<br>`style/values/text/StyleTextAutospace.h` | BSD-2-Clause |
| `Style/Values/Text/TextIndent` | .cpp, .hpp | `style/values/text/StyleTextIndent.cpp`<br>`style/values/text/StyleTextIndent.h` | BSD-2-Clause |
| `Style/Values/Text/TextSpacingTrim` | .hpp | `style/values/text/StyleTextSpacingTrim.h` | BSD-2-Clause |
| `Style/Values/Text/TextTransform` | .cpp, .hpp | `style/values/text/StyleTextTransform.cpp`<br>`style/values/text/StyleTextTransform.h` | BSD-2-Clause |
| `Style/Values/Text/WordSpacing` | .cpp, .hpp | `style/values/text/StyleWordSpacing.cpp`<br>`style/values/text/StyleWordSpacing.h` | BSD-2-Clause |
| `Style/Values/TextDecoration/TextDecorationLine` | .cpp, .hpp | `style/values/text-decoration/StyleTextDecorationLine.cpp`<br>`style/values/text-decoration/StyleTextDecorationLine.h` | BSD-2-Clause |
| `Style/Values/TextDecoration/TextDecorationThickness` | .cpp, .hpp | `style/values/text-decoration/StyleTextDecorationThickness.cpp`<br>`style/values/text-decoration/StyleTextDecorationThickness.h` | BSD-2-Clause |
| `Style/Values/TextDecoration/TextEmphasisPosition` | .cpp, .hpp | `style/values/text-decoration/StyleTextEmphasisPosition.cpp`<br>`style/values/text-decoration/StyleTextEmphasisPosition.h` | BSD-2-Clause |
| `Style/Values/TextDecoration/TextEmphasisStyle` | .cpp, .hpp | `style/values/text-decoration/StyleTextEmphasisStyle.cpp`<br>`style/values/text-decoration/StyleTextEmphasisStyle.h` | BSD-2-Clause |
| `Style/Values/TextDecoration/TextShadow` | .cpp, .hpp | `style/values/text-decoration/StyleTextShadow.cpp`<br>`style/values/text-decoration/StyleTextShadow.h` | BSD-2-Clause |
| `Style/Values/TextDecoration/TextUnderlineOffset` | .cpp, .hpp | `style/values/text-decoration/StyleTextUnderlineOffset.cpp`<br>`style/values/text-decoration/StyleTextUnderlineOffset.h` | BSD-2-Clause |
| `Style/Values/TextDecoration/TextUnderlinePosition` | .cpp, .hpp | `style/values/text-decoration/StyleTextUnderlinePosition.cpp`<br>`style/values/text-decoration/StyleTextUnderlinePosition.h` | BSD-2-Clause |
| `Style/Values/Transforms/Functions/Matrix3DTransformFunction` | .cpp, .hpp | `style/values/transforms/functions/StyleMatrix3DTransformFunction.cpp`<br>`style/values/transforms/functions/StyleMatrix3DTransformFunction.h` | BSD-2-Clause |
| `Style/Values/Transforms/Functions/PerspectiveTransformFunction` | .cpp, .hpp | `style/values/transforms/functions/StylePerspectiveTransformFunction.cpp`<br>`style/values/transforms/functions/StylePerspectiveTransformFunction.h` | BSD-2-Clause |
| `Style/Values/Transforms/Functions/TransformFunctionBase` | .cpp | `style/values/transforms/functions/StyleTransformFunctionBase.cpp` | BSD-2-Clause |
| `Style/Values/Transforms/Functions/TransformFunctionWrapper` | .hpp | `style/values/transforms/functions/StyleTransformFunctionWrapper.h` | BSD-2-Clause |
| `Style/Values/Transforms/Perspective` | .cpp, .hpp | `style/values/transforms/StylePerspective.cpp`<br>`style/values/transforms/StylePerspective.h` | BSD-2-Clause |
| `Style/Values/Transforms/PerspectiveOrigin` | .hpp | `style/values/transforms/StylePerspectiveOrigin.h` | BSD-2-Clause |
| `Style/Values/Transforms/Rotate` | .cpp, .hpp | `style/values/transforms/StyleRotate.cpp`<br>`style/values/transforms/StyleRotate.h` | BSD-2-Clause |
| `Style/Values/Transforms/Scale` | .cpp, .hpp | `style/values/transforms/StyleScale.cpp`<br>`style/values/transforms/StyleScale.h` | BSD-2-Clause |
| `Style/Values/Transforms/Transform` | .cpp, .hpp | `style/values/transforms/StyleTransform.cpp`<br>`style/values/transforms/StyleTransform.h` | BSD-2-Clause |
| `Style/Values/Transforms/TransformFunction` | .cpp, .hpp | `style/values/transforms/StyleTransformFunction.cpp`<br>`style/values/transforms/StyleTransformFunction.h` | BSD-2-Clause |
| `Style/Values/Transforms/TransformList` | .cpp, .hpp | `style/values/transforms/StyleTransformList.cpp`<br>`style/values/transforms/StyleTransformList.h` | BSD-2-Clause |
| `Style/Values/Transforms/TransformOrigin` | .hpp | `style/values/transforms/StyleTransformOrigin.h` | BSD-2-Clause |
| `Style/Values/Transforms/Translate` | .cpp, .hpp | `style/values/transforms/StyleTranslate.cpp`<br>`style/values/transforms/StyleTranslate.h` | BSD-2-Clause |
| `Style/Values/Transitions/SingleTransitionDelay` | .hpp | `style/values/transitions/StyleSingleTransitionDelay.h` | BSD-2-Clause |
| `Style/Values/Transitions/SingleTransitionDuration` | .hpp | `style/values/transitions/StyleSingleTransitionDuration.h` | BSD-2-Clause |
| `Style/Values/Transitions/SingleTransitionProperty` | .cpp, .hpp | `style/values/transitions/StyleSingleTransitionProperty.cpp`<br>`style/values/transitions/StyleSingleTransitionProperty.h` | BSD-2-Clause |
| `Style/Values/Transitions/Transition` | .cpp, .hpp | `style/values/transitions/StyleTransition.cpp`<br>`style/values/transitions/StyleTransition.h` | BSD-2-Clause |
| `Style/Values/Transitions/Transitions` | .hpp | `style/values/transitions/StyleTransitions.h` | BSD-2-Clause |
| `Style/Values/Types` | .hpp | `style/values/StyleValueTypes.h` | BSD-2-Clause |
| `Style/Values/UI/AccentColor` | .cpp, .hpp | `style/values/ui/StyleAccentColor.cpp`<br>`style/values/ui/StyleAccentColor.h` | BSD-2-Clause |
| `Style/Values/UI/Cursor` | .cpp, .hpp | `style/values/ui/StyleCursor.cpp`<br>`style/values/ui/StyleCursor.h` | BSD-2-Clause |
| `Style/Values/UI/Resize` | .cpp, .hpp | `style/values/ui/StyleResize.cpp`<br>`style/values/ui/StyleResize.h` | BSD-2-Clause |
| `Style/Values/Viewport/Zoom` | .hpp | `style/values/viewport/StyleZoom.h` | BSD-2-Clause |
| `Style/Values/Viewport/ZoomPrimitives` | .hpp | `style/values/viewport/StyleZoomPrimitives.h` | BSD-2-Clause |
| `Style/Values/WillChange/WillChange` | .cpp, .hpp | `style/values/will-change/StyleWillChange.cpp`<br>`style/values/will-change/StyleWillChange.h` | BSD-2-Clause |
| `Text/CharacterProperties` | .hpp | `WTF/wtf/text/CharacterProperties.h` | BSD-2-Clause |
| `Text/Hyphenation` | .cpp, .hpp | `platform/text/Hyphenation.cpp`<br>`platform/text/Hyphenation.h` | BSD-2-Clause |
| `Text/LocaleToScriptMapping` | .cpp, .hpp | `platform/text/LocaleToScriptMapping.cpp`<br>`platform/text/LocaleToScriptMapping.h` | BSD-3-Clause |
| `Text/TextBoundaries` | .cpp, .hpp | `platform/text/TextBoundaries.cpp`<br>`platform/text/TextBoundaries.h` | BSD-2-Clause |
| `Text/TextFlags` | .cpp, .hpp | `platform/text/TextFlags.cpp`<br>`platform/text/TextFlags.h` | BSD-2-Clause |
| `Text/TextSpacing` | .cpp, .hpp | `platform/text/TextSpacing.cpp`<br>`platform/text/TextSpacing.h` | BSD-2-Clause |
| `Text/UnicodeBidi` | .hpp | `platform/text/UnicodeBidi.h` | BSD-2-Clause |
| `Text/UnicodeHelpers` | .cpp, .hpp | `platform/text/UnicodeHelpers.cpp`<br>`platform/text/UnicodeHelpers.h` | BSD-2-Clause |
| `Text/WritingMode` | .hpp | `platform/text/WritingMode.h` | BSD-3-Clause |
| `Types/NestingLevelIncrementer` | .hpp | `html/parser/NestingLevelIncrementer.h` | BSD-2-Clause |
| `URL/URL` | .cpp, .hpp | `WTF/wtf/URL.cpp`<br>`WTF/wtf/URL.h` | BSD-2-Clause |
| `URL/URLHash` | .hpp | `WTF/wtf/URLHash.h` | BSD-2-Clause |
| `URL/URLParser` | .cpp, .hpp | `WTF/wtf/URLParser.cpp`<br>`WTF/wtf/URLParser.h` | BSD-2-Clause |
| `XML/XMLDocumentParserScope` | .cpp, .hpp | `xml/parser/XMLDocumentParserScope.cpp`<br>`xml/parser/XMLDocumentParserScope.h` | BSD-2-Clause |
| `XML/XMLErrors` | .cpp, .hpp | `xml/XMLErrors.cpp`<br>`xml/XMLErrors.h` | BSD-2-Clause |
| `codegen/css_properties` | .py | `css/scripts/process-css-properties.py` | BSD-2-Clause |
| `codegen/css_pseudo_selectors` | .py | `css/scripts/process-css-pseudo-selectors.py` | BSD-2-Clause |
| `codegen/css_value_keywords` | .py | `css/scripts/process-css-values.py` | BSD-2-Clause |
| `codegen/user-agent-style-sheets/counterStyles` | .css | `css/counterStyles.css` | BSD-2-Clause |
| `codegen/user-agent-style-sheets/detailsElementShadow` | .css | `html/shadow/detailsElementShadow.css` | BSD-2-Clause |
| `codegen/user-agent-style-sheets/horizontalFormControls` | .css | `css/horizontalFormControls.css` | BSD-2-Clause |
| `codegen/user-agent-style-sheets/mathml` | .css | `css/mathml.css` | BSD-2-Clause |
| `codegen/user-agent-style-sheets/mathmlCoreExtras` | .css | `css/mathmlCoreExtras.css` | BSD-2-Clause |
| `codegen/user-agent-style-sheets/mathmlFontSizeMath` | .css | `css/mathmlFontSizeMath.css` | BSD-2-Clause |
| `codegen/user-agent-style-sheets/meterElementShadow` | .css | `html/shadow/meterElementShadow.css` | no notice in WebKit |
| `codegen/user-agent-style-sheets/popover` | .css | `css/popover.css` | no notice in WebKit |
| `codegen/user-agent-style-sheets/svg` | .css | `css/svg.css` | BSD-2-Clause |
| `codegen/utils` | .py | `css/scripts/process-css-properties.py` | BSD-2-Clause |

## Hand-written

Written from the specifications rather than ported; no notice. Some share a name with a WebKit file but not its code.

- `Animation/AnimationPromise` (.cpp, .hpp)
- `Animation/GetAnimationsOptions` (.hpp)
- `Animation/KeyframeAnimationOptions` (.hpp)
- `Animation/KeyframeEffectOptions` (.hpp)
- `Animation/Keyframes/KeyframeInput` (.hpp)
- `Animation/Timelines/DocumentTimelineOptions` (.hpp)
- `Animation/Timelines/ScrollAxis` (.hpp)
- `Animation/Timelines/ScrollTimelineOptions` (.hpp)
- `Animation/Timelines/TimelineRangeOffset` (.hpp)
- `Animation/Timelines/ViewTimelineOptions` (.hpp)
- `Animation/Timing/AnimationEffectPhase` (.hpp)
- `Animation/Timing/FillMode` (.hpp)
- `Animation/Timing/OptionalEffectTiming` (.hpp)
- `Animation/Timing/PlaybackDirection` (.hpp)
- `CSS/Calc/AllowedSymbols` (.hpp)
- `CSS/Font/GenericFontFamilyNames` (.cpp, .hpp)
- `CSS/Media/MediaQueryListEvent` (.cpp, .hpp)
- `CSS/Parser/InputStream` (.hpp)
- `CSS/Parser/KeywordLookup` (.cpp, .hpp)
- `CSS/Parser/NumericValue` (.hpp)
- `CSS/Parser/ParseError` (.hpp)
- `CSS/Parser/TokenType` (.hpp)
- `CSS/Properties/Consumers/PrimitiveValue` (.hpp)
- `CSS/Properties/ParsedPropertyList` (.hpp)
- `CSS/Properties/PropertyFlag` (.hpp)
- `CSS/Properties/Types/IsImplicit` (.hpp)
- `CSS/Properties/Types/IsImportant` (.hpp)
- `CSS/Properties/Types/IsSetFromShorthand` (.hpp)
- `CSS/Queries/ContainerQueryAxis` (.hpp)
- `CSS/Queries/MediaEnvironment` (.hpp)
- `CSS/Types/CSSOMString` (.hpp)
- `CSS/Types/MediaListOrDOMString` (.hpp)
- `CSS/Values/NoConversionDataRequiredTag` (.hpp)
- `CSS/Values/NumberFormatting` (.cpp, .hpp)
- `CSS/Values/SerialisationContext` (.cpp, .hpp)
- `CSS/Values/Shapes/FillRule` (.hpp)
- `CSS/Values/ValueListBuilder` (.hpp)
- `DOM/Abort/AbortController` (.cpp, .hpp)
- `DOM/Abort/AbortSignal` (.cpp, .hpp)
- `DOM/Algorithms/AbortAlgorithms` (.cpp, .hpp)
- `DOM/Algorithms/DocumentAlgorithms` (.cpp, .hpp)
- `DOM/Algorithms/ElementAlgorithms` (.cpp, .hpp)
- `DOM/Algorithms/EventTargetAlgorithms` (.cpp, .hpp)
- `DOM/Algorithms/IteratorAlgorithms` (.cpp, .hpp)
- `DOM/Algorithms/LiveRangeUpdater` (.cpp, .hpp)
- `DOM/Algorithms/MutationAlgorithms` (.cpp, .hpp)
- `DOM/Algorithms/MutationObserverAlgorithms` (.cpp, .hpp)
- `DOM/Algorithms/NameValidation` (.cpp, .hpp)
- `DOM/Algorithms/NodeAlgorithms` (.cpp, .hpp)
- `DOM/Algorithms/OrderedSet` (.cpp, .hpp)
- `DOM/Algorithms/SlotAlgorithms` (.cpp, .hpp)
- `DOM/Algorithms/TextAlgorithms` (.cpp, .hpp)
- `DOM/Collections/DatasetDOMStringMap` (.cpp, .hpp)
- `DOM/Collections/HTMLCollection` (.hpp)
- `DOM/Collections/LiveCollection` (.cpp, .hpp)
- `DOM/Collections/LiveHTMLCollection` (.cpp, .hpp)
- `DOM/Collections/LiveNodeList` (.cpp, .hpp)
- `DOM/Collections/NodeList` (.hpp)
- `DOM/Collections/StaticHTMLCollection` (.cpp, .hpp)
- `DOM/Collections/StaticNodeList` (.cpp, .hpp)
- `DOM/Events/ClipboardEvent` (.cpp, .hpp)
- `DOM/Events/CommandEvent` (.cpp, .hpp)
- `DOM/Events/CompositionEvent` (.cpp, .hpp)
- `DOM/Events/ContentVisibilityAutoStateChangeEvent` (.cpp, .hpp)
- `DOM/Events/CustomEvent` (.hpp)
- `DOM/Events/DragEvent` (.cpp, .hpp)
- `DOM/Events/EventListener` (.hpp)
- `DOM/Events/EventListenerHandle` (.cpp, .hpp)
- `DOM/Events/EventNames` (.hpp)
- `DOM/Events/EventPathItem` (.cpp, .hpp)
- `DOM/Events/EventTarget` (.cpp, .hpp)
- `DOM/Events/FocusEvent` (.hpp)
- `DOM/Events/FormDataEvent` (.cpp, .hpp)
- `DOM/Events/ScopedEventQueue` (.cpp, .hpp)
- `DOM/Events/ToggleEvent` (.cpp, .hpp)
- `DOM/Events/UIEvent` (.cpp, .hpp)
- `DOM/Internals/ElementFactory` (.cpp, .hpp)
- `DOM/Internals/EventFactory` (.hpp)
- `DOM/Internals/LayoutOptions` (.hpp)
- `DOM/Iterators/SubtreeRanges` (.hpp)
- `DOM/Mixins/ChildNode` (.cpp, .hpp)
- `DOM/Mixins/NonDocumentTypeChildNode` (.cpp, .hpp)
- `DOM/Mixins/NonElementParentNode` (.cpp, .hpp)
- `DOM/Mixins/ParentNode` (.cpp, .hpp)
- `DOM/Mixins/Slottable` (.cpp, .hpp)
- `DOM/MutationObservers/AttributesMutationRecord` (.cpp, .hpp)
- `DOM/MutationObservers/CharacterDataMutationRecord` (.cpp, .hpp)
- `DOM/MutationObservers/MutationObserver` (.cpp, .hpp)
- `DOM/MutationObservers/MutationRecord` (.hpp)
- `DOM/MutationObservers/RegisteredObserver` (.hpp)
- `DOM/MutationObservers/TransientRegisteredObserver` (.hpp)
- `DOM/Nodes/Attr` (.cpp, .hpp)
- `DOM/Nodes/CDATASection` (.cpp, .hpp)
- `DOM/Nodes/CharacterData` (.cpp, .hpp)
- `DOM/Nodes/Comment` (.cpp, .hpp)
- `DOM/Nodes/DOMImplementation` (.cpp, .hpp)
- `DOM/Nodes/DocumentFragment` (.cpp, .hpp)
- `DOM/Nodes/DocumentType` (.cpp, .hpp)
- `DOM/Nodes/GetHTMLOptions` (.hpp)
- `DOM/Nodes/NamedNodeMap` (.cpp, .hpp)
- `DOM/Nodes/PointerLockOptions` (.hpp)
- `DOM/Nodes/XHTMLDocument` (.hpp)
- `DOM/Nodes/XMLDocument` (.hpp)
- `DOM/Ranges/AbstractRange` (.hpp)
- `DOM/Ranges/StaticRange` (.cpp, .hpp)
- `DOM/Scheduling/DocumentTimer` (.cpp, .hpp)
- `DOM/Traversal/NodeFilter` (.hpp)
- `DOM/Traversal/NodeIterator` (.cpp, .hpp)
- `DOM/Traversal/TraversalBase` (.hpp)
- `DOM/Traversal/TreeWalker` (.cpp, .hpp)
- `DOM/Types/DOMHighResTimeStamp` (.hpp)
- `DOM/Types/DOMString` (.hpp)
- `DOM/Types/ElementOrText` (.hpp)
- `DOM/Types/NodeOrString` (.hpp)
- `DOM/Types/SmallNodeList` (.hpp)
- `DOM/Types/USVString` (.hpp)
- `DOM/Types/Uint8ClampedArray` (.hpp)
- `FileAPI/BlobPropertyBag` (.hpp)
- `Graphics/Color/NamedColors` (.cpp, .hpp)
- `Graphics/Fonts/FontDatabase` (.cpp, .hpp)
- `Graphics/ImageBuffers/ImageBufferUtilities` (.cpp, .hpp)
- `HTML/Algorithms/CustomElementAlgorithms` (.cpp, .hpp)
- `HTML/Algorithms/DOMTreeAccessors` (.hpp)
- `HTML/Algorithms/FormControlAlgorithms` (.cpp, .hpp)
- `HTML/Algorithms/HTMLInsertionAlgorithms` (.cpp, .hpp)
- `HTML/Algorithms/HyperlinkAlgorithms` (.cpp, .hpp)
- `HTML/Algorithms/RenderBlocking` (.cpp, .hpp)
- `HTML/Attributes/EnumeratedAttributes` (.hpp)
- `HTML/Attributes/EventHandlerAttributes` (.cpp, .hpp)
- `HTML/Attributes/ReflectAttribute` (.hpp)
- `HTML/Canvas/CanvasDirection` (.hpp)
- `HTML/Canvas/CanvasFillRule` (.hpp)
- `HTML/Canvas/CanvasFontKerning` (.hpp)
- `HTML/Canvas/CanvasFontStretch` (.hpp)
- `HTML/Canvas/CanvasFontVariantCaps` (.hpp)
- `HTML/Canvas/CanvasLineCap` (.hpp)
- `HTML/Canvas/CanvasLineJoin` (.hpp)
- `HTML/Canvas/CanvasRenderingContext2DSettings` (.hpp)
- `HTML/Canvas/CanvasTextRendering` (.hpp)
- `HTML/Canvas/EngineTexture` (.hpp)
- `HTML/Canvas/EngineTextureRenderingContext` (.cpp, .hpp)
- `HTML/Canvas/ImageSmoothingQuality` (.hpp)
- `HTML/Collections/HTMLFormControlsCollection` (.cpp, .hpp)
- `HTML/CustomElement/CustomElement` (.hpp)
- `HTML/Elements/HTMLAudioElement` (.cpp, .hpp)
- `HTML/Elements/HTMLBDIElement` (.cpp, .hpp)
- `HTML/Elements/HTMLBaseElement` (.cpp, .hpp)
- `HTML/Elements/HTMLDListElement` (.cpp, .hpp)
- `HTML/Elements/HTMLDataElement` (.cpp, .hpp)
- `HTML/Elements/HTMLDataListElement` (.cpp, .hpp)
- `HTML/Elements/HTMLDirectoryElement` (.cpp, .hpp)
- `HTML/Elements/HTMLDivElement` (.cpp, .hpp)
- `HTML/Elements/HTMLEmbedElement` (.cpp, .hpp)
- `HTML/Elements/HTMLFontElement` (.cpp, .hpp)
- `HTML/Elements/HTMLHeadElement` (.cpp, .hpp)
- `HTML/Elements/HTMLHeadingElement` (.cpp, .hpp)
- `HTML/Elements/HTMLHtmlElement` (.cpp, .hpp)
- `HTML/Elements/HTMLMarqueeElement` (.cpp, .hpp)
- `HTML/Elements/HTMLMenuElement` (.cpp, .hpp)
- `HTML/Elements/HTMLModElement` (.cpp, .hpp)
- `HTML/Elements/HTMLParagraphElement` (.cpp, .hpp)
- `HTML/Elements/HTMLParamElement` (.cpp, .hpp)
- `HTML/Elements/HTMLPreElement` (.cpp, .hpp)
- `HTML/Elements/HTMLQuoteElement` (.cpp, .hpp)
- `HTML/Elements/HTMLSelectedContentElement` (.cpp, .hpp)
- `HTML/Elements/HTMLSlotElement` (.cpp, .hpp)
- `HTML/Elements/HTMLSpanElement` (.cpp, .hpp)
- `HTML/Elements/HTMLTableCaptionElement` (.cpp, .hpp)
- `HTML/Elements/HTMLTableRowElement` (.cpp, .hpp)
- `HTML/Elements/HTMLTemplateElement` (.cpp, .hpp)
- `HTML/Elements/HTMLTimeElement` (.cpp, .hpp)
- `HTML/Elements/HTMLTitleElement` (.cpp, .hpp)
- `HTML/Elements/HTMLTrackElement` (.cpp, .hpp)
- `HTML/Elements/HTMLUnknownElement` (.cpp, .hpp)
- `HTML/EventLoop/EventLoop` (.cpp, .hpp)
- `HTML/Events/PageTransitionEvent` (.cpp, .hpp)
- `HTML/Events/SubmitEvent` (.cpp, .hpp)
- `HTML/Internals/HTMLElementFactory` (.cpp, .hpp)
- `HTML/Internals/PresentationalHints` (.hpp)
- `HTML/MicroParsers/MicroParserError` (.hpp)
- `HTML/MicroParsers/MicroParserResult` (.hpp)
- `HTML/MicroParsers/Numbers` (.cpp, .hpp)
- `HTML/Parser/CharacterReference` (.hpp)
- `HTML/Parser/DocumentParser` (.cpp, .hpp)
- `HTML/Parser/InputStream` (.hpp)
- `HTML/Sanitizer/Sanitizer` (.cpp, .hpp)
- `HTML/Sanitizer/SanitizerAlgorithms` (.cpp, .hpp)
- `HTML/Sanitizer/SanitizerConfig` (.hpp)
- `HTML/Sanitizer/SanitizerConstants` (.cpp, .hpp)
- `HTML/Sanitizer/SetHTMLOptions` (.hpp)
- `HTML/Types/DocumentReadyState` (.hpp)
- `HTML/Types/DocumentVisibilityState` (.hpp)
- `HTML/Types/Namespace` (.hpp)
- `HTML/Types/TagName` (.hpp)
- `Infra/Namespaces` (.hpp)
- `Infra/StringAlgorithms` (.hpp)
- `MathML/Internals/MathMLElementFactory` (.cpp, .hpp)
- `MathML/MathMLNames` (.cpp, .hpp)
- `Page/IntersectionObserver/IntersectionObserverCallback` (.hpp)
- `Page/ResizeObserver/ResizeObserverBoxOptions` (.hpp)
- `Page/ResizeObserver/ResizeObserverCallback` (.hpp)
- `Page/ResizeObserver/ResizeObserverOptions` (.hpp)
- `Page/ScrollOptions` (.hpp)
- `Page/Settings` (.cpp, .hpp)
- `Platform/PathStrokeStrategy` (.hpp)
- `SVG/Internals/SVGElementFactory` (.cpp, .hpp)
- `SVG/SVGNames` (.cpp, .hpp)
- `Style/Builder/BuilderGenerated` (.hpp)
- `Style/Interpolation/ChangedAnimatablePropertiesGenerated` (.hpp)
- `Style/Invalidation/PseudoClassStateInvalidation` (.cpp, .hpp)
- `Style/Values/Text/HyphenateLimitChars` (.cpp, .hpp)
- `Text/EncodingRegistry` (.cpp, .hpp)
- `Types/IsEOF` (.hpp)
- `Types/SourceLocation` (.hpp)
- `XML/PseudoAttributes` (.cpp, .hpp)
