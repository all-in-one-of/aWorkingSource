#!/usr/bin/python
from lxml import etree

root = etree.Element("root")
tree = etree.ElementTree(root)
faceID_child1 = etree.SubElement(root, "faceID")
faceID_child1.set("name", "id")
faceID_child1.set("collection", "2142341")
faceID_child1.set("value", "1")
faceID_child1_faceUV_child1 = etree.SubElement(faceID_child1, "faceUV")
faceID_child1_faceUV_child1.set("name", "data")
faceID_child1_faceUV_child1.set("uv", "0.2,0.5")
faceID_child1_faceUV_child1.set("value", "1.0,0.0,0.0,0.5,0.5,0.5,0.5,0.5,0.5,0.5")
faceID_child1_faceUV_child2 = etree.SubElement(faceID_child1, "faceUV")
faceID_child1_faceUV_child2.set("name", "data")
faceID_child1_faceUV_child2.set("uv", "0.3,0.4")
faceID_child1_faceUV_child2.set("value", "1.0,0.0,0.0,0.5,0.5,0.5,0.5,0.5,0.5,0.5")
faceID_child2 = etree.SubElement(root, "faceID")
faceID_child2.set("name", "id")
faceID_child2.set("collection", "2142341")
faceID_child2.set("value", "55")
faceID_child2_faceUV_child1 = etree.SubElement(faceID_child2, "faceUV")
faceID_child2_faceUV_child1.set("name", "data")
faceID_child2_faceUV_child1.set("uv", "0.3,0.4")
faceID_child2_faceUV_child1.set("value", "1.0,0.0,0.0,0.5,0.5,0.5,0.5,0.5,0.5,0.5")

file_path = '/mnt/public/home/xukai/develop/lc_xgen_bridge/kt_xgen/test.v001.xml' 
tree.write(file_path, pretty_print=True, xml_declaration=True, encoding='utf-8')

class opXml():
    def __init__(self):
        pass
    def writeRoot(self,InPath):
        root = etree.Element("root")
        tree = etree.ElementTree(root)
        child1 = etree.SubElement(root, "project")
        child1.set("name", "CAT")
        child1.set("icon", "logocat.png")
        child1.set("path", "")
        child2 = etree.SubElement(root, "project")
        child2.set("name", "TPR")
        child2.set("icon", "logoteapet.png")
        child2.set("path", "")
        child3 = etree.SubElement(root, "project")
        child3.set("name", "GOD")
        child3.set("icon", "logodoorgod.png")
        child3.set("path", "")
        tree.write(InPath+'shelf_root.xml', pretty_print=True, xml_declaration=True, encoding='utf-8')

    def writeLgt(self,InPath):
        root = etree.Element("root")
        tree = etree.ElementTree(root)
        child1 = etree.SubElement(root, "sub_tool")
        child1.set("label", "download")
        child1.set("icon", "downloadcloud.png")
        child1.set("command", "")
        child2 = etree.SubElement(root, "sub_tool")
        child2.set("label", "upload")
        child2.set("icon", "uploadcloud.png")
        child2.set("command", "")
        child3 = etree.SubElement(root, "sub_tool")
        child3.set("label", "tool_box")
        child3.set("icon", "colorwheel.png")
        child3.set("command", "")
        child4 = etree.SubElement(root, "sub_tool")
        child4.set("label", "rocket")
        child4.set("icon", "rocket.png")
        child4.set("command", "")
        child5 = etree.SubElement(root, "spacing")
        child5.set("value","3")
        child6 = etree.SubElement(root, "sub_tool")
        child6.set("label", "edit")
        child6.set("icon", "compose.png")
        child6.set("command", "")
        tree.write(InPath+'shelf_lgt.xml', pretty_print=True, xml_declaration=True, encoding='utf-8')
    def readRoot(self,InPath):
        result = []
        tree = etree.parse(InPath+"shelf_root.xml")
        root = tree.getroot()
        for node in root:
            #print node.tag
            tmpMap = {}
            name = node.attrib['name']
            icon = node.attrib['icon']
            path = node.attrib['path']
            tmpMap["name"] = name
            tmpMap["icon"] = icon
            tmpMap["path"] = path
            result.append(tmpMap)
        return result
    def readLgt(self,InPath):
        result = []
        tree = etree.parse(InPath+"shelf_lgt.xml")
        root = tree.getroot()
        for node in root:
            if node.tag == "sub_tool":
                tmpMap = {}
                name = node.attrib['label']
                icon = node.attrib['icon']
                command = node.attrib['command']
                tmpMap["label"] = name
                tmpMap["icon"] = icon
                tmpMap["command"] = command
                tmpMap["type"] = node.tag
                result.append(tmpMap)
            elif node.tag == "spacing":
                tmpMap = {}
                name = node.attrib['value']
                tmpMap["value"] = name
                tmpMap["type"] = node.tag
                result.append(tmpMap)
            else:
                "Light Chaser Animation"
        return result
