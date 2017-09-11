#include "stitchNode.h"
#include "stitchMacros.h"

#include <maya\MFnDependencyNode.h>
#include <maya\MFnTypedAttribute.h>
#include <maya\MFnNumericAttribute.h>
#include <maya\MFnMesh.h>
#include <maya\MGlobal.h>
#include <maya\MFnMeshData.h>
#include <maya\MMeshSmoothOptions.h>
#include <maya\MFnNurbsCurve.h>
#include <maya\MFnNurbsCurveData.h>
#include <maya\MTransformationMatrix.h>
#include <maya\MDistance.h>
#include <maya\MAngle.h>
#include <maya\MEulerRotation.h>
#include <maya\MPxManipContainer.h>
#include <maya\MFnCompoundAttribute.h>
#include <maya\MFnEnumAttribute.h>
#include <maya\MPlugArray.h>
#include <maya\MTimer.h>

MTypeId     StitchNode::id(0x00127887);

// Base mesh
MObject		StitchNode::aInMesh;
MObject		StitchNode::aFlip;

// Lead curve
MObject		StitchNode::aInputCurve;
MObject		StitchNode::aUseRange;
MObject		StitchNode::aRange;
MObject		StitchNode::aRangeStart;
MObject		StitchNode::aRangeEnd;

// Output
MObject		StitchNode::aOutMesh;
MObject		StitchNode::aDivisions;

// Custom geometry
MObject		StitchNode::aGeometry;
MObject		StitchNode::aUseGeometry;

// Distribution
MObject		StitchNode::aDistance;
MObject		StitchNode::aCount;

// Settings
MObject		StitchNode::aLength;
MObject		StitchNode::aUseLength;
MObject		StitchNode::aThickness;
MObject		StitchNode::aSkew;

// Transformations
MObject		StitchNode::aRotate;
MObject		StitchNode::aRotateX;
MObject		StitchNode::aRotateY;
MObject		StitchNode::aRotateZ;

MObject		StitchNode::aScale;
MObject		StitchNode::aScaleX;
MObject		StitchNode::aScaleY;
MObject		StitchNode::aScaleZ;

MObject		StitchNode::aTranslate;
MObject		StitchNode::aTranslateX;
MObject		StitchNode::aTranslateY;
MObject		StitchNode::aTranslateZ;

StitchNode::StitchNode(){
}

StitchNode::~StitchNode() {
}

void* StitchNode::creator() {
	return new StitchNode();
}

void StitchNode::postConstructor(){
	m_meshDirty = m_distanceDirty = m_geometryDirty = m_curveDirty = m_nodeDirty = true;
	m_countDirty = false;
}

