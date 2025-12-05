//--------------------------------------------------------------
//
//  Kevin M. Smith
//  Student Name:   Alex Lim and Steven Lu
//  Date: 21/1/2025

#include "ofApp.h"
#include "Util.h"


//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){
	//Set up models
	hmary.setScaleNormalization(false);
	if (hmary.load("geo/lander.obj")) {
		bLanderLoaded = true;
		hmary.setPosition(-300, 470, -550);
		cout << "lander loaded" << endl;
	}
	else {
		bLanderLoaded = false;
		cout << "could not load lander" << endl;
	}

	terrain.setScaleNormalization(false);
	terrain.loadModel("geo/peak-terrain.obj");

	//Fonts and Art
	guiFont.load("fonts/MouldyCheeseRegular.ttf", 25);

	if (background.load("images/space.jpg")) {
		cout << "Background loaded" << endl;
	} else {
		cout << "Can't open background image file" << endl;
	}

	//Shaders
	ofDisableArbTex();
	if (!ofLoadImage(particleTex, "images/dot.png")) {
		cout << "Particle Texture File: images/dot.png not found" << endl;
		ofExit();
	}

	#ifdef TARGET_OPENGLES
	shader.load("shaders_gles/shader");
	#else
	shader.load("shaders/shader");
	#endif

	//initialize controls
	wKeyDown = false;
	aKeyDown = false;
	sKeyDown = false;
	dKeyDown = false;
	spaceKeyDown = false;
	shiftKeyDown = false;
	qKeyDown = false;
	eKeyDown = false;
	bAltKeyDown = false;
	showAltitude = true;
	bTerrainSelected = true;

	// create sliders for testing
	gui.setup();
	gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	bHide = false;

	//Cameras
	ofSetVerticalSync(true);
	ofEnableSmoothing();
	ofEnableDepthTest();
	onboardCam = ofCamera();
	trackingCam = ofCamera();
	thirdPerCam = ofCamera();
	trackingCam.setPosition(-180, 80, 180);
	theCam = &thirdPerCam;

	// setup rudimentary lighting 
	//initLightingAndMaterials();
	//keyLight.setup();
	//keyLight.setAmbientColor(ofFloatColor(200, 0, 150));
	//keyLight.setDiffuseColor(ofFloatColor(200, 0, 150));
	//keyLight.setSpecularColor(ofFloatColor(200, 0, 150));
	//keyLight.setSpotlight();
	//keyLight.setSpotlightCutOff(5);
	//keyLight.setOrientation(glm::vec3(-90, 0, 0));
	//keyLight.enable();
	fillLight.setup();
	fillLight.setPointLight();
	fillLight.setPosition(-180, 300, 180);
	fillLight.enable();

	//  Create Octree for testing.
	//
	float t1 = ofGetElapsedTimeMicros();

	ofMesh combinedTerrain;
	for (int i = 0; i < terrain.getMeshCount(); i++) {
		combinedTerrain.append(terrain.getMesh(i));

	}
	octree.create(combinedTerrain, 20);
	float t2 = ofGetElapsedTimeMicros();
	cout << "Time to Create Octree: " << (t2 - t1) / 1000 << " millisec" << endl;
	cout << "Number of Verts: " << terrain.getMesh(0).getNumVertices() << endl;

	//Set up exhaust emitter
	exhaustEmitter.setEmitterType(RadialEmitter);
	exhaustEmitter.setLifespan(0.3);
	exhaustEmitter.setParticleRadius(particleRadius);
	exhaustEmitter.setRate(0);
	exhaustEmitter.start();
	exhaustTurbulence = new TurbulenceForce(ofVec3f(-500, -500, -500), ofVec3f( 500,  500,  500));
	exhaustEmitter.sys->addForce(exhaustTurbulence);
}
 
