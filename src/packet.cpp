#include "packet.h"
#include "profile.h"
#include "body.h"
#include "version.h"

Packet::Packet(int type) :
    m_data(),
    m_pos(0),
    m_size(0)
{
    writeByte(type);
}

Packet::Packet(const bytes& data) :
    m_data(data),
    m_size(data.size()),
    m_pos(1) // skip type
{
}

byte Packet::readByte()
{
    if (m_pos+1 > m_size)
    {
        clog << "WARNING: " << Exception("m_pos+1 > m_size") << endl;
        return 0;
    }
    return m_data[m_pos++];
}

short Packet::readShort()
{
    if (m_pos+2 > m_size)
    {
        clog << "WARNING: " << Exception("m_pos+2 > m_size") << endl;
        return 0;
    }
    short x = m_data[m_pos++] << 0;
    x |= (m_data[m_pos++] << 8);
    return x;
}

int Packet::readInt()
{
    if (m_pos+4 > m_size)
    {
        clog << "WARNING: " << Exception("m_pos+4 > m_size") << endl;
        return 0;
    }
    int x = m_data[m_pos++] << 0;
    x |= (m_data[m_pos++] << 8);
    x |= (m_data[m_pos++] << 16);
    x |= (m_data[m_pos++] << 24);
    return x;
}

float Packet::readFloat()
{
    if (m_pos+4 > m_size)
    {
        clog << "WARNING: " << Exception("m_pos+4 > m_size") << endl;
        return 0.0f;
    }
    float x = *reinterpret_cast<float*>(&m_data[m_pos]);
    m_pos += 4;
    return x;
}

string Packet::readString()
{
    byte length = readByte();
    if (m_pos+length > m_size)
    {
        clog << "WARNING: " << Exception("m_pos+length > m_size") << endl;
        return "";
    }
    string x;
    for (size_t i=m_pos; i<m_pos+length; i++)
    {
        x.push_back(m_data[i]);
    }
    m_pos += length;
    return x;
}

void Packet::writeByte(byte x)
{
    m_data.push_back(x);
    m_size++;
}

void Packet::writeShort(short x)
{
    m_data.push_back((x >> 0) & 0xFF);
    m_data.push_back((x >> 8) & 0xFF);
    m_size += 2;
}

void Packet::writeInt(int x)
{
    m_data.push_back((x >> 0) & 0xFF);
    m_data.push_back((x >> 8) & 0xFF);
    m_data.push_back((x >> 16) & 0xFF);
    m_data.push_back((x >> 24) & 0xFF);
    m_size += 4;
}

void Packet::writeFloat(float x)
{
    m_data.resize(m_size+4);
    *reinterpret_cast<float*>(&m_data[m_size]) = x;
    m_size += 4;
}

void Packet::writeString(const string& x)
{
    string xx = x;
    if (x.size() > 255)
    {
        clog << "Network packet warning: " << Exception("x.size() > 255") << endl;
        xx = x.substr(0, 255);
    }
    m_data.push_back(static_cast<byte>(xx.size()));
    m_size++;
    m_data.resize(m_size+xx.size());
    for (size_t i=0; i<xx.size(); i++)
    {
        m_data[m_size + i] = xx[i];
    }
    m_size += xx.size();
}

const bytes& Packet::data() const
{
    return m_data;
}

ControlPacket::ControlPacket(const bytes& data) : Packet(data)
{
    m_netDirection.x = readShort()/512.0f;
    m_netDirection.y = readShort()/512.0f;
    m_netDirection.z = readShort()/512.0f;
    m_netRotation.x = readShort()/512.0f;
    m_netRotation.y = readShort()/512.0f;
    m_netRotation.z = readShort()/512.0f;
	byte idxJumpKick = readByte();
    m_idx = idxJumpKick & 15;
	m_netJump = (idxJumpKick & 16) > 0;
    m_netKick = (idxJumpKick & 32) > 0;

}

ControlPacket::ControlPacket(byte idx, const Vector& direction, const Vector& rotation, bool jump, bool kick)
    : Packet(ID_CONTROL)
{
    writeShort(static_cast<short>(direction.x*512.0f));
    writeShort(static_cast<short>(direction.y*512.0f));
    writeShort(static_cast<short>(direction.z*512.0f));
    writeShort(static_cast<short>(rotation.x*512.0f));
    writeShort(static_cast<short>(rotation.y*512.0f));
    writeShort(static_cast<short>(rotation.z*512.0f));
	byte idxJumpKick = idx | (jump ? 16 : 0) | (kick ? 32 : 0);
    writeByte(idxJumpKick);
}

JoinPacket::JoinPacket(const bytes& data) : Packet(data), m_profile(NULL)
{
    m_idx = readInt();
    m_version = readString();
    m_profile = new Profile();
    m_profile->m_name = readString();
    m_profile->m_collisionID = readString();
    float x = readFloat();
    float y = readFloat();
    float z = readFloat();
    m_profile->m_color = Vector(x, y, z);
    m_profile->m_accuracy = readFloat();
    m_profile->m_jump = readFloat();
    m_profile->m_speed = readFloat();
}

RefereePacket::RefereePacket(int faultID, int bodyID, int points) : Packet(ID_REFEREE)
{
	byte faultIDbodyID = ((faultID << 4) & 0xF0) | (bodyID & 0x0F);
    writeByte(faultIDbodyID);
    writeByte(static_cast<byte>(points));
}