MStatus StitchNode::initialize() {
	MStatus status;
	
	MFnTypedAttribute tAttr;
	MFnNumericAttribute nAttr;
	MFnEnumAttribute eAttr;

	// Output mesh
	MAKE_TYPED_ATTR(aOutMesh, "outMesh", MFnData::kMesh, MObject::kNullObj, false);

	MAKE_NUMERIC_ATTR(aDivisions, "divisions", MFnNumericData::kInt, 1, 1, 3);
	ATTRIBUTE_AFFECTS(aDivisions, aOutMesh);

	// Base mesh
	MAKE_TYPED_ATTR(aInMesh, "inMesh", MFnData::kMesh, MObject::kNullObj, true);
	ATTRIBUTE_AFFECTS(aInMesh, aOutMesh);

	aFlip = nAttr.create("flip", "flip", MFnNumericData::kBoolean, false, &status);
	ADD_ATTRIBUTE(aFlip);
	ATTRIBUTE_AFFECTS(aFlip, aOutMesh);

	// Lead curve
	MAKE_TYPED_ATTR(aInputCurve, "inputCurve", MFnData::kNurbsCurve, MObject::kNullObj, true);
	ATTRIBUTE_AFFECTS(aInputCurve, aOutMesh);

	aUseRange = nAttr.create("useRange", "useRange", MFnNumericData::kBoolean, true, &status);
	ADD_ATTRIBUTE(aUseRange);
	ATTRIBUTE_AFFECTS(aUseRange, aOutMesh);

	aRangeStart = nAttr.create("rangeStart", "rangeStart", MFnNumericData::kDouble, 0.0, &status);
	aRangeEnd = nAttr.create("rangeEnd", "rangeEnd", MFnNumericData::kDouble, 1.0, &status);

	aRange = nAttr.create("range", "range", aRangeStart, aRangeEnd);
	ADD_ATTRIBUTE(aRange);

	ATTRIBUTE_AFFECTS(aRangeStart, aOutMesh);
	ATTRIBUTE_AFFECTS(aRangeEnd, aOutMesh);

	// Custom geometry
	MAKE_TYPED_ATTR(aGeometry, "geometry", MFnData::kMesh, MObject::kNullObj, true);
	ATTRIBUTE_AFFECTS(aGeometry, aOutMesh);

	aUseGeometry = nAttr.create("useGeometry", "useGeometry", MFnNumericData::kBoolean, false, &status);
	ADD_ATTRIBUTE(aUseGeometry);
	ATTRIBUTE_AFFECTS(aUseGeometry, aOutMesh);

	// Distribution
	MAKE_NUMERIC_ATTR(aDistance, "distance", MFnNumericData::kDouble, 5.0, 0.0, 10.0);
	ATTRIBUTE_AFFECTS(aDistance, aOutMesh);

	MAKE_NUMERIC_ATTR(aCount, "count", MFnNumericData::kInt, 100, 1, 100);
	ATTRIBUTE_AFFECTS(aCount, aOutMesh);

	// Settings
	MAKE_NUMERIC_ATTR(aLength, "length", MFnNumericData::kDouble, 5.0, 0.0, 10.0);
	ATTRIBUTE_AFFECTS(aLength, aOutMesh);

	aUseLength = nAttr.create("useLength", "useLength", MFnNumericData::kBoolean, false, &status);
	ADD_ATTRIBUTE(aUseLength);
	ATTRIBUTE_AFFECTS(aUseLength, aOutMesh);

	MAKE_NUMERIC_ATTR(aThickness, "thickness", MFnNumericData::kDouble, 0.5, 0.0, 2.0);
	ATTRIBUTE_AFFECTS(aThickness, aOutMesh);

	aSkew = nAttr.create("skew", "skew", MFnNumericData::kDouble, 0.2, &status);
	nAttr.setSoftMin(-2.0);
	nAttr.setSoftMax(2.0);
	ADD_ATTRIBUTE(aSkew);
	ATTRIBUTE_AFFECTS(aSkew, aOutMesh);

	// Transformations
	MAKE_CMP_NUMERIC_ATTR(aTranslate, "translate", MFnNumericData::kDouble, 0.0, aOutMesh);
	MAKE_CMP_NUMERIC_ATTR(aRotate, "rotate", MFnNumericData::kDouble, 0.0, aOutMesh);
	MAKE_CMP_NUMERIC_ATTR(aScale, "scale", MFnNumericData::kDouble, 1.0, aOutMesh);

	MPxManipContainer::addToManipConnectTable(id);

	return MS::kSuccess;
}

MStatus StitchNode::setDependentsDirty(const MPlug& plug, MPlugArray& plugArray)
{
	MStatus status(MStatus::kSuccess);

	if (plug == aInMesh)
		m_meshDirty = m_curveDirty = m_nodeDirty = true;

	if (plug == aInputCurve || plug == aUseRange || plug == aRangeStart || plug == aRangeEnd)
		m_curveDirty = m_nodeDirty = true;

	if (plug == aGeometry)
		m_geometryDirty = m_nodeDirty = true;

	if (plug == aDistance) {
		m_distanceDirty = m_nodeDirty = true;
		m_countDirty = false;
	}

	if (plug == aCount) {
		m_countDirty = m_nodeDirty = true;
		m_distanceDirty = false;
	}

	if (plug == aThickness ||
		plug == aSkew ||
		plug == aRotate ||
		plug == aFlip ||
		plug == aDivisions ||
		plug == aUseGeometry ||
		plug == aLength ||
		plug == aUseLength)
		m_nodeDirty = true;

	if (plug == aTranslateX || plug == aTranslateY || plug == aTranslateZ)
		m_nodeDirty = true;

	if (plug == aRotateX || plug == aRotateY || plug == aRotateZ)
		m_nodeDirty = true;

	if (plug == aScaleX || plug == aScaleY || plug == aScaleZ)
		m_nodeDirty = true;

	return status;
}

