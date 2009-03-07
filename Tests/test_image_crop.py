from tester import *

from PIL import Image

im = Image.new("L", (100, 100), 1)

def crop(*bbox):
    i = im.crop(bbox)
    h = i.histogram()
    while h and not h[-1]:
        del h[-1]
    return tuple(h)

def test_crop():

    assert_equal(crop(0, 0, 100, 100), (0, 10000))
    assert_equal(crop(25, 25, 75, 75), (0, 2500))

    assert_equal(crop(-25, -25, 25, 25), (1875, 625))
    assert_equal(crop(75, -25, 125, 25), (1875, 625))
    assert_equal(crop(75, 75, 125, 125), (1875, 625))
    assert_equal(crop(-25, 75, 25, 125), (1875, 625))