#include "packet.h"
#include "profile.h"

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
    m_control = readInt();
}

ControlPacket::ControlPacket(int control) : Packet(ID_CONTROL)
{
    writeInt(control);
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
