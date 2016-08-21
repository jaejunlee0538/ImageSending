//
// Created by ub1404 on 16. 8. 20.
//

#ifndef RECVTESTUBUNTU_IMAGEPACKET_H
#define RECVTESTUBUNTU_IMAGEPACKET_H
#include <stdint.h>
#include <stdlib.h>
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
    DeserialzeHelper(void * inBuffer)
            :inBuffer((uint8_t*)inBuffer){

    }

    //template <typename T>
    //void unload(const T& data){
    //	memcpy(&data, inBuffer, sizeof(T));
    //	inBuffer += sizeof(T);
    //}

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
    uint8_t * inBuffer;
};

struct ImagePacket{
    uint32_t len;//ÆÐÅ¶ ¹ÙÀÌÆ®Œö(len Æ÷ÇÔ)
    uint32_t width;
    uint32_t height;
    uint8_t bytes;
    uint8_t * buffer;
    uint32_t bufferLen;

    ImagePacket(){
        len = width = height = bufferLen = 0;
        bytes = 0;
        buffer = NULL;
    }

    ImagePacket(const uint32_t& width, const uint32_t& height, const uint8_t& bytes){
        this->init(width, height, bytes);
    }

    ~ImagePacket(){
        delete[] buffer;
    }

    void init(const uint32_t& width, const uint32_t& height, const uint8_t& bytes){
        this->width = width;
        this->height = height;
        this->bytes = bytes;
        this->buffer = NULL;
        this->initBuffer();
    }

    void initBuffer(){
        uint32_t newBufferLen = sizeof(uint8_t)* bytes*width*height;
        if (newBufferLen == bufferLen)
            return;
        if (buffer){
            delete[] buffer;
        }
        bufferLen = newBufferLen;
        buffer = new uint8_t[bufferLen];
        len = sizeof(width) + sizeof(height) +
              sizeof(bytes) + bufferLen;
    }

    void load(void* out){
        SerializeHelper loader(out);
        loader.load(len);
        loader.load(width);
        loader.load(height);
        loader.load(bytes);
        loader.load(buffer, bufferLen);
    }

    bool unload(void * in, const size_t& packLen){
        DeserialzeHelper unloader(in);
        unloader.unload(len);
        if(len != packLen){
            fprintf(stderr, "Packet length info in the packet and actual packet length are different.(%d vs %d)\n", len, packLen);
            return false;
        }
        unloader.unload(width);
        unloader.unload(height);
        unloader.unload(bytes);
        initBuffer();
        unloader.unload(buffer, bufferLen);
        return true;
    }
};

#endif //RECVTESTUBUNTU_IMAGEPACKET_H
