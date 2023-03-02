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
ENV CFLAGS "-std=c99"

# Install prerequisites.
RUN yum install -y epel-release && \
    yum update -y && \
    yum groupinstall -y "Development Tools" && \ 
    yum install -y cmake3 opencv opencv-devel qt5-qtbase qt5-qtbase-devel gcc openssl-devel bzip2-devel libffi-devel zlib-devel wget java-11-openjdk-devel \
        gcc make zlib-devel bzip2 bzip2-devel readline-devel sqlite sqlite-devel openssl-devel tk-devel libffi-devel xz-devel \
        atlas atlas-devel lapack lapack-devel

# Install pyenv
RUN mkdir /app && \
    git clone https://github.com/pyenv/pyenv.git "${HOME}/.pyenv" && \
    echo PYENV_ROOT="${HOME}/.pyenv" >> /etc/profile.d/pyenv.sh && \
    echo 'command -v pyenv >/dev/null || export PATH="$PYENV_ROOT/bin:$PATH"' >> /etc/profile.d/pyenv.sh && \
    echo 'eval "$(pyenv init -)"' >> /etc/profile.d/pyenv.sh && \
    chmod +x /etc/profile.d/pyenv.sh && \
    source /etc/profile.d/pyenv.sh && \
    pyenv install 3.9 && \
    pyenv global 3.9

# Clone repositories and any necessary submodules.
RUN ( mkdir /app || echo "/app already exists" ) && \
    cd /app && \
    git clone https://github.com/mitre/biqt.git --branch "java-11" && \
    git clone https://github.com/mitre/biqt-iris.git && \
    git clone https://github.com/mitre/biqt-face.git && \
    git clone https://github.com/mitre/biqt-contact-detector.git --branch "python3.9" && \
    git clone --recurse-submodules https://github.com/biometrics/openbr.git --branch "v1.1.0"

# # Perform the build.
RUN export NUM_CORES=$(cat /proc/cpuinfo | grep -Pc "processor\s*:\s*[0-9]+\s*$") && \
    echo "Builds will use ${NUM_CORES} core(s)." && \
    cd /app/biqt && \
    mkdir build && \
    cd build && \
    cmake3 -DCMAKE_BUILD_TYPE=Release -DWITH_JAVA=ON .. && \
    make -j${NUM_CORES} && \
    make install && \
    source /etc/profile.d/biqt.sh && \
    cd /app/openbr && \
    mkdir build && \
    cd build && \
    cmake3 -DCMAKE_BUILD_TYPE=Release -DBR_INSTALL_BRPY=OFF -DBR_WITH_OPENCV_NONFREE=OFF -DCMAKE_INSTALL_PREFIX=/opt/openbr .. && \
    make -j${NUM_CORES} && \
    make install

RUN export NUM_CORES=$(cat /proc/cpuinfo | grep -Pc "processor\s*:\s*[0-9]+\s*$") && \
    source /etc/profile.d/biqt.sh && \
    cd /app/biqt-face && \
    mkdir build && \
    cd build && \
    cmake3 -DCMAKE_BUILD_TYPE=Release -DOPENBR_DIR=/opt/openbr .. && \
    make -j${NUM_CORES} && \
    make install

RUN export NUM_CORES=$(cat /proc/cpuinfo | grep -Pc "processor\s*:\s*[0-9]+\s*$") && \
    source /etc/profile.d/biqt.sh && \
    cd /app/biqt-iris && \
    mkdir build && \
    cd build && \
    cmake3 -DCMAKE_BUILD_TYPE=Release .. && \
    make -j${NUM_CORES} && \
    make install

# Skipping contact detector for now.
RUN export NUM_CORES=$(cat /proc/cpuinfo | grep -Pc "processor\s*:\s*[0-9]+\s*$") && \
    source /etc/profile.d/biqt.sh && \
    source /etc/profile.d/pyenv.sh && echo "${PYENV_ROOT}" && \
    cd /app/biqt-contact-detector && \ 
    python --version && pip --version && \
    pip install -r requirements.txt && \
    mkdir build && \
    cd build && \
    cmake3 -DCMAKE_BUILD_TYPE=Release .. && \
    make -j${NUM_CORES} && \
    make install

# # Reduce image size.
FROM centos:7 as release
COPY --from=build /opt/openbr /opt/openbr
COPY --from=build /usr/local /usr/local
COPY --from=build /etc/profile.d/pyenv.sh /etc/profile.d/pyenv.sh
COPY --from=build /etc/profile.d/biqt.sh /etc/profile.d/biqt.sh
COPY --from=build /root/.pyenv /root/.pyenv

RUN yum -y install qt5-qtbase qt5-qtbase-gui wget java-11-openjdk-devel bzip2 sqlite atlas lapack opencv && \
	rm -rf /root/.pyenv/.git && \
	find /root/.pyenv/ -type d -name '__pycache__' | xargs rm -rf
         
# #  
# #  echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/openbr/lib:/opt/openbr/lib64' >> /etc/profile.d/biqt.sh
ENTRYPOINT ["/bin/bash", "-l"]
