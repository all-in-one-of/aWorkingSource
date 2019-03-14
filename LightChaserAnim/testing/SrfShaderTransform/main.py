import glob,os
from multiprocessing.dummy import Pool as ThreadPool

__script = os.path.join(os.path.dirname(__file__), 'script.py')
__logs = os.path.join(os.path.dirname(__file__), 'logs.json')

def getSrfFiles(proj, dept):
    result = []
    asset_dirs = glob.glob("/mnt/work/projects/%s/asset/*/*/%s/task/katana/"%(proj, dept))
    for asset_dir in asset_dirs:
        asset_files = os.listdir(asset_dir)
        versions = []
        for asset_file in asset_files:
            #~ skip folders
            if os.path.isdir(os.path.join(asset_dir, asset_file)):
                continue
            #~ skip invild katana files
            if asset_file.find(".srf.surfacing.") < 0:
                continue
            #~ skip default katana files
            if asset_file.find(".md.katana") >= 0:
                continue
            #~ skip default v000 katana files
            if asset_file.find(".srf.surfacing.v000.") >= 0:
                continue
            versions.append(asset_file)
        #~ return the last version of file
        if len(versions) >= 1:
            result.append(os.path.join(asset_dir, sorted(versions)[-1]))
    return result

def excuted(command):
    os.system(command)

def writeSrfJsonDataOut(katana_files):
    # katana_files = getSrfFiles("nza", "srf")
    #~ build tasks 
    tasks = []
    for file in katana_files:
        print "####", file
        katana_exec = "/mnt/utility/linked_tools/lca_rez/launchers/nza/linux/katana_classic"
        command = "%s --script %s -f %s -l %s -m %s"%(katana_exec, __script, file, __logs, "write")
        tasks.append(command)
        # excuted(command)
    #~ make the Pool of workers
    threads = 4
    pool = ThreadPool(int(threads)) 
    results = pool.map(excuted, tasks)


def recoverSrfJsonData(katana_files):
    # katana_files = getSrfFiles("nza", "srf")
    #~ build tasks 
    tasks = []
    for file in katana_files:
        print "####", file
        katana_exec = "/mnt/utility/linked_tools/lca_rez/launchers/nza/linux/katana3"
        command = "%s --script %s -f %s -l %s -m %s"%(katana_exec, __script, file, __logs, "recover")
        # tasks.append(command)
        excuted(command)
    #~ make the Pool of workers
    # threads = 4
    # pool = ThreadPool(int(threads)) 
    # results = pool.map(excuted, tasks)

def check(katana_files):
    # katana_files = getSrfFiles("nza", "srf")
    #~ build tasks 
    tasks = []
    for file in katana_files:
        print "####", file
        katana_exec = "/mnt/utility/linked_tools/lca_rez/launchers/nza/linux/katana3"
        command = "%s --script %s -f %s -l %s -m %s"%(katana_exec, __script, file, __logs, "check")
        # tasks.append(command)
        excuted(command)

if __name__ == "__main__":
    if os.path.isfile(__logs):
        os.remove(__logs)
    # get all NZA srf asset file
    # katana_files = getSrfFiles("nza", "srf")
    katana_files = [
        '/mnt/work/projects/pws/asset/prp/turret_tower_a/srf/task/katana/turret_tower_a.srf.surfacing.v003.katana',
    ]
    writeSrfJsonDataOut(katana_files)
    recoverSrfJsonData(katana_files)