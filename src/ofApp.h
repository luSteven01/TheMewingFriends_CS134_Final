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
		bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point);
		bool raySelectWithOctree(ofVec3f &pointRet);
		float rayFindAlt();
		glm::vec3 getMousePointOnPlane(glm::vec3 p , glm::vec3 n);
		ofVec3f getAverageNormal();
    	void resolveCollision(glm::vec3 normal);
		void loadExhaustVbo();
		void loadExplosionVbo();
		void winCheck();
	
		//Integrated movement
		void integrateMove();
		void integrateRot();
		float gravity = -9;
		glm::vec3 velocity = glm::vec3(0,0,0);
		glm::vec3 acceleration = glm::vec3(0, -50, 0);
		glm::vec3 force = glm::vec3(0, 0, 0);
		float damping = 0.99;
		float mass = 1.0;
		float rotation = 0.0;
		float rotVel = 0;
		float rotAcc = 0;
		float rotForce = 0;
		float speed = 100;

		//Camera
		ofEasyCam cam;
		ofCamera * theCam = NULL;
		ofCamera onboardCam, trackingCam, thirdPerCam;
		string cameraName;

		//lights
		ofLight keyLight, fillLight, landSiteLightMid, landSiteLightRavine, landSiteLightHill;
		ofLight light; //Starter code. Maybe remove later

		//art assets
		ofxAssimpModelLoader terrain, hmary;
		bool bLanderLoaded;
		bool bTerrainSelected;
		ofVec3f terrainPoint;
		ofTrueTypeFont guiFont;
		ofImage background;

		//Landing Zone Boxes
		Box landingZoneMidBox;
		Box landingZoneRavineBox;
		Box landingZoneHillBox;

		//sounds
		ofSoundPlayer thrust;
		ofSoundPlayer explosion;

		//controls
		bool wKeyDown;
		bool aKeyDown;
		bool sKeyDown;
		bool dKeyDown;
		bool spaceKeyDown;
		bool shiftKeyDown;
		bool qKeyDown;
		bool eKeyDown;
		bool bAltKeyDown;
		glm::vec3 mouseDownPos, mouseLastPos;

		bool bLanderSelected = false;
		bool bInDrag = false;
		bool bLanderPaused = false;

		//GUI
		ofxPanel gui;
		ofxIntSlider numLevels;
		bool bHide;
		bool gameOver = false;
		bool win = false;

		//Spatial Subdivision
		Octree octree;
		TreeNode selectedNode;
		Box boundingBox, landerBounds;
		vector<Box> colBoxList;
		vector<Box> bboxList;
		bool pointSelected = false;
		bool bDisplayOctree = false;
		float altPoint = 0;

		//Particles
		ParticleEmitter exhaustEmitter;
		TurbulenceForce* exhaustTurbulence = nullptr;
		ofTexture particleTex;
		float particleRadius = 50;

		ParticleEmitter explosionEmitter;
		ofTexture explosionTex;
		TurbulenceForce* turbForce;
		GravityForce* gravityForce;
		ImpulseRadialForce* radialForce;
		float explosionParticleRadius = 50;
		float explosionLifespan = 5;
		float explosionRate = 1;

		// shaders
		ofVbo exhaustVbo;
		ofVbo explosionVbo;
		ofShader shader;

		float  crashSpeed   = 20.0;
		float  landSpeed  = 2.0;
		bool   bLanded    = false;
		bool   bCrashed   = false;
		vector<int> terrainFaces;        // all face indices in the terrain mesh
    	vector<int> collidingFaces; 

		float maxFuel = 120;
		float remainingFuel;
		bool hasFuel;
		float lastTime;
		bool hasThrust;

		float altitude;
};
