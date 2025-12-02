
//--------------------------------------------------------------
//
//  Kevin M. Smith
// 
//
//  Student Name:   Alex Lim and Steven Lu
//  Date: 21/1/2025

#include "ofApp.h"
#include "Util.h"


//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){
	wKeyDown = false;
	aKeyDown = false;
	sKeyDown = false;
	dKeyDown = false;
	spaceKeyDown = false;
	shiftKeyDown = false;
	leftKeyDown = false;
	rightKeyDown = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;
//	ofSetWindowShape(1024, 768);
	cam.setDistance(10);
	cam.setNearClip(.1);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	ofSetVerticalSync(true);
	cam.disableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();
	
	// create sliders for testing
	//
	gui.setup();
	gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	bHide = false;
	gui.add(timingInfo.setup("Timing Info", false));

	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	terrain.loadModel("geo/cs134_terrain_noshading.obj");
	terrain.setScaleNormalization(false);

	if (background.load("images/space.jpg")) {
		cout << "Background loaded" << endl;
	} else {
		cout << "Can't open background image file" << endl;
	}

	//  Create Octree for testing.
	//
	float t1 = ofGetElapsedTimeMicros();
	octree.create(terrain.getMesh(0), 20);
	float t2 = ofGetElapsedTimeMicros();
	cout << "Time to Create Octree: " << (t2 - t1) / 1000 << " millisec" << endl;
	
	cout << "Number of Verts: " << terrain.getMesh(0).getNumVertices() << endl;

}
 
//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {

	//Show boxes lander collides with and move lander up when u is pressed
	if ((colBoxList.size() >= 10) && uClicked) {
		hmary.setPosition(hmary.getPosition().x, hmary.getPosition().y + 0.1, hmary.getPosition().z);
		ofVec3f min = hmary.getSceneMin() + hmary.getPosition();
		ofVec3f max = hmary.getSceneMax() + hmary.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		colBoxList.clear();
		octree.intersect(bounds, octree.root, colBoxList);
	} else {
		uClicked = false;
	}
	
	//movement
	if (bLanderLoaded && leftKeyDown) {
		rotForce = 30;
	}
	if (bLanderLoaded && rightKeyDown) {
		rotForce = -30;
	}
	

	float d = glm::radians(hmary.getRotationAngle(0));
	glm::vec3 heading = glm::normalize(glm::vec3(sin(d), 0, -cos(d)));
	cout << "Heading in update angle: " << heading << endl;

	if (bLanderLoaded && wKeyDown) {
		force = heading * 5;
	}
	if (bLanderLoaded && sKeyDown) {
		force = heading * -5;
	}
	if (bLanderLoaded && dKeyDown) {
		force = glm::vec3(heading.z, 0, heading.x) * 5;
	}
	if (bLanderLoaded && aKeyDown) {
		force = glm::vec3(heading.z, 0, heading.x) * -5;
	}

	integrateMove();
	integrateRot();
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofNoFill();
	ofDisableDepthTest();
	background.draw(0, 0, ofGetWidth(), ofGetHeight());
	glDepthMask(false);
	if (!bHide) gui.draw();
	glDepthMask(true);

	ofEnableDepthTest();
	cam.begin();
	ofPushMatrix();

	ofEnableLighting();              // shaded mode
	terrain.drawFaces();
	if (bLanderLoaded) {
		hmary.drawFaces();
		if (!bTerrainSelected) drawAxis(hmary.getPosition());
		if (bDisplayBBoxes) {
			ofNoFill();
			ofSetColor(ofColor::white);
			for (int i = 0; i < hmary.getNumMeshes(); i++) {
				ofPushMatrix();
				ofMultMatrix(hmary.getModelMatrix());
				ofRotate(-90, 1, 0, 0);
				Octree::drawBox(bboxList[i]);
				ofPopMatrix();
			}
		}

		if (bLanderSelected) {
			ofVec3f min = hmary.getSceneMin() + hmary.getPosition();
			ofVec3f max = hmary.getSceneMax() + hmary.getPosition();

			Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
			ofSetColor(ofColor::white);
			Octree::drawBox(bounds);

			// draw colliding boxes
			//
			ofSetColor(ofColor::lightBlue);
			for (int i = 0; i < colBoxList.size(); i++) {
				Octree::drawBox(colBoxList[i]);
			}
		}
	}

	// recursively draw octree
	//
	ofDisableLighting();
	int level = 0;

	if (bDisplayLeafNodes) {
		octree.drawLeafNodes(octree.root);
		cout << "num leaf: " << octree.numLeaf << endl;
    }
	else if (bDisplayOctree) {
		ofNoFill();
		ofSetColor(ofColor::white);
		octree.draw(numLevels, 0);
	}

	ofPopMatrix();
	cam.end();
}


