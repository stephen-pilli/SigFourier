#include <opencv2/core/mat.hpp>
#include <opencv2/core/mat.inl.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <sys/types.h>
#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

#include "FDFiltering.h"


typedef struct Signal{
	float x, y, r;
}Signal;

void preprocessing(cv::Mat& in, float bin_thresh);
void signalExtraction(cv::Mat in, std::vector<Signal> &out);
void signalMembership(std::vector<Signal> signals , std::vector<Signal>& out, float signal_membership_threshold);
void varInRadii(std::vector<Signal> signals, float &mean, float &stdv, float &var, std::vector<Signal>& out);
void calculateMSD(std::vector<float> data, float &mean, float &stdv);

int main(int argc, char **argv) {


	std::string input_image_path = argv[1];
	std::string output_path = argv[2];
	std::string filter_path = "";
	
	if(argc > 3)
		filter_path = argv[3];

	float radii_variance_threshold = 800;
	float signal_membership_threshold = -1;
	float bin_threshold = 50;

	if(argc > 4) 
		radii_variance_threshold = std::stoi(argv[4]);
	
	if(argc > 5) 
		signal_membership_threshold = std::stoi(argv[5]);

	if(argc > 6) 
		bin_threshold = std::stoi(argv[6]);

	cv::Mat filter;
	cv::Mat imgIn = cv::imread(input_image_path);
	
	if(filter_path != "")
		filter = cv::imread(filter_path);
	
	cv::Mat outputImage = imgIn.clone();

	std::vector<cv::Point3d> circle;
	circle.push_back(cv::Point3d(320, 240, 21));

	//frequency domain filtering
	FDFiltering fdf;
        cv::Mat fdresponse;

        if(filter_path == "")
		fdresponse = fdf.forward(imgIn, circle);
	else
		fdresponse = fdf.forward(imgIn, filter);

	//preprocessing
	preprocessing(fdresponse, bin_threshold);

	//signal extraction
	std::vector<Signal> signals, varSignals, memberSignals;
	signalExtraction(fdresponse, signals);

	//variance in radii for thresholding
	float mean_r, stddev_r, var_r;
	varInRadii(signals, mean_r, stddev_r, var_r, varSignals);

	//signal membership.
	signalMembership(varSignals, memberSignals, signal_membership_threshold);


	if(var_r < radii_variance_threshold)
		for(uint i = 0 ; i < signals.size() ; ++i)
			try{
				cv::circle(outputImage, cv::Point(memberSignals[i].x, memberSignals[i].y), memberSignals[i].r,cv::Scalar(36, 255, 12), 2, 8);
			}catch(std::exception e){
				std::cout<<e.what()<<std::endl;
			}

	cv::imwrite(output_path, outputImage);

	return 0;
}

inline void calculateMSD(std::vector<float> data, float &mean, float &stdv)
{
	float sum = 0.0;
	float mean_ = 0.0;
	float stdv_ = 0.0;

	for(uint i = 0; i < data.size(); ++i)
		sum += data[i];

	mean_ = sum/data.size();

	for(uint i = 0; i < data.size(); ++i)
		stdv_ += pow(data[i] - mean_, 2);

	stdv = sqrt(stdv_ / data.size());
	mean = mean_;
}

inline void signalMembership(std::vector<Signal> signals , std::vector<Signal>& out, float signal_membership_threshold) {

	std::vector<float> stds;
	for(uint i = 0 ; i < signals.size(); ++i){
		std::vector<float> dcols;
		for(uint j = 0 ; j < signals.size(); ++j){
			float x1 = signals[i].x;
			float x2 = signals[j].x;
			float y1 = signals[i].y;
			float y2 = signals[j].y;
			float distance = std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2) * 1.0);
			dcols.push_back(distance);
		}
		std::sort(dcols.begin(), dcols.end());
		std::vector<float> dataVec;
		dataVec.push_back(dcols[1]);
		dataVec.push_back(dcols[2]);
		float mean, stdv;
		calculateMSD(dataVec, mean, stdv);
		stds.push_back(stdv);
	}

	float mean, stdv;

	calculateMSD(stds, mean, stdv);

	if(signal_membership_threshold == -1){
		for(uint i = 0 ; i < stds.size() ; ++i){
			if( stds[i] < mean+ stdv){
				out.push_back(signals[i]);
			}
		}

	}else{
		for(uint i = 0 ; i < stds.size() ; ++i){
			if( stds[i] < mean+ signal_membership_threshold){
				out.push_back(signals[i]);
			}
		}
	}
}

inline void varInRadii(std::vector<Signal> signals, float &mean, float &stdv, float &var, std::vector<Signal>& out){

	std::vector<float> data;

	for(uint i = 0 ; i < signals.size() ; ++i){
		data.push_back(signals[i].r);
	}

	calculateMSD(data, mean, stdv);
	var = stdv*stdv;

	for(uint i = 0 ; i < signals.size() ; ++i){
		if(signals[i].r > mean - stdv && signals[i].r < mean /*+ stdv*/ )
			out.push_back(signals[i]);
	}

}


inline void preprocessing(cv::Mat& in, float bin_thresh){

	cv::Mat gray, thresh;
	cv::threshold(in, thresh, bin_thresh, 255, cv::THRESH_BINARY);

	int morph_size = 3;
	cv::Mat kernel_1 = cv::getStructuringElement( cv::MORPH_RECT, cv::Size(morph_size, morph_size), cv::Point(-1, -1));
	cv::morphologyEx( thresh, thresh, cv::MORPH_OPEN, kernel_1, cv::Point(-1, -1), 1);

	cv::Mat kernel_2 = cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size(morph_size, morph_size), cv::Point(-1, -1));
	cv::morphologyEx( thresh, in, cv::MORPH_DILATE, kernel_2, cv::Point(-1, -1), 10);
}


inline void signalExtraction(cv::Mat in, std::vector<Signal>& out) {

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(in, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );

	std::vector<cv::Point2f> centers(contours.size());
	std::vector<float> radii_(contours.size());
	for(uint i = 0 ; i < contours.size() ; ++i)
		cv::minEnclosingCircle(contours[i], centers[i], radii_[i]);

	//cleaning
	for(int i = radii_.size()-1 ; i > -1 ; --i){
		if(radii_[i] > 5 && centers[i].x > 1 && centers[i].y > 1){
			out.push_back({centers[i].x, centers[i].y, radii_[i]});
		}
	}
}
