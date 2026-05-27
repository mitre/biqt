# NOTICE
#
# This software (or technical data) was produced for the U.S. Government
# under contract, and is subject to the Rights in Data-General Clause
# 52.227-14, Alt. IV (DEC 2007).
#
# Copyright The MITRE Corporation. All Rights Reserved.

FROM dhi.io/debian-base:bookworm-debian12-dev AS build

SHELL ["/bin/bash", "-c"] 

# TODO: BIQT Face runs into an interesting issue where it fails to find `libQt5Core.so` despite it being installed
# in `/usr/lib/x86_64-linux-gnu/`. None of the other Qt libraries share this issue - they are linked and resolved.
# A cursory review reveals that it is likely related to the information baked into the core library's `.note.ABI-tag`,
# and specifically, the library does not appear to be compatible with the kernel used by Ubuntu. The working theory
# is that this is a false-positive and can be safely ignored given all of the other Qt5 modules are compatible.
#
# The QUIRK_STRIP_QT5CORE_METADATA build argument is exposed as a workaround. When set to ON, the `strip` command is
# used to remove this metadata which allows the library to link as expected. However, the default setting is OFF
# because the actual reason for the perceived incompatibility is not understood.
ARG QUIRK_STRIP_QT5CORE_METADATA=OFF
ENV QUIRK_STRIP_QT5CORE_METADATA=${QUIRK_STRIP_QT5CORE_METADATA}

ARG WITH_BIQT_FACE=OFF
ENV WITH_BIQT_FACE=${WITH_BIQT_FACE}

ARG WITH_BIQT_IRIS=OFF
ENV WITH_BIQT_IRIS=${WITH_BIQT_IRIS}

ARG WITH_BIQT_CONTACT_DETECTOR=OFF
ENV WITH_BIQT_CONTACT_DETECTOR=${WITH_BIQT_CONTACT_DETECTOR}

ARG DEBIAN_FRONTEND=noninteractive
ENV DEBIAN_FRONTEND=${DEBIAN_FRONTEND}

