#include "stdafx.h"

#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

struct infoControl
{
	// H
	int iLowH;
	int iHighH;
	// S
	int iLowS;
	int iHighS;
	// V
	int iLowV;
	int iHighV;

	int iAreaSize;
};

// find all connect components
void FindBlobs(const Mat &binary, vector < vector<Point2i> > &blobs)
{
    blobs.clear();

    // Fill the label_image with the blobs
    // 0  - background
    // 1  - unlabelled foreground
    // 2+ - labelled foreground

    Mat label_image;
    binary.convertTo(label_image, CV_32SC1);

    int label_count = 2; // starts at 2 because 0,1 are used already

    for(int y=0; y < label_image.rows; y++) {
        int *row = (int*)label_image.ptr(y);
        for(int x=0; x < label_image.cols; x++) {
            if(row[x] != 1) {
                continue;
            }

            Rect rect;
            floodFill(label_image, Point(x,y), label_count, &rect, 0, 0, 4);

            vector <Point2i> blob;

            for(int i=rect.y; i < (rect.y+rect.height); i++) {
                int *row2 = (int*)label_image.ptr(i);
                for(int j=rect.x; j < (rect.x+rect.width); j++) {
                    if(row2[j] != label_count) {
                        continue;
                    }

                    blob.push_back(cv::Point2i(j,i));
                }
            }

            blobs.push_back(blob);

            label_count++;
        }
    }
}

// input CV_8UC3 RGB image
Vec2i FindFingerPoint(Mat &imgFinger, const Mat &imgOriginal, infoControl infocontrol){

	Vec2i fpos;
	fpos[0] = 0;
	fpos[1] = 0;

	//Convert the captured frame from BGR to HSV
	Mat imgHSV;
	cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);
	
	//Threshold the image
	Mat imgThresholded;
	inRange(imgHSV, Scalar(infocontrol.iLowH, infocontrol.iLowS, infocontrol.iLowV), Scalar(infocontrol.iHighH, infocontrol.iHighS, infocontrol.iHighV), imgThresholded);

	//morphological opening (removes small objects from the foreground)
	erode( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
	dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
	//morphological closing (removes small holes from the foreground)
	dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
	erode( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

	// 1. ------ Calculate the moments of the thresholded image
	
	// Create a black image with the size as the camera output
	///*
	imgThresholded *= 1./255;

	vector <vector<cv::Point2i > > blobs;
	FindBlobs(imgThresholded, blobs);

	if( blobs.size() > 0 ){

		// find the bigest blob.
		for(size_t i = 0; i < blobs.size(); i++){
			for(size_t j=0; j < blobs[i].size(); j++) {
				int x = blobs[i][j].x;
				int y = blobs[i][j].y;

				imgFinger.at<Vec3b>(y,x)[0] = 128;
				imgFinger.at<Vec3b>(y,x)[1] = 128;
				imgFinger.at<Vec3b>(y,x)[2] = 128;
			}
		}
	}
	//*/
	

	// 2. ------ Find The bigest connect component and calculate the center of it.
		
	/*
	imgThresholded *= 1./255;

	vector <vector<cv::Point2i > > blobs;
	FindBlobs(imgThresholded, blobs);

	if( blobs.size() > 0 ){

		// find the bigest blob.
		int LBlob = 0;						// mark the largest blob.
		int LBlobSize = blobs[0].size();	// mark the largest blob's size.
		for(size_t i = 0; i < blobs.size(); i++){
			if( blobs[i].size() > LBlobSize ){
				LBlob = i;
				LBlobSize = blobs[i].size();
			}
		}

		// color the bigest blob
		if ( blobs[LBlob].size() > infocontrol.iAreaSize ){
			int cxall = 0;
			int cyall = 0;
			for(size_t j=0; j < blobs[LBlob].size(); j++) {
				int x = blobs[LBlob][j].x;
				int y = blobs[LBlob][j].y;
				cxall += x;
				cyall += y;

				imgFinger.at<Vec3b>(y,x)[0] = 60;
				imgFinger.at<Vec3b>(y,x)[1] = 60;
				imgFinger.at<Vec3b>(y,x)[2] = 60;
			}
			fpos[0] = int(cyall / blobs[LBlob].size());
			fpos[1] = int(cxall / blobs[LBlob].size());
			imgFinger.at<Vec3b>(fpos[0],fpos[1])[0] = 255;
			imgFinger.at<Vec3b>(fpos[0],fpos[1])[1] = 255;
			imgFinger.at<Vec3b>(fpos[0],fpos[1])[2] = 255;
		}
	}
	*/

	//imshow("Thresholded Image", imgFinger);			//show the thresholded image
	//imshow("Original", imgOriginal);				//show the original image

	return fpos;
}

void ChooseFingerColor(infoControl &infocontrol){
	
	//create a window called "Control"
    namedWindow("Control",CV_WINDOW_AUTOSIZE);

	//Create trackbars in "Control" window
	cvCreateTrackbar("LowH", "Control", &infocontrol.iLowH, 255);		//Hue (0 - 255)
	cvCreateTrackbar("HighH", "Control", &infocontrol.iHighH, 255);

	cvCreateTrackbar("LowS", "Control", &infocontrol.iLowS, 255);		//Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Control", &infocontrol.iHighS, 255);

	cvCreateTrackbar("LowV", "Control", &infocontrol.iLowV, 255);		//Value (0 - 255)
	cvCreateTrackbar("HighV", "Control", &infocontrol.iHighV, 255);

	cvCreateTrackbar("AreaSizeThreshold", "Control", &infocontrol.iAreaSize, 1000);//Value (0 - 100000)

}

int main( int argc, char** argv )
{
	//capture the video from web cam
    VideoCapture cap(0); 
    if ( !cap.isOpened() )  
    {
		// if not success, exit program
        cout << "Cannot open the web cam" << endl;
        return -1;
    }

	// ------ Control Window ------
	// Use to control the ROI Color.

	infoControl ifctl;
	// H
	ifctl.iLowH = 38;
	ifctl.iHighH = 75;
	// S
	ifctl.iLowS = 90;
	ifctl.iHighS = 150;
	// V
	ifctl.iLowV = 130;
	ifctl.iHighV = 250;
	// Area Size Threshold
	ifctl.iAreaSize = 200;

	ChooseFingerColor(ifctl);

	// ------ Original Image Window / Result Image Window ------
	// Use to show original/result image.

    while (true){
		
		// read a new RGB frame from video
		Mat imgOriginal;
		bool bSuccess = cap.read(imgOriginal); 
		if (!bSuccess){
			//if not success, break loop
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}

		Mat imgFinger = Mat::zeros( imgOriginal.size(), CV_8UC3 );
		Vec2i fpos = FindFingerPoint(imgFinger, imgOriginal, ifctl);
		
		imshow("Original", imgOriginal);				//show the original image
		imshow("Thresholded Image", imgFinger);			//show the thresholded image
		cout << "finger pos: " << fpos[0] << ", " << fpos[1] << endl;

		//wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        if (waitKey(30) == 27){
            cout << "esc key is pressed by user" << endl;
            break; 
		}
    }
	return 0;
}