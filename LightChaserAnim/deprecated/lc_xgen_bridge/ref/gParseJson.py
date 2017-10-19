#!/usr/bin/python
import json
out_path = "/mnt/public/home/xukai/develop/lc_xgen_bridge/kt_xgen/test.1.json"
data = {"kt_value":1.0}
file = open(out_path, "w")
json.dump(data, file, indent=3)
file.close()
