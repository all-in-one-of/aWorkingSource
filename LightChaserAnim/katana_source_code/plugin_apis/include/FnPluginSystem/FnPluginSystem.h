// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNPLUGINSYSTEM_H
#define FNPLUGINSYSTEM_H

/**
 *  \mainpage Katana APIs Reference
 *
 *  <p>
 *  Katana's functionality can be extended by C++ plug-ins. There are several APIs
 *  that can be used in their implementation. Each API can be found in the
 *  <a href="modules.html">Modules page</a> of this documentation.
 *  </p>
 *
 *  <p>
 *  Both plug-ins and the APIs are versioned, which means that Katana can
 *  verify if a certain plug-in is compatible with its APIs. The plug-ins communicate
 *  with Katana through a C interface, which means that any compiler can be used
 *  to build them. It also means that a plugin doesn't need to be recompiled
 *  for a new version of Katana, as long as the API versions used by the plugin
 *  are still supported inside Katana.
 *  <br><br>
 *  Note: Currently there is only one version of the APIs, so there is no need to
 *  be aware of their versioning for now.
 *  </p>
 *
 *  <p>
 *  Because of this architecture some C++ source files (provided in the PLUGIN_APIS
 *  directory) have to be compiled together with the plug-in source files. Please
 *  refer to the Makefiles of the example plug-ins to know which files are needed for
 *  each API type.
 *  </p>
 *
 *  <p>
 *  More than one plug-in (including different versions of a certain plug-in)
 *  can be compiled into a single shared object.
 *  </p>
 *
 *  <h2>Implementing and Registering the Plug-ins</h2>
 *
 *  <p>
 *  Each API will have a set of include files and C++ source files (provided in the
 *  PLUGIN_APIS directory) that need to be compiled together with the plug-in code.
 *  Please check the Makefiles provided with the example Plug-ins in the PLUGINS
 *  directory for more information about this.
 *  </p>
 *
 *  <p>
 *  For each plug-in there will be one main class (for example, a class that extends
 *  ScenegraphGenerator will be the main class for the Scenegraph Generator plug-ins).
 *  This needs to be registered from within its shared object so that the plug-in host
 *  (Katana in this case) can detect the plug-ins. This is achieved by:
 *  </p>
 *
 *
 *    - Calling the  DEFINE_XXX_PLUGIN(CLASS) macro - in which
 *      XXX is the acronym of the API name (for example: AMP for the Attribute
 *      Modifier Plug-ins) and CLASS the main plug-in class.
 *
 *    - Implementing the <i>void registerPlugins()</i> in which the macro
 *      <i>REGISTER_PLUGIN(PLUGIN_CLASS, PLUGIN_NAME, MAJOR, MINOR_VERSION)</i>
 *      should be called for each plug-in class in the shared library
 *
 *  <p>
 *  For example, if we are compiling a shared object with a Scenegraph Generator called
 *  AlembicIn version 1.0  and an Attribute Modifier called MeshScaler version 2.3, we would
 *  need to have the following calls:
 *  </p>
 *
 *  \code
 *  DEFINE_SGG_PLUGIN(AlembicIn)
 *  DEFINE_AMP_PLUGIN(MeshScaler)
 *
 *  void registerPlugins()
 *  {
 *     REGISTER_PLUGIN(AlembicIn, "AlembicIn", 1, 0);
 *     REGISTER_PLUGIN(MeshScaler, "MeshScaler", 2, 3);
 *  }
 *  \endcode
 *
 *  <h2>Compile Plugins</h2>
 *
 *  Please check the Makefiles provided with the example plug-ins in the PLUGINS directory for
 *
 *  Once compiled, the shared objects should be placed in a Libs/ sub-directory of the standard
 *  resource search path.  By default, this includes UI4/Resources, PLUGINS/Resources/Core, and
 *  and paths specified via $KATANA_RESOURCES.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup PluginSystemGrp Plug-in System
///
/// This system provides the framework for retrieving plug-ins from custom built libraries.
///
/// To implement a plug-in:
/// - Decide on the API to use, and include the appropriate header
/// - Implement the \ref FnGetPluginsFunc "getPlugins()" function (of type \ref FnGetPluginsFunc)
/// - Make your \ref FnGetPluginsFunc "getPlugins()" return a \ref FnPlugin structure with appropriate API identifiers
///   from the headers
/// - Implement the functions required by the API, and return them in the appropriate structure in your plug-in's
///   \ref FnPlugin::getSuite "getSuite()" function
/// - Build
/// - Copy (or symbolic-link) the library into the application's custom plug-ins folder
/// - Start the application, and check that your plug-in has loaded correctly
/// \{

//------------------------------------------------------------------------------

/// The return status of some of the Plug-in System functions
typedef enum FnPluginStatus
{
    FnPluginStatusOK = 0,
    FnPluginStatusError = -1
} FnPluginStatus;

// Back compatible type definition
typedef  FnPluginStatus FnPlugStatus;

/// The basic host information that will be provided to a plug-in by \ref FnPlugin::setHost "setHost()"
typedef struct FnPluginHost
{
    /// \brief Returns a pointer to the host's function suite for the given API.
    /// \param apiName The name of the API that the host implements
    /// \param apiVersion The integer version of the API that the host implements
    /// \return A pointer to the host's function suite for the given API if successful, or NULL otherwise.
    const void *(*getSuite)(const char *apiName, unsigned int apiVersion);

    const char *name;               ///< The name of the host application
    const char *versionStr;         ///< A string description of the application version number (e.g. "1.4v1")
    unsigned int versionInt;        ///< An integer version number for easy comparison (e.g., "1.2" could be 12 or 102)

} FnPluginHost;


/// The basic plug-in information that must be returned from a call to \ref FnGetPluginsFunc "getPlugins()"
typedef struct FnPlugin
{
    const char *name;                   ///< A descriptive name for the plug-in
    const char *apiName;                ///< The string name of the API supported by this plug-in
    unsigned int apiVersion;            ///< The integer version of the API supported by this plug-in
    unsigned int pluginVersionMajor;    ///< The major version of a plug-in changes when a new incompatible version is released
    unsigned int pluginVersionMinor;    ///< The minor version of a plug-in changes on small, but compatible, updates

    /// \brief The host will call this on the plug-in to provide its details and check for acceptance.
    /// \param host Basic information about the host, for identification and access to the host function suite
    /// \return FnPluginStatusOK if the plug-in accepts the host, or FnPluginStatusError if not
    //FnPluginStatus (*setHost)(const FnPluginHost *host);
    FnPluginStatus (*setHost)(FnPluginHost *host);

    /// \brief The plug-in must implement this to return its suite of functions that implements the API it supports.
    /// \return A pointer to a suite of functions in a structure, according to the API
    const void *(*getSuite)();

    /// \brief The host will call this to clean up any plug-in information before unloading the library.
    void (*flush)();

} FnPlugin;


/// \brief Returns a list of plug-ins provided by a plug-in library.
/// \details \code
/// FnPlugin *getPlugins(unsigned int *numPlugins); \endcode
///
/// This is the only externally-visible function that must be implemented by every plug-in library.
/// \param numPlugins Write the number of plug-ins provided by the library here.  The pointer will not be NULL.
/// \return A pointer to an array of plug-in structures, or NULL if none are supported
typedef FnPlugin *(*FnGetPluginsFunc)(unsigned int *numPlugins);

//------------------------------------------------------------------------------

/// \}

#ifdef __cplusplus
}
#endif

#endif // FNPLUGINSYSTEM_H

