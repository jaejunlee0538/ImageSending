#pragma once
#include <opencv2/opencv.hpp>
#include "PreciseClock.h"
#include <iostream>
#include <sstream>

class CVWindow
{
public:
	void init(const int& id){
		m_id = id;
		windowName = std::string("Camera-") + std::to_string(m_id);
		cv::namedWindow(windowName);
	}

	void imshow(const cv::Mat& img){
		freq.update(1);
		std::ostringstream oss;
		oss << "FPS : " << freq.getFrequency() << " Hz";
		cv::putText(img, oss.str().c_str(), cv::Point(10, 50), 2, 1.2, cv::Scalar::all(255));
		cv::imshow(windowName,img);
	}

	int m_id;
	std::string windowName;
	FrequencyEstimator freq;
};

class CVUniqueWindows{
public:
	static void imshow(const int& id, const cv::Mat& img);
	static int findWindow(const int& id);
protected:
	static std::vector<CVWindow> windows;
};

