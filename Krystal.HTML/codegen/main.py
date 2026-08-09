import pathlib
import sys
import importlib
import traceback
from utils import GeneratorContext, time_stamp


def get_project_root_from_args() -> str:
    if len(sys.argv) != 2:
        print("Project root should be passed as an argument to the script.")
        sys.exit(1)

    # MSVC adds a trailing quote to the argument when passed from the command line. Remove it if present.
    project_root = sys.argv[1]
    if project_root.endswith('"'):
        project_root = project_root[:-1]

    return project_root


def run_generator(module_name: str, context: GeneratorContext):
    try:
        module = importlib.import_module(module_name)

        if hasattr(module, "generate") is False:
            print(f"ERROR: {module_name} has no 'generate()' function.")
            sys.exit(1)

        module.generate(context)
    except Exception as e:
        print(f"ERROR: {module_name} encountered an error: {e}")
        traceback.print_exc()
        sys.exit(1)


def main():
    project_root = get_project_root_from_args()
    GENERATORS_TO_RUN = [
        # CSS
        "css-properties.generator",
        "css-values.generator",
        # HTML
    ]

    context: GeneratorContext = GeneratorContext(pathlib.Path(project_root), pathlib.Path(__file__).parent)
    for module_name in GENERATORS_TO_RUN:
        print(f"[{time_stamp()}]: Started running {module_name}...")
        run_generator(module_name, context)
        print(f"[{time_stamp()}]: Finished running {module_name}.")

    print(f"[{time_stamp()}]: Finished running all generators.")
    sys.exit(0)


if __name__ == "__main__":
    main()
