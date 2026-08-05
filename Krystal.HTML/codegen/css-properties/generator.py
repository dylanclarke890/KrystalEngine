import pathlib
from utils import GeneratorContext, EnumValue, get_validated_data, kebab_case_to_pascal_case, time_stamp


def generate_css_property_enum(
    context: GeneratorContext,
    all_properties: list[EnumValue],
    shorthand_properties: list[EnumValue],
):
    enum_template = context.template_env.get_template("enum.hpp.jinja")
    enum_data: dict[str, object] = {
        "enum_comment": "/// @brief An enumeration of all CSS properties, including both shorthand and longhand properties.",
        "enum_name": "CSSPropertyId",
        "enum_values": [value.__dict__ for value in all_properties],
        "enum_type": "uint16",
        "enum_default": "Invalid",
        "enum_default_comment": "/// @brief Represents an invalid or unrecognized CSS property.",
        "additional_data": f"constexpr size_t MaxShorthandPropertyId = {len(shorthand_properties)}uz;",
        "generated_at": time_stamp(),
    }

    output_dir = context.project_root / "include" / "Krystal.HTML" / "CSS" / "Properties" / "Enums"
    output = enum_template.render(model=enum_data)
    with open(output_dir / f"CSSPropertyId.hpp", "w") as hpp_file:
        hpp_file.write(output)


def generate(context: GeneratorContext):
    module_dir: pathlib.Path = pathlib.Path(__file__).parent
    data = get_validated_data(module_dir)
    shorthand_properties: list[EnumValue] = []
    longhand_properties: list[EnumValue] = []

    for property_name, property_data in data["properties"].items():
        comment = f"/// @syntax {property_data['codegen-properties']['parser-syntax']}\n    /// @see {property_data['specification']['url']}"
        enum_value = EnumValue(property_name, kebab_case_to_pascal_case(property_name), comment)
        if "longhands" in property_data.get("codegen-properties", {}):
            # If the property has longhands, it is a shorthand property.
            shorthand_properties.append(enum_value)
        else:
            longhand_properties.append(enum_value)

    all_properties = shorthand_properties + longhand_properties

    generate_css_property_enum(context, all_properties, shorthand_properties)
