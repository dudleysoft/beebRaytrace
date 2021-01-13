# beebRaytrace
BBC Micro Native Pi Co-processor real time raytracing.

## Introduction

This is a demo of real-time raytracing using a Pi Native Co-processor on a standard BBC Micro computer, 
this is a port of RAYTRC in BBC Basic (well techincally a port of the subtalis port of the BBC Basic RAYTRC program).

The raytrace code has been converted (rather crudely) to C code and that code bolted into a testbed (actually my smacker player code)
for my beebScreen library. 

## Usage

To run normally type <code>*RT</code> or if you have a VideoNULA installed you can type <code>*RT -N</code> which uses the VideoNULA extended palette for nicer shadows.

The program also allows for the checkerboard ground to be replaced with a textured image, the image should be a square raw image in 8bpp using the standard BBC palette in the first 8 colours 
(black, red, green, yellow, blue, magenta, cyan and white), the bmp2raw.exe file in the directory will convert 24bit BMP images into usable textures. To use an image for the ground simply type
<code>*RT image -S sizeInPixels</code>. There are 3 example images in the repository:-

**FE2** is the loading screen for my Frontier elite 2 port (64x64 pixels), 

**OWL** is my partners logo (128x128 pixels) and 

**PACMAN** is from Tricky's new pacman port to the Beeb (256x256 pixels)

## Technical details

The ray trace code was ported from a BBC Basic program, therefore the variable names are a obtuse for speed reasons in an interpreted language, it probably doesn't make easy reading
and I certainly had my share of headaches while trying to work out why it wasn't working properly while converting it.

The transfer of the screen is done using my beebScreen library code, this provides a collection of functions that make converting graphics into the BBC micro screen layout, it handles
colour conversion from a 12bit RGB palette to a 1 bit RGB palette along with dithering, it correctly munges the format of pixel colours and the layout of the screen data to match the BBC
screen layout, as well as handling transmission of the delta changes to the host machine, along with injecting the code required for the host to decode the screen.

### Compression format

The data is compressed in a fairly simple delta format, the bytes of this are sent from the parasite to the host via the OSWRCH FIFO, the parasite is fast enough that once the initial
signal code has been sent the host can continuously read from this FIFO and be guaranteed that there will always be a byte waiting for it. For the compressed screen character 255 is used
as the initial signal code.

After the signal we then send the first address to write as a little endian 2 byte value, this is followed by a series of command bytes, these are:-

* **0** - End of data stream.
* **1-127** - The number of changed bytes we're sending, this is followed by that number of bytes which are written in reverse order to the current address, which is then incremented by the number of bytes.
* **128** - New data address, this is followed by a new address in the next 2 bytes, encoded the same as the initial address (in fact the code simply branches to the same initialise codee used for the start address)
* **129-255** - Skip the number-128 bytes of unchanged data (0-127)

The parasite skips any unchanged bytes at the start of the buffer and then sends the correct start address to the host, technically the next command will always be a change block, however for code size it's
easier to assume it's a command and treat it as such since our code space is limited. It will also skip any unchanged bytes at the end of the file too. If there are no changes then the code will send the 4 bytes in total, the initial command, the end of screen memory as an address and a 0 command.


