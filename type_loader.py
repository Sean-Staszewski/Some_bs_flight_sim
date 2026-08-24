import importlib.util
import inspect
import sys
from pathlib import Path


def load_subclass(type_dir, base_class):
    """Finds the single .py file in type_dir and returns the base_class subclass defined in it."""
    py_files = sorted(Path(type_dir).glob("*.py"))
    if not py_files:
        raise FileNotFoundError(f"No .py file found in {type_dir}")

    module_file = py_files[0]
    spec = importlib.util.spec_from_file_location(module_file.stem, module_file)
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module  # so inspect/pickle can trace the class back to its file
    spec.loader.exec_module(module)

    for _, obj in inspect.getmembers(module, inspect.isclass):
        if issubclass(obj, base_class) and obj is not base_class:
            return obj

    raise ImportError(f"No subclass of {base_class.__name__} found in {module_file}")
