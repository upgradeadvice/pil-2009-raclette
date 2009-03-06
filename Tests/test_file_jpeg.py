from tester import *

from PIL import Image

codecs = dir(Image.core)

if "jpeg_encoder" not in codecs or "jpeg_decoder" not in codecs:
    skip("jpeg support not available")

# sample jpeg stream
file = "Images/lena.jpg"
data = open(file, "rb").read()

def test_sanity():
    im = Image.open(file)
    im.load()
    assert_equal(im.mode, "RGB")
    assert_equal(im.size, (128, 128))
    assert_equal(im.format, "JPEG")

def test_archive():
    import glob
    for file in glob.glob("../pil-archive/*.jpg"):
        try:
            im = Image.open(file)
            im.load()
        except:
            print "- failed to open", file

def test_app():
    # Test APP/COM reader (@PIL135)
    im = Image.open(file)
    assert_equal(im.applist[0],
                 ("APP0", "JFIF\x00\x01\x01\x00\x00\x01\x00\x01\x00\x00"))
    assert_equal(im.applist[1], ("COM", "Python Imaging Library"))
    assert_equal(len(im.applist), 2)

def test_dpi():
    def test(xdpi, ydpi=None):
        out = StringIO()
        im = Image.open(file)
        im.save(out, "JPEG", dpi=(xdpi, ydpi or xdpi))
        out.seek(0)
        im = Image.open(out)
        return im.info.get("dpi")
    assert_equal(test(72), (72, 72))
    assert_equal(test(300), (300, 300))
    assert_equal(test(100, 200), (100, 200))
    assert_equal(test(0), None) # square pixels

def test_truncated_jpeg():
    def test(junk):
        if junk:
            # replace "junk" bytes at the end with junk
            file = StringIO(data[:-junk] + (junk*chr(0)))
        else:
            file = StringIO(data)
        im = Image.open(file)
        im.load()
    assert_no_exception(lambda: test(0))
    assert_exception(IOError, lambda: test(1))
    assert_no_exception(lambda: test(2))
    assert_no_exception(lambda: test(4))
    assert_no_exception(lambda: test(8))
    assert_exception(IOError, lambda: test(10))

run()