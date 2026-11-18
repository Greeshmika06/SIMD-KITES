from pathlib import Path

from PIL import Image


ROOT = Path(__file__).parent


def pack_frame(image_name: str, output_name: str) -> None:
    image = Image.open(ROOT / image_name).convert("L")
    pixels = list(image.getdata())

    with (ROOT / output_name).open("w") as output:
        for offset in range(0, len(pixels), 8):
            packed = sum(
                pixel << (8 * byte)
                for byte, pixel in enumerate(pixels[offset:offset + 8])
            )
            output.write(f"{packed:016X}\n")

    print(f"{image_name}: {image.size[0]}x{image.size[1]} pixels")
    print(f"Packed values written to {output_name}: {(len(pixels) + 7) // 8}")


pack_frame("frame 1.jpg", "frame1.txt")
pack_frame("frame 2.jpg", "frame2.txt")
