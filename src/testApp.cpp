#include "testApp.h"
#ifndef _WIN32
#include <GLUT/GLUT.h>
#else
#include "glut.h"
#endif
#include "DiceContent.h"
#include <boost/lexical_cast.hpp>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm/find.hpp>

using namespace boost::range;
typedef std::numeric_limits<float> limfloat;

//--------------------------------------------------------------
void testApp::setup() {
	ofSetFrameRate(60);
	
	mode = E_MODE_CAMERA;
	ofBackground(0, 0, 0);
	ofSetWindowShape(1280, 800);
	ofSetFullscreen(true);
	
	// モデルの設定
	modelBase.setPosition(0.0f, -0.204f, -0.272324f);
	modelBase.setScale(0.01f);
	modelBase.tilt(49.2f);
	
	model.setScaleNomalization(false);
	model.loadModel("cube_ver5.obj");
	
	ofEnableNormalizedTexCoords();
	
	// プロジェクターとカメラの設定
	projectorBase.setPosition(-0.05585744f, 0.0265303f, 0.02653026f);
	projectorBase.tilt(-45.0f);
	
	projector.setParent(projectorBase);
	projector.setPosition(-0.0455f, 0.0f, -0.0375f);
	
	camera.setParent(projectorBase);
	camera.setPosition(0.0f, 0.075f, -0.0575f);
	camera.tilt(-3.0f);
	
	// エディタ用カメラ
	editorCamera.setDistance(1.0f);
	editorCamera.setNearClip(0.01f);
	editorCamera.setFarClip(100.0f);
	
	// コンテンツ
	currentContent.reset(new DiceContent());
}

//--------------------------------------------------------------
void testApp::update() {
	camera.update();
	
	if (isTracking) {
		// 考慮すべき頂点のリストの生成
		determineWhichModelPointsConsidered();
		
		// 検出された頂点に近づくようにモデルを移動させる
		if (camera.lightPoints.size() > 0) {
			optimizeWithMappedTranslation(ofVec3f(0.3f, 0.3f, 0.3f), 6);
			optimizeWithMappedOrientation(ofVec3f(5.0f), 5);
			//optimizeWithMapping(ofVec3f(0.05f), 1, ofVec3f(1.0f), 2);
			optimizeWithMappedTranslation(ofVec3f(0.05f, 0.05f, 0.05f), 6);
			optimizeWithMappedOrientation(ofVec3f(1.0f), 5);
			optimizeWithMappedTranslation(ofVec3f(0.02f, 0.02f, 0.02f), 6);
			
			updateMapping();
			
			//predictNextLightPoints();
			
		} else {
			isTracking = false;
		}
	} else {
		predictor.clearHistory();
	}
	
	currentContent->update();
}

void testApp::predictNextLightPoints() {
	if (predictor.canPredict()) {
		MovementPredictor::Transform transform;
		transform.position = modelBase.getPosition();
		transform.orientation = modelBase.getOrientationQuat();
		predictor.pushHistory(transform);
		
		transform = predictor.predictNext();
		auto vps = getViewportVerts(
			transform.position - modelBase.getPosition(),
			transform.orientation * modelBase.getOrientationQuat().inverse()
		);
		foreach (const auto& vp, vps) {
			auto mapItr = find_if(mappings, [vp] (const VertMapping& vm) {
				return vm.second == vp.source;
			});
			if (mapItr != mappings.end()) {
				auto lpItr = find_if(camera.lightPoints, [mapItr] (const CameraNode::LightPoint& lp) {
					return mapItr->first == lp.id;
				});
				if (lpItr != camera.lightPoints.end()) {
					lpItr->position = vp.position;
				}
			}
		}
	}
}

