//
//  LevelMeterContent.cpp
//  emptyExample
//
//  Created by 森本 陽介 on 2012/12/02.
//
//

#include "LevelMeterContent.h"

LevelMeterContent::LevelMeterContent() {
	video.loadMovie("levelmeter.mov");
	video.setLoopState(OF_LOOP_NORMAL);
	video.play();
	
	// auto edge = std::max(video.getWidth(), video.getHeight());
	// output.allocate(edge, edge, OF_IMAGE_COLOR);
}

void LevelMeterContent::update() {
	video.update();
	output.setFromPixels(video.getPixels(), video.getWidth(), video.getHeight(), OF_IMAGE_COLOR);
	
	ofVec3f pos = getModelPosition();
	ofQuaternion quat = getModelOrientation();
}

void LevelMeterContent::draw() {
//	video.draw(ofPoint());
}