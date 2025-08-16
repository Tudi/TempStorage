#!/bin/bash

# Path to nDPI
NDPI_DIR="./dependencies/nDPI"
PROTO_FILE="$NDPI_DIR/src/lib/ndpi_main.c"

# Whitelist of dissectors you want to keep
WHITELIST=("tls", "quic", "dns")

# Make backup
cp "$PROTO_FILE" "$PROTO_FILE.bak"

# Build a regex from the whitelist
regex=$(printf "|^%s$" "${WHITELIST[@]}")
regex=${regex:1}  # Remove leading '|'

awk -v regex="$regex" '
{
  if ($0 ~ /init_[a-zA-Z0-9_]+_dissector\s*\(.*\);/) {
    match($0, /init_([a-zA-Z0-9_]+)_dissector/, m);
    proto = m[1];
    if (proto !~ ("^(" regex ")$")) {
      print "// " $0;
      next;
    }
  }
  print;
}
' "$PROTO_FILE.bak" > "$PROTO_FILE"