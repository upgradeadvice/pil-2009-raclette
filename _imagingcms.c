/* 
 * pyCMS
 * a Python / PIL interface to the littleCMS ICC Color Management System
 * Copyright (C) 2002-2003 Kevin Cazabon
 * kevin@cazabon.com
 * http://www.cazabon.com
 * 
 * pyCMS home page:  http://www.cazabon.com/pyCMS
 * littleCMS home page:  http://www.littlecms.com
 * (littleCMS is Copyright (C) 1998-2001 Marti Maria)
 * 
 * Originally released under LGPL.  Graciously donated to PIL in
 * March 2009, for distribution under the standard PIL license
 */

#define COPYRIGHTINFO "\
pyCMS\n\
a Python / PIL interface to the littleCMS ICC Color Management System\n\
Copyright (C) 2002-2003 Kevin Cazabon\n\
kevin@cazabon.com\n\
http://www.cazabon.com\n\
"

/////////////////////////////////////////////////////////////////////////////
// includes
/////////////////////////////////////////////////////////////////////////////
#include "Python.h"
//#include "patchlevel.h" // so we can include the Python version automatically in pyCMSdll.versions()
#include "lcms.h"
#include "Imaging.h"


/////////////////////////////////////////////////////////////////////////////
// version information: update this before compiling for the versions you're using
/////////////////////////////////////////////////////////////////////////////
#define PYCMSVERSION        "0.0.2 alpha"
#define LITTLECMSVERSION    "1.09b"
#define PILVERSION          "1.1.3"

//#ifndef PY_MAJOR_VERSION
  // before 1.5.2b2, these were not supported
//  #define PY_MAJOR_VERSION 0
//  #define PY_MINOR_VERSION 0
//  #define PY_MICRO_VERSION 0
//#endif
#define PYTHONVERSION       "2.2.0"

/////////////////////////////////////////////////////////////////////////////
// version history
/////////////////////////////////////////////////////////////////////////////
/*
0.0.2 alpha:  Minor updates, added interfaces to littleCMS features, Jan 6, 2003
    - fixed some memory holes in how transforms/profiles were created and passed back to Python
       due to improper destructor setup for PyCObjects
    - added buildProofTransformFromOpenProfiles() function
    - eliminated some code redundancy, centralizing several common tasks with internal functions

0.0.1 alpha:  First public release Dec 26, 2002

*/

/////////////////////////////////////////////////////////////////////////////
// known to-do list with current version
/////////////////////////////////////////////////////////////////////////////
/*
getDefaultIntent doesn't seem to work properly... whassup??? I'm getting very large int return values instead of 0-3
getProfileName and getProfileInfo are a bit shaky... work on these to solidify them!

Add comments to code to make it clearer for others to read/understand!!!
Verify that PILmode->littleCMStype conversion in findLCMStype is correct for all PIL modes (it probably isn't for the more obscure ones)
  
Add support for reading and writing embedded profiles in JPEG and TIFF files
Add support for creating custom RGB profiles on the fly
Add support for checking presence of a specific tag in a profile
Add support for other littleCMS features as required

*/


/////////////////////////////////////////////////////////////////////////////
// options / configuration
/////////////////////////////////////////////////////////////////////////////
// Set the action to take upon error within the CMS module
// LCMS_ERROR_SHOW      pop-up window showing error, do not close application
// LCMS_ERROR_ABORT     pop-up window showing error, close the application
// LCMS_ERROR_IGNORE    ignore the error and continue
#define cmsERROR_HANDLER LCMS_ERROR_SHOW


/////////////////////////////////////////////////////////////////////////////
// reference
/////////////////////////////////////////////////////////////////////////////
/*
INTENT_PERCEPTUAL                 0
INTENT_RELATIVE_COLORIMETRIC      1
INTENT_SATURATION                 2
INTENT_ABSOLUTE_COLORIMETRIC      3
*/


/////////////////////////////////////////////////////////////////////////////
// structs
/////////////////////////////////////////////////////////////////////////////
typedef struct {
    PyObject_HEAD
    Imaging image;
} ImagingObject;


