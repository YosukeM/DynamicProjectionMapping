#include "testApp.h"
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#include <GLUT/GLUT.h>
#include "LevelMeterContent.h"
#include <boost/lexical_cast.hpp>

//--------------------------------------------------------------
void testApp::setup() {
	mode = E_MODE_CAMERA;
	ofBackground(0, 0, 0);
	ofSetWindowShape(1280, 800);
	
	// モデルの設定
	modelBase.setPosition(0.0f, -0.204f, -0.272324f);
	// modelBase.setOrientation(ofVec3f(310.7939f, 0.0f, 0.0f));
	modelBase.setScale(0.01f);
	modelBase.tilt(49.2f);
	
	model.setScaleNomalization(false);
	model.loadModel("cube11.obj");
	
	ofEnableNormalizedTexCoords();
	
	// プロジェクターとカメラの設定
	projectorBase.setPosition(-0.05585744f, 0.0265303f, 0.02653026f);
	//projectorBase.setOrientation(ofVec3f(45.0f, 0.0f, 0.0f));
	projectorBase.tilt(-45.0f);
	
	projector.setParent(projectorBase);
	projector.setPosition(-0.0455f, 0.0f, -0.0375f);
	//projector.setOrientation(ofVec3f(349.177f, 0.0f, 0.0f));
	
	camera.setParent(projectorBase);
	camera.setPosition(0.0f, 0.075f, -0.0575f);
	// camera.setOrientation(ofVec3f(11.0f, 0.0f, 0.0f));
	camera.tilt(3.0f);
	
	// エディタ用カメラ
	editorCamera.setDistance(1.0f);
	editorCamera.setNearClip(0.01f);
	editorCamera.setFarClip(100.0f);
	
	// コンテンツ
	currentContent.reset(new LevelMeterContent());
}

//--------------------------------------------------------------
void testApp::update() {
	camera.update();
	
	if (isTracking) {
		// 考慮すべき頂点のリストの生成
		determineWhichModelPointsConsidered();
		
		// 検出された頂点に近づくようにモデルを移動させる
		if (camera.lightPoints.size() > 0) {
			optimizeWithMappedTranslation(ofVec3f(0.3f, 0.3f, 0.3f), 4);
			optimizeWithMappedOrientation(ofVec3f(6.0f), 5);
			optimizeWithMappedTranslation(ofVec3f(0.05f, 0.05f, 0.05f), 4);
			optimizeWithMappedOrientation(ofVec3f(1.0f), 5);
			// std::cout << pointsDifference(lightpoints, getViewportVerts()) << std::endl;
			
			updateMapping();
		} else {
			isTracking = false;
		}
	}
	
	currentContent->update();
}

void testApp::updateMapping() {
	
	const float WITHIN = 15000.0f;
	auto lightPoints = camera.lightPoints;	// copy
	if (lightPoints.size() == 0) return;
	
	determineWhichModelPointsConsidered();
	auto viewportVerts = getViewportVerts();
	if (viewportVerts.size() == 0) return;
	
	mappings.clear();
	if (lightPoints.size() > viewportVerts.size()) {
		foreach (const auto& vv, viewportVerts) {
			float minDistanceSq = WITHIN * WITHIN;
			auto minItr = lightPoints.end();
			for (auto itr = lightPoints.begin(); itr != lightPoints.end(); itr++) {
				float distanceSq = vv.position.distanceSquared(itr->position);
				if (distanceSq <  minDistanceSq) {
					minItr = itr;
					minDistanceSq = distanceSq;
				}
			}
			if (minItr != lightPoints.end()) {
				VertMapping mapping;
				mapping.first = minItr->id;
				mapping.second = vv.source;
				mappings.insert(mapping);
				lightPoints.erase(minItr);
			}
		}
	} else {
		foreach (const auto& lp, lightPoints) {
			float minDistanceSq = WITHIN * WITHIN;
			auto minItr = viewportVerts.end();
			for (auto itr = viewportVerts.begin(); itr != viewportVerts.end(); itr++) {
				float distanceSq = lp.position.distanceSquared(itr->position);
				if (distanceSq < minDistanceSq) {
					minItr = itr;
					minDistanceSq = distanceSq;
				}
			}
			if (minItr != viewportVerts.end()) {
				VertMapping mapping;
				mapping.first = lp.id;
				mapping.second = minItr->source;
				mappings.insert(mapping);
				viewportVerts.erase(minItr);
			}
		}
	}
	
	isTracking = true;
}


