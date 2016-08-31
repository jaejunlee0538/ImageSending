#include "PicoloReader\PicoloImageReader.h"
#include <sstream>

size_t PicoloImageReader::getBoardCount(){
	return MC::Boards.GetCount();
}

const std::string& PicoloImageReader::getConnectorName() const {
	return this->connector;
}

void PicoloImageReader::getBoardsInfo(std::vector<BoardInfo>& boardsInfo){
	int nBoards = MC::Boards.GetCount();
	char buffer[100];
	boardsInfo.resize(nBoards);
	for (int i = 0; i < nBoards; i++){
		boardsInfo[i].boardIndex = i;
		MC::Boards.GetBoardByDriverIndex(i)->GetParam(MC_BoardType, buffer, 100);
		boardsInfo[i].boardType = buffer;
		MC::Boards.GetBoardByDriverIndex(i)->GetParam(MC_BoardName, buffer, 100);
		boardsInfo[i].boardName = buffer;
	}
}

std::string PicoloImageReader::getBoardsInfoString(){
	std::vector<BoardInfo> infos;
	PicoloImageReader::getBoardsInfo(infos);
	
	std::ostringstream oss;
	if (!infos.empty()){
		oss << "Index\t\tBoardName\t\tBoardType" << std::endl;
		for (int i = 0; i < infos.size(); i++){
			oss << infos[i].boardIndex << "\t\t" << infos[i].boardName << "\t\t" << infos[i].boardType << std::endl;
		}
	}
	else{
		oss << "Picolo boards aren't detected.\n";
	}
	
	return oss.str();
}