//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
	if (bLanderLoaded) {
		//Lander transformation data
		glm::vec3 landerPos = hmary.getPosition();
		float d = glm::radians(rotation);
		glm::vec3 heading = glm::normalize(glm::vec3(-sin(d), 0, -cos(d)));

		//Cameras
		trackingCam.lookAt(landerPos);
		onboardCam.setPosition(landerPos);
		thirdPerCam.setPosition(landerPos - heading * 20 + glm::vec3(0, 10, 0));
		thirdPerCam.lookAt(landerPos);

		//Lights
		//keyLight.setPosition(landerPos);

		//Rotation
		if (qKeyDown) {
			rotForce += 30;
		}
		if (eKeyDown) {
			rotForce -= 30;
		}
		integrateRot();

		//Movement and thrust particles
		bool hasThrust = false;
		glm::vec3 exhaustDir(0, 0, 0);

		if (wKeyDown) {
			force += heading * speed;
			hasThrust = true;
			exhaustDir += -heading;
		}
		if (sKeyDown) {
			force += heading * -speed;
			hasThrust = true;
			exhaustDir += heading;
		}
		if (dKeyDown) {
			force += glm::vec3(-heading.z, 0, heading.x) * speed;
			hasThrust = true;
			glm::vec3 right = glm::vec3(heading.z, 0, heading.x);
			exhaustDir += -right;
		}
		if (aKeyDown) {
			force += glm::vec3(heading.z, 0, -heading.x) * speed;
			hasThrust = true;
			glm::vec3 right = glm::vec3(heading.z, 0, heading.x);
			exhaustDir += right;
		}
		if (spaceKeyDown) {
			force += glm::vec3(0, 1, 0) * speed;
			hasThrust = true;
			glm::vec3 up = glm::vec3(0, 1, 0);
			exhaustDir += -up;
		}
		if (shiftKeyDown) {
			force += glm::vec3(0, 1, 0) * -speed;
		}

        if (hasThrust && exhaustDir != glm::vec3(0, 0, 0)) {
            exhaustDir = glm::normalize(exhaustDir);

            float exhaustSpeed  = 20.0;
            float exhaustOffset = 0.5;

            ofVec3f emitterPos(landerPos.x, landerPos.y, landerPos.z);
            ofVec3f offset(exhaustDir.x * exhaustOffset,
                           exhaustDir.y * exhaustOffset,
                           exhaustDir.z * exhaustOffset);

            exhaustEmitter.setPosition(emitterPos + offset);

            exhaustEmitter.setVelocity(ofVec3f(exhaustDir.x * exhaustSpeed,
                                                exhaustDir.y * exhaustSpeed,
                                                exhaustDir.z * exhaustSpeed));

            exhaustEmitter.setRate(200.0);
        } else {
            exhaustEmitter.setRate(0.0);
        }

        exhaustEmitter.update();
	}

	integrateMove();
	if (bLanderLoaded) {
        ofVec3f min = hmary.getSceneMin() + hmary.getPosition();
        ofVec3f max = hmary.getSceneMax() + hmary.getPosition();
        Box bounds(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

        colBoxList.clear();
        octree.intersect(bounds, octree.root, colBoxList);

        if (!colBoxList.empty()) {
            ofVec3f avgNormal = getAverageNormal();
            resolveCollision(avgNormal);
        }
    }

}