void testApp::updateMapping() {
	const float WITHIN = 50.0f;
	
	auto lightPoints = camera.lightPoints;	// copy
	if (lightPoints.size() == 0) return;
	
	determineWhichModelPointsConsidered();
	auto viewportVerts = getViewportVerts();
	if (viewportVerts.size() == 0) return;
	
	// すでにmappingされているLightPointsとViewportVertsを削除
	// ロストしたmappingを削除
	for (auto itr = mappings.begin(); itr != mappings.end(); ) {
		auto lpItr = find_if(lightPoints, [itr] (const CameraNode::LightPoint& lp) {
			return lp.id == itr->first;
		});
		auto vpItr = find_if(viewportVerts, [itr] (const ViewportVert& vv) {
			return vv.source == itr->second;
		});
		if (lpItr == lightPoints.end() || vpItr == viewportVerts.end()) {
			itr = mappings.erase(itr);
		} else if (lpItr->position.distanceSquared(vpItr->position) > WITHIN*WITHIN) {
			itr = mappings.erase(itr);
		} else {
			lightPoints.erase(lpItr);
			viewportVerts.erase(vpItr);
			itr++;
		}
	}
	
	// 予測によるLightPointを削除
	remove_if(lightPoints, [] (const CameraNode::LightPoint& lp) {
		return lp.isPrediction();
	});
	
	// すべてのLpとVpの距離を計算
	float* distanceSqs = new float[lightPoints.size() * viewportVerts.size()];
	for (int i = 0; i < lightPoints.size() * viewportVerts.size(); i++) {
		distanceSqs[i] = 0.0f;
		int lpI = i % lightPoints.size();
		int vvI = i / lightPoints.size();
		distanceSqs[i] = lightPoints[lpI].position.distanceSquared(viewportVerts[vvI].position);
	}
	
	// 距離の近いものから順にマッピングに追加
	int consumed = 0;
	while (consumed < lightPoints.size() && consumed < viewportVerts.size()) {
		int minI = -1;
		float minDistSq = WITHIN*WITHIN;
		
		for (int i = 0; i < lightPoints.size() * viewportVerts.size(); i++) {
			if (minDistSq > distanceSqs[i]) {
				minI = i;
				minDistSq = distanceSqs[i];
			}
		}
		
		if (minI != -1) {
			int lpI = minI % lightPoints.size();
			int vvI = minI / lightPoints.size();
			for (int i = 0; i < viewportVerts.size(); i++) {
				distanceSqs[i * lightPoints.size() + lpI] = std::numeric_limits<float>::infinity();
			}
			for (int i = 0; i < lightPoints.size(); i++) {
				distanceSqs[vvI * lightPoints.size() + i] = std::numeric_limits<float>::infinity();
			}
			
			mappings.insert(VertMapping(lightPoints[lpI].id, viewportVerts[vvI].source));
			consumed++;
		} else {
			break;
		}
	}
	
	delete[] distanceSqs;
	
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
	
	auto difference = 0.0f;
	
	if (viewportVerts.size() == 0 || mappings.size() == 0) {
		return limfloat::infinity();
	}
	
	foreach (const auto& mapping, mappings) {
		auto lpitr = find_if(lightPoints, [mapping] (const CameraNode::LightPoint& lp) {
			return mapping.first == lp.id;
		});
		auto vpitr = find_if(viewportVerts, [mapping] (const ViewportVert& vp) {
			return mapping.second == vp.source;
		});
		if (lpitr != lightPoints.end() && vpitr != viewportVerts.end() && !lpitr->isPrediction()) {
			difference += vpitr->position.distanceSquared(lpitr->position);
		} else if (vpitr != viewportVerts.end()) {
			difference += 3.0f * vpitr->position.distanceSquared(lpitr->position);
		}
	}
	
	return difference;
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

void testApp::optimizeWithMapping(const ofVec3f& maxTrans, int transRes, const ofVec3f& maxRot, int rotRes) {
	auto originalOrientation = modelBase.getOrientationQuat();
	float minDistance = std::numeric_limits<float>::infinity();
	bool reconsider_points = true;
	
	ofVec3f minTrans;
	ofQuaternion minRot;

	for (int tx = -transRes; tx <= transRes; tx++) {
		for (int ty = -transRes; ty <= transRes; ty++) {
			for (int tz = -transRes; tz <= transRes; tz++) {
				ofVec3f trans(maxTrans.x * tx / transRes, maxTrans.y * ty / transRes, maxTrans.z * tz / transRes);
				for (int ix = -rotRes; ix <= rotRes; ix++) {
					float tilt = ix * maxRot.x / rotRes;
					for (int iy = -rotRes; iy <= rotRes; iy++) {
						float pan = iy * maxRot.y / rotRes;
						for (int iz = -rotRes; iz <= rotRes; iz++) {
							float roll = iz * maxRot.z / rotRes;
							auto rot = ofQuaternion(tilt, ofVec3f(1,0,0), pan, ofVec3f(0,1,0), roll, ofVec3f(0,0,1));
							auto verts = getViewportVerts(trans, rot, reconsider_points);
							auto distance = pointsDifference(camera.lightPoints, verts);
							if (distance < minDistance) {
								minDistance = distance;
								minRot = rot;
								minTrans = trans;
							}
						}
					}
				}
			}
		}
	}
	
	modelBase.move(minTrans);
	modelBase.setOrientation(modelBase.getOrientationQuat() * minRot);
}

void testApp::optimizeWithTranslation(const ofVec3f max, int step) {
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

void testApp::optimize(const ofVec3f& maxTrans, int transRes, const ofVec3f& maxRot, int rotRes) {
	auto originalOrientation = modelBase.getOrientationQuat();
	float minDistance = std::numeric_limits<float>::infinity();
	bool reconsider_points = true;
	
	ofVec3f minTrans;
	ofQuaternion minRot;
	
	
	for (int tx = -transRes; tx <= transRes; tx++) {
		for (int ty = -transRes; ty <= transRes; ty++) {
			for (int tz = -transRes; tz <= transRes; tz++) {
				ofVec3f trans(maxTrans.x * tx / transRes, maxTrans.y * ty / transRes, maxTrans.z * tz / transRes);
				for (int ix = -rotRes; ix <= rotRes; ix++) {
					float tilt = ix * maxRot.x / rotRes;
					for (int iy = -rotRes; iy <= rotRes; iy++) {
						float pan = iy * maxRot.y / rotRes;
						for (int iz = -rotRes; iz <= rotRes; iz++) {
							float roll = iz * maxRot.z / rotRes;
							auto rot = ofQuaternion(tilt, ofVec3f(1,0,0), pan, ofVec3f(0,1,0), roll, ofVec3f(0,0,1));
							auto verts = getViewportVerts(trans, rot, reconsider_points);
							auto distance = pointsDifference(camera.lightPoints, verts);
							if (distance < minDistance) {
								minDistance = distance;
								minRot = rot;
								minTrans = trans;
							}
						}
					}
				}
			}
		}
	}
	
	modelBase.move(minTrans);
	modelBase.setOrientation(modelBase.getOrientationQuat() * minRot);
}


void testApp::drawHowToUse() {
	if (isTracking) {
		ofDrawBitmapString("TRACKING.", ofVec2f(5, 25));
	} else {
		ofDrawBitmapString("NOT TRACKING. Press A key to detect vertices.", ofVec2f(5, 25));
	}
	
	ofDrawBitmapString(
		"A: Reset vertices detection\n\
		C: Camera Mode: show the webcam's view.\n\
		E: Editor Mode: show the computer supeculatation of the world.\n\
		P: Projection Mode\n\
		up/down: tilt the camera.\n\
		shift+up/down: change the fov of the camera.\n\
		left/right: move the projector relative to the camera.\n\
		shift+left/right: change the fov of the projector.",
		ofVec2f(5, 50)
	);
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
			
			drawHowToUse();
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
			
			// カメラの映像を描く
			camera.getVideoGrabber()->draw(0.0f, 0.0f);
			
			// モデルのWireframeを描く
			// glEnable(GL_DEPTH_TEST);
			camera.begin();
			modelBase.transformGL();
			//model.drawFaces();
			model.drawWireframe();
			modelBase.restoreTransformGL();
			camera.end();
			// glDisable(GL_DEPTH_TEST);
			
			// LightPointsを赤で描く
			ofSetColor(255, 0, 0);
			foreach (auto lp, camera.lightPoints) {
				ofCircle(lp.position.x, lp.position.y, 3);
				ofDrawBitmapString(boost::lexical_cast<std::string>(lp.id), lp.position + ofVec2f(5, -5));
			}
			ofSetColor(255, 255, 255);
			
			// ViewportVertsを青で描く
			ofEnableBlendMode(OF_BLENDMODE_ADD);
			ofSetColor(0, 0, 255);
			auto verts = getViewportVerts();
			foreach (auto vert, verts) {
				ofCircle(vert.position.x, vert.position.y, 3);
			}
			ofSetColor(255, 255, 255);
			ofDisableBlendMode();
			ofEnableBlendMode(OF_BLENDMODE_ALPHA);
			
			foreach (const auto& mapping, mappings) {
				auto lpitr = find_if(camera.lightPoints, [mapping] (const CameraNode::LightPoint& lp) {
					return mapping.first == lp.id;
				});
				auto vertitr = find_if(verts, [mapping] (const ViewportVert& vv) {
					return vv.source == mapping.second;
				});
				if (vertitr != verts.end() && lpitr != camera.lightPoints.end()) {
					ofLine(lpitr->position, vertitr->position);
				}
			}
			
			drawHowToUse();
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
			if (ofGetKeyPressed('r')) {
				if (ofGetKeyPressed('x')) {
					modelBase.rotate(1.0f, ofVec3f(0.01f, 0.0f, 0.0f));
				} else if (ofGetKeyPressed('y')) {
					modelBase.rotate(1.0f, ofVec3f(0.0f, 0.01f, 0.0f));
				} else if (ofGetKeyPressed('z')) {
					modelBase.rotate(1.0f, ofVec3f(0.0f, 0.0f, 0.01f));
				}
			} else if (ofGetKeyPressed('x')) {
				modelBase.move(0.01f, 0.0f, 0.0f);
			} else if (ofGetKeyPressed('y')) {
				modelBase.move(0.0f, 0.01f, 0.0f);
			} else if (ofGetKeyPressed('z')) {
				modelBase.move(0.0f, 0.0f, 0.01f);
			} else if (!shift) {
				camera.tilt(0.2f);
			} else {
				camera.setFov(camera.getFov() + 0.2f);
			}
			break;
			
		case OF_KEY_DOWN:
			if (ofGetKeyPressed('r')) {
				if (ofGetKeyPressed('x')) {
					modelBase.rotate(-1.0f, ofVec3f(0.01f, 0.0f, 0.0f));
				} else if (ofGetKeyPressed('y')) {
					modelBase.rotate(-1.0f, ofVec3f(0.0f, 0.01f, 0.0f));
				} else if (ofGetKeyPressed('z')) {
					modelBase.rotate(-1.0f, ofVec3f(0.0f, 0.0f, 0.01f));
				}
			} else if (ofGetKeyPressed('x')) {
				modelBase.move(-0.01f, 0.0f, 0.0f);
			} else if (ofGetKeyPressed('y')) {
				modelBase.move(0.0f, -0.01f, 0.0f);
			} else if (ofGetKeyPressed('z')) {
				modelBase.move(0.0f, 0.0f, -0.01f);
			} else if (!shift) {
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
			
		case 'A':
			mappings.clear();
			isTracking = false;
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
			optimizeWithTranslation(ofVec3f(3.0f), 5);
			optimizeWithOrientation(ofVec3f(90.0f), 3);
			optimizeWithTranslation(ofVec3f(0.9f), 5);
			optimizeWithOrientation(ofVec3f(30.0f), 10);
			optimize(ofVec3f(0.1f), 2, ofVec3f(2.8), 1);
			optimizeWithTranslation(ofVec3f(0.05f), 5);
			optimizeWithOrientation(ofVec3f(2.0f), 5);
			if (pointsDifference(camera.lightPoints, getViewportVerts()) < 30) {
				break;
			}
		}
		mappings.clear();
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