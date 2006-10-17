#ifndef __PACKET_H__
#define __PACKET_H__

#include "common.h"

class Profile;

class Packet : NoCopy
{
public:
    enum
    {
        // lobby
        ID_JOIN  = 1,
        ID_QUIT  = 2,
        ID_PLACE = 3,
        ID_KICK  = 4,
        ID_CHAT  = 5,

        ID_START = 6,
        ID_READY = 7,

        // game
        ID_UPDATE  = 8,
        ID_CONTROL = 9,
    };

    const bytes& data() const;

protected:
    Packet(int type);
    Packet(const bytes& data);
    virtual ~Packet() {}

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

    int m_control;
};

class JoinPacket : public Packet
{
public:
    JoinPacket(const bytes& data);
    JoinPacket(int idx, Profile* profile);
    ~JoinPacket();

    int      m_idx;
    Profile* m_profile;
};

class KickPacket : public Packet
{
public:
    KickPacket(const bytes& data);
    KickPacket(const string& reason);

    string m_reason;
};

class SetPlacePacket : public Packet
{
public:
    SetPlacePacket(const bytes& data);
    SetPlacePacket(int idx);

    int m_idx;
};

class QuitPacket : public Packet
{
public:
    QuitPacket(const bytes& data);
    QuitPacket();
};

#endif
