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


def __generate_css_property_id_enum(
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


class LonghandsToShorthandData:
    longhand_names: list[str]
    shorthand_names: list[str]

    def __init__(self, longhand: list[str], shorthands: list[str]):
        self.longhand_names = longhand
        self.shorthand_names = shorthands


def __generate_css_property_shorthand_files(
    context: GeneratorContext, shorthand_properties: list[CSSProperty], longhand_properties: list[CSSProperty]
):
    model: dict[str, object] = generator_metadata(GENERATOR_NAME)

    shorthand_to_longhand_data: dict[str, list[CSSProperty]] = {}
    longhand_to_shorthand_data: dict[str, list[CSSProperty]] = {}
    shorthand_to_longhand_count: dict[CSSProperty, int] = {}
    longhand_properties_by_name: dict[str, CSSProperty] = {prop.id: prop for prop in longhand_properties}

    for property in shorthand_properties:
        shorthand_to_longhand_count[property] = 0
        for longhand_name in property.longhand_names or []:
            if longhand_name == "all":
                for inner_property in longhand_properties:
                    if inner_property.id == "direction" or inner_property.id == "unicode-bidi":
                        continue
                    longhand_to_shorthand_data.setdefault(inner_property.id_pascal_case, []).append(property)
                    shorthand_to_longhand_count[property] += 1
            else:
                id = longhand_properties_by_name[longhand_name].id_pascal_case
                longhand_to_shorthand_data.setdefault(id, []).append(property)
                shorthand_to_longhand_count[property] += 1

        if property.id == "all":
            shorthand_to_longhand_data[property.id_pascal_case] = [
                inner_property
                for inner_property in longhand_properties
                if inner_property.id != "direction" and inner_property.id != "unicode-bidi"
            ]
        else:
            shorthand_to_longhand_data[property.id_pascal_case] = [
                longhand_properties_by_name[name] for name in property.longhand_names or []
            ]

    # https://drafts.csswg.org/cssom/#concept-shorthands-preferred-order
    def preferred_order_for_shorthands(x):
        return (-shorthand_to_longhand_count[x], x.id.startswith("-"), not x.id.startswith("-webkit-"), x.id)

    model.update(
        {
            "shorthand_to_longhand_data": [
                ShorthandToLonghandData(shorthand, [longhand.id_pascal_case for longhand in longhands])
                for shorthand, longhands in shorthand_to_longhand_data.items()
            ],
            "longhands_to_shorthand_data": [
                LonghandsToShorthandData([longhand], [shorthand.id_pascal_case for shorthand in sorted(shorthands, key=preferred_order_for_shorthands)])
                for longhand, shorthands in longhand_to_shorthand_data.items()
            ],
            "shorthand_to_longhand_count": shorthand_to_longhand_count,
        }
    )

    template = context.template_env.get_template("css-property-shorthand.cpp.jinja")
    output = template.render(model=model)

    output_filepath = context.project_root / "src" / "CSS" / "Properties" / "CSSPropertyShorthand.cpp"
    with open(output_filepath, "w") as cpp_file:
        cpp_file.write(output)


def generate(context: GeneratorContext):
    properties_json = get_validated_json(pathlib.Path(__file__).parent)

    shorthand_properties: list[CSSProperty] = []
    longhand_properties: list[CSSProperty] = []
    for property_name, property_data in properties_json["properties"].items():
        css_property = CSSProperty.from_json_dict(property_name, property_data)
        if css_property.longhand_names is not None:
            shorthand_properties.append(css_property)
        else:
            longhand_properties.append(css_property)
    all_properties = shorthand_properties + longhand_properties

    __generate_css_property_id_enum(context, all_properties, len(shorthand_properties))

    __generate_css_property_shorthand_files(context, shorthand_properties, longhand_properties)