/////////////////////////////////////////////////////////////////////////////
// internal functions
/////////////////////////////////////////////////////////////////////////////
DWORD 
findLCMStype (char* PILmode) {
  char *errorMsg = NULL;

  if (strcmp(PILmode, "RGB") == 0) {
    return TYPE_RGBA_8;
  }
  else if (strcmp(PILmode, "RGBA") == 0) {
    return TYPE_RGBA_8;
  }
  else if (strcmp(PILmode, "RGBX") == 0) {
    return TYPE_RGBA_8;
  }
  else if (strcmp(PILmode, "RGBA;16B") == 0) {
    return TYPE_RGBA_16;
  }
  else if (strcmp(PILmode, "CMYK") == 0) {
    return TYPE_CMYK_8;
  }
  else if (strcmp(PILmode, "L") == 0) {
    return TYPE_GRAY_8;
  }
  else if (strcmp(PILmode, "L;16") == 0) {
    return TYPE_GRAY_16;
  }
  else if (strcmp(PILmode, "L;16B") == 0) {
    return TYPE_GRAY_16_SE;
  }
  else if (strcmp(PILmode, "YCCA") == 0) {
    return TYPE_YCbCr_8;
  }
  else if (strcmp(PILmode, "YCC") == 0) {
    return TYPE_YCbCr_8;
  }

  else {
    // take a wild guess... but you probably should fail instead.
    return TYPE_GRAY_8; // so there's no buffer overrun... 
  }
}

int
pyCMSdoTransform (Imaging im, Imaging imOut, cmsHTRANSFORM hTransform) {
  int i;

  if (im->xsize > imOut->xsize) {
    return -1;
  }
  if (im->ysize > imOut->ysize) {
    return -1;
  }

  Py_BEGIN_ALLOW_THREADS

  for (i=0; i < im->ysize; i++)
  {
    cmsDoTransform(hTransform, im->image[i],
                              imOut->image[i],
                              im->xsize);
  }

  Py_END_ALLOW_THREADS

  return 0;
}

cmsHTRANSFORM
_buildTransform (cmsHPROFILE hInputProfile, cmsHPROFILE hOutputProfile, char *sInMode, char *sOutMode, int iRenderingIntent) {
  cmsHTRANSFORM hTransform;

  cmsErrorAction(cmsERROR_HANDLER);

  Py_BEGIN_ALLOW_THREADS

  // create the transform
  hTransform = cmsCreateTransform(hInputProfile,
                                 findLCMStype(sInMode),
                                 hOutputProfile,
                                 findLCMStype(sOutMode),
                                 iRenderingIntent, 0);

  Py_END_ALLOW_THREADS

  return hTransform;
}

cmsHTRANSFORM
_buildProofTransform(cmsHPROFILE hInputProfile, cmsHPROFILE hOutputProfile, cmsHPROFILE hDisplayProfile, char *sInMode, char *sOutMode, int iRenderingIntent, int iDisplayIntent) {
  cmsHTRANSFORM hTransform;

  cmsErrorAction(cmsERROR_HANDLER);

  Py_BEGIN_ALLOW_THREADS

  // create the transform
  hTransform =  cmsCreateProofingTransform(hInputProfile,
                          findLCMStype(sInMode),
                          hOutputProfile,
                          findLCMStype(sOutMode),
                          hDisplayProfile,
                          iRenderingIntent,
                          iDisplayIntent,
                          0);

  Py_END_ALLOW_THREADS

  return hTransform;
}

/////////////////////////////////////////////////////////////////////////////
// Python callable functions
/////////////////////////////////////////////////////////////////////////////
static PyObject *
versions (PyObject *self, PyObject *args)
{
  return Py_BuildValue("ssss", PYCMSVERSION, LITTLECMSVERSION, PYTHONVERSION, PILVERSION);
}

static PyObject *
about (PyObject *self, PyObject *args)
{
  return Py_BuildValue("s", COPYRIGHTINFO);
}

static PyObject *
copyright (PyObject *self, PyObject *args)
{
  return about(self, args);
}

static PyObject *
getOpenProfile(PyObject *self, PyObject *args)
{
  char *sProfile = NULL;

  cmsHPROFILE hProfile;

  if (!PyArg_ParseTuple(args, "s", &sProfile)) {
    PyErr_Clear(); /* FIXME */
    return Py_BuildValue("s", "ERROR: Could not parse argument tuple passed to pyCMSdll.getOpenProfile()");
  }

  cmsErrorAction(cmsERROR_HANDLER);

  hProfile = cmsOpenProfileFromFile(sProfile, "r");

  return Py_BuildValue("O", PyCObject_FromVoidPtr(hProfile, cmsCloseProfile));
}

