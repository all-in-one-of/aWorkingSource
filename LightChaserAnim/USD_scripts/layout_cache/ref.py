# -*- coding:utf-8 -*-

############################################
#
# Copyright (c) 2013 Light Chaser Animation
#
# Author: XiangQuan
#
# Date: 2015.08
#
# Description:
#
############################################

import os
import sys
import traceback
import shutil
from xml.dom.minidom import Document, parse

import pymel.core as pm
import cfx.lcaCfxCache.assetdata as ad
import production.CacheUtils.FilterCacheObjects as cf
reload(cf)
import maya.cmds as cmds

def getConstrainAssemblyReference():
    all_top_ars = list()
    if cmds.objExists('assets'):
        for grp in cmds.listRelatives('assets',c=1,f=1):
            try:
                for ar in cmds.listRelatives(grp,c=1,f=1):
                    if cmds.nodeType(ar) == 'assemblyReference':
                        all_top_ars.append(ar)
            except:
                pass
            
    # all_arChildren = [j for i in all_top_ars for j in cmds.listRelatives(i,ad=1,f=1)]
    all_arChildren = list()
    for top in all_top_ars:
        if cmds.listRelatives(top,ad=1,f=1):
            all_arChildren.extend(cmds.listRelatives(top,ad=1,f=1))
    
    constrainType=['pointConstraint', 'pointOnPolyConstraint', 'aimConstraint', 'orientConstraint', 'parentConstraint', 'scaleConstraint', 'normalConstraint', 'tangentConstraint', 'geometryConstraint']
    noRefConstrains = [i for i in cmds.ls(type=constrainType,l=1) if not cmds.referenceQuery(i,inr=1) and i in all_arChildren]

    all_user_constrains = list()
    for ar in all_top_ars:
        for con in noRefConstrains:
            if not cmds.container(ar,findContainer=con,q=1):
                all_user_constrains.append(con)

    all_connectARs = list()

    for i in all_user_constrains:
        parent = cmds.listRelatives(i,p=1,f=1)
        while cmds.nodeType(parent) != 'assemblyReference':
            parent = cmds.listRelatives(parent,p=1,f=1)
        else:
            all_connectARs.extend(parent)

    connectASBs = list()
    ASB_childARs = list()
    for ar in all_connectARs:
        children_ars = cmds.listRelatives(ar, type='assemblyReference', ad=1, f=1)
        if children_ars is None:
            pass
        else:
            ASB_childARs.extend(children_ars)
            connectASBs.append(ar)

    new_connectARs = [i for i in list(set([i for i in (all_connectARs+ASB_childARs) if i not in connectASBs]))]

    return new_connectARs


def is_asb(ar):
    path = cmds.getAttr(ar + '.definition')
    if 'asb' in path.split('/') or 'scn' in path.split('/'):
        return True
    else:
        return False

def getConstrainAssemblyReferenceNew():
    all_top_ars = list()
    if cmds.objExists('assets'):
        for grp in cmds.listRelatives('assets', c=1, f=1):
            try:
                for ar in cmds.listRelatives(grp, c=1, f=1):
                    if cmds.nodeType(ar) == 'assemblyReference':
                        all_top_ars.append(ar)
            except:
                pass
    all_parent_cons = list()
    connects_ars = list()
    for top_ar in all_top_ars:
        all_constraints = cmds.listRelatives(top_ar, ad=True, type='constraint')
        if all_constraints:
            for constraint in all_constraints:
                parent_con = cmds.listRelatives(constraint, p=1, f=1)[0]
                all_parent_cons.append(parent_con)
                parents = cmds.listRelatives(constraint, p=1, f=1)[0].split('|')
                parents.reverse()
                for p in parents:
                    if p.endswith('_AR'):
                        if cmds.objExists(p) and cmds.nodeType(p) == 'assemblyReference':
                            if not is_asb(p):
                                connects_ars.append(p)
                                continue
    for parent_con in all_parent_cons:
        ars = cmds.listRelatives(parent_con, ad=1, f=1, type='assemblyReference')
        if ars:
            connects_ars.extend(ars)
    
    all_connects = list()
    for ar in list(set(connects_ars)):
        if is_asb(ar):
            act = cmds.assembly(ar, q=1, a=1)
            if act.endswith('.abc'):
                acts = cmds.assembly(ar, q=1, lr=1)
                for a in acts:
                    if a.endswith('.ma'):
                        cmds.assembly(ar, e=1, a=a)
                asb_c = cmds.listRelatives(ar, ad=1, type='assemblyReference')
                all_connects.extend(asb_c)
        else:
            all_connects.append(ar)
    
    all_connects_ars = list()
    for ar in all_connects:
        full = pm.PyNode(ar).fullPath()
        all_connects_ars.append(full)
    return all_connects_ars


