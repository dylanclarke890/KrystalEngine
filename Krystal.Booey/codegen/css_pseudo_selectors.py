import argparse
import json
import os
import textwrap

from utils import run_gperf, Writer

GENERATOR_NAME = "codegen/css_pseudo_selectors.py"

NAMESPACE = "krys::boo::css"
SELECTORS_DIR = "Krystal.Booey/CSS/Selectors"

PSEUDO_CLASS_TYPE = "SelectorPseudoClass"
PSEUDO_ELEMENT_TYPE = "SelectorPseudoElement"

PSEUDO_CLASS_MAP_NAME = "SelectorPseudoClassAndCompatibilityElementMap"
PSEUDO_ELEMENT_MAP_NAME = "SelectorPseudoElementMap"

# Upstream's flags for these maps: try harder (-m 10) to find a small table.
GPERF_OPTIONS = ["--key-positions=*", "-m", "10", "-s", "2"]


def generate(args: argparse.Namespace):
    with open(args.pseudo_selectors, "r", encoding="utf-8-sig") as input_file:
        data = json.load(input_file)

    InputValidator(data)

    output_hpp_dir = os.path.join(args.output_headers_dir, SELECTORS_DIR)
    output_cpp_dir = os.path.join(args.output_sources_dir, "CSS/Selectors")
    os.makedirs(output_hpp_dir, exist_ok=True)
    os.makedirs(output_cpp_dir, exist_ok=True)

    pseudo_classes = data["pseudo-classes"]
    pseudo_elements = data["pseudo-elements"]

    generate_enums_hpp(os.path.join(output_hpp_dir, "SelectorEnums.hpp"), pseudo_classes, pseudo_elements)
    generate_inlines_hpp(os.path.join(output_hpp_dir, "SelectorInlines.hpp"), pseudo_classes, pseudo_elements)
    generate_user_agent_parts(
        os.path.join(output_hpp_dir, "UserAgentParts.hpp"),
        os.path.join(output_cpp_dir, "UserAgentParts.cpp"),
        pseudo_elements,
    )

    generate_pseudo_class_gperf(pseudo_classes, pseudo_elements)
    run_gperf(
        gperf_executable=args.gperf_executable,
        filename=PSEUDO_CLASS_MAP_NAME,
        output_cpp_dir=output_cpp_dir,
        remove_gperf_file=True,
        options=GPERF_OPTIONS,
    )

    generate_pseudo_element_gperf(pseudo_elements)
    run_gperf(
        gperf_executable=args.gperf_executable,
        filename=PSEUDO_ELEMENT_MAP_NAME,
        output_cpp_dir=output_cpp_dir,
        remove_gperf_file=True,
        options=GPERF_OPTIONS,
    )


# MARK: - Input validation.

KNOWN_KEY_TYPES = {
    "pseudo-classes": {
        "argument": [str],
        "comment": [str],
        "settings-flag": [str],
    },
    "pseudo-elements": {
        "argument": [str],
        "comment": [str],
        "settings-flag": [str],
        "supports-single-colon-for-compatibility": [bool],
        "user-agent-part": [bool],
        "user-agent-part-string": [bool],
        "user-agent-sheet-only": [bool],
    },
}


