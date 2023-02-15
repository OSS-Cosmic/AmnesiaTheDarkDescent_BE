#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>

#include <engine/Event.h>
#include <engine/RTTI.h>
#include <math/MathTypes.h>
#include <system/HandleWrapper.h>
#include <bx/debug.h>

union SDL_Event;

namespace hpl::window {

    enum class WindowEventType : uint16_t {
        ResizeWindowEvent,
        MoveWindowEvent,
        QuitEvent,
        // Mouse can move to a separate interface
        MouseMoveEvent,
        // keyboard can move to a separate interface

    };
    enum class WindowStyle: uint32_t {
        WindowStyleNone = 0,
        WindowStyleTitleBar = 0x01,
        WindowStyleResizable = 0x02,
        WindowStyleBorderless = 0x04,
        WindowStyleFullscreen = 0x08,
        WindowStyleFullscreenDesktop = 0x10,

        WindowStyleClosable = 0x20,
        WindowStyleMinimizable = 0x40,
        WindowStyleMaximizable = 0x80,
    };

    enum class WindowStatus: uint32_t {
        WindowStatusNone = 0,
        WindowStatusInputFocus = 0x01,
        WindowStatusInputMouseFocus = 0x02,
        WindowStatusWindowMinimized = 0x04,
        WindowStatusWindowMaximized = 0x08,
        WindowStatusVisible = 0x10,
    };

    struct InternalEvent {
        union {
            SDL_Event* m_sdlEvent; // SDL_Event when compiling for SDL2
        };
    };

    struct WindowEventPayload {
        WindowEventType m_type;
        union {
            struct {
                uint32_t m_width;
                uint32_t m_height;
            } m_resizeWindow;
            struct {
                int32_t m_x;
                int32_t m_y;
            } m_moveWindow;
            struct {
                int32_t m_x;
                int32_t m_y;
            } m_mouseEvent;
        } payload;
    };
    using WindowEvent = hpl::Event<WindowEventPayload&>;

    enum class WindowType : uint8_t { Window, Fullscreen, Borderless };
    namespace internal {

        using WindowInternalEvent = hpl::Event<InternalEvent&>;
        class NativeWindowHandler final {
            HPL_HANDLER_IMPL(NativeWindowHandler)
        };

        void ConnectInternalEventHandler(NativeWindowHandler& handler, WindowInternalEvent::Handler& eventHandle);
        void ConnectionWindowEventHandler(NativeWindowHandler& handler, WindowEvent::Handler& eventHandle);

        NativeWindowHandler Initialize();
        void SetWindowTitle(NativeWindowHandler& handler, const std::string_view title);
        void SetWindowSize(NativeWindowHandler& handler, const cVector2l& size);

        void* NativeWindowHandle(NativeWindowHandler& handler);
        void* NativeDisplayHandle(NativeWindowHandler& handler);
        cVector2l GetWindowSize(NativeWindowHandler& handler);
        WindowStatus GetWindowStatus(NativeWindowHandler& handler);

        void WindowGrabCursor(NativeWindowHandler& handler);
        void WindowReleaseCursor(NativeWindowHandler& handler);
        void ShowHardwareCursor(NativeWindowHandler& handler);
        void HideHardwareCursor(NativeWindowHandler& handler);
        void ConstrainCursor(NativeWindowHandler& handler);
        void UnconstrainCursor(NativeWindowHandler& handler);
        
        void fetchQueuedEvents(NativeWindowHandler& handler, std::function<void(InternalEvent&)>);

        void Process(NativeWindowHandler& handler);
    } // namespace internal

    // wrapper over an opaque pointer to a windowing system
    // this is the only way to interact with the windowing system
    class NativeWindowWrapper final {
        HPL_RTTI_CLASS(NativeWindow, "{d17ea5c7-30f1-4d5d-b38e-1a7e88e137fc}")
    public:

