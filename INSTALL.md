# Docker (Ubuntu 22.04)

## Prerequisites

This guide builds a BIQT Docker image which includes the reference BIQTIris and BIQTFace providers. The BIQT Java
bindings are unavailable in this image.

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
BIQT v0.1 
``` 
  
# Linux (Ubuntu Linux 22.04)

## Prerequisites

This guide targets Ubuntu Linux 22.04.

This provider relies on OpenCV and other core development tools including gcc. These dependencies can be installed using the following commands:

```bash
sudo apt update
sudo apt install -y cmake build-essential libjsoncpp-dev openjdk-17-jdk
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

Remember to open a new console window or explicitly call `source /etc/profile.d/biqt` before attempting
to start BIQT!

## Running BIQT

```bash
$> biqt --version
BIQT v23.04
```


## Building and Installing

As an administrator, execute the following commands from the VS2013 x64 Cross Tools Command Prompt. By default, a shortcut to 
this prompt can be found by browsing to `C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\Shortcuts`. Right-click 
on the shortcut and select 'Run as administrator' from the popup menu.

**Note:** Using the VS2012 x64 Cross Tools Command Prompt Start Menu shortcut will not work!

By default, CMake will attempt to build the BIQT Java bindings. Java bindings can be disabled by adding
`-DWITH_JAVA=OFF` to the `cmake` command.

```
git clone git@github.com:mitre/biqt biqt
cd biqt
mkdir build
cmake -G "NMake Makefiles" -DBUILD_STATIC_LIBS=OFF -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release ..
nmake
nmake install
```


## Verifying installation.

After installation, you can invoke the BIQT CLI from any command prompt. If you encounter any errors,
please verify that `JAVA_HOME` and the `PATH` variables are assigned correctly.

```
$> biqt --version
BIQT v23.04
```