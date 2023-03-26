#include "graphics/ImmediateDrawBatch.h"

#include "graphics/GraphicsContext.h"
#include "graphics/Layouts.h"

#include "scene/Camera.h"

#include "math/Math.h"
#include "math/MathTypes.h"

namespace hpl {

    ImmediateDrawBatch::ImmediateDrawBatch(GraphicsContext& context, RenderTarget& target, const cMatrixf& view, const cMatrixf& projection)
        : m_context(context)
        , m_target(target)
        , m_view(view)
        , m_projection(projection) {
        m_colorProgram = context.resolveProgramCache<StringLiteral("vs_color"), StringLiteral("fs_color")>();
        m_uvProgram = context.resolveProgramCache<StringLiteral("vs_basic_uv"), StringLiteral("fs_basic_uv")>();
        m_meshColorProgram = context.resolveProgramCache<StringLiteral("vs_color"), StringLiteral("fs_color_1")>();
    }

    void ImmediateDrawBatch::DebugDrawBoxMinMax(
        const cVector3f& avMin, const cVector3f& avMax, const cColor& color, const DebugDrawOptions& options) {
        DebugDrawLine(cVector3f(avMax.x, avMax.y, avMax.z), cVector3f(avMin.x, avMax.y, avMax.z), color, options);
        DebugDrawLine(cVector3f(avMax.x, avMax.y, avMax.z), cVector3f(avMax.x, avMin.y, avMax.z), color, options);
        DebugDrawLine(cVector3f(avMin.x, avMax.y, avMax.z), cVector3f(avMin.x, avMin.y, avMax.z), color, options);
        DebugDrawLine(cVector3f(avMin.x, avMin.y, avMax.z), cVector3f(avMax.x, avMin.y, avMax.z), color, options);

        // Neg Z Quad
        DebugDrawLine(cVector3f(avMax.x, avMax.y, avMin.z), cVector3f(avMin.x, avMax.y, avMin.z), color, options);
        DebugDrawLine(cVector3f(avMax.x, avMax.y, avMin.z), cVector3f(avMax.x, avMin.y, avMin.z), color, options);
        DebugDrawLine(cVector3f(avMin.x, avMax.y, avMin.z), cVector3f(avMin.x, avMin.y, avMin.z), color, options);
        DebugDrawLine(cVector3f(avMin.x, avMin.y, avMin.z), cVector3f(avMax.x, avMin.y, avMin.z), color, options);

        // Lines between
        DebugDrawLine(cVector3f(avMax.x, avMax.y, avMax.z), cVector3f(avMax.x, avMax.y, avMin.z), color, options);
        DebugDrawLine(cVector3f(avMin.x, avMax.y, avMax.z), cVector3f(avMin.x, avMax.y, avMin.z), color, options);
        DebugDrawLine(cVector3f(avMin.x, avMin.y, avMax.z), cVector3f(avMin.x, avMin.y, avMin.z), color, options);
        DebugDrawLine(cVector3f(avMax.x, avMin.y, avMax.z), cVector3f(avMax.x, avMin.y, avMin.z), color, options);
    }

    void ImmediateDrawBatch::DrawTri(
        const cVector3f& v1, const cVector3f& v2, const cVector3f& v3, const cColor& color, const DebugDrawOptions& options) {
        cVector3f vv1 = cMath::MatrixMul(options.m_transform, v1);
        cVector3f vv2 = cMath::MatrixMul(options.m_transform, v2);
        cVector3f vv3 = cMath::MatrixMul(options.m_transform, v3);
        m_colorTriangles.push_back(ColorTriRequest{
            .m_depthTest = options.m_depthTest,
            .m_v1 = { vv1.x, vv1.y, vv1.z },
            .m_v2 = { vv2.x, vv2.y, vv2.z },
            .m_v3 = { vv3.x, vv3.y, vv3.z },
            .m_color = { color.r, color.g, color.b, color.a },
        });
    }
    void ImmediateDrawBatch::DrawTri(
        const Eigen::Vector3f& v1,
        const Eigen::Vector3f& v2,
        const Eigen::Vector3f& v3,
        const Eigen::Vector4f& color,
        const DebugDrawOptions& options) {
        DrawTri(
            cVector3f(v1.x(), v1.y(), v1.z()),
            cVector3f(v2.x(), v2.y(), v2.z()),
            cVector3f(v3.x(), v3.y(), v3.z()),
            cColor(color.x(), color.y(), color.z(), color.w()),
            options);
    }

