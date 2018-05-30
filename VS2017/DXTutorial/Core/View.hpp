#ifndef __PW_CORE_VIEW__
#define __PW_CORE_VIEW__
#include <stdafx.h>
#include <set>
#include <string>

#include <Core/Message.hpp>

namespace PW
{
    namespace Core
    {
        class View
        {
            //public Facade facade
            //{
            //    get{ return Facade.instance; }
            //}
        public:
            View() = default;
            virtual ~View() = default;

            /* No copy, no move */
            View(const View& rhs) = delete;
            View(View&& rhs) = delete;
            View &operator=(const View &rhs) = delete;
            View &operator=(View &&rhs) = delete;

            virtual void Initialize() = 0;
            virtual void Shutdown() = 0;
            virtual void OnMessage(const Message &msg) = 0;
        protected:
            std::set<std::string> messages;
        };
    }
}

#endif
