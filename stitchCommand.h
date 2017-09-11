#pragma once

#include <maya\MPxCommand.h>
#include <maya\MDagModifier.h>
#include <maya\MDGModifier.h>
#include <maya\MDagPath.h>


class StitchCommand : public MPxCommand
{
public:
	StitchCommand();
	virtual MStatus undoIt();
	virtual MStatus doIt(const MArgList& argList);
	virtual MStatus redoIt();
	virtual bool isUndoable() const;
	static void* creator();
	static MSyntax newSyntax();

private:
	MDagModifier m_dagMod;
	MDGModifier m_dgMod;
	MObject m_node;
};