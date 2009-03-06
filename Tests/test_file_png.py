from tester import *

from PIL import Image
from PIL import PngImagePlugin

codecs = dir(Image.core)

if "zip_encoder" not in codecs or "zip_decoder" not in codecs:
    skip("zip/deflate support not available")

# sample png stream

file = "Images/lena.png"
data = open(file, "rb").read()

# stuff to create inline PNG images

MAGIC = PngImagePlugin._MAGIC

def chunk(cid, *data):
    file = StringIO()
    apply(PngImagePlugin.putchunk, (file, cid) + data)
    return file.getvalue()

o32 = PngImagePlugin.o32

IHDR = chunk("IHDR",  o32(1), o32(1), chr(8)+chr(2), chr(0)*3)
IDAT = chunk("IDAT")
IEND = chunk("IEND")

HEAD = MAGIC + IHDR
TAIL = IDAT + IEND

def load(data):
    return Image.open(StringIO(data))

def roundtrip(im, **options):
    out = StringIO()
    im.save(out, "PNG", **options)
    out.seek(0)
    return Image.open(out)

# --------------------------------------------------------------------

def test_sanity():
    im = Image.open(file)
    im.load()
    assert_equal(im.mode, "RGB")
    assert_equal(im.size, (128, 128))
    assert_equal(im.format, "PNG")

def test_archive():
    import glob
    for file in glob.glob("../pil-archive/*.png"):
        try:
            im = Image.open(file)
            im.load()
        except:
            print "- failed to open", file

# --------------------------------------------------------------------

def test_load_verify():
    # Check open/load/verify exception (@PIL150)

    im = Image.open("Images/lena.png")
    assert_no_exception(lambda: im.verify())

    im = Image.open("Images/lena.png")
    im.load()
    assert_exception(RuntimeError, lambda: im.verify())


def test_bad_text():
    # Make sure PIL can read malformed tEXt chunks (@PIL152)

    im = load(HEAD + chunk('tEXt') + TAIL)
    assert_equal(im.info, {})

    im = load(HEAD + chunk('tEXt', 'spam') + TAIL)
    assert_equal(im.info, {'spam': ''})

    im = load(HEAD + chunk('tEXt', 'spam\0') + TAIL)
    assert_equal(im.info, {'spam': ''})

    im = load(HEAD + chunk('tEXt', 'spam\0egg') + TAIL)
    assert_equal(im.info, {'spam': 'egg'})

    im = load(HEAD + chunk('tEXt', 'spam\0egg\0') + TAIL)
    assert_equal(im.info,  {'spam': 'egg\x00'})

def test_text_roundtrip():
    # Check text roundtripping

    im = Image.open(file)

    info = PngImagePlugin.PngInfo()
    info.add_text("KEY", "VALUE")

    im = roundtrip(im, pnginfo=info)
    assert_equal(im.info, {'KEY': 'VALUE'})

def test_dpi_roundtrip():
    # Check dpi roundtripping

    im = Image.open(file)

    im = roundtrip(im, dpi=(100, 100))
    assert_equal(im.info["dpi"], (100, 100))

def test_trns_rgb():
    # Check writing and reading of tRNS chunks for RGB images.
    # Independent file sample provided by Sebastian Spaeth.

    file = "Tests/images/caption_6_33_22.png"
    im = Image.open(file)
    assert_equal(im.info["transparency"], (248, 248, 248))

    im = roundtrip(im, transparency=(0, 1, 2))
    assert_equal(im.info["transparency"], (0, 1, 2))

def test_scary():
    # Check reading of evil PNG file.  For information, see:
    # http://scary.beasts.org/security/CESA-2004-001.txt

    file = "Tests/images/pngtest_bad.png"
    assert_exception(IOError, lambda: Image.open(file))

def test_broken():
    # Check reading of totally broken files.  In this case, the test
    # file was checked into Subversion as a text file.

    file = "Tests/images/broken.png"
    assert_exception(IOError, lambda: Image.open(file))

run()