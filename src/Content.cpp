//
//  Content.cpp
//  emptyExample
//
//  Created by 森本 陽介 on 2012/11/30.
//
//

#include "Content.h"
#include "testApp.h"

void Content::setTestApp(class testApp* app) {
	parent = app;
}

ofVec3f Content::getModelPosition() const {
	return parent->modelBase.getPosition();
}

ofQuaternion Content::getModelOrientation() const {
	return parent->modelBase.getOrientationQuat();
}