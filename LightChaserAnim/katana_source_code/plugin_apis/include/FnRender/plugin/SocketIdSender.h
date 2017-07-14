// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDER_SOCKETIDSENDER_H
#define FNRENDER_SOCKETIDSENDER_H

#include "FnRender/plugin/IdSenderInterface.h"

#include <string>

namespace Foundry
{
namespace Katana
{
namespace Render
{
    class SocketIdSender : public IdSenderInterface
    {
    public:

        SocketIdSender(const std::string & host, int64_t frameID);
        virtual ~SocketIdSender();

        void getIds(int64_t* nextId, int64_t* maxId);
        void send(int64_t id, const char* const objectName);

        class SocketIdSenderException : public std::exception
        {
        public:
            //! Constructor
            /*!
              @param what A message describing what caused the exception
             */
            SocketIdSenderException(const char* what) throw()
                : m_what(what) {};

            //! Get the error string.
            /*!
              @return A description of the exception
             */
            virtual const char* what() const throw() { return m_what; }

        private:
            //! An error message
            const char* m_what;
        };

    private:
        std::string _host;
        int64_t _frameID;
        int _socketId;

        void connect_if_first_time();

        SocketIdSender(const SocketIdSender&);
        SocketIdSender& operator = (const SocketIdSender&);
    };
}
}
}

namespace FnKat = Foundry::Katana;

#endif
