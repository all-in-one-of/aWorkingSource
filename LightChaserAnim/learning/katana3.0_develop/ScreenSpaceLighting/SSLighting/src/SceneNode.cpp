
#ifdef _WIN32
    #include <FnPlatform/Windows.h>
#endif
#include "SceneNode.h"
#include "pystring/pystring.h"
#include <iostream>

Imath::M44d SceneNode::globalWorldMatrix;

SceneNode::SceneNode(std::string locationPath, bool proxy) :
    m_dirty(true),
    m_active(true),
    m_mesh(0x0),
    m_locationPath(locationPath),
    m_parent(0x0),
    m_proxy(proxy)
{
}

SceneNode::~SceneNode()
{
    removeAllChildren();
    delete m_mesh;
    m_mesh = 0x0;
}

bool SceneNode::preDraw()
{
    if(!isActive())
    {
        return false;
    }

    if(isDirty())
    {
        // Setup node geometry
        if(m_locationAttributes.isValid())
        {
            // Replace the existing mesh entirely
            if(m_mesh)
            {
                delete m_mesh;
            }
            m_mesh = new DrawableMesh();
            m_mesh->setDrawAttribute("meshAttrs", m_locationAttributes);
        }
        else if(m_mesh)
        {
            delete m_mesh;
            m_mesh = 0x0;
        }

        setDirty(false);
    }

    globalWorldMatrix = m_worldMatrix * globalWorldMatrix;

    return true;
}

void SceneNode::draw(FnAttribute::DoubleAttribute projectionMatrix,
                     FnAttribute::DoubleAttribute viewMatrix)
{
    if(m_mesh && m_mesh->isValid())
    {
        m_mesh->setDrawAttribute("viewMatrix", viewMatrix);
        m_mesh->setDrawAttribute("projectionMatrix", projectionMatrix);

        FnAttribute::DoubleAttribute worldMtxAttr(
            globalWorldMatrix.getValue(),16, 4);
        m_mesh->setDrawAttribute("worldMatrix", worldMtxAttr);

        m_mesh->draw();
    }
}

void SceneNode::drawChildren(FnAttribute::DoubleAttribute projectionMatrix,
                             FnAttribute::DoubleAttribute viewMatrix)
{
    // Draw child nodes
    NodeList::iterator it;
    for(it = m_children.begin(); it != m_children.end(); ++it)
    {
        if((*it)->preDraw())
        {
            (*it)->draw(projectionMatrix, viewMatrix);
            (*it)->drawChildren(projectionMatrix, viewMatrix);
            (*it)->postDraw();
        }
    }
}

void SceneNode::pickerDraw(
    ViewportLayer& layer,
    GLShaderProgram& pickingShader,
    FnAttribute::DoubleAttribute projectionMatrix,
    FnAttribute::DoubleAttribute viewMatrix,
    std::set<std::string>& ignoreLocations)
{
    if(m_mesh && m_mesh->isValid())
    {
        // Use the proxy root node as selectable if this node has proxy geometry
        SceneNode* selectableNode = this;
        while (selectableNode != 0x0 && selectableNode->m_proxy)
        {
            selectableNode = selectableNode->getParent();
        }
        if (selectableNode == 0x0)
        {
            // This should never happen
            return;
        }

        // Get the pickable location path
        std::string locationPath = selectableNode->getLocationPath();
        if (ignoreLocations.find(locationPath) != ignoreLocations.end())
        {
            // If this location is meant to be ignored, then don't render it
            return;
        }

        // Build a location entry attribute:
        // The convention for a location is a GroupAttribute with a child
        // called "location" and the value must be a StringAttribute containing
        // the location path in its value. This will be recognized by
        // SelectionLayer.
        FnAttribute::GroupAttribute locationAttr =
            FnAttribute::GroupBuilder()
                .set("location", FnAttribute::StringAttribute(locationPath))
                .build();

        // Add the pickable object
        FnPickId pickId = layer.addPickableObject(locationAttr);

        // Get the color for this ID (see FnPickingTypes.h)
        Vec4f color;
        pickIdToColor(pickId, color);
        float colorValues[4] = {color.x, color.y, color.z, color.w};
        FnAttribute::FloatAttribute colorAttr(colorValues, 4, 4);

        // Set the mesh attrs        
        m_mesh->setDrawAttribute("viewMatrix", viewMatrix);
        m_mesh->setDrawAttribute("projectionMatrix", projectionMatrix);
        FnAttribute::DoubleAttribute worldMtxAttr(
            globalWorldMatrix.getValue(),16, 4);
        m_mesh->setDrawAttribute("worldMatrix", worldMtxAttr);
        m_mesh->setDrawAttribute("color", colorAttr);
        
        // Render
        m_mesh->pickerDraw(pickingShader);
    }
}

