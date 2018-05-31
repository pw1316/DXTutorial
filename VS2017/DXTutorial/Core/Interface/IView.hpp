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
        class IView
        {
            //public Facade facade
            //{
            //    get{ return Facade.instance; }
            //}
        public:
            IView() = default;
            virtual ~IView() = default;

            /* No copy, no move */
            IView(const IView& rhs) = delete;
            IView(IView&& rhs) = delete;
            IView &operator=(const IView &rhs) = delete;
            IView &operator=(IView &&rhs) = delete;

            virtual void Initialize() = 0;
            virtual void Shutdown() = 0;
            virtual void OnMessage(const Message &msg) = 0;
        protected:
            std::set<std::string> messages;
        };
    }
}

#endif
