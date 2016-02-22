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
    ofParameter<bool> param_group1_param0;
    ofParameter<string> param_group1_param1;
    ofxButton param_group1_param2;
    
    ofImage image;
    ofxCvColorImage color;
    ofxCvGrayscaleImage binary;
    ofxCvContourFinder contourFinder;
    
    ofxCvBlob blob;
    
    SimulateApproximateThread approximate;
    bool drawImage = true;
    
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
//        ofSetCircleResolution(32);
        
        setupGUI();
        
        image.loadImage("sample.png");
        image.setImageType(OF_IMAGE_COLOR);
//        color.
        color.setFromPixels(image.getPixelsRef());
//        approximate.startThread(true, true);
//        metaball
        metaball.allocate(ofGetWidth(), ofGetHeight(), GL_RGB);
        blur.setup(ofGetWidth(), ofGetHeight(), 10, 0.2, 4);
//        metaball.
    }
    
    void setupGUI() {
        gui.setDefaultWidth(240);
        gui.setup();
        gui.setName("Setting");
        param_group0.setName("Parameter Group 0");
        param_group0.add(param_group0_param0.set("IR Threshold", 128, 0.0, 255.0));
        param_group0.add(param_group0_param1.set("Param 1", 128, 0, 255));
        param_group0.add(param_group0_param2.set("Param 2", 0, 0, 255));
        gui.add(param_group0);
        param_group1.setName("Parameter Group 1");
        param_group1.add(param_group1_param0.set("toggle", false));
        param_group1.add(param_group1_param1.set("string", "test"));
        gui.add(param_group1);
        gui.add(param_group1_param2.setup("button"));
        
        
        param_group0_param0.addListener(this, &ofApp::updateFloatParam);
        param_group0_param1.addListener(this, &ofApp::updateIntParam);
        param_group0_param2.addListener(this, &ofApp::updateIntParam);
        
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
        
        blur.setScale(ofMap(mouseX, 0, ofGetWidth(), 0, 32));
//        blur.setRotation(ofMap(mouseY, 0, ofGetHeight(), -PI, PI));
        updateContour();
    }
    void updateContour(){
        binary = color;
        binary.threshold(128);
        contourFinder.findContours(binary, 100, image.width*image.height, 10, true);
        contourFinder.blobs;
        if(contourFinder.blobs.size()>0){
            //            blob = *contourFinder.blobs.begin();
            //            cout << blob.boundingRect << endl;
            
            approximate.blob = *contourFinder.blobs.begin();
        }
    }
    
    //--------------------------------------------------------------
    void draw() {
//        ofEnableBlendMode(OF_BLENDMODE_ADD);
        ofBackground(32);
        if(drawImage) {
            binary.draw(0, 0);
//            blur.draw();
//            color.draw(0, 0);
        }
        drawMetaball();
//        binary.draw(0, 0);
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
//        contourFinder.draw(0, 0);
//        ofFill();
//        ofCircle(approximate.current.x, approximate.current.y, 4);
        ofSetColor(50, 70, 167);
        vector<ofVec3f>::iterator c = approximate.results.begin();
        while (c!=approximate.results.end()) {
            ofFill();
            ofCircle(c->x, c->y, 2);
            ofNoFill();
            ofCircle(c->x, c->y, c->z);
            c++;
        }
        ofSetColor(167, 70, 50);
        ofNoFill();
//        ofRect(approximate.area.x, approximate.area.y, approximate.area.width, approximate.area.height);
//        ofCircle(approximate.circle.x, approximate.circle.y, approximate.circle.z);
        ofSetColor(255);
        if(mode == AppMode_Debug) {
            drawDebug();
        }
        
    }
    void drawMetaball() {
        blur.begin();
        ofFill();
//        ofClear(0,0,0, 255);
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
//        ofSetColor(0, 0, 0);
//        ofRect(0, 0, ofGetWidth(), ofGetHeight());
//        ofSetColor(255);
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
        }else if(key == ' ') {
            if(!approximate.isThreadRunning()){
                approximate.startThread();
            }
            approximate.reset();
        }else if(key == 'b') {
            drawImage = !drawImage;
        }else if(key == OF_KEY_UP) {
            addCircle();
        }else if(key == OF_KEY_DOWN) {
            deleteCircle();
        }
    }
    
    void updateFloatParam(float &value) {
        //do something.
        cout << "updateFloatParam : " << value << endl;
    }
    
    void updateIntParam(int &value) {
        //do something.
        cout << "updateIntParam : " << value << endl;
    }
};

//========================================================================
int main( ){
	ofSetupOpenGL(1280,720, OF_WINDOW);
	ofRunApp( new ofApp());
}
