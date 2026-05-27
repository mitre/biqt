# Docker (Ubuntu 22.04)

## Prerequisites

This guide builds a BIQT Docker image which includes the reference BIQTContactDetector, BIQTFace, and BIQTIris providers. The BIQT Java bindings are unavailable in this image.

Docker 17 or higher is required.

## Building and Installing

From the `docker/` directory in this project's repository, run the following commands from a Windows console or
Linux terminal:

```bash
docker build -t biqt-github .
```

## Running BIQT

Run the `biqt-github` image using `docker`.

```bash
$ docker run --rm -it biqt-github
# biqt --version
BIQT 26.05 
``` 
  
# Linux (Ubuntu Linux 22.04)

## Prerequisites

This guide targets Ubuntu Linux 22.04.

This provider relies on OpenCV and other core development tools including gcc. These dependencies can be installed using the following commands:

```bash
sudo apt update
sudo apt install -y cmake build-essential default-jdk libjsoncpp-dev openjdk-17-jdk
```

## Building and Installing

The following commands clone the BIQT source code, build the framework,
and install it to the default location `/usr/local/`. Note that installation
requires superuser privileges.

```bash
git clone git@github.com:mitre/biqt
cd biqt
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4
sudo make install
```

Remember to open a new console window or explicitly call `source /etc/profile.d/biqt.sh` before attempting
to start BIQT!

## Running BIQT

```bash
$> biqt --version
BIQT v26.05
```
