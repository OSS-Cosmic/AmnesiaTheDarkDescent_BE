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

#include "graphics/GraphicsContext.h"
#include "graphics/Image.h"
#include "graphics/Material.h"
#include "graphics/MaterialType.h"
#include <bgfx/bgfx.h>
#include <cstdint>
#include <graphics/ShaderVariantCollection.h>

namespace hpl
{

    //---------------------------------------------------

    class iMaterialVars;

    //---------------------------------------------------
    // SOLID BASE
    //---------------------------------------------------

    namespace material::solid
    {
        enum DiffuseVariant : uint32_t
        {
            Diffuse_None = 0,
            Diffuse_NormalMap = 0x00001,
            Diffuse_SpecularMap = 0x00002,
            Diffuse_ParallaxMap = 0x00004,
            Diffuse_EnvMap = 0x00008
        };

		enum ZVariant : uint32_t
        {
            Z_None = 0,
            Z_UseAlphaMap = 0x00001,
            Z_UseDissolveFilter = 0x00002,
            Z_UseDissolveAlphaMap = 0x00004
        };
    }

    class iMaterialType_SolidBase : public iMaterialType
    {
    public:
        iMaterialType_SolidBase(cGraphics* apGraphics, cResources* apResources);
        ~iMaterialType_SolidBase();

        void CreateGlobalPrograms();

        iMaterialVars* CreateSpecificVariables()
        {
            return NULL;
        }
        void LoadVariables(cMaterial* apMaterial, cResourceVarsObject* apVars);
        void GetVariableValues(cMaterial* apMaterial, cResourceVarsObject* apVars);

        void CompileMaterialSpecifics(cMaterial* apMaterial);

    protected:
        virtual void CompileSolidSpecifics(cMaterial* apMaterial)
        {
        }

        virtual void LoadSpecificData() = 0;

        void LoadData();
        void DestroyData();

        ShaderVariantCollection<
            material::solid::DiffuseVariant::Diffuse_NormalMap | 
			material::solid::DiffuseVariant::Diffuse_SpecularMap |
            material::solid::DiffuseVariant::Diffuse_ParallaxMap | 
			material::solid::DiffuseVariant::Diffuse_EnvMap>
            m_diffuseProgramVariant;
		
        ShaderVariantCollection<
			material::solid::ZVariant::Z_UseAlphaMap |
			material::solid::ZVariant::Z_UseDissolveFilter |
			material::solid::ZVariant::Z_UseDissolveAlphaMap 
		> m_ZProgramVariant;
        bgfx::ProgramHandle m_illuminationProgram;

        bgfx::UniformHandle m_s_normalMap;
        bgfx::UniformHandle m_s_specularMap;
        bgfx::UniformHandle m_s_heightMap;
        bgfx::UniformHandle m_s_diffuseMap;
        bgfx::UniformHandle m_s_envMapAlphaMap;

        bgfx::UniformHandle m_s_dissolveMap;
        bgfx::UniformHandle m_s_dissolveAlphaMap;

        bgfx::UniformHandle m_s_envMap;

        bgfx::UniformHandle m_u_param;
        bgfx::UniformHandle m_u_mtxUv;
        bgfx::UniformHandle m_u_normalMtx;

        bgfx::UniformHandle m_u_mtxInvViewRotation;

        Image* m_dissolveImage = nullptr;

        iTexture* mpDissolveTexture;

        static float mfVirtualPositionAddScale;
    };

    //---------------------------------------------------
    // SOLID DIFFUSE
    //---------------------------------------------------

    class cMaterialType_SolidDiffuse_Vars : public iMaterialVars
    {
    public:
        cMaterialType_SolidDiffuse_Vars()
            : mfHeightMapScale(0.05f)
            , mfHeightMapBias(0.0f)
            , mbAlphaDissolveFilter(false)
        {
        }
        ~cMaterialType_SolidDiffuse_Vars()
        {
        }

        float mfHeightMapScale;
        float mfHeightMapBias;
        float mfFrenselBias;
        float mfFrenselPow;
        bool mbAlphaDissolveFilter;
    };

    //---------------------------------------------------

    class cMaterialType_SolidDiffuse : public iMaterialType_SolidBase
    {
    public:
        cMaterialType_SolidDiffuse(cGraphics* apGraphics, cResources* apResources);
        ~cMaterialType_SolidDiffuse();

        virtual void ResolveShaderProgram(
            eMaterialRenderMode aRenderMode,
            cMaterial* apMaterial,
            iRenderable* apObject,
            iRenderer* apRenderer, 
            std::function<void(GraphicsContext::ShaderProgram&)> handler) override;

        iMaterialVars* CreateSpecificVariables();
        void LoadVariables(cMaterial* apMaterial, cResourceVarsObject* apVars);
        void GetVariableValues(cMaterial* apMaterial, cResourceVarsObject* apVars);

    private:
        void CompileSolidSpecifics(cMaterial* apMaterial);

        void LoadSpecificData();
    };

    //---------------------------------------------------

}; // namespace hpl
