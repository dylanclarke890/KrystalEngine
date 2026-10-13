add_executable(Krystal.Booey.Tests)

# Third-party - RapidJSON (WPT URL test data)
find_package(RapidJSON CONFIG REQUIRED)

target_link_libraries(Krystal.Booey.Tests
  PRIVATE
	 	Krystal.Core
		Krystal.Booey
   	Catch2::Catch2WithMain
		rapidjson
)

set_target_properties(Krystal.Booey.Tests
	PROPERTIES
  	CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
		COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Booey.Tests
	PUBLIC
  	cxx_std_23
)

target_compile_options(Krystal.Booey.Tests
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

target_compile_definitions(Krystal.Booey.Tests PRIVATE
    KRYS_BOOEY_TESTS_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

catch_discover_tests(Krystal.Booey.Tests)

target_precompile_headers(Krystal.Booey.Tests
  PRIVATE
    <algorithm>
    <chrono>
    <concepts>
    <format>
    <functional>
    <memory>
    <optional>
    <span>
    <ranges>
    <string>
    <string_view>
    <tuple>
    <type_traits>
    <variant>
    <vector>

    <catch2/catch_test_macros.hpp>
    <catch2/matchers/catch_matchers_floating_point.hpp>

    <Krystal.Booey/DOM/AbortSignal.hpp>
    <Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp>
    <Krystal.Booey/DOM/Attr.hpp>
    <Krystal.Booey/DOM/Document.hpp>
    <Krystal.Booey/DOM/EventTarget.hpp>
    <Krystal.Booey/DOM/Node.hpp>
    <Krystal.Booey/DOM/ShadowRoot.hpp>
    <Krystal.Booey/DOM/Types/DOMString.hpp>
    <Krystal.Booey/DOM/Types/ExceptionOr.hpp>
    <Krystal.Booey/DOM/Types/USVString.hpp>
    <Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp>
    <Krystal.Booey/HTML/HTMLSlotElement.hpp>
)

target_sources(Krystal.Booey.Tests
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
		include/Krystal.Booey.Tests/DOM/TestElement.hpp
		include/Krystal.Booey.Tests/CSS/ValueTestUtils.hpp
		include/Krystal.Booey.Tests/TestParserUtils.hpp
	PRIVATE
		# Core
		tests/Core/LayoutUnit.cpp
		tests/Core/NumberFormatting.cpp

		# CSS
		tests/CSS/Calc/CalcType.cpp
		tests/CSS/Calc/Executor.cpp
		tests/CSS/Calc/SymbolTable.cpp
		tests/CSS/Calc/Tree.cpp
		tests/CSS/Calc/TreeEvaluation.cpp
		tests/CSS/Calc/TreeParser.cpp
		tests/CSS/Calc/TreeSerialisation.cpp
		tests/CSS/Calc/TreeSimplification.cpp
		tests/CSS/Calc/Value.cpp
		# tests/CSS/Parser/Parser.cpp
		tests/CSS/Parser/CustomPropertySyntax.cpp
		tests/CSS/Parser/KeywordLookup.cpp
		tests/CSS/Parser/ParserContext.cpp
		tests/CSS/Parser/ParserFastPaths.cpp
		tests/CSS/Parser/Token.cpp
		tests/CSS/Parser/Tokenizer.cpp
		tests/CSS/Parser/VariableParser.cpp
		tests/CSS/Properties/Consumers/Color.cpp
		tests/CSS/Properties/Consumers/ColorAdjust.cpp
		tests/CSS/Properties/Consumers/Definitions.cpp
		tests/CSS/Properties/Consumers/Ident.cpp
		tests/CSS/Properties/Consumers/List.cpp
		tests/CSS/Properties/Consumers/MetaConsumer.cpp
		tests/CSS/Properties/Consumers/Percentage.cpp
		tests/CSS/Properties/Consumers/PrimitiveValue.cpp
		tests/CSS/Properties/Consumers/Primitives.cpp
		tests/CSS/Properties/Consumers/String.cpp
		tests/CSS/Properties/Property.cpp
		tests/CSS/Properties/PropertyParserResult.cpp
		tests/CSS/Properties/PropertyShorthand.cpp
		tests/CSS/Values/Aggregates.cpp
		tests/CSS/Values/Borders/BorderRadius.cpp
		tests/CSS/Values/Borders/BoxShadow.cpp
		tests/CSS/Values/Color/Color.cpp
		tests/CSS/Values/Easing/EasingFunction.cpp
		tests/CSS/Values/FilterEffects/AppleColorFilter.cpp
		tests/CSS/Values/FilterEffects/Filter.cpp
		tests/CSS/Values/Motion/RayFunction.cpp
		tests/CSS/Values/VariableData.cpp
		tests/CSS/Values/FontFaceSrcValue.cpp
		tests/CSS/Values/Images/Gradient.cpp
		tests/CSS/Values/Grid/GridTemplateAreas.cpp
		tests/CSS/Values/NumericTypesSerialisation.cpp
		tests/CSS/Values/Primitives/NumericRaw.cpp
		tests/CSS/Values/Primitives/PrimitiveNumeric.cpp
		tests/CSS/Values/Primitives/Position.cpp
		tests/CSS/Values/Primitives/Ratio.cpp
		tests/CSS/Values/Primitives/URL.cpp
		tests/CSS/Values/Primitives/Units.cpp
		tests/CSS/Values/PrimitiveValue.cpp
		tests/CSS/Values/Quad.cpp
		tests/CSS/Values/Shapes/BasicShape.cpp
		tests/CSS/Values/Shapes/PathFunction.cpp
		tests/CSS/Values/Shapes/ShapeFunction.cpp
		tests/CSS/Values/TextDecoration/TextShadow.cpp
		tests/CSS/Values/Types.cpp
		tests/CSS/Values/Units.cpp
		tests/CSS/Values/ValuePair.cpp
		tests/CSS/Values/ValueList.cpp
		tests/CSS/Values/FunctionValue.cpp
		tests/CSS/Values/QuadValue.cpp
		tests/CSS/Values/RectValue.cpp
		tests/CSS/Values/RatioValue.cpp
		tests/CSS/Values/BackgroundRepeatValue.cpp
		tests/CSS/Values/BorderImageSliceValue.cpp
		tests/CSS/Values/BorderImageWidthValue.cpp
		tests/CSS/Values/ReflectValue.cpp
		tests/CSS/Values/UnicodeRangeValue.cpp
		tests/CSS/Values/FontVariationValue.cpp
		tests/CSS/Values/FontFeatureValue.cpp
		tests/CSS/Values/FontStyleWithAngleValue.cpp
		tests/CSS/Values/FontStyleRangeValue.cpp
		tests/CSS/Values/FontValue.cpp
		tests/CSS/Values/GridLineNamesValue.cpp
		tests/CSS/Values/GridLineValue.cpp
		tests/CSS/Values/SubgridValue.cpp
		tests/CSS/Values/TransformListValue.cpp
		tests/CSS/Values/GridAutoRepeatValue.cpp
		tests/CSS/Values/GridIntegerRepeatValue.cpp
		tests/CSS/Values/CounterValue.cpp
		tests/CSS/Values/OffsetRotateValue.cpp
		tests/CSS/Values/AttrValue.cpp
		tests/CSS/Values/PositionValue.cpp
		tests/CSS/Values/UrlValue.cpp
		tests/CSS/Values/GridTemplateAreasValue.cpp
		tests/CSS/Values/EasingFunctionValue.cpp
		tests/CSS/Values/BoxShadowPropertyValue.cpp
		tests/CSS/Values/TextShadowPropertyValue.cpp
		tests/CSS/Values/PathValue.cpp
		tests/CSS/Values/BasicShapeValue.cpp
		tests/CSS/Values/FilterValue.cpp
		tests/CSS/Values/AppleColorFilterValue.cpp
		tests/CSS/Values/RayValue.cpp
		tests/CSS/Values/ScrollValue.cpp
		tests/CSS/Values/ViewValue.cpp
		tests/CSS/Values/CustomPropertyValue.cpp
		tests/CSS/Values/VariableReferenceValue.cpp
		tests/CSS/Values/ColorValue.cpp
		tests/CSS/Values/ColorSchemeValue.cpp
		tests/CSS/Values/DynamicRangeLimitValue.cpp
		tests/CSS/Values/NamedImageValue.cpp
		tests/CSS/Values/CanvasValue.cpp
		tests/CSS/Values/PaintImageValue.cpp
		tests/CSS/Values/ImageSetOptionValue.cpp
		tests/CSS/Values/ImageValue.cpp
		tests/CSS/Values/ImageSetValue.cpp
		tests/CSS/Values/CrossFadeValue.cpp
		tests/CSS/Values/FilterImageValue.cpp
		tests/CSS/Values/CursorImageValue.cpp
		tests/CSS/Values/GradientValue.cpp
		# tests/CSS/Selectors/SelectorParser.cpp

		# Graphics
		tests/Graphics/DoublePoint.cpp
		tests/Graphics/FloatPoint.cpp
		tests/Graphics/FloatPoint3D.cpp
		tests/Graphics/FloatQuad.cpp
		tests/Graphics/FloatRect.cpp
		tests/Graphics/FloatSize.cpp
		tests/Graphics/FontTaggedSettings.cpp
		tests/Graphics/GeometryUtilities.cpp
		tests/Graphics/IntPoint.cpp
		tests/Graphics/IntRect.cpp
		tests/Graphics/IntSize.cpp
		tests/Graphics/LayoutPoint.cpp
		tests/Graphics/LayoutRect.cpp
		tests/Graphics/LayoutSize.cpp
		tests/Graphics/Region.cpp
		tests/Graphics/Transforms/AffineTransform.cpp
		tests/Graphics/Color/Color.cpp
		tests/Graphics/Color/ColorBlending.cpp
		tests/Graphics/Color/ColorConversion.cpp
		tests/Graphics/Color/ColorInterpolation.cpp
		tests/Graphics/Color/ColorSerialisation.cpp
		tests/Graphics/Color/ColorUtilities.cpp
		tests/Graphics/Color/NamedColors.cpp
		tests/Graphics/Transforms/Quaternion.cpp
		tests/Graphics/Transforms/TransformExtras.cpp
		tests/Graphics/Transforms/TransformationMatrix.cpp
		tests/Rendering/Style/GridSpan.cpp

		# Style
		tests/Style/Calc/Tree.cpp
		tests/Style/ScopedName.cpp
		tests/Style/Values/Color/Color.cpp
		tests/Style/Values/Color/DynamicRangeLimit.cpp
		tests/Style/Values/Color/SystemColor.cpp
		tests/Style/Values/Grid/GridPosition.cpp
		tests/Style/Values/Primitives/LengthResolution.cpp
		tests/Style/Values/Primitives/LengthWrapper.cpp
		tests/Style/Values/Primitives/NumericTypesConversions.cpp
		tests/Style/Values/Primitives/NumericTypesEvaluation.cpp
		tests/Style/Values/Primitives/PrimitiveNumeric.cpp
		tests/Style/Values/Types.cpp

		# DOM
		tests/DOM/Algorithms/EventDispatcher.cpp
		tests/DOM/Algorithms/Mixins.cpp
		tests/DOM/Algorithms/MutationAlgorithms.cpp
		tests/DOM/Algorithms/NameValidation.cpp
		tests/DOM/Algorithms/OrderedSet.cpp
		tests/DOM/Algorithms/TreeQueries.cpp
		tests/DOM/Internals/Event/Event.cpp
		tests/DOM/Internals/Event/CustomEvent.cpp
		tests/DOM/Internals/EventFactory.cpp
		tests/DOM/Internals/QualifiedNameCache.cpp
		tests/DOM/Mixins/ChildNode.cpp
		tests/DOM/Mixins/NonDocumentTypeChildNode.cpp
		tests/DOM/Mixins/NonElementParentNode.cpp
		tests/DOM/Mixins/ParentNode.cpp
		tests/DOM/Attr.cpp
		tests/DOM/BoundaryPoint.cpp
		tests/DOM/CDATASection.cpp
		tests/DOM/CharacterData.cpp
		tests/DOM/Comment.cpp
		tests/DOM/ContainerNode.cpp
		tests/DOM/DocumentFragment.cpp
		tests/DOM/DocumentType.cpp
		tests/DOM/DOMImplementation.cpp
		tests/DOM/DOMTokenList.cpp
		tests/DOM/Element.cpp
		tests/DOM/HTMLCollection.cpp
		tests/DOM/MutationObserver.cpp
		tests/DOM/NamedNodeMap.cpp
		tests/DOM/Node.cpp
		tests/DOM/NodeIterator.cpp
		tests/DOM/ProcessingInstruction.cpp
		tests/DOM/Range.cpp
		tests/DOM/StaticRange.cpp
		tests/DOM/Text.cpp
		tests/DOM/TreeWalker.cpp

		# HTML
		tests/HTML/Attributes/Reflection.cpp
		tests/HTML/MicroParsers/Numbers.cpp
		tests/HTML/Parser/HTMLCharacterReference.cpp
		tests/HTML/Parser/HTMLDocumentParser.cpp
		# tests/HTML/Parser/HTMLEncodingSniffer.cpp
		tests/HTML/Parser/InputStream.cpp
		# tests/HTML/Parser/TextResourceDecoder.cpp
		tests/HTML/Parser/Tokenizer.cpp

		# Infra
		tests/Infra/StringAlgorithms.cpp

		# URL
		tests/SVG/Internals/SVGPath.cpp
		tests/URL/URL.cpp
		tests/URL/WPT.cpp
)
