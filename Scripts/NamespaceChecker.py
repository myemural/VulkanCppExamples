import os
import re
import sys

# Root namespace
ROOT_NAMESPACE = "examples"

# Source directory that you want to check
SRC_DIR = "../Examples"

def pascal_to_snake(name: str) -> str:
    """
    It converts PascalCase to snake_case
    Example: VertexBuffer -> vertex_buffer
    """
    s1 = re.sub("(.)([A-Z][a-z]+)", r"\1_\2", name)
    s2 = re.sub("([a-z0-9])([A-Z])", r"\1_\2", s1)
    result = s2.lower()

    # Fix postfixes like 2D, 3D etc.
    result = re.sub(r'(_?)([23])_d$', r'_\2d', result)
    return result


def expected_namespace(file_path: str) -> str:
    """
    It returns the expected namespace which respect to given file path.
    Then it converts names from pascal_case to snake_case.
    Example: Fundamentals/Basics/ClearScreenWithColor/VulkanApplication.h ->
             fundamentals::basics::clear_screen_with_color
    """
    rel_path = os.path.relpath(file_path, SRC_DIR)
    parts = rel_path.split(os.sep)

    # Extract file name
    if parts:
        parts.pop()

    # Convert namespace parts into pascal case
    parts = [pascal_to_snake(p) for p in parts]

    if parts:
        return ROOT_NAMESPACE + "::" + "::".join(parts)
    else:
        return ROOT_NAMESPACE


def extract_namespace_from_file(file_path: str) -> str | None:
    """
    Find namespace from given file and return first found one.
    Otherwise, return None.
    """
    namespace_pattern = re.compile(r'namespace\s+([a-zA-Z0-9_:]+)\s*') # namespace fundamental::basics
    with open(file_path, "r", encoding="utf-8") as f:
        for line in f:
            match = namespace_pattern.search(line)
            if match:
                return match.group(1)
    return None


def check_file(file_path: str) -> bool:
    expected = expected_namespace(file_path)
    actual = extract_namespace_from_file(file_path)

    if actual is None:
        print(f"❌ {file_path}: Namespace not found (expected: {expected})")
        return False

    if actual != expected:
        print(f"❌ {file_path}: Namespaces are not match (expected: {expected}, found: {actual})")
        return False

    print(f"✅ {file_path}: Expected namespace found ({expected})")
    return True


def main():
    failed = False
    for root, _, files in os.walk(SRC_DIR):
        for file in files:
            if file.endswith((".h", ".hpp", ".cpp")):
                path = os.path.join(root, file)
                if path.endswith("Main.cpp"):
                    continue
                if not check_file(path):
                    failed = True

    if failed:
        sys.exit(1)


if __name__ == "__main__":
    main()