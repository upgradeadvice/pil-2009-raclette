from tester import *

from PIL import Image
try:
    from PIL import ImageCms
except ImportError:
    skip()

SRGB = "Tests/icc/sRGB.icm"

def test_sanity():

    # basic smoke test.
    # this mostly follows the cms_test outline.

    i = ImageCms.profileToProfile(lena(), SRGB, SRGB)
    assert_image(i, "RGB", (128, 128))

    t = ImageCms.buildTransform(SRGB, SRGB, "RGB", "RGB")
    i = ImageCms.applyTransform(lena(), t)
    assert_image(i, "RGB", (128, 128))

    p = ImageCms.createProfile("sRGB")
    o = ImageCms.getOpenProfile(SRGB)
    t = ImageCms.buildTransformFromOpenProfiles(p, o, "RGB", "RGB")
    i = ImageCms.applyTransform(lena(), t)
    assert_image(i, "RGB", (128, 128))

    t = ImageCms.buildProofTransform(SRGB, SRGB, SRGB, "RGB", "RGB")
    i = ImageCms.applyTransform(lena(), t)
    assert_image(i, "RGB", (128, 128))

    # get profile information for file
    assert_equal(ImageCms.getProfileName(SRGB).strip(),
                 'IEC 61966-2.1 Default RGB colour space - sRGB')
    assert_equal(ImageCms.getProfileInfo(SRGB).splitlines(),
                 ['sRGB IEC61966-2.1', '',
                  'Copyright (c) 1998 Hewlett-Packard Company', '',
                  'WhitePoint : D65 (daylight)', '',
                  'Tests/icc/sRGB.icm'])
    assert_equal(ImageCms.getDefaultIntent(SRGB), 0)
    assert_equal(ImageCms.isIntentSupported(
            SRGB, ImageCms.INTENT_ABSOLUTE_COLORIMETRIC,
            ImageCms.DIRECTION_INPUT), 1)

    # same, using profile object
    p = ImageCms.createProfile("sRGB")
    assert_equal(ImageCms.getProfileName(p).strip(),
                 'sRGB built-in - (lcms internal)')
    assert_equal(ImageCms.getProfileInfo(p).splitlines(),
                 ['sRGB built-in', '', 'WhitePoint : D65 (daylight)', '', ''])
    assert_equal(ImageCms.getDefaultIntent(p), 0)
    assert_equal(ImageCms.isIntentSupported(
            p, ImageCms.INTENT_ABSOLUTE_COLORIMETRIC,
            ImageCms.DIRECTION_INPUT), 1)

    # the procedural pyCMS API uses PyCMSError for all sorts of errors
    assert_exception(ImageCms.PyCMSError, lambda: ImageCms.profileToProfile(lena(), "foo", "bar"))
    assert_exception(ImageCms.PyCMSError, lambda: ImageCms.getProfileName(None))
    assert_exception(ImageCms.PyCMSError, lambda: ImageCms.isIntentSupported(SRGB, None, None))