class InputValidator:
    def __init__(self, data):
        self.check_field_documentation(data)
        self.validate_fields(data, "pseudo-classes")
        self.validate_fields(data, "pseudo-elements")

    def check_field_documentation(self, data):
        documentation = data["documentation"]["fields"]
        for pseudo_type in KNOWN_KEY_TYPES:
            for field in KNOWN_KEY_TYPES[pseudo_type]:
                if field not in documentation:
                    raise Exception(f'Missing documentation for field "{field}" found in "{pseudo_type}".')

    def validate_fields(self, data, pseudo_type):
        for pseudo_name, pseudo_data in data[pseudo_type].items():
            # Only standard, unprefixed pseudos are supported, but for the internal user agent parts.
            if pseudo_name.startswith("-") and not key_is_true(pseudo_data, "user-agent-sheet-only"):
                raise Exception(f'"{pseudo_name}" is prefixed; only standard pseudos are supported.')
            if key_is_true(pseudo_data, "user-agent-sheet-only") and not key_is_true(pseudo_data, "user-agent-part"):
                raise Exception(f'"{pseudo_name}" is only for user agent sheets but is not a user agent part.')

            for key, value in pseudo_data.items():
                if key not in KNOWN_KEY_TYPES[pseudo_type]:
                    raise Exception(f'Unknown field "{key}" for {pseudo_type} found in "{pseudo_name}".')

                expected_types = KNOWN_KEY_TYPES[pseudo_type][key]
                if type(value) not in expected_types:
                    raise Exception(
                        f'Invalid value type {type(value)} for "{key}" in "{pseudo_name}". Expected type in set "{expected_types}".'
                    )

                if key == "argument":
                    if value not in ["required", "optional"]:
                        raise Exception(
                            f'Invalid "argument" field in "{pseudo_name}". The field should either be absent, "required" or "optional".'
                        )

                    if key_is_true(pseudo_data, "user-agent-part"):
                        raise Exception('Setting "argument" along with "user-agent-part": true is not supported.')

                if key == "supports-single-colon-for-compatibility":
                    allowlist = ["after", "before", "first-letter", "first-line"]
                    if pseudo_name not in allowlist:
                        raise Exception(
                            f'Unexpected "supports-single-colon-for-compatibility" field in "{pseudo_name}". This should never be set on new pseudo-elements.'
                        )

                if key == "user-agent-part-string" and key_is_true(pseudo_data, "user-agent-part"):
                    raise Exception('Setting "user-agent-part-string" along with "user-agent-part": true is not supported.')


# MARK: - Helpers.


def key_is_true(object, key):
    return object.get(key, False)


def format_name_for_enum_class(pseudo_name: str) -> str:
    return "".join(substring.capitalize() for substring in pseudo_name.split("-"))


def format_name_for_function(pseudo_name: str) -> str:
    return format_name_for_enum_class(pseudo_name)


def enum_values(pseudos) -> list[str]:
    return [
        format_name_for_enum_class(pseudo_name)
        for pseudo_name, pseudo_data in pseudos.items()
        if not key_is_true(pseudo_data, "user-agent-part")
    ]


def pseudo_element_enum_values(pseudo_elements) -> list[str]:
    return enum_values(pseudo_elements) + ["UserAgentPart"]


# MARK: - SelectorEnums.hpp


def generate_enums_hpp(path, pseudo_classes, pseudo_elements):
    with open(path, "w") as output_file:
        writer = Writer(output_file)
        writer.hpp_prelude(generator_name=GENERATOR_NAME, headers=["Krystal.Core/Enum.hpp"])

        class_values = enum_values(pseudo_classes)
        element_values = pseudo_element_enum_values(pseudo_elements)

        with writer.namespace(namespace=NAMESPACE):
            write_enum_class(writer, PSEUDO_CLASS_TYPE, class_values)
            writer.newline()
            write_enum_class(writer, PSEUDO_ELEMENT_TYPE, element_values)
        writer.newline()

        writer.write(f"KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(::{NAMESPACE}::{PSEUDO_CLASS_TYPE}, {len(class_values)}uz);")
        writer.write(f"KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(::{NAMESPACE}::{PSEUDO_ELEMENT_TYPE}, {len(element_values)}uz);")


def write_enum_class(writer: Writer, type_name: str, values: list[str]):
    with writer.enum_class_block(name=type_name, underlying_type="uint8"):
        for value in values:
            writer.enum_member(name=value)


# MARK: - SelectorInlines.hpp


