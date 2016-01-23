import Image
import sys
import itertools
import ImageFont
import ImageDraw

bgcolor = 0;
pallettes = [];

pallette_grid = [[False for x in range(15)] for x in range(16)];
invalid_grid = [[False for x in range(30)] for x in range(32)];


tile_grid = [[False for x in range(30)] for x in range(32)];
tile_added_grid = [[False for x in range(30)] for x in range(32)];
tile_grid_index = [[False for x in range(30)] for x in range(32)];

empty_tiles = [];
tiles = [[[] for x in range(16)] for x in range(16)];
is_tile_new = [[False for x in range(16)] for x in range(16)];
is_tile_empty = [[False for x in range(16)] for x in range(16)];

dump = open("dump.txt", "w");

def findBGColor(img, pix):
    global bgcolor;
    if bgcolor:
        return;
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
    
    bgcolor = max_color_key; 
    
    
def findPallettes(img, pix):
    a = img.size[0] / 16;
    b = img.size[1] / 16;
    all_pallettes = [];
    pallette_use_counts = [];
    for p in pallettes:
        all_pallettes.append(p);
        pallette_use_counts.append(0);

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
                    break;
                    
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
    pimg.save("out_tile_pallettes.bmp");

    
    pimg = Image.new("RGB", (252, 15), "white");
    pimgd = pimg.load();
    draw = ImageDraw.Draw(pimg);
    
    for i in xrange(len(pallettes)):
        for n in xrange(i*64, i*64+60):
            for m in xrange(0, 15):
                pimgd[n, m] = bgcolor;
    
    for i in xrange(len(pallettes)):
        p = pallettes[i];
        for j in xrange(len(p)):
            c = p[j];
            
            for n in xrange(i*64+j*15+15, i*64+j*15+30):
                for m in xrange(0, 15):
                    pimgd[n, m] = c;

                    
    pimg.save("out_used_pallette.bmp");

    
def load_pallette(img, pix): 
    global bgcolor;

    bgcolor = pix[0, 0];
    for i in xrange(4):
        p = [];
        for j in xrange(3):
            p.append(pix[i*64+j*15+15+5, 5]);
        pallettes.append(p);

def loadTileset(img, pix): 
    a = img.size[0] / 8;
    b = img.size[1] / 8;
    empty_added = False;

    for j in xrange(b):
        for i in xrange(a):
            is_empty = True;
            t = []
            for n in xrange(8):
                for m in xrange(8):
                    x = i*8+n;
                    y = j*8+m;
                    c = int(round(pix[x, y][0] / 80.0));
                    if c > 0:
                        is_empty = False;
                    t.append(c);
            
            if is_empty:
                if empty_added == False:
                    empty_added = True;
                else:
                    empty_tiles.append([i, j]);
                    is_tile_empty[i][j] = True;
                    continue;
            
            tiles[i][j] = t;
            

    
def printTileset(img, pix): 
    a = 16;
    b = 16;
    
    pimg = Image.new("RGB", (128, 128), "black");
    pimgd = pimg.load();
    draw = ImageDraw.Draw(pimg);
    
    colors = [ (0, 0, 0), (80, 80, 80), (160, 160, 160), (255, 255, 255)];

    for j in xrange(b):
        for i in xrange(a):
            if is_tile_empty[i][j] == True:
                continue;
            for n in xrange(8):
                for m in xrange(8):
                    t = tiles[i][j];
                    c = t[n*8+m];
                    
                    x = i*8+n;
                    y = j*8+m;
                    
                    pimgd[x, y] = colors[c];
    pimg.save("out_tiles.bmp");
    
    pimg = Image.new("RGB", (img.size[0]+1, img.size[1]+1), "white");
    pimgd = pimg.load();
    draw = ImageDraw.Draw(pimg);
    a = img.size[0] / 16;
    b = img.size[1] / 16;
    
    for j in xrange(img.size[1]):
        for i in xrange(img.size[0]):
            pimgd[i, j] = pix[i, j];
    
    a = img.size[0] / 8;
    b = img.size[1] / 8;
    
    font = ImageFont.load_default();
    for j in xrange(b):
        for i in xrange(a):
            if tile_grid[i][j] == False:
                draw.rectangle(((i*8, j*8), (i*8+8, j*8+8)), outline=(255,0,0));
            elif tile_added_grid[i][j]:
                draw.rectangle(((i*8, j*8), (i*8+8, j*8+8)), outline=(0,0,255));
            
    pimg.save("out_tile_grid.bmp");

def findTiles(img, pix): 
    a = img.size[0] / 8;
    b = img.size[1] / 8;
    empty_added = False;

    for j in xrange(b):
        for i in xrange(a):
            if invalid_grid[i][j]:
                continue;
            t = [];
            p = pallette_grid[i/2][j/2];
            for n in xrange(8):
                for m in xrange(8):
                    x = i*8+n;
                    y = j*8+m;
                    c = pix[x, y];
                    if c == bgcolor:
                        t.append(0);
                    else:
                        t.append(p.index(c)+1);
            found_tile = False;
            found_tile_index = -1;
            is_new = False;
            for x in xrange(16):
                for y in xrange(16):
                    if is_tile_empty[x][y]:
                        continue;
                    t2 = tiles[x][y];
                    if t == t2:
                        found_tile = t2;
                        found_tile_index = x*16+y;
                        is_new = is_tile_new[x][y];
                        break;
                    
            if found_tile == False:
                if len(empty_tiles) == 0:
                    invalid_grid[i][j] = True;
                else:
                    tile_added_grid[i][j] = True;
                    new_index = empty_tiles.pop();
                    tile_grid_index[i][j] = new_index[0]*16+new_index[1];
                    tiles[new_index[0]][new_index[1]] = t;
                    is_tile_empty[new_index[0]][new_index[1]] = False;
                    is_tile_new[new_index[0]][new_index[1]] = True;
                    tile_grid[i][j] = t;
                    
            else:
                tile_grid_index[i][j] = found_tile_index;
                tile_grid[i][j] = found_tile;
                if is_new:
                    tile_added_grid[i][j] = True;
                    
pal_img = Image.open("pallette.bmp");
pal_pix = pal_img.load();

load_pallette(pal_img, pal_pix);


tiles_img = Image.open("tiles.bmp");
tiles_pix = tiles_img.load();

loadTileset(tiles_img, tiles_pix);


bg_img = Image.open("ss2.bmp");
bg_pix = bg_img.load();

findBGColor(bg_img, bg_pix);
findPallettes(bg_img, bg_pix);
findTiles(bg_img, bg_pix);


printPallettes(bg_img, bg_pix);
printTileset(bg_img, bg_pix);