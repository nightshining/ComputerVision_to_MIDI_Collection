#pragma once
#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxGui.h"
#include "ofxCv.h"
#include "ofxOsc.h"

// send host (aka ip address)
#define HOST "localhost"
#define PORT 7400

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();

    ofVideoGrabber video;
    ofxCvColorImage image;        //The current video frame

    //The current and the previous video frames as grayscale images
    ofxCvGrayscaleImage grayImage, grayImagePrev;

    ofxCvGrayscaleImage diff;        //Absolute difference of the frames
    ofxCvFloatImage diffFloat;        //Amplified difference images
    ofxCvFloatImage bufferFloat;    //Buffer image

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    ofxPanel gui;
    ofxFloatSlider threshold, damping;
    ofxFloatSlider min, max, cvThresh;
    ofxColorSlider color;
    
    //ofxCV
    
    ofxCv::ContourFinder contourFinder;
    
    ofPixels pix;
    ofFbo    fbo;
    
    
    //ofxOSC
    ofxOscSender sender;
    vector<ofVec2f> smoothPos;
    ofVec2f easingAmt;
            
};
