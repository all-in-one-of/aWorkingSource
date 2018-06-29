// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <ErrorReport.h>
#include <iostream>

namespace instance_scene
{ 

    ErrorReport::ErrorReport()
    : m_errors(""),
    m_warnings("")
    {
    }

    std::string ErrorReport::getErrorMessages()
    {
        return m_errors;
    }

    std::string ErrorReport::getWarningMessages()
    {
        return m_warnings;
    }

    void ErrorReport::addErrorMessage(std::string message)
    {
        m_errors.append(message);
    }

    void ErrorReport::addWarningMessage(std::string message)
    {
        m_warnings.append(message);
    }

    bool ErrorReport::hasErrors()
    {
        return !m_errors.empty();
    }

    bool ErrorReport::hasWarnings()
    {
        return !m_warnings.empty();
    }

}

