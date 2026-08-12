from utils import EnumValue, GeneratorContext, generator_metadata, get_validated_json
import pathlib

GENERATOR_NAME = "css-values.generator"

class CSSKeyword:
    id: str
    id_pascal_case: str

    @staticmethod
    def from_json_dict(keyword_name: str) -> "CSSKeyword":
        keyword = CSSKeyword()
        keyword.id = keyword_name
        keyword.id_pascal_case = "".join(word.capitalize() for word in keyword_name.split("-"))
        return keyword

    def get_enum_value(self) -> "EnumValue":
        return EnumValue(self.id, self.id_pascal_case)

def generate(context: GeneratorContext):
    values_json = get_validated_json(pathlib.Path(__file__).parent)
    all_keywords = [CSSKeyword.from_json_dict(keyword) for keyword in values_json["keywords"]]

    model: dict[str, object] = {
        "enum_comment": "/// @brief An enumeration of all CSS value identifiers.",
        "enum_name": "CSSValueId",
        "enum_values": [keyword.get_enum_value() for keyword in all_keywords],
        "enum_type": "uint16",
        "enum_default": "Invalid",
        "enum_default_comment": "/// @brief Represents an invalid or unrecognized CSS keyword.",
    }
    model.update(generator_metadata(GENERATOR_NAME))

    template = context.template_env.get_template("enum.hpp.jinja")
    output = template.render(model=model)

    output_filepath = (
        context.project_root / "include" / "Krystal.HTML" / "CSS" / "Values" / "Enums" / "CSSValueId.hpp"
    )
    with open(output_filepath, "w") as hpp_file:
        hpp_file.write(output)

    model = {
        "keywords": [keyword.id_pascal_case for keyword in all_keywords],
    }
    model.update(generator_metadata(GENERATOR_NAME))

    template = context.template_env.get_template("css-keyword-constants.hpp.jinja")
    output = template.render(model=model)

    output_filepath = (
        context.project_root / "include" / "Krystal.HTML" / "CSS" / "Values" / "CSSKeywordConstants.hpp"
    )
    with open(output_filepath, "w") as hpp_file:
        hpp_file.write(output)
