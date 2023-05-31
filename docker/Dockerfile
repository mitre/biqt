# NOTICE
#
# This software (or technical data) was produced for the U.S. Government
# under contract, and is subject to the Rights in Data-General Clause
# 52.227-14, Alt. IV (DEC 2007).
#
# Copyright 2020 The MITRE Corporation. All Rights Reserved.

FROM centos:7 as build

ARG GIT_SSL_NO_VERIFY
ENV GIT_SSL_NO_VERIFY ${GIT_SSL_NO_VERIFY}

# Install prerequisites.
RUN yum install -y epel-release && \
    yum update -y && \
    yum groupinstall -y "Development Tools" && \ 
    yum install -y cmake3 opencv opencv-devel qt5-qtbase qt5-qtbase-devel

# Clone repositories and any necessary submodules.
RUN mkdir /app && \
    cd /app && \
    git clone https://github.com/mitre/biqt.git && \
    git clone https://github.com/mitre/biqt-iris.git --branch r0.4 && \
    git clone https://github.com/mitre/biqt-face.git && \
    git clone --recurse-submodules https://github.com/biometrics/openbr.git --branch v1.1.0

# Perform the build.
RUN export NUM_CORES=$(cat /proc/cpuinfo | grep -Pc "processor\s*:\s*[0-9]+\s*$") && \
    echo "Builds will use ${NUM_CORES} core(s)." && \
    cd /app/biqt && \
    mkdir build && \
    cd build && \
    cmake3 -DCMAKE_BUILD_TYPE=Release -DWITH_JAVA=OFF .. && \
    make -j${NUM_CORES} && \
    make install && \
    source /etc/profile.d/biqt.sh && \
    cd /app/openbr && \
    mkdir build && \
    cd build && \
    cmake3 -DCMAKE_BUILD_TYPE=Release -DBR_WITH_OPENCV_NONFREE=OFF -DCMAKE_INSTALL_PREFIX=/opt/openbr .. && \
    make -j${NUM_CORES} && \
    make install && \
    cd /app/biqt-face && \
    mkdir build && \
    cd build && \
    cmake3 -DCMAKE_BUILD_TYPE=Release -DOPENBR_DIR=/opt/openbr .. && \
    make -j${NUM_CORES} && \
    make install && \
    cd /app/biqt-iris && \
    mkdir build && \
    cd build && \
    cmake3 -DCMAKE_BUILD_TYPE=Release .. && \
    make -j${NUM_CORES} && \
    make install

# Reduce image size.
FROM centos:7 as release
COPY --from=build /opt/openbr /opt/openbr
COPY --from=build /usr/local /usr/local
COPY --from=build /etc/profile.d/biqt.sh /etc/profile.d/biqt.sh
RUN yum -y install opencv qt5-qtbase qt5-qtbase-gui && echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/openbr/lib:/opt/openbr/lib64' >> /etc/profile.d/biqt.sh
ENTRYPOINT ["/bin/bash", "-l"]
