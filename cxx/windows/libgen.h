//
// Created by biqt on 10/20/17.
//

#ifndef BIQT_LIBGEN_H
#define BIQT_LIBGEN_H

/**
 * Provides an implementation of dirname() for Windows
 *
 * @param path The path to find the directory name for. If path does not contain
 * / or \, "." is returned. This may modify the contents of path, so be sure to
 * pass a copy if this value needs to be preserved.
 */
static char *dirname(char *path)
{
    int i;
    for (i = strlen(path) - 2; i >= 0; --i) {
        if (path[i] == '\\' || path[i] == '/') {
            path[i] = '\0';
            return path;
        }
    }
    return ".";
}

#endif // BIQT_LIBGEN_H