    void ImmediateDrawBatch::DrawPyramid(const cVector3f& baseCenter, const cVector3f& top, float halfWidth, const cColor& color, const DebugDrawOptions& options) {

        	cVector3f vNormal = top - baseCenter;
            cVector3f vPoint = baseCenter + 1;
            cVector3f vRight = cMath::Vector3Cross(vNormal, vPoint);
            vRight.Normalize();
            cVector3f vForward = cMath::Vector3Cross(vNormal, vRight);
            vForward.Normalize();

            cVector3f topRight = baseCenter + (vRight+vForward)*halfWidth;
            cVector3f topLeft = baseCenter + (vRight-vForward)*halfWidth;
            cVector3f bottomLeft = baseCenter + (vRight*(-1)-vForward)*halfWidth;
            cVector3f bottomRight = baseCenter + (vRight*(-1)+vForward)*halfWidth;

            DrawTri(top, topRight, topLeft, color, options);
            DrawTri(top, topLeft, bottomLeft, color, options);
            DrawTri(top, bottomLeft, bottomRight, color, options);
            DrawTri(top, bottomRight, topRight, color, options);
    }


    void ImmediateDrawBatch::DebugDrawLine(
        const cVector3f& start, const cVector3f& end, const cColor& color, const DebugDrawOptions& options) {
        cVector3f transformStart = cMath::MatrixMul(options.m_transform, start);
        cVector3f transformEnd = cMath::MatrixMul(options.m_transform, end);
        m_lineSegments.push_back(LineSegmentRequest{
            .m_depthTest = options.m_depthTest,
            .m_start = { transformStart.x, transformStart.y, transformStart.z },
            .m_end = { transformEnd.x, transformEnd.y, transformEnd.z },
            .m_color = { color.r, color.g, color.b, color.a },
        });
    }
    void ImmediateDrawBatch::DebugDrawMesh(
        const GraphicsContext::LayoutStream& layout, const cColor& color, const DebugDrawOptions& options) {
        m_debugMeshes.push_back({
            .m_layout = layout,
            .m_depthTest = options.m_depthTest,
            .m_transform = options.m_transform,
            .m_color = { color.r, color.g, color.b, color.a },
        });
    }

    void ImmediateDrawBatch::DrawQuad(
        const cVector3f& v1,
        const cVector3f& v2,
        const cVector3f& v3,
        const cVector3f& v4,
        const cVector2f& uv0,
        const cVector2f& uv1,
        hpl::Image* image,
        const cColor& aTint,
        const DebugDrawOptions& options) {
        cVector3f vv1 = cMath::MatrixMul(options.m_transform, v1);
        cVector3f vv2 = cMath::MatrixMul(options.m_transform, v2);
        cVector3f vv3 = cMath::MatrixMul(options.m_transform, v3);
        cVector3f vv4 = cMath::MatrixMul(options.m_transform, v4);

        m_uvQuads.push_back(UVQuadRequest{
            .m_depthTest = options.m_depthTest,
            .m_v1 = Eigen::Vector3f(vv1.x, vv1.y, vv1.z),
            .m_v2 = Eigen::Vector3f(vv2.x, vv2.y, vv2.z),
            .m_v3 = Eigen::Vector3f(vv3.x, vv3.y, vv3.z),
            .m_v4 = Eigen::Vector3f(vv4.x, vv4.y, vv4.z),
            .m_uv0 = Eigen::Vector2f(uv0.x, uv0.y),
            .m_uv1 = Eigen::Vector2f(uv1.x, uv1.y),
            .m_uvImage = image,
            .m_color = aTint,
        });
    }

