# Set up the default environment for stand-alone plugin developers
# Some environment variables that my be needed are:
#
#   $(INTELCOMPILER_HOME)   - The installation path of the Intel Compiler. Will force the use of intel compiler if set.
#   $(BOOSTLIB_DECORATORS)  - Disables auto library detection in boost & allows you to specify diffent lib decorators
#                             For example if you're boost libraries are built with a different compiler
#                               eg. boostLibDecorators = "-vc100-mt-1_46"
#   $(BOOST_INCLUDE_PATH)   - The path to the boost headers. Default is '$(BOOST_HOME)/include'
#   $(MSINTTYPES_HOME)      - The path to MSIntTypes library
#   $(BOOST_HOME)           - The boost installation path
#

import sys, os

env2 = Environment()

if sys.platform.startswith("win"):
    cppdefines = {
        "_USE_MATH_DEFINES": "1",
        "WIN32_LEAN_AND_MEAN": "1",
        "NOMINMAX": "1",
        "_WIN32_WINNT": "0x0501",
        "BOOST_DISABLE_ASSERTS": "1",
        "BOOST_PROPERTY_TREE_PAIR_BUG": "1",

    }

    env2.Append(CPPDEFINES=cppdefines)
    env2.Append(CCFLAGS=[
        "/EHsc", "/GR", "/FC", "/MD",
        "/Qrestrict",
        "/Qansi-alias",
        "/DBOOST_HAS_STDINT_H",
        "/DBOOST_PYTHON_STATIC_LIB",
        "/DBOOST_REGEX_NO_LIB",
    ])

    intelCompilerHome = os.environ.get("INTELCOMPILER_HOME", None)
    if intelCompilerHome is not None:
        # Setup for Intel Compiler
        intelCompilerPath = intelCompilerHome + "/bin/intel64"
        env2.AppendENVPath("INCLUDE", [intelCompilerHome + "/compiler/include"])
        env2.AppendENVPath('LIB', [intelCompilerHome + "/compiler/lib/intel64"])
        env2.AppendENVPath("PATH", [intelCompilerPath])

        env2.Replace(CC ="icl")
        env2.Replace(CXX ="icl")
        env2.Replace(LINK = "xilink")
        env2.AppendUnique(LIBPATH = [intelCompilerHome + "/compiler/lib/intel64"])
        env2.AppendUnique(CPPPATH=[intelCompilerHome + "/compiler/include"])


    boostHome = os.environ.get("BOOST_HOME")

    # Allow users to specify a decorator for boost libraries so that explicit libs can be used
    #     eg. BoostLibDecorators = "-vc100-mt-1_46"
    #         To link to libboost_filesystem-vc100-mt-1_46
    boostLibDecorators = os.environ.get("BOOSTLIB_DECORATORS", None)
    if boostLibDecorators is not None:
        env2.Append(CCFLAGS=["/DBOOST_ALL_NO_LIB",])

        env2.AppendUnique(LIBS=[
            "libboost_%s%s" % ("python", boostLibDecorators),
            "libboost_%s%s" % ("regex", boostLibDecorators),
            "libboost_%s%s" % ("system", boostLibDecorators),
            "libboost_%s%s" % ("filesystem", boostLibDecorators),
            "libboost_%s%s" % ("thread", boostLibDecorators)
        ])

    env2.AppendUnique(LIBPATH=[boostHome + "/lib"])

    boostIncludePath = os.environ.get("BOOST_INCLUDE_PATH", boostHome + "/include")
    env2.AppendUnique(CPPPATH=[boostIncludePath])

    msIntTypesHome = os.environ.get("MSINTTYPES_HOME")
    env2.AppendUnique(CPPPATH=[msIntTypesHome])

Return('env2')
