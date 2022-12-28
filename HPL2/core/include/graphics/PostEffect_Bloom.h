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

#include "bgfx/bgfx.h"
#include "graphics/PostEffect.h"
#include "graphics/RenderTarget.h"

namespace hpl {

	//------------------------------------------

	class cPostEffectParams_Bloom : public iPostEffectParams
	{
	public:
		cPostEffectParams_Bloom() : iPostEffectParams("Bloom"),
			mvRgbToIntensity(0.3f, 0.58f, 0.12f),
			mlBlurIterations(2),
			mfBlurSize(1.0f)
		{}

		kPostEffectParamsClassInit(cPostEffectParams_Bloom)

        cVector3f mvRgbToIntensity;

		float mfBlurSize;
		int mlBlurIterations;
	};

	//------------------------------------------

	class cPostEffectType_Bloom : public iPostEffectType
	{
	friend class cPostEffect_Bloom;
	public:
		cPostEffectType_Bloom(cGraphics *apGraphics, cResources *apResources);
		virtual ~cPostEffectType_Bloom();

		iPostEffect *CreatePostEffect(iPostEffectParams *apParams);

	private:
		bgfx::ProgramHandle m_blurProgram;
		bgfx::ProgramHandle m_bloomProgram;

		bgfx::UniformHandle m_u_blurMap;
		bgfx::UniformHandle m_u_diffuseMap;

		bgfx::UniformHandle m_u_rgbToIntensity;
		bgfx::UniformHandle m_u_param;

		iGpuProgram *mpBlurProgram[2];
		iGpuProgram *mpBloomProgram;
	};

	//------------------------------------------

	class cPostEffect_Bloom : public iPostEffect
	{
	public:
		cPostEffect_Bloom(cGraphics *apGraphics,cResources *apResources, iPostEffectType *apType);
		~cPostEffect_Bloom();

	private:
		void OnSetParams();
		iPostEffectParams *GetTypeSpecificParams() { return &mParams; }
		virtual void RenderEffect(GraphicsContext& context, Image& input, RenderTarget& target) override;

		iTexture* RenderEffect(iTexture *apInputTexture, iFrameBuffer *apFinalTempBuffer);
		
		std::array<RenderTarget, 2> m_blurTarget;

		cPostEffectType_Bloom *mpBloomType;
		cPostEffectParams_Bloom mParams;
	};

	//------------------------------------------

};
