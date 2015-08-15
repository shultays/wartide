import Image
import sys
import itertools
tiles = []
tilesid = []
pallette = []
selectedPallettes = {}
convertedImage = {}
tileFound = {}


def add_pallette(img, pix, colorcount):
    a = img.size[0] / 16;
    b = img.size[1] / 16;
    
    for j in xrange(b):
        for i in xrange(a):
            f = False
            fi = -1
            for pi, p in enumerate(pallette):
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
                    fi = pi
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
                    fi = len(pallette)
                    pallette.append(p)

            if fi != -1:
                selectedPallettes[i, j] = fi

def findPallette(img):
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
                    
    bgcolor = (129, 226, 0)
    for p in pallette:
        while len(p) < 4:
            p.append(bgcolor)
            
    pallette2 =[]
    for p in pallette:
        p2=[]
        
        for i in xrange(len(p)):
            c = p[i]
            if c == bgcolor:
                t = (255*3)*10+i
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
        
    print pallette[0]    

def addTiles(img):
    a = img.size[0] / 8;
    b = img.size[1] / 8;

    pix = img.load()
    convert = {}
    for j in xrange(b):
        for i in xrange(a):
            f = -1
            fx = False
            fy = False
            for pi in xrange(len(tiles)):
                if f != -1:
                    break
                    
                p = tiles[pi]
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
                        if (pix[x, y] in convert) == False:
                            convert[pix[x, y]] = len(convert)
                f = len(tiles)
                tiles.append(p)
                tilesid.append(i + j*b)
                
    perm = list(itertools.permutations([0, 1, 2, 3]))[6]
    
    convert[(248, 157, 6)] = perm[0]
    convert[(129, 226, 0)] = perm[1]
    convert[(167, 77, 0)] = perm[2]
    convert[(96, 28, 0)] = perm[3]
    for t in tiles:
        for n in xrange(8):
            for m in xrange(8):
                t[n, m] = convert[t[n, m]]
                        
def convertImage(img):
    a = img.size[0] / 8;
    b = img.size[1] / 8;

    pix = img.load()
	
    for j in xrange(b):
        for i in xrange(a):
            p = pallette[selectedPallettes[i/2, j/2]]
            for n in xrange(8):
                for m in xrange(8):
                    x = i*8+n
                    y = j*8+m
                    convertedImage[x, y] = p.index(pix[x, y])

    

def findTiles(img):
    a = img.size[0] / 8;
    b = img.size[1] / 8;
    found = 0
    notfound = 0
    for j in xrange(b):
        for i in xrange(a):
            fi = -1
            for ti, t in enumerate(tiles):
                br = False
                for n in xrange(8):
                    if br:
                        break
                    for m in xrange(8):
                        x = i*8+n
                        y = j*8+m
                        if convertedImage[x, y] != t[n, m]:
                            br = True
                            break
                if br == False:
                    fi = ti
                    break
            if fi == -1:
                notfound += 1
            else:
                found += 1

            tileFound[i, j] = fi
    print "found " + str(found) + " not found " + str(notfound)
img = Image.open("tileset_default.bmp")
addTiles(img)

img = Image.open("ss2.bmp")
findPallette(img)


p = 2

sw0 = 1
sw1 = 3
t = pallette[p][sw0]
pallette[p][sw0] = pallette[p][sw1]
pallette[p][sw1] = t

sw0 = 3
sw1 = 2
p = 1
t = pallette[p][sw0]
pallette[p][sw0] = pallette[p][sw1]
pallette[p][sw1] = t


sw0 = 2
sw1 =1
p = 3
t = pallette[p][sw0]
pallette[p][sw0] = pallette[p][sw1]
pallette[p][sw1] = t


sw0 = 2
sw1 =3
p = 3
t = pallette[p][sw0]
pallette[p][sw0] = pallette[p][sw1]
pallette[p][sw1] = t



convertImage(img)
findTiles(img)

pimg = Image.new("RGB", (len(pallette)*4*4+8, 4), "purple")
pimgd = pimg.load();

for i in xrange(len(pallette)):
    for j in xrange(4):
        for n in xrange(4):
            for m in xrange(4):
                pimgd[i*4*4 + j*4 + n+i*2, m] = pallette[i][j]
pimg.save("pallette.bmp")

