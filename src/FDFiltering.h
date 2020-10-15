/*
 * FrequencyDomainFiltering.h
 *
 *  Created on: 10-Oct-2020
 *      Author: Stephen Pilli 
 */

#ifndef FDFILTERING_H_
#define FDFILTERING_H_


#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

class FDFiltering {

private:

	void fftshift(const cv::Mat& inputImg, cv::Mat& outputImg);
	void filter2DFreq(const cv::Mat& inputImg, cv::Mat& outputImg, const cv::Mat& H);
	void synthesizeFilterH(cv::Mat& inputOutput_H, cv::Point center, int radius);
	void calcPSD(const cv::Mat& inputImg, cv::Mat& outputImg, int flag = 0);

public:
	FDFiltering();
	virtual ~FDFiltering();
	cv::Mat forward(cv::Mat image, std::vector<cv::Point3d> filters );
	cv::Mat forward(cv::Mat image, cv::Mat& filter);

};

#endif /* FDFILTERING_H_ */
