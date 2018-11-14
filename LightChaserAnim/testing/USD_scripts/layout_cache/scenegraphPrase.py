import os,shutil,sys

sys.path.append('/mnt/utility/lca_sgtk_apps/tk-lca-publish/python/tk_lca_publish/publish_process/ani')
from list_ani_assets import *

class AssetsPrase(StdProcess):
    def __init__(self,dialog):
        StdProcess.__init__(self,dialog)
        self.dialog = dialog
    def getAssetsList(self):
        assetDatas = get_assetDatas()
        print assetDatas
        result = []
        for asset in assetDatas:
            asset_map = {}
            asset_map['type'] = asset.type
            asset_map['name'] = asset.name
            asset_map['namespace'] = asset.namespace
            asset_map['transform'] = asset.transform
            asset_map['constrained'] = str(asset.constrained)
            asset_map['status'] = str(asset.status)
        return result
    def printAssetsList(self):
        assetDatas = get_assetDatas()
        print assetDatas
        for asset in assetDatas:
            print 'type', asset.type
            print 'name', asset.name
            print 'namespace', asset.namespace
            print 'transform', asset.transform
            print 'constrained', str(asset.constrained)
            print 'status', str(asset.status)

class dialog():
    def __init__(self):
        self.version_dir = '/home/xukai/'
        return

d = dialog()

p = AssetsPrase(d)
p.getAssetsList()