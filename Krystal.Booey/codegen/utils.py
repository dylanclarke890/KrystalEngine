import enum
import datetime
import io
import os
import pathlib
import subprocess
import shutil
import textwrap
from typing import Iterable

PROJECT_BASE = pathlib.Path(__file__).parent.parent
PROJECT_HPP_BASE = PROJECT_BASE / "include"
PROJECT_CPP_BASE = PROJECT_BASE / "src"


def output_hpp_path(relative_path: str) -> str:
    path = str(PROJECT_HPP_BASE / relative_path)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    return path


def output_cpp_path(relative_path: str) -> str:
    path = str(PROJECT_CPP_BASE / relative_path)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    return path


def run_gperf(*, gperf_executable: str, filename: str, output_cpp_dir: str, remove_gperf_file: bool):
    """Runs gperf on the specified .gperf file to produce a .cpp file."""

    gperf_result_code = subprocess.call(
        [
            gperf_executable,
            "--key-positions=*",
            "-D",
            "-n",
            "-s",
            "2",
            f"{filename}.gperf",
            f"--output-file={filename}.cpp",
        ]
    )

    if gperf_result_code != 0:
        raise Exception(f"Error when generating {filename}.cpp from {filename}.gperf: {gperf_result_code}")

    # move the generated file to the correct output path (made generating it easier)
    output_path = os.path.join(output_cpp_dir, f"{filename}.cpp")
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    shutil.move(f"{filename}.cpp", output_path)

    if remove_gperf_file:
        os.remove(f"{filename}.gperf")


def time_stamp() -> str:
    return datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")


class StringEqualingEnum(enum.Enum):
    def __eq__(self, b):
        if isinstance(b, str):
            return self.name == b
        else:
            return self.name == b.name

    def __hash__(self):
        return id(self.name)

