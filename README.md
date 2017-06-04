# Edit Distance (Levenshtein distance) calculation on a GPU cluster, managed by MPI.

## Installation
Mpich can be installed from the repositories of the distribution, but if multiple distributions or distribution versions will be used you should install Mpich manually to guarantee the same version is installed on all the nodes.
Mpich and MSMPI are not compatible and Mpich is not available on Windows, but maybe OpenMPI could work on both.
The following setups have been tested:

### Debian (Jessie) with AMD GPUs
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

### Ubuntu (16.04) with Nvidia GPUs
Required packages:
 * nvidia-375
 * nvidia-opencl-icd-375
 * nvidia-opencl-dev
 * gcc
 * cmake
 * mpich repo or [manual](http://www.mpich.org/static/downloads/3.2/mpich-3.2-installguide.pdf) (if manual do not forget the --enable-cxx flag while configuring)
 
### Windows
Required programs:
 * Visual Studio (tested with 2017)
 * Microsoft MPI (MSMPI) v8 [Hosted by Microsoft](https://www.microsoft.com/en-us/download/details.aspx?id=54607)
 * Driver of your graphics card

### Compilation
Build with CMake.

### Issues
Make sure the executing user has enough pliviliges
