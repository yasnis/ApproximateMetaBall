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
    ofxCvGrayscaleImage binary;
    ofxCvBlob blob;
    
    ofVec3f circle;
    ofRectangle area;
    ofPoint current;
    
    vector<ofVec3f> results;
    vector<ofVec3f> results_old;
    
    int step = 10;
    int anglestep = 45;
    int min_radius = 50;
    
    int starttime = 0;
//    bool isStopped = false;
    void threadedFunction() {
        results.clear();
        results_old.clear();
        reset();
        while(isThreadRunning()) {
            update();
        }
    }
    void reset() {
        //*
        //*
        results_old = results;
        vector<ofVec3f> v;
        if(results_old.size()>0) {
            vector<ofVec3f>::iterator c = results_old.begin();
            while (c!=results_old.end()) {
                if(checkInside(*c)){
                    v.push_back(*c);
                }
                c++;
            }
            results = v;
         }
         //*/
//        results.clear();
        circle.z = min(blob.boundingRect.width, blob.boundingRect.height)/2;
        step = circle.z/7;
        min_radius = circle.z/2;
        /*/
        results.clear();
        circle.z = min(blob.boundingRect.width, blob.boundingRect.height)/2;
        step = 1;
        min_radius = 0;
         //*/
        starttime = ofGetElapsedTimeMillis();
    }
    void update() {
//        cout << "update" << endl;
        circle.x = blob.boundingRect.x + circle.z;
        circle.y = blob.boundingRect.y + circle.z;
        updateArea();
        for (int y = area.y; y <= area.y + area.height; y+=step) {
            for (int x = area.x; x <= area.x + area.width; x+=step) {
                bool b = true;
                b = PointInPolygon(ofPoint(x, y), blob.pts);
                if(b){
                    vector<ofVec3f>::iterator c = results.begin();
                    while (c!=results.end()) {
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
            cout << "loop : " << ofGetElapsedTimeMillis() - starttime << "ms, " << results.size() << endl;
            sleep(10);
            reset();
//
        }
    }
    void updateArea() {
        area.x = blob.boundingRect.x + circle.z;
        area.y = blob.boundingRect.y + circle.z;
        area.width = blob.boundingRect.width - circle.z*2;
        area.height = blob.boundingRect.height - circle.z*2;
    }
    bool checkInside(ofVec3f c){
        float rad = 0;
        float step = M_PI/180*anglestep;
        while(rad < M_PI*2){
            current.set(c.x+c.z*cos(rad), c.y+c.z*sin(rad));
            if(!PointInPolygon(current, blob.pts)){
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
