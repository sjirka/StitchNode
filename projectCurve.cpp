#include "projectCurve.h"

ProjectCurve::ProjectCurve()
{
}


ProjectCurve::~ProjectCurve()
{
}

MStatus ProjectCurve::setCurve(MObject& curve){
	if (curve.apiType() != MFn::kNurbsCurveGeom || curve.apiType() != MFn::kNurbsCube || curve.apiType() != MFn::kNurbsCurveData)
		return MStatus::kInvalidParameter;
	
	m_curve = curve;
	return MStatus::kSuccess;
}

MStatus ProjectCurve::setMesh(MObject& mesh) {
	if (mesh.apiType() != MFn::kMeshData || mesh.apiType() != MFn::kMesh || mesh.apiType() != MFn::kMeshGeom)
		return MStatus::kInvalidParameter;

	m_mesh = mesh;
	return MStatus::kSuccess;
}

MStatus ProjectCurve::project(MObjectArray& projectedCurves){
	MStatus status;

	if (m_mesh.isNull() || m_curve.isNull())
		return MStatus::kUnknownParameter;

	MFnNurbsCurve curveFn(m_curve);
	double paraMin, paraMax;
	curveFn.getKnotDomain(paraMin, paraMax);

	double paraStep = (paraMax - paraMin) / m_samples;

	MPointArray points;
	bool onSrf = false;

	for (int i = 0; i < (m_samples + 1); i++) {
		MPoint point, closestPoint;
		MVector normal;
		status = curveFn.getPointAtParam(paraMin + paraStep*i, point);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		bool prevOnSrf = onSrf;
		onSrf = closestPointOnMesh(m_mesh, point, closestPoint, normal);

		double distance = (onSrf)? (closestPoint - point).length() : 0;
		normal *= distance*2;

		points.append(point + normal);
		points.append(point - normal);

		if (!prevOnSrf && onSrf) {
			for (unsigned j = 0; j < points.length() - 2; j++)
				points[j] += (j%2)? normal : -normal;
		}


		if ((prevOnSrf && !onSrf) || (onSrf && i == m_samples)) {
			if (points.length() > 3) {
				MObject surface;

				status = generateNurbsSurface(points, surface, 2, 1);

				CHECK_MSTATUS_AND_RETURN_IT(status);
				projectedCurves.append(surface);
			}
			points.clear();
		}
	}

	return MStatus::kSuccess;
}

bool ProjectCurve::closestPointOnMesh(MObject& mesh, MPoint& point, MPoint& closestPoint, MVector& normal) {
	MFnMesh meshFn(mesh);

	meshFn.getClosestPointAndNormal(point, closestPoint, normal, MSpace::kWorld);

	MFloatPoint fClosestPoint;
	MMeshIsectAccelParams accelParams = meshFn.autoUniformGridParams();
	int hitFace, hitTriangle;
	float hitBary1, hitBary2, hitRayParam;

	bool overMesh = meshFn.closestIntersection(point, normal, NULL, NULL, false, MSpace::kObject, 99999.9f, true, &accelParams, fClosestPoint, &hitRayParam, &hitFace, &hitTriangle, &hitBary1, &hitBary2, (float)1e-6);

	return overMesh;
}

MStatus	ProjectCurve::generateNurbsSurface(MPointArray& points, MObject& surface, int degreeU, int degreeV) {
	MStatus status(MStatus::kSuccess);

	MFnNurbsSurfaceData dataCreator;
	surface = dataCreator.create(&status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	int numV = degreeV+1;
	int numU = points.length()/numV;

	MDoubleArray knotsU;
	MDoubleArray knotsV;

	int numKnotsU = numU + degreeU - 1;
	int numKnotsV = numV + degreeV - 1;

	for (int i = 0; i < numKnotsU; i++)
		knotsU.append(i / (double)(numKnotsU - 1));

	for (int i = 0; i < numKnotsV; i++)
		knotsV.append(i / (double)(numKnotsV - 1));

	for (int i = 1; i < degreeU; i++) {
		knotsU[i] = knotsU[0];
		knotsU[numKnotsU - 1 - i] = knotsU[numKnotsU - 1];
	}

	for (int i = 1; i < degreeV; i++) {
		knotsV[i] = knotsV[0];
		knotsV[numKnotsV - 1 - i] = knotsV[numKnotsV - 1];
	}

	MFnNurbsSurface surfaceFn;
	MFnNurbsSurface::Form form = MFnNurbsSurface::kOpen;
	surfaceFn.create(points, knotsU, knotsV, degreeU, degreeV, form, form, false, surface, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}