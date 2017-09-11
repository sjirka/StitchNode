#include "stitchCommand.h"
#include "stitchNode.h"

#include <maya\MArgDatabase.h>
#include <maya\MSelectionList.h>
#include <maya\MItSelectionList.h>
#include <maya\MFnDagNode.h>
#include <maya\MGlobal.h>
#include <maya\MSyntax.h>
#include <maya\MAngle.h>
#include <maya\MFnNurbsCurve.h>

StitchCommand::StitchCommand(){
}

void* StitchCommand::creator(){
	return new StitchCommand;
}

MStatus StitchCommand::doIt(const MArgList& argList){
	MStatus status;

	MArgDatabase argData(syntax(), argList, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MSelectionList selectionList;
	status = argData.getObjects(selectionList);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MItSelectionList iterMesh(selectionList, MFn::kMesh);
	MItSelectionList iterCurve(selectionList, MFn::kNurbsCurve);

	if (iterMesh.isDone() || iterCurve.isDone()){
		MGlobal::displayError("Select a mesh and a nurbs curve");
		return MStatus::kFailure;
	}

	// Create nodes ////////////////////////////////////////////////////////////////////////////////
	// Create plugin node
	m_node = m_dgMod.createNode(StitchNode::id, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	m_dgMod.doIt();
	MFnDependencyNode nodeFn(m_node);

	// Create transform
	MObject transform = m_dagMod.createNode("transform", MObject::kNullObj, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	MFnDagNode transFn(transform);
	transFn.setName("stitch#");

	// Create output mesh node
	MObject oNewMesh = m_dagMod.createNode("mesh", transform, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	MFnDagNode newMeshFn(oNewMesh);
	newMeshFn.setName("stitchShape#");

	// Set attributes
	if (argData.isFlagSet("-distance")) {
		double value = argData.flagArgumentDouble("-distance", 0);
		MPlug plug = nodeFn.findPlug(StitchNode::aDistance);
		plug.setDouble(value);
	}
	if (argData.isFlagSet("-count")) {
		int value = argData.flagArgumentInt("-count", 0);
		MPlug plug = nodeFn.findPlug(StitchNode::aCount);
		plug.setInt(value);
	}
	if (argData.isFlagSet("-length")) {
		double value = argData.flagArgumentDouble("-length", 0);
		MPlug plug = nodeFn.findPlug(StitchNode::aLength);
		plug.setDouble(value);
		plug = nodeFn.findPlug(StitchNode::aUseLength);
		plug.setBool(true);
	}
	if (argData.isFlagSet("-thickness")) {
		double value = argData.flagArgumentDouble("-thickness", 0);
		MPlug plug = nodeFn.findPlug(StitchNode::aThickness);
		plug.setDouble(value);
	}
	if (argData.isFlagSet("-skew")) {
		double value = argData.flagArgumentDouble("-skew", 0);
		MPlug plug = nodeFn.findPlug(StitchNode::aSkew);
		plug.setDouble(value);
	}
	if (argData.isFlagSet("-translate")) {
		double value = argData.flagArgumentDouble("-translate", 0);
		MPlug plug = nodeFn.findPlug(StitchNode::aTranslateX);
		plug.setDouble(value);

		value = argData.flagArgumentDouble("-translate", 1);
		plug = nodeFn.findPlug(StitchNode::aTranslateY);
		plug.setDouble(value);

		value = argData.flagArgumentDouble("-translate", 2);
		plug = nodeFn.findPlug(StitchNode::aTranslateZ);
		plug.setDouble(value);
	}
	if (argData.isFlagSet("-rotate")) {
		double value = argData.flagArgumentDouble("-rotate", 0);
		MPlug plug = nodeFn.findPlug(StitchNode::aRotateX);
		plug.setDouble(value);

		value = argData.flagArgumentDouble("-rotate", 1);
		plug = nodeFn.findPlug(StitchNode::aRotateY);
		plug.setDouble(value);

		value = argData.flagArgumentDouble("-rotate", 2);
		plug = nodeFn.findPlug(StitchNode::aRotateZ);
		plug.setDouble(value);
	}
	if (argData.isFlagSet("-scale")) {
		double value = argData.flagArgumentDouble("-scale", 0);
		MPlug plug = nodeFn.findPlug(StitchNode::aScaleX);
		plug.setDouble(value);

		value = argData.flagArgumentDouble("-scale", 1);
		plug = nodeFn.findPlug(StitchNode::aScaleY);
		plug.setDouble(value);

		value = argData.flagArgumentDouble("-scale", 2);
		plug = nodeFn.findPlug(StitchNode::aScaleZ);
		plug.setDouble(value);
	}

	// Make connections ///////////////////////////////////////////////////////
	// Input mesh to stitch node
	MDagPath meshPath;
	iterMesh.getDagPath(meshPath);
	MFnDagNode meshFn(meshPath);

	MPlug meshOutPlug = meshFn.findPlug("worldMesh", &status);
	meshOutPlug = meshOutPlug.elementByLogicalIndex(0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MPlug stitchInPlug = nodeFn.findPlug(StitchNode::aInMesh, &status);
	m_dagMod.connect(meshOutPlug, stitchInPlug);

	iterMesh.next();
	if (!iterMesh.isDone()) {
		MDagPath meshPath;
		iterMesh.getDagPath(meshPath);
		MFnDagNode meshFn(meshPath);

		MPlug meshOutPlug = meshFn.findPlug("worldMesh", &status);
		meshOutPlug = meshOutPlug.elementByLogicalIndex(0, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		MPlug stitchInPlug = nodeFn.findPlug(StitchNode::aGeometry, &status);
		m_dagMod.connect(meshOutPlug, stitchInPlug);

		nodeFn.findPlug(StitchNode::aUseGeometry, &status).setBool(true);
	}
	
	// Stitch node to output mesh
	MPlug stitchOutPlug = nodeFn.findPlug(StitchNode::aOutMesh, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MPlug meshInPlug = newMeshFn.findPlug("inMesh", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	status = m_dagMod.connect(stitchOutPlug, meshInPlug);

	// Connect Input curve to stich node
	MDagPath curvePath;
	iterCurve.getDagPath(curvePath);
	MFnNurbsCurve curveFn(curvePath);

	MPlug curveOutPlug = curveFn.findPlug("worldSpace", &status);
	curveOutPlug = curveOutPlug.elementByLogicalIndex(0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MPlug stitchCurvePlug = nodeFn.findPlug(StitchNode::aInputCurve, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = m_dagMod.connect(curveOutPlug, stitchCurvePlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	double rangeStart, rangeEnd;
	curveFn.getKnotDomain(rangeStart, rangeEnd);

	MPlug rangePlug = nodeFn.findPlug(StitchNode::aRange, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	if (rangePlug.isCompound())
	{
		MPlug rangeStartPlug = rangePlug.child(StitchNode::aRangeStart, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		rangeStartPlug.setDouble(rangeStart);

		MPlug rangeEndPlug = rangePlug.child(StitchNode::aRangeEnd, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		rangeEndPlug.setDouble(rangeEnd);
	}

	return redoIt();
}

MStatus StitchCommand::redoIt(){
	MStatus status;

	status = m_dagMod.doIt();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MFnDependencyNode nodeFn(m_node);
	MString command("select "+nodeFn.name());
	MGlobal::executeCommand(command, false, false);

	MPxCommand::setResult(nodeFn.name());

	return MStatus::kSuccess;
}

MStatus StitchCommand::undoIt(){
	MStatus status;
	
	
	status = m_dagMod.undoIt();
	CHECK_MSTATUS_AND_RETURN_IT(status);
	status = m_dgMod.undoIt();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return MStatus::kSuccess;
}

bool StitchCommand::isUndoable() const{
	return true;
}

MSyntax StitchCommand::newSyntax(){
	MSyntax syntax;

	syntax.addFlag("-d", "-distance", MSyntax::kDouble);
	syntax.addFlag("-c", "-count", MSyntax::kLong);
	syntax.addFlag("-l", "-length", MSyntax::kDouble);
	syntax.addFlag("-th", "-thickness", MSyntax::kDouble);
	syntax.addFlag("-sk", "-skew", MSyntax::kDouble);
	syntax.addFlag("-t", "-translate", MSyntax::kDouble, MSyntax::kDouble, MSyntax::kDouble);
	syntax.addFlag("-r", "-rotate", MSyntax::kDouble, MSyntax::kDouble, MSyntax::kDouble);
	syntax.addFlag("-s", "-scale", MSyntax::kDouble, MSyntax::kDouble, MSyntax::kDouble);

	syntax.useSelectionAsDefault(true);
	syntax.setObjectType(MSyntax::kSelectionList, 1);
	syntax.enableEdit(false);
	syntax.enableQuery(false);

	return syntax;
}