#!/usr/bin/env python3
"""Generate C word-list fragments from official BIP-39 text lists."""

from __future__ import annotations

import argparse
from pathlib import Path

WORD_COUNT = 2048


def c_string(word: str) -> str:
    """Return a C string literal containing the word's UTF-8 bytes."""
    output = []
    for byte in word.encode("utf-8"):
        if 0x20 <= byte <= 0x7E and byte not in (ord('"'), ord("\\")):
            output.append(chr(byte))
        else:
            output.append(f"\\{byte:03o}")
    return '"' + "".join(output) + '"'


def generate(language: str, wordlist_dir: Path) -> None:
    source = wordlist_dir / f"{language}.txt"
    destination = wordlist_dir / f"{language}.inc"
    words = source.read_text(encoding="utf-8").splitlines()

    if len(words) != WORD_COUNT:
        raise ValueError(f"{source}: expected {WORD_COUNT} words, found {len(words)}")
    if any(not word for word in words):
        raise ValueError(f"{source}: empty word found")
    if len(set(words)) != WORD_COUNT:
        raise ValueError(f"{source}: duplicate word found")

    content = "".join(f"    {c_string(word)},\n" for word in words)
    destination.write_text(content, encoding="utf-8")
    print(f"Generated {destination}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--language", action="append")
    parser.add_argument(
        "--wordlist-dir",
        type=Path,
        default=Path(__file__).resolve().parent.parent / "common" / "wordlists",
    )
    args = parser.parse_args()

    languages = args.language or sorted(
        path.stem for path in args.wordlist_dir.glob("*.txt")
    )
    if not languages:
        raise ValueError(f"{args.wordlist_dir}: no .txt word lists found")
    for language in dict.fromkeys(languages):
        generate(language, args.wordlist_dir)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
