#ifndef SERIALIZE_HELPER_H_
#define SERIALIZE_HELPER_H_
#include <stdint.h>
#include <string.h>
class SerializeHelper{
public:
	SerializeHelper(void * outBuffer)
		:outBuffer((uint8_t*)outBuffer){

	}

	//template <typename T>
	//void load(const T& data){
	//	memcpy(outBuffer, &data, sizeof(T));
	//	outBuffer += sizeof(T);
	//}
	void load(const int32_t& data){
		memcpy(outBuffer, &data, sizeof(int32_t));
		outBuffer += sizeof(int32_t);
	}

	void load(const uint32_t& data){
		memcpy(outBuffer, &data, sizeof(uint32_t));
		outBuffer += sizeof(uint32_t);
	}

	void load(const uint8_t& data){
		memcpy(outBuffer, &data, sizeof(uint8_t));
		outBuffer += sizeof(uint8_t);
	}

	void load(const void* data, const size_t& bytes){
		memcpy(outBuffer, data, bytes);
		outBuffer += bytes;
	}

protected:
	uint8_t * outBuffer;
};

class DeserialzeHelper{
public:
	DeserialzeHelper(const void * inBuffer)
		:inBuffer((const uint8_t *)inBuffer){

	}

	//template <typename T>
	//void unload(const T& data){
	//	memcpy(&data, inBuffer, sizeof(T));
	//	inBuffer += sizeof(T);
	//}

	void unload(int32_t& data){
		memcpy(&data, inBuffer, sizeof(int32_t));
		inBuffer += sizeof(int32_t);
	}

	void unload(uint8_t& data){
		memcpy(&data, inBuffer, sizeof(uint8_t));
		inBuffer += sizeof(uint8_t);
	}

	void unload(uint32_t& data){
		memcpy(&data, inBuffer, sizeof(uint32_t));
		inBuffer += sizeof(uint32_t);
	}

	void unload(void* data, const size_t& bytes){
		memcpy(data, inBuffer, bytes);
		inBuffer += bytes;
	}
protected:
	const uint8_t *  inBuffer;
};
#endif