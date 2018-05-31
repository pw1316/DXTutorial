#pragma once
#include "stdafx.h"
#include <Core/Interface/IView.hpp>
#include <Core/MessageHandler.hpp>

namespace PW
{
    namespace Manager
    {
        class InputManager : public Core::IView
        {
        private:
            static const UINT NUM_KEYS = 256U;

        public:
            InputManager() = default;
            ~InputManager() {}

            /* Override */
            virtual void Initialize() override
            {
                messages = { "SYS_KEY_DOWN", "SYS_KEY_UP" };
                Core::MessageHandler::GetInstance()->RegisterViewCommand(this, messages);
            }
            virtual void Shutdown() override
            {
                Core::MessageHandler::GetInstance()->RemoveViewCommand(this, messages);
            }
            virtual void OnMessage(const Core::Message &msg) override
            {
                if (msg.GetName() == "SYS_KEY_DOWN")
                {
                    KeyDown(msg.GetBody());
                }
                else if (msg.GetName() == "SYS_KEY_UP")
                {
                    KeyUp(msg.GetBody());
                }
            }

            BOOL IsKeyDown(std::remove_const<decltype(NUM_KEYS)>::type key)
            {
                assert(key < 256);
                return m_keys[key];
            }

        private:
            void KeyDown(std::remove_const<decltype(NUM_KEYS)>::type key)
            {
                assert(key < NUM_KEYS);
                m_keys[key] = true;
            }
            void KeyUp(std::remove_const<decltype(NUM_KEYS)>::type key)
            {
                assert(key < NUM_KEYS);
                m_keys[key] = false;
            }
            BOOL m_keys[NUM_KEYS];
        };
    }
}