RefereePacket::RefereePacket(const bytes& data) : Packet(data)
{
	byte faultIDbodyID = readByte();
    m_faultID = faultIDbodyID >> 4;
    m_bodyID = faultIDbodyID & 15;
    m_points = static_cast<int>(readByte());
}

ComboIncPacket::ComboIncPacket(int bodyID) : Packet(ID_INCCOMBO)
{
    writeByte(static_cast<byte>(bodyID));
}

ComboIncPacket::ComboIncPacket(const bytes& data) : Packet(data)
{
    m_bodyID = static_cast<int>(readByte());
}

ComboResetOwnPacket::ComboResetOwnPacket(int bodyID) :
    Packet(ID_RESETOWNCOMBO)
{
    writeByte(static_cast<byte>(bodyID));
}

ComboResetOwnPacket::ComboResetOwnPacket(const bytes& data) : Packet(data)
{
    m_bodyID = static_cast<int>(readByte());
}

ComboResetPacket::ComboResetPacket() : Packet(ID_RESETCOMBO)
{
}

ComboResetPacket::ComboResetPacket(const bytes& data) : Packet(data)
{
}

JoinPacket::JoinPacket(int idx, const string& version, Profile* profile) : Packet(ID_JOIN), m_profile(NULL)
{
    writeInt(idx);
    writeString(g_version);
    writeString(profile->m_name);
    writeString(profile->m_collisionID);
    writeFloat(profile->m_color.x);
    writeFloat(profile->m_color.y);
    writeFloat(profile->m_color.z);
    writeFloat(profile->m_accuracy);
    writeFloat(profile->m_jump);
    writeFloat(profile->m_speed);
}

JoinPacket::~JoinPacket()
{
    if (m_profile != NULL)
    {
        delete m_profile;
    }
}

KickPacket::KickPacket(const bytes& data) : Packet(data)
{
    m_reason = readString();
}

KickPacket::KickPacket(const string& reason) : Packet(ID_KICK)
{
    writeString(reason);
}

KickNamesPacket::KickNamesPacket(const bytes& data) : Packet(data)
{
}

KickPlacesPacket::KickPlacesPacket() : Packet(ID_KICKPLACES)
{
}

KickPlacesPacket::KickPlacesPacket(const bytes& data) : Packet(data)
{
}

KickNamesPacket::KickNamesPacket() : Packet(ID_KICKNAME)
{
}

SetPlacePacket::SetPlacePacket(const bytes& data) : Packet(data)
{
    m_idx = readByte();
    m_level = readByte();
}

SetPlacePacket::SetPlacePacket(int idx, byte level) : Packet(ID_PLACE)
{
    writeByte(idx);
    writeByte(level);
}

QuitPacket::QuitPacket(const bytes& data) : Packet(data)
{
}

QuitPacket::QuitPacket() : Packet(ID_QUIT)
{
}

StartPacket::StartPacket(const bytes& data) : Packet(data)
{
}

StartPacket::StartPacket() : Packet(ID_START)
{
}

ReadyPacket::ReadyPacket(const bytes& data) : Packet(data)
{
}

ReadyPacket::ReadyPacket() : Packet(ID_READY)
{
}

UpdatePacket::UpdatePacket(const bytes& data) : Packet(data)
{
    m_idx = readByte();

    float euler[3];
    m_position = Matrix::identity();
    euler[0] = readShort()/512.0f;
    euler[1] = readShort()/512.0f;
    euler[2] = readShort()/512.0f;
    NewtonSetEulerAngle(euler, m_position.m);
    m_position[12] = readShort()/512.0f;
    m_position[13] = readShort()/512.0f;
    m_position[14] = readShort()/512.0f;
}

UpdatePacket::UpdatePacket(byte idx, const Body* body) : Packet(ID_UPDATE)
{
    writeByte(idx);
    m_position = body->m_matrix;

    float euler[3];
    NewtonGetEulerAngle(m_position.m, euler);

    writeShort(static_cast<short>(std::floor(euler[0]*512.0f)));
    writeShort(static_cast<short>(std::floor(euler[1]*512.0f)));
    writeShort(static_cast<short>(std::floor(euler[2]*512.0f)));
    writeShort(static_cast<short>(std::floor(m_position[12]*512.0f)));
    writeShort(static_cast<short>(std::floor(m_position[13]*512.0f)));
    writeShort(static_cast<short>(std::floor(m_position[14]*512.0f)));
}

SoundPacket::SoundPacket(const bytes& data) : Packet(data)
{
    m_id = readByte();
    m_position.x = readShort()*512.0f;
    m_position.y = readShort()*512.0f;
    m_position.z = readShort()*512.0f;
}

SoundPacket::SoundPacket(byte id, const Vector& position) : Packet(ID_SOUND)
{
    writeByte(id);
    writeShort(static_cast<short>(std::floor(position.x*512.0f)));
    writeShort(static_cast<short>(std::floor(position.y*512.0f)));
    writeShort(static_cast<short>(std::floor(position.z*512.0f)));
}

ChatPacket::ChatPacket(const bytes& data) : Packet(data)
{
    m_player = readByte();
    m_msg = readString();
}

ChatPacket::ChatPacket(byte player, const string& msg) : Packet(ID_CHAT)
{
    writeByte(player);
    writeString(msg);
}