MStatus StitchNode::compute(const MPlug& plug, MDataBlock& data){
	MStatus status(MStatus::kSuccess);

	if (plug != aOutMesh)
		return MStatus::kUnknownParameter;

	// Node settings changed
	if (m_nodeDirty){
		status = batchLoadAttributes(data);
		CHECK_MSTATUS_AND_RETURN_IT(status);
	}

	// Input mesh changed
	if (m_meshDirty) {
		m_mesh = data.inputValue(aInMesh).asMeshTransformed();

		if (m_mesh.isNull())
			return MStatus::kSuccess;

		status = generateSmoothMesh(m_mesh, 2);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		m_meshDirty = false;
	}

	// Input curve changed
	if (m_curveDirty) {
		m_curve = data.inputValue(aInputCurve).asNurbsCurveTransformed();
		
		if (m_curve.isNull() || m_mesh.isNull())
			return MStatus::kSuccess;

		status = projectCurveOnMesh(m_mesh, m_curve, 100, m_projectedCurves);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		m_curveDirty = false;
	}

	// Custom geometry changed
	if (m_geometryDirty) {
		m_geometry = data.inputValue(aGeometry).asMeshTransformed();

		if (!m_geometry.isNull()){
			MFnDependencyNode nodeFn(thisMObject());
			MPlug geometry = nodeFn.findPlug(StitchNode::aGeometry, &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);
			MFnDagNode dagFn(geometry.source().node());
			MDagPath path;
			status = dagFn.getPath(path);
			CHECK_MSTATUS_AND_RETURN_IT(status);
			m_geoMatrix = path.inclusiveMatrix(&status);
			CHECK_MSTATUS_AND_RETURN_IT(status);
		}
	}

	// Build output mesh ////////////////////////////////////////////////////////////////////////
	if (m_nodeDirty){
		MFnNurbsCurve curveFn(m_curve);

		// Calculate distribution
		double paramMin, paramMax;
		curveFn.getKnotDomain(paramMin, paramMax);

		double domainStart = curveFn.findLengthFromParam((m_useRange) ? m_range[0] : paramMin);
		double domainEnd = curveFn.findLengthFromParam((m_useRange) ? m_range[1] : paramMax);
		double domain = domainEnd - domainStart;
		m_count = (m_distanceDirty && !m_countDirty) ? int(abs(round(domain / m_distance))) : m_count;
		m_distance = domain / m_count;
		
		// Generate mesh input data
		MPointArray polyPoints;
		MIntArray	polyCounts,
					polyIndices;

		for(unsigned i=0; i<m_projectedCurves.length(); i++)
			distributeStitches(m_projectedCurves[i], m_mesh, polyPoints, polyCounts, polyIndices);
		
		// Build mesh
		MObject meshData;
		generatePolyMesh(polyPoints, polyCounts, polyIndices, meshData);

		if (!m_useGeometry) {
			status = generateSmoothMesh(meshData, m_divisions);
			CHECK_MSTATUS_AND_RETURN_IT(status);
		}

		data.outputValue(aOutMesh).set(meshData);
		
		m_nodeDirty = false;
	}
	
	if (m_countDirty)
		data.outputValue(aDistance).setDouble(m_distance);
	if (m_distanceDirty)
		data.outputValue(aCount).setInt(m_count);

	data.setClean(plug);

	return status;
}