void SceneNode::pickerDrawChildren(
    ViewportLayer& layer,
    GLShaderProgram& pickingShader,
    FnAttribute::DoubleAttribute projectionMatrix,
    FnAttribute::DoubleAttribute viewMatrix,
    std::set<std::string>& ignoreLocations)
{
    // Draw child nodes
    NodeList::iterator it;
    for(it = m_children.begin(); it != m_children.end(); ++it)
    {
        if((*it)->preDraw())
        {
            (*it)->pickerDraw(
                layer,
                pickingShader,
                projectionMatrix,
                viewMatrix,
                ignoreLocations);
            (*it)->pickerDrawChildren(
                layer,
                pickingShader,
                projectionMatrix,
                viewMatrix,
                ignoreLocations);
            (*it)->postDraw();
        }
    }
}

void SceneNode::postDraw()
{
    globalWorldMatrix = m_worldMatrix.inverse() * globalWorldMatrix;
}

void SceneNode::addChild(const std::string& name, SceneNode* child)
{
    std::vector<std::string> pathTokens;

    pystring::split(pystring::lstrip(name, "/"), pathTokens, "/", 1);

    if(pathTokens.size() > 1)
    {
        int i = 0;
        std::vector<std::string>::iterator it;
        for(it = m_nodeNames.begin(); it != m_nodeNames.end(); ++it, ++i)
        {
            if(*it == pathTokens[0])
            {
                m_children[i]->addChild(pathTokens[1], child);
                return;
            }
        }
    }
    else
    {
        // Ensure that any existing child with the same name
        // is removed
        removeChild(pathTokens[0]);
        m_children.push_back(child);
        m_nodeNames.push_back(pathTokens[0]);

        child->m_parent = this;
    }
}

void SceneNode::removeChild(const std::string& name)
{
    std::vector<std::string> pathTokens;

    pystring::split(pystring::lstrip(name, "/"), pathTokens, "/", 1);

    int i=0;
    std::vector<std::string>::iterator it;
    for(it = m_nodeNames.begin(); it != m_nodeNames.end(); ++it, ++i)
    {
        if(*it == pathTokens[0])
        {
            if(pathTokens.size() > 1)
            {
                m_children[i]->removeChild(pathTokens[1]);
            }
            else
            {
                delete m_children[i];

                m_children.erase(m_children.begin() + i);
                m_nodeNames.erase(it);
            }
            return;
        }
    }
}

void SceneNode::removeAllChildren()
{
    NodeList::iterator it;
    for(it = m_children.begin(); it != m_children.end(); ++it)
    {
        delete *it;
    }

    m_children.clear();
    m_nodeNames.clear();
}

void SceneNode::setDrawable(DrawableMesh* mesh)
{
    if(m_mesh)
    {
        delete m_mesh;
    }

    m_mesh = mesh;
}

void SceneNode::setAttributes(FnAttribute::GroupAttribute attr)
{
    m_locationAttributes = attr;

    // Test if something needs an update here (eg mesh reload)
    // then set the dirty flag accordingly
    setDirty(true);
}

void SceneNode::setXform(Imath::M44d xform)
{
    m_worldMatrix = xform;
    setDirty(true);
}


SceneNode* SceneNode::getChild(const std::string& name)
{
    std::vector<std::string> pathTokens;

    pystring::split(pystring::lstrip(name, "/"), pathTokens, "/", 1);

    // Find child matching the first token
    int i=0;
    std::vector<std::string>::iterator it;
    for(it = m_nodeNames.begin(); it != m_nodeNames.end(); ++it, ++i)
    {
        if(*it == pathTokens[0])
        {
            if(pathTokens.size() > 1)
            {
                return m_children[i]->getChild(pathTokens[1]);
            }
            else
            {
                return m_children[i];
            }
        }
    }

    return 0x0;
}

void SceneNode::invalidate()
{
    if(m_mesh)
    {
        delete m_mesh;
        m_mesh = 0x0;
        setDirty(true);
    }

    invalidateChildren();
}

void SceneNode::invalidateChildren()
{
    NodeList::iterator it;
    for(it = m_children.begin(); it != m_children.end(); ++it)
    {
        (*it)->invalidate();
    }
}

void SceneNode::clearGLResources()
{
    NodeList::iterator it;
    for (it = m_children.begin(); it != m_children.end(); ++it)
    {
        (*it)->clearGLResources();
    }

    if (m_mesh)
    {
        m_mesh->clearGLResources();
    }

    setDirty(true);
}
