"""Generates include/Krystal.Image.Tests/GIFImages.hpp: GIFs that exercise the GIF decoder's harder paths.

- InterlacedGIF: 5x9, each row a colour from a 9-colour table (row y is colour y), stored interlaced.
- NoiseGIF: 64x64 of pseudo-random indices into a 256-colour table, so the LZW dictionary fills and resets.
- DisposalGIF: a 4x4 animation of four frames: red everywhere; green over the top-left 2x2, restored to the
  background after; blue over the bottom-right 2x2, restored to the previous frame after; white at (3, 0).

Usage: python make_gif_test_images.py
"""

import pathlib
import random
import struct

ROOT = pathlib.Path(__file__).resolve().parent.parent
OUTPUT = ROOT / 'include' / 'Krystal.Image.Tests' / 'GIFImages.hpp'

UNSPECIFIED, DO_NOT_DISPOSE, RESTORE_TO_BACKGROUND, RESTORE_TO_PREVIOUS = 0, 1, 2, 3


def lzw_encode(indices, minimum_code_size):
    """The indices LZW-compressed as GIF stores them, packed least significant bit first."""
    clear, end = 1 << minimum_code_size, (1 << minimum_code_size) + 1
    codes = []
    code_size = minimum_code_size + 1
    sizes = []

    def emit(code):
        codes.append(code)
        sizes.append(code_size)

    def reset():
        return {bytes([i]): i for i in range(clear)}, end + 1

    dictionary, next_code = reset()
    emit(clear)
    current = b''
    for index in indices:
        candidate = current + bytes([index])
        if candidate in dictionary:
            current = candidate
            continue
        emit(dictionary[current])
        if next_code < 4096:
            dictionary[candidate] = next_code
            next_code += 1
            # The decoder widens its codes once the dictionary reaches the current width's limit.
            if next_code > (1 << code_size) and code_size < 12:
                code_size += 1
        else:
            emit(clear)
            dictionary, next_code = reset()
            code_size = minimum_code_size + 1
        current = bytes([index])
    emit(dictionary[current])
    emit(end)

    bits, bit_count, out = 0, 0, bytearray()
    for code, size in zip(codes, sizes):
        bits |= code << bit_count
        bit_count += size
        while bit_count >= 8:
            out.append(bits & 0xFF)
            bits >>= 8
            bit_count -= 8
    if bit_count:
        out.append(bits & 0xFF)
    return bytes(out)


def sub_blocks(data):
    out = bytearray()
    for start in range(0, len(data), 255):
        chunk = data[start:start + 255]
        out += bytes([len(chunk)]) + chunk
    return bytes(out + b'\0')


def color_table(colors):
    """A colour table padded to a power of two, and its size field."""
    size_field = max(0, (len(colors) - 1).bit_length() - 1)
    padded = colors + [(0, 0, 0)] * ((2 << size_field) - len(colors))
    return b''.join(bytes(color) for color in padded), size_field


def gif(width, height, colors, frames, loop=None):
    """A GIF89a with a global colour table; each frame is (x, y, w, h, indices, disposal, interlaced)."""
    table, size_field = color_table(colors)
    out = bytearray(b'GIF89a' + struct.pack('<HHBBB', width, height, 0x80 | size_field, 0, 0) + table)
    if loop is not None:
        out += b'\x21\xFF\x0BNETSCAPE2.0\x03\x01' + struct.pack('<H', loop) + b'\0'
    minimum_code_size = max(2, size_field + 1)
    for x, y, w, h, indices, disposal, interlaced in frames:
        out += b'\x21\xF9\x04' + bytes([disposal << 2]) + struct.pack('<H', 10) + b'\0\0'
        out += b'\x2C' + struct.pack('<HHHHB', x, y, w, h, 0x40 if interlaced else 0)
        out += bytes([minimum_code_size]) + sub_blocks(lzw_encode(indices, minimum_code_size))
    return bytes(out + b'\x3B')


def interlaced_order(height):
    return [y for start, step in ((0, 8), (4, 8), (2, 4), (1, 2)) for y in range(start, height, step)]


def cpp_array(name, data):
    lines = [f'  constexpr inline uint8 {name}Bytes[] = {{']
    for start in range(0, len(data), 17):
        lines.append('    ' + ' '.join(f'0x{b:02X},' for b in data[start:start + 17]))
    lines += ['  };', f'  constexpr inline Span<const uint8> {name} {{{name}Bytes}};']
    return '\n'.join(lines)


def main():
    # Row y's colour is (y * 28, 255 - y * 28, y * 10), in a table of 9 colours.
    rows = [(y * 28, 255 - y * 28, y * 10) for y in range(9)]
    interlaced = gif(5, 9, rows, [(0, 0, 5, 9, [y for y in interlaced_order(9) for _ in range(5)], UNSPECIFIED, True)])

    random.seed(1234)
    palette = [(i, (i * 7) % 256, (i * 13) % 256) for i in range(256)]
    noise_indices = [random.randrange(256) for _ in range(64 * 64)]
    noise = gif(64, 64, palette, [(0, 0, 64, 64, noise_indices, UNSPECIFIED, False)])

    red, green, blue, white = 0, 1, 2, 3
    disposal = gif(4, 4, [(255, 0, 0), (0, 255, 0), (0, 0, 255), (255, 255, 255)], [
        (0, 0, 4, 4, [red] * 16, UNSPECIFIED, False),
        (0, 0, 2, 2, [green] * 4, RESTORE_TO_BACKGROUND, False),
        (2, 2, 2, 2, [blue] * 4, RESTORE_TO_PREVIOUS, False),
        (3, 0, 1, 1, [white], UNSPECIFIED, False),
    ], loop=0)

    text = '\n'.join([
        '﻿#pragma once',
        '',
        '#include "Krystal.Core/Containers/Span.hpp"',
        '#include "Krystal.Core/Numeric.hpp"',
        '',
        '// Generated by tools/make_gif_test_images.py: an interlaced GIF (5x9, row y coloured',
        '// (y * 28, 255 - y * 28, y * 10)); 64x64 of pseudo-random indices into a 256-colour table, with the',
        '// generator\'s indices in NoiseGIFIndices; and a 4x4 animation exercising each disposal method.',
        'namespace krys::img::tests::images',
        '{',
        cpp_array('InterlacedGIF', interlaced),
        cpp_array('NoiseGIF', noise),
        cpp_array('NoiseGIFIndices', bytes(noise_indices)),
        cpp_array('NoiseGIFPalette', b''.join(bytes(color) for color in palette)),
        cpp_array('DisposalGIF', disposal),
        '}',
        '',
    ])
    OUTPUT.write_text(text, encoding='utf-8', newline='\n')
    print(f'Wrote {OUTPUT.relative_to(ROOT.parent)}')


if __name__ == '__main__':
    main()
