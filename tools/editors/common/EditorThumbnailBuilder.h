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

#ifndef HPLEDITOR_EDITOR_THUMBNAIL_BUILDER_H
#define HPLEDITOR_EDITOR_THUMBNAIL_BUILDER_H

#include "../common/StdAfx.h"
#include "graphics/RenderTarget.h"
#include "system/SHA1.h"
#include <memory>

using namespace hpl;

//-------------------------------------------------------------------

class iEditorBase;
class cEditorThumbnailBuilder
{
public:
	cEditorThumbnailBuilder(iEditorBase* apEditor);


	void BuildThumbnailFromMeshEntity(cMeshEntity* apEntity, const tWString& asDestName);
	void BuildThumbnailFromMesh(const tWString& asMeshFilename, const tWString& asDestName);
	void BuildThumbnailFromImage(const tWString& asImageFilename, const tWString& asDestName);

	tString GetThumbnailNameFromFile(const tWString& asFile);
	tWString GetThumbnailNameFromFileW(const tWString& asFile);

	void VtxBufferAddNormals(const cMatrixf a_mtxTransform, iVertexBuffer *apVtxBuffer, cVector3f &avVecSum, float& afCount);
	void FocusCameraOnEntity(cMeshEntity* apEntity);

	void PreBuild();
	void PostBuild();

	void CleanUp();

	void Render();
	void SaveToFile(iTexture* apTexture);

protected:
	/////////////////////////////////////
	// Data
	cViewport* mpViewport;

	cLightPoint* mpCamLight;

	std::shared_ptr<LegacyRenderTarget> mpRenderTarget128;
	// std::shared_ptr<RenderTarget> mpFB64;
	// RenderViewport mpFB128;

	iEditorBase* mpEditor;

	tMeshEntityList mlstThumbnailEntities;

	SHA1 mSha;
};

//-------------------------------------------------------------------

#endif //HPLEDITOR_EDITOR_THUMBNAIL_BUILDER_H


