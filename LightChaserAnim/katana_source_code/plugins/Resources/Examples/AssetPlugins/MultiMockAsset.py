# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

"""
This plug-in implements a simple asset management system interface that uses
a specific directory and some conventions in the file system to emulate a
database where assets can be published into and resolved from.
This plug-in extends the PyMockAsset example plug-in implementing a
multi-protocol asset plug-in. In particular it implements a 'sandbox' protocol
that allows users to store assets in a temporary managed folder, outside the
current asset management system, but using compatible asset IDs.

The asset is defined by 4 fields: show, shot, name and version. The
default show and shot can be defined by the environment variables SHOW and
SHOT. A further 'protocol' field can be specified, the current supported values
are:
 - 'asset':   the default asset management system behavior
 - 'sandbox': protocol to publish/load assets to/from the current sandbox
              folder

Any asset can have an unlimited number of optional attributes associated
with it. These attributes are name/value pairs that contain meta-data about
the asset. Notice that attributes are only supported for the 'asset' protocol.

The asset ID is defined in the following ways:
mock://[show]/[shot]/[name]/[version] or
sandbox://[show]/[shot]/[name]/[version]

Example asset IDs:
- mock://bigShow/shot001/myKatanaScene/1
- mock://bigShow/shot001/heroRender/23
- sandbox://bigShow/shot002/propRender/11

"Partial" asset IDs are also allowed. In these cases, the asset ID will
consists of the first N fields (e.g.: mock://bigShow/shot001 - which will
be seen as a shot-wide asset - the asset directory for this would be
[base_dir]/bigShow/shot001).

The base directory is defined by the MOCK_ASSET_DIR environment variable,
or, if this is not set, default to '/tmp'. Asset files will be saved in a
directory structure like: [base_dir]/[show]/[shot]/[version]

The sandbox directory is defined by the MOCK_SANDBOX_DIR environment variable.
If this variable is not set, the default sandbox folder is '/tmp'.
Assets published to the sandbox will be saved to files using the following
convention:

asset ID:  sandbox://[show]/[shot]/[name]/[version]
file name: $MOCK_SANDBOX_DIR/mock_sandbox_[show].[shot].[name].[version]

For example, the asset 'sandbox://bigShow/shot002/propRender/11' will be saved
to '$MOCK_SANDBOX_DIR/mock_sandbox_bigShow.shot002.propRender.11'.

The plug-in is able to resolve asset IDs into the real file paths using
the resolveAsset(), resolveAllAssets() and/or resolvePath() functions
both for 'asset' and 'sandbox' protocols.

When publishing an asset, the asset file(s) will be saved in this directory
together with an extra "asset.info" text file where the metadata will
be stored (this is only valid for assets not published to the sandbox).
For example, for an asset of the type 'image' with attributes
for 'resolution' and 'colorspace', the following text would be saved in
the asset.info file:

---------
image
image_lnf.0001.exr
comment:
resolution:1ksq
colorspace:lnf
published:False
---------

For which each line represents:
  - asset type,
  - file name,
  - comments about the asset
  - colon separated key/value pairs for the attributes,
  - published status

In this mock asset management system, publishing happens before the creation
of the files, which means that the directory will be created when the
pre-asset creation publishing function, createAssetAndPath(), is called.
After Katana has written the assets successfully, postCreateAsset() is
called which in this plug-in will update the published status of the asset
by changing the last line of the asset.info file to:

published:True

Note:
Some asset management systems will publish the asset only after the creation
of the files by Katana (when rendering images, for example), in that case
the bulk of the publishing code will happen in postCreateAsset(). Katana
will need to know the path of the files that are going to be created (for
example, when launching a render for which the files will be published
somewhere on the asset management system), which means that in these cases
createAssetAndPath() will have to return an assetId that can be
resolved into a file path where the files will be saved, even if the
publishing will happen only in postCreateAsset().

"""

import os
import sys
import logging
import AssetAPI
import ConfigurationAPI_cmodule as Configuration

#///////////////////////////////////////////////////////////////////////////////

# Set up a Katana logger
log = logging.getLogger("PyMultiMockAsset")

#///////////////////////////////////////////////////////////////////////////////

