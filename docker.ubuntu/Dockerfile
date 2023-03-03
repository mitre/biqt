# NOTICE
#
# This software (or technical data) was produced for the U.S. Government
# under contract, and is subject to the Rights in Data-General Clause
# 52.227-14, Alt. IV (DEC 2007).
#
# Copyright The MITRE Corporation. All Rights Reserved.

FROM ubuntu:22.04 as build

ARG GIT_SSL_NO_VERIFY
ENV GIT_SSL_NO_VERIFY ${GIT_SSL_NO_VERIFY}

# Install prerequisites.
RUN apt update && \
    apt upgrade -y && \
    DEBIAN_FRONTEND=noninteractive apt install -y git less vim cmake openjdk-17-jdk g++ 

# Stage 1: BIQT
RUN ( mkdir /app 2>/dev/null || true ) && \
    cd app && \
    export JAVA_BIN=`dirname $(readlink -f \`which javac\`)` && \
    export JAVA_HOME=`dirname "${JAVA_BIN}"` && \
    git clone https://github.com/mitre/biqt.git && \
    export NUM_CORES=$(cat /proc/cpuinfo | grep -Pc "processor\s*:\s*[0-9]+\s*$") && \
    echo "Builds will use ${NUM_CORES} core(s)." && \
    cd /app/biqt && \
    mkdir build && \
    cd build && \
    cmake -DBUILD_TARGET=UBUNTU -DCMAKE_BUILD_TYPE=Release -DWITH_JAVA=ON .. && \
    make -j${NUM_CORES} && \
    make install && \
    source /etc/profile.d/biqt.sh

# Stage 2: BIQT Iris
RUN apt install -y libopencv-dev

# Stage 3: BIQT Contact Detector
RUN apt install -y pip && \
    pip install -r requirements.txt && \
    mkdir build && \
    cd build && \
    cmake3 -DCMAKE_BUILD_TYPE=Release .. && \
    make -j4 && \
    make install

# # opencv opencv-devel qt5-qtbase qt5-qtbase-devel

# # Clone repositories and any necessary submodules.
# RUN mkdir /app && \
#     cd /app && \
#     git clone https://github.com/mitre/biqt.git && \
#     git clone https://github.com/mitre/biqt-iris.git && \
#     git clone https://github.com/mitre/biqt-face.git && \
#     git clone --recurse-submodules https://github.com/biometrics/openbr.git

# # Perform the build.
# RUN export NUM_CORES=$(cat /proc/cpuinfo | grep -Pc "processor\s*:\s*[0-9]+\s*$") && \
#     echo "Builds will use ${NUM_CORES} core(s)." && \
#     cd /app/biqt && \
#     mkdir build && \
#     cd build && \
#     cmake -DCMAKE_BUILD_TYPE=Release -DWITH_JAVA=OFF .. && \
#     make -j${NUM_CORES} && \
#     make install && \
#     source /etc/profile.d/biqt.sh && \
#     cd /app/openbr && \
#     mkdir build && \
#     cd build && \
#     cmake3 -DCMAKE_BUILD_TYPE=Release -DBR_WITH_OPENCV_NONFREE=OFF -DCMAKE_INSTALL_PREFIX=/opt/openbr .. && \
#     make -j${NUM_CORES} && \
#     make install && \
#     cd /app/biqt-face && \
#     mkdir build && \
#     cd build && \
#     cmake3 -DCMAKE_BUILD_TYPE=Release -DOPENBR_DIR=/opt/openbr .. && \
#     make -j${NUM_CORES} && \
#     make install && \
#     cd /app/biqt-iris && \
#     mkdir build && \
#     cd build && \
#     cmake3 -DCMAKE_BUILD_TYPE=Release .. && \
#     make -j${NUM_CORES} && \
#     make install

# # Reduce image size.
# FROM centos:7 as release
# COPY --from=build /opt/openbr /opt/openbr
# COPY --from=build /usr/local /usr/local
# COPY --from=build /etc/profile.d/biqt.sh /etc/profile.d/biqt.sh
# RUN yum -y install opencv qt5-qtbase qt5-qtbase-gui && echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/openbr/lib:/opt/openbr/lib64' >> /etc/profile.d/biqt.sh
ENTRYPOINT ["/bin/bash", "-l"]
