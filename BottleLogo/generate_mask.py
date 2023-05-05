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
    print(hex(binary), ",")
