"""Walks WebKit's #include graph from a set of entry files to find what needs porting.

Text-based, no preprocessor: includes inside #if blocks are followed but tagged as conditional.
Only files under FOLLOW_PREFIXES (or --follow) are traversed; every other include is a boundary, listed but not
followed.
"""

import argparse
import collections
import pathlib
import re
import sys

DEFAULT_WEBKIT_ROOT = pathlib.Path(r"C:\Users\dylan\repos\WebKit")

# Paths are relative to Source/WebCore. Only these are traversed.
FOLLOW_PREFIXES = ("css/", "style/")

# Never index these (test fixtures shadow generated headers like CSSPropertyNames.h).
EXCLUDE_PREFIXES = ("css/scripts/test/",)

# Header names that exist in more than one place in WebCore, resolved by hand: name -> path.
AMBIGUOUS_OVERRIDES: dict[str, str] = {
    "config.h": "config.h",
    "Logging.h": "platform/Logging.h",
}

INCLUDE_RE = re.compile(r'^\s*#\s*(?:include|import)\s*([<"])([^>"]+)[>"]')
IF_RE = re.compile(r"^\s*#\s*if")
ENDIF_RE = re.compile(r"^\s*#\s*endif")

# Header guards / pragma-once style #ifndef at file scope shouldn't mark everything as conditional.
GUARD_RE = re.compile(r"^\s*#\s*ifndef\s+\w+_h\b")


class Resolver:
    def __init__(self, webkit_root: pathlib.Path):
        self.webcore = webkit_root / "Source" / "WebCore"
        self.wtf = webkit_root / "Source" / "WTF"
        self.by_name: dict[str, list[str]] = collections.defaultdict(list)
        for path in sorted(self.webcore.rglob("*.h")):
            rel = path.relative_to(self.webcore).as_posix()
            if not rel.startswith(EXCLUDE_PREFIXES):
                self.by_name[path.name].append(rel)

    def resolve(self, bracket: str, target: str) -> tuple[str, str]:
        """Returns (kind, key). kind is one of: webcore, wtf, external, system, unresolved, ambiguous."""
        if target.startswith("wtf/"):
            return "wtf", target
        if target.startswith("WebCore/"):
            target = target.removeprefix("WebCore/")
        elif bracket == "<":
            return ("external", target) if "/" in target else ("system", target)

        name = target.rsplit("/", 1)[-1]
        if name in AMBIGUOUS_OVERRIDES:
            return "webcore", AMBIGUOUS_OVERRIDES[name]

        candidates = self.by_name.get(name, [])
        if "/" in target:
            candidates = [c for c in candidates if c.endswith(target)]
        if not candidates:
            return "unresolved", target
        if len(candidates) > 1:
            return "ambiguous", f"{target} -> {', '.join(candidates)}"
        return "webcore", candidates[0]


def scan_includes(path: pathlib.Path) -> list[tuple[str, str, bool]]:
    """Returns (bracket, target, is_conditional) for each include in the file."""
    includes = []
    depth = 0
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        if IF_RE.match(line):
            if not GUARD_RE.match(line):
                depth += 1
            continue
        if ENDIF_RE.match(line):
            depth = max(depth - 1, 0)
            continue
        match = INCLUDE_RE.match(line)
        if match:
            includes.append((match.group(1), match.group(2), depth > 0))
    return includes


