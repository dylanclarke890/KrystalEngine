#pragma once

#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <cassert>
#include <format>

#pragma region Helper Macros

#define KRYS_FOR_EACH_KEY_STATE(func)                                                                        \
  func(Krys::Platform::KeyState::Pressed);                                                                   \
  func(Krys::Platform::KeyState::Held);                                                                      \
  func(Krys::Platform::KeyState::Released)

#define KRYS_FOR_EACH_KEY(func)                                                                              \
  func(Krys::Platform::Key::UNKNOWN);                                                                        \
  func(Krys::Platform::Key::Num0);                                                                           \
  func(Krys::Platform::Key::Num1);                                                                           \
  func(Krys::Platform::Key::Num2);                                                                           \
  func(Krys::Platform::Key::Num3);                                                                           \
  func(Krys::Platform::Key::Num4);                                                                           \
  func(Krys::Platform::Key::Num5);                                                                           \
  func(Krys::Platform::Key::Num6);                                                                           \
  func(Krys::Platform::Key::Num7);                                                                           \
  func(Krys::Platform::Key::Num8);                                                                           \
  func(Krys::Platform::Key::Num9);                                                                           \
  func(Krys::Platform::Key::A);                                                                              \
  func(Krys::Platform::Key::B);                                                                              \
  func(Krys::Platform::Key::C);                                                                              \
  func(Krys::Platform::Key::D);                                                                              \
  func(Krys::Platform::Key::E);                                                                              \
  func(Krys::Platform::Key::F);                                                                              \
  func(Krys::Platform::Key::G);                                                                              \
  func(Krys::Platform::Key::H);                                                                              \
  func(Krys::Platform::Key::I);                                                                              \
  func(Krys::Platform::Key::J);                                                                              \
  func(Krys::Platform::Key::K);                                                                              \
  func(Krys::Platform::Key::L);                                                                              \
  func(Krys::Platform::Key::M);                                                                              \
  func(Krys::Platform::Key::N);                                                                              \
  func(Krys::Platform::Key::O);                                                                              \
  func(Krys::Platform::Key::P);                                                                              \
  func(Krys::Platform::Key::Q);                                                                              \
  func(Krys::Platform::Key::R);                                                                              \
  func(Krys::Platform::Key::S);                                                                              \
  func(Krys::Platform::Key::T);                                                                              \
  func(Krys::Platform::Key::U);                                                                              \
  func(Krys::Platform::Key::V);                                                                              \
  func(Krys::Platform::Key::W);                                                                              \
  func(Krys::Platform::Key::X);                                                                              \
  func(Krys::Platform::Key::Y);                                                                              \
  func(Krys::Platform::Key::Z);                                                                              \
  func(Krys::Platform::Key::Equal);                                                                          \
  func(Krys::Platform::Key::Minus);                                                                          \
  func(Krys::Platform::Key::RightBracket);                                                                   \
  func(Krys::Platform::Key::LeftBracket);                                                                    \
  func(Krys::Platform::Key::Quote);                                                                          \
  func(Krys::Platform::Key::SemiColon);                                                                      \
  func(Krys::Platform::Key::Backslash);                                                                      \
  func(Krys::Platform::Key::Comma);                                                                          \
  func(Krys::Platform::Key::Slash);                                                                          \
  func(Krys::Platform::Key::Period);                                                                         \
  func(Krys::Platform::Key::Grave);                                                                          \
  func(Krys::Platform::Key::KeypadDecimal);                                                                  \
  func(Krys::Platform::Key::KeypadMultiply);                                                                 \
  func(Krys::Platform::Key::KeypadPlus);                                                                     \
  func(Krys::Platform::Key::KeypadClear);                                                                    \
  func(Krys::Platform::Key::KeypadDivide);                                                                   \
  func(Krys::Platform::Key::KeypadEnter);                                                                    \
  func(Krys::Platform::Key::KeypadMinus);                                                                    \
  func(Krys::Platform::Key::KeypadEquals);                                                                   \
  func(Krys::Platform::Key::Keypad0);                                                                        \
  func(Krys::Platform::Key::Keypad1);                                                                        \
  func(Krys::Platform::Key::Keypad2);                                                                        \
  func(Krys::Platform::Key::Keypad3);                                                                        \
  func(Krys::Platform::Key::Keypad4);                                                                        \
  func(Krys::Platform::Key::Keypad5);                                                                        \
  func(Krys::Platform::Key::Keypad6);                                                                        \
  func(Krys::Platform::Key::Keypad7);                                                                        \
  func(Krys::Platform::Key::Keypad8);                                                                        \
  func(Krys::Platform::Key::Keypad9);                                                                        \
  func(Krys::Platform::Key::Return);                                                                         \
  func(Krys::Platform::Key::Tab);                                                                            \
  func(Krys::Platform::Key::Space);                                                                          \
  func(Krys::Platform::Key::Del);                                                                            \
  func(Krys::Platform::Key::Escape);                                                                         \
  func(Krys::Platform::Key::Command);                                                                        \
  func(Krys::Platform::Key::Shift);                                                                          \
  func(Krys::Platform::Key::CapsLock);                                                                       \
  func(Krys::Platform::Key::Option);                                                                         \
  func(Krys::Platform::Key::Control);                                                                        \
  func(Krys::Platform::Key::RightShift);                                                                     \
  func(Krys::Platform::Key::RightOption);                                                                    \
  func(Krys::Platform::Key::RightControl);                                                                   \
  func(Krys::Platform::Key::Function);                                                                       \
  func(Krys::Platform::Key::VolumeUp);                                                                       \
  func(Krys::Platform::Key::VolumeDown);                                                                     \
  func(Krys::Platform::Key::Mute);                                                                           \
  func(Krys::Platform::Key::F1);                                                                             \
  func(Krys::Platform::Key::F2);                                                                             \
  func(Krys::Platform::Key::F3);                                                                             \
  func(Krys::Platform::Key::F4);                                                                             \
  func(Krys::Platform::Key::F5);                                                                             \
  func(Krys::Platform::Key::F6);                                                                             \
  func(Krys::Platform::Key::F7);                                                                             \
  func(Krys::Platform::Key::F8);                                                                             \
  func(Krys::Platform::Key::F9);                                                                             \
  func(Krys::Platform::Key::F10);                                                                            \
  func(Krys::Platform::Key::F11);                                                                            \
  func(Krys::Platform::Key::F12);                                                                            \
  func(Krys::Platform::Key::F13);                                                                            \
  func(Krys::Platform::Key::F14);                                                                            \
  func(Krys::Platform::Key::F15);                                                                            \
  func(Krys::Platform::Key::F16);                                                                            \
  func(Krys::Platform::Key::F17);                                                                            \
  func(Krys::Platform::Key::F18);                                                                            \
  func(Krys::Platform::Key::F19);                                                                            \
  func(Krys::Platform::Key::F20);                                                                            \
  func(Krys::Platform::Key::Help);                                                                           \
  func(Krys::Platform::Key::Home);                                                                           \
  func(Krys::Platform::Key::ForwardDelete);                                                                  \
  func(Krys::Platform::Key::End);                                                                            \
  func(Krys::Platform::Key::PageUp);                                                                         \
  func(Krys::Platform::Key::PageDown);                                                                       \
  func(Krys::Platform::Key::LeftArrow);                                                                      \
  func(Krys::Platform::Key::RightArrow);                                                                     \
  func(Krys::Platform::Key::DownArrow);                                                                      \
  func(Krys::Platform::Key::UpArrow);                                                                        \
  func(Krys::Platform::Key::Enter)

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
  case val: return std::format_to(ctx.out(), "{}", KRYS_STRINGIFY(val))

      KRYS_FOR_EACH_KEY(CASE);
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
  case val: return std::format_to(ctx.out(), "{}", KRYS_STRINGIFY(val))

      KRYS_FOR_EACH_KEY_STATE(CASE);
      default: assert(false); break;
#undef CASE
    }

    return std::format_to(ctx.out(), "{}", static_cast<Krys::uint32>(state));
  }
};