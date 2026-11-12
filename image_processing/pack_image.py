from PIL import Image

img = Image.open("input.png").convert("L")
pixels = list(img.getdata())

with open("input.txt", "w") as f:
    for i in range(0, len(pixels), 4):
        p = pixels[i:i+4]

        value = (
            (p[3] << 24) |
            (p[2] << 16) |
            (p[1] << 8) |
            p[0]
        )

        f.write(f"{value:08X}\n")

print("Pixels packed successfully!")
print("Total pixels:", len(pixels))
print("Packed values:", len(pixels) // 4)