static PyObject *
buildTransform(PyObject *self, PyObject *args) {
  char *sInputProfile;
  char *sOutputProfile;
  char *sInMode;
  char *sOutMode;
  int iRenderingIntent = 0;
  cmsHPROFILE hInputProfile, hOutputProfile;
  cmsHTRANSFORM transform;

  if (!PyArg_ParseTuple(args, "ssss|i", &sInputProfile, &sOutputProfile, &sInMode, &sOutMode, &iRenderingIntent)) {
    PyErr_Clear(); /* FIXME */
    return Py_BuildValue("s", "ERROR: Could not parse argument tuple passed to pyCMSdll.buildTransform()");
  }

  cmsErrorAction(cmsERROR_HANDLER);

  hInputProfile  = cmsOpenProfileFromFile(sInputProfile, "r");
  hOutputProfile = cmsOpenProfileFromFile(sOutputProfile, "r");

  transform = _buildTransform(hInputProfile, hOutputProfile, sInMode, sOutMode, iRenderingIntent);

  cmsCloseProfile(hInputProfile);
  cmsCloseProfile(hOutputProfile);

  return PyCObject_FromVoidPtr(transform, cmsDeleteTransform); // this may not be right way to call the destructor...?
}

static PyObject *
buildTransformFromOpenProfiles (PyObject *self, PyObject *args)
{
  char *sInMode;
  char *sOutMode;
  int iRenderingIntent = 0;
  PyObject *pInputProfile;
  PyObject *pOutputProfile;
  cmsHPROFILE hInputProfile, hOutputProfile;
  void *hTransformPointer = NULL;
  cmsHTRANSFORM transform;

  if (!PyArg_ParseTuple(args, "OOss|i", &pInputProfile, &pOutputProfile, &sInMode, &sOutMode, &iRenderingIntent)) {
    PyErr_Clear(); /* FIXME */
    return Py_BuildValue("s", "ERROR: Could not parse argument tuple passed to pyCMSdll.buildTransformFromOpenProfiles()");
  }

  cmsErrorAction(cmsERROR_HANDLER);

  hInputProfile = (cmsHPROFILE) PyCObject_AsVoidPtr(pInputProfile); 
  hOutputProfile = (cmsHPROFILE) PyCObject_AsVoidPtr(pOutputProfile); 

  transform = _buildTransform(hInputProfile, hOutputProfile, sInMode, sOutMode, iRenderingIntent);

  // we don't have to close these profiles... but do we have to decref them?

  return PyCObject_FromVoidPtr(transform, cmsDeleteTransform); // this may not be right way to call the destructor...?
}

static PyObject *
buildProofTransform(PyObject *self, PyObject *args)
{
  char *sInputProfile;
  char *sOutputProfile;
  char *sDisplayProfile;
  char *sInMode;
  char *sOutMode;
  int iRenderingIntent = 0;
  int iDisplayIntent = 0;
  cmsHTRANSFORM transform;

  cmsHPROFILE hInputProfile, hOutputProfile, hDisplayProfile;

  if (!PyArg_ParseTuple(args, "sssss|ii", &sInputProfile, &sOutputProfile, &sDisplayProfile, &sInMode, &sOutMode, &iRenderingIntent, &iDisplayIntent)) {
    PyErr_Clear(); /* FIXME */
    return Py_BuildValue("s", "ERROR: Could not parse argument tuple passed to pyCMSdll.buildProofTransform()");
  }

  cmsErrorAction(cmsERROR_HANDLER);

  // open the input and output profiles
  hInputProfile  = cmsOpenProfileFromFile(sInputProfile, "r");
  hOutputProfile = cmsOpenProfileFromFile(sOutputProfile, "r");
  hDisplayProfile = cmsOpenProfileFromFile(sDisplayProfile, "r");

  transform = _buildProofTransform(hInputProfile, hOutputProfile, hDisplayProfile, sInMode, sOutMode, iRenderingIntent, iDisplayIntent);
  
  cmsCloseProfile(hInputProfile);
  cmsCloseProfile(hOutputProfile);
  cmsCloseProfile(hDisplayProfile);

  return PyCObject_FromVoidPtr(transform, cmsDeleteTransform); // this may not be right way to call the destructor...?

}

