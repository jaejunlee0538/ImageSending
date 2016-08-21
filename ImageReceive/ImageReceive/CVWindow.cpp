#include "CVWindow.h"


void CVUniqueWindows::imshow(const int& id, const cv::Mat& img){
	int idx = findWindow(id);
	if (idx < 0){
		windows.push_back(CVWindow());
		windows.back().init(id);
		windows.back().imshow(img);
	}
	else{
		windows[idx].imshow(img);
	}
}

int CVUniqueWindows::findWindow(const int& id){
	int i = 0;
	for (; i < windows.size(); i++){
		if (windows[i].m_id == id)
			return i;
	}
	return -1;
}
std::vector<CVWindow> CVUniqueWindows::windows;