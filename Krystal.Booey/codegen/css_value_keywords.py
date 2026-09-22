import argparse
import re
import textwrap

import os
from utils import run_gperf, Writer

GENERATOR_NAME = "codegen/css_value_keywords.py"


def generate(args: argparse.Namespace):
    parsing_context = ParsingContext(defines_string=args.defines, parsing_for_codegen=True, verbose=args.verbose)

    parsed_values: list[Value] = []
    for value_file_path in args.values:
        with open(value_file_path, "r") as values_file:
            extract_values_from_file(parsing_context, parsed_values, values_file)

    if args.verbose:
        print(f"{len(parsed_values)} values active for code generation")

    output_hpp_dir = args.output_headers_dir

    generation_context = GenerationContext(parsed_values, verbose=args.verbose, gperf_executable=args.gperf_executable, output_hpp_dir=output_hpp_dir)
    generation_context.generate_css_value_id_hpp()
    generation_context.generate_css_value_id_gperf()

    output_cpp_dir = args.output_sources_dir
    run_gperf(
        gperf_executable=args.gperf_executable,
        filename="ValueId",
        output_cpp_dir=os.path.join(output_cpp_dir, "CSS/Values"),
        remove_gperf_file=True,
    )


def extract_values_from_file(parsing_context, parsed_values, values_file):
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
        return f"ValueId::{self.id_without_prefix}"


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
    def __init__(self, values: list[Value], *, verbose: bool, gperf_executable: str, output_hpp_dir: str):
        self.values = values
        self.verbose = verbose
        self.gperf_executable = gperf_executable
        self.output_hpp_dir = output_hpp_dir

    # Shared generation constants.

    number_of_predefined_values = 1

    def generate_css_value_id_hpp(self):
        with open(os.path.join(self.output_hpp_dir, "Krystal.Booey/CSS/Values/ValueId.hpp"), "w") as output_file:
            writer = Writer(output_file)
            writer.hpp_prelude(
                generator_name=GENERATOR_NAME,
                headers=[
                    "Krystal.Booey/CSS/Parser/ParserContext.hpp",
                    "Krystal.Booey/CSS/Types/CSSOMString.hpp",
                ],
            )

            with writer.namespace(namespace="krys::boo::css"):
                self._generate_hpp_property_constants(to=writer)
                self._generate_hpp_forward_declarations(to=writer)
                self._generate_hpp_constant_aliases(to=writer)
            writer.newline()

            with writer.namespace(namespace="krys"):
                writer.write_block("""\
                    template<> 
                    struct DefaultHash<::krys::boo::css::ValueId> : IntegerHash<uint16>
                    {
                    };

                    template<>
                    struct HashTraits<::krys::boo::css::ValueId> : StrongEnumHashTraits<::krys::boo::css::ValueId>
                    {
                    };""")
            writer.newline()

            with writer.namespace(namespace="std"):
                writer.write_block("""\
                    template<>
                    struct iterator_traits<::krys::boo::css::AllValueKeywordsRange::Iterator>
                    {
                      using value_type = ::krys::boo::css::ValueId;
                    };""")

    def _generate_hpp_property_constants(self, *, to: Writer):
        with to.enum_class_block(name="ValueId", underlying_type="uint16"):
            to.enum_member(name="Invalid", value=0)
            count = GenerationContext.number_of_predefined_values
            max_length = 0
            for value in self.values:
                to.enum_member(name=value.id_without_prefix, value=count)
                count += 1
                max_length = max(len(value.name), max_length)

        last = count - 1

        to.write(f"constexpr uint16 TotalValueKeywords = {count};")
        to.write(f"constexpr uint16 LastValueKeyword = {last};")
        to.write(f"constexpr size_t MaxValueKeywordLength = {max_length};")
        to.newline()

    def _generate_hpp_forward_declarations(self, *, to: Writer):
        to.write_block("""\
            KRYS_NODISCARD ValueId FindValueKeyword(CSSOMStringView characters) noexcept;

            KRYS_NODISCARD CSSOMStringView ToString(ValueId id) noexcept;
            
            /// @brief When serializing a CSS keyword, it should be converted to ASCII lowercase.
            /// @see https://drafts.csswg.org/cssom/#serialize-a-css-component-value
            KRYS_NODISCARD CSSOMStringView ToLower(ValueId id) noexcept;

            struct AllValueKeywordsRange
            {
                struct Iterator
                {
                    uint16 Index {0u};
                    constexpr ValueId operator*() const noexcept
                    {
                      return static_cast<ValueId>(Index);
                    }

                    constexpr Iterator &operator++() noexcept
                    {
                      ++Index;
                      return *this;
                    }

                    KRYS_NODISCARD constexpr bool operator==(std::nullptr_t) const noexcept
                    {
                      return Index >= TotalValueKeywords;
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
                  return TotalValueKeywords;
                }
            };

            KRYS_NODISCARD constexpr AllValueKeywordsRange AllValueKeywords() noexcept
            {
              return {};
            }
            """)

    def _generate_hpp_constant_aliases(self, *, to: Writer):
        with to.template_struct_block(template_signature="ValueId C", name="KeywordValueConstant"):
            to.write(f"constexpr static auto value = C;")
            to.write(f"constexpr bool operator==(const KeywordValueConstant &) const noexcept = default;")
            to.write(f"constexpr bool operator==(ValueId other) const noexcept {{ return value == other; }}")
        to.newline()

        with to.namespace(namespace="keywords"):
            for value in self.values:
                to.write(f"using {value.id_without_prefix} = KeywordValueConstant<{value.id}>;")
                to.newline()

    def generate_css_value_id_gperf(self):
        with open("ValueId.gperf", "w") as output_file:
            writer = Writer(output_file)

            self._generate_gperf_prelude(to=writer)
            self._generate_gperf_definition(to=writer)
            self._generate_name_string_tables(to=writer)
            self._generate_lookup_functions(to=writer)
            writer.write("}")

    def _generate_gperf_prelude(self, *, to: Writer):
        with to.block(block_start="%{", block_end="%}", indent=False):
            to.cpp_prelude(
                for_header="Krystal.Booey/CSS/Values/ValueId.hpp",
                generator_name=GENERATOR_NAME,
                headers=[
                    "Krystal.Booey/CSS/Parser/ParserContext.hpp",
                    "Krystal.Booey/CSS/Properties/Property.hpp",
                    "Krystal.Core/Types/SmallList.hpp",
                    "Krystal.Core/Types/String.hpp",
                ],
            )

            to.write_block("""\
                // Older versions of gperf like to use the `register` keyword.
                #define register""")

            to.write("namespace krys::boo::css")
            to.write("{")

    def _generate_gperf_definition(self, *, to: Writer):
        to.newline()
        to.write_block("""\
            %struct-type
            struct ValueHashTableEntry
            {
                const char* name;
                uint16 id;
            };
            %language=C++
            %readonly-tables
            %7bit
            %compare-strncmp
            %define class-name ValueKeywordsHash
            %enum""")

        to.newline()

        to.write("%%")
        for value in self.values:
            to.write(f"{value.name_lowercase}, static_cast<uint16>({value.id})")
        to.write("%%")

    def _generate_name_string_tables(self, *, to: Writer):
        to.write(f"constexpr CSSOMStringView KeywordNamesList[TotalValueKeywords] = {{")
        with to.indent():
            to.write(f'u8"",')
            for value in self.values:
                to.write(f'u8"{value.name}",')
        to.write("};")

        to.write(f"constexpr CSSOMStringView KeywordNamesListLowercase[TotalValueKeywords] = {{")
        with to.indent():
            to.write(f'u8"",')
            for value in self.values:
                to.write(f'u8"{value.name_lowercase}",')
        to.write("};")

    def _generate_lookup_functions(self, *, to: Writer):
        to.write(textwrap.dedent("""
            ValueId FindValueKeyword(CSSOMStringView characters) noexcept
            {
                auto* value = ValueKeywordsHash::in_word_set(reinterpret_cast<const char*>(characters.data()), characters.size());
                return value ? static_cast<ValueId>(value->id) : ValueId::Invalid;
            }

            CSSOMStringView ToString(ValueId id) noexcept
            {
                if (static_cast<uint16>(id) >= TotalValueKeywords)
                {
                    return {};
                }

                return KeywordNamesList[static_cast<uint16>(id)];
            }

            CSSOMStringView ToLower(ValueId id) noexcept
            {
                if (static_cast<uint16>(id) >= TotalValueKeywords)
                {
                    return {};
                }

                return KeywordNamesListLowercase[static_cast<uint16>(id)];
            }"""))
