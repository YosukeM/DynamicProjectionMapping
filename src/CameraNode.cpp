//
//  CameraNode.cpp
//  emptyExample
//
//  Created by 森本 陽介 on 2012/11/11.
//
//

#include "CameraNode.h"

CameraNode::CameraNode()
	: width(1280), height(720), near(0.01f), far(100.0f), fovy(29.8f)
{
	setNearClip(near);
	setFarClip(far);
	setFov(fovy);
	setScale(0.5f);
	
	vidGrabber.setDeviceID(1);
	vidGrabber.initGrabber(width, height);
}

void CameraNode::update() {
	vidGrabber.update();
}

ofVideoGrabber* CameraNode::getVideoGrabber() {
	return &vidGrabber;
}

void CameraNode::begin() {
	ofCamera::begin(ofRectangle(0.0f, 0.0f, width, height));
}

void CameraNode::end() {
	ofCamera::end();
}

void CameraNode::customDraw() {
	ofPushMatrix();
	ofScale(0.07f, 0.04f, 0.02f);
	ofBox(1.0f);
	ofPopMatrix();
	
	ofDrawAxis(0.1f);
	
	ofLine(ofPoint(), getLookAtDir());
}

ofVec3f CameraNode::worldToCameraScreen(const ofVec3f& p) {
	ofRectangle rect(0.0f, 0.0f, width, height);
	return worldToScreen(p, rect);
}