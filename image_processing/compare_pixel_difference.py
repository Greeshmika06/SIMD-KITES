from pathlib import Path

from PIL import Image, ImageChops


ROOT = Path(__file__).parent

simd = Image.open(ROOT / "pixel_difference_simd.png").convert("L")
scalar = Image.open(ROOT / "pixel_difference_scalar.png").convert("L")

if ImageChops.difference(simd, scalar).getbbox() is None:
    print("SUCCESS: SIMD and scalar pixel differences are identical.")
else:
    raise SystemExit("ERROR: SIMD and scalar pixel differences differ.")

simd.save(ROOT / "pixel_difference.png")
print("Movement image written to pixel_difference.png")
