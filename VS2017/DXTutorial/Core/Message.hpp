#ifndef __PW_CORE_MESSAGE__
#define __PW_CORE_MESSAGE__
#include <stdafx.h>
#include <string>

namespace PW
{
    namespace Core
    {
        class Message
        {
        public:
            explicit Message(std::string name) :m_name(name) {};
            Message(std::string name, ULONG body) :m_name(name), m_body(body) {};

            std::string GetName() const { return m_name; }
            void SetName(const std::string& name) { m_name = name; }
            void SetName(std::string&& name) { m_name = std::move(name); }

            ULONG GetBody() const { return m_body; }
            void SetBody(ULONG body) { m_body = body; }

            BOOL IsHandled() const { return m_handled; }
            void SetHandled(BOOL handled) { m_handled = handled; }
        private:
            std::string m_name;
            ULONG m_body = 0;
            BOOL m_handled = FALSE;
        };
    }
}

#endif
