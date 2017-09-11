#pragma once

#include <maya\MPxNode.h>
#include <maya\MPointArray.h>
#include <maya\MMatrix.h>
#include <maya\MObjectArray.h>

class StitchNode : public MPxNode
{
public:
	StitchNode();
	virtual				~StitchNode();
	static  MStatus		initialize();
	static  void*		creator();
	virtual void        postConstructor();

	virtual MStatus		compute(const MPlug& plug, MDataBlock& data);
	virtual MStatus		setDependentsDirty(const MPlug& plug, MPlugArray& plugArray);

	MStatus				distributeStitches(	MObject& curve,
											MObject& mesh,
											MPointArray& points,
											MIntArray& counts,
											MIntArray& indices);

	MStatus				generateStitch(		double length,
											MPointArray& points,
											MIntArray& counts,
											MIntArray& indices,
											MPoint& position,
											MMatrix& orientation);

	static MStatus		generateSmoothMesh(	MObject& mesh,
											int divisions);

	bool				closestPointOnMesh(	MObject& mesh,
											MPoint& point,
											MVector& normal);

	MStatus				findPointAtLength(	MObject& curve,
											double length,
											MPoint& point);

	MStatus				generatePolyMesh(	MPointArray& points,
											MIntArray& counts,
											MIntArray& indices,
											MObject& mesh);

	MStatus				generateNurbsCurve(	MPointArray& points, MObject& curve, int degree = 2);

	MStatus				projectCurveOnMesh(	MObject& mesh,
											MObject& curve,
											int samples,
											MObjectArray& projectedCurves);

	MStatus				batchLoadAttributes(MDataBlock& data);


	static MTypeId		id;

	// Base mesh
	static MObject		aInMesh;
	static MObject		aFlip;
	
	// Lead curve
	static MObject		aInputCurve;
	static MObject		aUseRange;
	static MObject		aRange;
	static MObject		aRangeStart;
	static MObject		aRangeEnd;
	
	// Output
	static MObject		aOutMesh;
	static MObject		aDivisions;

	// Custom geometry
	static MObject		aGeometry;
	static MObject		aUseGeometry;
	
	// Distribution
	static MObject		aDistance;
	static MObject		aCount;

	// Settings
	static MObject		aLength;
	static MObject		aUseLength;
	static MObject		aThickness;
	static MObject		aSkew;

	static MObject		aRotate;
	static MObject		aRotateX;
	static MObject		aRotateY;
	static MObject		aRotateZ;

	static MObject		aScale;
	static MObject		aScaleX;
	static MObject		aScaleY;
	static MObject		aScaleZ;

	static MObject		aTranslate;
	static MObject		aTranslateX;
	static MObject		aTranslateY;
	static MObject		aTranslateZ;

private:
	MObject m_mesh,
			m_curve,
			m_component,
			m_geometry;

	MObjectArray m_projectedCurves;

	MPointArray m_points;

	MMatrix m_geoMatrix;

	bool	m_meshDirty,
			m_curveDirty,
			m_nodeDirty,
			m_geometryDirty,

			m_distanceDirty,
			m_countDirty,

			m_useGeometry,
			m_useLength,
			m_useRange,
		
			m_flip;

	int		m_count,
			m_divisions;

	double	m_distance,
			m_length,
			m_thickness,
			m_skew,

			m_translate[3],
			m_rotate[3],
			m_scale[3],

			m_range[2];
};