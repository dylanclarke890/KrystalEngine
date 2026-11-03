"""Reports how far Booey's review against each specification has got, from the checklists in Krystal.Booey/docs/specs/.

An algorithm counts as implemented when Booey's sources link to it (`/// @see https://dom.spec.whatwg.org/#...`)
and as tested when a test is tagged with it (`[dom:concept-node-pre-insert]`). The tag is the checklist's
specification name and the algorithm's anchor; Catch gives `#` tags a meaning of their own, so tags don't use one.

HTML has a checklist per page of its multipage edition, so it reports a line per page, and a page's sections.

Usage, from the repository root:
  python Krystal.Booey/tools/spec_status.py                         # a line per specification
  python Krystal.Booey/tools/spec_status.py dom                     # a line per section of the DOM checklist
  python Krystal.Booey/tools/spec_status.py dom mutation-algorithms # each algorithm of a section, with its code and tests
  python Krystal.Booey/tools/spec_status.py dom --stale             # links and tags whose anchor isn't in the specification
  python Krystal.Booey/tools/spec_status.py html                    # a line per page of the HTML checklists
  python Krystal.Booey/tools/spec_status.py html forms              # a line per section of a page
  python Krystal.Booey/tools/spec_status.py html forms forms        # a section whose anchor is also a page's name
  python Krystal.Booey/tools/spec_status.py dom --tick interface-range --note "Code: ..."   # tick a section, set it reviewed
  python Krystal.Booey/tools/spec_status.py dom --tick sets --status "n/a (heading only)"  # or set another status
"""

import argparse
import collections
import dataclasses
import datetime
import pathlib
import re
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import spec_checklist  # noqa: E402

REPO_ROOT = spec_checklist.REPO_ROOT
SOURCE_DIRS = ("Krystal.Booey/include", "Krystal.Booey/src", "Krystal.UI/include", "Krystal.UI/src")
TEST_DIRS = ("Krystal.Booey.Tests", "Krystal.UI.Tests")
CODE_SUFFIXES = {".hpp", ".cpp", ".h", ".inl"}
# Anchors can hold parentheses (`hidden-state-(type=hidden)`); Unbalanced() drops a closing one that isn't theirs.
URL = re.compile(r"https?://[^\s\"'<>()]+#([A-Za-z0-9_.:%=,()-]+)")
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
    definitions: list[str] = dataclasses.field(default_factory=list)
    # The checklist it's in: the specification's, or for a paged one, its page's.
    path: pathlib.Path | None = None


def ReadChecklist(spec: spec_checklist.Spec) -> list[ChecklistSection]:
    paths = spec.Checklists()
    if not paths or not all(path.exists() for path in paths):
        raise SystemExit(f"No checklist for {spec.name}: run python Krystal.Booey/tools/spec_checklist.py {spec.name}")
    sections = []
    for path in paths:
        for line in path.read_text(encoding="utf-8-sig").splitlines():
            if match := spec_checklist.SECTION_HEADING.match(line):
                title = re.match(r"^#+ \[(.*)\]\(", line).group(1)
                anchor = spec_checklist.DecodeAnchor(match.group(1))
                sections.append(ChecklistSection(anchor, title, "", [], path=path))
            elif line.startswith("Status:") and sections:
                sections[-1].status = line.removeprefix("Status:").strip()
            elif line.startswith(spec_checklist.DEFINITIONS_PREFIX) and sections:
                sections[-1].definitions = line.removeprefix(spec_checklist.DEFINITIONS_PREFIX).split()[:-1]
            elif (match := spec_checklist.ALGORITHM_LINE.match(line)) and sections:
                name = re.match(r"^- \[.\] \[(.*?)\]\(", line).group(1)
                anchor = spec_checklist.DecodeAnchor(match.group(2))
                sections[-1].algorithms.append(ChecklistAlgorithm(anchor, name, match.group(1) != " "))
    return sections


def CodeFiles(dirs):
    for directory in dirs:
        root = REPO_ROOT / directory
        if root.is_dir():
            yield from (path for path in root.rglob("*") if path.suffix in CODE_SUFFIXES)


def Location(path: pathlib.Path, line: int) -> str:
    return f"{path.relative_to(REPO_ROOT).as_posix()}:{line}"


def Unbalanced(anchor: str) -> str:
    """@p anchor without the closing parentheses and punctuation that end the comment around it, as in
    `(see https://html.spec.whatwg.org/#foo)`."""
    anchor = anchor.rstrip(".,")
    while anchor.endswith(")") and anchor.count(")") > anchor.count("("):
        anchor = anchor[:-1].rstrip(".,")
    return anchor


def FindLinks(spec: spec_checklist.Spec) -> dict[str, list[str]]:
    """The places Booey's sources link to each anchor of @p spec."""
    links = collections.defaultdict(list)
    for path in CodeFiles(SOURCE_DIRS):
        for number, line in enumerate(path.read_text(encoding="utf-8-sig", errors="replace").splitlines(), 1):
            for match in URL.finditer(line):
                if match.group(0).startswith(spec.link_prefixes):
                    anchor = spec_checklist.DecodeAnchor(Unbalanced(match.group(1)))
                    links[anchor].append(Location(path, number))
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
        print(f"{spec.title}: {Counts(algorithms, FindLinks(spec), FindTags(spec))}; "
              f"sections: {StatusSummary(sections)}")


