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

class CameraNode : public ofCamera {
	const int width, height;
	const float near, far, fovy;
	ofVideoGrabber vidGrabber;
	
public:
	CameraNode();
	
	void update();
	ofVideoGrabber* getVideoGrabber();
	
	void begin();
	void end();
	
	int getWidth() const { return width; };
	int getHeight() const { return height; };
	
	ofVec3f worldToCameraScreen(const ofVec3f&);
	
protected:
	virtual void customDraw();
};


#endif /* defined(__emptyExample__CameraNode__) */