// 
// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {

	switch (key) {
	case 'w':
		wKeyDown = true;
		break;
	case 'a':
		aKeyDown = true;
		break;
	case 's':
		sKeyDown = true;
		break;
	case 'd':
		dKeyDown = true;
		break;
	case ' ':
		spaceKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		shiftKeyDown = true;
		break;
	case OF_KEY_RIGHT:
		rightKeyDown = true;
		break;
	case OF_KEY_LEFT:
		leftKeyDown = true;
		break;

	case 'B':
	case 'b':
		bDisplayBBoxes = !bDisplayBBoxes;
		break;
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
		else cam.enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		break;
	case 'L':
	case 'l':
		bDisplayLeafNodes = !bDisplayLeafNodes;
		break;
	case 'O':
	case 'o':
		bDisplayOctree = !bDisplayOctree;
		break;
	case 'r':
		cam.reset();
		break;
	case 't':
		break;
	case 'u':
		uClicked = true;
		break;
	case 'v':
	case 'V':
		break;
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_DEL:
		break;
	default:
		break;
	}
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::keyReleased(int key) {

	switch (key) {
	
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case 'w':
		wKeyDown = false;
		break;
	case 'a':
		aKeyDown = false;
		break;
	case 's':
		sKeyDown = false;
		break;
	case 'd':
		dKeyDown = false;
		break;
	case ' ':
		spaceKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		shiftKeyDown = false;
		break;
	case OF_KEY_RIGHT:
		rightKeyDown = false;
		break;
	case OF_KEY_LEFT:
		leftKeyDown = false;
		break;
	default:
		break;

	}
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

	
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	// if moving camera, don't allow mouse interaction
//
	if (cam.getMouseInputEnabled()) return;

	// if rover is loaded, test for selection
	//
	if (bLanderLoaded) {
		glm::vec3 origin = cam.getPosition();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		ofVec3f min = hmary.getSceneMin() + hmary.getPosition();
		ofVec3f max = hmary.getSceneMax() + hmary.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			bLanderSelected = true;
			mouseDownPos = getMousePointOnPlane(hmary.getPosition(), cam.getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
		else {
			bLanderSelected = false;
		}
	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

bool ofApp::raySelectWithOctree(ofVec3f &pointRet) {
	ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));

	float t1 = ofGetElapsedTimeMicros();
	pointSelected = octree.intersect(ray, octree.root, selectedNode);
	float t2 = ofGetElapsedTimeMicros();
	if (timingInfo) {
		cout << "Time to Search Octree: " << (t2 - t1) / 1000 << " millisec" << endl;
	}

	if (pointSelected) {
		pointRet = octree.mesh.getVertex(selectedNode.points[0]);
	}
	return pointSelected;
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	if (bInDrag) {

		glm::vec3 landerPos = hmary.getPosition();

		glm::vec3 mousePos = getMousePointOnPlane(landerPos, cam.getZAxis());
		glm::vec3 delta = mousePos - mouseLastPos;
	
		landerPos += delta;
		hmary.setPosition(landerPos.x, landerPos.y, landerPos.z);
		mouseLastPos = mousePos;

		ofVec3f min = hmary.getSceneMin() + hmary.getPosition();
		ofVec3f max = hmary.getSceneMax() + hmary.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		colBoxList.clear();
		octree.intersect(bounds, octree.root, colBoxList);
	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent2(ofDragInfo dragInfo) {

	ofVec3f point;
	mouseIntersectPlane(ofVec3f(0, 0, 0), cam.getZAxis(), point);
	if (hmary.loadModel(dragInfo.files[0])) {
		hmary.setScaleNormalization(false);
//		hmary.setScale(.1, .1, .1);
	//	hmary.setPosition(point.x, point.y, point.z);
		hmary.setPosition(1, 1, 0);

		bLanderLoaded = true;
		cout << bLanderLoaded << endl;
		for (int i = 0; i < hmary.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(hmary.getMesh(i)));
		}

		cout << "Mesh Count: " << hmary.getMeshCount() << endl;
	}
	else cout << "Error: Can't load model" << dragInfo.files[0] << endl;
}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point) {
	ofVec2f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (hmary.loadModel(dragInfo.files[0])) {
		bLanderLoaded = true;
		hmary.setScaleNormalization(false);
		hmary.setPosition(0, 0, 0);
		cout << "number of meshes: " << hmary.getNumMeshes() << endl;
		bboxList.clear();
		for (int i = 0; i < hmary.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(hmary.getMesh(i)));
		}

		//		hmary.setRotation(1, 180, 1, 0, 0);

				// We want to drag and drop a 3D object in space so that the model appears 
				// under the mouse pointer where you drop it !
				//
				// Our strategy: intersect a plane parallel to the camera plane where the mouse drops the model
				// once we find the point of intersection, we can position the hmary/hmary
				// at that location.
				//

				// Setup our rays
				//
		glm::vec3 origin = cam.getPosition();
		glm::vec3 camAxis = cam.getZAxis();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
		float distance;

		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
		if (hit) {
			// find the point of intersection on the plane using the distance 
			// We use the parameteric line or vector representation of a line to compute
			//
			// p' = p + s * dir;
			//
			glm::vec3 intersectPoint = origin + distance * mouseDir;

			// Now position the hmary's origin at that intersection point
			//
			glm::vec3 min = hmary.getSceneMin();
			glm::vec3 max = hmary.getSceneMax();
			float offset = (max.y - min.y) / 2.0;
			hmary.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);

			// set up bounding box for hmary while we are at it
			//
			landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
	}


}

