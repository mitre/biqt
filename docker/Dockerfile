# NOTICE
#
# This software (or technical data) was produced for the U.S. Government
# under contract, and is subject to the Rights in Data-General Clause
# 52.227-14, Alt. IV (DEC 2007).
#
# Copyright The MITRE Corporation. All Rights Reserved.

FROM ubuntu:22.04 AS build

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
ENV QUIRK_STRIP_QT5CORE_METADATA ${QUIRK_STRIP_QT5CORE_METADATA}

# Building on MITRE-internal infrastructure requires the installation of the MITRE certificate chain. This is OPTIONAL
# for external systems. The chain will be installed when provided the build argument `INSTALL_MITRE_CERTIFICATES=ON`.
ARG INSTALL_MITRE_CERTIFICATES=OFF
ENV INSTALL_MITRE_CERTIFICATES ${INSTALL_MITRE_CERTIFICATES}

ARG WITH_BIQT_FACE=OFF
ENV WITH_BIQT_FACE ${WITH_BIQT_FACE}

ARG WITH_BIQT_IRIS=OFF
ENV WITH_BIQT_IRIS ${WITH_BIQT_IRIS}

ARG WITH_BIQT_CONTACT_DETECTOR=OFF
ENV WITH_BIQT_CONTACT_DETECTOR ${WITH_BIQT_CONTACT_DETECTOR}

RUN set -e && \
    echo -e "Build plan:\n" \
        "\tBIQT Face: ${WITH_BIQT_FACE} \n" \
        "\tBIQT Iris: ${WITH_BIQT_IRIS} \n" \
        "\tBIQT Contact Detector: ${WITH_BIQT_CONTACT_DETECTOR}" && \
    apt update && \
    apt upgrade -y; \
    DEBIAN_FRONTEND=noninteractive apt -y install git less vim cmake g++ curl libopencv-dev libjsoncpp-dev openjdk-17-jdk pip && \
    pip install wheel; \
    if [[ "${WITH_BIQT_FACE}" == "ON" ]]; then \
        set -e; \
        echo "Installing QT5 for BIQT Face."; \
        DEBIAN_FRONTEND=noninteractive apt -y install qtbase5-dev; \
        if  [ "${QUIRK_STRIP_QT5CORE_METADATA}" == "ON" ]; then \
            echo "Stripping libQt5Core.so of its ABI metadata."; \
            strip --remove-section=.note.ABI-tag /usr/lib/x86_64-linux-gnu/libQt5Core.so; \
        fi; \
    fi;

# Install MITRE certificate chain (if opted in).
RUN for i in "http://pki.mitre.org/MITRE%20BA%20Root.crt" "http://pki.mitre.org/MITRE%20BA%20NPE%20CA-1.crt" "http://pki.mitre.org/MITRE%20BA%20NPE%20CA-3(1).crt" "http://pki.mitre.org/ZScaler_Root.crt"; \
    do \
        if [ "${INSTALL_MITRE_CERTIFICATES}" == "ON" ]; then \
            echo "Adding ${i}."; \
            curl $i >> /etc/ssl/certs/ca-certificates.crt; \
        fi; \
    done;

ARG BIQT_COMMIT
ENV BIQT_COMMIT ${BIQT_COMMIT}

# Clone and install BIQT from MITRE Github.
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
ENV BIQT_IRIS_COMMIT ${BIQT_IRIS_COMMIT}

# Stage 2: BIQT Iris
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

# Clone and install OpenBR from GitHub. 
RUN set -e; \
    if [ "${WITH_BIQT_FACE}" == "ON" ]; then \
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
    else \
        echo "Skipping OpenBR build for BIQT Face. BIQT Face not requested."; \
	mkdir /opt/openbr || true; \
 	echo "WITH_BIQT_FACE was not set to ON so openbr not compiled and installed." > /opt/openbr/README; \
    fi

ARG BIQT_FACE_COMMIT
ENV BIQT_FACE_COMMIT ${BIQT_FACE_COMMIT}
# Clone and install BIQT Face from the MITRE github repository. 
RUN set -e; \
    source /etc/profile.d/biqt.sh; \
    if [ "${WITH_BIQT_FACE}" == "ON" ]; then \
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
    fi;


ARG BIQT_CONTACT_DETECTOR_COMMIT
ENV BIQT_CONTACT_DETECTOR_COMMIT ${BIQT_CONTACT_DETECTOR_COMMIT}
# Stage 3: BIQT Contact Detector
RUN set -e; \
    source /etc/profile.d/biqt.sh; \
    if [ "${WITH_BIQT_CONTACT_DETECTOR}" == "ON" ]; then \
      ( mkdir /app 2>/dev/null || true ); \
      cd /app; \
      git clone https://github.com/mitre/biqt-contact-detector biqt-contact-detector --branch "${BIQT_CONTACT_DETECTOR_COMMIT}" --depth 1; \
      cd biqt-contact-detector; \
      pip install -r requirements.txt; \
      export NUM_CORES=$(cat /proc/cpuinfo | grep -Pc "processor\s*:\s*[0-9]+\s*$"); \
      mkdir build; \
      cd build; \
      cmake -DCMAKE_BUILD_TYPE=Release ..; \
      make -j${NUM_CORES}; \
      make install; \
    fi;

# Reduce image size.
FROM ubuntu:22.04 AS release
SHELL ["/bin/bash", "-c"] 
COPY --from=build /usr/local /usr/local
COPY --from=build /etc/profile.d/biqt.sh /etc/profile.d/biqt.sh
COPY --from=build /opt/openbr /opt/openbr

ARG QUIRK_STRIP_QT5CORE_METADATA=OFF
ENV QUIRK_STRIP_QT5CORE_METADATA ${QUIRK_STRIP_QT5CORE_METADATA}

RUN apt update && \
     DEBIAN_FRONTEND=noninteractive apt -y install openjdk-17-jre-headless g++ libopencv-core4.5d libopencv-highgui4.5d libopencv-imgcodecs4.5d libopencv-imgproc4.5d libjsoncpp25 libqt5xml5 libqt5sql5  libpython3.10 libopencv-objdetect4.5d libqt5widgets5 libopencv-ml4.5d libopencv-videoio4.5d libpython3.10-dev python3-distutils && \
     ( [ "${QUIRK_STRIP_QT5CORE_METADATA}" == "ON" ] && echo "Stripping libQt5Core.so of its ABI metadata." && strip --remove-section=.note.ABI-tag /usr/lib/x86_64-linux-gnu/libQt5Core.so* )

ENTRYPOINT ["/bin/bash", "-l"]
