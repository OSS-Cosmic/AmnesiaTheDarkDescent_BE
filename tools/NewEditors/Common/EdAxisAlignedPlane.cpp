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

#include "../Common/EdAxisAlignedPlane.h"
#include "../Common/EdActionMisc.h"

//--------------------------------------------------------

cVector3f iEdAxisAlignedPlane::mvPlaneNormals[3] =
{
	cVector3f(1,0,0),
	cVector3f(0,1,0),
	cVector3f(0,0,1)
};

tWString iEdAxisAlignedPlane::mvPlaneStrings[3] =
{
	_W("YZ"),
	_W("XZ"),
	_W("XY")
};

//--------------------------------------------------------

iEdAxisAlignedPlane::iEdAxisAlignedPlane()
{
	mbVisible = true;
	mvHeights = 0;

	mbUpdated = true;

	mNormal = ePlaneNormal_X;
}

const cPlanef& iEdAxisAlignedPlane::GetPlane()
{
	if(mbUpdated)
	{
		mbUpdated = false;

		cVector3f& vNormal = mvPlaneNormals[mNormal];
		mEnginePlane.FromNormalPoint(vNormal, vNormal*GetHeight());
	}

	return mEnginePlane;
}

//--------------------------------------------------------

void iEdAxisAlignedPlane::SetPlaneNormal(ePlaneNormal aNormal)
{
	if(mNormal==aNormal)
		return;

	mNormal = aNormal;
	mbUpdated=true;
	OnPlaneModified();
}

void iEdAxisAlignedPlane::SetHeight(float afHeight)
{
	if(mvHeights.v[mNormal]==afHeight)
		return;

	mvHeights.v[mNormal] = afHeight;
	mbUpdated=true;
	OnPlaneModified();
}

float iEdAxisAlignedPlane::GetHeight()
{
	return mvHeights.v[mNormal];
}

void iEdAxisAlignedPlane::SetHeights(const cVector3f& avX)
{
	if(mvHeights == avX)
		return;

	mvHeights = avX;
	mbUpdated=true;
	OnPlaneModified();
}

const cVector3f& iEdAxisAlignedPlane::GetHeights()
{
	return mvHeights;
}

const tWString& iEdAxisAlignedPlane::GetPlaneString()
{
	return mvPlaneStrings[mNormal];
}

ePlaneNormal iEdAxisAlignedPlane::GetPlaneNormalFromString(const tString& asX)
{
	tString sLowerCase = cString::ToLowerCase(asX);
	if(sLowerCase=="yz")
		return ePlaneNormal_X;
	else if(sLowerCase=="xy")
		return ePlaneNormal_Z;
	else
		return ePlaneNormal_Y;
}

cVector3f iEdAxisAlignedPlane::GetProjectedPosOnPlane(const cVector3f& avWorldPos)
{
	cVector3f vProjectedPos = avWorldPos;
	vProjectedPos.v[mNormal] = GetHeight();

	return vProjectedPos;
}

//--------------------------------------------------------

iEdAction* iEdAxisAlignedPlane::CreateSetHeightsAction(const cVector3f& avX)
{
	return hplNew(cEdActionPlaneChangeHeight,(this, avX));
}

iEdAction* iEdAxisAlignedPlane::CreateSetNormalAction(int alNormal)
{
	return hplNew(cEdActionPlaneChangeNormal,(this, alNormal));
}

bool iEdAxisAlignedPlane::PointOnPositiveSide(const cVector3f& avX)
{
	float fHeight = GetHeight();

	return avX.v[mNormal] >= fHeight;
}