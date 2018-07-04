
sys.path.append('/home/xukai/Documents/TestProgramFactory/learning/ExportXML')
import sceneGraphParser.sgXmlParser as sgxml
##########################################################
def exportSimpleXML():
    """
    Export a simple XML file with this function.
    """
    import pymel.core as pm
    shot_name = 'f40140'
    xml_long = '/mnt/public/home/xukai/factory/f40140/layout/%s.long.xml' % shot_name
    xml_short = '/mnt/public/home/xukai/factory/f40140/layout/%s.short.xml' % shot_name
    xml_normal = '/mnt/public/home/xukai/factory/f40140/layout/%s.xml' % shot_name
    l_frames = [1001]
    xml = sgxml.SgXmlParser()
    xml.exportXml(pm.PyNode('|assets'), xml_long,  shot_name + u"_cam", l_frames)
    # ssxml.main(xml_long, xml_short)
    # shutil.copy(xml_short, xml_normal)
##########################################################
exportSimpleXML()