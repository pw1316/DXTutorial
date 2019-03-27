#ifndef __CORE_IVIEW__
#define __CORE_IVIEW__
#include <set>
#include <string>

namespace Naiive::Core {
class Message;

class IView {
  // public Facade facade
  //{
  //    get{ return Facade.instance; }
  //}
 public:
  IView() = default;
  virtual ~IView() = default;

  /* No copy, no move */
  IView(const IView& rhs) = delete;
  IView(IView&& rhs) = delete;
  IView& operator=(const IView& rhs) = delete;
  IView& operator=(IView&& rhs) = delete;

  virtual void Initialize(HWND hWnd, UINT width, UINT height) = 0;
  virtual void Shutdown() = 0;
  virtual BOOL OnUpdate() = 0;
  virtual void OnMessage(const Message& msg) {}
  virtual void CALLBACK MessageHandler(HWND hWnd, UINT message, WPARAM wParam,
                                       LPARAM lParam) {}

 protected:
  std::set<std::string> messages;
};
}  // namespace Naiive::Core
#endif
