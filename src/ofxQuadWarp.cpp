//
//  ofxQuadWarp.cpp
//  Created by lukasz karluk on 19/06/11.
//

#include "ofxQuadWarp.h"
#include "opencv2/calib3d.hpp"

ofxQuadWarp::ofxQuadWarp() {
	anchorSize = 10;
	anchorSizeHalf = anchorSize * 0.5;
	selectedCornerIndex = -1;
	highlightCornerIndex = -1;

	bMouseEnabled = bKeyboardShortcuts = false;
	bShow = bMove = bRotate = false;
}

ofxQuadWarp::~ofxQuadWarp() {
	disableMouseControls();
	disableKeyboardShortcuts();
}

//----------------------------------------------------- setup.
void ofxQuadWarp::setup() {
	enableMouseControls();
	enableKeyboardShortcuts();
	show();
}

//----------------------------------------------------- setters.
void ofxQuadWarp::setPosition(float x, float y) {
	position.x = x;
	position.y = y;
}

void ofxQuadWarp::setAnchorSize(float value) {
	anchorSize = value;
	anchorSizeHalf = anchorSize * 0.5;
}

//----------------------------------------------------- enable / disable.
void ofxQuadWarp::enable() {    // DEPRECATED.
	enableMouseControls();
}

void ofxQuadWarp::disable() {   // DEPRECATED.
	disableMouseControls();
}

void ofxQuadWarp::enableMouseControls() {
	if (bMouseEnabled == true) {
		return;
	}
	bMouseEnabled = true;
	ofAddListener(ofEvents().mouseMoved, this, &ofxQuadWarp::onMouseMoved);
	ofAddListener(ofEvents().mousePressed, this, &ofxQuadWarp::onMousePressed);
	ofAddListener(ofEvents().mouseDragged, this, &ofxQuadWarp::onMouseDragged);
	ofAddListener(ofEvents().mouseReleased, this, &ofxQuadWarp::onMouseReleased);
}

void ofxQuadWarp::disableMouseControls(bool disableSelection) {
	if (bMouseEnabled == false) {
		return;
	}
	bMouseEnabled = false;
	try {
		ofRemoveListener(ofEvents().mouseMoved, this, &ofxQuadWarp::onMouseMoved);
		ofRemoveListener(ofEvents().mousePressed, this, &ofxQuadWarp::onMousePressed);
		ofRemoveListener(ofEvents().mouseDragged, this, &ofxQuadWarp::onMouseDragged);
		ofRemoveListener(ofEvents().mouseReleased, this, &ofxQuadWarp::onMouseReleased);
	}
	catch (exception e) {
		return;
	}
	bMove = false;

	if (disableSelection) {
		selectedCornerIndex = -1;
	}
}

void ofxQuadWarp::enableKeyboardShortcuts() {
	if (bKeyboardShortcuts == true) {
		return;
	}
	bKeyboardShortcuts = true;
	ofAddListener(ofEvents().keyPressed, this, &ofxQuadWarp::keyPressed);
}

void ofxQuadWarp::disableKeyboardShortcuts(bool disableSelection) {
	if (bKeyboardShortcuts == false) {
		return;
	}
	bKeyboardShortcuts = false;
	try {
		ofRemoveListener(ofEvents().keyPressed, this, &ofxQuadWarp::keyPressed);
	}
	catch (exception e) {
		return;
	}

	if (disableSelection) {
		selectedCornerIndex = -1;
	}
}

//----------------------------------------------------- source / target points.
void ofxQuadWarp::setSourceRect(const ofRectangle& r) {
	srcPoints[0].set(r.x, r.y);
	srcPoints[1].set(r.x + r.width, r.y);
	srcPoints[2].set(r.x + r.width, r.y + r.height);
	srcPoints[3].set(r.x, r.y + r.height);
}

void ofxQuadWarp::setSourcePoints(const vector<ofPoint>& points) {
	int t = MIN(4, points.size());
	for (int i = 0; i < t; i++) {
		srcPoints[i].set(points[i]);
	}
}

ofPoint* ofxQuadWarp::getSourcePoints() {
	return &srcPoints[0];
}

void ofxQuadWarp::setTargetRect(const ofRectangle& r) {
	dstPoints[0].set(r.x, r.y);
	dstPoints[1].set(r.x + r.width, r.y);
	dstPoints[2].set(r.x + r.width, r.y + r.height);
	dstPoints[3].set(r.x, r.y + r.height);
}

