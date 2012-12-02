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
private:
	typedef enum {
		E_MODE_EDITOR,
		E_MODE_PROJECTOR,
		E_MODE_CAMERA,
	} E_MODE;
	
	E_MODE mode;
	
	ofxCvContourFinder finder;
	std::vector<ofVec2f> lightpoints;
	boost::unordered_set<ofVec3f> modelPointsConsidered;
	
	CameraNode camera;
	ProjectorNode projector;
	ofNode projectorBase;
	ofxAssimpModelLoader model;
	ofNode modelBase;
	boost::scoped_ptr<Content> currentContent;

	ofxCvColorImage colorImage;
	ofxCvGrayscaleImage grayscaleImage;
	
	ofEasyCam editorCamera;
	
	int vertsNum = 0;

	std::vector<ofVec2f> getViewportVerts(const ofPoint& deltaPos = ofPoint(), const ofQuaternion& deltaRot = ofQuaternion(), bool reconsider_points = false);
	float pointsDifference(const std::vector<ofVec2f>& lightPointsOriginal, const std::vector<ofVec2f>& viewportVertsOriginal) const;
	
	void determineWhichModelPointsConsidered();

	void applyOptimizationForTranslation(const ofVec3f max, int resolution);
	void applyOptimizationForOrientation(const ofVec3f max, int resolution, bool reconsider_points = false);
	
	void activate();
	
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
