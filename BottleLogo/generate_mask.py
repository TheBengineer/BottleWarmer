import os
from scipy import misc

path = 'bottle.bmp'
image = misc.imread(os.path.join(path, 'image.bmp'), flatten=0)

image_width = image.shape[0]
image_height = image.shape[1]

# print image as strings of hex
for i in range(image_width):
    row = image[i]
    print(hex(row))
