"""Generates a review checklist of a specification's algorithms, for checking Booey against it.

Each section of the specification becomes a heading with a status line, and each algorithm in it a checkbox with
its steps beneath, labelled as the code's step comments are (`1.`, `1a.`, `1a.i.`, `1a.i.A.`). Regenerating
keeps each section's status line, each algorithm's checkbox and the `>` note lines under it, matched by anchor.

Usage, from the repository root:
  python tools/spec_checklist.py dom              # docs/specs/DOM.md from https://dom.spec.whatwg.org/
  python tools/spec_checklist.py infra html
  python tools/spec_checklist.py css-cascade-5    # docs/specs/css/css-cascade-5.md from the editor's draft
  python tools/spec_checklist.py dom --html saved.html   # from a saved copy instead of fetching
"""

import argparse
import dataclasses
import datetime
import html
import html.parser
import pathlib
import re
import sys
import urllib.request

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent


@dataclasses.dataclass(frozen=True)
class Spec:
    """A specification: where it's published, where its checklist goes, and the URL prefixes that link to it."""

    name: str
    title: str
    url: str
    output: pathlib.Path
    link_prefixes: tuple[str, ...]

    @property
    def tag(self) -> str:
        """The prefix of the Catch tags that name its algorithms, as in `[dom:concept-node-pre-insert]`."""
        return self.name


WHATWG_SPECS = {
    "dom": Spec("dom", "DOM", "https://dom.spec.whatwg.org/", REPO_ROOT / "docs/specs/DOM.md",
                ("https://dom.spec.whatwg.org/",)),
    "infra": Spec("infra", "Infra", "https://infra.spec.whatwg.org/", REPO_ROOT / "docs/specs/INFRA.md",
                  ("https://infra.spec.whatwg.org/",)),
    "html": Spec("html", "HTML", "https://html.spec.whatwg.org/", REPO_ROOT / "docs/specs/HTML.md",
                 ("https://html.spec.whatwg.org/",)),
}


def GetSpec(name: str) -> Spec:
    """The specification called @p name: a WHATWG one, or a CSS module by its short name (`css-cascade-5`)."""
    if name in WHATWG_SPECS:
        return WHATWG_SPECS[name]
    if re.fullmatch(r"(css|selectors|mediaqueries|cssom|compositing|filter-effects|motion|fill-stroke|"
                    r"scroll-animations|web-animations|resize-observer|pointerevents|uievents)[a-z0-9-]*", name):
        unversioned = re.sub(r"-\d+$", "", name)
        return Spec(name, name, f"https://drafts.csswg.org/{name}/", REPO_ROOT / f"docs/specs/css/{name}.md",
                    (f"https://drafts.csswg.org/{name}/", f"https://drafts.csswg.org/{unversioned}/",
                     f"https://www.w3.org/TR/{name}/", f"https://www.w3.org/TR/{unversioned}/"))
    raise SystemExit(f"Unknown specification {name!r}: use dom, infra, html or a CSS module's short name.")


def AllSpecs() -> list[Spec]:
    """Every specification that has a checklist."""
    specs = [spec for spec in WHATWG_SPECS.values() if spec.output.exists()]
    css = REPO_ROOT / "docs/specs/css"
    if css.is_dir():
        specs += [GetSpec(path.stem) for path in sorted(css.glob("*.md"))]
    return specs


# A minimal tree builder over html.parser, with the implied end tags the specifications' generators rely on.

VOID_ELEMENTS = {"area", "base", "br", "col", "embed", "hr", "img", "input", "link", "meta", "source", "track", "wbr"}
CLOSES_P = {"address", "article", "aside", "blockquote", "details", "div", "dl", "fieldset", "figcaption", "figure",
            "footer", "form", "h1", "h2", "h3", "h4", "h5", "h6", "header", "hr", "main", "menu", "nav", "ol", "p",
            "pre", "section", "table", "ul", "dd", "dt", "li"}


class Node:
    __slots__ = ("tag", "attrs", "children", "parent")

    def __init__(self, tag: str, attrs: dict[str, str], parent: "Node | None"):
        self.tag = tag
        self.attrs = attrs
        self.children: list["Node | str"] = []
        self.parent = parent

    def Elements(self):
        return (child for child in self.children if isinstance(child, Node))

    def Classes(self) -> set[str]:
        return set(self.attrs.get("class", "").split())