_logCalls = os.environ.get("MOCK_DEBUG", False) # Print debug information

class LogCall(object):
    """
    Simple decorator class to log function calls with the decorator "@LogCall".
    """

    def __init__ (self, fn, instance=None):
        self.fn = fn
        self.instance = instance

    def __call__ (self, *args, **kws):
        global _logCalls
        if self.instance is not None:
            msg = "> " + self.fn.__name__ + "("
            for a in args:
                msg += repr(a) + ", "
            msg += ")"
            if _logCalls:
                print msg
            args = (self.instance,) + args
            ret_val = self.fn(*args, **kws)
            if _logCalls:
                print "    :", ret_val
            return ret_val

    def __get__(self, instance, class_):
        if instance is None:
            return self
        return LogCall(self.fn, instance)


#///////////////////////////////////////////////////////////////////////////////

class MultiMockAsset (AssetAPI.BaseAssetPlugin):
    """
    The main class of the plug-in that will be registered as "PyMultiMockAsset". It
    extends AssetAPI.BaseAssetPlugin to create an asset plug-in class and
    implements all abstract functions from the plug-in interface.
    """

    #///////////////////////////////////////////////////////////////////////////

    # Implementation of the abstract functions of the plug-in

    @LogCall
    def createTransaction(self):
        """
        In this example plug-in we do not support transactions yet.
        """
        return None


    @LogCall
    def reset(self):
        """
        Nothing to reset in this plug-in. If, for example, some caching was
        implemented, then this would be the place to clear it.
        """
        pass


    @LogCall
    def getAssetFields(self, assetId, includeDefaults = False):
        """
        Returns a dictionary of fields from a given asset ID.
        """
        # split the result using "/" as the separator
        fieldsList = self._getFieldsStrFromAssetId(assetId).split("/")
        numFields = len(fieldsList)

        # Create the dictionary with the fields
        fields = {}

        # Check string's prefix to determine the current protocol
        if assetId.startswith("sandbox://"):
            fields["protocol"] = "sandbox"
        else:
            fields["protocol"] = "asset"

        if numFields >= 1:
            fields["show"] = fieldsList[0]

        if numFields >= 2:
            fields["shot"] = fieldsList[1]

        if numFields >= 3:
            fields[AssetAPI.kAssetFieldName] = fieldsList[2]

        if numFields == 4:
            fields[AssetAPI.kAssetFieldVersion] = fieldsList[3]

        # Show and shot can have default values if not specified
        if includeDefaults:
            if numFields < 1:
                fields["show"] = self._getDefaultShow()

            if numFields < 2:
                fields["shot"] = self._getDefaultShot()

        return fields


    @LogCall
    def isAssetId(self, string):
        """
        Checks whether a given string is an asset ID. For this simple example,
        the plug-in only checks whether the string starts with "mock://".
        """
        return string.startswith("mock://") or string.startswith("sandbox://")


    @LogCall
    def containsAssetId(self, string):
        """
        Checks whether a given string contains an asset ID. Just like the
        "isAssetId" function, for this simple example, the plug-in only checks
        whether the string contains "mock://".
        """
        return "mock://" in string or "sandbox://" in string


    @LogCall
    def resolveAsset(self, assetId, throwOnError=False):
        """
        Resolve a given asset ID and returns a path to the file. If the asset
        ID cannot be converted into fields or the asset.info file cannot be
        found, "None" is returned.
        """
        if not self.isAssetId(assetId):
            # Return the asset ID as it is if it is not recognized
            log.warning("Asset ID '%s' is not a valid for PyMultiMockAsset. "
                        "Skipping resolving asset." % assetId)
            return assetId

        # Check if the current asset ID represents a sandbox asset and, if so,
        # resolve the sandbox filename
        if self._isSandboxAsset(assetId):
            return self._buildSandboxFilename(assetId)

        assetId = self._replaceTagsWithNumbers(assetId)

        fieldsStr = self._getFieldsStrFromAssetId(assetId)

        if not fieldsStr:
            log.warning("resolveAsset: no fields found for asset ID \"%s\"" %
                        assetId)
            return None

        # Get the directory where the info file and the asset files are stored
        dir = os.path.join(self._getBaseDir(), fieldsStr)

        # Get the asset file from the info file
        info = self._readAssetInfoFile(dir)
        if not info:
            log.warning("resolveAsset: no asset.info file found for \"%s\"" %
                        assetId)
            return None

        # Only prepend dir to the asset file name if it is not an absolute path
        assetFile = info["assetFile"]
        if( not os.path.isabs(assetFile) ):
            assetFile = os.path.join(dir, assetFile )

        return assetFile


    @LogCall
    def resolveAllAssets(self, str):
        """
        For each asset ID found in the string (that is isolated by whitespaces
        before and after), resolve the asset ID using resolveAsset and
        substitute the result in the original.
        """
        result = str

        for token in str.split():
            if self.isAssetId(token):
                path = self.resolveAsset(token)
                result = result.replace(token, path)

        return result


    @LogCall
    def resolvePath(self, assetId, frame):
        """
        Resolves the asset ID and inserts the specified frame using the
        currently selected FileSequence plug-in.
        """
        if self.isAssetId(assetId):
            # Resolve the asset
            resolvedAsset = self.resolveAsset(assetId)
        else:
            resolvedAsset = assetId

        # Get the fileSequence plug-in and if the resolvedAsset is a file
        # sequence path, then use frame to resolve it
        fileSequencePlugin = AssetAPI.GetDefaultFileSequencePlugin()
        if fileSequencePlugin:
            if fileSequencePlugin.isFileSequence(resolvedAsset):
                # Get the FileSequence object and resolve the sequence with
                # the frame passed to this function.
                fileSequence = fileSequencePlugin.getFileSequence(resolvedAsset)
                resolvedAsset = fileSequence.getResolvedPath(frame)

        return resolvedAsset


    @LogCall
    def resolveAssetVersion(self, assetId, versionTag = ""):
        """
        Returns the version for a given asset ID.
        If it is a partial asset ID (which does not have a version) then "None"
        is returned. The only tag implemented here is "latest", which returns
        the highest version found on the versions directory for the asset.
        """
        versionField = None

        # No version tags support for sandbox assets
        if self._isSandboxAsset(assetId):
            return None

        if versionTag:
            versionField = versionTag

        else:
            fields = self.getAssetFields(assetId)
            versionField = fields.get(AssetAPI.kAssetFieldVersion, None)

        version = None
        if versionField == "latest":

            partialAssetId = self.getAssetIdForScope(assetId,
                                                     AssetAPI.kAssetFieldName)

            if not partialAssetId:
                return None

            dir = os.path.join(self._getBaseDir(),
                               self._getFieldsStrFromAssetId(partialAssetId))
            children = os.listdir(dir)

            for child in children:
                if not version or version < int(child):
                    version = int(child)

            version = str(version)

        else:
            version = versionField

        return version


    @LogCall
    def getAssetDisplayName(self, assetId):
        """
        The asset name that will be used as a display name.
        """
        fields = self.getAssetFields(assetId)
        return fields.get(AssetAPI.kAssetFieldName, assetId)


    @LogCall
    def getAssetVersions(self, assetId):
        """
        Returns a list of available versions for a given asset ID.

        This is used for example by the Importomatic Super Tool to display
        a list of versions to choose from.
        """
        # No versions support for sandbox assets
        if self._isSandboxAsset(assetId):
            return tuple()

        # Construct the path to the asset
        partialAssetId = self.getAssetIdForScope(assetId,
                                                 AssetAPI.kAssetFieldName)
        baseDir = self._getBaseDir()
        fieldStr = self._getFieldsStrFromAssetId(partialAssetId)
        assetPath = os.path.join( baseDir, fieldStr )

        # Get hold of all the directories in the one below the asset location
        if os.path.isdir(assetPath):
            VERSIONS = 1
            versions = os.walk(assetPath).next()[VERSIONS]
            versions.sort(reverse=True)
            return tuple(["latest"] + versions)

        # This asset does not exist
        else:
            return tuple()


    @LogCall
    def getUniqueScenegraphLocationFromAssetId(self, assetId,
                                               includeVersion=True):
        """
        Return the "/" separated list of fields (no "mock://" at the
        beginning). This will be enough to uniquely identify the asset.
        """
        fields = self.getAssetFields(assetId)
        if not includeVersion:
            del fields[AssetAPI.kAssetFieldVersion]
        return "/"+self._getFieldsStrFromFields(fields, allowNumeric=False)


    @LogCall
    def getRelatedAssetId(self, assetId, relation):
        """
        Return a related asset ID for a given asset and a given relation.

        In this implementation, the plug-in uses a simple convention to find
        a related asset ID. For args files of a shader with the asset ID
        "mock://Show/Shot/Shader/1", the related args file would be expected to
        to have the asset ID "mock://Show/Shot/Shader_ArgsFile/1".

        The function checks whether this asset exists before returning the
        related asset ID. "None" is returned in any other case.
        """
        # No related-assets support for sandbox assets
        if self._isSandboxAsset(assetId):
            return None

        if relation == AssetAPI.kAssetRelationArgsFile:
            fields = self.getAssetFields(assetId, False)
            argsFileName = "%s_ArgsFile" % fields.get(AssetAPI.kAssetFieldName)
            fields[AssetAPI.kAssetFieldName] = argsFileName
            relatedAssetId = self.buildAssetId(fields)

            # Only return the related asset ID if an asset there exists
            if self.__assetInfoFileExists(relatedAssetId):
                return relatedAssetId

        return None


    @LogCall
    def buildAssetId(self, fields):
        """
        Builds and returns an asset ID using a given dictionary of fields.
        """
        fieldsStr = self._getFieldsStrFromFields(fields)

        if fieldsStr:
            if fields.get("protocol", "") == "sandbox":
                return "sandbox://" + fieldsStr
            else:
                return "mock://" + fieldsStr

        return None


    @LogCall
    def getAssetAttributes(self, assetId, scope = None):
        """
        Return a dictionary of attributes for a given asset.
        """
        # No attributes support for sandbox assets
        if self._isSandboxAsset(assetId):
            return None

        # Replace all tags with version numbers
        assetId = self._replaceTagsWithNumbers(assetId)

        # In this case scope is ignored, since we only
        # have attributes at the version level
        fieldsStr = self._getFieldsStrFromAssetId(assetId)

        if not fieldsStr:
            log.info("getAssetAttributes: retrieving the fields string from "
                     "the AssetID: %s" % assetId)
            return None

        # Get the directory where the info file and the asset files are stored
        dir = os.path.join(self._getBaseDir(), fieldsStr)

        # Get the asset file from the info file
        info = self._readAssetInfoFile(dir)
        if not info:
            log.warning("getAssetAttributes: reading info file for asset ID: "
                        "%s" % assetId)
            return None


        return info["attrs"]


    @LogCall
    def setAssetAttributes(self, assetId, scope = None, attrs = {}):
        """
        Set a dictionary of attributes for a given asset.
        """
        # No attributes support for sandbox assets
        if self._isSandboxAsset(assetId):
            return None

        fieldsStr = self._getFieldsStrFromAssetId(assetId)

        if not fieldsStr:
            log.warning("setAssetAttributes:  retrieving the fields string "
                        "from the asset ID: %s" % assetId)
            return None

        # get the directory where the info file and the asset files are stored
        dir = os.path.join(self._getBaseDir(), fieldsStr)

        # Get the asset file from the info file
        info = self._readAssetInfoFile(dir)

        self._writeAssetInfoFile(dir, info["assetFile"], info["type"], attrs)


    @LogCall
    def getAssetIdForScope(self, assetId, scope):
        """
        Get the fields and construct the asset ID only up to the field
        specified by "scope".
        """
        fields = self.getAssetFields(assetId)

        show      = fields.get("show", None)
        shot      = fields.get("shot", None)
        assetName = fields.get(AssetAPI.kAssetFieldName, None)
        version   = fields.get(AssetAPI.kAssetFieldVersion, None)

        if not show:
            return None

        result = "mock://"
        if fields.get("protocol", "") == "sandbox":
            result = "sandbox://"

        result = result + show
        if scope == "show":
            return result

        if not shot:
            return None

        result = result + "/" + shot
        if scope == "shot":
            return result

        if not assetName:
            return None

        result = result + "/" + assetName
        if scope == AssetAPI.kAssetFieldName:
            return result

        if not version:
            return None

        result = result + "/" + version
        if scope == AssetAPI.kAssetFieldVersion:
            return result

        return None


    @LogCall
    def createAssetAndPath(self, txn, assetType, fields, args, createDirectory):
        """
        Builds an asset ID using the fields provided and prepares to publish
        an asset of a given asset type. This is the first step of publishing
        and Katana will use the resolved value of the asset ID returned by this
        as the location to write out the asset.

        Returns "None" for unsupported asset types or if this step fails (for
        example when the directory structure could not be written to).
        """
        # Create the asset filename and attributes dict.
        assetFile = None

        # For sandbox assets just return the asset ID as there's no support for
        # automatic versioning or asset info
        if fields.get("protocol", "") == "sandbox":
            return self.buildAssetId(fields)

        # The postCreateAsset() call will set "published" to True. We will save
        # "published" as an attribute so that we can save it in the asset.info
        # file in the same way as the other attributes
        attrs = {"published":"False"}
        attrs["comment"] = args.get("comment", "")

        # In some cases Katana will provide "versionUp" as a boolean argument.
        # If it is "True" then we will increase whatever version is provided
        # by the assetId. In a real asset management system this feature could
        # be implemented by increasing the latest available version for the
        # provided asset.
        self._incrementVersion(fields, args)

        # Get the string that contains the fields joined by "/"
        fieldsStr = self._getFieldsStrFromFields(fields)
        if not fieldsStr:
            log.warning("createAssetAndPath(): no fieldsStr! Aborting")
            return None


        # This section will look into the different Asset Types.
        # For most asset types  an argument called "file" can optionally be
        # specified with an explicit file name that will be set in the asset's
        # directory or an absolute path to a file outside the database's
        # directories
        if assetType == AssetAPI.kAssetTypeKatanaScene:
            assetFile = args.get("file", "scene.katana")

        elif assetType == AssetAPI.kAssetTypeImage:
            ext = args.get("ext", "exr")
            colorspace = args.get("colorspace", "lnf")
            res = args.get("res", "misc")

            attrs["ext"] = ext
            attrs["colorspace"] = colorspace
            attrs["res"] = res

            # Images published for the catalog are handled differently.
            if args.get("context") == AssetAPI.kAssetContextCatalog:
                # The filename of the exported sequences is passed to
                # postCreateAsset and handled there.
                assetFile = ""

            else:
                # Now we are going to build the asset filename
                assetFile = None

                # Check if we can get how to represent a file sequence path
                # using the currently selected FileSequence plug-in
                fileSequencePlugin = AssetAPI.GetDefaultFileSequencePlugin()
                if fileSequencePlugin:
                    # Get the prefix and suffix of the sequence string
                    assetFileName = str("image_%s." % colorspace)
                    assetExtension = str(".%s" % ext)

                    assetFile = fileSequencePlugin.buildFileSequenceString(
                                                        assetFileName,
                                                        assetExtension, 4)
                else:
                    log.warning("createAssetAndPath(): No FileSequencePlugin "
                                "found! Aborting.")
                    return None

        elif assetType == AssetAPI.kAssetTypeLookFile:
            assetFile = args.get("file", "lookfile.klf")

        elif assetType == AssetAPI.kAssetTypeFCurveFile:
            assetFile = "fcurve.xml"

        elif assetType == AssetAPI.kAssetTypeGafferRig:
            assetFile = "rig.gprig"

        elif assetType == AssetAPI.kAssetTypeGafferThreeRig:
            assetFile = "rig.rig"

        elif assetType == AssetAPI.kAssetTypeScenegraphXml:
            assetFile = args.get("file", "scene.xml")

        elif assetType == AssetAPI.kAssetTypeScenegraphBookmarks:
            assetFile = args.get("file", "scenegraph_bookmarks.xml")

        elif assetType == AssetAPI.kAssetTypeLookFileMgrSettings:
            assetFile = args.get("file", "lfmsettings.lfmexport")

        elif assetType == AssetAPI.kAssetTypeMacro:
            assetFile = args.get("file", "node.macro")

        elif assetType == AssetAPI.kAssetTypeLiveGroup:
            assetFile = args.get("file", "node.livegroup")

        else:
            log.error("createAssetAndPath(): Invalid asset type: \"%s\""
                      % assetType)
            return None

        # We don't pay attention to 'createDirectory' because we always have to
        # create the dir to keep the info file at least.
        dir = os.path.join(self._getBaseDir(), fieldsStr)
        if not os.path.exists(dir):
            os.makedirs(dir)

        # Create the info file that will keep the filename, asset type and the
        # extra attributes of the asset.
        self._writeAssetInfoFile(dir, assetFile, assetType, attrs)

        resultAssetId = self.buildAssetId(fields)

        return resultAssetId


    @LogCall
    def postCreateAsset(self, txn, assetType, fields, args):
        """
        Builds an asset ID using the fields provided and runs post-publish
        operations for an asset of a given asset type. This is the final step
        of publishing. The fields passed to this function are constructed using
        the asset ID returned by the first publishing step (see
        createAssetAndPath()).

        Returns "None" for if this step fails (for example when an asset ID
        cannot be constructed).
        """
        # Get the string that contains the fields joined by "/"
        fieldsStr = self._getFieldsStrFromFields(fields)
        if not fieldsStr:
            log.warning("postCreateAsset: error in provided fields. Aborting!")
            return None

        # For sandbox assets just return the asset ID as there's no support for
        # automatic versioning or asset info
        if fields.get("protocol", "") == "sandbox":
            return self.buildAssetId(fields)

        # Get the directory where the asset file and info file should be
        dir = os.path.join(self._getBaseDir(), fieldsStr)
        if not os.path.exists(dir):
            log.warning("postCreateAsset: dir doesn't exist: %s  Aborting!"
                        % dir)
            return None

        # Get the type, asset file and attrs from the info file mark
        info = self._readAssetInfoFile(dir)

        type       = info["type"]
        assetFile  = info["assetFile"]
        attrs      = info["attrs"]

        # All good, we will mark the "published" attr to True
        attrs["published"] = "True"

        # For exported catalog items, update asset file name.
        if(assetType == AssetAPI.kAssetTypeImage and
           args.get("context") == AssetAPI.kAssetContextCatalog):
            # When we export frames from the catalog, we do not know the
            # actual filenames until the postCreateAsset step.
            sequenceString = args.get("exportedSequence", None)
            if sequenceString:
                assetFile = sequenceString

        self._writeAssetInfoFile(dir, assetFile, type, attrs)

        return self.buildAssetId(fields)

    @LogCall
    def runAssetPluginCommand(self, assetId, command, commandArgs):
        """
        Allows users to develop custom commands to be run on the their asset
        management system.
        """
        return True

    #///////////////////////////////////////////////////////////////////////////

    # Auxiliary functions

    def __assetInfoFileExists(self, assetId):
        """
        Check if asset and asset.info file exist.
        """
        # No attributes support for sandbox assets
        if self._isSandboxAsset(assetId):
            return None

        fieldsStr = self._getFieldsStrFromAssetId(assetId)
        if not fieldsStr:
            return None

        # Get the directory where the info file and the asset files are stored
        dir = os.path.join(self._getBaseDir(), fieldsStr)
        infoFilePath = self._getAssetInfoFilePath(dir)

        return os.path.exists(infoFilePath)


    def _getAssetInfoFilePath(self, dir):
        """
        Once we know the directory of the asset this function returns the path
        to the metadata file.
        """
        return os.path.join(dir, "asset.info")


    def _writeAssetInfoFile(self, dir, assetFile, assetType, attrs):
        """
        Write out an asset.info file at a given location.
        """
        filePath = self._getAssetInfoFilePath(dir)

        if os.path.exists(filePath):
            os.remove(filePath)

        f = open(filePath, "w")
        f.write(assetType + "\n")
        f.write(assetFile + "\n")
        for key, value in attrs.items():
            f.write("%s:%s\n" % (key, value ) )

        f.close()


    def _readAssetInfoFile(self, dir):
        """
        Read the asset.info file in a given directory.
        """
        filePath = self._getAssetInfoFilePath(dir)

        if not os.path.exists(filePath) :
            log.error("'%s' does not exist" % filePath)
            return None

        f = open(filePath, "r")
        info = f.readlines()

        type = info[0].strip()
        assetFile = info[1].strip()

        attrs = {}
        for attrStr in info[2:] :
            if attrStr:
                tokens = attrStr.split(":")
                attrs[tokens[0].strip()] = tokens[1].strip()

        # Add "type" as an asset attribute so that it can be got hold of using
        # the standard API. This means that it is in two places:
        # info["type"] and info["attrs"]["type"]
        attrs["type"] = type

        return {"type": type, "assetFile": assetFile, "attrs": attrs}


    def _getBaseDir(self):
        """
        Returns the default root directory of the MOCK asset database from the
        "MOCK_ASSET_DIR" environment variable.
        """
        return os.environ.get("MOCK_ASSET_DIR", "/tmp")


    def _getDefaultShow(self):
        """
        Returns the default show from the "SHOW" environment variable.
        """
        return os.environ.get("SHOW", "default_show")


    def _getDefaultShot(self):
        """
        Returns the default shot from the "SHOT" environment variable.
        """
        return os.environ.get("SHOT", "default_shot")


    def _getFieldsStrFromAssetId(self, assetId):
        """
        Removes the "mock://" from the beginning of the asset ID, the result
        is the forward/slash separated fields list (useful to construct the
        directory path of the asset
        """
        if assetId.startswith("mock://"):
            return assetId[len("mock://"):]
        elif assetId.startswith("sandbox://"):
            return assetId[len("sandbox://"):]
        else:
            return ""


    def _getFieldsStrFromFields(self, fields, allowNumeric=True):
        """
        This merges the fields into a forward-slash separated string.
        """
        fieldsStr = ""

        if "show" in fields:
            fieldsStr += fields["show"]

            if "shot" in fields:
                fieldsStr += "/" + fields["shot"]

                if AssetAPI.kAssetFieldName in fields:
                    fieldsStr += "/" + fields.get(AssetAPI.kAssetFieldName)

                    if AssetAPI.kAssetFieldVersion in fields:
                        if allowNumeric:
                            fieldsStr += "/"
                        else:
                            fieldsStr += "/v"

                        fieldsStr += fields.get(AssetAPI.kAssetFieldVersion)

        return fieldsStr


    def _replaceTagsWithNumbers(self, assetId):
        """
        Convert the tags in the asset id in to version numbers.
        """
        # No version tags support for sandbox assets
        if self._isSandboxAsset(assetId):
            return assetId

        version = self.resolveAssetVersion(assetId)
        fields = self.getAssetFields(assetId, False)
        fields[AssetAPI.kAssetFieldVersion] = version
        assetId = self.buildAssetId(fields)
        return assetId


    def _incrementVersion(self, fields, args):
        """
        Increments the version by 1 in a given fields dictionary.
        """
        if "versionUp" in args and args["versionUp"] == "True":
            version = int(fields[AssetAPI.kAssetFieldVersion])
            fields[AssetAPI.kAssetFieldVersion] = str(version + 1)


    def _isSandboxAsset(self, assetId):
        """
        Determines if the given asset ID represents a sandbox asset
        """
        return assetId.startswith("sandbox://")

    def _buildSandboxFilename(self, assetId):
        """
        Builds the sandbox filename for the given asset ID.
        """
        fields = self.getAssetFields(assetId)

        sandboxFolder = os.getenv("MOCK_SANDBOX_DIR", "/tmp")
        keys = ("show", "shot", AssetAPI.kAssetFieldName,
                AssetAPI.kAssetFieldVersion)
        filename = ".".join("%s" % fields.get(k, "") for k in keys)
        filename = "mock_sandbox_" + filename
        filename = os.path.join(sandboxFolder, filename)

        return filename

#///////////////////////////////////////////////////////////////////////////////

# Register the plug-in calling it "PyMultiMockAsset".
# This name will be shown in Katana's Project Settings tab.
AssetAPI.RegisterAssetPlugin("PyMultiMockAsset", MultiMockAsset())
