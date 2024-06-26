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

#include "EntityWrapperPrimitivePlane.h"

#include "EditorWorld.h"
#include "EditorBaseClasses.h"
#include "EditorWindowViewport.h"
#include "EditorHelper.h"

#include "EngineEntity.h"
#include "graphics/DebugDraw.h"
#include "graphics/MeshUtility.h"
#include "impl/LegacyVertexBuffer.h"


//---------------------------------------------------------------------------

cEntityWrapperTypePrimitivePlane::cEntityWrapperTypePrimitivePlane() : iEntityWrapperTypePrimitive("Plane", eEditorPrimitiveType_Plane)
{
	AddVec3f(ePrimitivePlaneVec3f_StartCorner, "StartCorner", 0, ePropCopyStep_PreEnt);
	AddVec3f(ePrimitivePlaneVec3f_EndCorner, "EndCorner",0, ePropCopyStep_PreEnt);

	AddVec3f(ePrimitivePlaneVec3f_TileAmount, "TileAmount", 1);
	AddVec3f(ePrimitivePlaneVec3f_TileOffset, "TileOffset", 0);
	AddFloat(ePrimitivePlaneFloat_TextureAngle, "TextureAngle", 0);
	AddBool(ePrimitivePlaneBool_AlignToWorld, "AlignToWorldCoords", false);
}

//---------------------------------------------------------------------------

iEntityWrapperData* cEntityWrapperTypePrimitivePlane::CreateSpecificData()
{
	return hplNew(cEntityWrapperDataPrimitivePlane,(this));
}

//---------------------------------------------------------------------------

cEntityWrapperDataPrimitivePlane::cEntityWrapperDataPrimitivePlane(iEntityWrapperType* apType) : iEntityWrapperData(apType)
{
}

//---------------------------------------------------------------------------

void cEntityWrapperDataPrimitivePlane::CopyFromEntity(iEntityWrapper* apEnt)
{
	iEntityWrapperData::CopyFromEntity(apEnt);
	cEntityWrapperPrimitivePlane* pPlane = (cEntityWrapperPrimitivePlane*)apEnt;
	mlNormalAxisIndex = pPlane->GetNormalAxisIndex();
	mvUVCorners = pPlane->GetUVCorners();
}

//---------------------------------------------------------------------------

bool cEntityWrapperDataPrimitivePlane::Load(cXmlElement* apElement)
{
	if(iEntityWrapperData::Load(apElement)==false)
		return false;

	cVector3f vEnd = (GetVec3f(ePrimitivePlaneVec3f_EndCorner)-GetVec3f(ePrimitivePlaneVec3f_StartCorner))*GetVec3f(eObjVec3f_Scale);
	SetVec3f(ePrimitivePlaneVec3f_StartCorner, 0);
	SetVec3f(ePrimitivePlaneVec3f_EndCorner, vEnd);

	cVector3f vScale = GetVec3f(eObjVec3f_Scale);
	for(int i=0;i<3;++i)
	{
		if(vEnd.v[i]==0) vScale.v[i] = 1;
		else
			vScale.v[i] = vEnd.v[i];
	}

	SetVec3f(eObjVec3f_Scale, vScale);

	return true;
}

//---------------------------------------------------------------------------

bool cEntityWrapperDataPrimitivePlane::SaveSpecific(cXmlElement* apElement)
{
	if(iEntityWrapperData::SaveSpecific(apElement)==false)
		return false;

	cVector3f vScale = GetVec3f(eObjVec3f_Scale);
	const cVector3f& vStartCorner = GetVec3f(ePrimitivePlaneVec3f_StartCorner);
	cVector3f vEndCorner;
	//Log("Saving plane\n");
	//Log("\tNormal axis index: %d\n", mlNormalAxisIndex);
	for(int i=0;i<3;++i)
	{
		//Log("\t\tvStartCorner.v[%d] == %f, vScale.v[%d] == %f\n", i, vStartCorner.v[i], i, vScale.v[i]);
		if(i!=mlNormalAxisIndex)
            vEndCorner.v[i] = vStartCorner.v[i] + vScale.v[i];
		else
			vEndCorner.v[i] = vStartCorner.v[i];
	}
	//Log("\tStartCorner: %s EndCorner: %s\n", vStartCorner.ToFileString().c_str(), vEndCorner.ToFileString().c_str());

	apElement->SetAttributeVector3f("EndCorner", vEndCorner);
	apElement->SetAttributeVector3f("Scale", cVector3f(1));

	for(int i=0;i<4;++i)
		apElement->SetAttributeVector2f("Corner" + cString::ToString(i+1) + "UV", mvUVCorners[i]);

	return true;
}