def generate_inlines_hpp(path, pseudo_classes, pseudo_elements):
    with open(path, "w") as output_file:
        writer = Writer(output_file)
        writer.hpp_prelude(
            generator_name=GENERATOR_NAME,
            headers=[
                f"{SELECTORS_DIR}/Selector.hpp",
                f"{SELECTORS_DIR}/SelectorParserContext.hpp",
                "Krystal.Core/Text/ASCII.hpp",
            ],
        )

        with writer.namespace(namespace=NAMESPACE):
            write_is_pseudo_class_enabled(writer, pseudo_classes)
            writer.newline()
            write_is_pseudo_element_enabled(writer, pseudo_elements)
            writer.newline()
            write_selector_text_for_pseudo_class(writer, pseudo_classes)
            writer.newline()
            write_pseudo_argument_check_function(
                writer, "PseudoClassRequiresArgument", PSEUDO_CLASS_TYPE, ["required"], pseudo_classes
            )
            writer.newline()
            write_pseudo_argument_check_function(
                writer, "PseudoElementRequiresArgument", PSEUDO_ELEMENT_TYPE, ["required"], pseudo_elements
            )
            writer.newline()
            write_pseudo_argument_check_function(
                writer, "PseudoClassMayHaveArgument", PSEUDO_CLASS_TYPE, ["required", "optional"], pseudo_classes
            )
            writer.newline()
            write_pseudo_argument_check_function(
                writer, "PseudoElementMayHaveArgument", PSEUDO_ELEMENT_TYPE, ["required", "optional"], pseudo_elements
            )


def format_enablement_condition(settings_flag: str) -> str:
    return f"context.{settings_flag}"


def pseudo_element_enablement_condition(pseudo_data) -> str | None:
    if "settings-flag" in pseudo_data:
        return format_enablement_condition(pseudo_data["settings-flag"])
    if key_is_true(pseudo_data, "user-agent-sheet-only"):
        return "IsUASheetBehavior(context.Mode)"
    return None


def write_case(writer: Writer, *, case: str, body: list[str]):
    writer.write(f"case {case}:")
    with writer.indent():
        writer.write_lines(body)


def write_default_case(writer: Writer, *, body: list[str]):
    writer.write("default:")
    with writer.indent():
        writer.write_lines(body)


def write_is_pseudo_class_enabled(writer: Writer, pseudo_classes):
    flagged = {name: data for name, data in pseudo_classes.items() if "settings-flag" in data}
    if not flagged:
        signature = f"inline bool Selector::IsPseudoClassEnabled({PSEUDO_CLASS_TYPE}, const SelectorParserContext &) noexcept"
        with writer.function_block(signature=signature):
            writer.write("return true;")
        return

    signature = f"inline bool Selector::IsPseudoClassEnabled({PSEUDO_CLASS_TYPE} type, const SelectorParserContext &context) noexcept"
    with writer.function_block(signature=signature), writer.switch_block(expr="type"):
        for pseudo_name, pseudo_data in flagged.items():

            write_case(
                writer,
                case=f"{PSEUDO_CLASS_TYPE}::{format_name_for_enum_class(pseudo_name)}",
                body=[f"return {format_enablement_condition(pseudo_data['settings-flag'])};"],
            )

        write_default_case(writer, body=["return true;"])


def write_is_pseudo_element_enabled(writer: Writer, pseudo_elements):
    signature = f"inline bool Selector::IsPseudoElementEnabled({PSEUDO_ELEMENT_TYPE} type, CSSOMStringView name, const SelectorParserContext &context) noexcept"
    with writer.function_block(signature=signature), writer.switch_block(expr="type"):
        user_agent_parts = {}

        for pseudo_name, pseudo_data in pseudo_elements.items():
            enablement_condition = pseudo_element_enablement_condition(pseudo_data)
            if enablement_condition is None:
                continue

            # User agent parts share one enum value, so they are told apart by name.
            if key_is_true(pseudo_data, "user-agent-part"):
                user_agent_parts[pseudo_name] = enablement_condition
                continue

            write_case(
                writer,
                case=f"{PSEUDO_ELEMENT_TYPE}::{format_name_for_enum_class(pseudo_name)}",
                body=[f"return {enablement_condition};"],
            )

        writer.write(f"case {PSEUDO_ELEMENT_TYPE}::UserAgentPart:")
        with writer.indent():
            for pseudo_name, enablement_condition in user_agent_parts.items():
                condition = f'!{enablement_condition} && text::IsASCIICaselessEqual(name, CSSOMStringView {{u8"{pseudo_name}"}})'
                with writer.block(prologue=f"if ({condition})"):
                    writer.write("return false;")
            writer.write("return true;")

        write_default_case(writer, body=["return true;"])