//--------------------------------------------------------------
void ofApp::draw() {
	ofNoFill();

	loadVbo();

	//background
	ofDisableDepthTest();
	background.draw(0, 0, ofGetWidth(), ofGetHeight());
	ofEnableDepthTest();

	//3D assets
	theCam->begin();
	ofPushMatrix();
	//Shaded mode
	ofEnableLighting();

	//Draw terrain
	terrain.drawFaces();

	//Draw Lander
	if (bLanderLoaded) {
		hmary.drawFaces();
		//exhaustEmitter.draw();

		if (!bTerrainSelected) drawAxis(hmary.getPosition());

		//Draw lander selected bbounding box
		if (bLanderSelected) {
			ofVec3f min = hmary.getSceneMin() + hmary.getPosition();
			ofVec3f max = hmary.getSceneMax() + hmary.getPosition();

			Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
			ofSetColor(ofColor::white);
			Octree::drawBox(bounds);

			// draw colliding boxes
			ofSetColor(ofColor::lightBlue);
			for (int i = 0; i < colBoxList.size(); i++) {
				Octree::drawBox(colBoxList[i]);
			}
		}
	}

	// recursively draw octree
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
	theCam->end();

	glDepthMask(GL_FALSE);
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnablePointSprites();
	shader.begin();
	theCam->begin();

	particleTex.bind();
	vbo.draw(GL_POINTS, 0, (int)exhaustEmitter.sys->particles.size());
	particleTex.unbind();

	theCam->end();
	shader.end();
	ofDisablePointSprites();
	ofDisableBlendMode();
	ofEnableAlphaBlending();
	glDepthMask(GL_TRUE);

	//GUI
	ofDisableDepthTest();
	if (!bHide) gui.draw();
	if (showAltitude && bLanderLoaded) {
		guiFont.drawString("Altitude: " + ofToString(rayFindAlt()), (ofGetWidth() / 2) - 150, 120);
	}
	ofEnableDepthTest();
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
	case 'W':
	case 'w':
		wKeyDown = true;
		break;
	case 'A':
	case 'a':
		aKeyDown = true;
		break;
	case 'S':
	case 's':
		sKeyDown = true;
		break;
	case 'D':
	case 'd':
		dKeyDown = true;
		break;
	case ' ':
		spaceKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		shiftKeyDown = true;
		break;
	case 'Q':
	case 'q':
		qKeyDown = true;
		break;
	case 'E':
	case 'e':
		eKeyDown = true;
		break;
	case '1':
		theCam = &thirdPerCam;
		break;
	case '2':
		theCam = &onboardCam;
		break;
	case '3':
		theCam = &trackingCam;
		break;
	case '4':
		theCam = &cam;
		break;
	case 'B':
	case 'b':
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
	case 'R':
	case 'r':
		cam.reset();
		break;
	case 't':
		break;
	case 'u':
		break;
	case 'v':
	case 'V':
		break;
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
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
		break;
	case 'W':
	case 'w':
		wKeyDown = false;
		break;
	case 'A':
	case 'a':
		aKeyDown = false;
		break;
	case 'S':
	case 's':
		sKeyDown = false;
		break;
	case 'D':
	case 'd':
		dKeyDown = false;
		break;
	case ' ':
		spaceKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		shiftKeyDown = false;
		break;
	case 'E':
	case 'e':
		eKeyDown = false;
		break;
	case 'Q':
	case 'q':
		qKeyDown = false;
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

	pointSelected = octree.intersect(ray, octree.root, selectedNode);

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
	{ .5, .5, .5, 1.0 };
	static float diffuse[] =
	{ 1.0, 1.0, 1.0, 1.0 };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0, 1.0, 1.0, 1.0 };

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

	float fr = ofGetFrameRate();
		if (fr != 0) {
			float dt = 1.0 / fr;

			rotation += rotVel * dt;

			float accel = rotAcc;
			if (rotForce != 0.0) {
				accel += (rotForce / mass);
			}

			rotVel += accel * dt;
			rotVel *= damping;

			hmary.setRotation(0, rotation, 0, 1, 0);
		}

		rotForce = 0;
}

float ofApp::rayFindAlt() {
	ofVec3f origin = hmary.getPosition();

	Ray ray(Vector3(origin.x, origin.y, origin.z), Vector3(0, -1, 0));

	TreeNode groundNode;

	if (octree.intersect(ray, octree.root, groundNode)) {
		int i = groundNode.points[0]; // index
		ofVec3f groundPoint = octree.mesh.getVertex(i); // actual mesh vertex position
		return origin.y - groundPoint.y; // height difference
	}

	return 0;
}

