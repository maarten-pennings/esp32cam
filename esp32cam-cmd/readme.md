# ESP32CAM-CMD


## Steps

- Upload the sketch to the ESP32-CAM.
- Connect terminal (115200).
- Give a command, e.g. `h` for help.
- Capture and image with `0`, `1`, or `2` (captures with no flash, low-power LED, or high-power LED).
- Copy the lines from `[[` to `]]`.
- Paste them in an editor and save the file, e.g. as `img.hex`.
- Use `hex2png` python [script ](../py-hex2png) to convert the hex file to png.
- Use a viewer to open the `img.png`.


## Example output

```text
app : welcome to esp32cam-cmd
app : version V2
time: setup success
fled: setup success
cam : setup success

app : type 'h' for help
>> h
app : '0'..'9','x' to capture with 0..90,100% flash power
app : 't' for time
app : 'v' for version
app : 'i' for frame info
app : 'h' for this help
>> i
app : frame 320x240 pixels, monochrome 8 bit
app : capture returns '[[ <line>+ ]]' (takes 15sec) where
app : <line>='<rrrr>:<pp><pp><pp>...:<ssss>' where
app : <rrrr> is row number in 4 hex digits
app : <pp> is pixel value in 2 hex digits
app : <ssss> is sum of all <pp>s in 4 hex digits
>> v
app : esp32cam-cmd V2
>> t
app : time since power up 0d-00:00:51
>> 3
app : frame 0 flash 30% time 0d-00:01:00 
[[
0000:403f3c3d3e37393c3c3a343....fffffffffffffffffffffff:6ddf
0001:4f5558564d4d4e534946434....fffffffffffffffffffffff:87f5
0002:5856514a4548484d5856505....fffffffffffffffffffffff:8fd7
...                            
00e9:45434140424243403f40424....0919192949496979b9c9d9e:6bcf
00ea:434342403f40403f3f41434....d8d8e909192939497989a9b:6a5f
00eb:414242414040403f4041423....b8b8d8f9192939495979899:6925
00ec:464442423e3e403f4142424....a8b8c8d8f90919294959697:6917
00ed:4f434443474542424242424....9898b8c8e90909092939596:6976
00ee:493f444646444443443e404....6868888898a8d8e90919394:6832
00ef:46434246464443423c3a3c3....4858687898a8b8c8f8f9191:66ae
]]
>> 
```

(end)