class Writer:
    TAB_SIZE = 2

    class Block:
        def __init__(
            self,
            *,
            to: Writer,
            prologue: str = "",
            epilogue: str = "",
            block_start: str = "{",
            block_end: str = "}",
            indent: bool = True,
        ):
            self.to = to
            self.prologue = prologue
            self.epilogue = epilogue
            self.block_start = block_start
            self.block_end = block_end
            self.indent = indent

        def __enter__(self):
            if self.prologue:
                self.to.write_block(self.prologue)

            if self.block_start:
                self.to.write(self.block_start)

            if self.indent:
                self.to._indentation_level += 1

        def __exit__(self, exc_type, exc_value, traceback):
            if self.indent:
                self.to._indentation_level -= 1

            if self.block_end:
                self.to.write(self.block_end)

            if self.epilogue:
                self.to.write_block(self.epilogue)

    class Namespace:
        def __init__(self, *, to: Writer, namespace: str):
            self.to = to
            self.namespace = namespace

        def __enter__(self):
            if not self.namespace:
                self.to.write("namespace")
            else:
                self.to.write(f"namespace {self.namespace}")

            self.to.write("{")
            self.to._indentation_level += 1

        def __exit__(self, exc_type, exc_value, traceback):
            self.to._indentation_level -= 1
            self.to.write("}")

    def __init__(self, output: io.TextIOWrapper):
        self.output = output
        self._indentation_level = 0
        self._time_stamp = time_stamp()

    @property
    def _current_indent(self):
        return (self._indentation_level * Writer.TAB_SIZE) * " "

    def newline(self):
        self.output.write("\n")
        return self

    def write(self, text: str):
        self.output.write(self._current_indent)
        self.output.write(text)
        return self.newline()

    def write_lines(self, iterable: Iterable[str]):
        for line in iterable:
            self.write(line)
        return self

    def write_block(self, text: str):
        self.output.write(textwrap.indent(textwrap.dedent(text), self._current_indent))
        return self.newline()

    def write_if(self, condition: bool, text: str):
        if condition:
            self.write(text)
        return self

    def block(
        self,
        *,
        prologue: str = "",
        epilogue: str = "",
        block_start: str = "{",
        block_end: str = "}",
        indent: bool = True,
    ):
        return Writer.Block(
            to=self, prologue=prologue, epilogue=epilogue, block_start=block_start, block_end=block_end, indent=indent
        )

    def namespace(self, *, namespace: str):
        return Writer.Namespace(to=self, namespace=namespace)

    def indent(self):
        return Writer.Block(to=self, prologue="", block_start="", block_end="")

    def function_block(self, *, signature: str):
        return Writer.Block(to=self, prologue=signature)

    def lambda_block(self, *, signature: str):
        return Writer.Block(to=self, prologue=signature, block_start="{", block_end="};")

    def switch_block(self, *, expr: str):
        return Writer.Block(to=self, prologue=f"switch ({expr})")

    def case_block(self, *, case: str):
        return Writer.Block(to=self, prologue=f"case {case}:")

    def multi_case_block(self, *, cases: list[str]):
        prologue = "\n".join(f"case {case}:" for case in cases)
        return Writer.Block(to=self, prologue=prologue)

    def default_case_block(self):
        return Writer.Block(to=self, prologue="default:")

    def struct_block(self, *, name: str):
        return Writer.Block(to=self, prologue=f"struct {name}", block_end="};")

    def template_struct_block(self, *, template_signature: str, name: str):
        self.write(f"template<{template_signature}>")
        return Writer.Block(to=self, prologue=f"struct {name}", block_end="};")

    def class_block(self, *, name: str):
        return Writer.Block(to=self, prologue=f"class {name}", block_end="};")

    def template_class_block(self, *, template_signature: str, name: str):
        self.write(f"template<{template_signature}>")
        return Writer.Block(to=self, prologue=f"class {name}", block_end="};")

    def enum_class_block(self, *, name: str, underlying_type: str = "uint8"):
        return Writer.Block(to=self, prologue=f"enum class {name} : {underlying_type}", block_end="};")

    def enum_member(self, *, name: str, value: int | None = None, comment: str | None = None):
        if comment is not None:
            self.write_block(comment)

        if value is not None:
            self.write(f"{name} = {value},")
        else:
            self.write(f"{name},")

    def hpp_prelude(self, *, generator_name: str, headers: list[str] = [], system_headers: list[str] = []):
        self.autogenerated_heading(generator_name=generator_name)
        self.write("#pragma once")
        self.newline()
        self.includes(headers=headers, system_headers=system_headers)

    def cpp_prelude(
        self, *, generator_name: str, for_header: str, headers: list[str] = [], system_headers: list[str] = []
    ):
        self.autogenerated_heading(generator_name=generator_name)
        self.includes(headers=[for_header] + headers, system_headers=system_headers)

    def autogenerated_heading(self, *, generator_name: str):
        """Adds the standard auto-generated heading to the top of a generated file."""
        self.write(
            f"// <auto-generated>\n"
            f"//   Generated at {self._time_stamp} by {generator_name}\n"
            f"//   Do NOT make any changes to this file. Any changes made to this file will be lost when the code is\n"
            f"//   regenerated.\n"
            f"// </auto-generated>"
        )

    def includes(self, *, headers: list[str] = [], system_headers: list[str] = []):
        if not headers and not system_headers:
            return

        if headers:
            for header in headers:
                self.write(f'#include "{header}"')

        if system_headers:
            for header in system_headers:
                self.write(f"#include <{header}>")

        self.newline()

    def using_namespace_declarations(self, *, namespaces: list[str]):
        for namespace in namespaces:
            self.write(f"using namespace {namespace};")
        self.newline()

    def forward_declarations(self, *, structs: list[str] = [], classes: list[str] = [], namespace: str | None = None):
        if namespace:
            self.write(f"namespace {namespace}")
            self.write("{")
            self._indentation_level += 1

        for structName in structs:
            self.write(f"struct {structName};")
        if structs and classes:
            self.newline()
        for className in classes:
            self.write(f"class {className};")
        self.newline()

        if namespace:
            self._indentation_level -= 1
            self.write("}")