//---------------------------------------------------------------------------
/*
void cEntityWrapperDataPrimitivePlane::CopyFromEntity(iEntityWrapper* apEntity)
{
	cEntityWrapperDataPrimitive::CopyFromEntity(apEntity);
	cEntityWrapperPrimitivePlane* pEnt = (cEntityWrapperPrimitivePlane*)apEntity;

	mvStartCorner = pEnt->GetStartCorner();
	mvEndCorner = pEnt->GetEndCorner();

	mvTileAmount = pEnt->GetTileAmount();
	mvTileOffset = pEnt->GetTileOffset();
	mbAlignToWorldCoords = pEnt->GetAlignToWorld();
}

void cEntityWrapperDataPrimitivePlane::CopyToEntity(iEntityWrapper* apEntity)
{
	cEntityWrapperDataPrimitive::CopyToEntity(apEntity);
	cEntityWrapperPrimitivePlane* pEnt = (cEntityWrapperPrimitivePlane*)apEntity;

	pEnt->SetTileAmount(mvTileAmount);
	pEnt->SetTileOffset(mvTileOffset);
	pEnt->SetAlignToWorld(mbAlignToWorldCoords);
}
*/

/*
void cEntityWrapperDataPrimitivePlane::Load(cXmlElement* apElement)
{
	cEntityWrapperDataPrimitive::Load(apElement);

	mvStartCorner = apElement->GetAttributeVector3f("StartCorner");
	mvEndCorner = (apElement->GetAttributeVector3f("EndCorner")-mvStartCorner)*mvScale;
	mvStartCorner = 0;

	mvTileAmount = apElement->GetAttributeVector3f("TileAmount");
	mvTileOffset = apElement->GetAttributeVector3f("TileOffset");
	mbAlignToWorldCoords = apElement->GetAttributeBool("AlignToWorldCoords");

	for(int i=0;i<3;++i)
	{
		if(mvStartCorner.v[i]==0 && mvEndCorner.v[i]==0)
			mvScale.v[i]=1;
		else
		{
			mvScale.v[i] = mvEndCorner.v[i]-mvStartCorner.v[i];
            if(mvScale.v[i]==0) mvScale.v[i]=1;
		}
	}
}
*/

iEntityWrapper* cEntityWrapperDataPrimitivePlane::CreateSpecificEntity()
{
	return hplNew(cEntityWrapperPrimitivePlane,(this));
}

//------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------

cEntityWrapperPrimitivePlane::cEntityWrapperPrimitivePlane(iEntityWrapperData* apData) : iEntityWrapperPrimitive(apData)
{
	mlNormalAxisIndex=-1;
	mbAlignToWorldCoords = false;

	mvTileOffset = cVector3f(0);
	mvTileAmount = cVector3f(1);

	mvStartCorner = 999.0f;
	mvEndCorner = -999.0f;
}

//------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------

//------------------------------------------------------------------------

bool cEntityWrapperPrimitivePlane::GetProperty(int alPropID, bool& abX)
{
	if(iEntityWrapperPrimitive::GetProperty(alPropID, abX)==true)
		return true;

	switch(alPropID)
	{
	case ePrimitivePlaneBool_AlignToWorld:
		abX = GetAlignToWorld();
		break;
	default:
		return false;
	}

	return true;
}


