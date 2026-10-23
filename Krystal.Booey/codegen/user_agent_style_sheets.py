import argparse
import os
import pathlib
import re

from utils import Writer

GENERATOR_NAME = "codegen/user_agent_style_sheets.py"

NAMESPACE = "krys::boo::css"
OUTPUT_DIR = "Krystal.Booey/CSS"
FILE_NAME = "UserAgentStyleSheets"

# The sheets in user-agent-style-sheets/, by the name of the variable that holds each one.
SHEETS = {
    "HTMLUserAgentStyleSheet": "html.css",
    "QuirksUserAgentStyleSheet": "quirks.css",
    "SVGUserAgentStyleSheet": "svg.css",
    "MathMLUserAgentStyleSheet": "mathml.css",
    "MathMLLegacyFontSizeMathUserAgentStyleSheet": "mathmlLegacyFontSizeMath.css",
    "CounterStylesUserAgentStyleSheet": "counterStyles.css",
    "PopoverUserAgentStyleSheet": "popover.css",
    "ViewTransitionsUserAgentStyleSheet": "viewTransitions.css",
    "HorizontalFormControlsUserAgentStyleSheet": "horizontalFormControls.css",
    "ThemeAdwaitaUserAgentStyleSheet": "themeAdwaita.css",
    "MeterElementShadowUserAgentStyleSheet": "meterElementShadow.css",
}


def minify(text: str) -> str:
    """Strips comments and collapses whitespace."""
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    text = re.sub(r"\s+", " ", text)
    return text.strip()


def generate(args: argparse.Namespace):
    sheets_dir = pathlib.Path(__file__).parent / "user-agent-style-sheets"

    output_hpp_dir = os.path.join(args.output_headers_dir, OUTPUT_DIR)
    output_cpp_dir = os.path.join(args.output_sources_dir, "CSS")
    os.makedirs(output_hpp_dir, exist_ok=True)
    os.makedirs(output_cpp_dir, exist_ok=True)

    with open(os.path.join(output_hpp_dir, f"{FILE_NAME}.hpp"), "w") as output_file:
        writer = Writer(output_file)
        writer.hpp_prelude(generator_name=GENERATOR_NAME, headers=["Krystal.Booey/CSS/Types/CSSOMString.hpp"])

        with writer.namespace(namespace=NAMESPACE):
            for name in SHEETS:
                writer.write(f"extern const CSSOMStringView {name};")

    with open(os.path.join(output_cpp_dir, f"{FILE_NAME}.cpp"), "w") as output_file:
        writer = Writer(output_file)
        writer.cpp_prelude(generator_name=GENERATOR_NAME, for_header=f"{OUTPUT_DIR}/{FILE_NAME}.hpp")

        with writer.namespace(namespace=NAMESPACE):
            for index, (name, file_name) in enumerate(SHEETS.items()):
                if index:
                    writer.newline()

                text = minify((sheets_dir / file_name).read_text(encoding="utf-8-sig"))
                data = text.encode("utf-8")
                if any(byte >= 128 for byte in data):
                    raise ValueError(f"{file_name} is not ASCII")

                writer.write(f"static constexpr char8 {name}Data[] = {{")
                with writer.indent():
                    for start in range(0, len(data), 16):
                        chunk = data[start : start + 16]
                        writer.write(", ".join(str(byte) for byte in chunk) + ",")
                writer.write("};")
                writer.write(f"const CSSOMStringView {name} {{{name}Data, sizeof({name}Data)}};")