void ofxQuadWarp::setTargetPoints(const vector<ofPoint>& points) {
	int t = MIN(4, points.size());
	for (int i = 0; i < t; i++) {
		dstPoints[i].set(points[i]);
	}
}

ofPoint* ofxQuadWarp::getTargetPoints() {
	return &dstPoints[0];
}

ofPoint ofxQuadWarp::getCenter() {
	return (dstPoints[0] + dstPoints[1] + dstPoints[2] + dstPoints[3]) / 4;
}

ofPoint* ofxQuadWarp::getSelectedPoint() {
	if (bShow == false || ofGetKeyPressed(rotateModifireKey)) {
		return nullptr;
	}
	if (selectedCornerIndex < 0 || selectedCornerIndex > 3) {
		return nullptr;
	}

	return &dstPoints[selectedCornerIndex];
}

//----------------------------------------------------- matrix.
ofMatrix4x4 ofxQuadWarp::getMatrix() const {
	return getMatrix(&srcPoints[0], &dstPoints[0]);
}

ofMatrix4x4 ofxQuadWarp::getMatrixInverse() const {
	return getMatrix(&dstPoints[0], &srcPoints[0]);
}

ofMatrix4x4 ofxQuadWarp::getMatrix(const ofPoint* srcPoints, const ofPoint* dstPoints) const {

	//we need our points as opencv points
	//be nice to do this without opencv?
	CvPoint2D32f cvsrc[4];
	CvPoint2D32f cvdst[4];

	//we set the warp coordinates
	//source coordinates as the dimensions of our window

	//this is the slightly easier - but supposidly less
	//accurate warping method 
	//cvWarpPerspectiveQMatrix(cvsrc, cvdst, translate); 
    vector<cv::Point2f> srcCvPoints, dstCvPoints;
    for (int i=0;i<4;++i) {
        srcCvPoints.push_back(cv::Point2f(srcPoints[i].x, srcPoints[i].y));
        dstCvPoints.push_back(cv::Point2f(dstPoints[i].x, dstPoints[i].y));
    }

	//figure out the warping!
    //from openCV - this is a 3x3 2D matrix that is
    //row ordered
    cv::Mat translate = cv::findHomography(srcCvPoints, dstCvPoints);
        
	//get the matrix as a list of floats
    auto mat = new float[9];
    cout << "matrix" << endl;
    for (int i=0; i<3; ++i) {
        for (int j=0;j<3;++j) {
            mat[i + j * 3] = translate.at<double>(i, j);
            cout << ' ' << translate.at<double>(i, j);
        }
        cout << endl;
    }

	//we need to copy these values
	//
	// ie:   [0][1][2] x
	//       [3][4][5] y
	//       [6][7][8] w

	//to openGL's 4x4 matrix
	//        x  y  z  w   
	// ie:   [0][1][ ][2]
	//       [3][4][ ][5]
	//		 [ ][ ][ ][ ]
	//       [6][7][ ][8]
	//       

	ofMatrix4x4 matrixTemp;
	matrixTemp.getPtr()[0] = mat[0];
    matrixTemp.getPtr()[1] = mat[1];
    matrixTemp.getPtr()[3] = mat[2];

    matrixTemp.getPtr()[4] = mat[3];
	matrixTemp.getPtr()[5] = mat[4];
    matrixTemp.getPtr()[7] = mat[5];

    matrixTemp.getPtr()[12] = mat[6];
    matrixTemp.getPtr()[13] = mat[7];
    matrixTemp.getPtr()[15] = mat[8];
	
    // release memory
    translate.release();
    delete[] mat;
    
	return matrixTemp;
}

void ofxQuadWarp::update() {
	//
}

void ofxQuadWarp::reset() {
	dstPoints[0].set(srcPoints[0]);
	dstPoints[1].set(srcPoints[1]);
	dstPoints[2].set(srcPoints[2]);
	dstPoints[3].set(srcPoints[3]);
}

//----------------------------------------------------- interaction.
void ofxQuadWarp::onMouseMoved(ofMouseEventArgs& mouseArgs) {
	if (bShow == false) {
		return;
	}

	ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
	mousePoint -= position;
	for (int i = 0; i < 4; i++) {
		ofPoint& dstPoint = dstPoints[i];
		if (mousePoint.distance(dstPoint) <= anchorSizeHalf) {
			highlightCornerIndex = i;
			return;
		}
	}
	highlightCornerIndex = -1;
}