static PyObject *
buildProofTransformFromOpenProfiles(PyObject *self, PyObject *args)
{
  char *sInMode;
  char *sOutMode;
  int iRenderingIntent = 0;
  int iDisplayIntent = 0;
  PyObject *pInputProfile;
  PyObject *pOutputProfile;
  PyObject *pDisplayProfile;
  cmsHTRANSFORM transform;

  cmsHPROFILE hInputProfile, hOutputProfile, hDisplayProfile;

  if (!PyArg_ParseTuple(args, "OOOss|ii", &pInputProfile, &pOutputProfile, &pDisplayProfile, &sInMode, &sOutMode, &iRenderingIntent, &iDisplayIntent)) {
    PyErr_Clear(); /* FIXME */
    return Py_BuildValue("s", "ERROR: Could not parse argument tuple passed to pyCMSdll.buildProofTransform()");
  }

  cmsErrorAction(cmsERROR_HANDLER);

  hInputProfile = (cmsHPROFILE) PyCObject_AsVoidPtr(pInputProfile); 
  hOutputProfile = (cmsHPROFILE) PyCObject_AsVoidPtr(pOutputProfile); 
  hDisplayProfile = (cmsHPROFILE) PyCObject_AsVoidPtr(pDisplayProfile); 

  transform = _buildProofTransform(hInputProfile, hOutputProfile, hDisplayProfile, sInMode, sOutMode, iRenderingIntent, iDisplayIntent);
  
  // we don't have to close these profiles, but do we have to decref them?

  return PyCObject_FromVoidPtr(transform, cmsDeleteTransform); // this may not be right way to call the destructor...?
}

static PyObject *
applyTransform(PyObject *self, PyObject *args)
{
  long idIn;
  long idOut;
  PyObject *pTransform;
  cmsHTRANSFORM hTransform;
  Imaging im;
  Imaging imOut;

  int result;

  if (!PyArg_ParseTuple(args, "llO", &idIn, &idOut, &pTransform)) { 
    PyErr_Clear(); /* FIXME */
    return Py_BuildValue("s", "ERROR: Could not parse the data passed to pyCMSdll.applyTransform()");
  }

  im = (Imaging) idIn;
  imOut = (Imaging) idOut;

  cmsErrorAction(cmsERROR_HANDLER);

  hTransform = (cmsHTRANSFORM) PyCObject_AsVoidPtr(pTransform); 

  result = pyCMSdoTransform(im, imOut, hTransform);

  return Py_BuildValue("i", result);
}

static PyObject *
profileToProfile(PyObject *self, PyObject *args)
{
  Imaging im;
  Imaging imOut;
  long idIn;
  long idOut = 0L;
  char *sInputProfile = NULL;
  char *sOutputProfile = NULL;
  int iRenderingIntent = 0;
  char *inMode;
  char *outMode;
  int result;
  cmsHPROFILE hInputProfile, hOutputProfile;
  cmsHTRANSFORM hTransform;

  // parse the PyObject arguments, assign to variables accordingly
  if (!PyArg_ParseTuple(args, "llss|i", &idIn, &idOut, &sInputProfile, &sOutputProfile, &iRenderingIntent)) { 
    PyErr_Clear(); /* FIXME */
    return Py_BuildValue("s", "ERROR: Could not parse the argument tuple passed to pyCMSdll.profileToProfile()");
  }

  im = (Imaging) idIn;

  if (idOut != 0L) {
    imOut = (Imaging) idOut;
  }

  cmsErrorAction(cmsERROR_HANDLER);

  // Check the modes of imIn and imOut to set the color type for the transform
  // Note that the modes do NOT have to be the same, as long as they are each
  //    supported by the relevant profile specified

  inMode = im->mode;
  if (idOut == 0L) {
    outMode = inMode;
  }
  else {
    outMode = imOut->mode;
  }

  // open the input and output profiles
  hInputProfile  = cmsOpenProfileFromFile(sInputProfile, "r");
  hOutputProfile = cmsOpenProfileFromFile(sOutputProfile, "r");

  // create the transform
  hTransform = _buildTransform(hInputProfile, hOutputProfile, inMode, outMode, iRenderingIntent);

  // apply the transform to imOut (or directly to im in place if idOut is not supplied)
  if (idOut != 0L) {
    result = pyCMSdoTransform (im, imOut, hTransform);
  }
  else {
    result = pyCMSdoTransform (im, im, hTransform);
  }

  // free the transform and profiles
  cmsDeleteTransform(hTransform);
  cmsCloseProfile(hInputProfile);
  cmsCloseProfile(hOutputProfile);

  // return 0 on success, -1 on failure
  return Py_BuildValue("i", result);
}

