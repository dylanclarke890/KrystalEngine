# KrystalEngine

C++ game engine (latest standard, CMake + vcpkg). Each module has its own `CMakeLists.txt`, `include/<Module>/` and `src/` (same folder structure), and keeps its documents in `docs/` and scripts in `tools/`.

**Working on Krystal.Booey or its tests? Read `Krystal.Booey/CLAUDE.md` first.** Krystal.Gfx and Krystal.UI are described in their `docs/OVERVIEW.md`.

## Code style

- **Naming:** namespaces lowercase (`krys`, `krys::detail` for implementation details); types, functions, methods and enum values `PascalCase`; private/protected data members `_camelCase`, public members of plain structs `PascalCase`; locals and parameters `camelCase`; macros `KRYS_UPPER_CASE`. Template params are `T`/`U` or `TName` (`TEvent`), with a concept rather than `typename` when one fits. Lowercase exceptions: assertions (`krys_assert`, `krys_debug_assert`), std-like helpers (`krys::move`, `ipow`) and numeric aliases (`uint32`, `byte`).
- **Types:** use the engine's (`uint32`, `List`, `HashMap`, `Span`, `UniquePtr`, `Func`, `Maybe`, `Expected`) rather than std's, and `Krystal.Core/TypeTraits.hpp`/`Concepts.hpp` rather than `<type_traits>`/`<concepts>`. Convert `Ref`↔`RefPtr` by assignment (`ShareRef`/`ShareRefPtr` only from raw references or pointers). `CheckedPtr` targets must be heap-allocated (`CreateUnique`). `EnumSet` takes ordinal enumerators, not `1 << n` masks.
- **Parameters:** sink parameters (moved into a member or passed on) by value, not `&&`; read-only ones as a view or `const &`; `&&` only for move-only types and forwarding references. Don't pair a by-value overload with a `const &` one.
- **Specifiers:** `KRYS_*` attribute macros, never raw `[[...]]`. `KRYS_NODISCARD` where the return value is the point; `noexcept` unless the function can genuinely fail by throwing (conditional in templates); `constexpr` wherever it compiles; single-argument constructors `explicit`. Order: `KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static inline <return> Name(...) const noexcept`. Branch hints after the condition (`if (ptr) KRYS_LIKELY`). Copy/move suppression with `KRYS_NON_COPYABLE(X);`, `KRYS_NON_MOVABLE(X);`, `KRYS_NON_COPY_MOVABLE(X);`.
- **Class layout**, where it fits: friends, private aliases, copy/move macros; then `private:` data members (brace-initialised); then `public:` constructors and destructor, then the public interface; then private helpers. `struct` for all-public policy, trait and plain-data types.
- **Files:** UTF-8 with BOM, kept when rewriting; working-tree files are CRLF (expected). Headers: `#pragma once`, project includes (`"Krystal.Core/..."`), then std headers. Third-party sources outside vcpkg go in `third-party/`, included with `<...>`.
- **Comments:** Doxygen `///` with `@brief` (plus `@param`/`@return`/`@note` where they add something); `//` for implementation notes. Open work is a plain `// FIXME:`, never `TODO`.
- Brace every `if`/loop body.
- **Formatting** is `.clang-format`'s: run Visual Studio's `C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin/clang-format.exe -i <files>` on code files only, never by hand.

## Building and testing

Use Visual Studio's bundled CMake (`C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe`) with the `windows-x64` presets, never the system CMake or the bare `vcpkg` preset. Keep tool output out of context: look only at whether a step succeeded, or at the specific errors.

- **Configure:** `cmake --preset windows-x64 --log-level=WARNING > build/configure.log 2>&1`; read the log only on a non-zero exit code.
- **Build:** `cmake --build --preset windows-x64-debug --target <target> -- -v:quiet -nologo` (silent on success; warnings are errors). Read only the first lines of a long template error.
- **Tests:** Catch2 prints only failures and a summary; on a pass read the last few lines. Templates must be instantiated by a test, or their bugs stay hidden.
- Bash heredocs containing apostrophes fail in this environment; write such scripts with the Write tool.
