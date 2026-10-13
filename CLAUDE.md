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
