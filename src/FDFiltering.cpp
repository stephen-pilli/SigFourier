/*
 * FrequencyDomainFiltering.cpp
 *
 *  Created on: 10-Oct-2020
 *      Author: Stephen Pilli 
 */

#include "FDFiltering.h"

FDFiltering::FDFiltering() {}
FDFiltering::~FDFiltering() {}

void FDFiltering::fftshift(const cv::Mat& inputImg, cv::Mat& outputImg)
{
	outputImg = inputImg.clone();
	int cx = outputImg.cols / 2;
	int cy = outputImg.rows / 2;
	cv::Mat q0(outputImg, cv::Rect(0, 0, cx, cy));
	cv::Mat q1(outputImg, cv::Rect(cx, 0, cx, cy));
	cv::Mat q2(outputImg, cv::Rect(0, cy, cx, cy));
	cv::Mat q3(outputImg, cv::Rect(cx, cy, cx, cy));
	cv::Mat tmp;
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);
	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2);
}
void FDFiltering::filter2DFreq(const cv::Mat& inputImg, cv::Mat& outputImg, const cv::Mat& H)
{
	cv::Mat planes[2] = { cv::Mat_<float>(inputImg.clone()), cv::Mat::zeros(inputImg.size(), CV_32F) };
	cv::Mat complexI;
	merge(planes, 2, complexI);
	cv::dft(complexI, complexI, cv::DFT_SCALE);
	cv::Mat planesH[2] = { cv::Mat_<float>(H.clone()), cv::Mat::zeros(H.size(), CV_32F) };
	cv::Mat complexH;
	merge(planesH, 2, complexH);
	cv::Mat complexIH;
	mulSpectrums(complexI, complexH, complexIH, 0);
	idft(complexIH, complexIH);
	split(complexIH, planes);
	outputImg = planes[0];
}
void FDFiltering::synthesizeFilterH(cv::Mat& inputOutput_H, cv::Point center, int radius)
{
	cv::Point c2 = center, c3 = center, c4 = center;
	c2.y = inputOutput_H.rows - center.y;
	c3.x = inputOutput_H.cols - center.x;
	c4 = cv::Point(c3.x,c2.y);
	circle(inputOutput_H, center, radius, 0, -1, 8);
	circle(inputOutput_H, c2, radius, 0, -1, 8);
	circle(inputOutput_H, c3, radius, 0, -1, 8);
	circle(inputOutput_H, c4, radius, 0, -1, 8);
}

// Function calculates PSD(Power spectrum density) by fft with two flags
// flag = 0 means to return PSD
// flag = 1 means to return log(PSD)
void FDFiltering::calcPSD(const cv::Mat& inputImg, cv::Mat& outputImg, int flag)
{
	cv::Mat planes[2] = { cv::Mat_<float>(inputImg.clone()), cv::Mat::zeros(inputImg.size(), CV_32F) };
	cv::Mat complexI;
	merge(planes, 2, complexI);
	dft(complexI, complexI);
	split(complexI, planes);            // planes[0] = Re(DFT(I)), planes[1] = Im(DFT(I))
	planes[0].at<float>(0) = 0;
	planes[1].at<float>(0) = 0;
	// compute the PSD = sqrt(Re(DFT(I))^2 + Im(DFT(I))^2)^2
	cv::Mat imgPSD;
	magnitude(planes[0], planes[1], imgPSD);        //imgPSD = sqrt(Power spectrum density)
	pow(imgPSD, 2, imgPSD);                         //it needs ^2 in order to get PSD
	outputImg = imgPSD;
	// logPSD = log(1 + PSD)
	if (flag)
	{
		cv::Mat imglogPSD;
		imglogPSD = imgPSD + cv::Scalar::all(1);
		log(imglogPSD, imglogPSD);
		outputImg = imglogPSD;
	}
}

cv::Mat FDFiltering::forward(cv::Mat image, std::vector<cv::Point3d> filters ){

	cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);

	image.convertTo(image, CV_32F);

	cv::Rect roi = cv::Rect(0, 0, image.cols & -2, image.rows & -2);
	image = image(roi);

	cv::Mat imgPSD;
	calcPSD(image, imgPSD);
	fftshift(imgPSD, imgPSD);
	cv::normalize(imgPSD, imgPSD, 0, 255, cv::NORM_MINMAX);

	cv::Mat H = cv::Mat(roi.size(), CV_32F, cv::Scalar(1));
	for(uint i = 0 ; i < filters.size(); ++i)
		synthesizeFilterH(H,  cv::Point(filters[i].x,filters[i].y), filters[i].z);

	cv::Mat imgOut;
	fftshift(H, H);
	filter2DFreq(image, imgOut, H);

	imgOut.convertTo(imgOut, CV_8U);
	cv::normalize(imgOut, imgOut, 0, 255, cv::NORM_MINMAX);

	return imgOut;
}

cv::Mat FDFiltering::forward(cv::Mat image, cv::Mat& filter){

	cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);


	image.convertTo(image, CV_32F);

	cv::Rect roi = cv::Rect(0, 0, image.cols & -2, image.rows & -2);
	image = image(roi);

	cv::Mat imgPSD;
	calcPSD(image, imgPSD);
	fftshift(imgPSD, imgPSD);
	cv::normalize(imgPSD, imgPSD, 0, 255, cv::NORM_MINMAX);

	cv::Mat H;
	cv::cvtColor(filter, H, cv::COLOR_BGR2GRAY);

	H.convertTo(H, CV_32F);
	normalize(H, H, 0, 1, cv::NORM_MINMAX);

	cv::Mat imgOut;
	fftshift(H, H);
	filter2DFreq(image, imgOut, H);

	imgOut.convertTo(imgOut, CV_8U);
	cv::normalize(imgOut, imgOut, 0, 255, cv::NORM_MINMAX);

	return imgOut;
}

