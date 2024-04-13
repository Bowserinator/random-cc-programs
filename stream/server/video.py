import cv2
import numpy as np
from PIL import Image
from colormath.color_diff import delta_e_cie1976
from colormath.color_objects import sRGBColor, LabColor
from colormath.color_conversions import convert_color
from vidgear.gears import CamGear
import cppyy
import math, time

from config import SYMBOL_HEIGHT, SYMBOL_WIDTH, SCREEN_RES, PALETTE_SIZE

# Patch numpy
def patch_asscalar(a):
    return a.item()

setattr(np, 'asscalar', patch_asscalar)

# Setup block character image processor (C++)
cppyy.include("Color.h")
processor = cppyy.gbl.Processor()


def resize_with_border(img, IMG_ROW, IMG_COL):
    """
    Resize an image, keeping aspect ratio. Extra space is filled
    in with a black border.

    :param img: Opencv image
    :param IMG_ROW: Width of output
    :param IMG_COL: Height of output
    :return: Resized image
    """
    border_v = 0
    border_h = 0
    if (IMG_COL / IMG_ROW) >= (img.shape[0]/img.shape[1]):
        border_v = int((((IMG_COL / IMG_ROW) * img.shape[1]) - img.shape[0]) / 2)
    else:
        border_h = int((((IMG_ROW / IMG_COL) * img.shape[0]) - img.shape[1]) / 2)
    img = cv2.copyMakeBorder(img, border_v, border_v, border_h, border_h, cv2.BORDER_CONSTANT, 0)
    img = cv2.resize(img, (IMG_ROW, IMG_COL))
    return img


class Video(object):
    def __init__(self, url: str):
        options = { 'STREAM_RESOLUTION': 'worst', 'CAP_PROP_FPS': 10 }
        self.stream = CamGear(
                source = url,
                stream_mode = True,
                **options
            ).start()
        self.data = None
        self.counter = 0
        self.new_frame = True

    def process(self):
        """
        Advance a single frame and update self.data
        """
        img = self.stream.read()
        if img is None:
            self.stream.stop()
            return

        self.counter += 1
        if self.counter % 3 != 0:
            return

        self.new_frame = True
        img = resize_with_border(img, SCREEN_RES[0] * SYMBOL_WIDTH, SCREEN_RES[1] * SYMBOL_HEIGHT)

        # Quantize image
        pil_img = Image.fromarray(cv2.cvtColor(img, cv2.COLOR_BGR2RGB))
        pil_img = pil_img.quantize(PALETTE_SIZE, dither=Image.Dither.FLOYDSTEINBERG)

        palette = pil_img.getpalette()
        height, width, _ = img.shape

        np_img = np.array(pil_img, dtype=np.uint8, order='C') # Row major order [y][x]
        pal = np.array(pil_img.getpalette(), dtype=np.uint8)

        out_size = (math.ceil(width * height / float(SYMBOL_WIDTH * SYMBOL_HEIGHT)), )
        out1 = np.zeros(out_size, dtype=np.uint8)
        out2 = np.zeros(out_size, dtype=np.uint8)
        out3 = np.zeros(out_size, dtype=np.uint8)

        processor.process_image(
            np_img, width, height, pal, PALETTE_SIZE,
            out1, out2, out3
        )

        len_data = np.zeros((4,), dtype='<u2')
        len_data[0] = pal.shape[0]
        len_data[1] = out_size[0]
        len_data[2] = math.ceil(width / SYMBOL_WIDTH)
        len_data[3] = math.ceil(height / SYMBOL_HEIGHT)
        len_data = np.array([x for x in len_data.tobytes()], dtype=np.uint8)

        # Format:
        # [2 bytes: palette length in bytes]
        # [2 bytes: output size of each array]
        # [2 bytes: width]
        # [2 bytes: height]
        self.data = np.concatenate((len_data, pal, out1, out2, out3), axis = None)
