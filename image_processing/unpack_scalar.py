from PIL import Image

width = 384
height = 256

with open("scalar_output.txt", "r") as f:
    values = [int(line.strip(), 16) for line in f if line.strip()]

pixels = []

for value in values:
    pixels.extend([
        value & 0xFF,
        (value >> 8) & 0xFF,
        (value >> 16) & 0xFF,
        (value >> 24) & 0xFF,
        (value >> 32) & 0xFF,
        (value >> 40) & 0xFF,
        (value >> 48) & 0xFF,
        (value >> 56) & 0xFF
    ])

img = Image.new("L", (width, height))
img.putdata(pixels[:width * height])
img.save("scalar_output.png")

print("Scalar image saved:", img.size)