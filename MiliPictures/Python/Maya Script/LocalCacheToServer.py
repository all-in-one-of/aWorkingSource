#coding:utf-8
__author__ = 'kai.xu:this guy is bloody cool'

import time,os,sys
import maya.cmds as cmds
import Mili.Module.utilties.lsrunas as lsrunas
import Mili.Module.utilties.Get as Get
import Mili.Module.utilties.Path as Path
reload(lsrunas)
reload(Get)
reload(Path)


class LocalCacheToServer(object):
    def __init__(self,shot):
        self.shot = shot
        exportTime = time.time()
        tmpTime = time.localtime(exportTime)
        self.dateStr = '%0.4d_%0.2d_%0.2d'%(tmpTime.tm_year, tmpTime.tm_mon, tmpTime.tm_mday)
        self.localPath = Path.Path().GetLocalPath()+'/'+Get.Get().GetActiveProject()+'/FarmBatch/'+self.dateStr+'/'+self.shot+'/'
        self.serverPath = lsrunas.lsrunas().Mili_Back_Root+'/'+Get.Get().GetActiveProject()+'/FarmBatch/'+self.dateStr+'/'+self.shot+'/'

    def run(self):
        self.lsrunasPlus(self.serverPath,onlyCreate=True)
        cacheInfoList = self.findCacheNode()
        newCacheInfoList = self.addSpecifiedCacheInfo(cacheInfoList)
        for cacheInfo in newCacheInfoList:
            localPath = cacheInfo['path']+'/'
            serverPath = self.serverPath + cacheInfo['plug']
            fileList = os.listdir(localPath)
            for filename in fileList:
                """找到名字是cacheInfo['name']的文件夹，不管后缀，统统传上去"""
                key = filename.split('.')[0]
                if key == cacheInfo['name']:
                    self.lsrunasPlus(serverPath,localPath,filename)
        self.relinkCachePath(newCacheInfoList)


    def relinkCachePath(self,newCacheInfoList):
        '''
        将maya中的节点指向服务器
        '''
        for cacheInfo in newCacheInfoList:
            cacheNode = cacheInfo['node']
            serverPath = self.serverPath + cacheInfo['plug']
            cmds.setAttr(cacheNode+'.cachePath',serverPath,type='string')


    def lsrunasPlus(self,ServerPath,LocalPath=None,filename=None,onlyCreate=False):
        '''
        创建文件路径，上传文件的方法函数
        '''
        uploadTime = time.time()
        if LocalPath==None or os.path.isdir(LocalPath):
            pass
        else:
            os.makedirs(LocalPath)
        if not os.path.isdir(os.path.dirname(ServerPath)):
            lsrunas.lsrunas().CreateDir_1(os.path.dirname(ServerPath))
        if onlyCreate == False:
            lsrunas.lsrunas(uploadTime).UploadFile(LocalPath+filename,ServerPath+filename,DeleteSource=False)


    def findCacheNode(self):
        '''
        找到所有的缓存节点，获取缓存的名字，生成列表
                并把缓存的地址刷到服务器上
        '''
        cacheInfoList = []
        cacheNodeList = cmds.ls(type='cacheFile')
        for cacheNode in cacheNodeList:
            cacheName = cmds.getAttr(cacheNode+'.cacheName')
            cachePath = cmds.getAttr(cacheNode+'.cachePath')
            if cachePath != None:
                #~ 如果cachePath为空，说明这个是垃圾文件，跳过
                map = {'name':cacheName,'path':cachePath,'node':cacheNode}
                cacheInfoList.append(map)
        return cacheInfoList

    def addSpecifiedCacheInfo(self,cacheInfoList):
        '''
        检测是否有同名的缓存，如果有，在上层文件夹名字上做区分
                如果没有，用缓存名字做上层文件夹名字
        '''
        newCacheInfoList = cacheInfoList
        """获取所有的cache的文件名"""
        cacheNameList = []
        for cacheNode in newCacheInfoList:
            cacheName = cacheNode['name']
            cacheNameList.append(cacheName)
        id = 0 #~ 用于区别同名的缓存文件
        """遍历所有的文件名，对路径一一操作"""
        for cacheName in cacheNameList:
            """测试文件是否有同名的情况"""
            test = 0
            for tmp in cacheNameList:
                if cacheName == tmp:
                    test += 1
            #~ 没有命名冲突的缓存文件，在原路径上加一个以文件名命名的文件夹
            if test == 1:
                for cacheNode in newCacheInfoList:
                    cachePath = cacheNode['path']
                    if cacheNode['name'] == cacheName:
                        cacheNode['plug'] = '/'+ cacheName +'/'
            #~ 对于有命名冲突的文件，在路径上加一个特殊的文件夹
            else:
                for cacheNode in newCacheInfoList:
                    cachePath = cacheNode['path']
                    if cacheNode['name'] == cacheName:
                        specifiedName = cacheName+'_mark%s'%id
                        id += 1
                        cacheNode['plug'] = '/'+ specifiedName +'/'
        return newCacheInfoList

if __name__ == '__main__':
    lcts = LocalCacheToServer('CUT000_test')
    lcts.run()