//--------------------------------------------------------------
std::vector<testApp::ViewportVert> testApp::getViewportVerts(const ofPoint& deltaPos, const ofQuaternion& deltaRot, bool reconsider_points) {
	std::vector<ViewportVert> result;
	const auto points = modelPointsConsidered;
	
	auto originalTransform = modelBase.getLocalTransformMatrix();
	modelBase.move(deltaPos);
	modelBase.rotate(deltaRot);
	
	if (reconsider_points) {
		determineWhichModelPointsConsidered();
	}
	
	foreach (auto vert, modelPointsConsidered) {
		auto worldVert = vert * modelBase.getGlobalTransformMatrix();
		auto screenVert = camera.worldToCameraScreen(worldVert);
		ViewportVert vv;
		vv.position = screenVert;
		vv.source = vert;
		result.push_back(vv);
	}
	
	modelBase.setTransformMatrix(originalTransform);
	
	if (reconsider_points) {
		modelPointsConsidered = points;
	}
	
	return result;
}

//--------------------------------------------------------------
float testApp::mappedPointsDifference(const std::vector<CameraNode::LightPoint>& lightPoints, const std::vector<ViewportVert>& viewportVerts) const {
	using limfloat = std::numeric_limits<float>;
	
	auto difference = 0.0f;
	
	if (viewportVerts.size() == 0 || mappings.size() == 0) {
		return limfloat::infinity();
	}
	
	foreach (const auto& mapping, mappings) {
		foreach (const auto& lp, lightPoints) {
			if (lp.id == mapping.first) {
				foreach (const auto& vp, viewportVerts) {
					if (vp.source == mapping.second) {
						difference += vp.position.distanceSquared(lp.position);
						goto NEXT_MAPPING;
					}
				}
			}
		}
		NEXT_MAPPING: continue;
	}
	
	return std::sqrt(difference) + 10.0f * std::abs((int)lightPoints.size() - (int)viewportVerts.size());
}

void testApp::determineWhichModelPointsConsidered() {
	modelPointsConsidered.clear();
	auto transform = modelBase.getGlobalTransformMatrix();
	auto cameraTransformInv = camera.getModelViewMatrix();
	
	for (int i = 0; i < model.getNumMeshes(); i++) {
		auto mesh = model.getMesh(i);
		const auto& indices = mesh.getIndices();
		const auto& normals = mesh.getNormals();
		const auto& vertices = mesh.getVertices();
		switch (mesh.getMode()) {
			case OF_PRIMITIVE_TRIANGLES:
			{
				for (int j = 0; j < indices.size(); j += 3) {
					int i1 = indices[j], i2 = indices[j+1], i3 = indices[j+2];
					auto normal = (normals[i1] + normals[i2] + normals[i3]) / 3.0f;
					auto worldNormal = ofMatrix4x4::transform3x3(normal, transform);
					auto cameraNormal = ofMatrix4x4::transform3x3(worldNormal, cameraTransformInv);
					if (cameraNormal.z > 0.0f) {
						modelPointsConsidered.insert(vertices[i1]);
						modelPointsConsidered.insert(vertices[i2]);
						modelPointsConsidered.insert(vertices[i3]);
					}
				}
			}
				break;
			default:
				std::cout << "unsuported primitive mode";
				break;
		}
	}
}

void testApp::optimizeWithMappedTranslation(const ofVec3f max, int step) {
	using limfloat = std::numeric_limits<float>;
	
	float minDx = 0.0f, minDy = 0.0f, minDz = 0.0f;
	
	float minDistance = limfloat::infinity();
	
	for (int ix = -step; ix <= step; ix++) {
		float dx = max.x * ix / step;
		auto verts = getViewportVerts(ofVec3f(dx, 0.0f, 0.0f));
		auto distance = mappedPointsDifference(camera.lightPoints, verts);
		if (distance < minDistance) {
			minDistance = distance;
			minDx = dx;
		}
	}
	
	minDistance = limfloat::infinity();
	for (int iy = -step; iy <= step; iy++) {
		float dy = max.y * iy / step;
		auto verts = getViewportVerts(ofVec3f(minDx, dy, 0.0f));
		auto distance = mappedPointsDifference(camera.lightPoints, verts);
		if (distance < minDistance) {
			minDistance = distance;
			minDy = dy;
		}
	}
	
	minDistance = limfloat::infinity();
	for (int iz = -step; iz <= step; iz++) {
		float dz = max.z * iz / step;
		auto verts = getViewportVerts(ofVec3f(minDx, minDy, dz));
		auto distance = mappedPointsDifference(camera.lightPoints, verts);
		if (distance < minDistance) {
			minDistance = distance;
			minDz = dz;
		}
	}
	
	modelBase.setPosition(modelBase.getPosition() + ofVec3f(minDx, minDy, minDz));
}