MStatus	StitchNode::batchLoadAttributes(MDataBlock& data) {
	MStatus status(MStatus::kSuccess);

	// Geometry
	m_flip = data.inputValue(aFlip, &status).asBool();
	CHECK_MSTATUS_AND_RETURN_IT(status);
	m_divisions = data.inputValue(aDivisions, &status).asInt();
	CHECK_MSTATUS_AND_RETURN_IT(status);
	m_useGeometry = data.inputValue(aUseGeometry, &status).asBool();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	m_useRange = data.inputValue(aUseRange, &status).asBool();
	CHECK_MSTATUS_AND_RETURN_IT(status);
	double2& range = data.inputValue(aRange, &status).asDouble2();
	CHECK_MSTATUS_AND_RETURN_IT(status);
	m_range[0] = range[0];
	m_range[1] = range[1];

	// Settings
	MDistance unitConversion;
	m_length = unitConversion.uiToInternal(data.inputValue(aLength, &status).asDouble());
	CHECK_MSTATUS_AND_RETURN_IT(status);
	m_useLength = data.inputValue(aUseLength, &status).asBool();
	CHECK_MSTATUS_AND_RETURN_IT(status);
	m_thickness = unitConversion.uiToInternal(data.inputValue(aThickness, &status).asDouble());
	CHECK_MSTATUS_AND_RETURN_IT(status);
	m_skew = unitConversion.uiToInternal(data.inputValue(aSkew, &status).asDouble());
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Transformations
	double3& translate = data.inputValue(aTranslate, &status).asDouble3();
	CHECK_MSTATUS_AND_RETURN_IT(status);
	double3& rotate = data.inputValue(aRotate, &status).asDouble3();
	CHECK_MSTATUS_AND_RETURN_IT(status);
	double3& scale = data.inputValue(aScale, &status).asDouble3();
	CHECK_MSTATUS_AND_RETURN_IT(status);
	for (unsigned i = 0; i < 3; i++) {
		m_translate[i] = unitConversion.uiToInternal(translate[i]);
		m_rotate[i] = MAngle(rotate[i], MAngle::kDegrees).asRadians();
		m_scale[i] = scale[i];
	}

	// Distribution
	m_distance = unitConversion.uiToInternal(data.inputValue(aDistance, &status).asDouble());
	CHECK_MSTATUS_AND_RETURN_IT(status);
	m_count = data.inputValue(aCount, &status).asInt();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}
	
MStatus StitchNode::distributeStitches(MObject& curve, MObject& mesh, MPointArray& points, MIntArray& counts, MIntArray& indices) {
	MStatus status(MStatus::kSuccess);

	MFnNurbsCurve curveFn(curve);
	MFnMesh meshFn(mesh);

	int count = int(round(curveFn.length() / m_distance));
	double distance = curveFn.length() / count;

	for (int i = 0; i < count; i++)
	{
		MPoint pointA, pointB;
		findPointAtLength(curve, (distance*i), pointA);
		findPointAtLength(curve, (distance*(i+1)), pointB);

		MVector normal, normalB;
		if (!closestPointOnMesh(m_mesh, pointA, normal))
			continue;
		else
			if (!closestPointOnMesh(m_mesh, pointB, normalB))
				continue;

		if (m_flip)
			normal *= -1;

		MVector tangent = pointB - pointA;
		double length = (m_useLength) ? m_length : tangent.length();
		MPoint position = pointA + tangent / 2;
		tangent.normalize();
		MVector cross = tangent^normal;

		double orientation[4][4] = {{ tangent.x,	tangent.y,	tangent.z,	0 },
									{ cross.x,		cross.y,	cross.z,	0 },
									{ normal.x,		normal.y,	normal.z,	0 },
									{ 0,			0,			0,			1 } };

		MMatrix orientationMatrix(orientation);
		MTransformationMatrix tMatrix(orientationMatrix.homogenize());
		tMatrix.addRotation(m_rotate, MTransformationMatrix::kXYZ, MSpace::kObject);
		tMatrix.addScale(m_scale, MSpace::kObject);
		position += tangent*m_translate[0] + cross*m_translate[1] + normal*m_translate[2];

		generateStitch(length, points, counts, indices, position, tMatrix.asMatrix());	
	}
	return status;
}

