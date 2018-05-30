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

            /* No copy, no move */
            View(const View& rhs) = delete;
            View(View&& rhs) = delete;
            View &operator=(const View &rhs) = delete;
            View &operator=(View &&rhs) = delete;

            virtual void Initialize(){ MessageHandler::GetInstance()->RegisterViewCommand(this, messages); }
            virtual void Shutdown() { MessageHandler::GetInstance()->RemoveViewCommand(this, messages); }
            virtual void OnMessage(const Message &msg) = 0;
        protected:
            std::set<std::string> messages;
        };
    }
}