void ofxQuadWarp::onMousePressed(ofMouseEventArgs& mouseArgs) {
	if (bShow == false) {
		return;
	}

	// rotate
	if (ofGetKeyPressed(rotateModifireKey)) {
		selectedCornerIndex = -1;
		memcpy(rotateBeginDstPoints, dstPoints, sizeof(ofPoint) * 4);
		rotateBeginMousePosition.set(mouseArgs.x, mouseArgs.y);
		rotateBeginCenter.set(getCenter());
		bRotate = true;
		return;
	}

	// move point
	ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
	mousePoint -= position;
	for (int i = 0; i < 4; i++) {
		ofPoint& dstPoint = dstPoints[i];
		if (mousePoint.distance(dstPoint) <= anchorSizeHalf) {
			dstPoint.set(mousePoint);
			selectedCornerIndex = i;
			return;
		}
	}
	selectedCornerIndex = -1;

	auto isPointInTriangle = [](ofPoint& p, ofPoint& a, ofPoint& b, ofPoint& c) {
		bool bA = (p.x - a.x) * (b.y - a.y) - (b.x - a.x) * (p.y - a.y) < 0; // z of (p - a) x (b - a)
		bool bB = (p.x - b.x) * (c.y - b.y) - (c.x - b.x) * (p.y - b.y) < 0; // z of (p - b) x (c - b)
		bool bC = (p.x - c.x) * (a.y - c.y) - (a.x - c.x) * (p.y - c.y) < 0; // z of (p - c) x (a - c)
		return bA == bB && bB == bC;
	};

	// if in the warper clicked, then move start
	bool inTheWarper = false;
	ofPoint m(mouseArgs.x, mouseArgs.y);
	for (int i = 0; i < 4; i += 2) {
		int j = (i + 1) % 4;
		int k = (i + 2) % 4;
		if (isPointInTriangle(m, dstPoints[i], dstPoints[j], dstPoints[k])) {
			inTheWarper = true;
			break;
		}
	}
	bMove = inTheWarper;
}

void ofxQuadWarp::onMouseDragged(ofMouseEventArgs& mouseArgs) {
	if (bShow == false) {
		return;
	}

	if (bRotate) {
		auto getAngle = [](ofPoint a, ofPoint b, ofPoint c) {
			auto A = a - b;
			auto C = c - b;
			auto angle = acos(A.dot(C) / A.length() / C.length());
			if (A.cross(C).z < 0) angle = -angle;
			return angle;
		};

		// angle of rotate from drag begin pos (rad)
		float rotateAngle = getAngle(rotateBeginMousePosition, rotateBeginCenter, ofPoint(mouseArgs.x, mouseArgs.y));

		// if rect mode, snap angle every 15 deg
		if (ofGetKeyPressed(rectmodeModifireKey)) {
			float q = round(rotateAngle / TWO_PI * 24);
			rotateAngle = q * TWO_PI / 24;
			ofLog() << q << ", " << rotateAngle;
		}

		// make rotate matrix
		ofMatrix4x4 mat;
		mat.translate(-rotateBeginCenter);
		mat.rotateRad(rotateAngle, 0, 0, 1);
		mat.translate(rotateBeginCenter);

		for (int i = 0; i < 4; ++i) {
			dstPoints[i] = mat.preMult(rotateBeginDstPoints[i]);
		}

		return;
	}

	ofVec2f moved(mouseArgs.x - ofGetPreviousMouseX(), mouseArgs.y - ofGetPreviousMouseY());
	auto movePoint = [=](int index) {
		dstPoints[index].set(dstPoints[index] + moved);
	};

	if (0 <= selectedCornerIndex && selectedCornerIndex < 4) {
		movePoint(selectedCornerIndex);

		// if shift key pressed, then rectangle transform
		if (ofGetKeyPressed(rectmodeModifireKey)) {
			rectangulize();
		}
	}
	else if (bMove) {
		for (int i = 0; i < 4; ++i) {
			movePoint(i);
		}
	}

}

void ofxQuadWarp::onMouseReleased(ofMouseEventArgs& mouseArgs) {
	if (bShow == false) {
		return;
	}
	if (selectedCornerIndex < 0 || selectedCornerIndex > 3) {
		return;
	}

	ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
	mousePoint -= position;
	dstPoints[selectedCornerIndex].set(mousePoint);
	bMove = bRotate = false;
}

