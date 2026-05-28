//
// Created by biqt on 1/4/18.
//

#ifndef BIQT_JAVA_PROVIDER_H
#define BIQT_JAVA_PROVIDER_H

#include "ProviderInterface.h"

/**
 * A function to begin provider analysis.
 *
 * @param filePath The path to the input file.
 * @param providerName This should be the provider *directory* name where the
 * jar files can be found.
 * @param className The name of the Provider class to use for evaluation. This
 * should be the fully qualified class name, e.g.
 * org/mitre/biqt/IrisProvider
 * @param classPath The java class path required for this provider.
 *
 * @return The return status of the provider.
 */
const char *java_provider_eval(const char *filePath, const char *providerName,
                               const char *className, const char *classPath);

#endif //BIQT_JAVA_PROVIDER_H
