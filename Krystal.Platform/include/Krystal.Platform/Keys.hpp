#pragma once

#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Macros.hpp"
#include <cassert>
#include <format>

#pragma region Helper Macros

#define FOR_EACH_KEY_STATE(func)                                                                             \
  func(Pressed);                                                                                             \
  func(Held);                                                                                                \
  func(Released)

#define FOR_EACH_KEY(func)                                                                                   \
  func(UNKNOWN);                                                                                             \
  func(Num0);                                                                                                \
  func(Num1);                                                                                                \
  func(Num2);                                                                                                \
  func(Num3);                                                                                                \
  func(Num4);                                                                                                \
  func(Num5);                                                                                                \
  func(Num6);                                                                                                \
  func(Num7);                                                                                                \
  func(Num8);                                                                                                \
  func(Num9);                                                                                                \
  func(A);                                                                                                   \
  func(B);                                                                                                   \
  func(C);                                                                                                   \
  func(D);                                                                                                   \
  func(E);                                                                                                   \
  func(F);                                                                                                   \
  func(G);                                                                                                   \
  func(H);                                                                                                   \
  func(I);                                                                                                   \
  func(J);                                                                                                   \
  func(K);                                                                                                   \
  func(L);                                                                                                   \
  func(M);                                                                                                   \
  func(N);                                                                                                   \
  func(O);                                                                                                   \
  func(P);                                                                                                   \
  func(Q);                                                                                                   \
  func(R);                                                                                                   \
  func(S);                                                                                                   \
  func(T);                                                                                                   \
  func(U);                                                                                                   \
  func(V);                                                                                                   \
  func(W);                                                                                                   \
  func(X);                                                                                                   \
  func(Y);                                                                                                   \
  func(Z);                                                                                                   \
  func(Equal);                                                                                               \
  func(Minus);                                                                                               \
  func(RightBracket);                                                                                        \
  func(LeftBracket);                                                                                         \
  func(Quote);                                                                                               \
  func(SemiColon);                                                                                           \
  func(Backslash);                                                                                           \
  func(Comma);                                                                                               \
  func(Slash);                                                                                               \
  func(Period);                                                                                              \
  func(Grave);                                                                                               \
  func(KeypadDecimal);                                                                                       \
  func(KeypadMultiply);                                                                                      \
  func(KeypadPlus);                                                                                          \
  func(KeypadClear);                                                                                         \
  func(KeypadDivide);                                                                                        \
  func(KeypadEnter);                                                                                         \
  func(KeypadMinus);                                                                                         \
  func(KeypadEquals);                                                                                        \
  func(Keypad0);                                                                                             \
  func(Keypad1);                                                                                             \
  func(Keypad2);                                                                                             \
  func(Keypad3);                                                                                             \
  func(Keypad4);                                                                                             \
  func(Keypad5);                                                                                             \
  func(Keypad6);                                                                                             \
  func(Keypad7);                                                                                             \
  func(Keypad8);                                                                                             \
  func(Keypad9);                                                                                             \
  func(Return);                                                                                              \
  func(Tab);                                                                                                 \
  func(Space);                                                                                               \
  func(Del);                                                                                                 \
  func(Escape);                                                                                              \
  func(Command);                                                                                             \
  func(Shift);                                                                                               \
  func(CapsLock);                                                                                            \
  func(Option);                                                                                              \
  func(Control);                                                                                             \
  func(RightShift);                                                                                          \
  func(RightOption);                                                                                         \
  func(RightControl);                                                                                        \
  func(Function);                                                                                            \
  func(VolumeUp);                                                                                            \
  func(VolumeDown);                                                                                          \
  func(Mute);                                                                                                \
  func(F1);                                                                                                  \
  func(F2);                                                                                                  \
  func(F3);                                                                                                  \
  func(F4);                                                                                                  \
  func(F5);                                                                                                  \
  func(F6);                                                                                                  \
  func(F7);                                                                                                  \
  func(F8);                                                                                                  \
  func(F9);                                                                                                  \
  func(F10);                                                                                                 \
  func(F11);                                                                                                 \
  func(F12);                                                                                                 \
  func(F13);                                                                                                 \
  func(F14);                                                                                                 \
  func(F15);                                                                                                 \
  func(F16);                                                                                                 \
  func(F17);                                                                                                 \
  func(F18);                                                                                                 \
  func(F19);                                                                                                 \
  func(F20);                                                                                                 \
  func(Help);                                                                                                \
  func(Home);                                                                                                \
  func(ForwardDelete);                                                                                       \
  func(End);                                                                                                 \
  func(PageUp);                                                                                              \
  func(PageDown);                                                                                            \
  func(LeftArrow);                                                                                           \
  func(RightArrow);                                                                                          \
  func(DownArrow);                                                                                           \
  func(UpArrow);                                                                                             \
  func(Enter)

#pragma endregion

namespace Krys::Platform
{
  /// @brief The state of a key during an interaction.
  enum class KeyState : uint8
  {
    Pressed,
    Held,
    Released
  };

  /// @brief Supported keyboard keys.
  enum class Key : uint32
  {
    UNKNOWN,
    Num0,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    Equal,
    Minus,
    RightBracket,
    LeftBracket,
    Quote,
    SemiColon,
    Backslash,
    Comma,
    Slash,
    Period,
    Grave,
    KeypadDecimal,
    KeypadMultiply,
    KeypadPlus,
    KeypadClear,
    KeypadDivide,
    KeypadEnter,
    KeypadMinus,
    KeypadEquals,
    Keypad0,
    Keypad1,
    Keypad2,
    Keypad3,
    Keypad4,
    Keypad5,
    Keypad6,
    Keypad7,
    Keypad8,
    Keypad9,
    Return,
    Tab,
    Space,
    Del,
    Escape,
    Command,
    Shift,
    CapsLock,
    Option,
    Control,
    RightShift,
    RightOption,
    RightControl,
    Function,
    VolumeUp,
    VolumeDown,
    Mute,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    Help,
    Home,
    ForwardDelete,
    End,
    PageUp,
    PageDown,
    LeftArrow,
    RightArrow,
    DownArrow,
    UpArrow,
    Enter
  };
}

template <>
struct std::formatter<Krys::Platform::Key>
{
  constexpr auto parse(std::format_parse_context &ctx)
  {
    return ctx.begin();
  }

  auto format(const Krys::Platform::Key &key, std::format_context &ctx) const
  {
    switch (key)
    {
#define CASE(val)                                                                                            \
  case CONCAT(Krys::Platform::Key::, val): return std::format_to(ctx.out(), "{}", STRINGIFY(val))

      FOR_EACH_KEY(CASE);
      default: assert(false); break;

#undef CASE
    }

    return std::format_to(ctx.out(), "{}", static_cast<Krys::uint32>(key));
  }
};

template <>
struct std::formatter<Krys::Platform::KeyState>
{
  constexpr auto parse(std::format_parse_context &ctx)
  {
    return ctx.begin();
  }

  auto format(const Krys::Platform::KeyState &state, std::format_context &ctx) const
  {
    switch (state)
    {
#define CASE(val)                                                                                            \
  case CONCAT(Krys::Platform::KeyState::, val): return std::format_to(ctx.out(), "{}", STRINGIFY(val))

      FOR_EACH_KEY_STATE(CASE);
      default: assert(false); break;

#undef CASE
    }

    return std::format_to(ctx.out(), "{}", static_cast<Krys::uint32>(state));
  }
};