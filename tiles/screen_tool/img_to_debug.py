import Image
import sys

img = Image.open("ss2.bmp")

if img.size[0]%8 != 0 or img.size[1]%8 != 0:
    print "Wrong Size"
    sys.exit(0)

    
a = img.size[0] / 8;
b = img.size[1] / 8;


out = Image.new("RGB", (a * 9, b * 9), "purple")
pout = out.load();
pimg = img.load();

for x in xrange(a):
    for y in xrange(b):
        for n in xrange(8):
            for m in xrange(8):
                pout[n+x*9, m+y*9] = pimg[n+x*8, m+y*8] 

out.save("ss2_debug.bmp") 