ofVec3f ofApp::getAverageNormal() {
    ofVec3f sum(0, 0, 0);
    int count = 0;

    ofMesh &mesh = octree.mesh; 
    int nVerts   = mesh.getNumVertices();
    int nNormals = mesh.getNumNormals();

    if (nNormals == 0) {
        return ofVec3f(0, 1, 0);
    }

    for (int i = 0; i < nVerts; i++) {
        ofVec3f vert = mesh.getVertex(i);
        Vector3 vPoint(vert.x, vert.y, vert.z);

        bool insideAnyBox = false;
        for (Box &box : colBoxList) {
            if (box.inside(vPoint)) {     
                insideAnyBox = true;
                break;              
            }
        }

        if (insideAnyBox) {
            if (i < nNormals) {
                ofVec3f norm = mesh.getNormal(i);
                sum += norm;
                count++;
            }
        }
    }

    if (count == 0) {
        return ofVec3f(0, 1, 0);   // fallback: straight up
    }

    sum /= (float)count;
    sum.normalize();
    return sum;
}

void ofApp::resolveCollision(const ofVec3f &normalOF) {
    if (!bLanderLoaded) return;

    // 1) Normalize the normal
    glm::vec3 n(normalOF.x, normalOF.y, normalOF.z);
    if (glm::length2(n) < 0.000001) {
        n = glm::vec3(0, 1, 0);
    } else {
        n = glm::normalize(n);
    }

    // 2) Decompose velocity into normal and tangential components
    glm::vec3 v = velocity;
    float vn = glm::dot(v, n);     // velocity along normal
    glm::vec3 vN = vn * n;
    glm::vec3 vT = v - vN;

    // Speed INTO the terrain along the normal is:
    float impactSpeed = (vn < 0.0) ? -vn : 0.0;

    // --- Landing / crash classification (only once) ---
    if (!bLanded && !bCrashed && impactSpeed > 0.0) {
        if (impactSpeed > crashSpeed) {
            // CRASH
            bCrashed = true;
            bLanded  = false;
            velocity = glm::vec3(0, 0, 0);

            // optional: small push out so it doesn't clip
            glm::vec3 pos = hmary.getPosition();
            pos += n * 0.05;
            hmary.setPosition(pos.x, pos.y, pos.z);

            // TODO: trigger explosion particle, game-over, sound, etc.
            return;
        }
        else if (impactSpeed <= landSpeed) {
            // SOFT LANDING
            bLanded  = true;
            bCrashed = false;
            velocity = glm::vec3(0, 0, 0);

            glm::vec3 pos = hmary.getPosition();
            pos += n * 0.02;  // tiny lift
            hmary.setPosition(pos.x, pos.y, pos.z);
            return;
        }
        // Otherwise, "medium" impact → bounce
    }

    // --- Velocity response for overlap ---

    float restitution = 0.6;  // MORE bouncy
    float friction    = 0.3;  // LESS tangential damping

    if (vn < 0.0) {
        // Moving INTO the surface: reflect with restitution
        vN *= -restitution;
    } else {
        // Already moving out or parallel:
        // remove any tiny drift into terrain along normal
        vN = glm::vec3(0, 0, 0);
    }

    // Apply friction to tangential component
    vT *= (1.0 - friction * 0.02);

    velocity = vT + vN;

    // --- Positional correction (this is key for steep slopes) ---
    // Always push OUT along the normal if overlapping
    glm::vec3 pos = hmary.getPosition();
    pos += n * 0.1;  // a bit bigger than 0.05; tune this
    hmary.setPosition(pos.x, pos.y, pos.z);
}

void ofApp::loadVbo() {
	if (exhaustEmitter.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < exhaustEmitter.sys->particles.size(); i++) {
		points.push_back(exhaustEmitter.sys->particles[i].position);
		sizes.push_back(ofVec3f(particleRadius));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	vbo.clear();
	vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}
