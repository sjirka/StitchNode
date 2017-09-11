#pragma once
#include <maya\MStatus.h>
#include <maya\MObject.h>
#include <maya\MObjectArray.h>
#include <maya\MVectorArray.h>
#include <maya\MPointArray.h>
#include <maya\MDoubleArray.h>

#include <maya\MFnNurbsCurve.h>
#include <maya\MFnMesh.h>
#include <maya\MFnNurbsSurfaceData.h>
#include <maya\MFnNurbsSurface.h>

class ProjectCurve
{
public:
	ProjectCurve();
	virtual ~ProjectCurve();

	MStatus setCurve(MObject& curve);
	MStatus setMesh(MObject& mesh);
	MStatus project(MObjectArray& projectedCurves);

private:
	bool	closestPointOnMesh(MObject& mesh, MPoint& point, MPoint& closestPoint, MVector& normal);
	MStatus	generateNurbsSurface(MPointArray& points, MObject& surface, int degreeU, int degreeV);

	MObject m_curve,
			m_mesh;

	int m_samples = 50;
};