MStatus StitchNode::generateStitch(double length, MPointArray& points, MIntArray& counts, MIntArray& indices, MPoint& position, MMatrix& orientation){
	MStatus status(MStatus::kSuccess);

	double radius = m_thickness / 2;
	double outside = length / 2;
	double inside = outside - m_thickness;

	double polyPoints[16][4] = {{ -outside,		-radius+m_skew,		-m_thickness,		0 },
								{ outside,		-radius-m_skew,		-m_thickness,		0 },
								{ -outside,		radius+m_skew,		-m_thickness,		0 },
								{ outside,		radius-m_skew,		-m_thickness,		0 },
								{ -inside,		radius+m_skew/2,	0,					0 },
								{ inside,		radius-m_skew/2,	0,					0 },
								{ -inside,		-radius+m_skew/2,	0,					0 },
								{ inside,		-radius-m_skew/2,	0,					0 },
								{ inside,		-radius,			m_thickness,		0 },
								{ outside,		-radius,			m_thickness,		0 },
								{ inside,		radius,				m_thickness,		0 },
								{ outside,		radius,				m_thickness,		0 },
								{ -inside,		-radius,			m_thickness,		0 },
								{ -outside,		-radius,			m_thickness,		0 },
								{ -outside,		radius,				m_thickness,		0 },
								{ -inside,		radius,				m_thickness,		0 } };

	int	polyCounts[12] = { 4,4,4,4,4,4,4,4,4,4,4,4 };

	int	polyIndices[48] = {		15, 12, 6, 4,
								14, 15, 4, 2,
								13, 14, 2, 0,
								12, 13, 0, 6,
								11, 9, 1, 3,
								10, 11, 3, 5,
								8, 10, 5, 7,
								9, 8, 7, 1,
								0, 1, 7, 6,
								6, 7, 5, 4,
								4, 5, 3, 2,
								2, 3, 1, 0 };
	
	MPointArray geoPoints;
	MIntArray geoCounts, geoIndices;
	unsigned firstIndex = points.length();
	
	if (m_useGeometry && !m_geometry.isNull()){
		MFnMesh meshFn(m_geometry);
		meshFn.getPoints(geoPoints);
		meshFn.getVertices(geoCounts, geoIndices);
	}
	else{
		geoPoints = MPointArray(polyPoints, 16);
		geoCounts = MIntArray(polyCounts, 12);
		geoIndices = MIntArray(polyIndices, 48);
	}

	MTransformationMatrix tMatrix(m_geoMatrix.inverse());
	for (unsigned i = 0; i < geoPoints.length(); i++) {
		if (m_useGeometry && !m_geometry.isNull())
			geoPoints[i] += tMatrix.getTranslation(MSpace::kObject);
		geoPoints[i] *= orientation;
		geoPoints[i] += position;
		points.append(geoPoints[i]);
	}

	for (unsigned i = 0; i < geoCounts.length(); i++)
		counts.append(geoCounts[i]);

	for (unsigned i = 0; i < geoIndices.length(); i++)
		indices.append(firstIndex + geoIndices[i]);

	return status;
}

// Find closest point on mesh, returns true if the original point lies over the mesh //////////////
bool StitchNode::closestPointOnMesh(MObject& mesh, MPoint& point, MVector& normal){
	MFnMesh meshFn(mesh);

	MPoint closestPoint;
	meshFn.getClosestPointAndNormal(point, closestPoint, normal, MSpace::kWorld);

	MFloatPoint fClosestPoint;
	MMeshIsectAccelParams accelParams = meshFn.autoUniformGridParams();
	int hitFace, hitTriangle;
	float hitBary1, hitBary2, hitRayParam;

	bool overMesh = meshFn.closestIntersection(point, normal, NULL, NULL, false, MSpace::kObject, 99999.9f, true, &accelParams, fClosestPoint, &hitRayParam, &hitFace, &hitTriangle, &hitBary1, &hitBary2, (float)1e-6);

	point = closestPoint;
	return overMesh;
}

