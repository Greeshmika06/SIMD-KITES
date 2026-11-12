from PIL import Image

width = 384
height = 256

with open("simd_output.txt", "r") as f:
    values = [int(line.strip(), 16) for line in f if line.strip()]

pixels = []

for value in values:
    pixels.extend([
        value & 0xFF,
        (value >> 8) & 0xFF,
        (value >> 16) & 0xFF,
        (value >> 24) & 0xFF
    ])

img = Image.new("L", (width, height))
img.putdata(pixels[:width * height])
img.save("simd_output.png")

print("Processed image saved successfully!")
print("Image size:", img.size)