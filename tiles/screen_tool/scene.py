import Image
import sys

parts = []
grid = {}

pallette = []

def add_pallette(img, pix, colorcount):
    a = img.size[0] / 16;
    b = img.size[1] / 16;
    
    for j in xrange(b):
        for i in xrange(a):
            f = False
            for p in pallette:
                br = False
                for n in xrange(16):
                    if br:
                        break
                    for m in xrange(16):
                        x = i*16+n
                        y = j*16+m
                        if (pix[x, y] in p) == False:
                            br = True
                            break
                            
                if br == False:
                    f = p
                    break
            
            if f == False:
                p = []
                for n in xrange(16):
                    for m in xrange(16):
                        x = i*16+n
                        y = j*16+m
                        if (pix[x, y] in p) == False:
                            p.append(pix[x, y])
                if len(p)==colorcount:
                    pallette.append(p)

def find_pallette(img):
    pix = img.load()
    add_pallette(img, pix, 4)
    add_pallette(img, pix, 3)
    add_pallette(img, pix, 2)
    add_pallette(img, pix, 1)
   
    if len(pallette) > 4:
        print "Too many colors!"
        #exit(0)
    
    allcolors = {}
    for p in pallette:
        for c in p:
            if c in allcolors:
                allcolors[c] += 1
            else:
                allcolors[c] = 1
    
    
    for p in pallette:
        if len(p) < 4:
            for key, value in allcolors.iteritems():
                if (value in p) == False:
                    allcolors[key] += 1
                    
    bgcolor = False
    for key, value in allcolors.iteritems():
        if value >= 4:
            bgcolor = key
            break
            
    for p in pallette:
        while len(p) < 4:
            p.append(bgcolor)
            
    pimg = Image.new("RGB", (len(pallette)*4*4, 4), "black")
    pimgd = pimg.load();

    pallette2 =[]
    for p in pallette:
        p2=[]
        
        for i in xrange(len(p)):
            c = p[i]
            if c == bgcolor:
                t = (255*4)*10+i
            else:
                t = (c[0] + c[1] + c[2])*10+i
            p2.append(t);
        p2.sort(reverse=True)
        
        sorted_p = []
        for i in p2:
            sorted_p.append(p[i%10])
        pallette2.append(sorted_p)
        
    for i in xrange(len(pallette)):
        pallette[i] = pallette2[i]
        
    for i in xrange(len(pallette)):
        for j in xrange(4):
            for n in xrange(4):
                for m in xrange(4):
                    a = 1
                    pimgd[i*4*4 + j*4 + n, m] = pallette[i][j]
    
    
    pimg.save("pallette.bmp") 
    

def addTiles(img):
    a = img.size[0] / 8;
    b = img.size[1] / 8;
    if img.size[0] != 128 and img.size[1] != 128:
        print "Wrong Size for tile"
        exit(0)
        
    pix = img.load()
    for j in xrange(b):
        for i in xrange(a):
        
def addTiles(img, addGrid=False):
        
    a = img.size[0] / 8;
    b = img.size[1] / 8;

    if addGrid:
        if a != 32 or b != 30:
            print "Wrong Size"
            addGrid = False
            
    pix = img.load()
	
    for j in xrange(b):
        for i in xrange(a):
            f = -1
            fx = False
            fy = False
            for pi in xrange(len(parts)):
                if f != -1:
                    break
                    
                p = parts[pi]
                # no flip
                br = False
                for n in xrange(8):
                    if br:
                        break
                    for m in xrange(8):
                        x = i*8+n
                        y = j*8+m
                        c1 = pix[x, y]
                        c2 = p[n, m]
                        if c1 != c2:
                            br = True
                            break
                            
                if br == False:
                    f = pi
                    break
            
            if f == -1:
                p = {}
                for n in xrange(8):
                    for m in xrange(8):
                        x = i*8+n
                        y = j*8+m
                        p[n, m] = pix[x, y]
                f = len(parts)
                parts.append(p)
                
            if addGrid:
                grid[i, j] = f

img = Image.open("ss2.bmp")
find_pallette(img)

img = Image.open("tileset_default.bmp")
addTiles(img)

img = Image.open("ss2.bmp")
addTiles(img, True)

src = []
for j in xrange(30):
    for i in xrange(32):
        src.append(grid[i, j])
  
  

#

stat = []

for i in xrange(256):
    stat.append(0)
for i in src:
    stat[i]+=1;

min=256;
tag=255;

for i in range(256):
    if stat[i]<min:
        min=stat[i]
        tag=i

        
dst = []
dst.append(tag)
leng=0
sym=-1

size = len(src)
for i in xrange(size):
    if src[i]!=sym or leng==255 or i==size-1:
        if src[i]==sym and i==size-1:
            leng+=1;
        if leng > 0:
            dst.append(sym)
        if leng>1:
            if leng==2:
                dst.append(sym)
            else:
                dst.append(tag)
                dst.append(leng-1)
        sym=src[i];
        leng=1
    else:
        leng+=1
        
dst.append(tag)
dst.append(0)

print dst
#
  
  
yx = 16 * 8
yy = int(1+len(parts) / 16) * 8

yxd = 16 * 9
yyd = int(1+len(parts) / 16) * 9

im = Image.new("RGB", (yx, yy), "black")
imp = im.load();

imd = Image.new("RGB", (yxd, yyd), "purple")
impd = imd.load();

i = 0
j = 0

for p in parts:
    for n in xrange(8):
        for m in xrange(8):
            imp[n+i*8, m+j*8] = p[n, m]
            impd[n+i*9, m+j*9] = p[n, m]
    
    i+=1
    if i == 16:
        i = 0
        j+=1
        
im.save("tiles_out.bmp") 
imd.save("tiles_out_debug.bmp") 
