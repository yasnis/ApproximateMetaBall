#include "ofMain.h"
#include "ofxGui.h"
#include "ofxOpenCv.h"
#include "SimulateApproximateThread.h"
#include "ofxBlur.h"

#define img_w 1280
#define img_h 720

class ofApp : public ofBaseApp{
    enum AppMode {
        AppMode_Release,
        AppMode_Debug
    };
    
    AppMode mode = AppMode_Debug;
    ofxPanel gui;
    ofParameterGroup param_group0;
    ofParameter<float> param_group0_param0;
    ofParameter<int> param_group0_param1;
    ofParameter<int> param_group0_param2;
    
    ofParameterGroup param_group1;
    ofParameter<int> approximate_min_radius;
    ofParameter<int> approximate_radius_step;
    ofxButton param_group1_param2;
    
    
    ofImage image;
    ofxCvColorImage color;
    ofxCvGrayscaleImage binary;
    ofxCvContourFinder contourFinder;
    
    
    SimulateApproximateThread approximate;
    bool drawImage = true;
    bool drawApproximate = true;
    bool drawProcecss = false;
    
    vector<ofVec3f> circles;
    vector<ofVec2f> velocities;
    ofFbo metaball;
    ofPixels metaball_pix;
    ofxBlur blur;
    
    
    //--------------------------------------------------------------
    void setup(){
        ofBackground(0);
        ofSetFrameRate(60);
        ofSetVerticalSync(true);
        ofSetCircleResolution(32);
        
        setupGUI();
        image.allocate(ofGetWidth(), ofGetHeight(), OF_IMAGE_COLOR);
        metaball.allocate(ofGetWidth(), ofGetHeight(), GL_RGB);
        blur.setup(ofGetWidth(), ofGetHeight(), 10, 0.2, 4);
        
        addCircle();
        addCircle();
        addCircle();
    }
    
    void setupGUI() {
        gui.setDefaultWidth(240);
        gui.setup();
        gui.setName("Setting");
        param_group0.setName("Base Setting.");
        param_group0.add(param_group0_param0.set("IR Threshold", 128, 0.0, 255.0));
        param_group0.add(param_group0_param1.set("Background", 128, 0, 255));
        param_group0.add(param_group0_param2.set("Blur Length", 0, 0, 32));
        gui.add(param_group0);
        param_group1.setName("Approximate Setting.");
        param_group1.add(approximate_min_radius.set("Minimum radius", 10, 0, 255));
        param_group1.add(approximate_radius_step.set("string", 3, 1, 10));
        gui.add(param_group1);
        
        gui.setPosition(10, ofGetHeight()-gui.getHeight()-10);
        
        
        param_group0_param0.addListener(this, &ofApp::updateFloatParam);
        param_group0_param1.addListener(this, &ofApp::updateIntParam);
        param_group0_param2.addListener(this, &ofApp::updateIntParam);
        approximate_min_radius.addListener(this, &ofApp::updateIntParam);
        approximate_radius_step.addListener(this, &ofApp::updateIntParam);
        
        gui.loadFromFile("settings.xml");
    }
    
    //--------------------------------------------------------------
    void update(){
        for (int i = 0; i < circles.size(); i++) {
            circles[i].x+=velocities[i].x;
            circles[i].y+=velocities[i].y;
            if(circles[i].x < 0){
                circles[i].x = -circles[i].x;
                velocities[i].x*=-1;
            }else if(circles[i].x > ofGetWidth()) {
                circles[i].x = ofGetWidth()-(circles[i].x-ofGetWidth());
                velocities[i].x*=-1;
            }
            if(circles[i].y < 0){
                circles[i].y = -circles[i].y;
                velocities[i].y*=-1;
            }else if(circles[i].y > ofGetHeight()) {
                circles[i].y = ofGetHeight()-(circles[i].y-ofGetHeight());
                velocities[i].y*=-1;
            }
        }
        
        if(color.width==0)return;
        blur.setScale(param_group0_param2);
        /*
        updateContour();
        /*/
        if (approximate.isStopped) {
            updateContour();
            approximate.startThread();
        }
        //*/
    }
    void updateContour(){
        binary = color;
        binary.threshold(param_group0_param0);
        contourFinder.findContours(binary, 100, image.width*image.height, 10, true);
        contourFinder.blobs;
        if(contourFinder.blobs.size()>0){
            approximate.blobs = contourFinder.blobs;
        }
    }
    
    //--------------------------------------------------------------
    void draw() {
        ofBackground(param_group0_param1);
        
        updateMetaball();
        if(drawImage) {
            ofEnableBlendMode(OF_BLENDMODE_ADD);
            binary.draw(0, 0);
            ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        }
        if(drawApproximate) {
            ofSetColor(50, 70, 167);
            approximate.draw();
        }
        if(drawProcecss) {
            ofSetColor(167, 70, 50);
            approximate.drawProcess();
        }
        if(mode == AppMode_Debug) {
            ofSetColor(255, 0, 0);
            ofDrawBitmapString(ofToString(ofGetFrameRate())+" / fps", ofPoint(10,20));
            ofSetColor(255);
            drawDebug();
        }
        
    }
    void updateMetaball() {
        blur.begin();
        ofFill();
        ofSetColor(0, 0, 0);
        ofRect(0, 0, ofGetWidth(), ofGetHeight());
        ofSetColor(255);
        vector<ofVec3f>::iterator c = circles.begin();
        while (c!=circles.end()) {
            ofCircle(c->x, c->y, c->z);
            c++;
        }
        blur.end();
        metaball.begin();
        ofClear(0,0,0, 255);
        blur.draw();
        metaball.end();
        
        ofPixels pix;
        metaball.readToPixels(pix);
        color.setFromPixels(pix);
    }
    
    void addCircle() {
        ofVec3f c;
        c.x = ofRandom(ofGetWidth());
        c.y = ofRandom(ofGetHeight());
        c.z = ofRandom(100)+100;
        
        circles.push_back(c);
        velocities.push_back(ofVec2f((ofRandom(2)-1)*5, (ofRandom(2)-1)*5));
    }
    void deleteCircle() {
        circles.erase(circles.begin());
        velocities.erase(velocities.begin());
    }
    
    void drawDebug() {
        ofSetColor(255);
        gui.draw();
    }
    
    void exit() {
        gui.saveToFile("settings.xml");
    }
    
    void switchDebug() {
        if (mode == AppMode_Debug) {
            mode = AppMode_Release;
        }else{
            mode = AppMode_Debug;
        }
    }
    
    void keyPressed(int key){
        if(key == 'f'){
            ofToggleFullscreen();
        }else if(key == 'd') {
            switchDebug();
        }else if(key == 'b') {
            drawImage = !drawImage;
        }else if(key == OF_KEY_UP) {
            addCircle();
        }else if(key == OF_KEY_DOWN) {
            deleteCircle();
        }else if(key == 'a') {
            drawApproximate = !drawApproximate;
        }else if(key == 'p') {
            drawProcecss = !drawProcecss;
        }
    }
    
    void updateFloatParam(float &value) {
        //do something.
        cout << "updateFloatParam : " << value << endl;
    }
    
    void updateIntParam(int &value) {
        //do something.
        cout << "updateIntParam : " << value << endl;
        approximate.min_radius = approximate_min_radius;
        approximate.radius_step = approximate_radius_step;
    }
};

//========================================================================
int main( ){
	ofSetupOpenGL(1280,720, OF_WINDOW);
	ofRunApp( new ofApp());
}
