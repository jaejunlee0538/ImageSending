#pragma once
#include "PicoloImageReader.h"
#include <opencv2/opencv.hpp>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <boost/filesystem.hpp>
class PicoloImageRecorder :
	public PicoloImageReader
{
public:
	PicoloImageRecorder(){

	}
	PicoloImageRecorder(const std::string& file_prefix, const std::string& output_path){
		setRecordProperties(file_prefix, output_path);
	}

	virtual ~PicoloImageRecorder(){

	}

	void setRecordProperties(const std::string& file_prefix, const std::string& output_path){
		m_file_prefix = file_prefix;
		m_output_path = output_path;
		m_count = 0;

		boost::filesystem::path path(m_output_path);
		path.append(connector);
		if (!boost::filesystem::exists(path)){
			boost::filesystem::create_directories(path);
		}
		full_path = path;
	}

	void userCallback(std::shared_ptr<cv::Mat>& img, MC::Channel & ch, MC::SignalInfo & sig){
		std::string time_stamp = "00";
		
		auto tmp = full_path;
		std::stringstream ss;
		ss <<m_file_prefix << "_" << std::setfill('0') << std::setw(8) << m_count++ << ".png";
		tmp.append(ss.str());

		cv::imwrite(tmp.string(), *img);
	}

	size_t m_count;
	std::string m_file_prefix;
	std::string m_output_path;
	boost::filesystem::path full_path;
};

