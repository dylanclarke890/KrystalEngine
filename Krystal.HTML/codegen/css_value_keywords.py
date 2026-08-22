import argparse
import re
import textwrap

from css_properties import output_cpp_path
from utils import output_hpp_path, run_gperf, Writer

GENERATOR_NAME = "codegen/css_value_keywords.py"


def generate(args: argparse.Namespace):
    parsing_context = ParsingContext(defines_string=args.defines, parsing_for_codegen=True, verbose=args.verbose)

    parsed_values: list[Value] = []
    with open(args.values, "r") as values_file:
        for line in values_file:
            # Remove any text after a "//" comment string is started.
            index = line.find("//")
            if index != -1:
                line = line[:index]

            # Remove any trailing whitespace.
            line = line.rstrip()

            # If the line is empty at this point, we can just skip it.
            if not line:
                continue

            # Parse the line into its constituent parts. A "value" and set of
            # attributes (currently only two attributes, "enable-if" and "id"
            # are supported).
            parts = line.split(" ")

            # The first part will always be the name.
            name = parts[0]

            # There may additionally be attributes of the form "foo=bar" after
            # the name.
            conditional = None
            id = None

            for attribute_string in parts[1:]:
                conditional = attribute_from_attribute_string(conditional, "enable-if", attribute_string, name)
                id = attribute_from_attribute_string(id, "id", attribute_string, name)

            if conditional and not parsing_context.is_enabled(conditional=conditional):
                if parsing_context.verbose:
                    print(
                        f"SKIPPED value {name} due to failing to satisfy 'enable-if' condition, '{conditional}', with active macro set"
                    )
                continue

            parsed_values.append(Value(name, id, conditional))

    if args.verbose:
        print(f"{len(parsed_values)} values active for code generation")

    generation_context = GenerationContext(parsed_values, verbose=args.verbose, gperf_executable=args.gperf_executable)
    generation_context.generate_css_value_id_hpp()
    generation_context.generate_css_value_id_gperf()
    run_gperf(
        gperf_executable=args.gperf_executable,
        filename="CSSValueId",
        output_cpp_dir=output_cpp_path("CSS/Values/Enums"),
        remove_gperf_file=True,
    )


class ParsingContext:
    def __init__(self, *, defines_string: str, parsing_for_codegen: bool, verbose: bool):
        if defines_string:
            self.conditionals = frozenset(defines_string.split(" "))
        else:
            self.conditionals = frozenset()
        self.parsing_for_codegen = parsing_for_codegen
        self.verbose = verbose

    def is_enabled(self, *, conditional: str) -> bool:
        if "|" in conditional:
            return any([self.is_enabled(conditional=c) for c in conditional.split("|")])
        if "&" in conditional:
            return all([self.is_enabled(conditional=c) for c in conditional.split("&")])
        if conditional[0] == "!":
            return conditional[1:] not in self.conditionals
        return conditional in self.conditionals


class Value:
    name: str
    id_without_prefix: str
    conditional: str | None

    def __init__(self, name: str, id_without_prefix: str | None, conditional: str | None):
        self.name = name
        self.id_without_prefix = id_without_prefix or Value.convert_name_to_id(name)
        self.conditional = conditional

    def __str__(self):
        if self.conditional:
            return f"Value [{self.name}, id={self.id_without_prefix}, conditional={self.conditional}]"
        return f"Value [{self.name}, id={self.id_without_prefix}]"

    def __repr__(self):
        return self.__str__()

    @staticmethod
    def convert_name_to_id(name):
        return re.sub(r"(^[^-])|-(.)", lambda m: (m[1] or m[2]).upper(), name)

    @property
    def name_lowercase(self):
        return self.name.lower()

    @property
    def id(self):
        return f"CSSValueId::{self.id_without_prefix}"


