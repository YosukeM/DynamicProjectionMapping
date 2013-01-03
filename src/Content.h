//
//  Content.h
//  emptyExample
//
//  Created by 森本 陽介 on 2012/11/30.
//
//

#ifndef __emptyExample__Content__
#define __emptyExample__Content__

#include <ofImage.h>
#include "ofMain.h"

class testApp;

class Content {
protected:
	testApp* parent;
public:
	ofImage output;
	
	void setTestApp(class testApp*);
	
	ofVec3f getModelPosition() const;
	ofQuaternion getModelOrientation() const;
	
	void generateOutput(ofImage&, ofImage&, ofImage&, ofImage&, ofImage&, ofImage&, const ofColor& fillColor = ofColor::white);
	
	virtual void update() {};
	virtual void draw() {};
};

#endif /* defined(__emptyExample__Content__) */
