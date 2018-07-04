from pxr import Usd,UsdGeom

stage=Usd.Stage.Open('/mnt/proj/software/develop/usd/usd_test_data/usda_test/usd/flo_test.usda')
for x in stage.Traverse():
	print x
prim=stage.GetPrimAtPath("/scn/asb_AD2_USD")
UsdGeom.XformCommonAPI(prim).SetTranslate([0,-10,0])
UsdGeom.XformCommonAPI(prim).SetRotate([0,45,0])
stage.GetRootLayer().Save()
