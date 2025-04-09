import os
import difflib
import unicodedata
import re

def format_large_number(n: int) -> str:
    n = int(n)
    if n >= 1_000_000:
        base = n / 1_000_000
        return f"{base:.2f} M"
    elif n >= 1_000:
        base = n / 1_000
        return f"{base:.2f} K"
    else:
        return str(n)

def normalize_html(text: str) -> str:
    if not text:
        return ''

    # Remove BOM if present
    text = text.lstrip('\ufeff')

    # Normalize Unicode (e.g., composed characters)
    text = unicodedata.normalize('NFKC', text)

    # Replace common HTML entities
    text = text.replace('&nbsp;', '')
    text = text.replace(' ', '')
    text = text.replace('\n', '')
    text = text.replace('\r', '')

    # Remove all invisible/zero-width/control characters (except newline)
    text = ''.join(
        ch for ch in text
        if not unicodedata.category(ch).startswith('C') or ch in ('\n',)
    )

    # Collapse all whitespace (space, tab, newline) to nothing
    text = re.sub(r'\s+', '', text)

    return text
    
def find_real_diff(a: str, b: str, is_silent: bool):
    diff_count = 0
    print_first_n = 10
    for i, (ca, cb) in enumerate(zip(a, b)):
        if ca != cb and is_silent == False:
            print(f"Diff at index {i}:")
            print(f"  current: {repr(ca)} ({ord(ca)})")
            print(f"  new    : {repr(cb)} ({ord(cb)})")
            print_first_n -= 1
            diff_count += 1
            if print_first_n == 0:
                return diff_count
        if ca != cb and is_silent == True:
            diff_count += 1
    if len(a) != len(b):
        print(f"Strings are same up to length {min(len(a), len(b))} but differ in length.")
        
    return diff_count
        
def write_file_if_changed(file_path: str, new_content: str) -> bool:
    """
    Writes new_content to file_path only if the content has changed.
    Returns True if the file was written, False if skipped.
    """
    current_content = None
    if os.path.exists(file_path):
        with open(file_path, 'r', encoding='utf-8') as file:
            current_content = file.read()

    '''
    normalized_current = normalize_html(current_content)
    normalized_new = normalize_html(new_content)
    
    diff_count = 0
    if normalized_current != normalized_new:
        # this is just for debugging
        find_real_diff(normalized_current, normalized_new, True)
        # count the number of differences
        diff_count = find_real_diff(normalized_current, normalized_new, False)       
        # Debugging difference
        print(f"File changed: {file_path}")
        with open("cur.txt", 'w', encoding='utf-8') as file:
            file.write(normalized_current)
        with open("new.txt", 'w', encoding='utf-8') as file:
            file.write(normalized_new)
        
    if diff_count > 10:
    '''     
    if current_content != new_content:
        with open(file_path, 'w', encoding='utf-8') as file:
            file.write(new_content)
        return True
    return False
