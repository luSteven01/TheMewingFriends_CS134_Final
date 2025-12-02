#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include  "ofxAssimpModelLoader.h"
#include "Octree.h"
#include "glm/gtx/intersect.hpp"



class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent2(ofDragInfo dragInfo);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void toggleSelectTerrain();
		bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point);
		bool raySelectWithOctree(ofVec3f &pointRet);
		float rayFindAlt();
		glm::vec3 getMousePointOnPlane(glm::vec3 p , glm::vec3 n);

		//Integrated movement
		void integrateMove();
		void integrateRot();
		glm::vec3 velocity = glm::vec3(0,0,0);
		glm::vec3 acceleration = glm::vec3(0, 0, 0);
		glm::vec3 force = glm::vec3(0, 0, 0);
		float damping = 0.99;
		float mass = 1.0;
		float rotation = 0.0;
		float rotVel = 0;
		float rotAcc = 0;
		float rotForce = 0;


		ofEasyCam cam;
		ofxAssimpModelLoader terrain, hmary;
		ofTrueTypeFont guiFont;
		ofImage background;

		bool wKeyDown;
		bool aKeyDown;
		bool sKeyDown;
		bool dKeyDown;
		bool spaceKeyDown;
		bool shiftKeyDown;
		bool leftKeyDown;
		bool rightKeyDown;
		bool showAltitude;

		ofLight light;
		Box boundingBox, landerBounds;
		Box testBox;
		vector<Box> colBoxList;
		bool bLanderSelected = false;
		Octree octree;
		TreeNode selectedNode;
		glm::vec3 mouseDownPos, mouseLastPos;
		bool bInDrag = false;

		ofxIntSlider numLevels;
		ofxPanel gui;
		ofxToggle timingInfo;
		ofTime timer;

		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bPointSelected;
		bool bHide;
		bool pointSelected = false;
		bool bDisplayLeafNodes = false;
		bool bDisplayOctree = false;
		bool bDisplayBBoxes = false;
		bool uClicked = false;
		
		bool bLanderLoaded;
		bool bTerrainSelected;
	
		ofVec3f selectedPoint;
		ofVec3f intersectPoint;

		vector<Box> bboxList;

		const float selectionRange = 4.0;
};