def get_assetDatas():
    """
    assetDatas: list assets which will be written in ani_assets.xml later
    """
    dialog = StdProcess(None)
    #list all constrain assembly reference
    l_constrained = getConstrainAssemblyReferenceNew()
    
    #check traditional references
    assetDatas = dialog.list_ref_assets()
    #check assembly references
    assemblies = dialog.get_all_assemblies()
    scene_assemblies = []
    gpuCache_assemblies = []
    for assem in assemblies:
        if not dialog.is_bottom_assemblyReference(assem):
            continue
        crnt_repr = dialog.get_current_representation(assem)
        if dialog.is_scene(assem, crnt_repr):
            scene_assemblies.append(assem)
        elif dialog.is_gpuCache(crnt_repr):
            gpuCache_assemblies.append(assem)

    # # l_constrained = cf.constrainedAssets(assemblies)
    # l_constrained = getConstrainAssemblyReferenceNew()
    
    #check if a gpuCached assembly reference node has any constraint, 
    #or any real animation(any of its animCurves has more than one keyframe)
    for gpuCache_assembly in gpuCache_assemblies:
        if gpuCache_assembly.fullPath() in l_constrained:
            constraint = True
            dialog.append_to_list(gpuCache_assembly, str(constraint), 'constrained', assetDatas)
        elif cf.isAnimation(gpuCache_assembly):
            constraint = False
            dialog.append_to_list(gpuCache_assembly, str(constraint), 'animated', assetDatas)
        #else:
            #pass
    
    #a scene assembly reference is going to be added to the list anyhow, 
    #we only need to check its constraint status
    for scene_assembly in scene_assemblies:
        if not dialog.is_scene_meshShown(scene_assembly):
            continue
        
        constraint = False
        if scene_assembly.fullPath() in l_constrained:
            constraint = True
            status = 'constrained'
        elif cf.isAnimation(scene_assembly):
            status = 'animated'
        else:
            status = 'edited'
        
        dialog.append_to_list(scene_assembly, str(constraint), status, assetDatas)
        
    return assetDatas


