import sys,os
import NodegraphAPI as ngapi
from Katana import KatanaFile
import common.katanaUtils as cku
from xml.etree import ElementTree as ET
import common.lcProdProj as lcpp
import re
import common.proceduralUtils as cpu
import macro.plantCache as mpc
import macro.efxCacheIn as meci
import macro.cfxAssetsIn as mcai
import macro.assIn as mai
import common.asset_xml as cax
import production.pipeline.utils as pplu
from PyQt4.QtCore import *
from PyQt4.QtGui import * 

lctools_env=os.getenv('LCTOOLSET','/mnt/utility/toolset')
sys.path.append(lctools_env+'/applications/katana_v2/Scripts')

def startCheck():
    st_mk = ngapi.GetNode('LgtShotMaker_Lc')
    if st_mk:
        cks = checkShot(st_mk)
        try:
            cks.getCheckInfo()
        except:
            pass
    else:
        return

class infoDialog(QDialog):
    def __init__(self, infoList, header, parent=None):
        QDialog.__init__(self, parent)
        self.setWindowTitle("Need Update")
        self.view = QTableView()

        tableModel = infoTable(self, infoList, header)
        self.view.setModel(tableModel)
        self.view.setShowGrid(False)
        self.view.setSortingEnabled(False)

        self.view.resizeColumnsToContents()
        self.resizeWindowToColumns()
        
        self.layout = QVBoxLayout()
        self.layout.addWidget(self.view)
        self.setLayout(self.layout)

    def resizeWindowToColumns(self):
        frameWidth = self.view.frameWidth() * 2
        vertHeaderWidth = self.view.verticalHeader().width()
        horizHeaderWidth =self.view.horizontalHeader().length()
        vertScrollWidth = self.view.style().pixelMetric(QStyle.PM_ScrollBarExtent) 
        fudgeFactor = 6 
        newWidth = frameWidth + vertHeaderWidth + horizHeaderWidth + vertScrollWidth + fudgeFactor
        if newWidth <= 500:
            self.resize(newWidth, self.height())
        else:
            self.resize(500, self.height())


class infoTable(QAbstractTableModel):
    def __init__(self, parent, infoList, header, *args):
        QAbstractTableModel.__init__(self, parent, *args)
        self.infoList = infoList
        self.header = header

    def rowCount(self, parent):
        return len(self.infoList)
    
    def columnCount(self, parent):
        return len(self.infoList[0])
    
    def data(self, index, role):
        if not index.isValid():
            return None
        elif role != Qt.DisplayRole:
            return None
        return QVariant(self.infoList[index.row()][index.column()])
    
    def headerData(self, col, orientation, role):
        if orientation == Qt.Horizontal and role == Qt.DisplayRole:
            return self.header[col]
        return None
    
    def sort(self, col, order):
        """sort table by given column number col"""
        self.emit(SIGNAL("layoutAboutToBeChanged()"))
        self.infoList = sorted(self.infoList,
            key=operator.itemgetter(col))
        if order == Qt.DescendingOrder:
            self.infoList.reverse()
        self.emit(SIGNAL("layoutChanged()"))

