#pragma once
#include <stdafx.h>
#include <set>
#include <string>

#include <Message.hpp>
#include <MessageHandler.hpp>

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
            virtual void Initialize(){ MessageHandler::GetInstance()->RegisterViewCommand(this, messages); }
            virtual void Shutdown() { MessageHandler::GetInstance()->RemoveViewCommand(this, messages); }
            virtual void OnMessage(Message msg) = 0;
            std::set<std::string> messages;
        };
    }
}