void ofxQuadWarp::keyPressed(ofKeyEventArgs& keyArgs) {
	if (bShow == false) {
		return;
	}

	switch (keyArgs.key) {
	case '1':
		selectedCornerIndex = 0;
		break;
	case '2':
		selectedCornerIndex = 1;
		break;
	case '3':
		selectedCornerIndex = 2;
		break;
	case '4':
		selectedCornerIndex = 3;
		break;
	default:
		break;
	}

	if (selectedCornerIndex < 0 || selectedCornerIndex > 3) {
		return;
	}

	float nudgeAmount = 0.25;
	ofPoint& selectedPoint = dstPoints[selectedCornerIndex];

	switch (keyArgs.key) {
	case OF_KEY_LEFT:
		selectedPoint.x -= nudgeAmount;
		break;
	case OF_KEY_RIGHT:
		selectedPoint.x += nudgeAmount;
		break;
	case OF_KEY_UP:
		selectedPoint.y -= nudgeAmount;
		break;
	case OF_KEY_DOWN:
		selectedPoint.y += nudgeAmount;
		break;
	default:
		break;
	}
}

//----------------------------------------------------- corners.
void ofxQuadWarp::setCorners(const vector<ofPoint>& corners) {
	vector<ofPoint> _corners = corners;
	_corners.resize(4);
	setTopLeftCornerPosition(_corners[0]);
	setTopRightCornerPosition(_corners[1]);
	setBottomRightCornerPosition(_corners[2]);
	setBottomLeftCornerPosition(_corners[3]);
}

void ofxQuadWarp::setCorner(const ofPoint& p, int cornerIndex) {
	cornerIndex = ofClamp(cornerIndex, 0, 3);
	dstPoints[cornerIndex].set(p);
}

void ofxQuadWarp::setTopLeftCornerPosition(const ofPoint& p) {
	setCorner(p, 0);
}

void ofxQuadWarp::setTopRightCornerPosition(const ofPoint& p) {
	setCorner(p, 1);
}

void ofxQuadWarp::setBottomRightCornerPosition(const ofPoint& p) {
	setCorner(p, 2);
}

void ofxQuadWarp::setBottomLeftCornerPosition(const ofPoint& p) {
	setCorner(p, 3);
}

void ofxQuadWarp::rectangulize() {
	ofPoint a, b;

	switch (selectedCornerIndex) {
	case 0: case 2:
		a.set(dstPoints[0].x, dstPoints[2].y);
		b.set(dstPoints[2].x, dstPoints[0].y);

		// compatible to flip/rotate
		if ((dstPoints[3] - a).lengthSquared() <= (dstPoints[1] - a).lengthSquared()) {
			dstPoints[3].set(a);
			dstPoints[1].set(b);
		}
		else {
			dstPoints[3].set(b);
			dstPoints[1].set(a);
		}
		break;
	case 1: case 3:
		a.set(dstPoints[3].x, dstPoints[1].y);
		b.set(dstPoints[1].x, dstPoints[3].y);

		// compatible to flip/rotate
		if ((dstPoints[0] - a).lengthSquared() <= (dstPoints[2] - a).lengthSquared()) {
			dstPoints[0].set(a);
			dstPoints[2].set(b);
		}
		else {
			dstPoints[0].set(b);
			dstPoints[2].set(a);
		}
		break;
	}
}

//----------------------------------------------------- show / hide.
void ofxQuadWarp::show() {
	if (bShow) {
		return;
	}
	toggleShow();
}

void ofxQuadWarp::hide() {
	if (!bShow) {
		return;
	}
	toggleShow();
}

void ofxQuadWarp::toggleShow() {
	bShow = !bShow;
}

bool ofxQuadWarp::isShowing() {
	return bShow;
}

//----------------------------------------------------- save / load.
void ofxQuadWarp::save(const string& path) {
	ofXml xml;
	ofXml quadWarpTag = xml.appendChild("quadwarp");

	ofXml srcTag = quadWarpTag.appendChild("src");
	for (int i = 0; i < 4; i++) {
		auto pointTag = srcTag.appendChild("point");
		pointTag.setAttribute("x", ofToString(srcPoints[i].x));
		pointTag.setAttribute("y", ofToString(srcPoints[i].y));
	}

	ofXml dstTag = quadWarpTag.appendChild("dst");
	for (int i = 0; i < 4; i++) {
		ofXml pointTag = dstTag.appendChild("point");
		pointTag.setAttribute("x", ofToString(dstPoints[i].x));
		pointTag.setAttribute("y", ofToString(dstPoints[i].y));
	}

	xml.save(path);
}

