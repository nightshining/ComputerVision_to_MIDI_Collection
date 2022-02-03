#include "ofApp.h"

using namespace ofxCv;
using namespace cv;


//--------------------------------------------------------------
void ofApp::setup(){
    
    video.listDevices();
    video.setDeviceID(2);
    video.setup(320,240);
    int w = video.getWidth();
    int h = video.getHeight();
    
    gui.setup();
    gui.add(threshold.setup("Diff Threshold", 3.0, 0.0, 10.0));
    gui.add(damping.setup("Diff Damping", 0.85, 0.0, 1.0));
    gui.add(min.setup("CV Min", 1.0, 0.0, 320.0));
    gui.add(max.setup("CV Max", 50.0, 0.0, 320.0));
    gui.add(cvThresh.setup("CV Thresh", 5.0, 0.0, 50.0));
    gui.add(color.setup("color", ofColor(ofColor::black), ofColor(0, 0), ofColor(255, 255)));

    // OFX CV //
    contourFinder.setMinAreaRadius(min);
    contourFinder.setMaxAreaRadius(max);
    contourFinder.setThreshold(cvThresh);
    // wait for half a second before forgetting something
    contourFinder.getTracker().setPersistence(15);
    // an object can move up to 32 pixels per frame
    contourFinder.getTracker().setMaximumDistance(32);
    
    pix.allocate(w,h,OF_PIXELS_RGBA);
    pix.begin();
    ofClear(255,255,255);
    pix.end();
    
    fbo.allocate(w,h,GL_RGBA);
    fbo.begin();
    ofClear(255,255,255);
    fbo.end();
    
    sender.setup(HOST, PORT);
    
    easingAmt = ofVec2f(0.005,0.005);

}


