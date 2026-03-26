from PIL import Image
img = Image.open('/Users/rybosafar/.gemini/antigravity/brain/14e54da1-cf4e-4741-980d-6533e13532e2/blue_render_test_1771930411024.png')
w, h = img.size

lit_pixels = 0
for y in range(800): # Only top third
    for x in range(w):
        if x > w - 300: continue # SKIP debug panel
        p = img.getpixel((x,y))
        r, g, b = p[:3]
        if r > 30 or g > 30 or b > 30: # Anything brighter than background
            lit_pixels += 1

print("Lit pixels in top 800 rows:", lit_pixels)