class TreeBuilder(html.parser.HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.root = Node("#root", {}, None)
        self.stack = [self.root]

    def _CloseUntil(self, tags: set[str], stop: set[str]):
        for i in range(len(self.stack) - 1, 0, -1):
            tag = self.stack[i].tag
            if tag in tags:
                del self.stack[i:]
                return
            if tag in stop:
                return

    def handle_starttag(self, tag, attrs):
        if tag == "body":
            self._CloseUntil({"head"}, set())
        if tag in CLOSES_P:
            self._CloseUntil({"p"}, {"div", "li", "dd", "dt", "td", "th", "blockquote", "section"})
        if tag == "li":
            self._CloseUntil({"li"}, {"ol", "ul", "menu"})
        elif tag in ("dt", "dd"):
            self._CloseUntil({"dt", "dd"}, {"dl"})
        elif tag in ("td", "th"):
            self._CloseUntil({"td", "th"}, {"tr", "table"})
        elif tag == "tr":
            self._CloseUntil({"tr"}, {"table", "tbody", "thead", "tfoot"})
        node = Node(tag, {name: value or "" for name, value in attrs}, self.stack[-1])
        self.stack[-1].children.append(node)
        if tag not in VOID_ELEMENTS:
            self.stack.append(node)

    def handle_startendtag(self, tag, attrs):
        self.handle_starttag(tag, attrs)
        if tag not in VOID_ELEMENTS and self.stack[-1].tag == tag:
            self.stack.pop()

    def handle_endtag(self, tag):
        for i in range(len(self.stack) - 1, 0, -1):
            if self.stack[i].tag == tag:
                del self.stack[i:]
                return

    def handle_data(self, data):
        self.stack[-1].children.append(data)


# Extraction.

SKIPPED_CLASSES = {"note", "example", "domintro", "XXX", "issue", "annotation", "mdn-anno", "wpt-tests-block",
                   "status", "def", "warning", "compat"}
SKIPPED_TAGS = {"script", "style", "pre", "aside", "details", "head", "nav", "figure", "table"}
STEPS_SENTENCE = re.compile(r"\bsteps?\s+(?:are|is|must be)\s+to\b|\bsteps\s+are\s+as\s+follows\b", re.I)
MAX_INTRO = 300
MAX_PROSE = 800
MAX_STEP = 400


def IsSkipped(node: Node) -> bool:
    return node.tag in SKIPPED_TAGS or bool(node.Classes() & SKIPPED_CLASSES)


def Text(node: "Node | str", exclude=("ol",)) -> str:
    """The node's text, without notes and examples, and without the lists in @p exclude."""
    if isinstance(node, str):
        return node
    if IsSkipped(node) or node.tag in exclude:
        return ""
    if node.tag == "ul":
        items = [Collapse(Text(child, exclude)) for child in node.Elements() if child.tag == "li"]
        return " " + "; ".join(item for item in items if item) + " "
    if node.tag == "br":
        return " "
    return "".join(Text(child, exclude) for child in node.children)


def Collapse(text: str) -> str:
    return re.sub(r"\s+", " ", text).strip()


def Shorten(text: str, limit: int) -> str:
    return text if len(text) <= limit else text[: limit - 1].rstrip() + "…"


def Roman(number: int) -> str:
    numerals = [(10, "x"), (9, "ix"), (5, "v"), (4, "iv"), (1, "i")]
    result = ""
    for value, numeral in numerals:
        while number >= value:
            result += numeral
            number -= value
    return result


def Letters(number: int, upper: bool) -> str:
    result = ""
    while number > 0:
        number, remainder = divmod(number - 1, 26)
        result = chr((65 if upper else 97) + remainder) + result
    return result


def StepLabel(prefix: str, depth: int, index: int) -> str:
    """The label of step @p index at @p depth: 1., 1a., 1a.i., 1a.i.A., then numbers again."""
    match depth % 4:
        case 0:
            part = str(index) if depth == 0 else f".{index}"
        case 1:
            part = Letters(index, upper=False)
        case 2:
            part = f".{Roman(index)}"
        case _:
            part = f".{Letters(index, upper=True)}"
    return prefix + part


@dataclasses.dataclass
class Step:
    label: str
    text: str
    children: list["Step"]


@dataclasses.dataclass
class Algorithm:
    anchor: str
    name: str
    intro: str
    steps: list[Step]


@dataclasses.dataclass
class Section:
    anchor: str
    title: str
    depth: int
    algorithms: list[Algorithm]


def Steps(ol: Node, prefix: str = "", depth: int = 0) -> list[Step]:
    steps = []
    start = int(ol.attrs.get("start", "1") or 1)
    for index, li in enumerate((child for child in ol.Elements() if child.tag == "li"), start):
        label = StepLabel(prefix, depth, index)
        nested = [child for child in Descendants(li, stop={"ol"}) if child.tag == "ol"]
        children = []
        for sub in nested:
            children += Steps(sub, label, depth + 1)
        steps.append(Step(label + ".", Shorten(Collapse(Text(li)), MAX_STEP), children))
    return steps


def Descendants(node: Node, stop: set[str]):
    """The element descendants of @p node, not looking inside elements in @p stop or skipped ones."""
    for child in node.Elements():
        if IsSkipped(child):
            continue
        yield child
        if child.tag not in stop:
            yield from Descendants(child, stop)


def FirstDefinition(paragraph: Node) -> Node | None:
    for child in Descendants(paragraph, stop={"ol", "ul"}):
        if child.tag == "dfn" and child.attrs.get("id"):
            return child
    return None


def NextElement(node: Node) -> Node | None:
    siblings = node.parent.children
    for child in siblings[siblings.index(node) + 1:]:
        if isinstance(child, Node):
            if IsSkipped(child):
                continue
            return child
        if child.strip():
            return None
    return None


def AlgorithmName(dfn: Node) -> str:
    name = Collapse(Text(dfn, exclude=()))
    owner = dfn.attrs.get("data-dfn-for", "")
    if owner and " " not in owner and dfn.attrs.get("data-dfn-type") in ("method", "attribute", "constructor"):
        return f"{owner}.{name}"
    if owner and "/" not in owner:
        return f"{name} ({owner})"
    return name


def HeadingText(heading: Node) -> str:
    return Collapse(Text(heading, exclude=()).replace("¶", ""))


def Extract(root: Node) -> list[Section]:
    sections = [Section("", "Front matter", 2, [])]
    inside_steps = 0
    algorithm_div = None
    last_definition = ""

    def Visit(node: Node):
        nonlocal inside_steps, algorithm_div, last_definition
        if IsSkipped(node):
            return
        if re.fullmatch(r"h[2-6]", node.tag) and node.attrs.get("id"):
            sections.append(Section(node.attrs["id"], HeadingText(node), int(node.tag[1]), []))
            last_definition = node.attrs["id"]
            return
        if node.tag in ("p", "dd") and not inside_steps:
            in_algorithm_div = algorithm_div is not None
            algorithm = AlgorithmAt(node, in_algorithm_div) or StepsWithoutDefinition(node, in_algorithm_div,
                                                                                       last_definition)
            if algorithm:
                sections[-1].algorithms.append(algorithm)
                algorithm_div = None
        if node.tag == "dfn" and node.attrs.get("id"):
            last_definition = node.attrs["id"]
        is_div = node.tag == "div" and ("algorithm" in node.Classes() or "data-algorithm" in node.attrs)
        outer_div = algorithm_div
        if is_div:
            algorithm_div = node
        is_steps = node.tag == "ol"
        inside_steps += is_steps
        for child in list(node.Elements()):
            Visit(child)
        inside_steps -= is_steps
        if is_div:
            algorithm_div = outer_div

    for child in root.Elements():
        Visit(child)
    return [section for section in sections if section.anchor or section.algorithms]


def StartsWithDefinition(intro: str, dfn: Node) -> bool:
    """Whether the paragraph opens by defining an algorithm, as in "To <dfn>append</dfn> a node ..."."""
    name = Collapse(Text(dfn, exclude=()))
    opening = intro.split(",", 1)[0]
    if intro.startswith("To "):
        return name in opening[:len(name) + 40]
    # Otherwise it opens by naming what an algorithm computes, as in "The <dfn>union</dfn> of ... is the result of ...".
    return (intro.startswith("The ") and name in intro[:len(name) + 10]
            and bool(re.search(r"is the result of|creates a new|\breturns\b", intro)))


def IsInterfaceMember(dfn: Node, intro: str) -> bool:
    """Whether the paragraph defines an interface member's behaviour, as in "The <dfn>length</dfn> attribute must return ..."."""
    kind = dfn.attrs.get("data-dfn-type", "")
    return kind in ("method", "attribute", "constructor") and bool(re.search(r"\bmust\b", intro))


def Cases(dl: Node) -> list[Step]:
    """A definition list of the cases an algorithm switches on, as unnumbered steps."""
    cases = []
    terms = []
    for child in dl.Elements():
        if child.tag == "dt":
            terms.append(Collapse(Text(child)))
        elif child.tag == "dd":
            text = Collapse(Text(child))
            nested = [sub for sub in Descendants(child, stop={"ol"}) if sub.tag == "ol"]
            children = [step for sub in nested for step in Steps(sub)]
            cases.append(Step("•", Shorten(f"{' / '.join(terms)}: {text}", MAX_STEP), children))
            terms = []
    return cases


def AlgorithmAt(paragraph: Node, in_algorithm_div: bool) -> Algorithm | None:
    dfn = FirstDefinition(paragraph)
    if not dfn:
        return None
    intro = Collapse(Text(paragraph))
    following = NextElement(paragraph)
    if following is not None and following.tag == "ol":
        steps = Steps(following)
    elif (in_algorithm_div or STEPS_SENTENCE.search(intro) or StartsWithDefinition(intro, dfn)
          or IsInterfaceMember(dfn, intro)):
        steps = Cases(following) if following is not None and following.tag == "dl" else []
    else:
        return None
    return Algorithm(dfn.attrs["id"], AlgorithmName(dfn), Shorten(intro, MAX_INTRO if steps else MAX_PROSE), steps)


def StepsWithoutDefinition(paragraph: Node, in_algorithm_div: bool, anchor: str) -> Algorithm | None:
    """An algorithm whose paragraph defines nothing, as in "The value setter steps are to ...", under @p anchor: the
    definition it follows (the attribute whose setter it is), or the section's heading."""
    intro = Collapse(Text(paragraph))
    following = NextElement(paragraph)
    has_steps = following is not None and following.tag == "ol" and intro.endswith(":")
    names_steps = bool(re.search(r"\b(steps|algorithm)\b", intro, re.I))
    if not (STEPS_SENTENCE.search(intro) or (has_steps and (in_algorithm_div or names_steps))):
        return None
    steps = Steps(following) if has_steps else []
    # A paragraph that opens with a link, as in "The <a>value</a> setter steps", is under what it links to.
    link = next((child for child in Descendants(paragraph, stop={"ol", "ul"})
                 if child.tag == "a" and child.attrs.get("href", "").startswith("#")), None)
    if link is not None and intro.startswith("The " + Collapse(Text(link, exclude=()))):
        anchor = link.attrs["href"][1:]
    name = re.split(r":|\s(?:are|is)\s+(?:to|as follows)\b", intro, maxsplit=1)[0]
    name = Shorten(re.sub(r"\s(?:are|is)$", "", name), 80)
    return Algorithm(anchor, name, Shorten(intro, MAX_INTRO if steps else MAX_PROSE), steps)


# Output, keeping what reviewers wrote.

SECTION_HEADING = re.compile(r"^#{2,6} \[.*\]\(.*#([^)]+)\)\s*$")
def Keys(anchors):
    """Each algorithm's key: its anchor, with `~2`, `~3` and so on for further algorithms under the same anchor."""
    counts = {}
    for anchor in anchors:
        counts[anchor] = counts.get(anchor, 0) + 1
        yield anchor if counts[anchor] == 1 else f"{anchor}~{counts[anchor]}"


ALGORITHM_LINE = re.compile(r"^- \[([ xX])\] \[.*?\]\(.*?#([^)\s]+)\)")
DEFAULT_STATUS = "Status: unreviewed"


@dataclasses.dataclass
class Kept:
    statuses: dict[str, str]
    checked: dict[str, bool]
    notes: dict[str, list[str]]
    section_notes: dict[str, list[str]]


def ReadKept(path: pathlib.Path) -> Kept:
    kept = Kept({}, {}, {}, {})
    if not path.exists():
        return kept
    section = None
    algorithm = None
    lines = path.read_text(encoding="utf-8-sig").splitlines()
    keys = Keys(match.group(2) for line in lines if (match := ALGORITHM_LINE.match(line)))
    for line in lines:
        if match := SECTION_HEADING.match(line):
            section, algorithm = match.group(1), None
        elif line.startswith("Status:") and section is not None:
            kept.statuses[section] = line.strip()
        elif match := ALGORITHM_LINE.match(line):
            algorithm = next(keys)
            kept.checked[algorithm] = match.group(1) != " "
        elif algorithm and line.startswith("  >"):
            kept.notes.setdefault(algorithm, []).append(line)
        elif section is not None and not algorithm and line.startswith(">"):
            kept.section_notes.setdefault(section, []).append(line)
        elif line.startswith("#"):
            algorithm = None
    return kept


def WriteSteps(lines: list[str], steps: list[Step], indent: int):
    for step in steps:
        lines.append(f"{'  ' * indent}- {step.label} {step.text}".rstrip())
        WriteSteps(lines, step.children, indent + 1)


def Render(spec: Spec, sections: list[Section], kept: Kept) -> str:
    total = sum(len(section.algorithms) for section in sections)
    lines = [
        f"# {spec.title} review checklist",
        "",
        f"Generated from <{spec.url}> on {datetime.date.today().isoformat()} by `tools/spec_checklist.py`: "
        f"{total} algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each "
        "algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py` reports where each algorithm "
        "is implemented and tested.",
        "",
        "Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below "
        f"(`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on "
        "purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a "
        "`[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the "
        f"tests that cover it (`[{spec.tag}:<anchor>]`), tick it, and set the section's status. A section without "
        "algorithms still has definitions and requirements to check against the code: note where they live in `>` "
        "lines under its status, and tag their tests with the definition's or the section's anchor.",
        "",
        "Statuses: `unreviewed`, `in review`, `reviewed (<date>, <commit>)`, `n/a (<reason>)`. An algorithm is "
        "ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note.",
    ]
    sections = [section for section in sections if section.anchor]
    keys = Keys(algorithm.anchor for section in sections for algorithm in section.algorithms)
    for section in sections:
        lines += ["", f"{'#' * section.depth} [{section.title}]({spec.url}#{section.anchor})", ""]
        lines.append(kept.statuses.get(section.anchor, DEFAULT_STATUS))
        lines += kept.section_notes.get(section.anchor, [])
        for algorithm in section.algorithms:
            key = next(keys)
            box = "x" if kept.checked.get(key) else " "
            lines += ["", f"- [{box}] [{algorithm.name}]({spec.url}#{algorithm.anchor}) — {algorithm.intro}"]
            lines += kept.notes.get(key, [])
            WriteSteps(lines, algorithm.steps, 1)
    return "\n".join(lines) + "\n"


def Fetch(url: str) -> str:
    request = urllib.request.Request(url, headers={"User-Agent": "KrystalEngine spec_checklist"})
    with urllib.request.urlopen(request) as response:
        return response.read().decode("utf-8")


def Generate(spec: Spec, source: str | None) -> None:
    markup = pathlib.Path(source).read_text(encoding="utf-8") if source else Fetch(spec.url)
    builder = TreeBuilder()
    builder.feed(markup)
    builder.close()
    sections = Extract(builder.root)
    kept = ReadKept(spec.output)
    spec.output.parent.mkdir(parents=True, exist_ok=True)
    spec.output.write_text(Render(spec, sections, kept), encoding="utf-8-sig", newline="\r\n")
    total = sum(len(section.algorithms) for section in sections)
    print(f"{spec.output.relative_to(REPO_ROOT)}: {len(sections)} sections, {total} algorithms")


def Main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("specs", nargs="+", help="dom, infra, html, or a CSS module's short name")
    parser.add_argument("--html", help="a saved copy of the specification to read instead of fetching it")
    args = parser.parse_args()
    if args.html and len(args.specs) != 1:
        raise SystemExit("--html takes one specification")
    for name in args.specs:
        Generate(GetSpec(name), args.html)


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    Main()