//--------------------------------------------------------------
void ofApp::update(){
    video.update();
    
    if ( video.isFrameNew() ) {
        //Store the previous frame, if it exists till now
        if ( grayImage.bAllocated ) {
            grayImagePrev = grayImage;
        }

        //Getting a new frame
        image.setFromPixels( video.getPixelsRef() );
        grayImage = image;    //Convert to grayscale image

        //Do processing if grayImagePrev is inited
        if ( grayImagePrev.bAllocated ) {
            //Get absolute difference
            diff.absDiff( grayImage, grayImagePrev );

            //We want to amplify the difference to obtain
            //better visibility of motion
            //We do it by multiplication. But to do it, we
            //need to convert diff to float image first
            diffFloat = diff;    //Convert to float image
            diffFloat *= threshold;

            //Update the accumulation buffer
            if ( !bufferFloat.bAllocated ) {
                //If the buffer is not initialized, then
                //just set it equal to diffFloat
                bufferFloat = diffFloat;
            }
            else {
                //Slow damping the buffer to zero
                bufferFloat *= damping;
                //Add current difference image to the buffer
                bufferFloat += diffFloat;
            }
        }
    }
    /// OFX CV
    
        contourFinder.setMinAreaRadius(min);
        contourFinder.setMaxAreaRadius(max);
//        contourFinder.setThreshold(cvThresh);
        // wait for half a second before forgetting something
        contourFinder.getTracker().setPersistence(60);
        // an object can move up to 32 pixels per frame
        contourFinder.getTracker().setMaximumDistance(128);
    
    if(video.isFrameNew()) {
           contourFinder.setTargetColor(color, TRACK_COLOR_RGB);
           contourFinder.setThreshold(cvThresh);
           //contourFinder.findContours(video);
           contourFinder.findContours(pix);
       }
    ///
    
  
}
//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground( 255, 255, 255 );    //Set the background color

    //Draw only if diffFloat image is ready.
    //It happens when the second frame from the video is obtained
    int w = grayImage.width;
    int h = grayImage.height;
    ofVec2f transPos = ofVec2f( w/2+10, h/2+10 );
    float scale = 1.0f;
    
    if ( diffFloat.bAllocated ) {
        //Get image dimensions
//        int w = grayImage.width;
//        int h = grayImage.height;
       
        //Set color for images drawing
        ofSetColor( 255, 255, 255 );

        //Draw images grayImage,  diffFloat, bufferFloat
       // grayImage.draw( 0, 0, w/2, h/2 );
       // diffFloat.draw( w/2 + 10, 0, w/2, h/2 );
       // bufferFloat.draw( 0, h/2 + 10, w/2, h/2 );

        //Draw the image motion areas

//        Shift and scale the coordinate system
      
        
        ofPushMatrix();
        ofTranslate( transPos );
        ofScale( scale, scale );
        //Draw bounding rectangle
        ofSetColor(ofColor::black);
        ofNoFill();
        ofDrawRectangle( -1, -1, w+2, h+2 );
        ofPopMatrix();
        
     
//        ofPushMatrix();
//        ofTranslate( w/2+10, h/2+10 );
//        ofScale( 0.5, 0.5 );
        //Get bufferFloat pixels
        float *pixels = bufferFloat.getPixelsAsFloats();
        //Scan all pixels
        fbo.begin();
        ofClear(255,255,255);
        ofSetColor(ofColor::black);
        for (int y=0; y<h; y++) {
            for (int x=0; x<w; x++) {
                //Get the pixel value
                float value = pixels[ x + w * y ];
                //If value exceed threshold, then draw pixel
                if ( value >= 0.9 ) {
                    ofDrawRectangle( x, y, 1, 1 );
                    //Rectangle with size 1x1 means pixel
                    //Note, this is slow function,
                    //we use it here just for simplicity
                }
            }
        }
        fbo.end();
        //ofPopMatrix();    //Restore the coordinate system
    }
    
    ofPushMatrix();
    ofTranslate( transPos );
    ofScale( scale, scale);
    fbo.draw(0,0);
    ofPopMatrix();
    
    fbo.readToPixels(pix);
    
    // OFXCV //
  
    //contourFinder.draw();
 

    for (int i = 0; i < contourFinder.size(); i++) {
        
        ofPushMatrix();
        ofTranslate( transPos );
        ofScale( scale, scale );
        
        ofVec2f pos = toOf(contourFinder.getCenter(i)); // screen
        ofVec2f velocity = toOf(contourFinder.getVelocity(i));
        
        smoothPos.resize(contourFinder.size());
        ofVec2f targetPos = pos;
        ofVec2f dPos = targetPos - smoothPos[i];
        smoothPos[i] += dPos * easingAmt;
        cout<<smoothPos.size()<<endl;


//        ofSetColor(ofColor::red);
//        ofSetCircleResolution(60);
//        ofDrawCircle(pos.x,pos.y,25,25);
//        ofPopMatrix();
        
        
        // ellipse that best fits the contour
        ofSetColor(ofColor::red);
        cv::RotatedRect ellipse = contourFinder.getFitEllipse(i);
        ofPushMatrix();
        ofVec2f ellipseCenter = toOf(ellipse.center);
        ofVec2f ellipseSize = toOf(ellipse.size);
        ofTranslate(ellipseCenter.x, ellipseCenter.y);
        ofDrawEllipse(0, 0, ellipseSize.x, ellipseSize.x);
        ofPopMatrix();

        //print data to screen//
        int x = -100;
        int y = i * 40 + 40;

        stringstream text;
    
        while(text.str().length() < 16) {
            text << " ";
        }
        
        ofVec2f normpos = smoothPos[i].normalize(); // normalized
        ofVec2f normvel = velocity.normalize();
        ofVec2f normsize = ellipseSize.normalize();

        text << "Centroid Index: " << i;
        text << " X: " << normpos.x;
        text << " Y: " << normpos.y;
        text << " Vel X: " << normvel.x;
        text << " Vel Y: " << normvel.y;
        
        
            
        ofPushMatrix();
        ofTranslate( w * .75, h * 0.15 );
        ofSetColor(255,0,0);
        ofDrawBitmapString(text.str(), x, y);
        text.str(""); // clear
        ofPopMatrix();
        
        ofxOscMessage posMsg;
        posMsg.setAddress("/blob/position" + ofToString(i));
        posMsg.addFloatArg(normpos.x);
        posMsg.addFloatArg(normpos.y);
        sender.sendMessage( posMsg, false);
        
 
        ofxOscMessage velMsg;
        velMsg.setAddress("/blob/velocity" + ofToString(i));
        velMsg.addFloatArg(normvel.x);
        velMsg.addFloatArg(normvel.y);
        sender.sendMessage( velMsg, false);
        
        ofxOscMessage sizeMsg;
        sizeMsg.setAddress("/blob/size" + ofToString(i));
        sizeMsg.addFloatArg(normsize.x);
        sizeMsg.addFloatArg(normsize.y);
        sender.sendMessage( sizeMsg, false);
        
        //
    }
    
    //
    
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    if (key == 's') {
        gui.saveToFile("settings.xml");
    }
    
    if (key == 'l') {
        gui.loadFromFile("settings.xml");
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

 
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
