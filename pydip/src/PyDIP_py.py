# PyDIP 3.0, Python bindings for DIPlib 3.0
#
# (c)2017-2020, Flagship Biosciences, Inc., written by Cris Luengo.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
The portion of the PyDIP module that contains Python code.
"""

from PyDIP import PyDIP_bin
import importlib.util
import pathlib

hasMatPlotLib = True
if importlib.util.find_spec('matplotlib') is None:
    print("""
PyDIP requires matplotlib for its display functionality. Matplotlib was not found
on your system. Image display (PyDIP.Show and PyDIP.Image.Show) will not do anything.
You can install matplotlib by typing on your Linux/MacOS command prompt:
    pip3 install matplotlib
or under Windows:
    python3 -m pip install matplotlib
""")
    hasMatPlotLib = False
else:
    import matplotlib
    import matplotlib.pyplot as pp
    import numpy as np


hasBioFormats = True
if importlib.util.find_spec('javabridge') is None or importlib.util.find_spec('bioformats') is None:
    hasBioFormats = False
else:
    import javabridge
    import bioformats


# Label color map from the function of the same name in DIPimage:
def _label_colormap():
    if hasMatPlotLib:
        cm = np.array([
            [1.0000, 0.0000, 0.0000],
            [0.0000, 1.0000, 0.0000],
            [0.0000, 0.0000, 1.0000],
            [1.0000, 1.0000, 0.0000],
            [0.0000, 1.0000, 1.0000],
            [1.0000, 0.0000, 1.0000],
            [1.0000, 0.3333, 0.0000],
            [0.6667, 1.0000, 0.0000],
            [0.0000, 0.6667, 1.0000],
            [0.3333, 0.0000, 1.0000],
            [1.0000, 0.0000, 0.6667],
            [1.0000, 0.6667, 0.0000],
            [0.0000, 1.0000, 0.5000],
            [0.0000, 0.3333, 1.0000],
            [0.6667, 0.0000, 1.0000],
            [1.0000, 0.0000, 0.3333],
        ])
        n = len(cm)
        index = list(i % n for i in range(0, 255))
        cm = np.concatenate((np.array([[0, 0, 0]]), cm[index]))
        return matplotlib.colors.ListedColormap(cm)
    return None


# Using matplotlib to show an image
def Show(img, range=(), complexMode='abs', projectionMode='mean', coordinates=(), dim1=0, dim2=1, colormap=''):
    """Show an image in the current pyplot window

    Keyword arguments:
    range -- a 2-tuple indicating the range of input values to map to the
        output range, or a string indicating how to compute the range and
        how to map. Valid strings are:
        - `'unit'`: use the `(0,1)` range.
        - `'8bit'` or `'normal'`: use the `(0,255)` range.
        - `'12bit'`: use the `(0,2**12)` range.
        - `'16bit'`: use the `(0,2**16)` range.
        - `'s8bit'`: use the `(-128,127)` range.
        - `'s12bit'`: use the `(-2**11,12**11-1)` range.
        - `'s16bit'`: use the `(-2**15,12**15-1)` range.
        - `'angle'`: use the `(0,2*pi)` range, with folding of out-of-
            range values by modulo operation. Additionally, it sets the
            color map such that 0 and 2*pi are shown in the same color.
        - `'orientation'`: use the `(0,pi)` range, with folding of out-of-
            range values by modulo operation. Additionally, it sets the
            color map such that 0 and pi are shown in the same color.
        - `'lin'` or `'all'`: use the range from lowest to highest value in
            `img`. This is the default.
        - `'percentile'`: use the range from 5th to 95th percentile value
            in `img`.
        - `'base'` or `'based'`: like 'lin', but setting the value of 0 to
            the middle of the output range. Additionally, it sets the color
            map to `'coolwarm'`, such that negative and positive values
            have blue and red colors, respectively, and 0 is a neutral
            grey.
        - `'log'`: use a logarithmic mapping.
        - `'modulo'` or `'labels'`: use the `(0,255)` range, with folding
            of out-of-range values by modulo operation. Additionally, it
            sets the color map such that nearby values get very different
            colors. This mode is suitable for labeled images.
    complexMode -- a string indicating how to convert complex values to
        real values for display. One of `'abs'` or `'magnitude'`,
        `'phase'`, `'real'`, `'imag'`. The default is `'abs'`.
    projectionMode -- a string indicating how to extract a 2D slice from a
        multi-dimensional image for display. One of `'slice'`, `'max'`,
        `'mean'`. The default is `'mean'`.
    coordinates -- Coordinates of a pixel to be shown, as a tuple with as
        many elements as image dimensions. Determines which slice is shown
        out of a multi-dimensional image.
    dim1 -- Image dimension to be shown along x-axis of display.
    dim2 -- Image dimension to be shown along y-axis of display.
    colormap -- Name of a color map to use for display.

    For images with more than 2 dimensions, a slice is extracted for
    display. The direction of the slice is determined using the `dim1` and
    `dim2` parameters, and the location using the `coordinates` parameter.
    If `projectionMode` is `'slice'`, then the single slice is shown. If
    `projectionMode` is `'max'` or `'mean'`, then a projection is computed
    across the full image volume along the non-displayed dimensions.

    For 1D images, or if `dim1==dim2`, a line is plotted. In this case, the
    `colormap` is ignored. Note that, if `dim1==dim2`, a 2D image is also
    projected as described above for higher-dimensional images.
    """
    if hasMatPlotLib:
        out = PyDIP_bin.ImageDisplay(img, range, complexMode=complexMode, projectionMode=projectionMode, coordinates=coordinates, dim1=dim1, dim2=dim2)
        if out.Dimensionality() == 1:
            axes = pp.gca()
            axes.clear()
            axes.plot(out)
            axes.set_ylim((0, 255))
            axes.set_xlim((0, out.Size(0) - 1))
        else:
            if colormap == '':
                if range == 'base' or range == 'based':
                    colormap = 'coolwarm'
                elif range == 'modulo' or range == 'labels':
                    colormap = 'labels'
                elif range == 'angle' or range == 'orientation':
                    colormap = 'hsv'
                else:
                    colormap = 'gray'
            if colormap == 'labels':
                cmap = _label_colormap()
            else:
                cmap = pp.get_cmap(colormap)
            pp.imshow(out, cmap=cmap, norm=matplotlib.colors.NoNorm(), interpolation='none')
        pp.draw()
        pp.pause(0.001)


PyDIP_bin.Image.Show = Show


# Support functions for ImageReadBioFormats()
def _DeinitBioFormats():
    print('Killing Java VM')
    javabridge.kill_vm()


def _InitializeBioFormats():
    if not hasattr(_InitializeBioFormats, "_initialized"):
        javabridge.start_vm(class_path=bioformats.JARS, run_headless=True)
        _InitializeBioFormats._initialized = True
        import atexit
        atexit.register(_DeinitBioFormats)
        # TODO: The interpreter hangs upon exit after using Bio-Formats, whether
        #       we register the cleanup function or not. The clean-up function
        #       is never called.


# A function that uses BioFormats to load images
def ImageReadBioFormats(filename):
    """Reads an image file using Bio-Formats.

    The module 'python-bioformats' must be installed.
    See https://pythonhosted.org/python-bioformats/.

    If the installation fails while trying to build the javabridge module
    because of a missing header file, try the following:
    ```
    export CFLAGS="-I`python3 -c 'import numpy;print(numpy.get_include())'`"
    pip3 install javabridge
    ```
    """
    if not hasBioFormats:
        print("""
