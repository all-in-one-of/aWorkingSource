from Katana import CatalogManager

def main():
	length =  CatalogManager.GetLastCatalogItem(slot=None).getIndex()
	catalogs = []
	for i in range(length+1):
	    if i == 0:
	        next = CatalogManager.GetFirstCatalogItem(slot=None)
	    else:
	        next = CatalogManager.GetNextCatalogItem(next, slot=None)
	    catalogs.append(next)
	CatalogManager.DeleteCatalogItems(catalogs)
	print '[LCA Logs]: Free image memory is completed!'