bool cEntityWrapperPrimitivePlane::GetProperty(int alPropID, cVector3f& avX)
{
	if(iEntityWrapper::GetProperty(alPropID, avX)==true)
		return true;

	switch(alPropID)
	{
	case ePrimitivePlaneVec3f_TileAmount:
		avX = GetTileAmount();
		break;
	case ePrimitivePlaneVec3f_TileOffset:
		avX = GetTileOffset();
		break;
	case ePrimitivePlaneVec3f_StartCorner:
		avX = GetStartCorner();
		break;
	case ePrimitivePlaneVec3f_EndCorner:
		avX = GetEndCorner();
		break;
	default:
		return false;
	}

	return true;
}

bool cEntityWrapperPrimitivePlane::GetProperty(int alPropID, float& afX)
{
	switch(alPropID)
	{
	case ePrimitivePlaneFloat_TextureAngle:
		afX = GetTextureAngle();
		break;
	default:
		return iEntityWrapper::SetProperty(alPropID, afX);
	}

	return true;
}


bool cEntityWrapperPrimitivePlane::SetProperty(int alPropID, const bool& abX)
{
	if(iEntityWrapperPrimitive::SetProperty(alPropID, abX)==true)
		return true;

	switch(alPropID)
	{
	case ePrimitivePlaneBool_AlignToWorld:
		SetAlignToWorld(abX);
		break;
	default:
		return false;
	}

	return true;
}


bool cEntityWrapperPrimitivePlane::SetProperty(int alPropID, const cVector3f& avX)
{
	if(iEntityWrapper::SetProperty(alPropID, avX)==true)
		return true;

	switch(alPropID)
	{
	case ePrimitivePlaneVec3f_TileAmount:
		SetTileAmount(avX);
		break;
	case ePrimitivePlaneVec3f_TileOffset:
		SetTileOffset(avX);
		break;
	case ePrimitivePlaneVec3f_StartCorner:
		SetStartCorner(avX);
		break;
	case ePrimitivePlaneVec3f_EndCorner:
		SetEndCorner(avX);
		break;
	default:
		return false;
	}

	return true;
}

bool cEntityWrapperPrimitivePlane::SetProperty(int alPropID, const float& afX)
{
	switch(alPropID)
	{
	case ePrimitivePlaneFloat_TextureAngle:
		SetTextureAngle(afX);
		break;
	default:
		return iEntityWrapper::SetProperty(alPropID, afX);
	}

	return true;
}


void cEntityWrapperPrimitivePlane::Draw(cEditorWindowViewport* apViewport, DebugDraw* apFunctions, iEditorEditMode* apEditMode, bool abIsSelected,
										const cColor& aHighlightCol, const cColor& aDisabledCol)
{
	if(abIsSelected==false) return;

	// apFunctions->SetProgram(NULL);
	// apFunctions->SetMatrix(NULL);
	// apFunctions->SetTextureRange(NULL,0);
	// apFunctions->SetBlendMode(eMaterialBlendMode_None);

	cBoundingVolume* pBV = mpEngineEntity->GetRenderBV();

	apFunctions->DebugDrawBoxMinMax(cMath::ToForgeVec3(pBV->GetMin()), cMath::ToForgeVec3(pBV->GetMax()), Vector4(1,1,1,1));
}

//------------------------------------------------------------------------

void cEntityWrapperPrimitivePlane::SetTileAmount(const cVector3f& avTileAmount)
{
	if(mvTileAmount==avTileAmount) return;
	mvTileAmount = avTileAmount;

	UpdateUVMapping();
}

//------------------------------------------------------------------------

void cEntityWrapperPrimitivePlane::SetTileOffset(const cVector3f& avOffset)
{
	if(mvTileOffset==avOffset) return;
	mvTileOffset=avOffset;

	UpdateUVMapping();
}

//------------------------------------------------------------------------

void cEntityWrapperPrimitivePlane::SetTextureAngle(float afX)
{
	if(mfTextureAngle==afX) return;
	mfTextureAngle = afX;

	UpdateUVMapping();
}

//------------------------------------------------------------------------

void cEntityWrapperPrimitivePlane::SetAbsScale(const cVector3f& avScale, int alAxis)
{
	iEntityWrapper::SetAbsScale(avScale);
	for(int i=0;i<3;++i)
	{
		if(mvScale.v[i]<0)
			mvScale.v[i]=0;
	}
}

//------------------------------------------------------------------------

