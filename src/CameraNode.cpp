//
//  CameraNode.cpp
//  emptyExample
//
//  Created by 森本 陽介 on 2012/11/11.
//
//

#include "CameraNode.h"
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#define NOT_FOUND (-1)

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
	const int MIN_AREA = 5;
	const int MAX_AREA = width * height / 3;
	const int MAX_VERTS_NUM = 11;
	const float CONTI = 50.0f;
	
	vidGrabber.update();
	
	// VideoGrabberの画像を2値画像に変換
	colorImage.setFromPixels(vidGrabber.getPixelsRef());
	grayscaleImage.setFromColorImage(colorImage);
	grayscaleImage.threshold(10);
	
	// 2値画像を元に、輪郭検出を行う
	finder.findContours(grayscaleImage, MIN_AREA, MAX_AREA, MAX_VERTS_NUM, false);
	
	// 近くにある頂点どうしを対応づける
	auto blobs = finder.blobs;
	std::vector<LightPoint> nextLightPoints;
	for (auto itr = blobs.begin(); itr != blobs.end(); ) {
		int id = popNearestWithin(itr->centroid, lightPoints, CONTI);
		if (id != NOT_FOUND) {
			LightPoint lp;
			lp.position = ofVec2f(itr->centroid.x, itr->centroid.y);
			lp.id = id;
			nextLightPoints.push_back(lp);
			itr = blobs.erase(itr);
		} else {
			itr++;
		}
	}
	
	// 対応づけられなかった頂点に新しいIDを割り振る
	foreach (auto blob, blobs) {
		LightPoint lp;
		lp.position = blob.centroid;
		lp.id = ++lastId;
		nextLightPoints.push_back(lp);
	}
	
	lightPoints = nextLightPoints;
}

int CameraNode::popNearestWithin(ofVec2f pos, std::vector<LightPoint>& lightPoints, float within) {
	auto minItr = lightPoints.end();
	auto minDistanceSq = within*within;
	
	for (auto itr = lightPoints.begin(); itr != lightPoints.end(); itr++) {
		float distanceSq = pos.distanceSquared(itr->position);
		if (distanceSq < minDistanceSq) {
			minItr = itr;
		}
	}
	
	if (minItr != lightPoints.end()) {
		int id = minItr->id;
		lightPoints.erase(minItr);
		return id;
	} else {
		return NOT_FOUND;
	}
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