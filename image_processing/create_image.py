from PIL import Image

img = Image.open("input.png").convert("L")

img = img.resize((384, 256), Image.Resampling.LANCZOS)

img.save("input.png")

print("Image prepared successfully!")
print("Size:", img.size)