from pxr import Usd,UsdGeom,Kind

stage=Usd.Stage.Open("/mnt/proj/software/develop/usd/usd_test_data/usda_test/usd/flo_test.usda")
for x in stage.Traverse():
	print x
prim = stage.GetPrimAtPath("/scn/asb_AD2_USD")
prim.GetReferences().AppendReference("/mnt/proj/software/develop/usd/usd_test_data/usda_test/usd/asb.usda")
stage.GetRootLayer().Save()
