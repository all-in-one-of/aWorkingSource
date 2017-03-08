# -*- coding:utf-8 -*-
__author__ = 'chengshun'
__maintainer__ = 'chengshun'

import sys
import os
import traceback
import getpass
import datetime as dt

import MClientAPI
import MTemplateAPI

import production.http_service
reload(production.http_service)
import ConfigParser


def abort(err, session):
    print "Program aborted: " + err.getDescription()
    MClientAPI.Disconnect(session)
    return -1


class MusterSubmit(object):

    def __init__(self, config=None,url='10.0.7.11'):
        self.dispatcher_url = url
        self.username = config["full_name"]
        self.password = config["password"]
        self.department = config["department"]

        print "Connecting to " + self.dispatcher_url

        err = MClientAPI.InitializeLibrary("")
        if err.getErrorCode() != 0:
            abort(err, self.session)

        self.session = MClientAPI.InitializeSession()
        err = MClientAPI.Connect(self.session, self.dispatcher_url, 9781)
        if err.getErrorCode() != 0:
            abort(err, self.session)

        err = MClientAPI.Authenticate(
            self.session, self.username, self.password)
        if err.getErrorCode() != 0:
            abort(err, self.session)

    def sendJobRender(self, job):
        new_job = MClientAPI.MJob()
        new_job.setName(job['name'])
        new_job.setParentId(-1)
        new_job.setDependMode(MClientAPI.MJob.kJobDependSuccess)
        new_job.setJobType(MClientAPI.MJob.kJobTypeJob)
        new_job.setPriority(job.get('priority',100))
        new_job.setTemplateID(950)
        new_job.setIncludedPools(job.get('pools','common'))
        new_job.setPacketSize(job.get('packet_size',1))

        # other things
        new_job.setProject(job.get('proj',''))
        new_job.setDepartment(job.get('department',''))
        new_job.setLogsErrorCheckType(MClientAPI.MJob.kSkipWarnings)

        ''' Template specific attributes '''
        new_job.attributeSetString("job_file", job.get('file',''))
        new_job.attributeSetString("job_project", job.get('shot',''))
        new_job.attributeSetString("output_folder", job.get('outputpath',''))
        new_job.attributeSetFloat("start_frame", float(job.get('f_start',1)))
        new_job.attributeSetFloat("end_frame", float(job.get('f_end',1)))
        new_job.attributeSetFloat("by_frame", float(job.get('by_frame',1)))
        new_job.attributeSetString("render_node", job.get('node',''))
        new_job.setDepends(job.get('dep_id',''))

        err = MClientAPI.ActionSendJob(self.session, new_job)
        if err.getErrorCode() != 0:
            abort(err, self.session)

        job_id = new_job.getJobId()
        self.disconnect()

        return job_id

    def sendJob_lgt(self, job):

        # send job based on user account
        new_job = MClientAPI.MJob()
        new_job.setName(job['name'])
        new_job.setParentId(-1)
        new_job.setDependMode(MClientAPI.MJob.kJobDependSuccess)
        new_job.setJobType(MClientAPI.MJob.kJobTypeJob)

        # set priority and pool
        if self.department == 'LGT':

            if job['isR']:
                new_job.setPriority(5)
                new_job.attributeSetString('eye', 'R')
            else:
                new_job.attributeSetString('eye', 'L')
                if job['resolution'] == 'R1':
                    new_job.setPriority(15)
                elif job['resolution'] == 'R2':
                    new_job.setPriority(25)
                elif job['resolution'] == 'Rough':
                    new_job.setPriority(35)
                elif 'AA' in job['resolution']:
                    new_job.setPriority(40)
                elif 'VR' in job['resolution']:
                    new_job.setPriority(20)
                # no quality jobs
                else:
                    new_job.setPriority(1)

        elif self.department == 'EFX':
            new_job.setPriority(60)

        elif self.department == 'TD':
            new_job.setPriority(65535)

        else:
            new_job.setPriority(1)
        new_job.setMaximumNodes(50)

        # one frame job
        if job['f_start'] == job['f_end']:
            new_job.setPriority(45)

        # fix job
        if job['fix']:
            new_job.setPriority(45)

        new_job.setPacketType(MTemplateAPI.kTemplateMultiframe)
        new_job.setPacketSize(1)
        new_job.setIncludedPools('common')

        # set template id
        if os.getenv('CRYPTOMATTE'):
            new_job.setTemplateID(610)
            print '======Use Katana2.1v1_cymat for rendering======'
        else:
            new_job.setTemplateID(950)
            print '======Use Katana2.1v1 for rendering======'

        # depend id
        if job['dep_id']:
            new_job.setTemplateID(953)
            new_job.setDepends(job['dep_id'])
            new_job.setIncludedPools('farm_cache')

        # other things
        new_job.setProject(job['project'])
        new_job.setDepartment(self.department)
        new_job.setLogsErrorCheckType(MClientAPI.MJob.kSkipWarnings)

        ''' Template specific attributes '''
        new_job.attributeSetString("job_file", job['file'])
        new_job.attributeSetString("job_project", job['project'])
        new_job.attributeSetString("shot_name", job['shot'])
        new_job.attributeSetString("output_folder", job['outputpath'])
        new_job.attributeSetFloat("start_frame", float(job['f_start']))
        new_job.attributeSetFloat("end_frame", float(job['f_end']))
        new_job.attributeSetString("seq_batch", job['seq_batch'])
        new_job.attributeSetString('resolution', job['resolution'])
        if job['isR']:
            new_job.attributeSetString('eye', 'R')
        else:
            new_job.attributeSetString('eye', 'L')
        new_job.attributeSetFloat("by_frame", float(job['f_chunk']))
        new_job.attributeSetString("render_node", job['node'])
        new_job.attributeSetString("env_path", '/mnt/projcache/projects')

        err = MClientAPI.ActionSendJob(self.session, new_job)
        if err.getErrorCode() != 0:
            abort(err, self.session)

        job_id = new_job.getJobId()
        print "Job submitted with ID " + str(job_id) + ". Name: " + job['name']

        return job_id

    def send_prepare_job(self, job):
        new_job = MClientAPI.MJob()
        new_job.setName(job['name'])
        new_job.setParentId(-1)
        new_job.setDependMode(MClientAPI.MJob.kJobDependSuccess)
        new_job.setJobType(MClientAPI.MJob.kJobTypeJob)
        new_job.setIncludedPools(job['cache'])
        new_job.setPriority(100)
        new_job.setTemplateID(952)
        new_job.setProject(job['project'])
        new_job.setDepartment(self.department)
        new_job.setLogsErrorCheckType(MClientAPI.MJob.kSkipWarnings)
        new_job.setMaximumNodes(1)

        new_job.attributeSetString("job_file", job['file'])
        new_job.attributeSetString("cache_node", job['cache'])
        err = MClientAPI.ActionSendJob(self.session, new_job)
        if err.getErrorCode() != 0:
            abort(err, self.session)

        job_id = new_job.getJobId()
        print "Prepare job submitted with ID " + str(job_id) + ". Name: " + job['name']
        return job_id

    def disconnect(self):
        err = MClientAPI.Disconnect(self.session)
        print "\nMuster connection closed..."
