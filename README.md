# PopLib
This fork of PopLib aims to re-open and commpliment TeamPopWork's copy, by adding more contributions and features.  

# What has changed from the original SexyAppBase 1.3 and 1.34:
    A more appropriate name.  
    C++20 and 23 instead of C++14.
    Renderers such as SDL and OpenGL, ridding of platform specifics from the Windows API, and making this independent to all Operating Systems, including Apple and Linux.
    Opensource philosophy.
    partial paklib support (PopCap's pak file format).
    
    The replacement of ImageLib with Sean Barett's `stb_image` set of headers, to load all of PopCap's image formats into memory at runtime. 
    This means additional formats will be supported in the near future: BMP, GIF, PSD and HDR, alongside the already established PNG & JPEG.
    It also unifies the entire imagelib into one set of files, therefore ridding of separate folders of libpng and libjpeg.  
    This also rids of the proprietary "J2K" or JPEG2000 formats, as there is no point in specifying external libs for 2K resolution JPEG images.

# Future Plans:
    Adding Contributors/collaborators.
    Adding further support for OpenGL and Vulkan rendering capabilities, including shader support.
    Re-Adding back the Discord Social SDK (the Discord RPC had to be removed due to discontinuation and replacement by the discord Social/App SDKs).
    Full Steam API support.
    Adding TIFF support, including SVG and other vector image formats.
    Adding Blender model support (P3D?).  
    Full support of HTTP and HTTP/2 (maybe even HTTP/3), example: if one wants to share stats on an external server. 
    (Prominent in all of PopCap's games before 2015).
    
    Full paklib support, with encoding and decoding available.
    Adding some more game examples.
    Complete rework of the font classes, unified to PopFont, instead of using 3 separate rendering capabilties to render a font, combining 
    all 3 rendering capabilities and dding more if needed.
   
    Add support for 3D coordinate systems, used alongside the blender model support in the near future. [?].
    Add an IDE similar to unity or gamemaker but with specific use with this library, along with LawnProject's ResoddedFramework (called PopIDE).  
    Code can be edited in another IDE if preferred.
    Adding a brand new resource generator which picks any parameter from an XML file such as cols, rows, nobits, nopalettes, etc., including other 
    random parameters and tags.  
    A new font generator and tester that can be used alongside the future PopFont class.
    
# Licenses
    Please read the respective documents covering the separate licenses included with this library.

# Setting Up/Installing/Building PopLib
    A complete step by step guide will be provided later on.


    
    
    
    
    
