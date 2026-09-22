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
		# CSS
		# tests/CSS/Parser/Parser.cpp
		# tests/CSS/Parser/Tokenizer.cpp
		# tests/CSS/Selectors/SelectorParser.cpp

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