MStatus StitchNode::findPointAtLength(MObject& curve, double length, MPoint& point) {
	MStatus status(MStatus::kSuccess);

	MFnNurbsCurve curveFn(curve);
	
	double param = curveFn.findParamFromLength(length, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	status = curveFn.getPointAtParam(param, point, MSpace::kObject);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}

MStatus StitchNode::projectCurveOnMesh(MObject& mesh, MObject& curve, int samples, MObjectArray& projectedCurves) {
	MStatus status(MStatus::kSuccess);

	projectedCurves.clear();

	MFnNurbsCurve curveFn(curve, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	double paramMin, paramMax;
	status = curveFn.getKnotDomain(paramMin, paramMax);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	double domainStart = curveFn.findLengthFromParam((m_useRange) ? m_range[0] : paramMin, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	double domainEnd = curveFn.findLengthFromParam((m_useRange) ? m_range[1] : paramMax, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	double distance = (domainEnd - domainStart) / samples;

	MPointArray curvePoints;
	bool onSrf = false;

	for (int i = 0; i < (samples + 1); i++) {
		MPoint point;
		MVector normal;
		status = findPointAtLength(curve, (domainStart + distance*i), point);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		bool prevOnSrf = onSrf;
		onSrf = closestPointOnMesh(mesh, point, normal);

		if (!prevOnSrf && !onSrf)
			curvePoints.clear();

		curvePoints.append(point);

		if ((prevOnSrf && !onSrf) || (onSrf && i == samples)) {
			if (curvePoints.length() < 3)
				continue;
			MObject projectedCurve;
			status = generateNurbsCurve(curvePoints, projectedCurve);
			CHECK_MSTATUS_AND_RETURN_IT(status);
			projectedCurves.append(projectedCurve);
			curvePoints.clear();
		}
	}

	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Geometry ///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// Mesh creation
MStatus	StitchNode::generatePolyMesh(MPointArray& points, MIntArray& counts, MIntArray& indices, MObject& mesh) {
	MStatus status(MStatus::kSuccess);

	MFnMeshData dataCreator;
	mesh = dataCreator.create(&status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MFnMesh meshFn;
	meshFn.create(points.length(), counts.length(), points, counts, indices, mesh, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}

// Curve from CVs
MStatus	StitchNode::generateNurbsCurve(MPointArray& points, MObject& curve, int degree) {
	MStatus status(MStatus::kSuccess);

	MFnNurbsCurveData dataCreator;
	curve = dataCreator.create(&status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	int numCVs = points.length();
	
	if (numCVs <= degree)
		degree = numCVs - 1;

	MDoubleArray knots;
	int numKnots = numCVs + degree - 1;
	for (int i = 0; i < numKnots; i++)
		knots.append(i / (double)(numKnots - 1));

	for (int i = 1; i < degree; i++) {
		knots[i] = knots[0];
		knots[numKnots - 1 - i] = knots[numKnots - 1];
	}

	MFnNurbsCurve curveFn;
	curveFn.create(points, knots, degree, MFnNurbsCurve::kOpen, false, false, curve, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}

// In place mesh smoothing ////////////////////////////////////////////////////////////////////////
MStatus StitchNode::generateSmoothMesh(MObject& mesh, int divisions) {
	MStatus status(MStatus::kSuccess);

	MFnMesh meshFn(mesh);

	MMeshSmoothOptions smoothOptions;
	smoothOptions.setDivisions(divisions);
	smoothOptions.setKeepBorderEdge(false);

	meshFn.generateSmoothMesh(mesh, &smoothOptions, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}