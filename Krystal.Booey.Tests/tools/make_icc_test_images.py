"""Generates include/Krystal.Booey.Tests/Platform/ImageDecoders/ICCImages.hpp.

Takes the PNG, WebP and JPEG from TestImages.hpp and embeds an ICC profile in each whose primaries are
sRGB's rotated (the image's red is sRGB's green, its green sRGB's blue and its blue sRGB's red), with sRGB's
transfer curve, so a decoder that applies it turns red pixels green, and so on. It also writes a PNG with
sRGB's own profile, which changes nothing, and a JPEG whose profile is split across two APP2 blocks.

Usage: python make_icc_test_images.py
"""

import pathlib
import re
import struct
import zlib

ROOT = pathlib.Path(__file__).resolve().parent.parent
IMAGES = ROOT / 'include' / 'Krystal.Booey.Tests' / 'Platform' / 'ImageDecoders'

# sRGB's colorants, adapted to D50, as ICC profiles hold them.
SRGB_RED = (0.4360747, 0.2225045, 0.0139322)
SRGB_GREEN = (0.3850649, 0.7168786, 0.0971045)
SRGB_BLUE = (0.1430804, 0.0606169, 0.7141733)
D50 = (0.9642, 1.0, 0.8249)


def read_images():
    text = (IMAGES / 'TestImages.hpp').read_text(encoding='utf-8-sig')
    images = {}
    for name, body in re.findall(r'inline constexpr uint8 (\w+)Bytes\[\] = \{(.*?)\};', text, re.S):
        images[name] = bytes(int(value, 16) for value in re.findall(r'0x[0-9a-fA-F]+', body))
    return images


def s15fixed16(value):
    return struct.pack('>i', round(value * 65536))


def xyz_tag(xyz):
    return b'XYZ ' + b'\0' * 4 + b''.join(s15fixed16(v) for v in xyz)


def text_description(text):
    ascii_text = text.encode('ascii') + b'\0'
    return (b'desc' + b'\0' * 4 + struct.pack('>I', len(ascii_text)) + ascii_text
            + b'\0' * 4 + b'\0' * 4 + b'\0' * 2 + b'\0' + b'\0' * 67)


def srgb_curve():
    # sRGB's own transfer function, as a parametric curve (type 3).
    params = (2.4, 1 / 1.055, 0.055 / 1.055, 1 / 12.92, 0.04045)
    return b'para' + b'\0' * 4 + struct.pack('>HH', 3, 0) + b''.join(s15fixed16(v) for v in params)


def icc_profile(description, red, green, blue, curve):
    tags = [
        (b'desc', text_description(description)),
        (b'wtpt', xyz_tag(D50)),
        (b'rXYZ', xyz_tag(red)),
        (b'gXYZ', xyz_tag(green)),
        (b'bXYZ', xyz_tag(blue)),
        (b'rTRC', curve),
        (b'gTRC', curve),
        (b'bTRC', curve),
        (b'cprt', b'text' + b'\0' * 4 + b'No copyright, use freely.\0'),
    ]
    table_size = 4 + 12 * len(tags)
    offset = 128 + table_size
    table = struct.pack('>I', len(tags))
    data = b''
    for signature, body in tags:
        while len(body) % 4:
            body += b'\0'
        table += signature + struct.pack('>II', offset + len(data), len(body))
        data += body
    size = 128 + table_size + len(data)
    header = (struct.pack('>I', size) + b'none' + bytes([2, 0x10, 0, 0]) + b'mntr' + b'RGB ' + b'XYZ '
              + b'\0' * 12 + b'acsp' + b'\0' * 4 + b'\0' * 4 + b'\0' * 4 + b'\0' * 4 + b'\0' * 8
              + struct.pack('>I', 0) + b''.join(s15fixed16(v) for v in D50) + b'\0' * 4 + b'\0' * 16
              + b'\0' * 28)
    assert len(header) == 128
    return header + table + data


def png_chunk(kind, data):
    return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data) & 0xFFFFFFFF)


def png_with_profile(png, profile):
    # The iCCP chunk goes straight after IHDR (8 bytes of signature, then IHDR's 25).
    iccp = png_chunk(b'iCCP', b'Profile\0\0' + zlib.compress(profile))
    return png[:33] + iccp + png[33:]


