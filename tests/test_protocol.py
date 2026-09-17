"""Exercise the public C API with capture files and a silent serial pseudo-terminal.

Expected bytes and limits come from ITPP047 manual 1.00, sections 2.6,
2.8, 2.14-15, 2.43, 2.46, 2.48, 2.57, 2.61-63, and 2.72-73.
No physical printer is opened by these tests.
"""
import ctypes as c
import json
import os
import pathlib
import pty
import sys
import tempfile
import time
import unittest

lib = c.CDLL(sys.argv.pop(1))
P, B, U8, U16, SIZE, INT = c.c_void_p, c.c_char_p, c.c_uint8, c.c_uint16, c.c_size_t, c.c_int

class Status(c.Structure):
    _fields_ = [(name, c.c_bool) for name in (
        "paper", "cover", "online", "error", "cut", "recoverable", "unrecoverable")]

signatures = {
    "open_usb": [B, c.POINTER(P)], "open_serial": [B, INT, c.POINTER(P)], "close": [P],
    "read_data": [P, P, SIZE, c.POINTER(SIZE)], "get_status": [P, c.POINTER(Status)],
    "self_test": [P], "print_qr": [P, B, U8, INT], "print_pdf417": [P, B, U8, U8],
    "print_barcode": [P, INT, B], "print_bit_image": [P, U8, U16, B, SIZE],
    "define_downloaded_bit_image": [P, U8, U8, B, SIZE],
    "define_nv_bit_image": [P, U8, B, SIZE], "define_user_defined_chars": [P, U8, U8, U8, B, SIZE],
    "realtime_request": [P, U8], "realtime_drawer_pulse": [P, U8, U8],
    "execute_test_print": [P, U8, U8], "execute_macro": [P, U8, U8, U8],
    "buzzer": [P, U8, U8], "buzzer_alarm": [P, U8, U8, U8],
    "set_page_area": [P, U16, U16, U16, U16], "set_relative_vertical_position": [P, c.c_int16],
    "set_wifi": [P, B, B, INT], "set_dhcp": [P, c.c_bool],
    "set_font": [P, INT],
    "set_profile": [P, INT], "print_encoded": [P, B, INT, U8],
    "print_barcode_bytes": [P, INT, B, SIZE], "define_kanji_char": [P, U8, U8, B, SIZE],
    "set_codepage": [P, INT], "print_and_cut": [P, B, INT],
}
for name, args in signatures.items():
    function = getattr(lib, "munbyn_" + name)
    function.argtypes = args
    function.restype = INT