    void ImmediateDrawBatch::DrawQuad(
        const Eigen::Vector3f& v1,
        const Eigen::Vector3f& v2,
        const Eigen::Vector3f& v3,
        const Eigen::Vector3f& v4,
        const Eigen::Vector2f& uv0,
        const Eigen::Vector2f& uv1,
        hpl::Image* image,
        const Eigen::Vector4f& aTint,
        const DebugDrawOptions& options) {
        DrawQuad(
            cVector3f(v1.x(), v1.y(), v1.z()),
            cVector3f(v2.x(), v2.y(), v2.z()),
            cVector3f(v3.x(), v3.y(), v3.z()),
            cVector3f(v4.x(), v4.y(), v4.z()),
            cVector2f(uv0.x(), uv0.y()),
            cVector2f(uv1.x(), uv1.y()),
            image,
            cColor(aTint.x(), aTint.y(), aTint.z(), aTint.w()),
            options);
    }
    void ImmediateDrawBatch::DrawQuad(
        const cVector3f& v1,
        const cVector3f& v2,
        const cVector3f& v3,
        const cVector3f& v4,
        const cColor& aColor,
        const DebugDrawOptions& options) {
        cVector3f vv1 = cMath::MatrixMul(options.m_transform, v1);
        cVector3f vv2 = cMath::MatrixMul(options.m_transform, v2);
        cVector3f vv3 = cMath::MatrixMul(options.m_transform, v3);
        cVector3f vv4 = cMath::MatrixMul(options.m_transform, v4);

        m_colorQuads.push_back(ColorQuadRequest{
            .m_depthTest = options.m_depthTest,
            .m_v1 = { vv1.x, vv1.y, vv1.z },
            .m_v2 = { vv2.x, vv2.y, vv2.z },
            .m_v3 = { vv3.x, vv3.y, vv3.z },
            .m_v4 = { vv4.x, vv4.y, vv4.z },
            .m_color = { aColor.r, aColor.g, aColor.b, aColor.a },
        });
    }

    void ImmediateDrawBatch::DrawQuad(
        const Eigen::Vector3f& v1,
        const Eigen::Vector3f& v2,
        const Eigen::Vector3f& v3,
        const Eigen::Vector3f& v4,
        const Eigen::Vector4f& color,
        const DebugDrawOptions& options) {
        DrawQuad(
            cVector3f(v1.x(), v1.y(), v1.z()),
            cVector3f(v2.x(), v2.y(), v2.z()),
            cVector3f(v3.x(), v3.y(), v3.z()),
            cVector3f(v4.x(), v4.y(), v4.z()),
            cColor(color.x(), color.y(), color.z(), color.w()),
            options);
    }

    void ImmediateDrawBatch::DrawBillboard(
        const cVector3f& pos,
        const cVector2f& size,
        const cVector2f& uv0,
        const cVector2f& uv1,
        hpl::Image* image,
        const cColor& aTint,
        const DebugDrawOptions& options) {
        DrawBillboard(
            Eigen::Vector3f(pos.x, pos.y, pos.z),
            Eigen::Vector2f(size.x, size.y),
            Eigen::Vector2f(uv0.x, uv0.y),
            Eigen::Vector2f(uv1.x, uv1.y),
            image,
            Eigen::Vector4f(aTint.r, aTint.g, aTint.b, aTint.a),
            options);
    }

    // scale based on distance from camera
    float ImmediateDrawBatch::BillboardScale(cCamera* apCamera, const Eigen::Vector3f& pos) {
        const auto avViewSpacePosition = cMath::MatrixMul(apCamera->GetViewMatrix(), cVector3f(pos.x(), pos.y(), pos.z()));
        switch (apCamera->GetProjectionType()) {
        case eProjectionType_Orthographic:
            return apCamera->GetOrthoViewSize().x * 0.25f;
        case eProjectionType_Perspective:
            return cMath::Abs(avViewSpacePosition.z);
        default:
            break;
        }
        BX_ASSERT(false, "invalid projection type");
        return 0.0f;
    }

    void ImmediateDrawBatch::DrawBillboard(
        const Eigen::Vector3f& pos,
        const Eigen::Vector2f& size,
        const Eigen::Vector2f& uv0,
        const Eigen::Vector2f& uv1,
        hpl::Image* image,
        const Eigen::Vector4f& aTint,
        const DebugDrawOptions& options) {
        Eigen::Matrix4f rotation = Eigen::Matrix4f::Identity();
        rotation.block<3, 3>(0, 0) = Eigen::Matrix3f({ { m_view.m[0][0], m_view.m[0][1], m_view.m[0][2] },
                                                       { m_view.m[1][0], m_view.m[1][1], m_view.m[1][2] },
                                                       { m_view.m[2][0], m_view.m[2][1], m_view.m[2][2] } });

        Eigen::Matrix4f billboard = Eigen::Matrix4f::Identity();
        billboard.block<3, 1>(0, 3) = pos;
        Eigen::Matrix4f transform = billboard * rotation;

        Eigen::Vector2f halfSize = Eigen::Vector2f(size.x() / 2.0f, size.y() / 2.0f);
        Eigen::Vector4f v1 = (transform * Eigen::Vector4f(halfSize.x(), halfSize.y(), 0, 1));
        Eigen::Vector4f v2 = (transform * Eigen::Vector4f(-halfSize.x(), halfSize.y(), 0, 1));
        Eigen::Vector4f v3 = (transform * Eigen::Vector4f(halfSize.x(), -halfSize.y(), 0, 1));
        Eigen::Vector4f v4 = (transform * Eigen::Vector4f(-halfSize.x(), -halfSize.y(), 0, 1));
        DrawQuad(v1.head<3>(), v2.head<3>(), v3.head<3>(), v4.head<3>(), uv0, uv1, image, aTint, options);
    }

