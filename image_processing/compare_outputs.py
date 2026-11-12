from PIL import Image, ImageChops

input_img = Image.open("input.png").convert("L")
simd_img = Image.open("simd_output.png").convert("L")
scalar_img = Image.open("scalar_output.png").convert("L")

print("Input size:", input_img.size)
print("SIMD size:", simd_img.size)
print("Scalar size:", scalar_img.size)

# Compare SIMD and scalar outputs
simd_scalar_diff = ImageChops.difference(simd_img, scalar_img)

if simd_scalar_diff.getbbox() is None:
    print("\nSUCCESS: SIMD and scalar outputs are identical!")
else:
    print("\nERROR: SIMD and scalar outputs differ.")

# Compare input and SIMD output
input_simd_diff = ImageChops.difference(input_img, simd_img)

# Compare input and scalar output
input_scalar_diff = ImageChops.difference(input_img, scalar_img)

if input_simd_diff.getbbox() is not None:
    print("SIMD output differs from input: Brightness adjustment detected.")

if input_scalar_diff.getbbox() is not None:
    print("Scalar output differs from input: Brightness adjustment detected.")

# Create a side-by-side comparison
comparison = Image.new(
    "L",
    (input_img.width * 3, input_img.height)
)

comparison.paste(input_img, (0, 0))
comparison.paste(simd_img, (input_img.width, 0))
comparison.paste(scalar_img, (input_img.width * 2, 0))

comparison.save("comparison.png")

print("\nSaved side-by-side comparison as comparison.png")