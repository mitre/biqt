# Docker (CentOS 7)

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
  
# Linux (CentOS Linux 7.4)

## Prerequisites

This guide targets CentOS Linux 7.4.

This provider relies on OpenCV and other core development tools including gcc. These dependencies can be installed from 
the [Extra Packages for Enterprise Linux (EPEL)](https://fedoraproject.org/wiki/EPEL#How_can_I_use_these_extra_packages.3F) 
repository using the following commands:

```bash
sudo yum groupinstall "Development Tools"
sudo yum install -y cmake3 maven
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
cmake3 -DCMAKE_BUILD_TYPE=Release ..
make -j4
sudo make install
```

Remember to open a new console window or explicitly call `source /etc/profile.d/biqt` before attempting
to start BIQT!

## Running BIQT

```bash
$> biqt --version
BIQT v0.1
```

# Windows 10

## Prerequisites

This guide targets Microsoft Windows 10.

The following tools are required to build BIQT from source. When installing CMake and Git,
please select the option to add the executables to the system's `PATH` environment variable.
  * [CMake 3.9+](https://cmake.org/files/v3.9/cmake-3.9.2-win64-x64.msi)
  * [Git 2.16+](https://git-scm.com/)
  * [Microsoft Visual Studio Community Edition 2013](https://www.visualstudio.com/vs/older-downloads/) (Registration required.)

The following software is required to build the Java bindings for BIQT.
  * [Apache Maven 3.5.2](https://maven.apache.org/download.cgi)
  * [Java SE JDK 9](http://www.oracle.com/technetwork/java/javase/downloads/index.html)

After installing Apache Maven and a Java JDK, set the following environment variables according to the example below. Note that
these examples may need to be changed depending on the software versions you install and where you install them.
  * Set `JAVA_HOME` to `C:\Program Files\Java\jdk-9.0.4`.
  * Update `PATH` to include `C:\apache-maven-3.5.2\bin` and `C:\Program Files\Java\jdk-9.0.4\bin\server`.

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

By default, BIQT will be installed to `C:\Program Files\biqt`. During installation, you will be reminded to set 
the `BIQT_HOME` and `PATH` environment variables as follows:
  * Set `BIQT_HOME` to `C:\Program Files\biqt`.
  * Update `PATH` to include `C:\Program Files\biqt\bin`.

## Verifying installation.

After installation, you can invoke the BIQT CLI from any command prompt. If you encounter any errors,
please verify that `JAVA_HOME` and the `PATH` variables are assigned correctly.

```
$> biqt --version
BIQT v0.1
```