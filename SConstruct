#!/usr/bin/env python
import os
import sys
import fnmatch

#Thanks to PeanutButterAddict from the discords for the flag change & folder source finding snippets (though I've lightly modified them)

def collect_source_files(directory, patterns):
    for root, dirs, files in os.walk(directory):
        for basename in files:
            if any(fnmatch.fnmatch(basename, pattern) for pattern in patterns):
                #print("out - " + basename)
                filename = os.path.join(root, basename)
                yield filename

env = SConscript("godot-cpp/SConstruct") # type: ignore

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

if '/std:c++17' in env['CXXFLAGS']: #windows
    env['CXXFLAGS'].remove('/std:c++17')
    env.Append(CXXFLAGS=['/std:c++20'])
elif "-std=c++17" in env['CXXFLAGS']: #linux
    env['CXXFLAGS'].remove('-std=c++17')
    env.Append(CXXFLAGS=['-std=c++20'])
else:
    print("Failed to replace flag, current flags: ", env["CXXFLAGS"])
    exit()


env.Append(CPPPATH=["src/"])
sources = list(collect_source_files('src', ('*.cpp', '*.c')))

extName = "ScriptableAnimationPlayer/SAP"

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "../"+extName+".{}.{}.framework/libgdVoxEngCPP.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "../"+extName+"{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )
    
Default(library) # type: ignore
print("Flags:",env['CXXFLAGS'])
