#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* List of platform features */
#ifdef _WIN32
#define OS "win32"
#define IS_WINDOWS
#endif
#ifdef __linux
#define OS "linux"
#define IS_LINUX
#endif
#ifdef __APPLE__
#define OS "darwin"
#define IS_MACOS
#endif

/* System, but with string replace */
int run(const char *cmd, ...) {
    char buf[512];
    va_list args;
    va_start(args, cmd);
    vsprintf(buf, cmd, args);
    va_end(args);
    printf("--> %s\n\n", buf);
    return system(buf);
}

/* List of Node.js versions */
struct node_version {
    char *name;
    char *abi;
} versions[] = {
    {"v10.0.0", "64"},
    {"v11.1.0", "67"},
    {"v12.0.0", "72"}
};

struct electron_version {
    char *name;
    char *abi;
} electron_versions[] = {
    {"v5.0.6", "70"}
};

/* Downloads headers, creates folders */
void prepare() {
    if (run("mkdir dist") || run("mkdir targets")) {
        return;
    }

    run("git submodule update --init --recursive");
    /* For all versions */
    for (unsigned int i = 0; i < sizeof(versions) / sizeof(struct node_version); i++) {
        run("curl -OJ https://nodejs.org/dist/%s/node-%s-headers.tar.gz", versions[i].name, versions[i].name);
        run("tar xzf node-%s-headers.tar.gz -C targets", versions[i].name);
        run("curl https://nodejs.org/dist/%s/win-x64/node.lib > targets/node-%s/node.lib", versions[i].name, versions[i].name);
    }

    // Build for electron's node version
    for (unsigned int i = 0; i < sizeof(electron_versions) / sizeof(struct electron_version); i++) {
        run("curl -OJL https://electronjs.org/headers/%s/node-%s-headers.tar.gz", electron_versions[i].name, electron_versions[i].name);
        run("mkdir targets/node-%s", electron_versions[i].name);
        run("tar xzf node-%s-headers.tar.gz -C targets/node-%s", electron_versions[i].name, electron_versions[i].name);
        run("curl -OJL https://atom.io/download/atom-shell/%s/x64/node.lib > targets/node-%s/node.lib", electron_versions[i].name, electron_versions[i].name);
    }
}

/* Build for Unix systems */
void build(char *compiler, char *cpp_compiler, char *cpp_linker, char *os, char *arch) {
    char *c_shared = "-DLIBUS_USE_LIBUV -flto -O3 -c -fPIC -I uWebSockets/uSockets/src uWebSockets/uSockets/src/*.c uWebSockets/uSockets/src/eventing/*.c";
    char *cpp_shared = "-DLIBUS_USE_LIBUV -flto -O3 -c -fPIC -std=c++17 -I uWebSockets/uSockets/src -I uWebSockets/src src/addon.cpp";

    for (unsigned int i = 0; i < sizeof(versions) / sizeof(struct node_version); i++) {
        run("%s %s -I targets/node-%s/include/node", compiler, c_shared, versions[i].name);
        run("%s %s -I targets/node-%s/include/node", cpp_compiler, cpp_shared, versions[i].name);
        run("%s %s %s -o dist/uws_%s_%s_%s.node", cpp_compiler, "-flto -O3 *.o -std=c++17 -shared", cpp_linker, os, arch, versions[i].abi);
    }
    // build for electron
    for (unsigned int i = 0; i < sizeof(electron_versions) / sizeof(struct electron_version); i++) {
        run("%s %s -I targets/node-%s/node_headers/include/node", compiler, c_shared, electron_versions[i].name);
        run("%s %s -I targets/node-%s/node_headers/include/node", cpp_compiler, cpp_shared, electron_versions[i].name);
        run("%s %s %s -o dist/uws_%s_%s_%s.node", cpp_compiler, "-flto -O3 *.o -std=c++17 -shared", cpp_linker, os, arch, electron_versions[i].abi);
    }
}

void copy_files() {
#ifdef IS_WINDOWS
    run("copy \"src\\uws.js\" dist /Y");
#else
    run("cp src/uws.js dist/uws.js");
#endif
}

/* Special case for windows */
void build_windows(char *arch) {
    /* For all versions */
    for (unsigned int i = 0; i < sizeof(versions) / sizeof(struct node_version); i++) {
        run("cl /D \"LIBUS_USE_LIBUV\" /std:c++17 /I uWebSockets/uSockets/src uWebSockets/uSockets/src/*.c "
            "uWebSockets/uSockets/src/eventing/*.c /I targets/node-%s/include/node /I uWebSockets/src /EHsc "
            "/Ox /LD /Fedist/uws_win32_%s_%s.node src/addon.cpp targets/node-%s/node.lib",
            versions[i].name, arch, versions[i].abi, versions[i].name);
    }

    // build for electron
    for (unsigned int i = 0; i < sizeof(electron_versions) / sizeof(struct electron_version); i++) {
        run("cl /D \"LIBUS_USE_LIBUV\" /std:c++17 /I uWebSockets/uSockets/src uWebSockets/uSockets/src/*.c "
                    "uWebSockets/uSockets/src/eventing/*.c /I targets/node-%s/node_headers/include/node /I uWebSockets/src /EHsc "
                    "/Ox /LD /Fedist/uws_win32_%s_%s.node src/addon.cpp targets/node-%s/node.lib",
                    electron_versions[i].name, arch, electron_versions[i].abi, electron_versions[i].name);
    }
}

int main() {
    printf("[Preparing]\n");
    prepare();
    printf("\n[Building]\n");

#ifdef IS_WINDOWS
    build_windows("x64");
#else
#ifdef IS_MACOS
    /* Apple special case */
    build("clang -mmacosx-version-min=10.7",
          "clang++ -stdlib=libc++ -mmacosx-version-min=10.7",
          "-undefined dynamic_lookup",
          OS,
          "x64");
#else
    /* Linux */
    build("clang",
          "clang++",
          "-static-libstdc++ -static-libgcc -s",
          OS,
          "x64");

    /* If linux we also want arm64 */
    build("aarch64-linux-gnu-gcc", "aarch64-linux-gnu-g++", "-static-libstdc++ -static-libgcc -s", OS, "arm64");
#endif
#endif

    copy_files();
}
