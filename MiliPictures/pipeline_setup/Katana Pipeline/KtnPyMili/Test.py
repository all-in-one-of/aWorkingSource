import os,sys,json
import KatanaFileBase
import KatanaSceneBuild

def erroRaise(info=None):

	file = open("D:/Katana Projects/Test/SceneBuildErroRaise/erroLog.json","rb")
	erroInfo = json.load(file)
	file.close()
	
	erroMap = {}
	s=sys.exc_info()
	erroMap["File"] = "c_NJLJ"
	erroMap["Error"] = s[1]
	erroMap["Line"] = "Happened on line %d"%s[2].tb_lineno
	erroInfo.append(erroMap) 
	
	file = open("D:/Katana Projects/Test/SceneBuildErroRaise/erroLog.json","wb")
	json.dump(erroInfo, file, indent=3)
	file.close()
def execute():
	try:
		KatanaFileBase.openNew()
		KatanaSceneBuild.execute()
	except:
		erroRaise()