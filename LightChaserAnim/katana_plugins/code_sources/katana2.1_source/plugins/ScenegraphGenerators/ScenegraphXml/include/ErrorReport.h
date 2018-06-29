// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef INCLUDED_INSTANCE_SCENE_ERRORREPORT_H
#define INCLUDED_INSTANCE_SCENE_ERRORREPORT_H

#include "Manifest.h"

namespace instance_scene
{

    class ErrorReport
    {
    public:
        ErrorReport();
        std::string getErrorMessages();
        std::string getWarningMessages();
        void addErrorMessage(std::string message);
        void addWarningMessage(std::string message);
        bool hasErrors();
        bool hasWarnings();
        
    private:
        std::string m_errors;
        std::string m_warnings;
    };

}

#endif
