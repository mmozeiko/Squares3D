#ifndef __PACKET_H__
#define __PACKET_H__

#include "common.h"

class Packet : NoCopy
{
protected:
    enum
    {
        // lobby
        ID_JOIN = 1,
        ID_QUIT = 2,
        ID_KICK = 3,
        ID_CHAT = 4,

        ID_START = 5,
        ID_READY = 6,

        // game
        ID_UPDATE = 7,
        ID_CONTROL = 8
    };

    Packet(int type);
    Packet(const bytes& data);

    byte   readByte();
    int    readInt();
    float  readFloat();
    string readString();

    void writeByte(byte x);
    void writeInt(int x);
    void writeFloat(float x);
    void writeString(const string& x);

private:
    bytes  m_data;
    size_t m_pos;
    size_t m_size;
};

class ControlPacket : public Packet
{
public:
    ControlPacket(const bytes& data);
    ControlPacket(int control);

private:
    int m_control;

};

#endif
