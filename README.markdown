# Computer Vision Algorithms

## Installation
To download all files, run

```
git clone --recursive https://github.com/hucal/cv-composer
```

CMake and OpenCV are needed to compile this project.
On Ubuntu, these can be installed easily by running:

```
sudo apt install libopencv-dev cmake
```

On NixOS, you can use the `defaut.nix` file should install the complete development version of OpenCV.

```
nix-shell
```

### Compilation

```
cd build
cmake ..
make
```

## Help

Run the following to get a list of commands.

```
./composer --help
```

Run the Canny edge detector. Get help by running `./composer canny --help`

```
./composer --image image.png canny
```

See intermediate results while running the edge detector on input from the composer camera:

```
./composer --camera canny --show-intermediate
```

Calculate the convolution between an image `image.png` and a kernel.
Save results to a file.

```
./composer --no-gui --image image.png convolution --kernel sharpen
```

```
./composer --no-gui --image image.png convolution --polar-x scharr-west --polar-y scharr-north
```