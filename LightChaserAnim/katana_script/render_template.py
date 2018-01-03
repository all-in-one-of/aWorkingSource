''' Import Muster base modules '''
import MClientAPI
import MTemplateAPI
''' Import Python base lib modules '''
import os.path

import re
import shutil

import sys
import subprocess
import traceback

''' Get the template manager singleton '''
manager = MTemplateAPI.MManager.getUniqueClass()

class KatanaTemplate(MTemplateAPI.MTemplate):
    """
    Define a new class derived from MTemplateAPI.MTemplate base class , override required methods
    """

    def __init__(self):
        MTemplateAPI.MTemplate.__init__(self)
        self.setID(962)
        self.setName("Katana2.5v5")
        self.setDescription("Katana render")
        self.setTemplateLogic(MTemplateAPI.kTemplateMultiframe)
        self.setDefaultPriority(1)
        self.setDefaultPools("")
        self.setDefaultExcludedPools("")
        self.setMaximumLicenses(0)
        self.setLicensesGroup("Katana-licensing")
        self.setDelayGroup("Katana")
        self.setDelay(0)

        ''' Submission form items allocation '''
        katanaFilename = MTemplateAPI.MTemplateItemFile("KATFILENAME","Scene file","Specifies the Katana scene file to render","",0,1,1,"*.*")
        katanaStartFrame = MTemplateAPI.MTemplateItemInteger("KATSTARTFRAME","Start frame","Specifies the starting frame for the rendering job",1,0,0,1)
        katanaEndFrame = MTemplateAPI.MTemplateItemInteger("KATENDFRAME","End frame","Specifies the ending frame for the rendering job",1,0,0,1)
        katanaByFrame = MTemplateAPI.MTemplateItemInteger("KATBYFRAME","By frame","Specifies the ending frame for the rendering job",1,0,0,1)
        katanaNode = MTemplateAPI.MTemplateItemString("KATNODE","By node","Specifies the node for the rendering job","",0,0,1)
        katanaNotes = MTemplateAPI.MTemplateItemString("JOBNOTES","Job notes","Specifies the node for the rendering job","",0,0,1)

        self.addSubmissionItem(katanaFilename)
        self.addSubmissionItem(katanaStartFrame)
        self.addSubmissionItem(katanaEndFrame)
        self.addSubmissionItem(katanaByFrame)
        self.addSubmissionItem(katanaNode)
        self.addSubmissionItem(katanaNotes)

        ''' items mapping to Muster default tags '''
        self.addMapping("KATSTARTFRAME","start_frame")
        self.addMapping("KATENDFRAME","end_frame")
        self.addMapping("KATBYFRAME","by_frame")
        self.addMapping("KATFILENAME","job_file")
        self.addMapping("KATNODE","render_node")
        self.addMapping("JOBNOTES","job_notes")
        self.addMapping("OUTPUTFOLDER",'output_foler')

        ''' Linux support '''
        self.platformLinux.setPlatformEnabled(1)
        self.platformLinux.setEnableErrorCheck(1)
        self.platformLinux.setEnabledByDefault(1)
        self.platformLinux.setFramesFloating(0)
        self.platformLinux.setDetectionLogic(MTemplateAPI.kProcessDirect)
        startingFolderPath = MTemplateAPI.MTemplateItemFolder("KATHOME","Katana root folder","Path to Katana executable root folder","/mnt/utility/linked_tools/lca_rez/launchers/pws/linux",0,0,1)
        executablePath = MTemplateAPI.MTemplateItemFolder("KATAPP","Katana batch executable","Path to Katana executable","/mnt/utility/linked_tools/lca_rez/launchers/pws/linux/katana",0,0,1)
        self.platformLinux.addClientConfigurationItem(startingFolderPath)
        self.platformLinux.addClientConfigurationItem(executablePath)

    ''' virtual functions overrides '''
    def onBuildCommandLine(self, platform, job, chunk, clientTemplatePreferences,instanceNum):
        renderCmd = "--batch --threads3d=0 " + job.attributeGetString("ADD_FLAGS")
        renderCmd += " -t " + '%.0f' % chunk.getStartFrame()
        renderCmd += "-" + '%.0f' % chunk.getEndFrame()
        renderCmd += " --katana-file \"" + job.attributeGetString(manager.resolveMappingToJob(self.getID(),"KATFILENAME")) + "\""
        renderCmd += " --render-node \"" + job.attributeGetString(manager.resolveMappingToJob(self.getID(),"KATNODE")) + "\""
        return renderCmd

    def onBuildEnvironment(self, job, chunk, clientTemplatePreferences, existingEnvironment):
        environment = existingEnvironment
        platform = MClientAPI.GetPlatform()
        if platform == MClientAPI.kPlatformLinux:
            environment.setValue("peregrinel_LICENSE", '5059@10.0.0.5')
            # environment.setValue("LC_PROJ", '/mnt/proj')
            # environment.setValue("LC_PROJ_PATH", '/mnt/proj/projects')
            # environment.setValue("LC_PROJ_PATH_A", '/mnt/proj/projects')
            # environment.setValue("LC_WORK", '/mnt/work')
            # environment.setValue("LC_WORK_PATH", '/mnt/work/projects')
            # environment.setValue("LC_OUTPUT", '/mnt/output')
            # environment.setValue("LC_OUTPUT_PATH", '/mnt/output/projects')
            # environment.setValue("LC_PRODUCTION_LIB", '/mnt/work/projects/lib/publish')

            # environment.setValue("LC_UTILITY", '/mnt/utility')
            # environment.setValue("LC_TOOLSET", '/mnt/utility/toolset')
            # environment.setValue("LCTOOLSET", '/mnt/utility/toolset')

            # environment.setValue("LCA_SOFTWARE_PATH", '/mnt/work/software')
            # environment.setValue("LCA_DEVELOP_PATH", '/mnt/work/software/develop')

            # environment.setValue("PATH", '/mnt/work/software/rez/linux/bin/rez')
            # environment.setValue("REZ_CONFIG_FILE", '/mnt/utility/linked_tools/lca_rez/settings/rezconfig.py')
        return environment

    def onGetApplicationPath(self,job,clientTemplatePreferences,pathOut ):
        pathOut.setString(clientTemplatePreferences['KATAPP'])
        return MTemplateAPI.MTemplateError()

    def onGetApplicationStartingFolder(self,job,clientTemplatePreferences,pathOut):
        pathOut.setString(clientTemplatePreferences['KATHOME'])
        return MTemplateAPI.MTemplateError()

    def onCheckLog(self, job, chunk, clientTemplatePreferences, log):
        if log.find('Katana Crash Report') != -1:
            error = MTemplateAPI.MTemplateError(
                1, 'Katana Crash Report', MTemplateAPI.MTemplateError.kTemplateErrorTypeError)
            error.addErrorAction(MTemplateAPI.MTemplateError.kTemplateErrorAbortRender)
            return error
        if log.find('Invalid memory reference') != -1:
            error = MTemplateAPI.MTemplateError(
                1, 'Invalid memory reference', MTemplateAPI.MTemplateError.kTemplateErrorTypeError)
            error.addErrorAction(MTemplateAPI.MTemplateError.kTemplateErrorAbortRender)
            return error
        return MTemplateAPI.MTemplateError()

    def onCheckExitCode(self,job,chunk,clientTemplatePreferences,exitCode):
        if exitCode != 0:
            return MTemplateAPI.MTemplateError(exitCode, "Exit code %d different from expected 0 value" % exitCode,MTemplateAPI.MTemplateError.kTemplateErrorTypeError)
        return MTemplateAPI.MTemplateError()

    def onCheckExitCode(self, job, chunk, clientTemplatePreferences, exitCode):
        if exitCode != 0:
            return MTemplateAPI.MTemplateError(exitCode, "Exit code %d different from expected 0 value" % exitCode, MTemplateAPI.MTemplateError.kTemplateErrorTypeError)
        return MTemplateAPI.MTemplateError()

    def onJobEnd(self, job):
        # give rights to modify output folder
        name = job.getName()
        try:
            outputfolder = job.attributeGetString('output_folder')
            if os.path.exists(outputfolder):
                cmd = 'chmod 777 ' + outputfolder
                os.system(cmd)
        except:
            pass

        username = name.split(" ")[2]
        id = job.getJobId()
        msg = "\'" + name + ", id: " + str(id) + ", is finished." + "\'"
        p = subprocess.Popen("/usr/bin/python " + "/mnt/utility/toolset/lib/production/farm_notify.py " +
                             username + " " + msg,  shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        t = p.communicate()

        return MTemplateAPI.MTemplateError()


''' Create an instance of the template class and install it into the template manager context '''
template = KatanaTemplate()
manager.installPyTemplate(template)