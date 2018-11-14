#!/usr/bin/python
import os

from production.shotgun_connection import Connection
from argparse import ArgumentParser

def argsChaser():
    parser = ArgumentParser(usage='\t Batch resize EXR file.')
    parser.add_argument("-o", "--open", metavar='DIR/', action='store', default="", help="")
    parser.add_argument("-c", "--check", type=bool, nargs='?',action='store',
                                        const=True, default=False,metavar="Bool",
                                        help="")
    args=None
    temp_args,unknow = parser.parse_known_args()
    args=vars(temp_args)
    return args

sg = Connection('get_project_info').get_sg()

shots = ( "p90315",
          "p90290",
          "p90125",
          "p40730",
          "p40925",
          "p40080",
          "r40100",
          "r40140",
          "r40125",
          "p40075",
          "m90267",
          "m90241",
          "m90273",
          "m90277",
          "m90380",
          "m90590",
          "m90536",
          "k90010", 
          "k90020", 
          "k90030")
def doseR1Exist(shot, task):
    # frame_range = getShotFramerange(shot)
    output_folder = os.path.join("/","output", "projects", "pws", "shot")
    sequence = shot[0:3]
    # if task == "katana":
    task_folder = os.path.join(output_folder, sequence, shot, "lgt", "output", task) 
    for folder in os.listdir(task_folder):
         folder_path = os.path.join(task_folder, folder)
         layer_folders = os.listdir(folder_path)
         for layer in layer_folders:
            layer_path = os.path.join(folder_path, layer)
            r1_path = os.path.join(layer_path, "R1")
            if os.path.isdir(r1_path):
                return_info = "shot: %s, path: %s"%(shot, task_folder)
                return return_info
    # target_folders = os.path.join(task_folder, sorted(os.listdir(task_folder))[-1])
    # r1_output = os.path.join(target_folder, "")
    # print r1_output
def openFolder(shot, task = None):
    output_folder = os.path.join("/","output", "projects", "pws", "shot")
    sequence = shot[0:3]
    # if task == "katana":
    task_folder = os.path.join(output_folder, sequence, shot, "lgt", "output", task) 
    os.system('xdg-open "%s"' % task_folder)

def getShotFramerange(shot):
    projFilter = [ ['name', 'is', "PWS"] ]
    shotgunProjInfo = sg.find_one('Project', projFilter)

    shotFilter = [
        ['code', 'is', shot],
        ['project', 'is', {'type': 'Project', 'id': shotgunProjInfo['id']}]
        ]
    info = sg.find_one('Shot', shotFilter, ['sg_sequence', 'sg_cut_in', 'sg_cut_out','sg_head_in', 'sg_tail_out'])

    if info:
        frame_range=[info['sg_cut_in'],info['sg_cut_out']]
        return frame_range



args = argsChaser()
if args["open"]:
    openFolder(args["open"])
if args["check"]:
    for shot in shots:
        if(doseR1Exist(shot, "katana")):
            print doseR1Exist(shot, "katana")


