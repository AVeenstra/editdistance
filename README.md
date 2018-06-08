# Edit Distance (Levenshtein distance) calculation on a GPU cluster, managed by MPI.

## License: GNU Affero General Public License v3.0 (GNU AGPLv3)
This project is licensed under GNU AGPLv3, but if for some reason you need a more permissive license please let me know by creating an issue.
Short description of the contents of the [license](/LICENSE.md):
> Permissions of this strongest copyleft license are conditioned on making available complete source code of licensed works and modifications, which include larger works using a licensed work, under the same license. Copyright and license notices must be preserved. Contributors provide an express grant of patent rights. When a modified version is used to provide a service over a network, the complete source code of the modified version must be made available. 

> Source: https://choosealicense.com/licenses/agpl-3.0/

## Installation
Mpich can be installed from the repositories of the distribution, but if multiple distributions or distribution versions will be used you should install Mpich manually to guarantee the same version is installed on all the nodes.
Mpich and MSMPI are not compatible and Mpich is not available on Windows, but maybe OpenMPI could work on both.
The following setups have been tested:

### Linux general
Required packages:
 * g++
 * cmake
 * opencl-headers
 * mpi:
   * mpich repo or [manual](http://www.mpich.org/static/downloads/3.2/mpich-3.2-installguide.pdf) (if manual do not forget the --enable-cxx flag while configuring)
   * libopenmpi-dev

#### Debian (Jessie) with AMD GPUs
Required packages:
 * amd-opencl-dev
 * amd-libopencl1-dev
 * amd-opencl-icd
 * amd-clinfo
 * fglrx-driver

#### Ubuntu (16.04) with Nvidia GPUs
Required packages:
 * nvidia-375
 * nvidia-opencl-icd-375
 * nvidia-opencl-dev
 
#### Ubuntu (16.04) with AMD GPUs
PPA with extra packages (probably optional): `sudo add-apt-repository ppa:oibaf/graphics-drivers`

Required packages:
 * ocl-icd-opencl-dev
 * mesa-opencl-icd-dbg
 
### Windows
Required programs:
 * Visual Studio (tested with 2017)
 * Microsoft MPI (MSMPI) v8 [Hosted by Microsoft](https://www.microsoft.com/en-us/download/details.aspx?id=54607)
 * Driver of your graphics card

### Compilation
Build with CMake.

### Issues
Make sure the executing user has enough pliviliges