# Stage 1: Install build dependencies
RUN set -e && \
    echo -e "Build plan:\n" \
        "\tBIQT Face: ${WITH_BIQT_FACE} \n" \
        "\tBIQT Iris: ${WITH_BIQT_IRIS} \n" \
        "\tBIQT Contact Detector: ${WITH_BIQT_CONTACT_DETECTOR}" && \
    apt upgrade -y && \
    apt install -y --no-install-recommends \
        build-essential \
        cmake \
        curl \
        git \
        libjsoncpp-dev \
        libopencv-dev \
        openjdk-17-jdk && \
    apt clean && rm -rf /var/lib/apt/lists/*

# Building on some infrastructure requires the installation of a certificate authority. This is OPTIONAL
# for external systems and will only be installed when the ca_file secret is provided in the build command.
RUN --mount=type=secret,id=ca_file,required=false \
    if [[ -f /run/secrets/ca_file ]]; then \
       cat /run/secrets/ca_file >> /etc/ssl/certs/ca-certificates.crt; \
    fi

ARG BIQT_COMMIT
ENV BIQT_COMMIT=${BIQT_COMMIT}

# Stage 2: Clone and install BIQT from MITRE Github.
RUN set -e; \
    echo "BIQT_COMMIT=${BIQT_COMMIT}" ; \
    mkdir /app 2>/dev/null || true; \
    cd /app; \
    export JAVA_BIN=`dirname $(readlink -f \`which javac\`)`; \
    export JAVA_HOME=`dirname "${JAVA_BIN}"`; \
    git clone --verbose https://github.com/mitre/biqt --branch "${BIQT_COMMIT}" biqt-pub; \
    export NUM_CORES=$(cat /proc/cpuinfo | grep -Pc "processor\s*:\s*[0-9]+\s*$"); \
    echo "Builds will use ${NUM_CORES} core(s)."; \
    cd /app/biqt-pub; \
    mkdir build; \
    cd build; \
    cmake -DBUILD_TARGET=UBUNTU -DCMAKE_BUILD_TYPE=Release -DWITH_JAVA=ON ..; \
    make -j${NUM_CORES}; \
    make install; \
    source /etc/profile.d/biqt.sh

ARG BIQT_IRIS_COMMIT
ENV BIQT_IRIS_COMMIT=${BIQT_IRIS_COMMIT}

# Stage 3: BIQT Iris
RUN set -e; \
    if [ "${WITH_BIQT_IRIS}" == "ON" ]; then \
        echo "BIQT_IRIS_COMMIT: ${BIQT_IRIS_COMMIT}"; \
        source /etc/profile.d/biqt.sh; \
        ( mkdir /app 2>/dev/null || true ); \
        cd /app; \
        git clone --verbose https://github.com/mitre/biqt-iris --branch "${BIQT_IRIS_COMMIT}" biqt-iris; \
        export NUM_CORES=$(cat /proc/cpuinfo | grep -Pc "processor\s*:\s*[0-9]+\s*$"); \
        echo "Builds will use ${NUM_CORES} core(s)."; \
        cd /app/biqt-iris; \
        mkdir build; \
        cd build; \
        cmake -DCMAKE_BUILD_TYPE=Release ..; \
        make -j${NUM_CORES}; \
        make install; \
    else \
        echo "Skipping BIQT Iris."; \
    fi;

ARG BIQT_FACE_COMMIT
ENV BIQT_FACE_COMMIT=${BIQT_FACE_COMMIT}

# Stage 4: BIQT Face
RUN set -e; \
    source /etc/profile.d/biqt.sh; \
    if [ "${WITH_BIQT_FACE}" == "ON" ]; then \
        # Install BIQT Face dependencies
        set -e; \
        echo "Installing QT5 for BIQT Face."; \
        apt update && \
        apt install -y qtbase5-dev --no-install-recommends && \
        apt clean && rm -rf /var/lib/apt/lists/* && \
        if  [ "${QUIRK_STRIP_QT5CORE_METADATA}" == "ON" ]; then \
            echo "Stripping libQt5Core.so of its ABI metadata."; \
            strip --remove-section=.note.ABI-tag /usr/lib/x86_64-linux-gnu/libQt5Core.so; \
        fi; \
        # Clone and install OpenBR from GitHub. 
        echo "Using OpenSSL Configuration at ${OPENSSL_CONF}: `cat ${OPENSSL_CONF}`"; \
        mkdir /app 2>/dev/null || true; \
        cd /app; \
        git clone https://github.com/biometrics/openbr.git openbr || exit 5; \
        cd /app/openbr; \
	    git checkout 1e1c8f; \
        mkdir build; \
        cd build; \
        cmake -DCMAKE_BUILD_TYPE=Release -DBR_WITH_OPENCV_NONFREE=OFF -DCMAKE_INSTALL_PREFIX=/opt/openbr ..; \
        export NUM_CORES=$(cat /proc/cpuinfo | grep -Pc "processor\s*:\s*[0-9]+\s*$"); \
        make -j${NUM_CORES}; \
        make install; \
        # Clone and install BIQT Face from the MITRE github repository.
        echo "BIQT_FACE_COMMIT: ${BIQT_FACE_COMMIT}"; \        
        echo "Using OpenSSL Configuration at ${OPENSSL_CONF}: `cat ${OPENSSL_CONF}`"; \
        mkdir /app 2>/dev/null || true; \
        cd /app; \
        git clone https://github.com/mitre/biqt-face.git biqt-face --depth=1 --branch "${BIQT_FACE_COMMIT}"; \
        cd /app/biqt-face; \
        mkdir build; \
        cd build; \
        cmake -DCMAKE_BUILD_TYPE=Release -DOPENBR_DIR=/opt/openbr ..; \
        make -j${NUM_CORES}; \
        make install; \
    else \
        echo "Skipping BIQT Face."; \
        mkdir /opt/openbr || true; \
        echo "WITH_BIQT_FACE was not set to ON so openbr not compiled and installed." > /opt/openbr/README; \
    fi;


ARG BIQT_CONTACT_DETECTOR_COMMIT
ENV BIQT_CONTACT_DETECTOR_COMMIT=${BIQT_CONTACT_DETECTOR_COMMIT}
ARG USE_GPU=OFF
ENV USE_GPU=${USE_GPU}

# Stage 5: BIQT Contact Detector
RUN set -e; \
    source /etc/profile.d/biqt.sh; \
    if [ "${WITH_BIQT_CONTACT_DETECTOR}" == "ON" ]; then \
        apt update && \
        apt install -y python3 python3-dev python3-pip --no-install-recommends && \
        apt clean && rm -rf /var/lib/apt/lists/* && \
        ( mkdir /app 2>/dev/null || true ); \
        cd /app; \
        git clone https://github.com/mitre/biqt-contact-detector biqt-contact-detector --branch "${BIQT_CONTACT_DETECTOR_COMMIT}" --depth 1; \
        cd biqt-contact-detector; \
        if [ "${USE_GPU}" == "ON"]; then \
            pip install -r requirements.txt; \
        else \
            pip install -r requirements-cpu.txt --index-url https://download.pytorch.org/whl/cpu --extra-index-url https://pypi.org/simple; \
        fi; \
        export NUM_CORES=$(cat /proc/cpuinfo | grep -Pc "processor\s*:\s*[0-9]+\s*$"); \
        mkdir build; \
        cd build; \
        cmake -DCMAKE_BUILD_TYPE=Release ..; \
        make -j${NUM_CORES}; \
        make install; \
    fi;

# Reduce image size.
FROM dhi.io/debian-base:bookworm-debian12-dev AS release
SHELL ["/bin/bash", "-c"] 
COPY --from=build /usr/local /usr/local
COPY --from=build /etc/profile.d/biqt.sh /etc/profile.d/biqt.sh
COPY --from=build /opt/openbr /opt/openbr

ENV BIQT_HOME=/usr/local/share/biqt
ENV PATH="/usr/local/bin:${PATH}"
ENV LD_LIBRARY_PATH="/usr/local/lib:/usr/local/lib64"

ARG QUIRK_STRIP_QT5CORE_METADATA=OFF
ENV QUIRK_STRIP_QT5CORE_METADATA=${QUIRK_STRIP_QT5CORE_METADATA}

RUN apt update && \
    DEBIAN_FRONTEND=noninteractive apt -y install --no-install-recommends \
    g++ \
    libjsoncpp25 \
    libopencv-core406 \
    libopencv-highgui406 \
    libopencv-imgcodecs406 \
    libopencv-imgproc406 \
    libopencv-ml406 \
    libopencv-objdetect406 \
    libopencv-videoio406 \
    libpython3.11-dev \
    libqt5sql5 \
    libqt5widgets5  \
    libqt5xml5 \
    openjdk-17-jre-headless \
    python3-distutils \
    python3-pkg-resources && \
    apt clean && rm -rf /var/lib/apt/lists/* && \
    mkdir -p /etc/ld.so.conf.d && \
    printf "/usr/local/lib\\n/usr/local/lib64\\n" > /etc/ld.so.conf.d/biqt.conf && \
    ldconfig && \
    if [ "${QUIRK_STRIP_QT5CORE_METADATA}" == "ON" ]; then \
        echo "Stripping libQt5Core.so of its ABI metadata."; \
        strip --remove-section=.note.ABI-tag /usr/lib/*-linux-gnu/libQt5Core.so*; \
    fi

ENTRYPOINT ["/bin/bash", "-l"]
