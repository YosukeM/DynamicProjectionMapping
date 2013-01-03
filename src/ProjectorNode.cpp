//
//  Projector.cpp
//  emptyExample
//
//  Created by 森本 陽介 on 2012/11/11.
//
//

#include "ProjectorNode.h"

ProjectorNode::ProjectorNode() {
	setNearClip(0.01f);
	setFarClip(100.0f);
	setFov(26.2f);	// 25.2
}

void ProjectorNode::customDraw() {
	ofPushMatrix();
	ofTranslate(0.04557438f, 0.007044787f, 0.06f);
	ofScale(0.27f, 0.1f, 0.21f);
	ofBox(1.0f);
	ofPopMatrix();
	
	ofDrawAxis(0.1f);
	
	ofLine(ofPoint(), getLookAtDir());
}