#include "stitchNode.h"
#include "stitchManipContainer.h"
#include "stitchCommand.h"

#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject obj)
{
	MStatus status;

	MFnPlugin fnPlugin(obj, "Stepan Jirka", "1.0", "Any");

	// Register node
	status = fnPlugin.registerNode(
		"stitchNode",
		StitchNode::id,
		StitchNode::creator,
		StitchNode::initialize);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Register command
	status = fnPlugin.registerCommand(
		"stitch",
		StitchCommand::creator,
		StitchCommand::newSyntax);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Register manipulator
	status = fnPlugin.registerNode(
		"stitchNodeManip",
		StitchManipContainer::id,
		&StitchManipContainer::creator,
		&StitchManipContainer::initialize,
		MPxNode::kManipContainer);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus status;

	MFnPlugin fnPlugin(obj);

	// Deregister Manip
	status = fnPlugin.deregisterNode(StitchManipContainer::id);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Deregister node
	status = fnPlugin.deregisterNode(StitchNode::id);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Deregister command
	status = fnPlugin.deregisterCommand("stitch");
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return MS::kSuccess;
}
