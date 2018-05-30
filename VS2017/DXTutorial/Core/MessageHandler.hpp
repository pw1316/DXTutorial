#pragma once
#include <stdafx.h>
#include <map>
#include <set>

#include <View.hpp>
#include <Message.hpp>
#include <ICommand.hpp>

namespace PW
{
    namespace Core
    {
        class MessageHandler
        {
        public:
            static MessageHandler* GetInstance()
            {
                if (g_instance == nullptr)
                {
                    g_instance = new MessageHandler;
                }
                return g_instance;
            }

            static void ReleaseInstance()
            {
                if (g_instance != nullptr)
                {
                    delete g_instance;
                    g_instance = nullptr;
                }
            }

            void RegisterCommand(const std::string &name, ICommand *cmd)
            {
                auto iter = m_commandMap.find(name);
                if (iter != m_commandMap.end())
                {
                    delete iter->second;
                }
                m_commandMap[name] = cmd;
            }
            void RemoveCommand(const std::string &name)
            {
                auto iter = m_commandMap.find(name);
                if (iter != m_commandMap.end())
                {
                    delete iter->second;
                    m_commandMap.erase(iter);
                }
            }

            void RegisterViewCommand(View *view, const std::set<std::string> &list)
            {
                auto iter = m_viewCommandMap.find(view);
                if (iter != m_viewCommandMap.end())
                {
                    iter->second.insert(list.cbegin(), list.cend());
                }
                else
                {
                    m_viewCommandMap[view] = list;
                }
            }
            void RemoveViewCommand(View *view, const std::set<std::string> &list)
            {
                auto iter = m_viewCommandMap.find(view);
                if (iter != m_viewCommandMap.end())
                {
                    for (auto cmd : list)
                    {
                        auto cmdIter = iter->second.find(cmd);
                        if (cmdIter != iter->second.end())
                        {
                            iter->second.erase(cmdIter);
                        }
                    }
                    if (iter->second.empty())
                    {
                        m_viewCommandMap.erase(iter);
                    }
                }
            }

            void ExecuteCommand(const Message &message)
            {
                auto iter = m_commandMap.find(message.GetName());
                /* Is command message, directly execute */
                if (iter != m_commandMap.end())
                {
                    (*(iter->second))(message);
                }
                /* Is view message, send to view */
                else
                {
                    for (auto view : m_viewCommandMap)
                    {
                        if (view.second.find(message.GetName()) != view.second.end())
                        {
                            (*(view.first)).OnMessage(message);// View的对象必须具有OnMessage(message)成员函数
                        }
                    }
                }
            }
            BOOL HasCommand(const std::string &name)
            {
                return (m_commandMap.find(name) != m_commandMap.end());
            }

        private:
            static MessageHandler *g_instance;
            MessageHandler() = default;

            std::map<std::string, ICommand*> m_commandMap;// 命令的对象必须有效，命令移除列表时要负责删除，命令的对象的创建者不负责
            std::map<View*, std::set<std::string>> m_viewCommandMap;// View的对象可以不有效，View的对象移除时不负责删除，由该View的管理者删除。
        };
    }
}
