#ifndef __PW_CORE_ICOMMAND__
#define __PW_CORE_ICOMMAND__
#include <stdafx.h>

#include <Core/Message.hpp>

namespace PW
{
    namespace Core
    {
        class ICommand
        {
        public:
            ICommand() = default;
            virtual void operator()(Message msg) = 0;
            virtual ~ICommand() = default;
        };
    }
}

#endif