def walk(resolver: Resolver, entries: list[str], follow_impl: bool, follow_prefixes: tuple[str, ...]):
    files: dict[str, int] = {}  # webcore path -> hops from entries
    included_by: dict[str, set[str]] = collections.defaultdict(set)
    conditional: set[str] = set()  # includes only ever reached through #if blocks
    unconditional: set[str] = set()
    boundaries: dict[str, dict[str, set[str]]] = collections.defaultdict(lambda: collections.defaultdict(set))
    problems: dict[str, dict[str, set[str]]] = collections.defaultdict(lambda: collections.defaultdict(set))

    queue = collections.deque((entry, 0) for entry in entries)
    while queue:
        rel, hops = queue.popleft()
        if rel in files:
            continue
        files[rel] = hops

        # A header's .cpp pulls in its implementation dependencies (always for entries, optionally for the rest).
        to_scan = [rel]
        if rel.endswith(".h") and (follow_impl or hops == 0):
            cpp = rel.removesuffix(".h") + ".cpp"
            if (resolver.webcore / cpp).exists():
                to_scan.append(cpp)

        for source in to_scan:
            for bracket, target, is_conditional in scan_includes(resolver.webcore / source):
                kind, key = resolver.resolve(bracket, target)
                if kind == "system" or key == "config.h":
                    continue
                if kind in ("unresolved", "ambiguous"):
                    problems[kind][key].add(rel)
                    continue
                if kind != "webcore" or not key.startswith(follow_prefixes):
                    group = "wtf" if kind == "wtf" else key.split("/", 1)[0] if kind == "webcore" else "external"
                    boundaries[group][key].add(rel)
                    continue
                if key == rel:
                    continue
                included_by[key].add(rel)
                (conditional if is_conditional else unconditional).add(key)
                queue.append((key, hops + 1))

    return files, included_by, conditional - unconditional, boundaries, problems


def write_report(out, entries, files, included_by, conditional_only, boundaries, problems):
    out.write(f"# Port graph\n\nEntries: {', '.join(f'`{e}`' for e in entries)}\n\n")
    out.write(f"- Files to port: **{len(files)}**\n")
    for group in sorted(boundaries):
        out.write(f"- Boundary `{group}`: {len(boundaries[group])}\n")
    for kind in sorted(problems):
        out.write(f"- {kind.capitalize()}: {len(problems[kind])}\n")

    out.write("\n## Files to port\n\nDeepest first (rough leaves-first order).\n\n")
    out.write("| Hops | File | Cond | Included by |\n|---|---|---|---|\n")
    for rel, hops in sorted(files.items(), key=lambda item: (-item[1], item[0])):
        parents = ", ".join(sorted(included_by[rel])) if hops else "(entry)"
        cond = "yes" if rel in conditional_only else ""
        out.write(f"| {hops} | `{rel}` | {cond} | {parents} |\n")

    for title, groups in (("Boundaries", boundaries), ("Problems", problems)):
        out.write(f"\n## {title}\n")
        for group in sorted(groups):
            out.write(f"\n### {group} ({len(groups[group])})\n\n")
            for key in sorted(groups[group]):
                out.write(f"- `{key}` ← {len(groups[group][key])} file(s)\n")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("entries", nargs="+", help="Entry files, relative to Source/WebCore (e.g. css/parser/CSSTokenizer.cpp).")
    parser.add_argument("--webkit-root", type=pathlib.Path, default=DEFAULT_WEBKIT_ROOT)
    parser.add_argument("--headers-only", action="store_true",
                        help="Don't follow the .cpp of non-entry headers (interface dependencies only).")
    parser.add_argument("--follow", nargs="+", default=list(FOLLOW_PREFIXES), metavar="PREFIX",
                        help=f"Path prefixes to traverse instead of {', '.join(FOLLOW_PREFIXES)} "
                             "(e.g. platform/graphics/).")
    parser.add_argument("--out", type=pathlib.Path, help="Write the markdown report here instead of stdout.")
    args = parser.parse_args()

    resolver = Resolver(args.webkit_root)
    for entry in args.entries:
        if not (resolver.webcore / entry).exists():
            parser.error(f"entry not found: {entry}")

    result = walk(resolver, args.entries, follow_impl=not args.headers_only, follow_prefixes=tuple(args.follow))
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        with args.out.open("w", encoding="utf-8", newline="\n") as out:
            write_report(out, args.entries, *result)
    else:
        sys.stdout.reconfigure(encoding="utf-8")
        write_report(sys.stdout, args.entries, *result)
    return 0


if __name__ == "__main__":
    sys.exit(main())
