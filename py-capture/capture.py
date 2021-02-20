from PIL import Image
import sys
import serial
from os import path



print( f"capture.py <COM> - capture images from COM port" )

if len(sys.argv)!=2 :
  sys.exit( f"error: expected <COM> port" )

port = sys.argv[1]
if not path.exists(port) :
  sys.exit( f"error: port {port} not found")

print( f"opening {port}" )
with serial.Serial() as ser :
    ser.baudrate = 115200
    ser.port = port
    ser.timeout=1 # sec
    ser.open()
    for frame in range(3) :
      ser.write(b'1')
      line="#"
      while len(line)>0 :
        line=ser.readline()
        print(line)
    ser.close()