void cEntityWrapperPrimitivePlane::SetStartCorner(const cVector3f& avX)
{
	if(mvStartCorner==avX)
		return;

	mvStartCorner = avX;
	mlNormalAxisIndex = ComputeNormalAxisIndex();
}

void cEntityWrapperPrimitivePlane::SetEndCorner(const cVector3f& avX)
{
	if(mvEndCorner==avX)
		return;

	mvEndCorner = avX;
	mlNormalAxisIndex = ComputeNormalAxisIndex();
}

int cEntityWrapperPrimitivePlane::ComputeNormalAxisIndex()
{
	//Log("\tComputing plane normal axis index...\n");
	cVector3f vDiff = mvEndCorner-mvStartCorner;
	//Log("\tvDiff = %s\n", vDiff.ToFileString().c_str());
	for(int i=0;i<3;++i)
		if(vDiff.v[i]<kEpsilonf)  return i;

	return -1;
}

//------------------------------------------------------------------------

void cEntityWrapperPrimitivePlane::UpdateUVMapping() {
    // VERY IMPORTANT TO CLEAR THE CURRENT UVS (for god's sake)
    mvUVCorners.clear();

    cSubMesh* pVB = ((cEngineEntityGeneratedMesh*)mpEngineEntity)->GetSubMesh();

    auto posStream = pVB->getStreamBySemantic(ShaderSemantic::SEMANTIC_POSITION);
    auto texStream = pVB->getStreamBySemantic(ShaderSemantic::SEMANTIC_TEXCOORD0);
    auto posView = posStream->GetStructuredView<float3>();
    auto texView = texStream->GetStructuredView<float2>();

    for (int i = 0; i < posStream->m_numberElements; ++i) {
        cVector3 vCoords = cMath::FromForgeVector3(f3Tov3(posView.Get(i)));
        if (mbAlignToWorldCoords) {
            vCoords = cMath::MatrixMul(mmtxTransform, vCoords);
        } else {
            vCoords = cMath::MatrixMul(mmtxScale, vCoords);
        }
        vCoords = cMath::MatrixMul(cMath::MatrixRotateY(mfTextureAngle), vCoords);

        cVector2f res = cVector2f(0, 0);
        if (mlNormalAxisIndex == 0) {
            res.x = vCoords.v[1] * mvTileAmount.v[1] + mvTileOffset.v[1];
        } else {
            res.x = vCoords.v[0] * mvTileAmount.v[0] + mvTileOffset.v[0];
        }
        if (mlNormalAxisIndex == 2) {
            res.y = vCoords.v[1] * mvTileAmount.v[1] + mvTileOffset.v[1];
        } else {
            res.y = vCoords.v[2] * mvTileAmount.v[2] + mvTileOffset.v[2];
        }

        mvUVCorners.push_back(res);
        texView.Write(i, float2(res.x, res.y));
    }
    pVB->m_notify.Signal(cSubMesh::NotifyMesh{
        .m_semanticSize = 1,
        .m_semantic = { ShaderSemantic::SEMANTIC_TEXCOORD0 },
    });
}

//------------------------------------------------------------------------

