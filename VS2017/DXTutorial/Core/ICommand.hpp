#pragma once
#include <stdafx.h>

#include <Message.hpp>

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
