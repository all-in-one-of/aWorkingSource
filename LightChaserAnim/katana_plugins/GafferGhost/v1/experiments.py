import Catalog
catalogItem = CatalogManager.CatalogItem.CreateCatalogItemFromID('i_9_0')
layerviews = catalogItem.getLayerViews()
i = 0
for layerview in layerviews:
    i += 1
    print layerview
    buf = catalogItem.getBuffer(layerview)
    filename = "/mnt/public/home/xukai/factory2.0/ExportCatalog/test.%i.exr"%i
    
    buf.writeToDisk(filename, {}, 0, 0)

