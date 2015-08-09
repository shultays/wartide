#converts rotates 16x16 sprite blocks

import Image

img = Image.open("T.png")

a = img.size[0] / 17;
b = img.size[1] / 17;

out = Image.new( 'RGB', (a*16,b*16), "black")

print a

for i in range(a):
    for j in range(b):
        for n in range(8):
            for m in range(8):
            
                out.putpixel((i*16+n, j*16+m), img.getpixel((i*17+n, j*17+m)))
                out.putpixel((i*16+n+8, j*16+m+8), img.getpixel((i*17+n+8, j*17+m+8)))
                out.putpixel((i*16+n+8, j*16+m), img.getpixel((i*17+n, j*17+m+8)))
                out.putpixel((i*16+n, j*16+m+8), img.getpixel((i*17+n+8, j*17+m)))
out.save("example.png")