        ~NativeWindowWrapper() {
        }
        NativeWindowWrapper() = default;
        NativeWindowWrapper(internal::NativeWindowHandler&& handle)
            : m_impl(std::move(handle)) {
        }
        NativeWindowWrapper(NativeWindowWrapper&& other)
            : m_impl(std::move(other.m_impl)) {
        }
        NativeWindowWrapper(const NativeWindowWrapper& other) = delete;

        NativeWindowWrapper& operator=(NativeWindowWrapper& other) = delete;
        void operator=(NativeWindowWrapper&& other) {
            m_impl = std::move(other.m_impl);
        }
        
        void* NativeWindowHandle() {
            BX_ASSERT(m_impl, "NativeWindowHandle is null")
            return internal::NativeWindowHandle(m_impl);
        }

        void* NativeDisplayHandle() {
            BX_ASSERT(m_impl, "NativeDisplayHandle is null")
            return internal::NativeDisplayHandle(m_impl);
        }

        void SetWindowSize(cVector2l size) {
            BX_ASSERT(m_impl, "NativeWindowHandle is null")
            internal::SetWindowSize(m_impl, size);
        }

        void SetWindowTitle(const std::string_view title) {
            BX_ASSERT(m_impl, "NativeWindowHandle is null")
            internal::SetWindowTitle(m_impl, title);
        }

        cVector2l GetWindowSize() {
            BX_ASSERT(m_impl, "NativeWindowHandle is null")
            return internal::GetWindowSize(m_impl);
        }

        void SetWindowEventHandler(WindowEvent::Handler& handler) {
            BX_ASSERT(m_impl, "NativeWindowHandle is null")
            internal::ConnectionWindowEventHandler(m_impl, handler);
        }

        WindowStatus GetWindowStatus() {
            BX_ASSERT(m_impl, "NativeWindowHandle is null")
            return internal::GetWindowStatus(m_impl);
        }

        void Process() {
            BX_ASSERT(m_impl, "NativeWindowHandle is null")
            internal::Process(m_impl);
        }

        void WindowGrabCursor() {
            BX_ASSERT(m_impl, "NativeWindowHandle is null")
            internal::WindowGrabCursor(m_impl);
        }
        void WindowReleaseCursor() {
            BX_ASSERT(m_impl, "NativeWindowHandle is null")
            internal::WindowReleaseCursor(m_impl);
        }
        void ShowHardwareCursor() {
            BX_ASSERT(m_impl, "NativeWindowHandle is null")
            internal::ShowHardwareCursor(m_impl);
        }
        void HideHardwareCursor() {
            BX_ASSERT(m_impl, "NativeWindowHandle is null")
            internal::HideHardwareCursor(m_impl);
        }
        void ConstrainCursor() {
            BX_ASSERT(m_impl, "NativeWindowHandle is null")
            internal::ConstrainCursor(m_impl);
        }
        void UnconstrainCursor() {
            BX_ASSERT(m_impl, "NativeWindowHandle is null")
            internal::UnconstrainCursor(m_impl);
        }

        void fetchQueuedEvents(std::function<void(InternalEvent&)> handler) {
            BX_ASSERT(m_impl, "NativeWindowHandle is null")
            internal::fetchQueuedEvents(m_impl, handler);
        }


        WindowType GetWindowType() {
            return WindowType::Window;
        }

    private:
        internal::NativeWindowHandler m_impl = internal::NativeWindowHandler();
    };
    
    inline hpl::window::WindowStatus operator&(hpl::window::WindowStatus lhs, hpl::window::WindowStatus rhs) {
        return static_cast<hpl::window::WindowStatus>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
    }
    inline hpl::window::WindowStatus operator|(hpl::window::WindowStatus lhs, hpl::window::WindowStatus rhs) {
        return static_cast<hpl::window::WindowStatus>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    }
    inline  bool any(hpl::window::WindowStatus lhs) {
        return static_cast<uint32_t>(lhs) != 0;
    }

} // namespace hpl::window