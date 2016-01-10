import Image
import sys
import itertools
import ImageFont
import ImageDraw

bgcolor = 0;
pallettes = [];

pallette_grid = [[False for x in range(15)] for x in range(16)];
invalid_grid = [[False for x in range(30)] for x in range(32)];

tiles = []

def findBGColor(img, pix):
    allcolors = {};
    for j in xrange(img.size[1]):
        for i in xrange(img.size[0]):
            c = pix[i, j];
            
            if c in allcolors:
                allcolors[c] += 1
            else:
                allcolors[c] = 1
    
    max_color_key = 0;
    max_color_value = 0;
    for key, value in allcolors.iteritems():
        if value > max_color_value:
            max_color_key = key;
            max_color_value = value;
    
    global bgcolor;
    bgcolor = max_color_key;
    
    
def findPallettes(img, pix):
    a = img.size[0] / 16;
    b = img.size[1] / 16;
    all_pallettes = [];
    pallette_use_counts = [];
    for j in xrange(b):
        for i in xrange(a):
            used_colors = [];
            
            for n in xrange(16):
                for m in xrange(16):
                    x = i*16+n
                    y = j*16+m
                    if pix[x, y] != bgcolor and (pix[x, y] in used_colors) == False:
                        used_colors.append(pix[x, y])

            if(len(used_colors) > 3):
                print(i, j);
                invalid_grid[i*2][j*2] = True;
                invalid_grid[i*2+1][j*2] = True;
                invalid_grid[i*2+1][j*2+1] = True;
                invalid_grid[i*2][j*2+1] = True;
                pallette_grid[i][j] = False;
                continue;
            
            found_pallette = False;
            found_pallette_i = 0;
            
            for pi in xrange(len(all_pallettes)):
                p = all_pallettes[pi];
                not_found_colors = [];
                for c in used_colors:
                    if (c in p) == False:
                        not_found_colors.append(c);
                
                    
                if (len(p) + len(not_found_colors)) <= 3:
                    for c in not_found_colors:
                        p.append(c);
                    
                    found_pallette = p;
                    found_pallette_i = pi;
                    
            if found_pallette == False:
                all_pallettes.append(used_colors);
                pallette_use_counts.append(1);
                found_pallette = used_colors;
            else:
                pallette_use_counts[found_pallette_i] += 1;
                
            pallette_grid[i][j] = found_pallette;

            
    while len(pallettes) < 4:
        max_count = max(pallette_use_counts);
        if max_count == 0:
            break;
        for i in xrange(len(all_pallettes)):
            if pallette_use_counts[i] == max_count:
                pallette_use_counts[i] = 0;
                pallettes.append(all_pallettes[i]);
                break;
    
    for j in xrange(b):
        for i in xrange(a):
            if (pallette_grid[i][j] in pallettes) == False:
                pallette_grid[i][j] = False;

    
def printPallettes(img, pix):
    pimg = Image.new("RGB", (256, 300), "white");
    pimgd = pimg.load();
    draw = ImageDraw.Draw(pimg);
    a = img.size[0] / 16;
    b = img.size[1] / 16;
    
    colors = [(255, 0, 0), (0, 255, 0), (0, 0, 255), (255, 255, 0)];
    
    for i in xrange(len(pallettes)):
        for n in xrange(i*64+12, i*64+52):
            for m in xrange(250, 260):
                pimgd[n, m] = bgcolor;

        for n in xrange(i*64+12, i*64+52):
            for m in xrange(262, 267):
                pimgd[n, m] = colors[i];
                
    for i in xrange(len(pallettes)):
        p = pallettes[i];
        for j in xrange(len(p)):
            c = p[j];
            
            for n in xrange(i*64+j*10+22, i*64+j*10+32):
                for m in xrange(250, 260):
                    pimgd[n, m] = c;

    for j in xrange(img.size[1]):
        for i in xrange(img.size[0]):
            if i%16 == 0 or i%16 == 15 or j%16 == 0 or j%16 == 15:
                pimgd[i, j] = 0xff;
            else:
                pimgd[i, j] = pix[i, j];
    

    font = ImageFont.load_default();
    for j in xrange(b):
        for i in xrange(a):
            text = "X"
            color = (255, 255, 255);
            if pallette_grid[i][j] in pallettes:
                index = pallettes.index(pallette_grid[i][j]);
                text = str(index);
                color = colors[index];
                
            draw.rectangle(((i*16+3, j*16+3), (i*16+12, j*16+12)), fill=(0,0,0));
            draw.text((i*16+5, j*16+3), text, fill=color, font=font);
    pimg.save("pallette.bmp");

def loadTileset(file, shift):    


    
    
img = Image.open("ss2.bmp");
pix = img.load();

findBGColor(img, pix);
findPallettes(img, pix);
printPallettes(img, pix);

loadTileset("../../tileset.chr", 1);

  