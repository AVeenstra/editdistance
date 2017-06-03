# Edit Distance (Levenshtein distance) calculation with a GPU (cluster)

## Installation
### Debian (Jessie)
#### AMD GPUs
Required packages:
 * amd-opencl-dev
 * amd-libopencl1-dev
 * gcc
 * g++
 * cmake
 * amd-opencl-icd
 * amd-clinfo
 * opencl-headers
 * fglrx-driver
 * mpich repo or [manual](http://www.mpich.org/static/downloads/3.2/mpich-3.2-installguide.pdf) (if manual do not forget the --enable-cxx flag while configuring)

### Windows
Required programs:
 * Visual Studio (tested with 2017)
 * Microsoft MPI v8 [Hosted by Microsoft](https://www.microsoft.com/en-us/download/details.aspx?id=54607)
 * Driver of your graphics card

### Compilation
Build with CMake.

### Issues
Make sure the executing user has enough pliviliges
