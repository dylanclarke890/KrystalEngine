add_executable(Krystal.Booey.Tests)

target_link_libraries(Krystal.Booey.Tests
  PRIVATE
	 	Krystal.Core
		Krystal.Booey
   	Catch2::Catch2WithMain
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
		tests/CSS/Parser/KeywordLookup.cpp
		tests/CSS/Parser/ParserFastPaths.cpp
		tests/CSS/Parser/Token.cpp
		tests/CSS/Parser/Tokenizer.cpp
		tests/CSS/Properties/Consumers/Color.cpp
		tests/CSS/Properties/Consumers/ColorAdjust.cpp
		tests/CSS/Properties/Consumers/Ident.cpp
		tests/CSS/Properties/Consumers/MetaConsumer.cpp
		tests/CSS/Properties/Consumers/Primitives.cpp
		tests/CSS/Values/Aggregates.cpp
		tests/CSS/Values/Color/Color.cpp
		tests/CSS/Values/NumericTypesSerialisation.cpp
		tests/CSS/Values/Primitives/NumericRaw.cpp
		tests/CSS/Values/Primitives/PrimitiveNumeric.cpp
		tests/CSS/Values/Primitives/Units.cpp
		tests/CSS/Values/PrimitiveValue.cpp
		tests/CSS/Values/Types.cpp
		tests/CSS/Values/Units.cpp
		# tests/CSS/Selectors/SelectorParser.cpp

		# Graphics
		tests/Graphics/FloatPoint.cpp
		tests/Graphics/FloatSize.cpp
		tests/Graphics/IntPoint.cpp
		tests/Graphics/IntSize.cpp
		tests/Graphics/LayoutPoint.cpp
		tests/Graphics/LayoutSize.cpp
		tests/Graphics/Color/Color.cpp
		tests/Graphics/Color/ColorBlending.cpp
		tests/Graphics/Color/ColorConversion.cpp
		tests/Graphics/Color/ColorInterpolation.cpp
		tests/Graphics/Color/ColorSerialisation.cpp
		tests/Graphics/Color/ColorUtilities.cpp
		tests/Graphics/Color/NamedColors.cpp

		# Style
		tests/Style/Calc/Tree.cpp
		tests/Style/ScopedName.cpp
		tests/Style/Values/Color/SystemColor.cpp
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
)
