from PIL import Image
try:
    img = Image.open('/Users/rybosafar/.gemini/antigravity/brain/14e54da1-cf4e-4741-980d-6533e13532e2/initial_180x48_grid_1771929309224.png')
    w, h = img.size
    found_white = 0
    for y in range(h//2):
        for x in range(w):
            p = img.getpixel((x,y))
            if x > w - 300: continue
            r, g, b = p[:3]
            if r > 180 and g > 180 and b > 180:
                found_white += 1
    print("Found white in top half:", found_white)
    
except Exception as e:
    print(e)