//////////////////////////////////////////////////////////////////////////////
// Python-Callable On-The-Fly profile creation functions
//////////////////////////////////////////////////////////////////////////////
static PyObject *
createProfile(PyObject *self, PyObject *args)
{
  char *sColorSpace;
  cmsHPROFILE hProfile;
  int iColorTemp = 0;
  LPcmsCIExyY whitePoint = NULL;
  BOOL result;

  if (!PyArg_ParseTuple(args, "s|i", &sColorSpace, &iColorTemp)) { 
    PyErr_Clear(); /* FIXME */
    return Py_BuildValue("s", "ERROR: Could not parse the argument tuple passed to pyCMSdll.createProfile()");
  }

  cmsErrorAction(cmsERROR_HANDLER);

  if (strcmp(sColorSpace, "LAB") == 0) {
    if (iColorTemp > 0) {
      result = cmsWhitePointFromTemp(iColorTemp, whitePoint);
      if (result == FALSE) {
        return Py_BuildValue("s", "ERROR: Could not calculate white point from color temperature provided, must be integer in degrees Kelvin");
      }
      hProfile = cmsCreateLabProfile(whitePoint);
    }
    else {
      hProfile = cmsCreateLabProfile(NULL);
    }
  }
  else if (strcmp(sColorSpace, "XYZ") == 0) {
    hProfile = cmsCreateXYZProfile();
  }
  else if (strcmp(sColorSpace, "sRGB") == 0) {
    hProfile = cmsCreate_sRGBProfile();
  }
  else {
    return Py_BuildValue("s", "ERROR: Color space requested is not valid for built-in profiles");
  }

  return Py_BuildValue("O", PyCObject_FromVoidPtr(hProfile, cmsCloseProfile));
}

//////////////////////////////////////////////////////////////////////////////
// Python callable profile information functions
//////////////////////////////////////////////////////////////////////////////

static
int getprofile(PyObject* profile, cmsHPROFILE *hProfile, BOOL *closeProfile)
{
  if (PyString_Check(profile)) {
    *hProfile = cmsOpenProfileFromFile(PyString_AsString(profile), "r");
    *closeProfile = TRUE;
    return 1;
  }

  if (PyCObject_Check(profile)) {
    *hProfile = (cmsHPROFILE) PyCObject_AsVoidPtr(profile); 
    *closeProfile = FALSE;
    return 1;
  }

  PyErr_SetString(PyExc_TypeError, "illegal profile argument (must be string or profile handle");
  return 0;
}

static PyObject *
getProfileName(PyObject *self, PyObject *args)
{
  PyObject* result;
  PyObject* profile;
  cmsHPROFILE hProfile;
  BOOL closeProfile;

  if (!PyArg_ParseTuple(args, "O", &profile))
    return NULL;
  if (!getprofile(profile, &hProfile, &closeProfile))
    return NULL;

  result = PyString_FromString(cmsTakeProductName(hProfile));

  if (closeProfile)
    cmsCloseProfile(hProfile);

  return result;
}

static PyObject *
getProfileInfo(PyObject *self, PyObject *args)
{
  PyObject* profile;
  PyObject* result;
  cmsHPROFILE hProfile;
  BOOL closeProfile;

  if (!PyArg_ParseTuple(args, "O", &profile))
    return NULL;
  if (!getprofile(profile, &hProfile, &closeProfile))
    return NULL;

  result = PyString_FromString(cmsTakeProductInfo(hProfile));

  if (closeProfile)
    cmsCloseProfile(hProfile);

  return result;
}

static PyObject *
getDefaultIntent(PyObject *self, PyObject *args)
{
  PyObject* profile;
  int intent;
  cmsHPROFILE hProfile;
  BOOL closeProfile;

  if (!PyArg_ParseTuple(args, "O", &profile))
    return NULL;
  if (!getprofile(profile, &hProfile, &closeProfile))
    return NULL;

  intent = cmsTakeRenderingIntent(hProfile);

  if (closeProfile)
    cmsCloseProfile(hProfile);

  return PyInt_FromLong(intent);
}

