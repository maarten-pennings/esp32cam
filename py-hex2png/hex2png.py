from PIL import Image
import sys
from os import path


# Read complete file with name `filename`.
# It must have lines of the form
#   "00cd:363AFF1111:349e"
# First number is row number (hex), next are the row bytes (hex)
# at the end a 16 bit sum of the bytes (hex).
# Returns a list of row, each row is a list of bytes.
def hexread(filename) :
  img = []
  with open(filename,"r") as fhex :
    rnum = 0
    for line in fhex :
      hnum = line[0:4]
      data = line[5:-6]
      csum = line[-5:-1]
      try :
        if int(hnum,16)==rnum :
          pixels = map( lambda s:int(s,16) , [data[i:i+2] for i in range(0,len(data),2)] )
          img.append(list(pixels))
          rnum += 1
        else :
          print( f"  error: row {rnum} not found" )
          return None
      except :
        pass
        # if outfile== None : print( f"  warning: skipping '{line[:-1]}'" )
  return img


def imgcheck(img) :
  if not isinstance(img,list) :
    print( f"  error: img is not a list" )
    return False
  if not len(img)>0 :
    print( f"  error: img list is empty" )
    return False
  if not isinstance(img[0],list) :
    print( f"  error: img[0] is not a list" )
    return False
  width = len(img[0])
  for y,row in enumerate(img) :
    if not isinstance(row,list) :
      print( f"  error: img[{y}] is not a list" )
      return False
    if len(row)!=width :
      print( f"  error: img[{y}] does not have width {width}" )
      return False
    for x,pixel in enumerate(row) :
      if pixel<0 or pixel > 255 : 
        print( f"  error: pixel ({x},{y}) out of range {pixel}" )
        return False
  return True


print( f"hex2png.py <hexfile> - image convert hex to bin" )

if len(sys.argv)!=2 :
  sys.exit( f"error: expected hex file name" )

infilename = sys.argv[1]
if not path.exists(infilename) :
  sys.exit( f"error: hex file {infilename} not found")

print( f"reading {infilename}" )
img = hexread(infilename)

print( f"checking image" )
ok = imgcheck(img)
width= len(img[0])
height= len(img)

outfilename = path.splitext(infilename)[0]+".png"
print( f"writing {outfilename}" )
img2 = Image.frombytes('L', (width,height), bytes([pix for row in img for pix in row]) )
#img2.show()
img2.save(outfilename)

print( f"done" )