class checkShot:
    def __init__(self, st_mk=None):
        self.st_mk = st_mk
        self.proj = st_mk.getParameter('user.proj').getValue(0)
        self.shot = st_mk.getParameter('user.shot').getValue(0)
        self.ani_xml = self.st_mk.getParameter('user.aniXml').getValue(0)
        self.srf_xml = self.st_mk.getParameter('user.srfXml').getValue(0)
        self.ppInfo=lcpp.lcProdProj()
        self.ppInfo.setProj(self.proj)
        self.resultList=[]

    def getrealAsset(self,assetNameWithNameSP):
        assetWithNum=assetNameWithNameSP.split('.')[-1]
        asset=re.findall('\w+\D+',assetWithNum)[0]
        return asset

    def getAniInfo(self, xml):
        print "Check Ani..."
        if not xml:
            return
        root = ET.parse(xml)
        for asset in root.getiterator('instance'):
            if asset.get('type') == 'reference':
                assetNamespace = asset.get('namespace')
                if not assetNamespace:
                    assetNamespace = asset.get('name')
                cur_ref = asset.get('refFile')
                lat_ref = self.ppInfo.getAniAsset(self.shot, assetNamespace)
                if not lat_ref:
                    assetName = self.getrealAsset(assetNamespace)
                    task_name = 'model'
                    find_task = re.findall('\S+/mod/publish/'+assetName+'\.mod\.(\w+)',cur_ref)
                    if find_task:
                        task_name = find_task[0]
                    lat_ref = self.ppInfo.getModAsset(assetName,task_name=task_name)
                if cur_ref != lat_ref:
                    aniAsset_List=[assetNamespace, self.ppInfo.getPathVersion(cur_ref), self.ppInfo.getPathVersion(lat_ref)]
                    self.resultList.append(aniAsset_List)
        
    def getSrfInfo(self,xml):
        print "Check Srf..."
        if not xml:
            return
        root = ET.parse(xml)
        for asset in root.getiterator('instance'):
            if asset.get('type')=='reference':
                assetNamespace = asset.get('namespace')
                if not assetNamespace:
                    assetNamespace = asset.get('name')
                cur_ref = asset.get('refFile')
                try:
                    assetName = self.getrealAsset(assetNamespace)
                    lat_srf = self.ppInfo.getSrfAsset(assetName)
                    lat_ref = lat_srf['xml']
                except:
                    continue
                if cur_ref != lat_ref:
                    srf_list=[assetNamespace, self.ppInfo.getPathVersion(cur_ref), self.ppInfo.getPathVersion(lat_ref)]
                    self.resultList.append(srf_list)
                    
    def getCfxInfo(self):
        print "Check Cfx..."
        if not self.st_mk:
            return
        maker_nodes = self.st_mk.getChildren()
        cfx_cache_node=None
        for c in maker_nodes:
            if c.getName().startswith('Cfx_AssetsIn_Group_'):
                cfx_cache_node = c
        if not cfx_cache_node:
            return

        fur_nodes=cpu.get_proc_list(cfx_cache_node)
        ass_raw_data=mcai.get_proc_raw_data(fur_nodes)
        need_update=mcai.match_cfx_data(ass_raw_data, cfx_cache_node)

        if not need_update[0]:
            for an in fur_nodes:
                ass_dir=''
                if an.getParameter('user.assDir'):
                    ass_dir=an.getParameter('user.assDir').getValue(0)
                    fur_mode='ass'
                elif an.getParameter('user.furDir'):
                    ass_dir=an.getParameter('user.furDir').getValue(0)
                    fur_mode='hair'
                elif an.getParameter('user.xGenDir'):
                    ass_dir=an.getParameter('user.xGenDir').getValue(0)
                    fur_mode='xgen'

                if need_update[1]:
                    cache_name=pplu.get_asset_name(ass_dir)
                    asset_key=cache_name+'.'+ass_dir.split('/')[-1]
                    if need_update[1].has_key(asset_key):
                        cfx_list=[cache_name, self.ppInfo.getPathVersion(ass_dir), self.ppInfo.getPathVersion(need_update[1][asset_key])]
                        self.resultList.append(cfx_list)

    def getPltInfo(self):
        print "Check Plt..."
        if not self.st_mk:
            return

        plt_try_nodes=cku.get_node_children_by_name(self.st_mk,'Plant_CacheIn_Group_')
        if not plt_try_nodes:
            return
        
        plt_cache_node = plt_try_nodes[0]
        node_contents = mpc.get_node_content(plt_cache_node)
        latest_plt = mpc.get_plant_data(plt_cache_node)
        # latest_plt = self.ppInfo.get_plant_shot(self.shot)

        for plt in latest_plt:
            latest_version = self.ppInfo.getPathVersion(plt['xgen'])
            current_version = ""
            plt_name = plt['asset']
            task_name = plt['task_name']
            if task_name == 'plant':
                name = plt_name+'_'+task_name
            else:
                name = task_name

            for p in node_contents:
                if p['asset_name'] == plt_name:
                    current_version = self.ppInfo.getPathVersion(p['file_path'])
            
            if current_version != latest_version:
                pltList = [name, current_version, latest_version]
                self.resultList.append(pltList)

    def getEfxInfo(self):
        print "Check Efx..."
        if not self.st_mk:
            return

        efx_cache_node,efx_handler,efx_stuff,task_node_dict = self.__get_efx_type_data('efx')

        if not efx_stuff.get('abc') and not efx_stuff.get('vdb'):
            return

        latest_efx = self.ppInfo.lcGetEfxAssets(self.shot,step='efx')
        latest_efx_data=self.ppInfo.analyzeFXAssets(latest_efx)

        for k,v in latest_efx_data.items():
            if not efx_handler.raw_data.has_key(k):
                print v.get('abc')
                print v.get('vdb')
                continue
            if v.get('abc')!=efx_handler.raw_data[k].get('abc'):                
                for abc in v.get('abc'):
                    name = abc.split('/')[-2]
                    cur_ver = ''

                    for a in efx_handler.raw_data[k].get('abc'):
                        if name in a:
                            cur_ver = self.ppInfo.getPathVersion(a)
                    lat_ver = self.ppInfo.getPathVersion(abc)

                efx_list = [name, cur_ver, lat_ver]
                self.resultList.append(efx_list)

                print v.get('abc')
                print efx_handler.raw_data[k].get('abc')

            if v.get('vdb')!=efx_handler.raw_data[k].get('vdb'):
                for vdb in v.get('vdb'):
                    name = vdb.split('/')[-2]
                    cur_ver = ''
                    for a in efx_handler.raw_data[k].get('vdb'):
                        if name in a:
                            cur_ver = elf.ppInfo.getPathVersion(a)
                    lat_ver = self.ppInfo.getPathVersion(vdb)
                
                efx_list = [name, cur_ver, lat_ver]
                self.resultList.append(efx_list)

                print v.get('vdb')
                print efx_handler.raw_data[k].get('vdb')


    def getLayInfo(self):
        if not self.st_mk:
            return

        maker_nodes = self.st_mk.getChildren()
        cam_path=''
        cam_katana_node=None
        for mn in maker_nodes:
            if 'CamInAbc_' in mn.getName():
                cam_path=mn.getParameter('user.abcAsset').getValue(0)
                cam_katana_node=mn
                break

        xml_data=self.ppInfo.lcGetLayXml(self.shot)
        assetxml = cax.AssetXml(self.ani_xml, xml_context='')
        layxml = assetxml.s_sourceXml
        
        print "Check Lay..."
        if layxml and xml_data:
            if layxml != xml_data[-1]:
                cur_lay = self.ppInfo.getPathVersion(layxml)
                latest_lay = self.ppInfo.getPathVersion(xml_data[-1])

                lay_list = ['lay', cur_lay, latest_lay]
                self.resultList.append(lay_list)
                print cur_lay
                print latest_lay

        print "Check Cam..."
        if cam_path:          
            new_cam_path=self.ppInfo.lcGetLayCam(self.shot)
            if cam_path!=new_cam_path:
                cam_list = ['camera', self.ppInfo.getPathVersion(cam_path), self.ppInfo.getPathVersion(new_cam_path)]
                self.resultList.append(cam_list)

                print cam_path
                print new_cam_path

    def getSetInfo(self):
        if not self.st_mk:
            return
        print "Check Set..."
        katana_file=self.st_mk.getParameter('user.katanaFile').getValue(0)    
        if '/lgt/task/katana/set_cache' not in katana_file:
            return

        file_sp=katana_file.split('/')
        set_katana_folder=file_sp[file_sp.index('set_cache')+1].split('.')
        
        if len(set_katana_folder)!=4:
            return

        set_version=set_katana_folder[2]
        set_file_path=self.ppInfo.getShotPathAtDep(self.shot,'set')+\
                        '/publish/'+self.shot+'.set.set_dressing.'+set_version+'/'+self.shot+'.katana'
        latest_set = self.ppInfo.getLatestSet(self.shot)
        
        if set_file_path!=latest_set[0]:
            set_list = ['set', self.ppInfo.getPathVersion(set_file_path), self.ppInfo.getPathVersion(latest_set[0])]
            self.resultList.append(set_list)

            print set_file_path
            print latest_set[0]

    def get_task_name(self, path):
        path_sp = path.split('/')
        for p in path_sp:
            task_name = re.findall('\S+\.[a-z]{3}\.(\w+)\.v\d{3}$', p)
            if task_name:
                return task_name[0]
        return ''

    def __get_efx_type_data(self,fx_type='efx'):
        try_nodes=None
        #get crd cache in node in shotmaker
        if fx_type=='crd':
            try_nodes=cku.get_node_children_by_name(self.st_mk, 'Crd_CacheIn_Group_\d*')
        elif fx_type=='efx':
            try_nodes=cku.get_node_children_by_name(self.st_mk, 'Efx_CacheIn_Group_\d*')
            
        if not try_nodes:
            return

        cache_node=try_nodes[0]

        handler = meci.EfxCache(cache_node)
        task_node_dict=handler.get_tasknode_dict()
        data_stuff= handler.get_obj_data()
        handler.analyze_data(data_stuff)

        return cache_node,handler,data_stuff,task_node_dict

    def getCheckInfo(self):
        self.getAniInfo(self.ani_xml)
        self.getSrfInfo(self.srf_xml)
        self.getCfxInfo()
        self.getPltInfo()
        self.getEfxInfo()
        self.getLayInfo()
        self.getSetInfo()

        if self.resultList:
            print "Need update!"
            header=['Name', 'Cur_ref', 'Lat_ref']
            w = infoDialog(self.resultList, header)
            w.show()
            w.exec_()
        else:
            print "Check Success!"