void ofxQuadWarp::load(const string& path) {
	ofXml xml;
	bool bOk = xml.load(path);
	if (bOk == false) {
		return;
	}

	auto quadWarpTag = xml.getChild("quadwarp");
	bOk = quadWarpTag;
	if (bOk == false) {
		return;
	}

	auto srcTag = quadWarpTag.getChild("src");
	bOk = srcTag;
	if (bOk == false) {
		return;
	}

	auto srcPointTags = srcTag.find("point");
	bOk = srcPointTags.size() >= 4;
	if (bOk == false) {
		return;
	}
	for (int i = 0; i < srcPointTags.size(); i++) {
		srcPoints[i].x = srcPointTags[i].getAttribute("x").getFloatValue();
		srcPoints[i].y = srcPointTags[i].getAttribute("y").getFloatValue();
	}

	auto dstTag = quadWarpTag.getChild("dst");
	bOk = dstTag;
	if (bOk == false) {
		return;
	}

	auto dstPointTags = dstTag.find("point");
	bOk = dstPointTags.size() >= 4;
	if (bOk == false) {
		return;
	}
	for (int i = 0; i < dstPointTags.size(); i++) {
		dstPoints[i].x = dstPointTags[i].getAttribute("x").getFloatValue();
		dstPoints[i].y = dstPointTags[i].getAttribute("y").getFloatValue();
	}
}

//----------------------------------------------------- show / hide.
void ofxQuadWarp::draw() {
	if (bShow == false) {
		return;
	}

	drawQuadOutline();
	drawCorners();
	drawHighlightedCorner();
	drawSelectedCorner();
	drawRotateHandle();
}

void ofxQuadWarp::drawQuadOutline() {
	if (bShow == false) {
		return;
	}

	for (int i = 0; i < 4; i++) {
		int j = (i + 1) % 4;
		ofDrawLine(dstPoints[i].x + position.x,
			dstPoints[i].y + position.y,
			dstPoints[j].x + position.x,
			dstPoints[j].y + position.y);
	}
}

void ofxQuadWarp::drawCorners() {
	if (bShow == false || ofGetKeyPressed(rotateModifireKey)) {
		return;
	}

	for (int i = 0; i < 4; i++) {
		ofPoint& point = dstPoints[i];
		drawCornerAt(point);
	}
}

void ofxQuadWarp::drawHighlightedCorner() {
	if (bShow == false || ofGetKeyPressed(rotateModifireKey)) {
		return;
	}
	if (highlightCornerIndex < 0 || highlightCornerIndex > 3) {
		return;
	}

	ofPoint& point = dstPoints[highlightCornerIndex];
	drawCornerAt(point);
}

void ofxQuadWarp::drawSelectedCorner() {
	if (bShow == false || ofGetKeyPressed(rotateModifireKey)) {
		return;
	}
	if (selectedCornerIndex < 0 || selectedCornerIndex > 3) {
		return;
	}

	ofPoint& point = dstPoints[selectedCornerIndex];
	drawCornerAt(point);
}

void ofxQuadWarp::drawCornerAt(const ofPoint& point) {
	ofDrawRectangle(point.x + position.x - anchorSizeHalf,
		point.y + position.y - anchorSizeHalf,
		anchorSize, anchorSize);
}

void ofxQuadWarp::drawRotateHandle() {
	if (ofGetKeyPressed(rotateModifireKey) == false) {
		return;
	}

	ofPushMatrix();
	ofNoFill();
	ofSetCircleResolution(24);
	auto center = getCenter();
	float radius = (center - ofVec2f(ofGetMouseX(), ofGetMouseY())).length();
	ofTranslate(center);
	ofDrawCircle(0, 0, radius);

	if (ofGetKeyPressed(rectmodeModifireKey)) {
		int skip = 15; // per 15 degree
		for (int angle = 0; angle < 360; angle += skip) {
			float length = 10;
			ofDrawLine(0, radius - length, 0, radius + length);
			ofRotate(skip);
		}
	}
	ofPopMatrix();
}
