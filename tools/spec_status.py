"""Reports how far Booey's review against each specification has got, from the checklists in docs/specs/.

An algorithm counts as implemented when Booey's sources link to it (`/// @see https://dom.spec.whatwg.org/#...`)
and as tested when a test is tagged with it (`[dom:concept-node-pre-insert]`). The tag is the checklist's
specification name and the algorithm's anchor; Catch gives `#` tags a meaning of their own, so tags don't use one.

Usage, from the repository root:
  python tools/spec_status.py                         # a line per specification
  python tools/spec_status.py dom                     # a line per section of the DOM checklist
  python tools/spec_status.py dom mutation-algorithms # each algorithm of a section, with its code and tests
  python tools/spec_status.py dom --stale             # links and tags whose anchor isn't in the specification
"""

import argparse
import collections
import dataclasses
import pathlib
import re
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import spec_checklist  # noqa: E402

REPO_ROOT = spec_checklist.REPO_ROOT
SOURCE_DIRS = ("Krystal.Booey/include", "Krystal.Booey/src", "Krystal.UI/include", "Krystal.UI/src")
TEST_DIRS = ("Krystal.Booey.Tests", "Krystal.UI.Tests")
CODE_SUFFIXES = {".hpp", ".cpp", ".h", ".inl"}
URL = re.compile(r"https?://[^\s\"'<>)]+#([A-Za-z0-9_.:%-]+)")
TAG = re.compile(r"\[([a-z][a-z0-9-]*):([^\]\s#]+)\]")


@dataclasses.dataclass
class ChecklistAlgorithm:
    anchor: str
    name: str
    checked: bool


@dataclasses.dataclass
class ChecklistSection:
    anchor: str
    title: str
    status: str
    algorithms: list[ChecklistAlgorithm]


def ReadChecklist(spec: spec_checklist.Spec) -> list[ChecklistSection]:
    if not spec.output.exists():
        raise SystemExit(f"No checklist for {spec.name}: run python tools/spec_checklist.py {spec.name}")
    sections = []
    for line in spec.output.read_text(encoding="utf-8-sig").splitlines():
        if match := spec_checklist.SECTION_HEADING.match(line):
            title = re.match(r"^#+ \[(.*)\]\(", line).group(1)
            sections.append(ChecklistSection(match.group(1), title, "", []))
        elif line.startswith("Status:") and sections:
            sections[-1].status = line.removeprefix("Status:").strip()
        elif (match := spec_checklist.ALGORITHM_LINE.match(line)) and sections:
            name = re.match(r"^- \[.\] \[(.*?)\]\(", line).group(1)
            sections[-1].algorithms.append(ChecklistAlgorithm(match.group(2), name, match.group(1) != " "))
    return sections


def CodeFiles(dirs):
    for directory in dirs:
        root = REPO_ROOT / directory
        if root.is_dir():
            yield from (path for path in root.rglob("*") if path.suffix in CODE_SUFFIXES)


def Location(path: pathlib.Path, line: int) -> str:
    return f"{path.relative_to(REPO_ROOT).as_posix()}:{line}"


def FindLinks(spec: spec_checklist.Spec) -> dict[str, list[str]]:
    """The places Booey's sources link to each anchor of @p spec."""
    links = collections.defaultdict(list)
    for path in CodeFiles(SOURCE_DIRS):
        for number, line in enumerate(path.read_text(encoding="utf-8-sig", errors="replace").splitlines(), 1):
            for match in URL.finditer(line):
                if match.group(0).startswith(spec.link_prefixes):
                    links[match.group(1)].append(Location(path, number))
    return links


def FindTags(spec: spec_checklist.Spec) -> dict[str, list[str]]:
    """The places tests are tagged with each anchor of @p spec."""
    tags = collections.defaultdict(list)
    for path in CodeFiles(TEST_DIRS):
        for number, line in enumerate(path.read_text(encoding="utf-8-sig", errors="replace").splitlines(), 1):
            for match in TAG.finditer(line):
                if match.group(1) == spec.tag:
                    tags[match.group(2)].append(Location(path, number))
    return tags


def Counts(algorithms, links, tags) -> str:
    total = len(algorithms)
    implemented = sum(1 for algorithm in algorithms if algorithm.anchor in links)
    tested = sum(1 for algorithm in algorithms if algorithm.anchor in tags)
    reviewed = sum(1 for algorithm in algorithms if algorithm.checked)
    return f"{total} algorithms, {implemented} linked, {tested} tested, {reviewed} reviewed"


def ReportSpecs(specs) -> None:
    for spec in specs:
        sections = ReadChecklist(spec)
        algorithms = [algorithm for section in sections for algorithm in section.algorithms]
        statuses = collections.Counter(section.status.split(" (")[0] for section in sections)
        summary = ", ".join(f"{count} {status}" for status, count in sorted(statuses.items()))
        print(f"{spec.title}: {Counts(algorithms, FindLinks(spec), FindTags(spec))}; sections: {summary}")


def ReportSections(spec) -> None:
    links, tags = FindLinks(spec), FindTags(spec)
    for section in ReadChecklist(spec):
        counts = Counts(section.algorithms, links, tags) if section.algorithms else "no algorithms"
        print(f"{section.title} [{section.anchor}] — {section.status}: {counts}")


def ReportSection(spec, anchor: str) -> None:
    sections = [section for section in ReadChecklist(spec) if section.anchor == anchor]
    if not sections:
        raise SystemExit(f"No section {anchor!r} in {spec.output.relative_to(REPO_ROOT)}")
    section = sections[0]
    links, tags = FindLinks(spec), FindTags(spec)
    print(f"{section.title} [{section.anchor}] — {section.status}")
    if section.anchor in links or section.anchor in tags or not section.algorithms:
        print(f"    code:  {', '.join(links.get(section.anchor, [])) or 'none'}")
        print(f"    tests: {', '.join(tags.get(section.anchor, [])) or 'none'}")
    for algorithm in section.algorithms:
        box = "x" if algorithm.checked else " "
        print(f"[{box}] {algorithm.name} [{spec.tag}:{algorithm.anchor}]")
        print(f"    code:  {', '.join(links.get(algorithm.anchor, [])) or 'none'}")
        print(f"    tests: {', '.join(tags.get(algorithm.anchor, [])) or 'none'}")


def ReportStale(spec, source: str | None) -> None:
    markup = pathlib.Path(source).read_text(encoding="utf-8") if source else spec_checklist.Fetch(spec.url)
    ids = set(re.findall(r"\bid=\"?([^\s\">]+)", markup))
    for kind, found in (("link", FindLinks(spec)), ("tag", FindTags(spec))):
        for anchor, places in sorted(found.items()):
            if anchor not in ids:
                print(f"stale {kind} #{anchor}: {', '.join(places)}")


def Main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("spec", nargs="?", help="dom, infra, html, or a CSS module's short name")
    parser.add_argument("section", nargs="?", help="a section's anchor, to list its algorithms")
    parser.add_argument("--stale", action="store_true", help="list links and tags whose anchor isn't in the spec")
    parser.add_argument("--html", help="a saved copy of the specification, for --stale")
    args = parser.parse_args()
    if not args.spec:
        ReportSpecs(spec_checklist.AllSpecs())
        return
    spec = spec_checklist.GetSpec(args.spec)
    if args.stale:
        ReportStale(spec, args.html)
    elif args.section:
        ReportSection(spec, args.section)
    else:
        ReportSections(spec)


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    Main()