    void ImmediateDrawBatch::DebugDrawSphere(const cVector3f& pos, float radius, const cColor& color, const DebugDrawOptions& options) {
        DebugDrawSphere(pos, radius, color, color, color, options);
    }

    void ImmediateDrawBatch::DebugDrawSphere(
        const cVector3f& pos, float radius, const cColor& c1, const cColor& c2, const cColor& c3, const DebugDrawOptions& options) {
        constexpr int alSegments = 32;
        constexpr float afAngleStep = k2Pif / static_cast<float>(alSegments);
        // X Circle:
        for (float a = 0; a < k2Pif; a += afAngleStep) {
            DebugDrawLine(
                cVector3f(pos.x, pos.y + sin(a) * radius, pos.z + cos(a) * radius),
                cVector3f(pos.x, pos.y + sin(a + afAngleStep) * radius, pos.z + cos(a + afAngleStep) * radius),
                c1,
                options);
        }

        // Y Circle:
        for (float a = 0; a < k2Pif; a += afAngleStep) {
            DebugDrawLine(
                cVector3f(pos.x + cos(a) * radius, pos.y, pos.z + sin(a) * radius),
                cVector3f(pos.x + cos(a + afAngleStep) * radius, pos.y, pos.z + sin(a + afAngleStep) * radius),
                c2,
                options);
        }

        // Z Circle:
        for (float a = 0; a < k2Pif; a += afAngleStep) {
            DebugDrawLine(
                cVector3f(pos.x + cos(a) * radius, pos.y + sin(a) * radius, pos.z),
                cVector3f(pos.x + cos(a + afAngleStep) * radius, pos.y + sin(a + afAngleStep) * radius, pos.z),
                c3,
                options);
        }
    }