def attribute_from_attribute_string(
    attribute: str | None, attribute_name: str, attribute_string: str, value: str
) -> str | None:
    attribute_parts = attribute_string.partition("=")

    if attribute_parts[0] != attribute_name:
        return attribute

    if attribute:
        raise Exception(f"More than one '{attribute_name}' attribute specified for value '{value}'.")

    if not attribute_parts[2]:
        raise Exception(f"Empty '{attribute_name}' attribute specified for value '{value}'.")

    return attribute_parts[2]


# GENERATION


class GenerationContext:
    def __init__(self, values: list[Value], *, verbose: bool, gperf_executable: str):
        self.values = values
        self.verbose = verbose
        self.gperf_executable = gperf_executable

    # Shared generation constants.

    number_of_predefined_values = 1

    def generate_css_value_id_hpp(self):
        with open(output_hpp_path("Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"), "w") as output_file:
            writer = Writer(output_file)
            writer.hpp_prelude(
                generator_name=GENERATOR_NAME,
                headers=[
                    "Krystal.HTML/CSS/Parser/CSSParserContext.hpp",
                    "Krystal.HTML/CSS/Types/CSSOMString.hpp",
                ],
            )

            with writer.namespace(namespace="Krys::HTML"):
                self._generate_hpp_property_constants(to=writer)
                self._generate_hpp_forward_declarations(to=writer)
                self._generate_hpp_constant_aliases(to=writer)
            writer.newline()

            with writer.namespace(namespace="Krys"):
                writer.write_block("""\
                    template<> 
                    struct DefaultHash<::Krys::HTML::CSSValueId> : IntegerHash<uint16>
                    {
                    };

                    template<>
                    struct HashTraits<::Krys::HTML::CSSValueId> : StrongEnumHashTraits<::Krys::HTML::CSSValueId>
                    {
                    };""")
            writer.newline()

            with writer.namespace(namespace="std"):
                writer.write_block("""\
                    template<>
                    struct iterator_traits<::Krys::HTML::AllCSSValueKeywordsRange::Iterator>
                    {
                      using value_type = ::Krys::HTML::CSSValueId;
                    };""")

    def _generate_hpp_property_constants(self, *, to: Writer):
        with to.enum_class_block(name="CSSValueId", underlying_type="uint16"):
            to.enum_member(name="Invalid", value=0)
            count = GenerationContext.number_of_predefined_values
            max_length = 0
            for value in self.values:
                to.enum_member(name=value.id_without_prefix, value=count)
                count += 1
                max_length = max(len(value.name), max_length)

        last = count - 1

        to.write(f"constexpr uint16 NumCSSValueKeywords = {count};")
        to.write(f"constexpr uint16 LastCSSValueKeyword = {last};")
        to.write(f"constexpr size_t MaxCSSValueKeywordLength = {max_length};")
        to.newline()

    def _generate_hpp_forward_declarations(self, *, to: Writer):
        to.write_block("""\
            KRYS_NODISCARD CSSValueId FindCSSValueKeyword(CSSOMStringView characters) noexcept;

            KRYS_NODISCARD CSSOMStringView ToString(CSSValueId id) noexcept;
            
            /// @brief When serializing a CSS keyword, it should be converted to ASCII lowercase.
            /// @see https://drafts.csswg.org/cssom/#serialize-a-css-component-value
            KRYS_NODISCARD CSSOMStringView ToLowercaseString(CSSValueId id) noexcept;

            struct AllCSSValueKeywordsRange
            {
                struct Iterator
                {
                    uint16 Index {0u};
                    constexpr CSSValueId operator*() const noexcept
                    {
                      return static_cast<CSSValueId>(Index);
                    }

                    constexpr Iterator &operator++() noexcept
                    {
                      ++Index;
                      return *this;
                    }

                    KRYS_NODISCARD constexpr bool operator==(std::nullptr_t) const noexcept
                    {
                      return Index >= NumCSSValueKeywords;
                    }
                };

                KRYS_NODISCARD constexpr static Iterator begin() noexcept
                {
                  return {};
                }
                
                KRYS_NODISCARD constexpr static std::nullptr_t end() noexcept
                {
                  return nullptr;
                }

                KRYS_NODISCARD constexpr static uint16 size() noexcept
                {
                  return NumCSSValueKeywords;
                }
            };

            KRYS_NODISCARD constexpr AllCSSValueKeywordsRange AllCSSValueKeywords() noexcept
            {
              return {};
            }
            """)

    def _generate_hpp_constant_aliases(self, *, to: Writer):
        with to.template_struct_block(template_signature="CSSValueId C", name="ValueKeywordConstant"):
            to.write(f"constexpr static auto value = C;")
            to.write(f"constexpr bool operator==(const ValueKeywordConstant &) const noexcept = default;")
            to.write(f"constexpr bool operator==(CSSValueId other) const noexcept {{ return value == other; }}")
        to.newline()

        with to.namespace(namespace="Keywords"):
            for value in self.values:
                to.write(f"using {value.id_without_prefix} = ValueKeywordConstant<{value.id}>;")
                to.newline()

    def generate_css_value_id_gperf(self):
        with open("CSSValueId.gperf", "w") as output_file:
            writer = Writer(output_file)

            self._generate_gperf_prelude(to=writer)
            self._generate_gperf_definition(to=writer)
            self._generate_name_string_tables(to=writer)
            self._generate_lookup_functions(to=writer)
            writer.write("}")

    def _generate_gperf_prelude(self, *, to: Writer):
        with to.block(block_start="%{", block_end="%}", indent=False):
            to.cpp_prelude(
                for_header="Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp",
                generator_name=GENERATOR_NAME,
                headers=[
                    "Krystal.HTML/CSS/Parser/CSSParserContext.hpp",
                    "Krystal.HTML/CSS/Properties/CSSProperty.hpp",
                    "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp",
                    "Krystal.Lib/String/String.hpp",
                    "Krystal.Lib/Types/SmallList.hpp",
                ],
            )

            to.write_block("""\
                // Older versions of gperf like to use the `register` keyword.
                #define register""")

            to.write("namespace Krys::HTML")
            to.write("{")

    def _generate_gperf_definition(self, *, to: Writer):
        to.newline()
        to.write_block("""\
            %struct-type
            struct CSSValueHashTableEntry
            {
                const char* name;
                uint16 id;
            };
            %language=C++
            %readonly-tables
            %7bit
            %compare-strncmp
            %define class-name CSSValueKeywordsHash
            %enum""")

        to.newline()

        to.write("%%")
        for value in self.values:
            to.write(f"{value.name_lowercase}, {value.id_without_prefix}")
        to.write("%%")

    def _generate_name_string_tables(self, *, to: Writer):
        to.write(f"constexpr CSSOMStringView KeywordNamesList[NumCSSValueKeywords] = {{")
        with to.indent():
            to.write(f'u8"",')
            for value in self.values:
                to.write(f'u8"{value.name}",')
        to.write("};")

        to.write(f"constexpr CSSOMStringView KeywordNamesListLowercase[NumCSSValueKeywords] = {{")
        with to.indent():
            to.write(f'u8"",')
            for value in self.values:
                to.write(f'u8"{value.name_lowercase}",')
        to.write("};")

    def _generate_lookup_functions(self, *, to: Writer):
        to.write(textwrap.dedent("""
            CSSValueId FindCSSValueKeyword(CSSOMStringView characters) noexcept
            {
                auto* value = CSSValueKeywordsHash::in_word_set(reinterpret_cast<const char*>(characters.data()), characters.size());
                return value ? static_cast<CSSValueId>(value->id) : CSSValueId::Invalid;
            }

            CSSOMStringView ToString(CSSValueId id) noexcept
            {
                if (static_cast<uint16>(id) >= NumCSSValueKeywords)
                {
                    return {};
                }

                return KeywordNamesList[id];
            }

            CSSOMStringView ToLowercaseString(CSSValueId id) noexcept
            {
                if (static_cast<uint16>(id) >= NumCSSValueKeywords)
                {
                    return {};
                }

                return KeywordNamesListLowercase[id];
            }"""))
