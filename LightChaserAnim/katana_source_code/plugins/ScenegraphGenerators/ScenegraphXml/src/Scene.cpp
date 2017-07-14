// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <Scene.h>

#include <boost/filesystem.hpp>

#include <FnLogging/FnLogging.h>


namespace instance_scene
{
    FnLogSetup("ScenegraphXml")

    std::vector<Scene*> Scene::_allScenes;

Scene::Scene(const std::string & filepath)
    : m_filepath(filepath)
{
    _allScenes.push_back(this);

    ptree root = PTreeHelper::parse( filepath );

    if ( !root.empty() )
    {
        // check ScenegraphXML version
        if ( PTreeHelper::hasAttr( root, "version" ) )
        {
            std::string versionString = PTreeHelper::getAttr( root, "version" );
            size_t dotPos1 = versionString.find(".");
            if (dotPos1!=std::string::npos)
            {
                size_t dotPos2 = versionString.find(".", dotPos1+1);
                if (dotPos2!=std::string::npos)
                {
                    int v0 = atoi(versionString.substr(0, dotPos1).c_str());
                    int v1 = atoi(versionString.substr(dotPos1+1, dotPos2-dotPos1-1).c_str());
                    int v2 = atoi(versionString.substr(dotPos2+1).c_str());

                    if (v0 != SGXML_VERSION_0 || v1 != SGXML_VERSION_1 || v2 != SGXML_VERSION_2)
                    {
                        FnLogWarn("Version of ScenegraphXML file \""
                                  << filepath << "\" (" << versionString
                                  << ") does not match ScenegraphXML reader "
                                  << "version ("
                                  << SGXML_VERSION_0 << "."
                                  << SGXML_VERSION_1 << "."
                                  << SGXML_VERSION_2 << ").")
                    }
                }
            }
        }

        // get basedir from filepath
        std::string basedir = filepath;

        // remove any trailing '/' characters
        while (basedir.at(basedir.length()-1) == '/')
        {
            basedir = basedir.substr(0, basedir.length()-1);
        }

        // get the portion of filename after the final '/' character
        // also keep the portion up to and including the final / character as basedir
        size_t lastSlashPos = basedir.rfind("/");
        if (lastSlashPos != std::string::npos)
        {
            basedir = basedir.substr(0, lastSlashPos+1);
        }

        // before we parse other elements, check if channel data is specifies and
        // make channels to hand to other elements.

        for ( ptree::const_iterator iter = root.begin(); iter != root.end(); ++iter )
        {
            std::string const & type = (*iter).first;
            ptree const & child = (*iter).second;

            if( type == "channelData" )
            {
                int startFrame = 0;
                int endFrame = 0;

                if ( PTreeHelper::hasAttr( child, "startFrame" ) && PTreeHelper::hasAttr( child, "endFrame" ) )
                {
                    startFrame = atoi( PTreeHelper::getAttr( child, "startFrame" ).c_str() );
                    endFrame = atoi( PTreeHelper::getAttr( child, "endFrame" ).c_str() );
                }
                else
                {
                    // silent for now. Should catch error that startFrame and
                    // endFrame not specified in channelData
                }

                // If the channel data element specifies a base path for names
                // of XML files of channel data, explicitly use that, otherwise
                // construct the base path for channel data files from the file
                // path of the main XML file
                std::string basepath;
                if (PTreeHelper::hasAttr(child, "ref")
                        || PTreeHelper::hasAttr(child, "basepath"))
                {
                    // Prefer "ref" over "basepath", as "ref" is used by the
                    // ScenegraphXML Python scripts in EXTRAS/ScenegraphXML
                    std::string attrName;
                    if (PTreeHelper::hasAttr(child, "ref"))
                    {
                        attrName = "ref";
                    }
                    else
                    {
                        attrName = "basepath";
                    }
                    basepath = PTreeHelper::getAttr(child, attrName);

                    // Check if the given base path for names of XML files of
                    // channel data is relative, and make it absolute by adding
                    // the base directory at its start if so
#ifdef _WIN32
                    if (boost::filesystem::path(basepath).is_relative())
#else
                    if (!boost::filesystem::path(basepath).is_complete())
#endif
                    {
                        boost::filesystem::path absoluteBasePath (basedir);
                        absoluteBasePath /= basepath;
                        basepath = absoluteBasePath.string();
                    }
                }
                else
                {
                    basepath = filepath;
                    size_t found = basepath.find(".xml");
                    basepath = basepath.substr(0, found);
                }

                m_channels = ChannelRangeRcPtr(new ChannelRange(basepath, startFrame, endFrame));
            }
        }

        // create an instance of a group node and use that to parse the root instanceList
        // to create a dummy group element at the root level. This is then used to create
        m_instanceList = InstanceGroupRcPtr(new InstanceGroup(root, m_channels, basedir));
    }
}

namespace
{
typedef std::map<std::string, SceneRcPtr> SceneMap;
SceneMap g_cache;
}

SceneRcPtr Scene::loadCached(const std::string & filepath)
{
    SceneMap::iterator I = g_cache.find(filepath);

    if (I != g_cache.end())
    {
        return (*I).second;
    }
    else
    {
        try
        {
            SceneRcPtr scene = SceneRcPtr(new Scene(filepath));
            g_cache[filepath] = scene;
            return scene;
        }
        catch(...)
        {
            return SceneRcPtr();
        }
    }
}

void Scene::flushCache()
{
    g_cache.clear();
}


InstanceGroupRcPtr Scene::getInstanceList()
{
    return m_instanceList;
}

std::string Scene::getFilePath()
{
    return m_filepath;
}

std::string Scene::getErrorMessages()
{
    return m_errorReport.getErrorMessages();
}

std::string Scene::getWarningMessages()
{
    return m_errorReport.getWarningMessages();
}

bool Scene::hasErrors()
{
    return m_errorReport.hasErrors();
}

bool Scene::hasWarnings()
{
    return m_errorReport.hasWarnings();
}

}