PyDIP.ImageReadBioFormats requires the 'python-bioformats' module to be
installed. You can install it by typing on your Linux/MacOS command prompt:
    pip3 install python-bioformats
or under Windows:
    python3 -m pip install python-bioformats
""")
        raise ModuleNotFoundError('python-bioformats module required')
    _InitializeBioFormats()

    formatTools = bioformats.formatreader.make_format_tools_class()
    dataTypeTranslator = {
        formatTools.UINT8: 'UINT8',
        formatTools.INT8: 'SINT8',
        formatTools.UINT16: 'UINT16',
        formatTools.INT16: 'SINT16',
        formatTools.UINT32: 'UINT32',
        formatTools.INT32: 'SINT32',
        formatTools.FLOAT: 'SFLOAT',
        formatTools.DOUBLE: 'DFLOAT',
    }  # NOTE! This fails for binary images and for complex-valued images.
       # I think this is an issue with the BioFormats-Python interface
    with bioformats.ImageReader(filename) as reader:
        dataType = dataTypeTranslator[reader.rdr.getPixelType()]
        tensorElements = reader.rdr.getSizeC()
        sizes = [reader.rdr.getSizeX(), reader.rdr.getSizeY(), reader.rdr.getSizeZ(), reader.rdr.getSizeT()]
        out = PyDIP_bin.Image(sizes, tensorElements, dataType)
        for t in range(sizes[3]):
            for c in range(tensorElements):
                for z in range(sizes[2]):
                    out.TensorElement(c)[:, :, z, t] = reader.read(c, z, t, rescale=False)
        # reader.close()  # closed automatically by the with... syntax.

    meta = bioformats.OMEXML(bioformats.get_omexml_metadata(filename))
    pixels = meta.image(0).Pixels
    pixelSizes = [PyDIP_bin.PhysicalQuantity(1)]*4
    pixelSizeX = pixels.PhysicalSizeX
    if pixelSizeX:
        pixelSizes[0] = PyDIP_bin.PhysicalQuantity(pixelSizeX, pixels.PhysicalSizeXUnit)
    pixelSizeY = pixels.PhysicalSizeY
    if pixelSizeY:
        pixelSizes[1] = PyDIP_bin.PhysicalQuantity(pixelSizeY, pixels.PhysicalSizeYUnit)
    pixelSizeZ = pixels.PhysicalSizeZ
    if pixelSizeZ:
        pixelSizes[2] = PyDIP_bin.PhysicalQuantity(pixelSizeZ, pixels.PhysicalSizeZUnit)
    out.SetPixelSize(PyDIP_bin.PixelSize(pixelSizes))
    if tensorElements == 3:
        out.SetColorSpace('RGB')  # Assume a 3-channel image is RGB
    if sizes[3] == 1:
        out.Squeeze(3)  # Remove T dimension if singleton
    if sizes[2] == 1:
        out.Squeeze(2)  # Remove Z dimension if singleton
    return out


# A Python implementation of dip::ImageRead, so that we can use a different BioFormats interface
def ImageRead(filename, format=""):
    """Reads the image in a file filename.

    format can be one of:
    - `"ics"`: The file is an ICS file, use `dip.ImageReadICS`.
    - `"tiff"`: The file is a TIFF file, use `dip.ImageReadTIFF`. Reads only
      the first image plane.
    - `"jpeg"`: The file is a JPEG file, use `dip.ImageReadJPEG`.
    - `"bioformats"`: Use `dip.ImageReadBioFormats` to read the file with the
      Bio-Formats library.
    - `""`: Select the format by looking at the file name extension or the
      file's first few bytes. This is the default.

    If the 'python-bioformats' module is not installed (by the user, separately
    from PyDIP), then the `"bioformats"` format will not be available.

    Use the filetype-specific functions directly for more control over how the
    image is read.
    """

    if not format:  # Note! This logic copied from dip::ImageRead() in include/diplib/simple_file_io.h
        format = pathlib.Path(filename).suffix
        if format:
            format = format[1:].lower()  # remove the dot
        if format == 'ics' or format == 'ids':
            format = 'ics'
        elif format == 'tif' or format == 'tiff':
            format = 'tiff'
        elif  format == 'jpg' or format == 'jpeg':
            format = 'jpeg'
        else:
            format = 'bioformats'

    if format == 'ics':
        return PyDIP_bin.ImageReadICS(filename)
    elif format == 'tiff':
        return PyDIP_bin.ImageReadTIFF(filename)
    elif format == 'jpeg':
        return PyDIP_bin.ImageReadJPEG(filename)
    elif hasBioFormats and format == 'bioformats':
        return ImageReadBioFormats(filename)
    else:
        raise NameError('File format not recognized: ' + format)
