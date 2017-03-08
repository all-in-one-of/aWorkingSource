__author__ = 'chengshun'
__maintainer__ = 'chengshun'

import sys, os

sys.path.append('/mnt/utility/toolset/lib/production')
from shotgun_connection import Connection

sys.path.insert(0, '/usr/lib64/python2.6')
import xml.etree.ElementTree as ET

print ET
sg = Connection('get_project_info').get_sg()

filters = [
    ['project', 'name_is', 'cat'],
    ['entity', 'type_is', 'Shot'],
    ['entity', 'name_not_contains', 'z'],
    ['sg_status_list', 'in', ['aa', 'aaa', 'sc', 'ip', 'da']],
    ['content', 'is', 'hair'],
]

fields = [
    'assets',
    'entity.Shot.code'
]

tasks = sg.find('Task', filters, fields)

count = 0
for task in tasks:
    if task['entity.Shot.code'] in ['k50680', 'k50690', 'c50440', 'c50480']:
        continue
    filters = [
        ['project', 'name_is', 'cat'],
        ['entity.Shot.code', 'is', task['entity.Shot.code']],
        ['sg_task', 'name_is', 'hair']
    ]
    fields = ['sg_version_folder']
    orders = [{'field_name': 'code', 'direction': 'desc'}]
    r = sg.find_one('Version', filters, fields, orders)
    if r:
        xml = r['sg_version_folder'][
                  'local_path_linux'] + '/cache/blanket/hair/tanzi_pgYetiMaya_huzi01/tanzi_pgYetiMaya_huzi01.xml'
        if os.path.exists(xml):
            tree = ET.parse(xml)
            root = tree.getroot()
            try:
                version = int(root.findall('imageSearchPath')[0].text.split('/')[-2].split('.')[-1][2:])
                if version > 30:
                    aaa = 1
                else:
                    count += 1
                    print task['entity.Shot.code']
                    print r['sg_version_folder']['local_path_linux']
                    print str(version) + '    !!!!!!!!'

                    print '\n'
            except:
                pass
print count
