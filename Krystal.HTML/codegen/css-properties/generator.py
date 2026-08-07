import pathlib
from utils import EnumValue, GeneratorContext, get_validated_json, kebab_case_to_pascal_case, generator_metadata

GENERATOR_NAME = "css-properties.generator"


class CSSProperty:
    id: str
    id_pascal_case: str
    initial: str | None
    inherited: bool
    parser_syntax: str
    longhand_names: list[str] | None
    specification_module: str
    specification_url: str

    @staticmethod
    def from_json_dict(property_name: str, property_data: dict) -> "CSSProperty":
        property = CSSProperty()
        property.id = property_name
        property.id_pascal_case = kebab_case_to_pascal_case(property_name)
        property.initial = property_data.get("initial")
        property.inherited = property_data.get("inherited", False)
        property.parser_syntax = property_data["codegen-properties"]["parser-syntax"]
        property.longhand_names = property_data["codegen-properties"].get("longhands")
        property.specification_module = property_data["specification"]["module"]
        property.specification_url = property_data["specification"]["url"]
        return property

    def get_enum_value(self) -> "EnumValue":
        comment = f"/// @syntax {self.parser_syntax}\n    /// @specification {self.specification_module} ({self.specification_url})"
        return EnumValue(self.id, self.id_pascal_case, comment)


def __generate_css_property_enum(
    context: GeneratorContext,
    all_properties: list[CSSProperty],
    shorthand_property_count: int,
):
    model: dict[str, object] = {
        "enum_comment": "/// @brief An enumeration of all CSS properties, including both shorthand and longhand properties.",
        "enum_name": "CSSPropertyId",
        "enum_values": [prop.get_enum_value() for prop in all_properties],
        "enum_type": "uint16",
        "enum_default": "Invalid",
        "enum_default_comment": "/// @brief Represents an invalid or unrecognized CSS property.",
        "additional_data": f"constexpr size_t MaxShorthandPropertyId = {shorthand_property_count}uz;",
    }
    model.update(generator_metadata(GENERATOR_NAME))

    template = context.template_env.get_template("enum.hpp.jinja")
    output = template.render(model=model)

    output_filepath = (
        context.project_root / "include" / "Krystal.HTML" / "CSS" / "Properties" / "Enums" / "CSSPropertyId.hpp"
    )
    with open(output_filepath, "w") as hpp_file:
        hpp_file.write(output)


class ShorthandToLonghandData:
    shorthand_name: str
    longhand_names: list[str]

    def __init__(self, shorthand: str, longhands: list[str]):
        self.shorthand_name = shorthand
        self.longhand_names = longhands


def __generate_shorthand_to_longhand_mapping(
    context: GeneratorContext, shorthand_properties: list[CSSProperty], longhand_properties: list[CSSProperty]
):
    shorthand_to_longhand_data: list[ShorthandToLonghandData] = []
    for prop in shorthand_properties:
        if prop.id == "all":
            longhand_names = [lh.id_pascal_case for lh in longhand_properties]
        else:
            longhand_names = [kebab_case_to_pascal_case(lh) for lh in prop.longhand_names]  # type: ignore
        shorthand_to_longhand_data.append(ShorthandToLonghandData(prop.id_pascal_case, longhand_names))

    model: dict[str, object] = {
        "shorthand_to_longhand_data": shorthand_to_longhand_data,
    }
    model.update(generator_metadata(GENERATOR_NAME))

    template = context.template_env.get_template("css-property-shorthand.cpp.jinja")
    output = template.render(model=model)

    output_dir = context.project_root / "src" / "CSS" / "Properties"
    with open(output_dir / f"CSSPropertyShorthand.cpp", "w") as cpp_file:
        cpp_file.write(output)


class LonghandToShorthandData:
    longhand_name: str
    shorthand_names: list[str]

    def __init__(self, longhand: str, shorthands: list[str]):
        self.longhand_name = longhand
        self.shorthand_names = shorthands


def generate(context: GeneratorContext):
    properties_json = get_validated_json( pathlib.Path(__file__).parent)

    shorthand_properties: list[CSSProperty] = []
    longhand_properties: list[CSSProperty] = []
    for property_name, property_data in properties_json["properties"].items():
        css_property = CSSProperty.from_json_dict(property_name, property_data)
        if css_property.longhand_names is not None:
            # If the property has longhands, it is a shorthand property.
            shorthand_properties.append(css_property)
        else:
            longhand_properties.append(css_property)
    all_properties = shorthand_properties + longhand_properties

    __generate_css_property_enum(context, all_properties, len(shorthand_properties))

    __generate_shorthand_to_longhand_mapping(context, shorthand_properties, longhand_properties)
