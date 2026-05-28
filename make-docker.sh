declare -a BIQT_DOCKER_BUILD_ARGS

DEFAULT_BIQT_COMMIT="master"
DEFAULT_BIQT_IRIS_COMMIT="master"
DEFAULT_BIQT_FACE_COMMIT="master"
DEFAULT_BIQT_CONTACT_DETECTOR_COMMIT="master"

if [ "${BIQT_COMMIT}" == "" ]; then
	BIQT_COMMIT="${DEFAULT_BIQT_COMMIT}"
fi

BIQT_COMMIT_RESOLVED=$(git ls-remote https://github.com/mitre/biqt.git "${BIQT_COMMIT}" | awk '{print $1}')
if [ "${BIQT_COMMIT_RESOLVED}" == "" ]; then
	echo "Unable to resolve BIQT ref '${BIQT_COMMIT}'." >&2
	exit 1
fi
BIQT_DOCKER_BUILD_ARGS+=(--build-arg BIQT_COMMIT="${BIQT_COMMIT_RESOLVED}")
echo "Building with BIQT ${BIQT_COMMIT} @ ${BIQT_COMMIT_RESOLVED}."

if [ "${WITH_BIQT_IRIS}" == "ON" ]; then
	BIQT_DOCKER_BUILD_ARGS+=(--build-arg WITH_BIQT_IRIS=ON)
	if [ "${BIQT_IRIS_COMMIT}" == "" ]; then
		BIQT_IRIS_COMMIT="${DEFAULT_BIQT_IRIS_COMMIT}"
	fi
	BIQT_IRIS_COMMIT_RESOLVED=$(git ls-remote https://github.com/mitre/biqt-iris.git "${BIQT_IRIS_COMMIT}" | awk '{print $1}')
	if [ "${BIQT_IRIS_COMMIT_RESOLVED}" == "" ]; then
		echo "Unable to resolve BIQT_IRIS ref '${BIQT_IRIS_COMMIT}'." >&2
		exit 1
	fi
	BIQT_DOCKER_BUILD_ARGS+=(--build-arg BIQT_IRIS_COMMIT="${BIQT_IRIS_COMMIT_RESOLVED}")
	echo "Building with BIQT_IRIS ${BIQT_IRIS_COMMIT} @ ${BIQT_IRIS_COMMIT_RESOLVED}"
fi

if [ "${WITH_BIQT_FACE}" == "ON" ]; then
	BIQT_DOCKER_BUILD_ARGS+=(--build-arg WITH_BIQT_FACE=ON)
	if [ "${BIQT_FACE_COMMIT}" == "" ]; then
		BIQT_FACE_COMMIT="${DEFAULT_BIQT_FACE_COMMIT}"
	fi
	BIQT_FACE_COMMIT_RESOLVED=$(git ls-remote https://github.com/mitre/biqt-face.git "${BIQT_FACE_COMMIT}" | awk '{print $1}')
	if [ "${BIQT_FACE_COMMIT_RESOLVED}" == "" ]; then
		echo "Unable to resolve BIQT_FACE ref '${BIQT_FACE_COMMIT}'." >&2
		exit 1
	fi
	BIQT_DOCKER_BUILD_ARGS+=(--build-arg BIQT_FACE_COMMIT="${BIQT_FACE_COMMIT_RESOLVED}")
	echo "Building with BIQT_FACE ${BIQT_FACE_COMMIT} @ ${BIQT_FACE_COMMIT_RESOLVED}."
fi

if [ "${WITH_BIQT_CONTACT_DETECTOR}" == "ON" ]; then
	BIQT_DOCKER_BUILD_ARGS+=(--build-arg WITH_BIQT_CONTACT_DETECTOR=ON)
	if [ "${BIQT_CONTACT_DETECTOR_COMMIT}" == "" ]; then
		BIQT_CONTACT_DETECTOR_COMMIT="${DEFAULT_BIQT_CONTACT_DETECTOR_COMMIT}"
	fi
	BIQT_CONTACT_DETECTOR_COMMIT_RESOLVED=$(git ls-remote https://github.com/mitre/biqt-contact-detector.git "${BIQT_CONTACT_DETECTOR_COMMIT}" | awk '{print $1}')
	if [ "${BIQT_CONTACT_DETECTOR_COMMIT_RESOLVED}" == "" ]; then
		echo "Unable to resolve BIQT_CONTACT_DETECTOR ref '${BIQT_CONTACT_DETECTOR_COMMIT}'." >&2
		exit 1
	fi
	BIQT_DOCKER_BUILD_ARGS+=(--build-arg BIQT_CONTACT_DETECTOR_COMMIT="${BIQT_CONTACT_DETECTOR_COMMIT_RESOLVED}")
	echo "Building with BIQT_CONTACT_DETECTOR ${BIQT_CONTACT_DETECTOR_COMMIT} @ ${BIQT_CONTACT_DETECTOR_COMMIT_RESOLVED}."
fi

BIQT_TAG="${BIQT_COMMIT}"
if [ "${BIQT_TAG}" == "master" ]; then
       BIQT_TAG="latest"
fi

# Added `--pull` argument to force retrieval of the latest base image.
# Remove `--secret` argument if organization certificate does not need to be installed.
DOCKER_BUILDKIT=1 docker build \
		--pull \
		"${BIQT_DOCKER_BUILD_ARGS[@]}" \
		--build-arg QUIRK_OPENSSL_RENEGOTIATION=ON \
		--build-arg QUIRK_STRIP_QT5CORE_METADATA=ON \
		--secret id=ca_file,src=ca_file.crt \
		--progress=plain \
	 	--tag "ghcr.io/mitre/biqt:${BIQT_TAG}" \
		--platform=linux/amd64 \
		docker/