def StatusSummary(sections) -> str:
    statuses = collections.Counter(section.status.split(" (")[0] for section in sections)
    return ", ".join(f"{count} {status}" for status, count in sorted(statuses.items()))


def ReportPages(spec) -> None:
    links, tags = FindLinks(spec), FindTags(spec)
    pages = collections.defaultdict(list)
    for section in ReadChecklist(spec):
        pages[section.path].append(section)
    for path, sections in pages.items():
        algorithms = [algorithm for section in sections for algorithm in section.algorithms]
        print(f"{path.stem} ({sections[0].title}): {Counts(algorithms, links, tags)}; "
              f"sections: {StatusSummary(sections)}")


def ReportSections(spec, page: str | None = None) -> None:
    links, tags = FindLinks(spec), FindTags(spec)
    for section in ReadChecklist(spec):
        if page and section.path.stem != page:
            continue
        counts = Counts(section.algorithms, links, tags) if section.algorithms else "no algorithms"
        if section.definitions:
            linked = sum(1 for anchor in section.definitions if anchor in links)
            tested = sum(1 for anchor in section.definitions if anchor in tags)
            counts += f"; {len(section.definitions)} definitions, {linked} linked, {tested} tested"
        print(f"{section.title} [{section.anchor}] — {section.status}: {counts}")


def ReportSection(spec, anchor: str) -> None:
    sections = [section for section in ReadChecklist(spec) if section.anchor == anchor]
    if not sections:
        raise SystemExit(f"No section {anchor!r} in the {spec.title} checklist")
    section = sections[0]
    links, tags = FindLinks(spec), FindTags(spec)
    print(f"{section.title} [{section.anchor}] — {section.status}")
    for anchor in [section.anchor] + section.definitions:
        if anchor == section.anchor and anchor not in links and anchor not in tags:
            continue
        print(f"  {'section' if anchor == section.anchor else 'definition'} [{spec.tag}:{anchor}]")
        print(f"    code:  {', '.join(links.get(anchor, [])) or 'none'}")
        print(f"    tests: {', '.join(tags.get(anchor, [])) or 'none'}")
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


def Tick(spec, anchor: str, status: str | None, notes: list[str]) -> None:
    """Ticks the algorithms of the section @p anchor, sets its status (reviewed today unless @p status is given) and
    adds @p notes under it."""
    sections = [section for section in ReadChecklist(spec) if section.anchor == anchor]
    if not sections:
        raise SystemExit(f"No section {anchor!r} in the {spec.title} checklist")
    path = sections[0].path
    raw = path.read_bytes()
    newline = "\r\n" if b"\r\n" in raw else "\n"
    lines = raw.decode("utf-8-sig").replace("\r\n", "\n").split("\n")
    start = next(i for i, line in enumerate(lines) if (match := spec_checklist.SECTION_HEADING.match(line))
                 and spec_checklist.DecodeAnchor(match.group(1)) == anchor)
    end = next((i for i in range(start + 1, len(lines)) if spec_checklist.SECTION_HEADING.match(lines[i])),
               len(lines))
    ticked = 0
    for i in range(start, end):
        if lines[i].startswith("- [ ] "):
            lines[i] = "- [x] " + lines[i][len("- [ ] "):]
            ticked += 1
    status_index = next(i for i in range(start, end) if lines[i].startswith("Status:"))
    new_status = f"Status: {status or f'reviewed ({datetime.date.today().isoformat()})'}"
    if lines[status_index] != "Status: unreviewed":
        print(f"replacing {lines[status_index]!r}")
    lines[status_index] = new_status
    insert = status_index + 1
    while insert < end and lines[insert].startswith(">"):
        insert += 1
    lines[insert:insert] = [f"> {note}" for note in notes]
    text = "\n".join(lines).replace("\n", newline)
    path.write_bytes((b"\xef\xbb\xbf" if raw.startswith(b"\xef\xbb\xbf") else b"") + text.encode("utf-8"))
    print(f"{path.relative_to(REPO_ROOT).as_posix()}: {anchor} {new_status.lower()}, {ticked} ticked")


def Main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("spec", nargs="?", help="dom, infra, html, or a CSS module's short name")
    parser.add_argument("section", nargs="?", help="a section's anchor, to list its algorithms (or a page's name)")
    parser.add_argument("page_section", nargs="?", help="with a page's name, a section's anchor on it")
    parser.add_argument("--stale", action="store_true", help="list links and tags whose anchor isn't in the spec")
    parser.add_argument("--html", help="a saved copy of the specification, for --stale")
    parser.add_argument("--tick", metavar="ANCHOR", help="tick a section's algorithms and set its status")
    parser.add_argument("--status", help="with --tick, the status to set (default: reviewed today)")
    parser.add_argument("--note", action="append", default=[], help="with --tick, a `>` note to add (repeatable)")
    args = parser.parse_args()
    if not args.spec:
        ReportSpecs(spec_checklist.AllSpecs())
        return
    spec = spec_checklist.GetSpec(args.spec)
    pages = {path.stem for path in spec.Checklists()} if spec.paged else set()
    if args.tick:
        Tick(spec, args.tick, args.status, args.note)
    elif args.stale:
        ReportStale(spec, args.html)
    elif args.page_section:
        ReportSection(spec, args.page_section)
    elif args.section in pages:
        ReportSections(spec, args.section)
    elif args.section:
        ReportSection(spec, args.section)
    elif spec.paged:
        ReportPages(spec)
    else:
        ReportSections(spec)


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    Main()
