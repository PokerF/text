#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string.h>
#include <iostream>
#include "uart.h"
using namespace std;
using namespace cv;

//here the mian() start
int main(int argc, char** argv)
{
	int fd;                            //文件描述符  
	int err;                           //返回调用函数的状态  
	int len;
	char rcv_buf[100];
	char send_buf1[20] = "0x00";        //send data
	char send_buf2[20] = "0x01";
	char send_buf3[20] = "0x02";
	char recv_buf[20];		   //recv data
	VideoCapture cap;
	cap.open(1);
	if (argc != 2) {
		printf("Usage: %s /dev/ttySn 0(send data)/1 (receive data) \n", argv[0]);
		return FALSE;
	}
	fd = UART0_Open(fd, argv[1]); //打开串口，返回文件描述符
	do {
		err = UART0_Init(fd, 9600, 0, 8, 1, 'N');
		printf("Set Port Exactly!\n");
	} while (FALSE == err || FALSE == fd);
	//state red color recognition function
	Mat red_recognition(Mat dst, vector<Mat> hsv, Mat element, vector<vector<Point> > contours, char* flag);

	//state blue color recognition function
	Mat blue_recognition(Mat dst, vector<Mat> hsv, Mat element, vector<vector<Point> > contours, char* flag);

	//state gold color recognition function
	Mat gold_recognition(Mat dst, vector<Mat> hsv, Mat element, vector<vector<Point> > contours, char* flag);

	//state a VideoCapture class

	//state Mats class to handle frame  
	Mat frame, hsv_img, imgsrc, img_xor;

	//state three Mat to accept the return Mat from the color recognition 
	Mat red_recognition_image;
	Mat blue_recognition_image;
	Mat gold_recognition_image;

	//state a Mat element as the smallest matrix to handle frame 
	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));

	//state a three channel vector to store splited h,s,v Mat
	vector<Mat> hsv2(3), hsv1(3), hsv3(3), temp_mat(3), rgb(3);

	//state a point vector for the findcontours() function
	vector<vector<Point> > contours;

	//state a Rect vector for drawing a retangle
	vector<Rect> rect(contours.size());

	//define the output signal
	int i = 0;

	int x, y, width, height;
	int temp_area;

	char red_flag = 'N';
	char blue_flag = 'N';
	char gold_flag = 'N';
	//state some windows
	namedWindow("cammera", 0);
	namedWindow("img_xor", 0);

	if (!cap.isOpened())
	{
		cerr << "Couldn't open capture" << endl;
		return -1;
	}
	else
	{
		cout << "open the cammera successfully!" << endl;
	}
	while (1)
	{
		len = UART0_Recv(fd, rcv_buf, 9);
		if (len > 0)
		{
			rcv_buf[len] = '\0';
			printf("receive data is %s\n", rcv_buf);
			printf("len = %d\n", len);
			int i = strcmp(rcv_buf, "0x00\0");
			printf("result %d\n", i);
			if (strcmp(rcv_buf, "0x00") == 160) {
				while(1)
				{
					//capture a frame and handle it 
					cap >> frame;
					//check the frame if it is empty
					if (frame.empty())break;
					//create a counterpart named imsrc to operate
					frame.copyTo(imgsrc);
					//transform the RGB image to HSV image
					cvtColor(imgsrc,hsv_img,CV_RGB2HSV);

					//split HSV image into three single channel image (single Mat)
					/*statement:you have to split hsv_img Mat three times for each color recognition function,you can't use just 
					  one splited Mats for three function in a mian() function */
					split(hsv_img,hsv3);
					split(hsv_img,hsv2);
					split(hsv_img,hsv1);
					// split(imgsrc,rgb);
		
					//state three flags to sign the recongnized color

					 
					//use the color recognition function
					blue_recognition_image = blue_recognition(temp_mat[0],hsv2,element,contours,&blue_flag);
					red_recognition_image = red_recognition(temp_mat[1],hsv1,element,contours,&red_flag);	
					gold_recognition_image = gold_recognition(temp_mat[2],hsv3,element,contours,&gold_flag);

					//algorithm to jude which color is recognized
					if (red_flag == 'R'){
						img_xor = red_recognition_image;
						for (int i=0;i<2;i++){
							len = UART0_Send(fd, send_buf1, 4);
							if (len > 0)
								printf(" %d send data successful\n", i);
							else
								printf("send data failed!\n");	
						}
						break;
						
					}
		
					else if (blue_flag == 'B'){
						img_xor = blue_recognition_image;
						for (int i=0;i<2;i++){
							len = UART0_Send(fd, send_buf2, 4);
							if (len > 0)
								printf(" %d send data successful\n", i);
							else
								printf("send data failed!\n");	
						}
						break;
					}
					else if (gold_flag == 'G'){
						img_xor = gold_recognition_image;
						for (int i=0;i<2;i++){
							len = UART0_Send(fd, send_buf3, 4);
							if (len > 0)
								printf(" %d send data successful\n", i);
							else
								printf("send data failed!\n");	
						}
						break;
					}
					else img_xor = Mat::zeros(imgsrc.size(),CV_8UC1);
					//output the messages for user to judge	
					cout<<red_flag<<"    "<<blue_flag<<"     "<<gold_flag<<endl;

					//find contours
			    	findContours(img_xor,contours,RETR_EXTERNAL,CHAIN_APPROX_NONE);

			    	//use the rectangle to sign the special color place
					rect.resize(contours.size());
					for (i = 0;i < (int)contours.size();i++)
			    	{
			    		//draw the contours 
			    		drawContours(imgsrc,contours,i,Scalar(255),5,5);

					rect[i] = boundingRect(contours[i]);

					//calcaulate the rectangle area
					temp_area = rect[i].width*rect[i].height;

					//draw the biggest rectangle whose area is 11500 at least
					if(temp_area > 30000)
					{
						width = rect[i].width;
						height = rect[i].height;
						x = rect[i].x;
						y = rect[i].y;
						rectangle(imgsrc,Point(x,y),Point(x+width,y+height),Scalar(0,255,0),5);
					}
			    	}
			    	imshow("img_xor",img_xor);
					imshow("cammera",imgsrc);
					frame.release();
					if(waitKey(12) >=0 )break;
				}
			}
		}
		else
		{
			printf("cannot receive data\n");
		}
		UART0_Close(fd);
		cap.release();
		return 0;


	}
}
//red color recognition function
Mat red_recognition(Mat dst,vector<Mat> hsv,Mat element,vector<vector<Point> > contours,char* flag)
{
	/*red color recognition*/
	
	inRange(hsv[0],120,142,hsv[0]);
	inRange(hsv[1],50,60,hsv[1]);
	inRange(hsv[2],0,70,hsv[2]);
	    	//xor,not,and operation
	bitwise_xor(hsv[1],hsv[2],dst);
	bitwise_not(dst,dst);
	bitwise_and(dst,hsv[0],dst);

	//use the erode() function 
	erode(dst,dst,element);
	//dilate(img_xor,img_xor,element);
	morphologyEx(dst,dst,MORPH_OPEN,element);

	//find the contours and sign it 
	findContours(dst,contours,RETR_EXTERNAL,CHAIN_APPROX_NONE);

	int contours_area,temp_contour_area;
	for (int i = 0;i < (int)contours.size();i++)
	{
		contours_area = contourArea(contours[i],false);
		if (contours_area > temp_contour_area) temp_contour_area = contours_area;
	}
  	//use the erode() function 
	erode(dst,dst,element);

	//dilate(img_xor,img_xor,element);
	morphologyEx(dst,dst,MORPH_OPEN,element);

	//find the contours and sign it 
	//findContours(dst,contours,RETR_EXTERNAL,CHAIN_APPROX_NONE);
	if (temp_contour_area > 30000)* flag = 'R';
	else * flag = 'N';
	cout<<"red area : "<<temp_contour_area<<endl;
	//imshow("dst1",dst);
	return dst;

}	
//blue color recognition function
Mat blue_recognition(Mat dst,vector<Mat> hsv,Mat element,vector<vector<Point> > contours,char* flag)
{
 	// inRange(hsv[0],128,225,hsv[0]);
  // 	inRange(hsv[1],31,114,hsv[1]);
  //  	inRange(hsv[2],46,255,hsv[2]);
   	
  //  	bitwise_and(hsv[0],hsv[1],dst);
  //  	bitwise_not(hsv[2],hsv[2]);
  //  	bitwise_and(hsv[2],dst,dst);

    inRange(hsv[0],0,25,hsv[0]);
    inRange(hsv[1],0,40,hsv[1]);
    inRange(hsv[2],100,159,hsv[2]);
    bitwise_not(hsv[1],hsv[1]);
    bitwise_and(hsv[0],hsv[1],dst);
    bitwise_not(hsv[2],hsv[2]);
    bitwise_and(dst,hsv[2],dst);

    // inRange(hsv[0],2,15,hsv[0]);
    // inRange(hsv[1],0,100,hsv[1]);
    // inRange(hsv[2],125,159,hsv[2]);
    // bitwise_not(hsv[1],hsv[1]);
    // bitwise_and(hsv[0],hsv[1],dst);
    // bitwise_not(hsv[2],hsv[2]);
    // bitwise_and(dst,hsv[2],dst);

	//use the erode() function 
	erode(dst,dst,element);

	//dilate(img_xor,img_xor,element);
	morphologyEx(dst,dst,MORPH_OPEN,element);

	//find the contours and sign it 
	findContours(dst,contours,RETR_EXTERNAL,CHAIN_APPROX_NONE);

	int contours_area,temp_contour_area;
	for (int i = 0;i < (int)contours.size();i++)
	{
		contours_area = contourArea(contours[i],false);
		if (contours_area > temp_contour_area) temp_contour_area = contours_area;
	}
  	//use the erode() function 
	erode(dst,dst,element);

	//dilate(img_xor,img_xor,element);
	morphologyEx(dst,dst,MORPH_OPEN,element);

	//find the contours and sign it 
	//findContours(dst,contours,RETR_EXTERNAL,CHAIN_APPROX_NONE);
	
	if (temp_contour_area > 30000)* flag = 'B';
	else * flag = 'N';
	cout<<"blue area : "<<temp_contour_area<<endl;
	//imshow("dst2",dst);
	return dst;
	
}
//gold color recognition function
Mat gold_recognition(Mat dst,std::vector<Mat> hsv,Mat element,vector<vector<Point> > contours,char* flag)
{
    inRange(hsv[0],10,95,hsv[0]);
    inRange(hsv[1],87,215,hsv[1]);
    inRange(hsv[2],0,103,hsv[2]);
    
    bitwise_and(hsv[0],hsv[1],dst);
    bitwise_not(hsv[2],hsv[2]);
    bitwise_and(hsv[2],dst,dst);

	//use the erode() function 
	erode(dst,dst,element);

	morphologyEx(dst,dst,MORPH_OPEN,element);

	//find the contours and sign it 
	findContours(dst,contours,RETR_EXTERNAL,CHAIN_APPROX_NONE);

	int contours_area,temp_contour_area;
	for (int i = 0;i < (int)contours.size();i++)
	{
		contours_area = contourArea(contours[i],false);
		if (contours_area > temp_contour_area) temp_contour_area = contours_area;
	}

  	//use the erode() function 
	dilate(dst,dst,element);

	//dilate(img_xor,img_xor,element);
	morphologyEx(dst,dst,MORPH_OPEN,element);

	//find the contours and sign it 
	//findContours(dst,contours,RETR_EXTERNAL,CHAIN_APPROX_NONE);

	if (temp_contour_area > 30000)* flag = 'G';
	else * flag = 'N';
	cout<<"gold area : "<<temp_contour_area<<endl;
	//imshow("dst2",dst);
	return dst;
}



