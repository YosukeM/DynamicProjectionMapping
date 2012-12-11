//
//  CameraNode.cpp
//  emptyExample
//
//  Created by 森本 陽介 on 2012/11/11.
//
//

#include "CameraNode.h"

#define NOT_FOUND (-1)

CameraNode::LightPoint::LightPoint()
: lifetime(5) {
	
}

bool CameraNode::LightPoint::isPrediction() const {
	return lifetime != 5;
}


CameraNode::CameraNode()
	: width(720), height(480), near(0.01f), far(100.0f), fovy(29.8f)
{
	// 1280, 720
	// 720, 480
	setNearClip(near);
	setFarClip(far);
	setFov(fovy);
	//setScale(0.5f);
	
	vidGrabber.setDeviceID(0);
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
	bool isConsumedLps[lightPoints.size()];
	bool isConsumedBlobs[blobs.size()];
	for (int i = 0; i < lightPoints.size(); i++) {
		isConsumedLps[i] = false;
	}
	for (int i = 0; i < blobs.size(); i++) {
		isConsumedBlobs[i] = false;
	}
	
	float distanceSqs[blobs.size() * lightPoints.size()];
	{
		int i = 0;
		foreach (const auto& blob, blobs) {
			foreach (const auto& lp, lightPoints) {
				distanceSqs[i] = blob.centroid.distanceSquared(lp.position);
				i++;
			}
		}
	}
	
	float infinity = std::numeric_limits<float>::infinity();
	int consumed = 0;
	int lpSize = lightPoints.size();
	const float THRESHOLD = 30.0f;
	
	while (blobs.size() > consumed && lightPoints.size() > consumed) {
		float minDistanceSq = THRESHOLD * THRESHOLD;
		int minI = 0;
		for (int i = 0; i < blobs.size() * lightPoints.size(); i++) {
			if (distanceSqs[i] < minDistanceSq) {
				minDistanceSq = distanceSqs[i];
				minI = i;
			}
		}
		
		int minBlob = minI / lightPoints.size();
		int minLp = minI % lightPoints.size();
		isConsumedBlobs[minBlob] = true;
		isConsumedLps[minLp] = true;
		
		LightPoint lp;
		lp.position = blobs[minBlob].centroid;
		lp.id = lightPoints[minLp].id;
		lightPoints[minLp].id = -1;
		nextLightPoints.push_back(lp);
		
		for (int i = 0; i < blobs.size(); i++) {
			distanceSqs[lpSize * i + minLp] = infinity;
		}
		for (int i = 0; i < lightPoints.size(); i++) {
			distanceSqs[lpSize * minBlob + i] = infinity;
		}
		
		consumed++;
	}

	// 要らなくなったlightPointを削除
	{
		int i = 0;
		for (auto itr = lightPoints.begin(); itr != lightPoints.end(); ++itr) {
			if (!isConsumedLps[i]) {
				// 残った頂点の寿命を減らし、0より大きいものだけを残す
				itr->lifetime--;
				if (itr->lifetime > 0) {
					nextLightPoints.push_back(*itr);
				}
			}
			i++;
		}
	}
	
	// 対応づけられなかった頂点に新しいIDを割り振る
	{
		int i = 0;
		foreach (auto blob, blobs) {
			if (!isConsumedBlobs[i]) {
				LightPoint lp;
				lp.position = blob.centroid;
				lp.id = ++lastId;
				nextLightPoints.push_back(lp);
			}
			i++;
		}
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

CameraNode::LightPoint CameraNode::getLightPointById(int id) {
	foreach (const auto& lp, lightPoints) {
		if (lp.id == id) return lp;
	}
	return LightPoint();
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