import datetime
import json
import pathlib
from jinja2 import Environment, FileSystemLoader
import jsonschema


def kebab_case_to_pascal_case(kebab_case: str) -> str:
    return "".join(word.capitalize() for word in kebab_case.split("-"))


def get_validated_json(dir: pathlib.Path):
    instance = json.loads((dir / "data.json").read_text(encoding='utf-8-sig'))
    schema = json.loads((dir / "data.schema.json").read_text(encoding='utf-8-sig'))
    jsonschema.validate(instance, schema)
    return instance


def time_stamp() -> str:
    return datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")


def generator_metadata(name: str) -> dict[str, object]:
    return {
        "generator_name": name,
        "generated_at": time_stamp(),
    }


class GeneratorContext:
    project_root: pathlib.Path
    codegen_dir: pathlib.Path
    template_dir: pathlib.Path
    template_env: Environment

    def __init__(self, project_root: pathlib.Path, codegen_dir: pathlib.Path):
        self.project_root = project_root
        self.codegen_dir = codegen_dir
        self.template_dir = codegen_dir / "templates"
        self.template_env = Environment(
            loader=FileSystemLoader(self.template_dir), trim_blocks=True, lstrip_blocks=True
        )


class EnumValue:
    str_name: str
    value_name: str
    comment: str | None

    def __init__(self, str_name: str, value_name: str, comment: str | None = None):
        self.str_name = str_name
        self.value_name = value_name
        self.comment = comment