/*color recognition algorithm*/

// 			/*red color*/
	  // 	inRange(hsv[0],120,142,hsv[0]);
	  //  	inRange(hsv[1],50,60,hsv[1]);
	  //   	inRange(hsv[2],0,70,hsv[2]);
	  //   	//xor,not,and operation
	  //   	bitwise_xor(hsv[1],hsv[2],img_xor);
	  //   	bitwise_not(img_xor,img_xor);
	  //   	bitwise_and(img_xor,hsv[0],img_xor);

			/*bule color*/
/*		    inRange(hsv[0],2,15,hsv[0]);
		    inRange(hsv[1],0,100,hsv[1]);
		    inRange(hsv[2],125,159,hsv[2]);
		    bitwise_not(hsv[1],hsv[1]);
		    bitwise_and(hsv[0],hsv[1],img_xor);
		    bitwise_not(hsv[2],hsv[2]);
		    bitwise_and(img_xor,hsv[2],img_xor);*/

			/*gold color*/  
/*		    inRange(hsv[0],85,95,hsv[0]);
		    inRange(hsv[1],130,190,hsv[1]);
		    inRange(hsv[2],130,185,hsv[2]);
		    bitwise_and(hsv[0],hsv[1],img_xor);*/

		    // bitwise_not(hsv[2],hsv[2]);
	    	// bitwise_and(hsv[2],img_xor,img_xor);

  //   	//use the erode() function 
  //   	erode(img_xor,img_xor,element);
  //   	//dilate(img_xor,img_xor,element);
  //   	morphologyEx(img_xor,img_xor,MORPH_OPEN,element);
  //   	//find the contours and sign it 
