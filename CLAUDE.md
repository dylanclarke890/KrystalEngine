# KrystalEngine

C++ game engine (latest standard, CMake + vcpkg). Formatting is owned by `.clang-format` — run it rather than hand-formatting.
Composed of multiple modules, each with its own `CMakeLists.txt` and `include/<Module>`/`src` structure.

## Naming

- Namespaces: lowercase — `krys`, `krys::io`, `krys::detail` for implementation details.
- Types, functions, methods, enum values: `PascalCase` (`EventManager`, `DispatchAll`, `SeekOrigin::Begin`).
- Private/protected data members: `_camelCase`. Public data members on plain structs: `PascalCase` (`Value`).
- Locals and parameters: `camelCase`.
- Template params: `T`/`U` for plain types, `TName` for named ones (`TEvent`, `TSize`, `TChar`). Prefer a concept over `typename` when one fits (`template <Number T>`).
- Macros: `KRYS_UPPER_CASE`. Assertions are lowercase: `krys_assert`, `krys_assert_msg`, `krys_debug_assert`.
- Exceptions to PascalCase: low-level std-like helpers (`krys::move`, `ipow`), numeric primitive aliases (`uint32`, `byte`).

## Types

- Use the engine types rather than std: `uint32`/`int64`/`byte`, `List`, `HashMap`, `Queue`, `Span`, `UniquePtr`, `Func`, `Maybe`, `Expected`.
- Prefer `Krystal.Core/TypeTraits.hpp` and `Krystal.Core/Concepts.hpp` over including `<type_traits>` or `<concepts>` directly. Add common type traits/concepts there.

## Attributes and specifiers

- Use the `KRYS_*` attribute macros (`Attributes.hpp`), never raw `[[...]]`.
- `KRYS_NODISCARD` on every function whose return value is the point: getters, queries, factories, pure maths.
- `noexcept` by default on functions, constructors and destructors. Leave it off only when a function can genuinely throw or allocate in a way that can fail (e.g. `Open()`). Use conditional `noexcept(...)` in templates.
- `constexpr` wherever it compiles.
- Single-argument constructors are `explicit` unless implicit conversion is intended.
- Order: `KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static inline <return> Name(...) const noexcept` (use only the ones that apply).
- Branch hints go after the condition: `if (ptr) KRYS_LIKELY`.
- Copy/move suppression uses `KRYS_NON_COPYABLE(X);`, `KRYS_NON_MOVABLE(X);` or `KRYS_NON_COPY_MOVABLE(X);`.

## Class layout

Not possible to follow in all cases but the general case class layout is:

```cpp
/// @brief One-line description.
class Example
{
  friend class Friend;         // friends first
  using Base = ...;            // then private aliases
  KRYS_NON_COPYABLE(Example);  // then copy/move suppression macros

private:
  uint32 _count {0};           // data members, brace-initialised
  KRYS_NO_UNIQUE_ADDRESS Debugger _debugger;

public:
  Example() noexcept = default; // constructors/destructors next
  ~Example() noexcept;

  void DoThing() noexcept; // public interface next
  KRYS_NODISCARD uint32 GetCount() const noexcept;

private:
  void Helper() noexcept;      // private/protected helpers last
};
```

Use `struct` for policy/trait/plain-data types with all-public members.

## Files and comments

- Always create files as UTF-8 with BOM (never plain UTF-8 or Windows-1252), and keep the BOM when rewriting a file.
- Headers: `#pragma once`, then project includes as `"<Module>/..."` (e.g. `"Krystal.Core/..."`), then std headers. Headers go under `include/<Module>/`, sources under `src/` in the same folder structure.
- Third-party sources (not managed by vcpkg) go under `third-party/` and are included with `<...>`.
- Doxygen `///` with `@brief`, plus `@param`/`@return`/`@note`/`@attention` where they add something. Use `//` for implementation notes.
- Brace every `if`/loop body, even if it's a single short statement.

## Building and testing

Use Visual Studio's bundled CMake (`C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe`) with the `windows-x64` presets; never the system CMake or the bare `vcpkg` preset. Keep tool output out of context: only look at whether a step succeeded, or at the specific errors if it failed.

- **Configure:** `cmake --preset windows-x64 --log-level=WARNING > build/configure.log 2>&1`. vcpkg still prints its installed-package list (no flag hides it), so don't read the log unless the exit code is non-zero.
- **Build:** `cmake --build --preset windows-x64-debug --target <target> -- -v:quiet -nologo`. MSBuild then prints nothing on success and only errors/warnings on failure (warnings are errors). If a template error is long, read only its first lines.
- **Tests:** Catch2's default reporter prints only failures and a summary; on a pass read only the last few lines.

## Documents

Project documents live in `docs/`, one folder per module; scripts live in `tools/`.

