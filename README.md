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

### Compilation
`gcc -Wall -o hello hello.c -lOpenCL`

### Run
`./hello`

### Issues
Make sure the executing user has enough pliviliges
