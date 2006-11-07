#ifndef __PACKET_H__
#define __PACKET_H__

#include "common.h"
#include "vmath.h"

class Profile;
class Body;

class Packet : public NoCopy
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
        ID_KICKPLACES  = 6,
        ID_KICKNAME    = 7,

        ID_START = 8,
        ID_READY = 9,

        // game
        ID_UPDATE  = 10,
        ID_CONTROL = 11,
        ID_REFEREE_PROCESS = 12,
        ID_SOUND = 13,
    };

    const bytes& data() const;
    virtual ~Packet() {}

protected:
    Packet(int type);
    Packet(const bytes& data);

    byte   readByte();
    short  readShort();
    int    readInt();
    float  readFloat();
    string readString();

    void writeByte(byte x);
    void writeShort(short x);
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
    ControlPacket(byte idx, const Vector& direction, const Vector& rotation, bool jump, bool kick);

    Vector m_netDirection;
    Vector m_netRotation;
    bool   m_netJump;
    bool   m_netKick;
    byte   m_idx;
};

class UpdatePacket : public Packet
{
public:
    UpdatePacket(const bytes& data);
    UpdatePacket(byte idx, const Body* body);

    Matrix m_position;
    byte   m_idx;
};

class RefereeProcessPacket : public Packet
{
public:
    RefereeProcessPacket(const bytes& data);
    RefereeProcessPacket(int idx1, int idx2);

    int m_idx1;
    int m_idx2;
};

class JoinPacket : public Packet
{
public:
    JoinPacket(const bytes& data);
    JoinPacket(int idx, const string& version, Profile* profile);
    ~JoinPacket();

    int      m_idx;
    string   m_version;
    Profile* m_profile;
};

class KickPacket : public Packet
{
public:
    KickPacket(const bytes& data);
    KickPacket(const string& reason);

    string m_reason;
};

class KickNamesPacket : public Packet
{
public:
    KickNamesPacket(const bytes& data);
    KickNamesPacket();
};

class KickPlacesPacket : public Packet
{
public:
    KickPlacesPacket(const bytes& data);
    KickPlacesPacket();
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

class StartPacket : public Packet
{
public:
    StartPacket(const bytes& data);
    StartPacket();
};

class ReadyPacket : public Packet
{
public:
    ReadyPacket(const bytes& data);
    ReadyPacket();
};

class SoundPacket : public Packet
{
public:
    SoundPacket(const bytes& data);
    SoundPacket(byte id, const Vector& position);

    byte   m_id;
    Vector m_position;
};

#endif