    void ImmediateDrawBatch::flush() {
        if (m_lineSegments.empty() && m_colorQuads.empty() && m_uvQuads.empty()) {
            return;
        }

        cVector2l imageSize = m_target.GetImage()->GetImageSize();
        if (!m_colorTriangles.empty()) {
            size_t vertexBufferOffset = 0;
            size_t indexBufferOffset = 0;
            GraphicsContext::ViewConfiguration viewConfiguration{ m_target };
            viewConfiguration.m_viewRect = { 0, 0, static_cast<uint16_t>(imageSize.x), static_cast<uint16_t>(imageSize.y) };
            viewConfiguration.m_projection = m_projection;
            viewConfiguration.m_view = m_view;
            const auto view = m_context.StartPass("Immediate - Tries", viewConfiguration);

            std::sort(m_colorTriangles.begin(), m_colorTriangles.end(), [](const ColorTriRequest& a, const ColorTriRequest& b) {
                return a.m_depthTest < b.m_depthTest;
            });

            const size_t numVertices = m_colorTriangles.size() * 3;

            bgfx::TransientVertexBuffer vb;
            bgfx::TransientIndexBuffer ib;
            bgfx::allocTransientVertexBuffer(&vb, numVertices, layout::PositionColor::layout());
            bgfx::allocTransientIndexBuffer(&ib, numVertices);

            auto it = m_colorTriangles.begin();
            auto lastIt = m_colorTriangles.begin();
            while (it != m_colorTriangles.end()) {
                size_t vertexBufferIndex = 0;
                size_t indexBufferIndex = 0;
                do {
                    reinterpret_cast<uint16_t*>(ib.data)[indexBufferOffset + (indexBufferIndex++)] = vertexBufferIndex;
                    reinterpret_cast<hpl::layout::PositionColor*>(vb.data)[vertexBufferOffset + (vertexBufferIndex++)] = {
                        { it->m_v1.x(), it->m_v1.y(), it->m_v1.z() }, { it->m_color.x(), it->m_color.y(), it->m_color.z(), it->m_color.w() }
                    };

                    reinterpret_cast<uint16_t*>(ib.data)[indexBufferOffset + (indexBufferIndex++)] = vertexBufferIndex;
                    reinterpret_cast<hpl::layout::PositionColor*>(vb.data)[vertexBufferOffset + (vertexBufferIndex++)] = {
                        { it->m_v2.x(), it->m_v2.y(), it->m_v2.z() }, { it->m_color.x(), it->m_color.y(), it->m_color.z(), it->m_color.w() }
                    };

                    reinterpret_cast<uint16_t*>(ib.data)[indexBufferOffset + (indexBufferIndex++)] = vertexBufferIndex;
                    reinterpret_cast<hpl::layout::PositionColor*>(vb.data)[vertexBufferOffset + (vertexBufferIndex++)] = {
                        { it->m_v3.x(), it->m_v3.y(), it->m_v3.z() }, { it->m_color.x(), it->m_color.y(), it->m_color.z(), it->m_color.w() }
                    };

                    lastIt = it;
                    it++;
                } while (it != m_colorTriangles.end() && it->m_depthTest == lastIt->m_depthTest);

                GraphicsContext::LayoutStream layout;
                layout.m_vertexStreams.push_back({ .m_transient = vb,
                                                   .m_startVertex = static_cast<uint32_t>(vertexBufferOffset),
                                                   .m_numVertices = static_cast<uint32_t>(vertexBufferIndex) });
                layout.m_indexStream = { .m_transient = ib,
                                         .m_startIndex = static_cast<uint32_t>(indexBufferOffset),
                                         .m_numIndices = static_cast<uint32_t>(indexBufferIndex) };
                indexBufferOffset += indexBufferIndex;
                vertexBufferOffset += vertexBufferIndex;

                GraphicsContext::ShaderProgram shaderProgram;
                shaderProgram.m_handle = m_colorProgram;
                shaderProgram.m_configuration.m_depthTest = lastIt->m_depthTest;
                shaderProgram.m_configuration.m_rgbBlendFunc =
                    CreateBlendFunction(BlendOperator::Add, BlendOperand::One, BlendOperand::One);
                shaderProgram.m_configuration.m_alphaBlendFunc =
                    CreateBlendFunction(BlendOperator::Add, BlendOperand::One, BlendOperand::One);
                shaderProgram.m_configuration.m_write = Write::RGB;

                GraphicsContext::DrawRequest request = {
                    layout,
                    shaderProgram,
                };
                m_context.Submit(view, request);
            }
        }

        if (!m_colorQuads.empty()) {
            size_t vertexBufferOffset = 0;
            size_t indexBufferOffset = 0;
            GraphicsContext::ViewConfiguration viewConfiguration{ m_target };
            viewConfiguration.m_viewRect = { 0, 0, static_cast<uint16_t>(imageSize.x), static_cast<uint16_t>(imageSize.y) };
            viewConfiguration.m_projection = m_projection;
            viewConfiguration.m_view = m_view;
            const auto view = m_context.StartPass("Immediate - Quads", viewConfiguration);

            std::sort(m_colorQuads.begin(), m_colorQuads.end(), [](const ColorQuadRequest& a, const ColorQuadRequest& b) {
                return a.m_depthTest < b.m_depthTest;
            });

            const size_t numVertices = m_colorQuads.size() * 4;
            const size_t numIndices = m_colorQuads.size() * 6;

            bgfx::TransientVertexBuffer vb;
            bgfx::TransientIndexBuffer ib;
            bgfx::allocTransientVertexBuffer(&vb, numVertices, layout::PositionColor::layout());
            bgfx::allocTransientIndexBuffer(&ib, numIndices);

            auto it = m_colorQuads.begin();
            auto lastIt = m_colorQuads.begin();
            while (it != m_colorQuads.end()) {
                size_t vertexBufferIndex = 0;
                size_t indexBufferIndex = 0;
                do {
                    // reinterpret_cast<uint16_t*>(ib.data)[indexBufferOffset + (indexBufferIndex++)] = vertexBufferIndex;
                    reinterpret_cast<hpl::layout::PositionColor*>(vb.data)[vertexBufferOffset + (vertexBufferIndex++)] = {
                        { it->m_v1.x(), it->m_v1.y(), it->m_v1.z() }, { it->m_color.x(), it->m_color.y(), it->m_color.z(), it->m_color.w() }
                    };

                    // reinterpret_cast<uint16_t*>(ib.data)[indexBufferOffset + (indexBufferIndex++)] = vertexBufferIndex;
                    reinterpret_cast<hpl::layout::PositionColor*>(vb.data)[vertexBufferOffset + (vertexBufferIndex++)] = {
                        { it->m_v2.x(), it->m_v2.y(), it->m_v2.z() }, { it->m_color.x(), it->m_color.y(), it->m_color.z(), it->m_color.w() }
                    };

                    // reinterpret_cast<uint16_t*>(ib.data)[indexBufferOffset + (indexBufferIndex++)] = vertexBufferIndex;
                    reinterpret_cast<hpl::layout::PositionColor*>(vb.data)[vertexBufferOffset + (vertexBufferIndex++)] = {
                        { it->m_v3.x(), it->m_v3.y(), it->m_v3.z() }, { it->m_color.x(), it->m_color.y(), it->m_color.z(), it->m_color.w() }
                    };

                    // reinterpret_cast<uint16_t*>(ib.data)[indexBufferOffset + (indexBufferIndex++)] = vertexBufferIndex;
                    reinterpret_cast<hpl::layout::PositionColor*>(vb.data)[vertexBufferOffset + (vertexBufferIndex++)] = {
                        { it->m_v4.x(), it->m_v4.y(), it->m_v4.z() }, { it->m_color.x(), it->m_color.y(), it->m_color.z(), it->m_color.w() }
                    };

                    reinterpret_cast<uint16_t*>(ib.data)[indexBufferOffset + (indexBufferIndex++)] = vertexBufferIndex - 4;
                    reinterpret_cast<uint16_t*>(ib.data)[indexBufferOffset + (indexBufferIndex++)] = vertexBufferIndex - 3;
                    reinterpret_cast<uint16_t*>(ib.data)[indexBufferOffset + (indexBufferIndex++)] = vertexBufferIndex - 2;
                    reinterpret_cast<uint16_t*>(ib.data)[indexBufferOffset + (indexBufferIndex++)] = vertexBufferIndex - 3;
                    reinterpret_cast<uint16_t*>(ib.data)[indexBufferOffset + (indexBufferIndex++)] = vertexBufferIndex - 2;
                    reinterpret_cast<uint16_t*>(ib.data)[indexBufferOffset + (indexBufferIndex++)] = vertexBufferIndex - 1;

                    lastIt = it;
                    it++;
                } while (it != m_colorQuads.end() && it->m_depthTest == lastIt->m_depthTest);

                GraphicsContext::LayoutStream layout;
                layout.m_vertexStreams.push_back({ .m_transient = vb,
                                                   .m_startVertex = static_cast<uint32_t>(vertexBufferOffset),
                                                   .m_numVertices = static_cast<uint32_t>(vertexBufferIndex) });
                layout.m_indexStream = { .m_transient = ib,
                                         .m_startIndex = static_cast<uint32_t>(indexBufferOffset),
                                         .m_numIndices = static_cast<uint32_t>(indexBufferIndex) };
                indexBufferOffset += indexBufferIndex;
                vertexBufferOffset += vertexBufferIndex;

                GraphicsContext::ShaderProgram shaderProgram;
                shaderProgram.m_handle = m_colorProgram;
                shaderProgram.m_configuration.m_depthTest = lastIt->m_depthTest;

                shaderProgram.m_configuration.m_rgbBlendFunc =
                    CreateBlendFunction(BlendOperator::Add, BlendOperand::One, BlendOperand::One);
                shaderProgram.m_configuration.m_alphaBlendFunc =
                    CreateBlendFunction(BlendOperator::Add, BlendOperand::One, BlendOperand::One);
                shaderProgram.m_configuration.m_write = Write::RGB;

                GraphicsContext::DrawRequest request = {
                    layout,
                    shaderProgram,
                };
                m_context.Submit(view, request);
            }
        }

        if (!m_lineSegments.empty()) {
            GraphicsContext::ViewConfiguration viewConfiguration{ m_target };
            viewConfiguration.m_viewRect = { 0, 0, static_cast<uint16_t>(imageSize.x), static_cast<uint16_t>(imageSize.y) };
            viewConfiguration.m_projection = m_projection;
            viewConfiguration.m_view = m_view;
            const auto view = m_context.StartPass("Immediate - Lines", viewConfiguration);

            std::sort(m_lineSegments.begin(), m_lineSegments.end(), [](const LineSegmentRequest& a, const LineSegmentRequest& b) {
                return a.m_depthTest < b.m_depthTest;
            });

            const size_t numVertices = m_lineSegments.size() * 2;

            bgfx::TransientVertexBuffer vb;
            bgfx::TransientIndexBuffer ib;
            bgfx::allocTransientVertexBuffer(&vb, numVertices, layout::PositionColor::layout());
            bgfx::allocTransientIndexBuffer(&ib, numVertices);

            auto it = m_lineSegments.begin();
            auto lastIt = m_lineSegments.begin();
            size_t vertexBufferOffset = 0;
            size_t indexBufferOffset = 0;
            while (it != m_lineSegments.end()) {
                size_t vertexBufferIndex = 0;
                size_t indexBufferIndex = 0;
                do {
                    {
                        reinterpret_cast<uint16_t*>(ib.data)[indexBufferOffset + (indexBufferIndex++)] = vertexBufferIndex;
                        auto& posColor = reinterpret_cast<hpl::layout::PositionColor*>(vb.data)[vertexBufferOffset + (vertexBufferIndex++)];
                        posColor.m_pos[0] = it->m_start.x();
                        posColor.m_pos[1] = it->m_start.y();
                        posColor.m_pos[2] = it->m_start.z();

                        posColor.m_color[0] = it->m_color.x();
                        posColor.m_color[1] = it->m_color.y();
                        posColor.m_color[2] = it->m_color.z();
                        posColor.m_color[3] = it->m_color.w();
                    }

                    {
                        reinterpret_cast<uint16_t*>(ib.data)[indexBufferOffset + (indexBufferIndex++)] = vertexBufferIndex;
                        auto& posColor = reinterpret_cast<hpl::layout::PositionColor*>(vb.data)[vertexBufferOffset + (vertexBufferIndex++)];
                        posColor.m_pos[0] = it->m_end.x();
                        posColor.m_pos[1] = it->m_end.y();
                        posColor.m_pos[2] = it->m_end.z();

                        posColor.m_color[0] = it->m_color.x();
                        posColor.m_color[1] = it->m_color.y();
                        posColor.m_color[2] = it->m_color.z();
                        posColor.m_color[3] = it->m_color.w();
                    }

                    lastIt = it;
                    it++;
                } while (it != m_lineSegments.end() && it->m_depthTest == lastIt->m_depthTest);

                GraphicsContext::LayoutStream layout;
                layout.m_vertexStreams.push_back({ .m_transient = vb,
                                                   .m_startVertex = static_cast<uint32_t>(vertexBufferOffset),
                                                   .m_numVertices = static_cast<uint32_t>(vertexBufferIndex) });
                layout.m_indexStream = { .m_transient = ib,
                                         .m_startIndex = static_cast<uint32_t>(indexBufferOffset),
                                         .m_numIndices = static_cast<uint32_t>(indexBufferIndex) };
                indexBufferOffset += indexBufferIndex;
                vertexBufferOffset += vertexBufferIndex;

                layout.m_drawType = eVertexBufferDrawType_Line;

                GraphicsContext::ShaderProgram shaderProgram;
                shaderProgram.m_handle = m_colorProgram;
                shaderProgram.m_configuration.m_depthTest = lastIt->m_depthTest;
                shaderProgram.m_configuration.m_rgbBlendFunc =
                    CreateBlendFunction(BlendOperator::Add, BlendOperand::One, BlendOperand::One);
                shaderProgram.m_configuration.m_alphaBlendFunc =
                    CreateBlendFunction(BlendOperator::Add, BlendOperand::One, BlendOperand::One);
                shaderProgram.m_configuration.m_write = Write::RGB;

                GraphicsContext::DrawRequest request = {
                    layout,
                    shaderProgram,
                };
                m_context.Submit(view, request);
            }
        }
        if (m_debugMeshes.size() > 0) {
            GraphicsContext::ViewConfiguration viewConfiguration{ m_target };
            viewConfiguration.m_viewRect = { 0, 0, static_cast<uint16_t>(imageSize.x), static_cast<uint16_t>(imageSize.y) };
            viewConfiguration.m_projection = m_projection;
            viewConfiguration.m_view = m_view;
            const auto view = m_context.StartPass("Immediate - Lines", viewConfiguration);

            for (auto& mesh : m_debugMeshes) {
                GraphicsContext::ShaderProgram shaderProgram;
                shaderProgram.m_handle = m_meshColorProgram;
                shaderProgram.m_configuration.m_depthTest = mesh.m_depthTest;
                shaderProgram.m_configuration.m_rgbBlendFunc =
                    CreateBlendFunction(BlendOperator::Add, BlendOperand::One, BlendOperand::One);
                shaderProgram.m_configuration.m_alphaBlendFunc =
                    CreateBlendFunction(BlendOperator::Add, BlendOperand::One, BlendOperand::One);
                struct {
                    float m_r;
                    float m_g;
                    float m_b;
                    float m_a;
                } color = { mesh.m_color.r, mesh.m_color.g, mesh.m_color.b, mesh.m_color.a };
                shaderProgram.m_uniforms.push_back({ m_u_color, &color });
                shaderProgram.m_configuration.m_write = Write::RGB;
                shaderProgram.m_configuration.m_depthTest = DepthTest::LessEqual;
                GraphicsContext::DrawRequest request = {
                    mesh.m_layout,
                    shaderProgram,
                };
                m_context.Submit(view, request);
            }
        }

        if (!m_uvQuads.empty()) {
            size_t vertexBufferOffset = 0;
            size_t indexBufferOffset = 0;
            GraphicsContext::ViewConfiguration viewConfiguration{ m_target };
            viewConfiguration.m_viewRect = { 0, 0, static_cast<uint16_t>(imageSize.x), static_cast<uint16_t>(imageSize.y) };
            viewConfiguration.m_projection = m_projection;
            viewConfiguration.m_view = m_view;
            const auto view = m_context.StartPass("Immediate - Quad", viewConfiguration);
            const size_t numVertices = m_colorQuads.size() * 4;

            bgfx::TransientIndexBuffer ib;
            bgfx::allocTransientIndexBuffer(&ib, 6);
            uint16_t* indexBuffer = reinterpret_cast<uint16_t*>(ib.data);
            indexBuffer[0] = 0;
            indexBuffer[1] = 1;
            indexBuffer[2] = 2;
            indexBuffer[3] = 1;
            indexBuffer[4] = 2;
            indexBuffer[5] = 3;

            for (auto& quad : m_uvQuads) {
                bgfx::TransientVertexBuffer vb;
                bgfx::allocTransientVertexBuffer(&vb, 4, layout::PositionTexCoord0::layout());
                auto* vertexBuffer = reinterpret_cast<hpl::layout::PositionTexCoord0*>(vb.data);

                vertexBuffer[0] = {
                    .m_x = quad.m_v1.x(), .m_y = quad.m_v1.y(), .m_z = quad.m_v1.z(), .m_u = quad.m_uv0.x(), .m_v = quad.m_uv0.y()
                };
                vertexBuffer[1] = {
                    .m_x = quad.m_v2.x(), .m_y = quad.m_v2.y(), .m_z = quad.m_v2.z(), .m_u = quad.m_uv1.x(), .m_v = quad.m_uv0.y()
                };
                vertexBuffer[2] = {
                    .m_x = quad.m_v3.x(), .m_y = quad.m_v3.y(), .m_z = quad.m_v3.z(), .m_u = quad.m_uv0.x(), .m_v = quad.m_uv1.y()
                };
                vertexBuffer[3] = {
                    .m_x = quad.m_v4.x(), .m_y = quad.m_v4.y(), .m_z = quad.m_v4.z(), .m_u = quad.m_uv1.x(), .m_v = quad.m_uv1.y()
                };

                struct {
                    float m_r;
                    float m_g;
                    float m_b;
                    float m_a;
                } color = { quad.m_color.r, quad.m_color.g, quad.m_color.b, quad.m_color.a };

                GraphicsContext::ShaderProgram shaderProgram;
                shaderProgram.m_handle = m_uvProgram;
                shaderProgram.m_configuration.m_depthTest = quad.m_depthTest;

                // shaderProgram.m_configuration.m_cull = Cull::None;
                shaderProgram.m_configuration.m_rgbBlendFunc =
                    CreateBlendFunction(BlendOperator::Add, BlendOperand::One, BlendOperand::One);
                shaderProgram.m_configuration.m_alphaBlendFunc =
                    CreateBlendFunction(BlendOperator::Add, BlendOperand::One, BlendOperand::One);
                shaderProgram.m_uniforms.push_back({ m_u_color, &color });
                shaderProgram.m_textures.push_back({ m_s_diffuseMap, quad.m_uvImage->GetHandle(), 0 });
                shaderProgram.m_configuration.m_write = Write::RGB;

                GraphicsContext::LayoutStream layout;
                layout.m_vertexStreams.push_back({
                    .m_transient = vb,
                });
                layout.m_indexStream = { .m_transient = ib };

                GraphicsContext::DrawRequest request = {
                    layout,
                    shaderProgram,
                };
                m_context.Submit(view, request);
            }
        }
        m_colorTriangles.clear();
        m_lineSegments.clear();
        m_colorQuads.clear();
        m_uvQuads.clear();
    }
} // namespace hpl