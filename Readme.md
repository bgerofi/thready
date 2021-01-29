Thready is a simple library that captures pthreads and reports their TID along with which library they execute from.

### Installation

```bash
# Clone source, create build folder and compile
git clone https://github.com/bgerofi/thready.git
cd thready
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/path/to/install ..
make install
```

### Usage example

```bash
$ export OMP_NUM_THREADS=4; ~/install/thready/bin/threadyrun ./affinity-gnuopenmp
[TID: 113076]: 0x7fede901d310 is in libgomp.so.1.0.0 @ offset: 111376
[TID: 113075]: 0x7fede901d310 is in libgomp.so.1.0.0 @ offset: 111376
[TID: 113077]: 0x7fede901d310 is in libgomp.so.1.0.0 @ offset: 111376
hostname = mimosa.r-ccs27.riken.jp, rank = 000, OMP tid = 2, TID = 113076, CPU = 0, affinity = 0 1 2 3 4
hostname = mimosa.r-ccs27.riken.jp, rank = 000, OMP tid = 0, TID = 113074, CPU = 1, affinity = 0 1 2 3 4
hostname = mimosa.r-ccs27.riken.jp, rank = 000, OMP tid = 3, TID = 113077, CPU = 4, affinity = 0 1 2 3 4
hostname = mimosa.r-ccs27.riken.jp, rank = 000, OMP tid = 1, TID = 113075, CPU = 4, affinity = 0 1 2 3 4
```

