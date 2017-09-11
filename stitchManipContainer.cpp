#include "stitchManipContainer.h"
#include "stitchNode.h"

#include <maya\MFnCurveSegmentManip.h>
#include <maya\MPlugArray.h>
#include <maya\MGlobal.h>
#include <maya\MFnNurbsCurve.h>

MTypeId StitchManipContainer::id(0x00127888);

StitchManipContainer::StitchManipContainer() {
}

StitchManipContainer::~StitchManipContainer() {
}

void * StitchManipContainer::creator() {
	return new StitchManipContainer();
}

MStatus StitchManipContainer::initialize()
{
	MStatus status;
	status = MPxManipContainer::initialize();
	return status;
}

// Add manipulators/handles //////////////////////////////////////////////////////////////////////
MStatus StitchManipContainer::createChildren()
{
	MStatus status;

	// Rotate manip //////////////////////////////////////////////////////////////////////////////////
	MString manipTypeName = "stitchRangeManip";
	MString manipNameS = "rangeStart";
	MString manipNameE = "rangeEnd";
	m_rangeManip = addCurveSegmentManip(manipTypeName, manipNameS, manipNameE);

	return MStatus::kSuccess;
}

MStatus StitchManipContainer::connectToDependNode(const MObject& node)
{
	MStatus status;

	m_node = node;
	MFnDependencyNode nodeFn(node);

	// Connect distance manip /////////////////////////////////////////////////////////////////////
	MFnCurveSegmentManip rangeManipFn(m_rangeManip);
	
	MPlug rangePlug = nodeFn.findPlug(StitchNode::aRange, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	if (rangePlug.isCompound())
	{
		MPlug startPlug = rangePlug.child(StitchNode::aRangeStart, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		rangeManipFn.connectToStartParamPlug(startPlug);

		MPlug endPlug = rangePlug.child(StitchNode::aRangeEnd, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		rangeManipFn.connectToEndParamPlug(endPlug);
	}
	
	MPlug curvePlug = nodeFn.findPlug(StitchNode::aInputCurve, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	MPlugArray curvePlugs;
	curvePlug.connectedTo(curvePlugs, true, false, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	m_curve = curvePlugs[0];
	rangeManipFn.connectToCurvePlug(m_curve);

	finishAddingManips();
	MPxManipContainer::connectToDependNode(node);

	return MStatus::kSuccess;
}

void StitchManipContainer::draw(M3dView& view, const MDagPath& path, M3dView::DisplayStyle style, M3dView::DisplayStatus status){
	MPxManipContainer::draw(view, path, style, status);

	getPoints(m_pointA, m_pointB);
	view.drawText("Start", m_pointA, M3dView::TextPosition::kLeft);
	view.drawText("End", m_pointB, M3dView::TextPosition::kLeft);
}

//Viewport 2.0 Override /////////////////////////////////////////////////////////////////////////////

void StitchManipContainer::preDrawUI(const M3dView &view){
	getPoints(m_pointA, m_pointB);
}

void StitchManipContainer::drawUI(MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext) const{
	drawManager.text(m_pointA, "Start");
	drawManager.text(m_pointB, "End");
}

// Helper methods /////////////////////////////////////////////////////////////////////////////////

void StitchManipContainer::getPoints(MPoint& pointA, MPoint& pointB){
	MFnCurveSegmentManip manipFn(m_rangeManip);
	double start = manipFn.startParameter();
	double end = manipFn.endParameter();

	MObject curve;
	m_curve.getValue(curve);
	MFnNurbsCurve curveFn(curve);

	curveFn.getPointAtParam(start, pointA, MSpace::kWorld);
	curveFn.getPointAtParam(end, pointB, MSpace::kWorld);
}