import argparse
import pathlib

import css_properties
import css_value_keywords


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Process CSS property definitions.")
    parser.add_argument(
        "--properties",
        default=pathlib.Path(__file__).parent / "css-properties.json",
        help="Path to the CSS properties JSON file.",
    )
    parser.add_argument(
        "--values",
        default=pathlib.Path(__file__).parent / "css-value-keywords.in",
        help="Path to the CSS value keywords input file.",
    )
    parser.add_argument("--defines", default="", help="Comma-separated list of defines to enable for code generation.")
    parser.add_argument("--gperf-executable", default="gperf", help="Path to the gperf executable.")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose output.")
    parser.add_argument("--dump-unused-grammars", action="store_true", help="Dump unused grammar rules.")
    parser.add_argument("--check-unused-grammars-values", action="store_true", help="Check unused grammar values.")

    return parser.parse_args()


def main():
    args = parse_args()
    css_value_keywords.generate(args)
    css_properties.generate(args)


if __name__ == "__main__":
    main()
