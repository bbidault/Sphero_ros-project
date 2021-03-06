//sphero_opencv, part of the sphero_move project, 2016
//Author : Boris Bidault

//Most of the Open CV part of the code was found online, but without any source of whom is the original writer

#include "ros/ros.h"
#include "std_msgs/ColorRGBA.h"
#include "geometry_msgs/Twist.h"
#include "std_msgs/MultiArrayLayout.h"
#include "std_msgs/MultiArrayDimension.h"
#include "std_msgs/Int32MultiArray.h"
#include <sstream>

#include "opencv/cvaux.h"
#include "opencv/highgui.h"
#include "opencv/cxcore.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "math.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>



using namespace cv;
using namespace std;


int main(int argc, char **argv){

	//Publisher initialisation 
	ros::init(argc, argv, "opencv");
	ros::NodeHandle n;
	ros::Publisher pub = n.advertise<std_msgs::Int32MultiArray>("array",100);	//The position publisher

	//Webcam initialisation
  	VideoCapture cap(0); //capture the video from web cam
  	if ( !cap.isOpened() )  // if not success, exit program
    	{
      		cout << "Cannot open the web cam" << endl;
     		return -1;
    	}


	//The control window, not usefull here since the parameters are already rightly setted
  	namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

	//The different parameters are setted here 
  	int iLowH = 0;
  	int iHighH = 255;
  	int iLowS = 0; 
  	int iHighS = 255;
  	int iLowV = 255;
  	int iHighV = 255;

  	//Create trackbars in "Control" window
  	cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
  	cvCreateTrackbar("HighH", "Control", &iHighH, 179);
  	cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
  	cvCreateTrackbar("HighS", "Control", &iHighS, 255);
  	cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
  	cvCreateTrackbar("HighV", "Control", &iHighV, 255);

  	namedWindow("Original", CV_WINDOW_AUTOSIZE); //create a window called "Original"

	while (ros::ok())
  	{ 

		Mat imgOriginal;	//The original image red from the video

    		bool bSuccess = cap.read(imgOriginal); // read a new frame from video

      		if (!bSuccess){		 //if not success, break loop
	  		cout << "Cannot read a frame from video stream" << endl;
	  		return 0;
        	}

      		Mat imgHSV; 	//HSV version of the original image

      		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

		Mat imgThresholded;	//Thresholded version of the HSV image

		inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

		//morphological opening (remove small objects from the foreground)
      		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      		dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 

      		//morphological closing (fill small holes in the foreground)
      		dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(15,15)) ); 
      		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(15,15)) );

		Moments oMoments = moments(imgThresholded);

      		double dM01 = oMoments.m01;
      		double dM10 = oMoments.m10;
      		double dArea = oMoments.m00;

      		float posX, posY, size;

      		if (dArea > 10000){
	  		//calculate the position of the sphero
	  		posX = dM10 / dArea ;
	  		posY = dM01 / dArea;
	  		size = (float)sqrt( dArea/3.14)/15;
		}

      		imshow("Original", imgOriginal); 	//show the original image
      		imshow("Thresholded", imgThresholded);


		std_msgs::Int32MultiArray array;
		array.data.clear();

		array.data.push_back(posX);
		array.data.push_back(posY);
		array.data.push_back(size);
		
		pub.publish(array);

    		ros::spinOnce();


    		if (waitKey(30) == 27){		 //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
			break; 
		}

	}
	
	return 0;

}