//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	glm::vec3 origin = cam.getPosition();
	glm::vec3 camAxis = cam.getZAxis();
	glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

	if (hit) {
		glm::vec3 intersectPoint = origin + distance * mouseDir;
		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}

void ofApp::integrateMove() {
	float framerate = ofGetFrameRate();
	if (ofGetFrameRate() != 0) {
		float dt = 1.0 / framerate;

		glm::vec3 pos = hmary.getPosition() + velocity * dt;
		hmary.setPosition(pos.x, pos.y, pos.z);
		glm::vec3 accel = acceleration;

		if (force != glm::vec3(0, 0, 0)) {
			accel += (1.0 / mass * force);
		}

		velocity += accel * dt;
		velocity *= damping;
	}
	force = glm::vec3(0, 0, 0);
}

void ofApp::integrateRot() {
	float framerate = ofGetFrameRate();
	if (framerate != 0) {
		float dt = 1.0f / framerate;

		float rotAcc = 0.0f;
		if (rotForce != 0.0f) {
			rotAcc = rotForce / mass;
		}

		rotVel += rotAcc * dt;
		rotVel *= damping;

		float rot = hmary.getRotationAngle(0) + rotVel * dt;

		rot = fmod(rot, 360.0f);
		if (rot < 0) rot += 360.0f;

		hmary.setRotation(0, rot, 0, 1, 0);
		cout << "Integrate Rot angle: " << hmary.getRotationAngle(0) << endl;
	}

	rotForce = 0.0f;
}


