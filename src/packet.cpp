#include "packet.h"
#include "profile.h"
#include "body.h"

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
    string x(&m_data[m_pos], &m_data[m_pos+length]);
    m_pos += length;
    return x;
}

void Packet::writeByte(byte x)
{
    m_data.push_back(x);
    m_size++;
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
    m_netDirection.x = readFloat();
    m_netDirection.y = readFloat();
    m_netDirection.z = readFloat();
    m_netRotation.x = readFloat();
    m_netRotation.y = readFloat();
    m_netRotation.z = readFloat();
    m_netJump = readByte()==1;
    m_netKick = readByte()==1;
    m_idx = readByte();
}

ControlPacket::ControlPacket(byte idx, const Vector& direction, const Vector& rotation, bool jump, bool kick)
    : Packet(ID_CONTROL)
{
    writeFloat(direction.x);
    writeFloat(direction.y);
    writeFloat(direction.z);
    writeFloat(rotation.x);
    writeFloat(rotation.y);
    writeFloat(rotation.z);
    writeByte(jump ? 1 : 0);
    writeByte(kick ? 1 : 0);
    writeByte(idx);
}

JoinPacket::JoinPacket(const bytes& data) : Packet(data), m_profile(NULL)
{
    m_idx = readInt();
    m_profile = new Profile();
    m_profile->m_name = readString();
    m_profile->m_collisionID = readString();
    m_profile->m_color = Vector(readFloat(), readFloat(), readFloat());
    m_profile->m_accuracy = readFloat();
    m_profile->m_jump = readFloat();
    m_profile->m_speed = readFloat();
}

JoinPacket::JoinPacket(int idx, Profile* profile) : Packet(ID_JOIN), m_profile(NULL)
{
    writeInt(idx);
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

SetPlacePacket::SetPlacePacket(const bytes& data) : Packet(data)
{
    m_idx = readInt();
}

SetPlacePacket::SetPlacePacket(int idx) : Packet(ID_PLACE)
{
    writeInt(idx);
}

QuitPacket::QuitPacket(const bytes& data) : Packet(data)
{
}

QuitPacket::QuitPacket() : Packet(ID_QUIT)
{
}

StartPacket::StartPacket(const bytes& data) : Packet(data)
{
    m_ai_count = readByte();
}

StartPacket::StartPacket(int ai_count) : Packet(ID_START)
{
    writeByte(ai_count);
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
    for (int i=0; i<16; i++)
    {
        m_position[i] = readFloat();
    }
    m_speed.x = readFloat();
    m_speed.y = readFloat();
    m_speed.z = readFloat();
    m_omega.x = readFloat();
    m_omega.y = readFloat();
    m_omega.z = readFloat();
}

UpdatePacket::UpdatePacket(byte idx, const Body* body) : Packet(ID_UPDATE)
{
    writeByte(idx);
    m_position = body->m_matrix;
    m_speed = body->getVelocity();
    NewtonBodyGetOmega(body->m_newtonBody, m_omega.v);

    for (int i=0; i<16; i++)
    {
        writeFloat(m_position[i]);
    }
    writeFloat(m_speed.x);
    writeFloat(m_speed.y);
    writeFloat(m_speed.z);
    writeFloat(m_omega.x);
    writeFloat(m_omega.y);
    writeFloat(m_omega.z);
}
