#!/usr/bin/env python3
"""Validate design doc ship list matches the regression roster."""
from __future__ import annotations
import pathlib
import re
import sys
import zipfile

REPO_ROOT = pathlib.Path(__file__).resolve().parents[1]
DOC_PATH = REPO_ROOT / "Galactic Miners.docx"
TEST_PATH = REPO_ROOT / "tests" / "game_test_combat.cpp"

# Map the human-readable ship names from the design document to ship ids
# used by the codebase.
DOC_SHIPS = {
    "Transport Vessel": "SHIP_TRANSPORT",
    "Corvette": "SHIP_CORVETTE",
    "Shield Ship": "SHIP_SHIELD",
    "Radar Ship": "SHIP_RADAR",
    "Salvage Ship": "SHIP_SALVAGE",
    "Repair Drone": "SHIP_REPAIR_DRONE",
    "Sunflare Sloop": "SHIP_SUNFLARE_SLOOP",
    "Interceptor": "SHIP_INTERCEPTOR",
    "Celestial Juggernaut": "SHIP_CAPITAL_JUGGERNAUT",
    "Nova Carrier": "SHIP_CAPITAL_NOVA",
    "Obsidian Sovereign": "SHIP_CAPITAL_OBSIDIAN",
    "Preemptor": "SHIP_CAPITAL_PREEMPTOR",
    "Aurora Protector": "SHIP_CAPITAL_PROTECTOR",
    "Eclipse Monolith": "SHIP_CAPITAL_ECLIPSE",
    "Juggernaut Frigate": "SHIP_FRIGATE_JUGGERNAUT",
    "Carrier Frigate": "SHIP_FRIGATE_CARRIER",
    "Sovereign Frigate": "SHIP_FRIGATE_SOVEREIGN",
    "Preemptor Frigate": "SHIP_FRIGATE_PREEMPTOR",
    "Protector Frigate": "SHIP_FRIGATE_PROTECTOR",
    "Eclipse Frigate": "SHIP_FRIGATE_ECLIPSE",
}


def extract_doc_text() -> str:
    try:
        with zipfile.ZipFile(DOC_PATH) as docx:
            raw = docx.read("word/document.xml")
    except FileNotFoundError:
        raise SystemExit(f"Design document not found: {DOC_PATH}")
    text = raw.decode("utf-8", errors="ignore")
    # Strip XML tags to work with plain text.
    return re.sub(r"<[^>]+>", "", text)


def get_doc_ship_ids(text: str) -> set[str]:
    doc_ids = set()
    for name, ship_id in DOC_SHIPS.items():
        if name not in text:
            print(f"Missing ship entry in design doc: {name}")
        else:
            doc_ids.add(ship_id)
    missing_names = [name for name in DOC_SHIPS if name not in text]
    if missing_names:
        raise SystemExit(1)
    return doc_ids


def get_test_ship_ids() -> list[str]:
    contents = TEST_PATH.read_text(encoding="utf-8")
    match = re.search(
        r"design_doc_ships\s*\[\s*\]\s*=\s*\{(.*?)\};",
        contents,
        re.DOTALL,
    )
    if not match:
        raise SystemExit("Could not locate design_doc_ships array in tests.")
    block = match.group(1)
    return re.findall(r"\{\s*(SHIP_[A-Z0-9_]+)", block)


def main() -> int:
    text = extract_doc_text()
    doc_ship_ids = get_doc_ship_ids(text)
    test_ship_ids = get_test_ship_ids()

    missing = sorted(doc_ship_ids - set(test_ship_ids))
    extra = sorted(set(test_ship_ids) - doc_ship_ids)

    status = 0
    if missing:
        print("Ships listed in the design document but missing from tests:")
        for ship_id in missing:
            print(f"  - {ship_id}")
        status = 1
    if extra:
        print("Ships present in tests but not documented in design doc:")
        for ship_id in extra:
            print(f"  - {ship_id}")
        status = 1

    if status == 0:
        print("Design document ships match the regression roster.")
    return status


if __name__ == "__main__":
    sys.exit(main())
