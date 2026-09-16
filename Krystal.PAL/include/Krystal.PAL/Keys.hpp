#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <format>

#pragma region Helper Macros

#define KRYS_FOR_EACH_KEY_STATE(func)                                                                        \
  func(krys::pal::KeyState::Pressed);                                                                        \
  func(krys::pal::KeyState::Held);                                                                           \
  func(krys::pal::KeyState::Released)

#define KRYS_FOR_EACH_KEY(func)                                                                              \
  func(krys::pal::Key::UNKNOWN);                                                                             \
  func(krys::pal::Key::Num0);                                                                                \
  func(krys::pal::Key::Num1);                                                                                \
  func(krys::pal::Key::Num2);                                                                                \
  func(krys::pal::Key::Num3);                                                                                \
  func(krys::pal::Key::Num4);                                                                                \
  func(krys::pal::Key::Num5);                                                                                \
  func(krys::pal::Key::Num6);                                                                                \
  func(krys::pal::Key::Num7);                                                                                \
  func(krys::pal::Key::Num8);                                                                                \
  func(krys::pal::Key::Num9);                                                                                \
  func(krys::pal::Key::A);                                                                                   \
  func(krys::pal::Key::B);                                                                                   \
  func(krys::pal::Key::C);                                                                                   \
  func(krys::pal::Key::D);                                                                                   \
  func(krys::pal::Key::E);                                                                                   \
  func(krys::pal::Key::F);                                                                                   \
  func(krys::pal::Key::G);                                                                                   \
  func(krys::pal::Key::H);                                                                                   \
  func(krys::pal::Key::I);                                                                                   \
  func(krys::pal::Key::J);                                                                                   \
  func(krys::pal::Key::K);                                                                                   \
  func(krys::pal::Key::L);                                                                                   \
  func(krys::pal::Key::M);                                                                                   \
  func(krys::pal::Key::N);                                                                                   \
  func(krys::pal::Key::O);                                                                                   \
  func(krys::pal::Key::P);                                                                                   \
  func(krys::pal::Key::Q);                                                                                   \
  func(krys::pal::Key::R);                                                                                   \
  func(krys::pal::Key::S);                                                                                   \
  func(krys::pal::Key::T);                                                                                   \
  func(krys::pal::Key::U);                                                                                   \
  func(krys::pal::Key::V);                                                                                   \
  func(krys::pal::Key::W);                                                                                   \
  func(krys::pal::Key::X);                                                                                   \
  func(krys::pal::Key::Y);                                                                                   \
  func(krys::pal::Key::Z);                                                                                   \
  func(krys::pal::Key::Equal);                                                                               \
  func(krys::pal::Key::Minus);                                                                               \
  func(krys::pal::Key::RightBracket);                                                                        \
  func(krys::pal::Key::LeftBracket);                                                                         \
  func(krys::pal::Key::Quote);                                                                               \
  func(krys::pal::Key::SemiColon);                                                                           \
  func(krys::pal::Key::Backslash);                                                                           \
  func(krys::pal::Key::Comma);                                                                               \
  func(krys::pal::Key::Slash);                                                                               \
  func(krys::pal::Key::Period);                                                                              \
  func(krys::pal::Key::Grave);                                                                               \
  func(krys::pal::Key::KeypadDecimal);                                                                       \
  func(krys::pal::Key::KeypadMultiply);                                                                      \
  func(krys::pal::Key::KeypadPlus);                                                                          \
  func(krys::pal::Key::KeypadClear);                                                                         \
  func(krys::pal::Key::KeypadDivide);                                                                        \
  func(krys::pal::Key::KeypadEnter);                                                                         \
  func(krys::pal::Key::KeypadMinus);                                                                         \
  func(krys::pal::Key::KeypadEquals);                                                                        \
  func(krys::pal::Key::Keypad0);                                                                             \
  func(krys::pal::Key::Keypad1);                                                                             \
  func(krys::pal::Key::Keypad2);                                                                             \
  func(krys::pal::Key::Keypad3);                                                                             \
  func(krys::pal::Key::Keypad4);                                                                             \
  func(krys::pal::Key::Keypad5);                                                                             \
  func(krys::pal::Key::Keypad6);                                                                             \
  func(krys::pal::Key::Keypad7);                                                                             \
  func(krys::pal::Key::Keypad8);                                                                             \
  func(krys::pal::Key::Keypad9);                                                                             \
  func(krys::pal::Key::Return);                                                                              \
  func(krys::pal::Key::Tab);                                                                                 \
  func(krys::pal::Key::Space);                                                                               \
  func(krys::pal::Key::Del);                                                                                 \
  func(krys::pal::Key::Escape);                                                                              \
  func(krys::pal::Key::Command);                                                                             \
  func(krys::pal::Key::Shift);                                                                               \
  func(krys::pal::Key::CapsLock);                                                                            \
  func(krys::pal::Key::Option);                                                                              \
  func(krys::pal::Key::Control);                                                                             \
  func(krys::pal::Key::RightShift);                                                                          \
  func(krys::pal::Key::RightOption);                                                                         \
  func(krys::pal::Key::RightControl);                                                                        \
  func(krys::pal::Key::Function);                                                                            \
  func(krys::pal::Key::VolumeUp);                                                                            \
  func(krys::pal::Key::VolumeDown);                                                                          \
  func(krys::pal::Key::Mute);                                                                                \
  func(krys::pal::Key::F1);                                                                                  \
  func(krys::pal::Key::F2);                                                                                  \
  func(krys::pal::Key::F3);                                                                                  \
  func(krys::pal::Key::F4);                                                                                  \
  func(krys::pal::Key::F5);                                                                                  \
  func(krys::pal::Key::F6);                                                                                  \
  func(krys::pal::Key::F7);                                                                                  \
  func(krys::pal::Key::F8);                                                                                  \
  func(krys::pal::Key::F9);                                                                                  \
  func(krys::pal::Key::F10);                                                                                 \
  func(krys::pal::Key::F11);                                                                                 \
  func(krys::pal::Key::F12);                                                                                 \
  func(krys::pal::Key::F13);                                                                                 \
  func(krys::pal::Key::F14);                                                                                 \
  func(krys::pal::Key::F15);                                                                                 \
  func(krys::pal::Key::F16);                                                                                 \
  func(krys::pal::Key::F17);                                                                                 \
  func(krys::pal::Key::F18);                                                                                 \
  func(krys::pal::Key::F19);                                                                                 \
  func(krys::pal::Key::F20);                                                                                 \
  func(krys::pal::Key::Help);                                                                                \
  func(krys::pal::Key::Home);                                                                                \
  func(krys::pal::Key::ForwardDelete);                                                                       \
  func(krys::pal::Key::End);                                                                                 \
  func(krys::pal::Key::PageUp);                                                                              \
  func(krys::pal::Key::PageDown);                                                                            \
  func(krys::pal::Key::LeftArrow);                                                                           \
  func(krys::pal::Key::RightArrow);                                                                          \
  func(krys::pal::Key::DownArrow);                                                                           \
  func(krys::pal::Key::UpArrow);                                                                             \
  func(krys::pal::Key::Enter)

#pragma endregion

namespace krys::pal
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
struct std::formatter<krys::pal::Key>
{
  constexpr auto parse(std::format_parse_context &ctx)
  {
    return ctx.begin();
  }

  auto format(const krys::pal::Key &key, std::format_context &ctx) const
  {
    switch (key)
    {
#define CASE(val)                                                                                            \
  case val: return std::format_to(ctx.out(), "{}", KRYS_STRINGIFY(val))

      KRYS_FOR_EACH_KEY(CASE);
#undef CASE
    }

    krys_unreachable();
  }
};

template <>
struct std::formatter<krys::pal::KeyState>
{
  constexpr auto parse(std::format_parse_context &ctx)
  {
    return ctx.begin();
  }

  auto format(const krys::pal::KeyState &state, std::format_context &ctx) const
  {
    switch (state)
    {
#define CASE(val)                                                                                            \
  case val: return std::format_to(ctx.out(), "{}", KRYS_STRINGIFY(val))

      KRYS_FOR_EACH_KEY_STATE(CASE);
#undef CASE
    }

    krys_unreachable();
  }
};