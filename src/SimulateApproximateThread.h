//
//  SimulateApproximateThread.h
//  ApproximateMetaBall
//
//  Created by X006819_nishimura on 2016/02/15.
//
//

#ifndef __ApproximateMetaBall__SimulateApproximateThread__
#define __ApproximateMetaBall__SimulateApproximateThread__

#include "ofMain.h"
#include "ofxOpenCv.h"

class SimulateApproximateThread : public ofThread {
public:
    
    vector<ofxCvBlob>::iterator blob;
    vector<ofxCvBlob> blobs;
    
    ofVec3f circle;
    ofRectangle area;
    ofPoint current;
    
    vector<ofVec3f> results;
    vector<ofVec3f> results_static;
    vector<ofVec3f> results_old;
    
    int step = 10;
    int anglestep = 45;
    int min_radius = 50;
    int radius_step = 10;
    int starttime = 0;
    bool isStopped = true;
    
    void threadedFunction() {
        results.clear();
        results_old.clear();
        isStopped = false;
        reset();
        while(isThreadRunning()) {
            update();
        }
    }
    void reset() {
        results_old = results;
        if(results_old.size()>0) {
            vector<ofVec3f> v;
            vector<ofVec3f>::iterator c = results_old.begin();
            vector<ofVec3f>::iterator b = results_old.begin();
            vector<ofVec3f>::iterator e = results_old.end();
            while (c!=e) {
                if(checkInside(*c)){
                    v.push_back(*c);
                }
                c++;
            }
            results = v;
         }
        blob = blobs.begin();
        resetCircle();
        starttime = ofGetElapsedTimeMillis();
    }
    void resetCircle(){
        circle.z = min(blob->boundingRect.width, blob->boundingRect.height)/2;
        step = circle.z/radius_step;
        
        if(step < 1)step = 1;
    }
    
    void update() {
        circle.x = blob->boundingRect.x + circle.z;
        circle.y = blob->boundingRect.y + circle.z;
        updateArea();
        int i = 0;
        for (int y = area.y; y < area.y + area.height; y+=step) {
            for (int x = area.x; x < area.x + area.width; x+=step) {
                
                bool b = true;
                b = PointInPolygon(ofPoint(x, y), blob->pts);
                if(b){
                    vector<ofVec3f>::iterator c = results.begin();
                    vector<ofVec3f>::iterator e = results.end();
                    while (c!=e) {
                        b = sqrt((c->x-x)*(c->x-x)+(c->y-y)*(c->y-y))>c->z;
                        if(!b) {
                            x+=step;
                            break;
                        }
                        c++;
                    }
                    if(b) {
                        circle.x = x;
                        circle.y = y;
                        if(checkInside(circle)){
                            x+=circle.z-1;
                        }
                    }
                }
            }
        }
        circle.z -= step;
        if(circle.z <= min_radius) {
            if(distance(blob, blobs.end())-1 != 0){
                blob++;
                resetCircle();
                return;
            }
//            cout << "loop : " << ofGetElapsedTimeMillis() - starttime << "ms, " << results.size() << endl;
            results_static = results;
            isStopped = true;
            stopThread();
        }
    }
    void updateArea() {
        area.x = blob->boundingRect.x + circle.z;
        area.y = blob->boundingRect.y + circle.z;
        area.width = blob->boundingRect.width - circle.z*2;
        area.height = blob->boundingRect.height - circle.z*2;
    }
    
    void draw() {
        if(results_static.size()==0)return;
        vector<ofVec3f>::iterator c = results.begin();
        while (c!=results.end()) {
            ofNoFill();
            ofCircle(c->x, c->y, c->z);
            ofFill();
            ofCircle(c->x, c->y, 2);
            c++;
        }
    }
    void drawProcess() {
        ofNoFill();
        ofRect(area.x, area.y, area.width, area.height);
        ofCircle(circle.x, circle.y, circle.z);
    }
    
    bool checkInside(ofVec3f c){
        float rad = 0;
        float step = M_PI/180*anglestep;
        while(rad < M_PI*2){
            current.set(c.x+c.z*cos(rad), c.y+c.z*sin(rad));
            if(!PointInPolygon(current, blob->pts)){
                return false;
            }
            rad+=step;
        }
        results.push_back(circle);
        return true;
    }
    bool PointInPolygon(ofPoint point, vector<ofPoint> points) {
        int i, j, nvert = points.size();
        bool c = false;
        for(i = 0, j = nvert - 1; i < nvert; j = i++) {
            if( ( (points[i].y >= point.y ) != (points[j].y >= point.y) ) &&
               (point.x <= (points[j].x - points[i].x) * (point.y - points[i].y) / (points[j].y - points[i].y) + points[i].x)
               )
                c = !c;
        }
        return c;
    }
};;

#endif /* defined(__ApproximateMetaBall__SimulateApproximateThread__) */
