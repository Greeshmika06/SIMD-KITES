from PIL import Image

img = Image.open("input.png").convert("L")
pixels = list(img.getdata())

with open("input.txt", "w") as f:
    for i in range(0, len(pixels), 8):
        p = pixels[i:i+8]

        value = sum(pixel << (8 * offset) for offset, pixel in enumerate(p))

        f.write(f"{value:016X}\n")

print("Pixels packed successfully!")
print("Total pixels:", len(pixels))
print("Pixels per packed value:", 8)
print("Packed values:", (len(pixels) + 7) // 8)