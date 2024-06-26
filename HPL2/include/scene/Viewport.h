/*
 * Copyright © 2009-2020 Frictional Games
 *
 * This file is part of Amnesia: The Dark Descent.
 *
 * Amnesia: The Dark Descent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Amnesia: The Dark Descent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Amnesia: The Dark Descent.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include "engine/Event.h"
#include "engine/IUpdateEventLoop.h"
#include "engine/Interface.h"
#include "engine/RTTI.h"

#include "graphics/ForgeHandles.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Image.h"
#include "graphics/RenderList.h"
#include "graphics/RenderTarget.h"

#include "gui/GuiTypes.h"
#include "math/cFrustum.h"
#include "math/MathTypes.h"
#include "scene/SceneTypes.h"
#include "windowing/NativeWindow.h"

#include <cstddef>
#include <cstdint>
#include <memory>

#include "Common_3/Utilities/Interfaces/ILog.h"
#include <FixPreprocessor.h>

#include "math/Uuid.h"

namespace hpl {

    class cScene;
    class cCamera;
    class iRenderer;
    class iFrameBuffer;
    class cRenderSettings;
    class cPostEffectComposite;
    class cWorld;
    class iViewportCallback;
    class cGuiSet;

    class cViewport final {
        HPL_RTTI_CLASS(cViewport, "{f5d42b52-6e84-4486-afa0-a5888f3513a0}")
    public:
        static constexpr Uuid PrimaryViewportTag = hpl::detail::From("{f5d42b52-6e84-4486-afa0-a5888f3513a0}");
        static constexpr size_t MaxViewportHandles = 12;

        using ResizeEvent = hpl::Event<hpl::cVector2l&>;
        using ViewportDispose = hpl::Event<>;
        using ViewportChange = hpl::Event<>;
        using ViewportBeforeDraw = hpl::Event<ForgeRenderer::Frame*>;

        struct DrawPayloadCommon {
            cFrustum* m_frustum;
            const ForgeRenderer::Frame* m_frame;
            SharedRenderTarget* m_outputTarget;
            cViewport* m_viewport;
            cRenderSettings* m_renderSettings;
            DebugDraw* m_debug;
            cRenderList* m_renderList;
        };
        struct PostSolidDrawPacket : public DrawPayloadCommon {
        };
        struct PostTranslucenceDrawPacket : public DrawPayloadCommon {
        };
        using PostSolidDraw = hpl::Event<PostSolidDrawPacket&>;
        using PostTranslucenceDraw = hpl::Event<PostTranslucenceDrawPacket&>;

        cViewport(cScene* apScene);

        cViewport(cViewport&&) = delete;
        void operator=(cViewport&&) = delete;
        cViewport(const cViewport&) = delete;
        cViewport& operator=(const cViewport&) = delete;

        ~cViewport();


        inline void SetActive(bool abX) { mbActive = abX;}
        inline void SetVisible(bool abX) { mbVisible = abX;}

        inline bool IsActive() { return mbActive;}
        inline bool IsVisible() { return mbVisible; }

        bool IsValid();

        inline size_t GetHandle() { return m_handle; }
        inline void SetIsListener(bool abX) { mbIsListener = abX;}

        inline bool IsListener() { return mbIsListener; }

        inline void SetCamera(cCamera* apCamera) { mpCamera = apCamera; }
        inline cCamera* GetCamera() { return mpCamera; }

        void SetWorld(cWorld* apWorld);
        inline cWorld* GetWorld() { return mpWorld; }

        inline void SetRenderer(iRenderer* apRenderer) { mpRenderer = apRenderer; }
        inline iRenderer* GetRenderer() { return mpRenderer; }

        cRenderSettings* GetRenderSettings() { return mpRenderSettings.get(); }

        inline void SetPostEffectComposite(cPostEffectComposite* apPostEffectComposite) {
            mpPostEffectComposite = apPostEffectComposite;
        }

        inline cPostEffectComposite* GetPostEffectComposite() {
            return mpPostEffectComposite;
        }

        void AddGuiSet(cGuiSet* apSet);
        void RemoveGuiSet(cGuiSet* apSet);
        cGuiSetListIterator GetGuiSetIterator();

        inline void SetSize(const cVector2l& avSize) {
            m_size = avSize;
            m_dirtyViewport = true;
        }
        const cVector2l GetSize() {
            return m_size;
        }
        const uint2 GetSizeU2() { return uint2(m_size.x, m_size.y); }
        uint2 GetSizeU() const { return uint2(m_size.x, m_size.y); }

        inline SharedRenderTarget& Target(ForgeRenderer::Frame& frame) { return m_targets[frame.m_frameIndex]; }
        void SetTarget(std::array<SharedRenderTarget, ForgeRenderer::SwapChainLength>&& target);
        inline std::shared_ptr<Image>& GetImage(ForgeRenderer::Frame& frame) {
            return m_images[frame.m_frameIndex];
        }

        // if a target is not initialized then its assumed to go to the swapchain
        void InitializeTarget(Renderer* renderer, RenderTargetDesc desc);
        void InvalidateTarget();

        void bindToWindow(window::NativeWindowWrapper& window);

        inline void ConnectViewportChanged(ViewportChange::Handler& handler) { handler.Connect(m_viewportChanged);}
        inline void ConnectDraw(PostSolidDraw::Handler& handler) { handler.Connect(m_postSolidDraw); }
        inline void ConnectDraw(PostTranslucenceDraw::Handler& handler) { handler.Connect(m_postTranslucenceDraw); }
        inline void ConnectBeforeDraw(ViewportBeforeDraw::Handler& handler) { handler.Connect(m_viewportBeforeDraw); }
        inline void SignalDraw(PostSolidDrawPacket& payload) { m_postSolidDraw.Signal(payload);}
        inline void SignalDraw(PostTranslucenceDrawPacket& payload) { m_postTranslucenceDraw.Signal(payload);}
        inline void SignalBeforeDraw(ForgeRenderer::Frame* frame) { m_viewportBeforeDraw.Signal(frame);}

        inline void SetTag(const Uuid& tag) { m_tag = tag; }
        inline Uuid Tag() { return m_tag; }

    private:
        bool mbActive;
        bool mbVisible;
        bool mbIsListener;
        bool m_dirtyViewport = false;
        size_t m_handle = 0;

        cScene* mpScene = nullptr;
        cCamera* mpCamera = nullptr;
        cWorld* mpWorld = nullptr;
        iRenderer* mpRenderer = nullptr;
        cPostEffectComposite* mpPostEffectComposite = nullptr;

        std::array<SharedRenderTarget, ForgeRenderer::SwapChainLength> m_targets;
        std::array<std::shared_ptr<hpl::Image>, ForgeRenderer::SwapChainLength> m_images;

        cVector2l m_size = { 0, 0 };
        Uuid m_tag;

        IUpdateEventLoop::UpdateEvent::Handler m_updateEventHandler;

        ViewportDispose m_disposeEvent;
        ViewportChange m_viewportChanged;
        ViewportBeforeDraw m_viewportBeforeDraw;
        window::WindowEvent::Handler m_windowEventHandler;

        PostSolidDraw m_postSolidDraw;
        PostTranslucenceDraw m_postTranslucenceDraw;

        tGuiSetList m_guiSets;

        std::unique_ptr<cRenderSettings> mpRenderSettings;
        template<typename TData>
        friend class UniqueViewportData;
    };

    // Data that is unique to a viewport
    template<typename TData>
    class UniqueViewportData final {
    public:
        UniqueViewportData(): m_targets(),
            m_disposeHandlers() {
        }

        UniqueViewportData(const UniqueViewportData& other) = delete;
        UniqueViewportData(UniqueViewportData&& other)
            :  m_disposeHandlers(std::move(other.m_disposeHandlers))
            , m_targets(std::move(other.m_targets)) {
        }
        UniqueViewportData& operator=(const UniqueViewportData& other) = delete;
        void operator=(UniqueViewportData&& other) {
            m_disposeHandlers = std::move(other.m_disposeHandlers);
            m_targets = std::move(other.m_targets);
        }

        TData* update(cViewport* viewport, std::unique_ptr<TData>&& newData) {
            if(!viewport) {
                return nullptr;
            }

            uint8_t handle = viewport->GetHandle();
            ASSERT(handle < cViewport::MaxViewportHandles && "Invalid viewport handle");
            auto& target = m_targets[handle];
            target = std::move(newData);
            if(target) {
                m_revision++;
                auto& disposeHandle = m_disposeHandlers[handle];
                if (!disposeHandle.IsConnected()) {
                    disposeHandle = std::move(cViewport::ViewportDispose::Handler([&, handle]() {
                        m_targets[handle] = nullptr;
                    }));
                    disposeHandle.Connect(viewport->m_disposeEvent);
                }
            } else {
                m_disposeHandlers[handle].Disconnect();
            }
            return target.get();
        }

        TData* update(cViewport& viewport, std::unique_ptr<TData>&& newData) {
            return update(&viewport, std::move(newData));
        }

        TData* resolve(cViewport* viewport) {
            if(!viewport) {
                return nullptr;
            }

            uint8_t handle = viewport->GetHandle();
            ASSERT(handle < cViewport::MaxViewportHandles && "Invalid viewport handle");
            return m_targets[handle].get();
        }
        TData* resolve(cViewport& viewport) {
            uint8_t handle = viewport.GetHandle();
            ASSERT(handle < cViewport::MaxViewportHandles && "Invalid viewport handle");
            return m_targets[handle].get();
        }

        uint32_t revision() {
            return m_revision;
        }

    private:
        uint32_t m_revision = 0;
        std::array<cViewport::ViewportDispose::Handler, cViewport::MaxViewportHandles> m_disposeHandlers;
        std::array<std::unique_ptr<TData>, cViewport::MaxViewportHandles> m_targets;
    };

}; // namespace hpl
