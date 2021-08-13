from PIL import Image
import sys
import os


#  Convert 
#    "  0: 191a22569e835f6376acabaaabacafb3c0c8cdd3b9dadcdedddddddbd9d8d5cfc9c4bdb6afa9a59f9b98928f8f WW@8oo8OO=======----=-------------=====oooooo"
#  to
#    "191a22569e835f6376acabaaabacafb3c0c8cdd3b9dadcdedddddddbd9d8d5cfc9c4bdb6afa9a59f9b98928f8f"
#  then to
#    [ 25,  26,  34,  86, 158, 131,  95,  99, 118, 172, 171, 170, 171, 172, 175, 179, 192, 200, 205, 211, 185, 218, 220, 222, 221, 221, 221, 219, 217, 216, 213, 207, 201, 196, 189, 182, 175, 169, 165, 159, 155, 152, 146, 143, 143]
def to_int(line) :
    end = line.find(" ",5)
    if end<0 : end = len(line)
    return list(map( lambda s:int(s,16) , [line[i:i+2] for i in range(5,end,2)] ))

# Parses the hex file with `filename` and returns a list of images (and their width and height)
# Each image is an int-array.
def load(filename) :
    print( f"loading {filename}" )
    STARTLINE = "  0: "
    DATALINE = ": "
    with open(filename,"r") as fhex :
        line = fhex.readline() # one line "look ahead"
        # build up a list of images
        imgs = []
        while line:
            # Search for start of an image (a line starting with STARTLINE)
            while line:
                if line.startswith(STARTLINE) : break
                line = fhex.readline()
            if not line: break
            # Collect all lines for the image
            img = []
            height = 0
            while line.startswith(DATALINE,3) :
                data = to_int(line)
                img = img + data
                width = len(data)
                height += 1
                line = fhex.readline()
            # Append to imgs list
            imgs.append(img)
    return imgs, width, height

# Checks if all images in `imgs` have the correct amount of pixels and the correct range
def analyse(imgs,width, height) :
    if not isinstance(imgs,list) :
        sys.exit( f"error: imgs is not a list" )
    if not len(imgs)>0 :
        sys.exit( f"error: images list is empty" )
    if not isinstance(imgs[0],list) :
        sys.exit( f"error: first image is not a list" )
    size = len(imgs[0])
    if not isinstance(imgs[0],list) :
        sys.exit( f"error: first image has size {size} but that is not {width}x{height}" )
    for ix1,img in enumerate(imgs) :
        if not isinstance(img,list) :
            sys.exit( f"error: image {ix1} is not a list" )
        if len(img)!=size :
            sys.exit( f"error: image {ix1} does not have size {size}" )
        for ix2,pixel in enumerate(img) :
          if pixel<0 or pixel > 255 : 
            sys.exit( f"error: pixel {ix2} of image {ix1} is out of range ({pixel})" )
    print( f"found {len(imgs)} correct images")

# Saves image `img` with size `width` bu `height` under name `filename`.
def save(filename,img,width,height) :
    # L means 8-bit pixels, black and white
    img2 = Image.frombytes('L', (width,height), bytes([pix for pix in img]) )
    print( f"saving {filename}")
    img2.save(filename)
    #img2.show()

def main() :
    print( f"hexs2pngs.py <hexfile> - image convert hex's to png's" )

    if len(sys.argv)!=2 :
      sys.exit( f"error: expected hex file name" )

    infilename = sys.argv[1]
    if not os.path.exists(infilename) :
      sys.exit( f"error: file {infilename} not found")

    imgs, width, height = load(infilename)
    
    analyse(imgs, width, height )

    for ix,img in enumerate(imgs) :
        outfilename = f"{os.path.splitext(infilename)[0]}{ix:03d}.png"
        save(outfilename,img,width, height)

if __name__ == "__main__":
    main()

