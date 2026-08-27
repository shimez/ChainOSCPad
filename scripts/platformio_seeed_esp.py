"""Make Seeed's ESP tool packages available to its Python build commands."""

from __future__ import annotations

import os
from pathlib import Path

Import("env")

core_dir = Path(env.subst("$PROJECT_CORE_DIR"))
esptool_dir = core_dir / "tools" / "tool-esptoolpy"
if esptool_dir.is_dir():
    current = env["ENV"].get("PYTHONPATH", "")
    entries = [str(esptool_dir)]
    if current:
        entries.append(current)
    env["ENV"]["PYTHONPATH"] = os.pathsep.join(entries)
