//
//  CameraNode.h
//  emptyExample
//
//  Created by 森本 陽介 on 2012/11/11.
//
//

#ifndef __emptyExample__CameraNode__
#define __emptyExample__CameraNode__

#include "ofMain.h"
#include <vector>
#include "ofxOpenCv.h"

class CameraNode : public ofCamera {
public:
	class LightPoint {
	public:
		ofVec2f position;
		int id;
		int lifetime;
		LightPoint();
		bool isPrediction() const;
	};
protected:
	const int width, height;
	const float near, far, fovy;
	ofVideoGrabber vidGrabber;
	
	ofxCvContourFinder finder;
	ofxCvColorImage colorImage;
	ofxCvGrayscaleImage grayscaleImage;
	
	int lastId = 0;
	
public:
	std::vector<LightPoint> lightPoints;
	
	CameraNode();
	
	void update();
	ofVideoGrabber* getVideoGrabber();
	
	void begin();
	void end();
	
	int getWidth() const { return width; };
	int getHeight() const { return height; };
	
	ofVec3f worldToCameraScreen(const ofVec3f&);
	
	LightPoint getLightPointById(int id);
	
protected:
	virtual void customDraw();
	
	int popNearestWithin(ofVec2f, std::vector<LightPoint>&, float within);
};


#endif /* defined(__emptyExample__CameraNode__) */
