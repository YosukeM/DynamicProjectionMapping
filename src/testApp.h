#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxAssimpModelLoader.h"
#include <vector>
#include "ProjectorNode.h"
#include "CameraNode.h"
#include "ofVectorExtension.h"
#include <boost/unordered_set.hpp>
#include <boost/scoped_ptr.hpp>
#include "Content.h"

class testApp : public ofBaseApp{
public:
	typedef enum {
		E_MODE_EDITOR,
		E_MODE_PROJECTOR,
		E_MODE_CAMERA,
	} E_MODE;
	
	class ViewportVert {
	public:
		ofVec3f source;
		ofVec2f position;
	};
	
private:
	typedef int Id2d;
	
	typedef std::pair<Id2d, ofVec3f> VertMapping;
	
	E_MODE mode;
	bool isTracking = false;
	int counter = 0;
	
	boost::unordered_set<VertMapping> mappings;
	boost::unordered_set<ofVec3f> modelPointsConsidered;
	
	CameraNode camera;
	ProjectorNode projector;
	ofNode projectorBase;
	ofxAssimpModelLoader model;
	boost::scoped_ptr<Content> currentContent;

	ofEasyCam editorCamera;
	
public:
	ofNode modelBase;
	
private:
	std::vector<ViewportVert> getViewportVerts(const ofPoint& deltaPos = ofPoint(), const ofQuaternion& deltaRot = ofQuaternion(), bool reconsider_points = false);

	void updateMapping();
	
	// tracking
	float mappedPointsDifference(const std::vector<CameraNode::LightPoint>&, const std::vector<ViewportVert>&) const;
	
	void determineWhichModelPointsConsidered();
	
	void optimizeWithMappedTranslation(const ofVec3f max, int resolution);
	void optimizeWithMappedOrientation(const ofVec3f max, int resolution);
	
	// not tracking
	void activate();
	
	float pointsDifference(const std::vector<CameraNode::LightPoint>&, const std::vector<ViewportVert>&);
	void optimizeWithTranslation(const ofVec3f max, int resolution);
	void optimizeWithOrientation(const ofVec3f max, int resolution);
	
	void drawHowToUse();
	
public:
	void setup();
	void update();
	void draw();
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
};