void testApp::optimizeWithMappedOrientation(const ofVec3f max, int step) {
	auto originalOrientation = modelBase.getOrientationQuat();
	float minDistance = std::numeric_limits<float>::infinity();
	
	ofQuaternion minRot;
	
	for (int ix = -step; ix <= step; ix++) {
		float tilt = ix * max.x / step;
		for (int iy = -step; iy <= step; iy++) {
			float pan = iy * max.y / step;
			for (int iz = -step; iz <= step; iz++) {
				float roll = iz * max.z / step;
				auto rot = ofQuaternion(tilt, ofVec3f(1,0,0), pan, ofVec3f(0,1,0), roll, ofVec3f(0,0,1));
				auto verts = getViewportVerts(ofVec3f(), rot);
				auto distance = mappedPointsDifference(camera.lightPoints, verts);
				if (distance < minDistance) {
					minDistance = distance;
					minRot = rot;
				}
			}
		}
	}
	
	modelBase.setOrientation(modelBase.getOrientationQuat() * minRot);
}


float testApp::pointsDifference(const std::vector<CameraNode::LightPoint>& lightPointsOriginal, const std::vector<ViewportVert>& viewportVertsOriginal) {
	using limfloat = std::numeric_limits<float>;
	
	auto difference = 0.0f;
	
	if (viewportVertsOriginal.size() == 0) {
		return limfloat::infinity();
	}
	
	if (viewportVertsOriginal.size() < lightPointsOriginal.size()) {
		auto lightPoints = lightPointsOriginal;	// copy
		const auto& viewportVerts = viewportVertsOriginal;
		
		foreach (auto vert, viewportVerts) {
			int i = 0;
			int nearest = 0;
			float minSqDistance = limfloat::infinity();
			foreach (auto lp, lightPoints) {
				auto distance = vert.position.distanceSquared(lp.position);
				if (distance < minSqDistance) {
					nearest = i;
					minSqDistance = distance;
				}
				i++;
			}
			lightPoints[nearest].position = ofVec2f(limfloat::infinity(), limfloat::infinity());
			difference += minSqDistance;
		}
	} else {
		const auto& lightPoints = lightPointsOriginal;
		auto viewportVerts = viewportVertsOriginal;	// copy
		foreach (auto lp, lightPoints) {
			int i = 0;
			int nearest = 0;
			float minSqDistance = limfloat::infinity();
			foreach (auto vp, viewportVerts) {
				auto distance = vp.position.distanceSquared(lp.position);
				if (distance < minSqDistance) {
					nearest = i;
					minSqDistance = distance;
				}
				i++;
			}
			viewportVerts[nearest].position = ofVec2f(limfloat::infinity(), limfloat::infinity());
			difference += minSqDistance;
		}
	}
	
	return std::sqrt(difference) + 10.0f * std::abs((int)lightPointsOriginal.size() - (int)viewportVertsOriginal.size());
}

void testApp::optimizeWithTranslation(const ofVec3f max, int step) {
	using limfloat = std::numeric_limits<float>;
	
	float minDx = 0.0f, minDy = 0.0f, minDz = 0.0f;
	
	float minDistance = limfloat::infinity();
	
	for (int ix = -step; ix <= step; ix++) {
		float dx = max.x * ix / step;
		auto verts = getViewportVerts(ofVec3f(dx, 0.0f, 0.0f));
		auto distance = pointsDifference(camera.lightPoints, verts);
		if (distance < minDistance) {
			minDistance = distance;
			minDx = dx;
		}
	}
	
	minDistance = limfloat::infinity();
	for (int iy = -step; iy <= step; iy++) {
		float dy = max.y * iy / step;
		auto verts = getViewportVerts(ofVec3f(minDx, dy, 0.0f));
		auto distance = pointsDifference(camera.lightPoints, verts);
		if (distance < minDistance) {
			minDistance = distance;
			minDy = dy;
		}
	}
	
	minDistance = limfloat::infinity();
	for (int iz = -step; iz <= step; iz++) {
		float dz = max.z * iz / step;
		auto verts = getViewportVerts(ofVec3f(minDx, minDy, dz));
		auto distance = pointsDifference(camera.lightPoints, verts);
		if (distance < minDistance) {
			minDistance = distance;
			minDz = dz;
		}
	}

	modelBase.setPosition(modelBase.getPosition() + ofVec3f(minDx, minDy, minDz));
}

void testApp::optimizeWithOrientation(const ofVec3f max, int step) {
	auto originalOrientation = modelBase.getOrientationQuat();
	float minDistance = std::numeric_limits<float>::infinity();
	bool reconsider_points = true;
	
	ofQuaternion minRot;
	
	for (int ix = -step; ix <= step; ix++) {
		float tilt = ix * max.x / step;
		for (int iy = -step; iy <= step; iy++) {
			float pan = iy * max.y / step;
			for (int iz = -step; iz <= step; iz++) {
				float roll = iz * max.z / step;
				auto rot = ofQuaternion(tilt, ofVec3f(1,0,0), pan, ofVec3f(0,1,0), roll, ofVec3f(0,0,1));
				auto verts = getViewportVerts(ofVec3f(), rot, reconsider_points);
				auto distance = pointsDifference(camera.lightPoints, verts);
				if (distance < minDistance) {
					minDistance = distance;
					minRot = rot;
				}
			}
		}
	}
	
	modelBase.setOrientation(modelBase.getOrientationQuat() * minRot);
}


