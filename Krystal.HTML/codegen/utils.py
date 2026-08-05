import datetime
import json
import pathlib
from jinja2 import Environment, FileSystemLoader
import jsonschema

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
    comment: str

    def __init__(self, str_name: str, value_name: str, comment: str):
        self.str_name = str_name
        self.value_name = value_name
        self.comment = comment


def time_stamp() -> str:
    return datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")


def get_validated_data(dir: pathlib.Path):
    instance = json.loads((dir / "data.json").read_text())
    schema = json.loads((dir / "data.schema.json").read_text())
    jsonschema.validate(instance, schema)
    return instance


def kebab_case_to_pascal_case(kebab_case: str) -> str:
    return "".join(word.capitalize() for word in kebab_case.split("-"))
