#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include  "ofxAssimpModelLoader.h"
#include "Octree.h"
#include "glm/gtx/intersect.hpp"
#include "Particle.h"
#include "ParticleEmitter.h"
#include "ParticleSystem.h"



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
		ofVec3f getAverageNormal();
    	void resolveCollision(const ofVec3f &normal);
		void loadVbo();
	
		//Integrated movement
		void integrateMove();
		void integrateRot();
		glm::vec3 velocity = glm::vec3(0,0,0);
		glm::vec3 acceleration = glm::vec3(0, -9.8, 0);
		glm::vec3 force = glm::vec3(0, 0, 0);
		float damping = 0.99;
		float mass = 1.0;
		float rotation = 0.0;
		float rotVel = 0;
		float rotAcc = 0;
		float rotForce = 0;
		float speed = 300;

		//Camera
		ofEasyCam cam;
		ofCamera * theCam = NULL;
		ofCamera onboardCam, trackingCam, thirdPerCam;

		//lights
		ofLight keyLight, fillLight;
		ofLight light; //Starter code. Maybe remove later

		//art assets
		ofxAssimpModelLoader terrain, hmary;
		bool bLanderLoaded;
		bool bTerrainSelected;
		ofTrueTypeFont guiFont;
		ofImage background;

		//controls
		bool wKeyDown;
		bool aKeyDown;
		bool sKeyDown;
		bool dKeyDown;
		bool spaceKeyDown;
		bool shiftKeyDown;
		bool qKeyDown;
		bool eKeyDown;
		bool showAltitude;
		bool bAltKeyDown;
		glm::vec3 mouseDownPos, mouseLastPos;

		bool bLanderSelected = false;
		bool bInDrag = false;

		//GUI
		ofxPanel gui;
		ofxIntSlider numLevels;
		bool bHide;

		//Spatial Subdivision
		Octree octree;
		TreeNode selectedNode;
		Box boundingBox, landerBounds;
		vector<Box> colBoxList;
		vector<Box> bboxList;
		bool pointSelected = false;
		bool bDisplayLeafNodes = false;
		bool bDisplayOctree = false;

		//Particles
		ParticleEmitter exhaustEmitter;
		TurbulenceForce* exhaustTurbulence = nullptr;
		ofTexture particleTex;
		float particleRadius = 50;

		// shaders
		ofVbo vbo;
		ofShader shader;

		float  crashSpeed   = 18.0;
		float  landSpeed  = 2.0;
		bool   bLanded    = false;
		bool   bCrashed   = false;
		vector<int> terrainFaces;        // all face indices in the terrain mesh
    	vector<int> collidingFaces; 
};
