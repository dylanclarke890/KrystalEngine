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

- `docs/booey/OVERVIEW.md`: what Krystal.Booey is, its scope, the translation rules for code that mirrors WebKit, the build, and how each area is put together.
- `docs/booey/DIFFERENCES.md`: where Booey differs from the specs, WebKit and Chrome, by area.
- `docs/booey/KNOWN-ISSUES.md`: known bugs and missing features, by area.
- `docs/gfx/OVERVIEW.md`: Krystal.Gfx's RHI and Render2D, and Krystal.UI's integration of Booey, with their issues and differences.
- `docs/specs/DOM.md`, `docs/specs/INFRA.md`: review checklists of the specifications' algorithms, by section, generated by `tools/spec_checklist.py` (CSS modules go in `docs/specs/css/`).
- `tools/spec_status.py`: reports each checklist's progress, and where each algorithm is linked from the code and tested.
- `tools/graph.py`: walks WebKit's include graph, for porting more of it.
- `tools/golden/`: captures Chrome's reference images for the goldens.

## Krystal.Booey

Booey is a C++ port of WebKit's WebCore (`C:\Users\dylan\repos\WebKit\Source\WebCore\`, commit `300e80c70fd3`) and keeps its structure, so most of its code has an upstream counterpart under the same name in PascalCase. Read `docs/booey/OVERVIEW.md` before working on it.

- **Fixing or reviewing mirrored code:** read the upstream file too, and keep the two in step unless `DIFFERENCES.md` records why not. Follow the translation rules in `OVERVIEW.md`.
- **Keep the documents current:** a new or changed difference from the specs, WebKit or Chrome goes in `DIFFERENCES.md`; a bug found but not fixed gets a `[!shouldfail]` test with a FIXME and an entry in `KNOWN-ISSUES.md`; a fix removes its entry.
- **Reviewing against a specification:** work through a checklist in `docs/specs/` a section at a time (`python tools/spec_status.py dom <section anchor>` lists each algorithm's code and tests). Label each step in its implementation as `// 1.`, `// 1a.`, `// 1a.i.` followed by the step's text, as the checklist numbers them; a step left out on purpose is `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, and a missing or wrong one is a bug as above. Tag the tests that cover an algorithm with `[<spec>:<anchor>]` (`[dom:concept-node-pre-insert]`), tick it, and set the section's `Status:` line to `reviewed (<date>, <commit>)`. Sections without algorithms (such as Trees) still get reviewed: check their definitions against the code, note where they live in `>` lines under the status, and tag their tests with the definition's or section's anchor. Existing tests that cover what's being reviewed move into the test file matching the file that implements it (pre-insert's tests into `tests/DOM/Algorithms/MutationAlgorithms.cpp`) and are renamed after the algorithm and behaviour they test.
- **Porting more of WebKit** (such as the rest of the Typed OM): graph the entry file with `python tools/graph.py <webkit path> --headers-only --out build/port/<name>.md` from the repository root (output stays in `build/`; `--follow css/ style/ platform/graphics/` and so on replaces the default directories), read the WebKit `.h`/`.cpp` and any Krystal counterpart in full, ask before anything that changes scope, keep upstream comments, and add tests. Templates must be instantiated by a test, or their bugs stay hidden.
- **Scope:** keep everything in a standard, even what Booey doesn't use yet; drop `-webkit-`/`-apple-` and non-standard features (see `OVERVIEW.md`).

### Gotchas

- Every header and source is listed in `Krystal.Booey/CMakeLists.txt`; a file that isn't listed isn't built. After a trial compile (temporarily adding a file), re-run `cmake --preset windows-x64` once it's reverted, or the generated project keeps the file.
- Booey is a unity build: file-local helpers share a batch, so make them unique or shared, and include what each file uses (see *The build* in `OVERVIEW.md`).
- Tests that create `Value`s need the static value pool: use `ValueFixture` from `Krystal.Booey.Tests/CSS/ValueTestUtils.hpp` (it also has `CssText`, `ToNarrow` and `Children`).
- Comments must not mention WebKit, upstream or the port unless they are FIXMEs; differences belong in `docs/booey/DIFFERENCES.md`.
- Mark open work with plain `// FIXME: ...` comments, never `TODO` (or tagged forms such as `TODO(impl)`), so there's one marker to search for.
- Format with Visual Studio's clang-format: `C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin/clang-format.exe -i <files>`. Working-tree files are CRLF (`core.autocrlf` + `text=auto`); that's expected.
- Catch2 can't print `std::u8string` (shows `{?}`); to see a failing string, print it from a temporary test.
- Inside `css::calc`, `Number`/`Percentage` are calc nodes; the primitives are `css::Number<...>`.
- Bash heredocs containing apostrophes fail in this environment; write such scripts with the Write tool.
