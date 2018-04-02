# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

import logging
import xml.etree.ElementTree as etree

log = logging.getLogger("CastingSheet")

#//////////////////////////////////////////////////////////////////////////////
#///
def IsCastingSheet(filepath):
    xmlTree = etree.parse(filepath)
    
    # Get the casting sheet root
    #
    xmlCastingSheet = xmlTree.getroot()
    
    # Ensure that the casting sheet has the correct tag
    #
    if xmlCastingSheet.tag == "castingsheet":
        return True
    
    return False

#//////////////////////////////////////////////////////////////////////////////
#///
def Iterator(filepath):
    """
    For iterating over a casting sheet.
    Returns a tuple for each casting sheet entry.
    The tuple contains an assetid and an instance name.
    
    A casting sheet entry looks like this:
    
    <castingsheet>
        <entry assetid="" name=""/>
    </castingsheet>
    
    and this will be returned on each iteration
    
    (id, name)
    """
    
    # Parse the xml file, check it's  well formed etc
    #
    xmlTree = etree.parse(filepath)
    
    # Get the casting sheet root
    #
    xmlCastingSheet = xmlTree.getroot()
    
    # Ensure that the casting sheet has the correct tag
    #
    if xmlCastingSheet.tag != "castingsheet":
        log.error("The given casting sheet '%s' does not have a root element with a 'castingsheet' tag." % filepath)
        raise StopIteration()

    # Step through each casting sheet entry
    # and get the information needed out of it
    #
    else:
        # Pull all the entries out
        #
        entries = xmlCastingSheet.findall("entry")
        
        if not entries:
            log.warning("There are no entries in this casting sheet")
            
        # Go through each entry in the casting sheet and get hold of the asset ids
        # and the entry names
        #
        else:
            for entry in entries:
                
                # Get the asset information
                #
                id = entry.attrib["assetid"]
                name = entry.attrib["name"]
                
                # 
                yield (id, name)