cMesh* cEntityWrapperPrimitivePlane::CreatePrimitiveMesh()
{
	cVector3f vEndCorner = mvEndCorner;
	for(int i=0;i<3;++i)
	{
		if(vEndCorner.v[i]!=0)
			vEndCorner.v[i] /= vEndCorner.v[i];
	}

	auto position = GraphicsBuffer::BufferStructuredView<float3>();
    auto color = GraphicsBuffer::BufferStructuredView<float4>();
    auto normal = GraphicsBuffer::BufferStructuredView<float3>();
    auto uv = GraphicsBuffer::BufferStructuredView<float2>();
    auto tangent = GraphicsBuffer::BufferStructuredView<float3>();
    cSubMesh::IndexBufferInfo indexInfo;
    cSubMesh::StreamBufferInfo positionInfo;
    cSubMesh::StreamBufferInfo tangentInfo;
    cSubMesh::StreamBufferInfo colorInfo;
    cSubMesh::StreamBufferInfo normalInfo;
    cSubMesh::StreamBufferInfo textureInfo;
    GraphicsBuffer::BufferIndexView index = indexInfo.GetView();
    cSubMesh::StreamBufferInfo::InitializeBuffer<cSubMesh::PostionTrait>(&positionInfo,  &position);
    cSubMesh::StreamBufferInfo::InitializeBuffer<cSubMesh::ColorTrait>(&colorInfo, &color);
    cSubMesh::StreamBufferInfo::InitializeBuffer<cSubMesh::NormalTrait>(&normalInfo, &normal);
    cSubMesh::StreamBufferInfo::InitializeBuffer<cSubMesh::TextureTrait>(&textureInfo, &uv);
    cSubMesh::StreamBufferInfo::InitializeBuffer<cSubMesh::TangentTrait>(&tangentInfo, &tangent);

    MeshUtility::MeshCreateResult result = MeshUtility::CreatePlane(
        Vector3(0,0,0), cMath::ToForgeVec3(vEndCorner),
        Vector2(1,0), Vector2(0,0), Vector2(0,1), Vector2(1,1),
        &index,
        &position,
        &color,
        &normal,
        &uv,
        &tangent);

    positionInfo.m_numberElements = result.numVertices;
    tangentInfo.m_numberElements = result.numVertices;
    colorInfo.m_numberElements = result.numVertices;
    normalInfo.m_numberElements = result.numVertices;
    textureInfo.m_numberElements = result.numVertices;    		//Create the mesh
    indexInfo.m_numberElements = result.numIndices;
    std::vector<cSubMesh::StreamBufferInfo> vertexStreams;
    vertexStreams.push_back(std::move(positionInfo));
    vertexStreams.push_back(std::move(tangentInfo));
    vertexStreams.push_back(std::move(colorInfo));
    vertexStreams.push_back(std::move(normalInfo));
    vertexStreams.push_back(std::move(textureInfo));
    auto* resouces = GetEditorWorld()->GetEditor()->GetEngine()->GetResources();
    auto* graphics = GetEditorWorld()->GetEditor()->GetEngine()->GetGraphics();
	cMesh* pMesh = hplNew( cMesh, ("", _W(""), resouces ->GetMaterialManager(), resouces->GetAnimationManager()));
	cSubMesh* pSubMesh = pMesh->CreateSubMesh("Main");
	cMaterial *pMat = resouces->GetMaterialManager()->CreateMaterial(msMaterial);
	pSubMesh->SetMaterial(pMat);
	iVertexBuffer* pVtxBuffer = new LegacyVertexBuffer(eVertexBufferDrawType_Tri, eVertexBufferUsageType_Static, 0, 0);
	pSubMesh->SetStreamBuffers(pVtxBuffer, std::move(vertexStreams), std::move(indexInfo));

	return pMesh;
}

//------------------------------------------------------------------------

/*
void cEntityWrapperPrimitivePlane::SaveToElement(cXmlElement* apElement)
{
	EntityWrapperPrimitive::SaveToElement(apElement);
	apElement->SetValue("Plane");

	apElement->SetAttributeVector3f("Scale", cVector3f(1));

	apElement->SetAttributeVector3f("StartCorner", mvStartCorner);
	cVector3f vEndCorner;
	for(int i=0;i<3;++i)
	{
		if(i!=mlNormalAxisIndex)
            vEndCorner.v[i] = mvStartCorner.v[i] + mvScale.v[i];
		else
			vEndCorner.v[i] = mvStartCorner.v[i];
	}
	apElement->SetAttributeVector3f("EndCorner", vEndCorner);

	for(int i=0;i<4;++i)
		apElement->SetAttributeVector2f("Corner" + cString::ToString(i+1) + "UV", mvUVCorners[i]);

	apElement->SetAttributeVector3f("TileAmount", mvTileAmount);
	apElement->SetAttributeVector3f("TileOffset", mvTileOffset);
	apElement->SetAttributeBool("AlignToWorldCoords", mbAlignToWorldCoords);
}
*/
//------------------------------------------------------------------------

void cEntityWrapperPrimitivePlane::UpdateEntity()
{
	iEntityWrapper::UpdateEntity();

    UpdateUVMapping();
}

//------------------------------------------------------------------------