//--------------------------------------------------------------
void testApp::draw(){
	glEnable(GL_DEPTH_TEST);
	
	switch (mode) {
		case E_MODE_EDITOR:
		{
			editorCamera.begin();
			
			ofLight light[2];
			light[0].setPosition(3.0f, 10.0f, 0.0f);
			light[0].setDiffuseColor(ofFloatColor(0.5f));
			light[0].enable();
			light[1].setPosition(-2.0f, 2.0f, -3.0f);
			light[1].setDiffuseColor(ofFloatColor(0.2f));
			light[1].enable();
			
			modelBase.transformGL();
			model.drawFaces();
			modelBase.restoreTransformGL();
			
			ofEnableLighting();
			projector.draw();
			camera.draw();
			ofDisableLighting();
			
			editorCamera.end();
			
		}
			break;
			
		case E_MODE_PROJECTOR:
			projector.begin();
			
			modelBase.transformGL();
			currentContent->output.bind();
			model.drawFaces();
			currentContent->output.unbind();
			modelBase.restoreTransformGL();
			
			projector.end();
			break;
		
		case E_MODE_CAMERA:
		{
			glDisable(GL_DEPTH_TEST);
			camera.getVideoGrabber()->draw(0.0f, 0.0f);
			
			/*
			ofLight light[2];
			light[0].setPosition(3.0f, 10.0f, 0.0f);
			light[0].setDiffuseColor(ofFloatColor(0.5f));
			light[0].enable();
			light[1].setPosition(-2.0f, 2.0f, -3.0f);
			light[1].setDiffuseColor(ofFloatColor(0.2f));
			light[1].enable();
			
			glEnable(GL_DEPTH_TEST);
			ofEnableLighting();
			camera.begin();
			modelBase.transformGL();
			model.drawFaces();
			modelBase.restoreTransformGL();
			camera.end();
			ofDisableLighting();
			glDisable(GL_DEPTH_TEST);
			*/
			
			ofSetColor(255, 0, 0);
			for (auto lp : camera.lightPoints) {
				ofCircle(lp.position.x, lp.position.y, 3);
				ofDrawBitmapString(boost::lexical_cast<std::string>(lp.id), lp.position);
			}
			ofSetColor(255, 255, 255);
						
			ofEnableBlendMode(OF_BLENDMODE_ADD);
			ofSetColor(0, 0, 255);
			auto verts = getViewportVerts();
			for (auto vert : verts) {
				ofCircle(vert.position.x, vert.position.y, 3);
			}
			ofSetColor(255, 255, 255);
			ofDisableBlendMode();
		}
			break;
	}
	glDisable(GL_DEPTH_TEST);
	
	currentContent->draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	bool shift = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0;
	
	switch (key) {
		case OF_KEY_UP:
			if (!shift) {
				camera.tilt(0.2f);
			} else {
				camera.setFov(camera.getFov() + 0.2f);
			}
			break;
			
		case OF_KEY_DOWN:
			if (!shift) {
				camera.tilt(-0.2f);
			} else {
				camera.setFov(camera.getFov() - 0.2f);
			}
			break;
			
		case OF_KEY_RIGHT:
			if (!shift) {
				projector.setPosition(projector.getPosition() + ofVec3f(0.01f, 0.0f, 0.0f));
			} else {
				projector.setFov(projector.getFov() + 0.2f);
			}
			break;
			
		case OF_KEY_LEFT:
			if (!shift) {
				projector.setPosition(projector.getPosition() - ofVec3f(0.01f, 0.0f, 0.0f));
			} else {
				projector.setFov(projector.getFov() - 0.2f);
			}
			break;
			
		case 'a':
			activate();
			break;
		case 'c':
			mode = E_MODE_CAMERA;
			break;
		case 'e':
			mode = E_MODE_EDITOR;
			break;
		case 'p':
			mode = E_MODE_PROJECTOR;
			break;
		case 'f':
			ofToggleFullscreen();
			break;
	}
}

void testApp::activate() {
	if (camera.lightPoints.size() > 0) {
		for (int i = 0; i < 3; i++) {
			optimizeWithTranslation(ofVec3f(3.0f, 3.0f, 3.0f), 30);
			optimizeWithOrientation(ofVec3f(180.0f), 10);
			optimizeWithTranslation(ofVec3f(0.01f, 0.01f, 0.01f), 40);
			optimizeWithOrientation(ofVec3f(1.8f), 10);
			if (pointsDifference(camera.lightPoints, getViewportVerts()) < 30) {
				break;
			}
		}
		updateMapping();
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}