class ProtocolTests(unittest.TestCase):
    def test_barcode_alphabets_and_binary_code_sets(self):
        cases = json.loads(pathlib.Path(__file__).with_name('barcode_cases.json').read_text())
        for case in cases:
            payload = bytes.fromhex(case['hex']) if 'hex' in case else case['text'].encode('ascii')
            with self.subTest(case=case):
                result, data = self.capture(lambda h: lib.munbyn_print_barcode_bytes(h, case['type'], payload, len(payload)))
                self.assertEqual(result, 0 if case['valid'] else -3)
                if not case['valid']: self.assertEqual(data, b'')
        self.assertEqual(self.capture(lambda h: lib.munbyn_print_barcode_bytes(h, 73, b'{A\0A', 4)),
                         (0, bytes.fromhex('1d6b49047b410041')))

    def test_strict_text_encoding(self):
        samples = [('Čćšžđ', 3, 18, 'cp852'), ('ЉЊЋЂ Ј', 7, 51, 'cp1251'), ('€ £', 8, 16, 'cp1252')]
        for text, encoding, page, codec in samples:
            self.assertEqual(self.capture(lambda h: lib.munbyn_print_encoded(h, text.encode(), encoding, page)),
                (0, bytes.fromhex('1c2e1b52001b74') + bytes([page]) + text.encode(codec)))
        for malformed in [b'\xc0\xaf', b'\xe0\x80\xaf', b'\xed\xa0\x80', b'\xf4\x90\x80\x80', b'\xc4', b'\xc4A', b'\x1b@', '😀'.encode()]:
            self.assertEqual(self.capture(lambda h: lib.munbyn_print_encoded(h, malformed, 3, 18)), (-3, b''))
        self.assertEqual(self.capture(lambda h: lib.munbyn_set_codepage(h, 69)), (0, bytes.fromhex('1b7445')))

    def test_kanji_glyph_and_native_capability_guard(self):
        glyph = b'\x80' * 72
        self.assertEqual(self.capture(lambda h: lib.munbyn_define_kanji_char(h, 0xfe, 0xa1, glyph, 72)),
                         (0, bytes.fromhex('1c32fea1') + glyph))
        for c1, c2, size in [(0xff, 0xa1, 72), (0xfe, 0xa0, 72), (0xfe, 0xff, 72), (0xfe, 0xa1, 71)]:
            self.assertEqual(self.capture(lambda h: lib.munbyn_define_kanji_char(h, c1, c2, glyph, size)), (-3, b''))
        def attempt(handle):
            self.assertEqual(lib.munbyn_set_profile(handle, 1), 0)
            return lib.munbyn_print_pdf417(handle, b'abc', 0, 1)
        self.assertEqual(self.capture(attempt), (-7, b''))
        self.assertEqual(self.capture(lambda h: lib.munbyn_print_and_cut(h, b'text', 99)), (-3, b''))

    def capture(self, action, initial=b""):
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "capture"
            path.write_bytes(initial)
            handle = P()
            self.assertEqual(lib.munbyn_open_usb(os.fsencode(path), c.byref(handle)), 0)
            try:
                result = action(handle)
            finally:
                lib.munbyn_close(handle)
            return result, path.read_bytes()

    def test_documented_commands(self):
        cases = [
            ("realtime_request", (2,), "10 05 02"),
            ("realtime_drawer_pulse", (1, 8), "10 14 01 01 08"),
            ("execute_test_print", (0, 1), "1d 28 41 02 00 00 01"),
            ("buzzer", (9, 9), "1b 42 09 09"),
            ("buzzer_alarm", (20, 20, 3), "1b 43 14 14 03"),
            ("execute_macro", (2, 10, 1), "1d 5e 02 0a 01"),
            ("set_page_area", (1, 256, 512, 1024), "1b 57 01 00 00 01 00 02 00 04"),
            ("set_relative_vertical_position", (-2,), "1d 5c fe ff"),
            ("print_barcode", (73, b"{B123"), "1d 6b 49 05 7b 42 31 32 33"),
            ("print_bit_image", (33, 1, b"\x80\x40\x20", 3), "1b 2a 21 01 00 80 40 20"),
        ]
        for name, args, expected in cases:
            with self.subTest(name=name):
                result, data = self.capture(lambda h: getattr(lib, "munbyn_" + name)(h, *args))
                self.assertEqual(result, 0)
                self.assertEqual(data, bytes.fromhex(expected))

    def test_extension_framing(self):
        # These extensions are not guaranteed by the bundled ITPP047 manual.
        self.assertEqual(self.capture(lambda h: lib.munbyn_self_test(h)), (0, bytes.fromhex("1f 1b 1f 67")))
        result, data = self.capture(lambda h: lib.munbyn_print_pdf417(h, b"abc", 0, 1))
        self.assertEqual(result, 0)
        self.assertEqual(data, bytes.fromhex(
            "1d 28 6b 03 00 30 41 00 1d 28 6b 04 00 30 45 30 31 "
            "1d 28 6b 06 00 30 50 30 61 62 63 1d 28 6b 03 00 30 51 30"))
        result, data = self.capture(lambda h: lib.munbyn_print_qr(h, b"abc", 6, 49))
        self.assertEqual(result, 0)
        self.assertIn(bytes.fromhex("1d 28 6b 06 00 31 50 30 61 62 63"), data)
        self.assertEqual(self.capture(lambda h: lib.munbyn_set_wifi(h, b"test", b"password", 8)),
                         (0, bytes.fromhex("1f 1b 1f b3 08") + b"test\0password\0"))

    def test_pdf417_length_boundary(self):
        result, data = self.capture(lambda h: lib.munbyn_print_pdf417(h, b"x" * 65532, 0, 1))
        self.assertEqual(result, 0)
        self.assertEqual(data[20:22], b"\xff\xff")
        self.assertEqual(len(data), 65532 + 33)
        self.assertEqual(self.capture(lambda h: lib.munbyn_print_pdf417(h, b"x" * 65533, 0, 1)), (-3, b""))

    def test_invalid_parameters_write_nothing(self):
        cases = [
            ("print_qr", (b"", 6, 49)), ("print_qr", (b"x", 17, 49)),
            ("print_qr", (b"x" * 7090, 6, 49)),
            ("print_pdf417", (b"x", 31, 1)), ("print_pdf417", (b"x", 0, 9)),
            ("realtime_request", (3,)), ("realtime_drawer_pulse", (0, 9)),
            ("execute_test_print", (0, 2)), ("execute_macro", (1, 0, 2)),
            ("buzzer_alarm", (21, 1, 3)), ("buzzer_alarm", (1, 0, 3)),
            ("print_bit_image", (0, 1024, b"x" * 1024, 1024)),
            ("print_bit_image", (33, 1, b"x", 1)),
            ("define_downloaded_bit_image", (20, 48, b"x" * 7680, 7680)),
            ("define_nv_bit_image", (1, b"\x01\0\x01\0x", 5)),
            ("define_user_defined_chars", (2, 65, 65, b"\x01xx", 3)),
            ("define_user_defined_chars", (3, 65, 65, b"\x01xx", 3)),
            ("set_wifi", (b"test", b"pass", -1)),
        ]
        for name, args in cases:
            with self.subTest(name=name, args=str(args)[:80]):
                self.assertEqual(self.capture(lambda h: getattr(lib, "munbyn_" + name)(h, *args)), (-3, b""))

    def test_image_and_glyph_boundaries(self):
        data = b"x" * (19 * 48 * 8)
        result, output = self.capture(lambda h: lib.munbyn_define_downloaded_bit_image(h, 19, 48, data, len(data)))
        self.assertEqual((result, output), (0, b"\x1d\x2a\x13\x30" + data))
        data = b"\x01\0\x01\0" + b"\x80" * 8
        self.assertEqual(self.capture(lambda h: lib.munbyn_define_nv_bit_image(h, 1, data, len(data))),
                         (0, b"\x1c\x71\x01" + data))
        for malformed in (data[:-1], data + b"x", b"\0\0\x01\0" + b"x" * 8):
            self.assertEqual(self.capture(lambda h: lib.munbyn_define_nv_bit_image(h, 1, malformed, len(malformed))), (-3, b""))
        glyph = b"\x01\x80\x40\x20"
        self.assertEqual(self.capture(lambda h: lib.munbyn_define_user_defined_chars(h, 3, 65, 65, glyph, 4)),
                         (0, b"\x1b\x26\x03AA" + glyph))

    def status(self, replies):
        status = Status()
        initial = b"".join(b"\0\0\0" + bytes([b]) for b in replies)
        result, output = self.capture(lambda h: lib.munbyn_get_status(h, c.byref(status)), initial)
        return result, status, output

    def test_status_groups_and_bit_mapping(self):
        result, status, output = self.status([0x16, 0x12, 0x12, 0x12])
        self.assertEqual(result, 0)
        self.assertTrue(status.online and status.paper and status.cover)
        self.assertFalse(status.error)
        self.assertEqual(output, bytes.fromhex("10 04 01 16 10 04 02 12 10 04 03 12 10 04 04 12"))
        result, status, _ = self.status([0x1a, 0x56, 0x7a, 0x72])
        self.assertEqual(result, 0)
        self.assertFalse(status.online or status.paper or status.cover)
        self.assertTrue(status.error and status.cut and status.recoverable and status.unrecoverable)
        self.assertTrue(self.status([0x12, 0x52, 0x12, 0x12])[1].error)

    def test_incomplete_or_invalid_status_is_not_success(self):
        for replies in ([], [0x12], [0x12, 0x12], [0x12, 0x12, 0x12], [0x00]):
            with self.subTest(replies=replies):
                result, status, _ = self.status(replies)
                self.assertNotEqual(result, 0)
                self.assertFalse(any(getattr(status, field) for field, _ in status._fields_))

    def test_silent_serial_port_times_out(self):
        master, slave = pty.openpty()
        handle = P()
        try:
            self.assertEqual(lib.munbyn_open_serial(os.ttyname(slave).encode(), 9600, c.byref(handle)), 0)
            buffer, count = c.create_string_buffer(1), SIZE()
            start = time.monotonic()
            self.assertEqual(lib.munbyn_read_data(handle, buffer, 1, c.byref(count)), -5)
            self.assertLess(time.monotonic() - start, 2.5)
            self.assertEqual(count.value, 0)
        finally:
            if handle: lib.munbyn_close(handle)
            os.close(master)
            os.close(slave)

if __name__ == "__main__":
    unittest.main()
