from tester import *

from PIL import Image

def verify(im1):
    im2 = lena("I")
    assert_equal(im1.size, im2.size)
    for y in range(im1.size[1]):
        for x in range(im1.size[0]):
            v1 = im1.getpixel((x, y))
            v2 = im2.getpixel((x, y))
            if v1 != v2:
                failure("got %r at %s, expected %r" % (v1, (x, y), v2))
    success()

def test_basic():
    # PIL 1.1 has limited support for 16-bit image data.  Check that
    # create/copy/transform and save works as expected.

    def basic(mode):

        imIn = lena("I").convert(mode)
        verify(imIn)

        w, h = imIn.size

        imOut = imIn.copy()
        verify(imOut) # copy

        imOut = imIn.transform((w, h), Image.EXTENT, (0, 0, w, h))
        verify(imOut) # transform

        filename = tempfile("temp.im")
        imIn.save(filename)

        imOut = Image.open(filename)

        verify(imIn)
        verify(imOut)

        imOut = imIn.crop((0, 0, w, h))
        verify(imOut)

        imOut = Image.new(mode, (w, h), None)
        imOut.paste(imIn.crop((0, 0, w/2, h)), (0, 0))
        imOut.paste(imIn.crop((w/2, 0, w, h)), (w/2, 0))

        verify(imIn)
        verify(imOut)

        imIn = Image.new(mode, (1, 1), 1)
        assert_equal(imIn.getpixel((0, 0)), 1)

        imIn.putpixel((0, 0), 2)
        assert_equal(imIn.getpixel((0, 0)), 2)

    basic("L")

    basic("I;16")
    basic("I;16B")

    basic("I")


def test_tostring():

    def tostring(mode):
        return Image.new(mode, (1, 1), 1).tostring()

    assert_equal(tostring("L"), "\x01")
    assert_equal(tostring("I;16"), "\x01\x00")
    assert_equal(tostring("I;16B"), "\x00\x01")
    assert_equal(tostring("I"), "\x01\x00\x00\x00")