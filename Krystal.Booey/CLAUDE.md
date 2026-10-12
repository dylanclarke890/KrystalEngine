# Krystal.Booey: WebKit CSS port

Booey's `CSS/` and `Style/` code is being ported from WebKit, one file or slice at a time, with the user reviewing each step. `PORTING.md` (next to this file) is the ledger: translation rules, verdicts per WebKit file, bugs fixed, differences, and a "Where we are" section. Read its "Where we are" and "Translation rules" sections before starting. Don't port ahead of what was asked, and ask before editing unreviewed files outside the current slice.

## Per-file checklist

Do these in order for each WebKit file (or tightly coupled group, e.g. a `.h`/`.cpp` pair):

1. **Graph.** `python tools/port/graph.py <webkit path> --headers-only --out tools/port/out/<name>.md`. Also read the WebKit `.cpp`'s own includes: headers-only mode doesn't follow them. Note anything new for the ledger.
2. **Read both sides in full.** The WebKit `.h`/`.cpp` (`C:\Users\dylan\repos\WebKit\Source\WebCore\...`) and the Krystal counterpart, if one exists.
3. **Compare.** List declarations missing on either side, behaviour that differs, and bugs in the Krystal version (wrong semantics, undefined behaviour, members that are declared but never defined, wrong enum traits, names that clash with types).
4. **Ask** before anything that changes scope: pulling in unreviewed files, choosing between upstream behaviour and the existing Krystal behaviour, or a decision that will affect other slices.
5. **Fix and complete.** Fix bugs, add missing methods, keep upstream comments, follow the root `CLAUDE.md` style. A `.cpp` includes its own header first. New or rewritten files are UTF-8 with BOM.
6. **Build.** Add reviewed files to `KRYSTAL_BOOEY_CSS_HEADERS`/`KRYSTAL_BOOEY_CSS_SOURCES` in `CMakeLists.txt`. Build: `cmake --build --preset windows-x64-debug --target Krystal.Booey.Tests` using Visual Studio's bundled CMake (never the system CMake or the bare `vcpkg` preset). No warnings: warnings are errors.
7. **Test.** Add Catch2 tests under `Krystal.Booey.Tests/tests/CSS/...`, mirroring the source path, for anything that can be instantiated. Templates must be instantiated by a test, or their bugs stay hidden. Known bugs outside the slice get a test tagged `[!shouldfail]` with a FIXME. Run `Debug/Krystal.Booey.Tests.exe` from `build/Krystal.Booey.Tests`; the whole suite must pass.
8. **Ledger.** Update `PORTING.md` using the template below, and the "Where we are" section.
9. **Report** what changed, what's flagged, and what's next. The user commits (or asks you to).

## Ledger entry template

```markdown
### `<Krystal name>` (<date>)

Graph from `<webkit entry>` (headers-only): N files (`tools/port/out/<name>.md`). <Scope notes.>

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|

**Bugs fixed:** one bullet per bug: what was wrong and its effect.
**Added (upstream API):** methods/types added.
**Differences:** intentional or unresolved differences from upstream.
Tests: `<test files>`: N cases / N assertions pass.
```

## Gotchas

- Only files listed in the CMake lists are built; the rest of `CSS/`/`Style/` is unreviewed and may not compile.
- A trial compile (temporarily adding a file to CMake) must be followed by re-running `cmake --preset windows-x64` after reverting, or the generated project keeps the trial file.
- `Value.cpp`/`PrimitiveValue.cpp` can't build until every `Value` subclass compiles; `Value` instances can't be created in tests yet.
- Comments must not mention WebKit or the port unless they are TODO/FIXME; upstream differences belong in `PORTING.md`.
