import math
import os
from PIL import Image

img = Image.open('bottle.bmp')

image_width = img.size[0]
image_height = img.size[1]

# print image as strings of hex
for i in range(image_height):
    binary = 0
    for j in range(image_width):
        p = img.getpixel((j, i))
        if p != 0:
            binary |= 1 << j
    b1 = binary & 255
    b2 = int(math.floor(binary / 256))

    print(hex(b1), ",", hex(b2), ",", end=" ")

data = {0x80, 0x1, 0x40, 0x2, 0x40, 0x2, 0x40, 0x2, 0x40, 0x2, 0x20, 0x4, 0x10, 0x8, 0x8, 0x10, 0xfc, 0x3f, 0x2, 0x40,
        0x2, 0x40, 0x2, 0x40, 0xfe, 0x7f, 0x4, 0x20, 0x2, 0x40, 0x81, 0x81, 0x1, 0x80, 0x1, 0x80, 0x1, 0x80, 0x81, 0x81,
        0x1, 0x80, 0x1, 0x80, 0x1, 0x80, 0x81, 0x81, 0x1, 0x80, 0x1, 0x80, 0x1, 0x80, 0x81, 0x81, 0x1, 0x80, 0x1, 0x80,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9f, 0xf9, 0x6f, 0xf6, 0xef, 0xf7, 0xdf, 0xfb, 0xbf, 0xfd,
        0x7f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xfc, 0x3f, }
