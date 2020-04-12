//
//  ofxQuadWarp.h
//  Created by lukasz karluk on 19/06/11.
//
//  nay added hack to invert on 2011/06/21
//
#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "calib3d_c.h"

class ofxQuadWarp 
{
public: 
    
     ofxQuadWarp();
    ~ofxQuadWarp();
    
    void setup();
    
    void setPosition(float x, float y);
    void setAnchorSize(float value);
    
    void setSourceRect(const ofRectangle& rect);
    void setSourcePoints(const vector<ofPoint>& points);
    ofPoint* getSourcePoints();
    void setTargetRect(const ofRectangle& rect);
    void setTargetPoints(const vector<ofPoint>& points);
    ofPoint* getTargetPoints();
	ofPoint getCenter();
    ofPoint* getSelectedPoint();

    OF_DEPRECATED_MSG("Use enableMouseControls() and enableKeyboardShortcuts() instead.", void enable());
    OF_DEPRECATED_MSG("Use disableMouseControls() and disableKeyboardShortcuts() instead.", void disable());
    
    void enableMouseControls();
    void disableMouseControls(bool disableSelection = true);
    
    void enableKeyboardShortcuts();
    void disableKeyboardShortcuts(bool disableSelection = true);
    
    void update();
    void reset();
    
    ofMatrix4x4 getMatrix() const;
    ofMatrix4x4 getMatrixInverse() const;
    ofMatrix4x4 getMatrix(const ofPoint * srcPoints, const ofPoint * dstPoints) const;
    
    void setCorners(const vector<ofPoint>& corners);
    void setCorner(const ofPoint& p, int cornerIndex);
    void setTopLeftCornerPosition(const ofPoint& p);
    void setTopRightCornerPosition(const ofPoint& p);
    void setBottomRightCornerPosition(const ofPoint& p);
    void setBottomLeftCornerPosition(const ofPoint& p);
    bool isCornerSelected(int cornerIndex) { return selectedCornerIndex == cornerIndex; }
    void rectangulize();

    void show();
    void hide();
    void toggleShow();
    bool isShowing();
    
    void save(const string& path="quadwarp.xml");
    void load(const string& path="quadwarp.xml");
    
    void draw();
    void drawQuadOutline();
    void drawCorners();
    void drawHighlightedCorner();
    void drawSelectedCorner();
    void drawCornerAt(const ofPoint& point);
	void drawRotateHandle();
    
    ofPoint srcPoints[4];
    ofPoint dstPoints[4];
    
protected:

    void onMouseMoved(ofMouseEventArgs & mouseArgs);
    void onMousePressed(ofMouseEventArgs & mouseArgs);
    void onMouseDragged(ofMouseEventArgs & mouseArgs);
    void onMouseReleased(ofMouseEventArgs & mouseArgs);
    void keyPressed(ofKeyEventArgs & keyArgs);

    ofPoint position;
    
	float anchorSize;
    float anchorSizeHalf;
    int selectedCornerIndex;
    int highlightCornerIndex;
    
    bool bMouseEnabled;
    bool bKeyboardShortcuts;
    bool bShow;
	bool bMove;
	bool bRotate;
	ofPoint rotateBeginDstPoints[4];
	ofPoint rotateBeginMousePosition, rotateBeginCenter;

	static const ofKey rectmodeModifireKey = OF_KEY_SHIFT;
	static const ofKey rotateModifireKey = OF_KEY_ALT;
};