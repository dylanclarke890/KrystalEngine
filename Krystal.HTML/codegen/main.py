import datetime
import sys
import os
import importlib
import traceback

GENERATORS_TO_RUN = [
    # CSS
    "generators.css.properties",

    # HTML
]

def main():
    if (len(sys.argv) != 2):
        print("data root should be passed as an argument to the script.")
        sys.exit(1)

    data_root = sys.argv[1]
    print(f"Running with data root: {data_root}")
    
    # Add the 'codegen' folder to Python's path so imports work
    script_dir = os.path.dirname(os.path.abspath(__file__))
    sys.path.insert(0, script_dir)
    
    for module_name in GENERATORS_TO_RUN:
        print(f"Started running {module_name} at {datetime.datetime.now()}...")

        try:
            module = importlib.import_module(module_name)

            if hasattr(module, 'generate') is False:
                print(f"ERROR: {module_name} has no 'generate()' function.")
                sys.exit(1)

            module.generate(data_root)
        except Exception as e:
            print(f"ERROR: {module_name} encountered an error: {e}")
            traceback.print_exc()
            sys.exit(1)

        print(f"Finished running {module_name} at {datetime.datetime.now()}...")
    
    print(f"Finished at {datetime.datetime.now()}...")
    sys.exit(0)

if __name__ == "__main__":
    main()