def grey_png(gamma, profile):
    # A 1x1 PNG of mid grey (128), with a gAMA chunk and, if there's one, an iCCP profile.
    ihdr = png_chunk(b'IHDR', struct.pack('>IIBBBBB', 1, 1, 8, 2, 0, 0, 0))
    gama = png_chunk(b'gAMA', struct.pack('>I', round(gamma * 100000)))
    iccp = png_chunk(b'iCCP', b'Profile\0\0' + zlib.compress(profile)) if profile else b''
    idat = png_chunk(b'IDAT', zlib.compress(bytes([0, 128, 128, 128])))
    return b'\x89PNG\r\n\x1a\n' + ihdr + gama + iccp + idat + png_chunk(b'IEND', b'')


def jpeg_with_profile(jpeg, profile, blocks=1):
    size = -(-len(profile) // blocks)
    markers = b''
    for index in range(blocks):
        part = profile[index * size:(index + 1) * size]
        data = b'ICC_PROFILE\0' + bytes([index + 1, blocks]) + part
        markers += b'\xff\xe2' + struct.pack('>H', len(data) + 2) + data
    return jpeg[:2] + markers + jpeg[2:]


def webp_with_profile(webp, profile):
    # A simple (VP8L) file becomes an extended one: a VP8X chunk flagging the profile, the ICCP chunk, then
    # the image.
    assert webp[12:16] == b'VP8L'
    bits = int.from_bytes(webp[21:25], 'little')
    width = (bits & 0x3FFF) + 1
    height = ((bits >> 14) & 0x3FFF) + 1
    has_alpha = (bits >> 28) & 1
    flags = 0x20 | (0x10 if has_alpha else 0)
    vp8x = b'VP8X' + struct.pack('<I', 10) + bytes([flags, 0, 0, 0]) + (width - 1).to_bytes(3, 'little') \
        + (height - 1).to_bytes(3, 'little')
    iccp = b'ICCP' + struct.pack('<I', len(profile)) + profile + (b'\0' if len(profile) % 2 else b'')
    body = b'WEBP' + vp8x + iccp + webp[12:]
    return b'RIFF' + struct.pack('<I', len(body)) + body


def array(name, data):
    lines = []
    for start in range(0, len(data), 24):
        lines.append('    ' + ', '.join(f'0x{byte:02x}' for byte in data[start:start + 24]) + ',')
    return (f'  inline constexpr uint8 {name}Bytes[] = {{\n' + '\n'.join(lines) + '\n  };\n'
            f'  inline constexpr Span<const uint8> {name} {{{name}Bytes}};\n')


def main():
    images = read_images()
    rotated = icc_profile('Rotated primaries', SRGB_GREEN, SRGB_BLUE, SRGB_RED, srgb_curve())
    srgb = icc_profile('sRGB', SRGB_RED, SRGB_GREEN, SRGB_BLUE, srgb_curve())
    outputs = [
        ('RotatedProfilePNG', png_with_profile(images['PNG'], rotated)),
        ('SRGBProfilePNG', png_with_profile(images['PNG'], srgb)),
        ('SRGBProfileLinearGammaPNG', grey_png(1.0, srgb)),
        ('LinearGammaPNG', grey_png(1.0, None)),
        ('RotatedProfileWebP', webp_with_profile(images['WebP'], rotated)),
        ('RotatedProfileJPEG', jpeg_with_profile(images['JPEG'], rotated)),
        ('SplitRotatedProfileJPEG', jpeg_with_profile(images['JPEG'], rotated, blocks=2)),
    ]
    header = '''﻿#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Span.hpp"

// Generated by tools/make_icc_test_images.py from TestImages.hpp's images, with ICC profiles embedded: the
// rotated profile's primaries are sRGB's rotated (red is sRGB's green, green sRGB's blue, blue sRGB's red)
// and sRGB's transfer curve; the sRGB one is sRGB's own. The split JPEG's profile is in two APP2 blocks.
// The linear gamma PNG is a mid grey pixel with sRGB's profile and a gAMA chunk of 1, which the profile
// overrides; the other linear gamma PNG has no profile.
namespace krys::boo::tests::images
{
'''
    body = '\n'.join(array(name, data) for name, data in outputs)
    text = header + body + '}\n'
    (IMAGES / 'ICCImages.hpp').write_bytes(text.replace('\n', '\r\n').encode('utf-8'))


if __name__ == '__main__':
    main()
