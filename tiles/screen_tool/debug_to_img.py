import Image
import sys

img = Image.open("bannercrop_debug2.bmp")

if img.size[0]%9 != 0 or img.size[1]%9 != 0:
    print "Wrong Size"
    sys.exit(0)

    
a = img.size[0] / 9;
b = img.size[1] / 9;


out = Image.new("RGB", (a * 8, b * 8), "purple")
pout = out.load();
pimg = img.load();

for x in xrange(a):
    for y in xrange(b):
        for n in xrange(8):
            for m in xrange(8):
                pout[n+x*8, m+y*8] = pimg[n+x*9, m+y*9] 

out.save("bannercrop_2.bmp") 

