
#include <graphics/GraphicsContext.h>

#include "engine/Event.h"
#include "engine/Interface.h"

#include "math/Math.h"
#include "math/MathTypes.h"
#include "scene/Camera.h"

#include "graphics/Enum.h"
#include "graphics/ShaderUtil.h"
#include "graphics/Layouts.h"
#include "graphics/GraphicsTypes.h"

#include "bx/math.h"
#include <bgfx/bgfx.h>
#include <bgfx/defines.h>

#include <bx/debug.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <variant>


namespace hpl {

    GraphicsContext::ShaderProgram::ShaderProgram() {
        m_configuration.m_rgbBlendFunc = CreateBlendFunction(BlendOperator::Add, BlendOperand::One, BlendOperand::Zero);
        m_configuration.m_alphaBlendFunc = CreateBlendFunction(BlendOperator::Add, BlendOperand::One, BlendOperand::Zero);
    }

    GraphicsContext::GraphicsContext()
        : m_current(0){
    }

    GraphicsContext::~GraphicsContext() {
        for (auto& it : m_programCache) {
            if(bgfx::isValid(it.second)) {
                bgfx::destroy(it.second);
            }
        }
    }

    void GraphicsContext::UpdateScreenSize(uint16_t width, uint16_t height) {
    }

    void GraphicsContext::Init() {
        m_copyProgram = resolveProgramCache<StringLiteral("vs_post_effect"), StringLiteral("fs_post_effect_copy")>();
        m_colorProgram = resolveProgramCache<StringLiteral("vs_color"), StringLiteral("fs_color")>();
        m_uvProgram = resolveProgramCache<StringLiteral("vs_basic_uv"), StringLiteral("fs_basic_uv")>();
        m_meshColorProgram = resolveProgramCache<StringLiteral("vs_color"), StringLiteral("fs_color_1")>();

        m_s_diffuseMap.Initialize();
        m_u_normalMtx.Initialize();
        m_u_color.Initialize();
    }

    void GraphicsContext::Frame() {
        m_current = 0;
        bgfx::frame();
    }


    bool GraphicsContext::isOriginBottomLeft() const {
        BX_ASSERT(bgfx::getCaps(), "GraphicsContext::Init() must be called before isOriginBottomLeft()");
        return bgfx::getCaps()->originBottomLeft;
    }

    void GraphicsContext::CopyTextureToFrameBuffer(Image& image, cRect2l dstRect, LegacyRenderTarget& target, Write write) {
        GraphicsContext::LayoutStream layout;
        cMatrixf projMtx;
        ScreenSpaceQuad(layout, projMtx, dstRect.w, dstRect.h);

        GraphicsContext::ViewConfiguration viewConfig{ target };
        viewConfig.m_viewRect = dstRect;
        viewConfig.m_projection = projMtx;
        ASSERT(false && "REMOVE");

        GraphicsContext::ShaderProgram program;
        program.m_handle = m_copyProgram;
        program.m_configuration.m_write = write;
        // program.m_projection = projMtx;
        program.m_textures.push_back({ m_s_diffuseMap, image.GetHandle(), 0 });

        DrawRequest request = { layout, program };
    }

    void GraphicsContext::Quad(
        GraphicsContext::LayoutStream& input, const cVector3f& pos, const cVector2f& size, const cVector2f& uv0, const cVector2f& uv1) {
        BX_ASSERT(bgfx::getCaps(), "GraphicsContext::Init() must be called before ScreenSpaceQuad()");

        bgfx::TransientVertexBuffer vb;
        bgfx::allocTransientVertexBuffer(&vb, 4, layout::PositionTexCoord0::layout());
        auto* vertex = reinterpret_cast<layout::PositionTexCoord0*>(vb.data);

        const float minx = pos.x;
        const float maxx = pos.x + size.x;
        const float miny = pos.y;
        const float maxy = pos.y + size.y;

        vertex[0].m_x = minx;
        vertex[0].m_y = miny;
        vertex[0].m_z = pos.z;
        vertex[0].m_u = uv0.x;
        vertex[0].m_v = uv0.y;

        vertex[1].m_x = maxx;
        vertex[1].m_y = miny;
        vertex[1].m_z = pos.z;
        vertex[1].m_u = uv1.x;
        vertex[1].m_v = uv0.y;

        vertex[2].m_x = maxx;
        vertex[2].m_y = maxy;
        vertex[2].m_z = pos.z;
        vertex[2].m_u = uv1.x;

        vertex[3].m_x = minx;
        vertex[3].m_y = maxy;
        vertex[3].m_z = pos.z;
        vertex[3].m_u = uv0.x;
        vertex[3].m_v = uv1.y;

        input.m_vertexStreams.push_back({
            .m_transient = vb,
        });
    }

    void GraphicsContext::ScreenSpaceQuad(
        GraphicsContext::LayoutStream& input, cMatrixf& proj, float textureWidth, float textureHeight, float width, float height) {
        BX_ASSERT(bgfx::getCaps(), "GraphicsContext::Init() must be called before ScreenSpaceQuad()");

        bgfx::TransientVertexBuffer vb;
        bgfx::allocTransientVertexBuffer(&vb, 3, layout::PositionTexCoord0::layout());
        auto* vertex = reinterpret_cast<layout::PositionTexCoord0*>(vb.data);

        bx::mtxOrtho(proj.v, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);

        const float minx = -width;
        const float maxx = width;
        const float miny = 0.0f;
        const float maxy = height * 2.0f;

        const bgfx::RendererType::Enum renderer = bgfx::getRendererType();
        const float texelHalf = bgfx::RendererType::Direct3D9 == renderer ? 0.5f : 0.0f;

        const float texelHalfW = texelHalf / textureWidth;
        const float texelHalfH = texelHalf / textureHeight;
        const float minu = -1.0f + texelHalfW;
        const float maxu = 1.0f + texelHalfH;

        const float zz = 0.0f;

        float minv = texelHalfH;
        float maxv = 2.0f + texelHalfH;

        if (bgfx::getCaps()->originBottomLeft) {
            const float temp = minv;
            minv = maxv;
            maxv = temp;

            minv -= 1.0f;
            maxv -= 1.0f;
        }

        vertex[0].m_x = minx;
        vertex[0].m_y = miny;
        vertex[0].m_z = zz;
        vertex[0].m_u = minu;
        vertex[0].m_v = minv;

        vertex[1].m_x = maxx;
        vertex[1].m_y = miny;
        vertex[1].m_z = zz;
        vertex[1].m_u = maxu;
        vertex[1].m_v = minv;

        vertex[2].m_x = maxx;
        vertex[2].m_y = maxy;
        vertex[2].m_z = zz;
        vertex[2].m_u = maxu;
        vertex[2].m_v = maxv;

        input.m_vertexStreams.push_back({
            .m_transient = vb,
        });
    }

} // namespace hpl