- `docs/booey/PORTING.md`: the WebKit port ledger.
- `docs/booey/TODO.md`: open work in Krystal.Booey outside the current port slice.
- `docs/booey/ROADMAP.md`: the order of Booey's remaining slices (B1 onwards), with their plans.
- `docs/gfx/DESIGN.md`: the graphics redesign (draft).
- `tools/graph.py`: walks WebKit's include graph for a port slice.

## Porting WebKit into Krystal.Booey

Booey's `CSS/` and `Style/` code is being ported from WebKit, one file or slice at a time, with the user reviewing each step. `docs/booey/PORTING.md` is the ledger: translation rules, verdicts per WebKit file, bugs fixed, differences, and a "Where we are" section. Read its "Where we are" and "Translation rules" sections before starting. Don't port ahead of what was asked, and ask before editing unreviewed files outside the current slice.

### Per-file checklist

Do these in order for each WebKit file (or tightly coupled group, e.g. a `.h`/`.cpp` pair):

1. **Graph.** `python tools/graph.py <webkit path> --headers-only --out build/port/<name>.md` (from the repository root). The output goes in the untracked build directory; never write it into the source tree. Also read the WebKit `.cpp`'s own includes: headers-only mode doesn't follow them. Only `css/` and `style/` are traversed by default; pass `--follow css/ style/ platform/graphics/` (etc.) for other directories: `--follow` replaces the defaults, so list `css/` and `style/` too. Note anything new for the ledger.
2. **Read both sides in full.** The WebKit `.h`/`.cpp` (`C:\Users\dylan\repos\WebKit\Source\WebCore\...`) and the Krystal counterpart, if one exists.
3. **Compare.** List declarations missing on either side, behaviour that differs, and bugs in the Krystal version (wrong semantics, undefined behaviour, members that are declared but never defined, wrong enum traits, names that clash with types).
4. **Ask** before anything that changes scope: pulling in unreviewed files, choosing between upstream behaviour and the existing Krystal behaviour, or a decision that will affect other slices.
5. **Fix and complete.** Fix bugs, add missing methods, keep upstream comments, follow the style above. A `.cpp` includes its own header first. New or rewritten files are UTF-8 with BOM.
6. **Build.** Add reviewed files to `KRYSTAL_BOOEY_CSS_HEADERS`/`KRYSTAL_BOOEY_CSS_SOURCES` in `Krystal.Booey/CMakeLists.txt`. Build: `cmake --build --preset windows-x64-debug --target Krystal.Booey.Tests -- -v:quiet -nologo` using Visual Studio's bundled CMake (see "Building and testing" above; only read errors, not progress output). No warnings: warnings are errors.
7. **Test.** Add Catch2 tests under `Krystal.Booey.Tests/tests/CSS/...`, mirroring the source path, for anything that can be instantiated. Templates must be instantiated by a test, or their bugs stay hidden. Known bugs outside the slice get a test tagged `[!shouldfail]` with a FIXME. Run `Debug/Krystal.Booey.Tests.exe` from `build/Krystal.Booey.Tests`; the whole suite must pass.
8. **Ledger.** Update `docs/booey/PORTING.md` using the template below, and the "Where we are" section.
9. **Report** what changed, what's flagged, and what's next. The user commits (or asks you to).

### Ledger entry template

```markdown
### `<Krystal name>` (<date>)

Graph from `<webkit entry>` (headers-only): N files. <Scope notes.>

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|

**Bugs fixed:** one bullet per bug: what was wrong and its effect.
**Added (upstream API):** methods/types added.
**Differences:** intentional or unresolved differences from upstream.
Tests: `<test files>`: N cases / N assertions pass.
```

### Gotchas

- Only files listed in the CMake lists are built; the rest of `CSS/`/`Style/` is unreviewed and may not compile.
- A trial compile (temporarily adding a file to CMake) must be followed by re-running `cmake --preset windows-x64` after reverting, or the generated project keeps the trial file.
- Tests that create `Value`s need the static value pool: use `ValueFixture` from `Krystal.Booey.Tests/CSS/ValueTestUtils.hpp` (it also has `CssText`, `ToNarrow` and `Children`).
- Comments must not mention WebKit or the port unless they are FIXMEs; upstream differences belong in `docs/booey/PORTING.md`.
- Mark open work with plain `// FIXME: ...` comments, never `TODO` (or tagged forms such as `TODO(impl)`), so there's one marker to search for.
- Format with Visual Studio's clang-format: `C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin/clang-format.exe -i <files>`. Working-tree files are CRLF (`core.autocrlf` + `text=auto`); that's expected.
- Catch2 can't print `std::u8string` (shows `{?}`); to see a failing string, print it from a temporary test.
- Inside `css::calc`, `Number`/`Percentage` are calc nodes; the primitives are `css::Number<...>`.
- Bash heredocs containing apostrophes fail in this environment; write such scripts with the Write tool.