static PyObject *
isIntentSupported(PyObject *self, PyObject *args)
{
  BOOL result;
  PyObject* profile;
  int iIntent;
  int iDirection;
  cmsHPROFILE hProfile;
  BOOL closeProfile;

  if (!PyArg_ParseTuple(args, "Oii", &profile, &iIntent, &iDirection))
    return NULL;
  if (!getprofile(profile, &hProfile, &closeProfile))
    return NULL;

  result = cmsIsIntentSupported(hProfile, iIntent, iDirection);

  if (closeProfile)
    cmsCloseProfile(hProfile);

  if (result) {
    return PyInt_FromLong(1);
  } else {
    return PyInt_FromLong(-1);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Python interface setup
/////////////////////////////////////////////////////////////////////////////
static PyMethodDef pyCMSdll_methods[] = {
  // pyCMS info
  {"versions", versions, 1, "pyCMSdll.versions() returs tuple of pyCMSversion, littleCMSversion, pythonVersion that it was compiled with (don't trust this 100%, they must be set manually in the source code for now)"},
  {"about", about, 1, "pyCMSdll.about() returns info about pyCMSdll"},
  {"copyright", copyright, 1, "pyCMSdll.copyright() returns info about pyCMSdll"},

  // profile and transform functions
  {"profileToProfile", profileToProfile, 1, "pyCMSdll.profileToProfile (idIn, idOut, InputProfile, OutputProfile, [RenderingIntent]) returns 0 on success, -1 on failure.  If idOut is the same as idIn, idIn is modified in place, otherwise the results are applied to idOut"},
  {"getOpenProfile", getOpenProfile, 1, "pyCMSdll.getOpenProfile (profileName) returns a handle to an open pyCMS profile that can be used to build a transform"},
  {"buildTransform", buildTransform, 1, "pyCMSdll.buildTransform (InputProfile, OutputProfile, InMode, OutMode, [RenderingIntent]) returns a handle to a pre-computed ICC transform that can be used for processing multiple images, saving calculation time"},
  {"buildProofTransform", buildProofTransform, 1, "pyCMSdll.buildProofTransform (InputProfile, OutputProfile, DisplayProfile, InMode, OutMode, [RenderingIntent], [DisplayRenderingIntent]) returns a handle to a pre-computed soft-proofing (simulating the output device capabilities on the display device) ICC transform that can be used for processing multiple images, saving calculation time"},
  {"buildProofTransformFromOpenProfiles", buildProofTransformFromOpenProfiles, 1, "pyCMSdll.buildProofTransformFromOpenProfiles(InputProfile, OutputProfile, DisplayProfile, InMode, OutMode, [RenderingIntent], [DisplayRenderingIntent]) returns a handle to a pre-computed soft-proofing transform.  Profiles should be HANDLES, not pathnames."},
  {"applyTransform", applyTransform, 1, "pyCMSdll.applyTransform (idIn, idOut, hTransform) applys a pre-calcuated transform (from pyCMSdll.buildTransform) to an image.  If idIn and idOut are the same, it modifies the image in place, otherwise the new image is built in idOut.  Returns 0 on success, -1 on failure"},
  {"buildTransformFromOpenProfiles", buildTransformFromOpenProfiles, 1, "pyCMSdll.buildTransformFromOpenProfiles (InputProfile, OutputProfile, InMode, OutMode, RenderingIntent) returns a handle to a pre-computed ICC transform that can be used for processing multiple images, saving calculation time"},
   
  // on-the-fly profile creation functions
  {"createProfile", createProfile, 1, "pyCMSdll.createProfile (colorSpace, [colorTemp]) returns a handle to an open profile created on the fly.  colorSpace can be 'LAB', 'XYZ', or 'xRGB'.  If using LAB, you can specify a white point color temperature, or let it default to D50 (5000K)"},

  // profile info functions
  {"getProfileName", getProfileName, 1, "pyCMSdll.getProfileName (profile) returns the internal name of the profile"},
  {"getProfileInfo", getProfileInfo, 1, "pyCMSdll.getProfileInfo (profile) returns additional information about the profile"},
  {"getDefaultIntent", getDefaultIntent, 1, "pyCMSdll.getDefaultIntent (profile) returns the default rendering intent of the profile (as an integer)"},
  {"isIntentSupported", isIntentSupported, 1, "pyCMSdll.isIntentSupported (profile, intent, direction) returns 1 if profile supports that intent, -1 if it doesnt.  Direction is what the profile is being used for: INPUT = 0, OUTPUT = 1, PROOF = 2"},

  {NULL, NULL}
};

void init_imagingcms(void)
{
  Py_InitModule("_imagingcms", pyCMSdll_methods);
}
