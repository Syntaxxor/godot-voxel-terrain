#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

if env["platform"] == "windows" and env["target"] == "template_debug":
    env.Append(LINKFLAGS=["/DEBUG"])

if env["platform"] == "osx":
    library = env.SharedLibrary("addons/voxel-terrain/bin/lib-voxel-terrain.{}.framework/lib-voxel-terrain.{}".format(
        env["platform"], env["platform"]
    ), source=sources)
else:
    library = env.SharedLibrary("addons/voxel-terrain/bin/voxel-terrain.{}.{}{}".format(
        env["platform"], env["arch"], env["SHLIBSUFFIX"]
    ), source=sources)

Default(library)