def write_selector_text_for_pseudo_class(writer: Writer, pseudo_classes):
    signature = f"inline CSSOMStringView Selector::SelectorTextForPseudoClass({PSEUDO_CLASS_TYPE} type) noexcept"
    with writer.function_block(signature=signature), writer.switch_block(expr="type"):
        for pseudo_name in pseudo_classes:
            write_case(
                writer,
                case=f"{PSEUDO_CLASS_TYPE}::{format_name_for_enum_class(pseudo_name)}",
                body=[f'return u8":{pseudo_name}";'],
            )

        write_default_case(writer, body=['krys_debug_assert_msg(false, "Unknown pseudo-class");', 'return u8"";'])


def write_pseudo_argument_check_function(writer: Writer, function_name, enum_name, argument_types, pseudos):
    cases = [
        format_name_for_enum_class(pseudo_name)
        for pseudo_name, pseudo_data in pseudos.items()
        if pseudo_data.get("argument") in argument_types
    ]

    signature = f"inline bool Selector::{function_name}({enum_name} type) noexcept"
    with writer.function_block(signature=signature), writer.switch_block(expr="type"):
        if cases:
            writer.write_lines(f"case {enum_name}::{case}:" for case in cases)
            with writer.indent():
                writer.write("return true;")

        write_default_case(writer, body=["return false;"])


# MARK: - UserAgentParts.hpp/.cpp


def user_agent_parts_from(pseudo_elements):
    return [
        pseudo_name
        for pseudo_name, pseudo_data in pseudo_elements.items()
        if "user-agent-part" in pseudo_data or "user-agent-part-string" in pseudo_data
    ]


def generate_user_agent_parts(hpp_path, cpp_path, pseudo_elements):
    user_agent_parts = user_agent_parts_from(pseudo_elements)

    with open(hpp_path, "w") as output_file:
        writer = Writer(output_file)
        writer.hpp_prelude(generator_name=GENERATOR_NAME, headers=["Krystal.Booey/CSS/Types/CSSOMString.hpp"])

        with writer.namespace(namespace=f"{NAMESPACE}::UserAgentParts"):
            for part_name in user_agent_parts:
                writer.write(f"KRYS_NODISCARD const CSSOMStringAtom &{format_name_for_function(part_name)}() noexcept;")

    with open(cpp_path, "w") as output_file:
        writer = Writer(output_file)
        writer.cpp_prelude(generator_name=GENERATOR_NAME, for_header=f"{SELECTORS_DIR}/UserAgentParts.hpp")

        with writer.namespace(namespace=f"{NAMESPACE}::UserAgentParts"):
            for index, part_name in enumerate(user_agent_parts):
                if index:
                    writer.newline()

                function_name = format_name_for_function(part_name)
                with writer.function_block(signature=f"const CSSOMStringAtom &{function_name}() noexcept"):
                    writer.write(f'static const CSSOMStringAtom part {{u8"{part_name}"}};')
                    writer.write("return part;")


# MARK: - gperf maps


def write_gperf_prelude(writer: Writer, *, entry_struct: str):
    with writer.block(block_start="%{", block_end="%}", indent=False):
        writer.cpp_prelude(
            generator_name=GENERATOR_NAME,
            for_header=f"{SELECTORS_DIR}/SelectorPseudoTypeMap.hpp",
            headers=["Krystal.Core/Detection/Compiler.hpp"],
        )

        writer.write_block("""\
            // Older versions of gperf like to use the `register` keyword.
            #define register

            KRYS_DISABLE_WARNING_PUSH()
            #if KRYS_COMPILER(MSVC)
            // gperf's hash narrows the key length from size_t to unsigned int.
            KRYS_DISABLE_WARNING(4267)
            #endif
            """)

        writer.write(f"namespace {NAMESPACE}")
        writer.write("{")
        writer.write_block(entry_struct)


