#pragma once
#include <maya/MPxManipContainer.h>

class StitchManipContainer : public MPxManipContainer
{
public:
	StitchManipContainer();
	virtual ~StitchManipContainer();

	static void * creator();
	static MStatus initialize();
	virtual MStatus createChildren();
	virtual MStatus connectToDependNode(const MObject& node);

	//Viewport 2.0 override
	virtual void preDrawUI(const M3dView& view);
	virtual void drawUI(MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext) const;

	virtual void draw(M3dView& view, const MDagPath& path, M3dView::DisplayStyle style, M3dView::DisplayStatus status);

	void getPoints(MPoint& pointA, MPoint& pointB);

	static MTypeId id;

private:
	MDagPath m_rangeManip;
	MObject m_node;
	MPlug	m_curve;

	MPoint m_pointA, m_pointB;
};