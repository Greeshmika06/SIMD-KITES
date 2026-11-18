from pathlib import Path

from PIL import Image


ROOT = Path(__file__).parent
WIDTH = 1408
HEIGHT = 655


def unpack(input_name: str, output_name: str) -> None:
    values = [
        int(line.strip(), 16)
        for line in (ROOT / input_name).open()
        if line.strip()
    ]
    pixels = [
        (value >> (8 * byte)) & 0xFF
        for value in values
        for byte in range(8)
    ]

    image = Image.new("L", (WIDTH, HEIGHT))
    image.putdata(pixels[:WIDTH * HEIGHT])
    image.save(ROOT / output_name)
    print(f"Difference image written to {output_name}: {image.size}")


unpack("pixel_difference_simd.txt", "pixel_difference_simd.png")
unpack("pixel_difference_scalar.txt", "pixel_difference_scalar.png")
