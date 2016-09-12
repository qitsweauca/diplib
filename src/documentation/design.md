DIPlib 3.0 design decisions {#design}
===

This page lists some of the design decisions that went into *DIPlib 3.0*.
Many of these decisions are inherited from the previous version of the library,
and some new ones are made possible by the port to C++.


Function signatures
---

There are two possible function signature styles for use in an image analysis
library:

1. `void Filter( Image &in, Image &out, int size );`

2. `Image Filter( Image &in, int size );`

Both of these options have advantages and disadvantages. Style 1 allows for
in-place operation:

    Image img = ...
    Filter( img, img, 1 );

The function here is able to write the results in the image's pixel buffer,
without having to allocate a temporary pixel buffer as would be the case for:

    Image img = ...
    img = Filter( img, 1 );

This is a huge advantage both in speed and memory usage. However, resulting
programs are not as easy to read (which parameters are inputs and which are
outputs?) and not as pretty as with style 2. For example, style 2 allows
for a very elegant chaingin of operations:

    Image img = ...
    img = Filter2( Filter1( img, 3 ), 1 );

Furthermore, style 2 makes it much easier to automatically generate interfaces
to languages (such as MATLAB) that do not allow a function to modify its input
arguments. Such an automatic interface generation tool needs to know which
arguments are inputs and which are outputs.

In the previous version of *DIPlib* (written in C), all functions returned an
error code, and so both input and output values were function arguments
(style 1). But in C++ we have exceptions to handle error conditions, and so
are free to have an image as the return value of the function (style 2).
However, the advantage of style 1 is too large to ignore. Therefore, we have
kept the function signature style (and argument order) of the old *DIPlib*.
However, we have written a small, inline wrapper function for most of the image
filters that follow the signature style 2. Such a wrapper is very straight-forward:

    inline Image Filter( Image &in, int size ) {
        Image out;
        Filter( in, out, size );
        return out;
    }

We have chosen not to pollute the documentation with these wrapper functions.
However, if a function `Filter( in, out )` exists, then you can assume that
there will also be a function `out = Filter( in )`.


Class method vs function
---

Some libraries put all image processing/analysis functionality into the
image object as methods. The idea is to filter an image by
`img.Gauss(sigma)`. This is a terrible idea for many reasons: it's ugly, one
never knows if the image object is modified by the method or not, and the core
include file for the library changes when adding any type of functionality,
forcing recompilation of the whole library. Filters should be functions, not
methods.

In *DIPlib*, methods to the core dip::Image class query and manipulate image
properties, not pixel data (with the exception of dip::Image::Copy and
dip::Image::Set).


Compile-time vs run-time pixel type identification
---

*DIPlib* uses run-time identification of an image's pixel type, and functions
dispatch internally to the appropriate sub-function. These sub-functions are
generated at compile time through templates

The alternative, seen in most C++ image analysis libraries (*ITK*, *Vigra*,
*CImg*, etc.), is to define the image class, as well as most functions, as
templates. The user declares an image having a specific data type, and the
compiler then creates an image class with that data type for the pixels, as well
as instances of all functions called with this image as input. This takes time.
Compiling even a trivial program that uses *CImg* takes a minute, rather than a
fraction of a second it takes to compile a trivial program that uses *DIPlib*.
Writing most functionality as templates implies that most code is actually in
the header files, rather than in the source files. This functionality then ends
up in the application executable, rather than in an independent library file
(shareable among many applications).

However, the largest disadvantage with templated functions happens when creating
an (even slightly) general image analysis program: you need to write code that
allows the user of your program to select the image data type, and write code
that does all the right dispatching depending on the data type. Alternatively,
you have to restrict the data type to one choice. A library is meant to take
away work from the programmer using the library, so it is logical that *DIPlib*
should allow all data types and do the dispatching as necessary. After all,
*DIPlib* is meant as a foundation for *DIPimage* and similar general-purpose
image analysis tools, where you cannot determine in advance which data type the
user will want to use. Think about how complicated each of the *DIPlib*-MEX-files
would be if *DIPlib* had compile-time typing: each MEX-file would have to check
the data type of the input array (or arrays), convert it to a *DIPlib* image
class of the same type, then call one of 8 or 10 instances of a function.
Furthermore, this MEX-file would need to know which image data types are
meaningful for the function being called (integer only? binary only? does it
work on complex values?). Instead, we simply convert the input array to a
*DIPlib* image and call a function. The MEX-file is trivial, the *DIPlib*
function itself takes care of everything.

*DIPlib* does expose a few templated functions to the user. However, these
templates typicaly abstract the type of a constant (see, for example, the
function dip::Add with a templated `rhs` argument), and never that of an image.
Such a template is always a trivial function that simplifies the library user's
code.


Passing options to a function
---

Many algorithms require parameters that select a mode of operation. *DIPlib 3.0*
uses strings for such parameters when the function is intended to be useable
from interfaces. By not defining C++ constants, the interface code can be
kept simple. For example, dip::Dilation has an option for the shape of the
structuring element. Instead of defining an `enum` (as the old *DIPlib* did)
with various values, that the interface code needs to translate, the option
parameter is a string. The user of the interface and the user of the C++ library
see the same parameter and use the function in the same way. The overhead of a
few string comparisons is trivial compared to the computational cost of any
image analysis algorithm.

An other advantage is having fewer possibilities for name clashes when defining
a lot of enumerator constants for the many, many options accumulated of the
large collection of functions and algorithms in *DIPlib*.

However, for infrastrucute functions not typically exposed in interfaces (i.e.
the functions that *DIPlib* uses internally to do its work) we do define
numeric constants for options. For example, see the enumerator dip::ThrowException,
or any of the flags defined through DIP_DECLARE_OPTIONS. These are more efficient
in use and equally convenient if limited to the C++ code.