def write_gperf_definition(writer: Writer, *, initializer_suffix: str, class_name: str, struct_name: str, entries):
    writer.write_block(f"""\
        %struct-type
        %define initializer-suffix ,{initializer_suffix}
        %define class-name {class_name}
        %omit-struct-type
        %language=C++
        %readonly-tables
        %global-table
        %ignore-case
        %compare-strncmp
        %enum

        struct {struct_name};""")

    writer.write("%%")
    writer.write_lines(entries)
    writer.write("%%")


def generate_pseudo_class_gperf(pseudo_classes, pseudo_elements):
    def pseudo_class(name):
        return f"{PSEUDO_CLASS_TYPE}::{format_name_for_enum_class(name)}"

    def pseudo_element(name):
        return f"{PSEUDO_ELEMENT_TYPE}::{format_name_for_enum_class(name)}"

    entries = [f"\"{name}\", {{{pseudo_class(name)}, null}}" for name in pseudo_classes]
    # Pseudo-elements that can also be written with a single colon, as in CSS 2.1.
    entries += [
        f"\"{name}\", {{null, {pseudo_element(name)}}}"
        for name, data in pseudo_elements.items()
        if key_is_true(data, "supports-single-colon-for-compatibility")
    ]

    with open(f"{PSEUDO_CLASS_MAP_NAME}.gperf", "w") as output_file:
        writer = Writer(output_file)
        write_gperf_prelude(
            writer,
            entry_struct="""\
                struct SelectorPseudoClassOrCompatibilityPseudoElementEntry
                {
                  const char *name;
                  PseudoClassOrCompatibilityPseudoElement pseudoTypes;
                };""",
        )
        write_gperf_definition(
            writer,
            initializer_suffix="{null,null}",
            class_name="SelectorPseudoClassAndCompatibilityElementMapHash",
            struct_name="SelectorPseudoClassOrCompatibilityPseudoElementEntry",
            entries=entries,
        )

        writer.write(textwrap.dedent("""
            PseudoClassOrCompatibilityPseudoElement FindPseudoClassAndCompatibilityElementName(CSSOMStringView name) noexcept
            {
              auto *entry = SelectorPseudoClassAndCompatibilityElementMapHash::in_word_set(reinterpret_cast<const char *>(name.data()), name.size());
              if (entry)
              {
                return entry->pseudoTypes;
              }

              return {null, null};
            }"""))
        writer.write("}")
        writer.newline()
        writer.write("KRYS_DISABLE_WARNING_POP()")


def generate_pseudo_element_gperf(pseudo_elements):
    entries = []
    for name, data in pseudo_elements.items():
        if key_is_true(data, "user-agent-part"):
            entries.append(f"\"{name}\", {PSEUDO_ELEMENT_TYPE}::UserAgentPart")
        else:
            entries.append(f"\"{name}\", {PSEUDO_ELEMENT_TYPE}::{format_name_for_enum_class(name)}")

    with open(f"{PSEUDO_ELEMENT_MAP_NAME}.gperf", "w") as output_file:
        writer = Writer(output_file)
        write_gperf_prelude(
            writer,
            entry_struct=f"""\
                struct SelectorPseudoTypeEntry
                {{
                  const char *name;
                  Maybe<{PSEUDO_ELEMENT_TYPE}> type;
                }};""",
        )
        write_gperf_definition(
            writer,
            initializer_suffix="null",
            class_name="SelectorPseudoElementMapHash",
            struct_name="SelectorPseudoTypeEntry",
            entries=entries,
        )

        writer.write(textwrap.dedent(f"""
            Maybe<{PSEUDO_ELEMENT_TYPE}> FindPseudoElementName(CSSOMStringView name) noexcept
            {{
              if (auto *entry = SelectorPseudoElementMapHash::in_word_set(reinterpret_cast<const char *>(name.data()), name.size()))
              {{
                return entry->type;
              }}

              return null;
            }}"""))
        writer.write("}")
        writer.newline()
        writer.write("KRYS_DISABLE_WARNING_POP()")
