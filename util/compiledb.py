#!/usr/bin/env python3
"""Build a clangd compile_commands.json at the repo root from QMK's one.

keymap.c is never compiled directly (keymap_introspection.c #includes it),
so keymap .c files QMK didn't compile get an entry reusing that TU's flags.
"""
import json
import sys
from pathlib import Path

qmk_dir, repo_dir, keymap_dir = (Path(p).resolve() for p in sys.argv[1:4])

entries = json.loads((qmk_dir / "compile_commands.json").read_text())

for e in entries:
    e["file"] = str((Path(e["directory"]) / e["file"]).resolve())

host = next(e for e in entries if e["file"].endswith("quantum/keymap_introspection.c"))
known = {e["file"] for e in entries}
for src in sorted(keymap_dir.glob("*.c")):
    if str(src) not in known:
        entries.append({**host, "file": str(src)})

out = repo_dir / "compile_commands.json"
out.write_text(json.dumps(entries, indent=2) + "\n")
print(f"==> Wrote {out.relative_to(repo_dir)} ({len(entries)} entries)")
