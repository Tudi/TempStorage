import re
import os

# Root directory to scan
NDPI_DIR = "./dependencies/nDPI/src"

# List of function names to preserve `static` for
SKIP_FUNCS = {
    "keep_extra_dissection",
    "check_content_type_and_change_protocol",
    "__get_master",
    "sha256_transform",
    "roaring_bitmap_remove_range",
    "roaring_bitmap_add_range",
    "roaring_bitmap_set_copy_on_write",
    "roaring_bitmap_get_copy_on_write",
    "roaring_bitmap_create",
    "bitset_print",
    "bitset_for_each",
    "bitset_next_set_bits",
    "bitset_get",
    "bitset_next_set_bit",
    "bitset_set_to_value",
    "bitset_set",
    "bitset_size_in_words",
    "bitset_size_in_bits",
    "bitset_size_in_bytes",
    "croaring_refcount_get",
    "croaring_refcount_dec",
    "croaring_refcount_inc",
    "roaring_hamming",
    "roaring_leading_zeroes",
    "roaring_trailing_zeroes",
    "roaring_bitmap_init_cleared",
    "is_ndpi_proto",
}

# Regex to match: static + optional whitespace/newlines + return + function name + (
FUNC_STATIC_PATTERN = re.compile(
    r'^[ \t]*static(?:\s|\n)+'            # static followed by space or newline
    r'([\w\s\*\n]+?)'                     # return type (possibly multiline)
    r'([a-zA-Z_][a-zA-Z0-9_]*)'           # function name
    r'(\s*\()',                           # opening parenthesis
    re.MULTILINE
)

FUNC_INLINE_PATTERN = re.compile(
    r'^[ \t]*inline(?:\s|\n)+'            # inline followed by space or newline
    r'([\w\s\*\n]+?)'                     # return type (possibly multiline)
    r'([a-zA-Z_][a-zA-Z0-9_]*)'           # function name
    r'(\s*\()',                           # opening parenthesis
    re.MULTILINE
)

# File extensions to include
INCLUDE_EXT = {'.c', '.h'}

for root, _, files in os.walk(NDPI_DIR):
    for name in files:
        if not any(name.endswith(ext) for ext in INCLUDE_EXT):
            continue

        path = os.path.join(root, name)

        with open(path, 'rb') as f:
            raw = f.read()
        try:
            content = raw.decode('utf-8')
        except UnicodeDecodeError:
            content = raw.decode('latin1')  # or 'windows-1252'

        original = content

        def replace_func(match):
            ret_type = match.group(1)
            func_name = match.group(2)
            paren = match.group(3)

            if func_name in SKIP_FUNCS:
                return f'static {ret_type}{func_name}{paren}'
            else:
                return f'{ret_type}{func_name}{paren}'

        content = FUNC_STATIC_PATTERN.sub(replace_func, content)
        content = FUNC_INLINE_PATTERN.sub(replace_func, content)

        if content != original:
            print(f"Modifying {path}")
            # Backup original
            with open(path + ".bak", 'w', encoding='utf-8') as backup:
                backup.write(original)
            # Write modified
            with open(path, 'wb') as f:
                f.write(content.encode('utf-8'))

print("Done.")