color = [(0, 0, 0), (80, 80, 80), (160, 160, 160), (240, 240, 240)]

yx = 16 * 8
yy = int(1+len(tiles) / 16) * 8

yxd = 16 * 9
yyd = int(1+len(tiles) / 16) * 9

im = Image.new("RGB", (yx, yy), "black")
imp = im.load();

imd = Image.new("RGB", (yxd, yyd), "purple")
impd = imd.load();

i = 0
j = 0

for p in tiles:
    for n in xrange(8):
        for m in xrange(8):
            imp[n+i*8, m+j*8] = color[p[n, m]]
            impd[n+i*9, m+j*9] = color[p[n, m]]
    
    i+=1
    if i == 16:
        i = 0
        j+=1
        
im.save("tiles_out.bmp") 
imd.save("tiles_out_debug.bmp") 


im = Image.new("RGB", (img.size[0], img.size[1]), "black")
imp = im.load();

imd = Image.new("RGB", (9*img.size[0]/8, 9*img.size[1]/8), "purple")
impd = imd.load();


for i in xrange(img.size[0]):
    for j in xrange(img.size[1]):
        imp[i, j] = color[convertedImage[i, j]]
        impd[i+int(i/8), j+int(j/8)] = color[convertedImage[i, j]]


im.save("ss_out.bmp") 
imd.save("ss_out_debug.bmp") 

im = Image.new("RGB", (img.size[0], img.size[1]), "black")
imp = im.load();

imd = Image.new("RGB", (9*img.size[0]/8, 9*img.size[1]/8), "purple")
impd = imd.load();


for i in xrange(img.size[0]/8):
    for j in xrange(img.size[1]/8):
        ti = tileFound[i, j]
        if ti == -1:
            for n in xrange(8):
                for m in xrange(8):
                    imp[n+i*8, m+j*8] = (255, 0, 0)
                    impd[n+i*9, m+j*9] = (255, 0, 0)
        else:
            t = tiles[ti]
            for n in xrange(8):
                for m in xrange(8):
                    imp[n+i*8, m+j*8] = color[t[n,m]]
                    impd[n+i*9, m+j*9] = color[t[n,m]]
        impd[i+int(i/8), j+int(j/8)] = color[convertedImage[i, j]]


im.save("ss_2_out.bmp") 
imd.save("ss_out_2_debug.bmp") 


im = Image.new("RGB", (img.size[0], img.size[1]), "black")
imp = im.load();

imd = Image.new("RGB", (9*img.size[0]/8, 9*img.size[1]/8), "purple")
impd = imd.load();


for i in xrange(img.size[0]/8):
    for j in xrange(img.size[1]/8):
        for n in xrange(8):
            for m in xrange(8):
                imp[n+i*8, m+j*8] = color[selectedPallettes[i/2, j/2]]
                impd[n+i*9, m+j*9] = color[selectedPallettes[i/2, j/2]]
        


im.save("ss_2_pallette.bmp") 
imd.save("ss_2_pallette_debug.bmp") 

#
src = []

for j in xrange(img.size[1]/8):
    for i in xrange(img.size[0]/8):
        src.append(tilesid[tileFound[i, j]])

for j in xrange(img.size[1]/32 + 1):
    for i in xrange(img.size[0]/32):
        t = 0
        t |= selectedPallettes[i*2,   j*2]
        t |= selectedPallettes[i*2+1, j*2]<<2
        if j != img.size[1]/32:
            t |= selectedPallettes[i*2,   j*2+1]<<4
            t |= selectedPallettes[i*2+1, j*2+1]<<6
        src.append(t)

        

src.append(t)
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

f = open('../../menu.h', 'w')
f.write('const unsigned char menu_data[]={\n') 
c = 16
for i in dst:
    f.write(hex(i)+ ", ") 
    c-=1
    if c == 0:
        c = 16
        f.write('\n') 
f.write('};\n\n') 

f.write('/*const unsigned char menu_data_full[]={\n') 
c = 16

for j in xrange(img.size[1]/8):
    for i in xrange(img.size[0]/8):
        f.write(hex(tilesid[tileFound[i, j]])+ ", ") 
        c-=1
        if c == 0:
            c = 16
            f.write('\n') 

f.write('};*/\n\n') 


f.close()
#
  