# All publish process will use StdProcess as the class name.
class StdProcess():

    def __init__(self, dialog):
        self.dialog = dialog
        self.process_name = u"记录带动画的资产"
        self.description = u"将镜头的起始结束帧和带动画的资产记录到版本文件夹内一个文本文件中，为自动Cache做准备。"
        return

    def proceed(self):
        try:
            
            #write result to ani_assets.xml, should consider whether the file exists or not, 
            #cause this script will work with list_ani_assets.py, which may create the xml first
            if hasattr(self.dialog, 'assetDatas'):
                self.write_to_xml(self.dialog.assetDatas)
            else:
                assetDatas = get_assetDatas()
                self.write_to_xml(assetDatas)
            
            return ""
        except:
            return traceback.format_exc()
            
    def list_ref_assets(self):
        '''
        comes from list_ani_assets.py, check traditional reference nodes
        '''
        l_assets = []
        asset_trans = pm.ls('master', recursive=True, referencedNodes=True)
        l_constrained = cf.constrainedAssets(asset_trans)

        for trans in asset_trans:
            l_sons = pm.listRelatives(trans, c=True, pa=True)
            for son in l_sons:
                if not son.name().endswith(':poly'):
                    continue

                #asset_name = trans.name().split(":")[-2]
                asset_name = '.'.join( trans.name().split(":")[:-1] )
                #print asset_name
                if pm.referenceQuery(trans, isNodeReferenced=True):
                    ref_path = pm.referenceQuery(trans, filename=True)
                    tokens = ref_path.split('/')
                    if 'asset' in tokens:
                        i = tokens.index('asset')
                        asset_type = tokens[i+1]
                        ref_asset_name = tokens[i+2]
                        asset_src = tokens[i+3]
                    else:
                        asset_type = '-'
                        ref_asset_name = '-'
                        asset_src = '-'
                        #self.d_asset[asset_name] = {'type':tokens[i+1], 'src':tokens[i+3]}
                else:
                    asset_type = '-'
                    ref_asset_name = '-'
                    asset_src = '-'
                    #AssetData(namespace='', type='',  name='',        step='',   transform=None)
                if trans.fullPath() in l_constrained or trans.fullPath().startswith("|assets|asb"):
                    status = 'constrained'
                elif cf.isAnimation(trans):
                    status = 'animated'
                else:
                    status = 'edited'

                asset_data = ad.AssetData(asset_name, asset_type, ref_asset_name, asset_src, trans, status)
                if status == 'constrained':
                    asset_data.constrained = True
                l_assets.append(asset_data)
        
        return l_assets
        
    def write_to_xml(self, assetDatas):
        '''
        write result to ani_assets.xml, should consider whether the file exists or not
        if the xml file exists, write after old info; else create a new file
        TODO: does not consider the situation that the existing file cannot be parsed, like it is an empty file
        '''
        cut_in = pm.animation.playbackOptions(q=True, minTime=True)
        cut_out = pm.animation.playbackOptions(q=True, maxTime=True)
        
        xml_path = self.dialog.version_dir + '/ani_assets.xml'

        dom = Document()
        root_dom = dom.createElement('anim')
        root_dom.setAttribute('start', str(int(cut_in)) )
        root_dom.setAttribute('end', str(int(cut_out)) )
        dom.appendChild(root_dom)
        
        for asset in assetDatas:
            dom_elem = dom.createElement('asset')
            dom_elem.setAttribute('type', asset.type)
            dom_elem.setAttribute('name', asset.name)
            dom_elem.setAttribute('namespace', asset.namespace)
            dom_elem.setAttribute('transform', asset.transform)
            dom_elem.setAttribute('constrained', str(asset.constrained))
            dom_elem.setAttribute('status', str(asset.status))
            if asset.type == 'chr':
                if pm.attributeQuery('asm_info', n=asset.transform, ex=1):
                    asm_info = pm.PyNode(asset.transform).asm_info.get()
                    dom_elem.setAttribute('asm_info', str(asm_info))
            
            root_dom.appendChild(dom_elem)
            
        #just formatting output xml
        pretty_text = dom.toprettyxml(indent = '    ',  newl = '')
        final_text = pretty_text.replace('        ',  '    ')
        final_text = final_text.replace('>    <',  '>\n    <')
        final_text = final_text.replace('><', '>\n<')
        f = open( xml_path, 'w')
        f.write(final_text)
        f.close()
        return


    def append_to_list(self, assem_node, constraint, status, data_list):
        '''
        '''
        assetData = self.create_assetData(assem_node)
        assetData.constrained = constraint
        assetData.flag = True
        assetData.status = status
        #import maya.cmds as cmds
        #assetData.type = cmds.ls(str(assem_node), long = True)[0].split('|')[2]
        data_list.append(assetData)
    
    def create_assetData(self, assem_node):
        '''
        create ad.AssetData instance, and add constrained attribute
        AssetData(namespace='', type='',  name='',        step='',   transform=None)
        '''

        assem_splits = assem_node.split(':')
        real_namespace = assem_splits[:-1]
        ar_prefix_ns = assem_splits[-1].replace('_AR', '')
        real_namespace.append(ar_prefix_ns)
        namespace = '.'.join(real_namespace)
        assem_def_path = pm.getAttr(assem_node + '.definition').replace('\\', '/')
        def_path_tokens = assem_def_path.split('/')
        if 'asset' in def_path_tokens:
            i = def_path_tokens.index('asset')
            asset_type = def_path_tokens[i+1]          #read type from reference path
            asset_name = def_path_tokens[i+2]
            asset_step = def_path_tokens[i+3]
        else:
            asset_type = '-'
            asset_name = '-'
            asset_step = '-'
            
        return ad.AssetData(namespace, asset_type, asset_name, asset_step, assem_node)

    
    def get_all_assemblies(self, start_node = ''):
        '''
        list all assemblyReference nodes under start_node
        '''
        if start_node == '':
            assemblies = pm.ls(type = 'assemblyReference')
        else:
            assemblies = pm.listRelatives(start_node, allDescendents = True, type = 'assemblyReference')
        
        return assemblies
    
    def get_current_representation(self, assembly_name):
        '''
        '''
        return pm.assembly(assembly_name, query = True, active = True)
    
    def is_scene(self, assem, assem_repr):
        '''
        when representation is a scene, handle it like what 'list_ani_assets.py' does:
        only check constraints under 'poly' hierarchy
        '''
        if assem_repr.endswith('.ma') or assem_repr.endswith('.mb'):
            assem_children = pm.listRelatives(assem, c=True, pa=True)
            if not assem_children:
                return False
            
            master = [assem_c for assem_c in assem_children if str(assem_c).endswith('master')][0]
            if not master:
                return False
            
            children = pm.listRelatives(master, c=True, pa=True)
            if not children:
                return False
            
            has_poly = False
            for child in children:
                if  child.endswith(':poly'):
                    has_poly = True
                    break
            return has_poly         #if repr is ma or mb, and hierarchy has poly, then it is a scene
        else:
            return False
        
    def is_scene_meshShown(self, assem):
        '''
        '''
        #cause is_scene check happens first, master must exists
        assem_children = pm.listRelatives(assem, c=True, pa=True)
        master = [assem_c for assem_c in assem_children if str(assem_c).endswith('master')][0]
        
        namespace = master.rsplit(':', 1)[0]
        hi_level = namespace+':hi'
        if not pm.objExists(hi_level):
            return False
        
        # we only count on meshes in hi group
        trans = pm.listRelatives(hi_level, allDescendents = True, path = True, type = 'transform')
        for t in trans:
            if pm.getAttr(t + '.visibility'):
                meshes = pm.listRelatives(t, allDescendents = True, path = True, type = ['mesh', 'subdiv', 'nurbsSurface'])
                if meshes:
                    return True
        
        return False
    
    def is_gpuCache(self, assem_repr):
        if assem_repr.endswith('.abc'):
            return True
        else:
            return False
    
    def is_locator(self, assem_repr):
        if assem_repr.endswith('.locator'):
            return True
        else:
            return False
        
    def is_bottom_assemblyReference(self, assem):
        is_bottom = False
        sub_assem = pm.listRelatives(assem, type = 'assemblyReference', allDescendents = True)
        if not sub_assem:
            is_bottom = True
        return is_bottom
    
    
    def is_transform_attrs_locked(self, node):
        is_locked = True
        locked_transformation = []
        locked_transformation.append(pm.getAttr(node + '.translateX', lock = True))
        locked_transformation.append(pm.getAttr(node + '.translateY', lock = True))
        locked_transformation.append(pm.getAttr(node + '.translateZ', lock = True))
        
        locked_transformation.append(pm.getAttr(node + '.rotateX', lock = True))
        locked_transformation.append(pm.getAttr(node + '.rotateX', lock = True))
        locked_transformation.append(pm.getAttr(node + '.rotateX', lock = True))
        
        locked_transformation.append(pm.getAttr(node + '.scaleX', lock = True))
        locked_transformation.append(pm.getAttr(node + '.scaleY', lock = True))
        locked_transformation.append(pm.getAttr(node + '.scaleZ', lock = True))
             
        if False in locked_transformation:
            is_locked = False
            
        return is_locked
        
        

    
    def get_process_name(self):
        return self.process_name
    
    def get_description(self):
        return self.description

class dialog():
    def __init__(self):
        self.version_dir = '/home/xukai/'
        return

d = dialog()

p = StdProcess(d)
p.proceed()



