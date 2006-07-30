#include "xml.h"
#include "utilities.h"


#include <istream>
#include <ostream>
#include <sstream>
#include <iomanip>

/** OUTPUT **/

std::string encode(const string& str)
{
    stringstream stream;
    for (size_t i=0; i<str.size(); ++i)
    {
        char ch = str[i];
        switch (ch)
        {
            case '<':  stream << "&lt;";   break;  
            case '>':  stream << "&gt;";   break;  
            case '&':  stream << "&amp;";  break;  
            case '"':  stream << "&quot;"; break;  
            case '\'': stream << "&apos;"; break;  
        default:
            if (ch<32 || ch>126)
            {
                stream << "&#x" << std::hex << std::setw(2) << std::setfill('0')
                    << static_cast<int>(ch) << ';';
            }
            else
            {
                stream << ch;
            }
        }
    }
    return stream.str();
}

void outputNode(const XMLnode& node, std::ostream& stream, int level)
{
    for (int i=0; i<level; i++)
    {
        stream << "  ";
    }

    stream << '<' << node.name;
    if (!node.attributes.empty())
    {
        for each_const(StringMap, node.attributes, iter)
        {
            stream << ' ' << iter->first << "=\"" << encode(iter->second) << '"';
        }
    }

    if (!node.value.empty())
    {
        stream << '>' << encode(node.value) << "</" << node.name << '>';
    }
    else if (node.childs.empty())
    {
        stream << " />";
    }
    else
    {
        stream << '>' << endl;
        for each_const(XMLnodes, node.childs, iter)
        {
            outputNode(*iter, stream, level + 1);
        }
        for (int i=0; i<level; i++)
        {
            stream << "  ";
        }
        stream << "</" << node.name << '>';
    }
    stream << endl;
}

void XMLnode::save(File::Writer& writer)
{
    std::stringstream ss;
    outputNode(*this, ss, 0);
    string s = ss.str();
    writer.write(s.c_str(), s.size());
}

/** INPUT **/

class XMLreader
{
public:
    XMLreader(File::Reader& reader) : 
        reader(reader), charBuf(0), curLine(1)
    {
    }

    void parse(XMLnode& xml)
    {
        xml.name = xml.value = "";
        xml.attributes.clear();
        xml.childs.clear();
    
        char ch;
        bool done = false;
        while (!done)
        {
            ch = scanWhite();

            if (ch != '<')
            {
                throw Exception(makeError("Expected <"));
            }

            ch = readChar();

            if (ch=='!' || ch=='?')
            {
                skipSpecialTag(0);
            }
            else
            {
                unreadChar(ch);
                scanElement(xml);
                done = true;
            }
        }
    }

private:
    File::Reader& reader;
    char charBuf;
    int curLine;

    string makeError(const string& errorMsg)
    {
        return errorMsg + " at line " + cast<string>(curLine) + "!";
    }

    char readChar()
    {
        char ch;
        if (charBuf != 0)
        {
            ch = charBuf;
            charBuf = 0;
        }
        else
        {
            if (reader.read(&ch, 1) != 1)
            {
                throw Exception(makeError("Failed to read stream"));
            }
            else if (ch=='\n')
            {
                curLine++;
            }
        }
        return ch;
    }

    void unreadChar(char ch)
    {
        charBuf = ch;
    }

    char scanWhite()
    {
        char ch;
        while (true)
        {
            ch = readChar();
            if (ch!=' ' && ch!='\t' && ch!='\n' && ch!='\r')
            {
                break;
            }
        }
        return ch;
    }

    char scanWhite(string& result)
    {
        char ch;
        while (true)
        {
            ch = readChar();
            if (ch==' ' || ch=='\t' || ch=='\n')
            {
                result.push_back(ch);
            }
            else if (ch!='\r')
            {
                break;
            }
        }
        return ch;
    }

    char decodeEntity()
    {
        char ch;
        string entity;
        while (true)
        {
            ch = readChar();
            if (ch==';')
            {
                break;
            }
            entity.push_back(ch);
        }

        if (entity[0]=='#')
        {
            stringstream ss;
            if (entity[1]=='x')
            {
                ss << std::hex << entity.substr(2);
            }
            else
            {
                ss << std::dec << entity.substr(1);
            }
            int i;
            ss >> i;
            if (static_cast<size_t>(ss.tellg()) != ss.str().size())
            {
                throw Exception(makeError("Unknown entity " + entity));
            }
            ch = static_cast<char>(i);
            return ch;
        }
        if (entity=="lt")
        {
            return '<';
        }
        else if (entity=="gt")
        {
            return '>';
        }
        else if (entity=="quot")
        {
            return '"';
        }
        else if (entity=="apos")
        {
            return '\''; 
        }
        else if (entity=="amp")
        {
            return '&';
        }
        throw Exception(makeError("Unknown entity " + entity));
    }

    string scanIdentifier()
    {
        string result;
        char ch;
        while (true)
        {
            ch = readChar();
            if (   ((ch<'A') || (ch>'Z')) 
                && ((ch<'a') || (ch>'z'))
                && ((ch<'0') || (ch>'9')) 
                && (ch!='_') && (ch != '.') && (ch!=':') && (ch!='-') && (ch<127))
            {
                unreadChar(ch);
                break;
            }
            result.push_back(ch);
        }
        return result;
    }

    string scanString()
    {
        string result;
        char delim = readChar();
        if ((delim!='\'' && delim!='"'))
        {
            return Exception(makeError("Expected ' or \""));
        }

        char ch;
        while (true)
        {
            ch = readChar();
            if (ch==delim)
            {
                break;
            }

            if (ch=='&')
            {
                ch = decodeEntity();
            }
            result.push_back(ch);
        }
        return result;
    }

    bool checkLiteral(const std::string& literal)
    {
        char ch;
        for (size_t i=0, len=literal.size(); i<len; ++i)
        {
            ch = readChar();
            if (ch!=literal[i])
            {
                return false;
            }
        }
        return true;
    }

    bool checkCDATA(std::string& data)
    {
        char ch = readChar();
        if (ch!='[')
        {
            unreadChar(ch);
            skipSpecialTag(0);
            return false;
        }

        if (!checkLiteral("CDATA["))
        {
            skipSpecialTag(1); // one [ has already been read
            return false;
        }

        int delimiterCharsSkipped = 0;
        while (delimiterCharsSkipped < 3)
        {
            ch = readChar();
            switch (ch)
            {
                case ']':
                    if (delimiterCharsSkipped < 2)
                    {
                        delimiterCharsSkipped += 1;
                    }
                    else
                    {
                        data += "]]";
                        delimiterCharsSkipped = 0;
                    }
                    break;
                case '>':
                    if (delimiterCharsSkipped < 2)
                    {
                        for (int i=0; i<delimiterCharsSkipped; ++i)
                        {
                            data.push_back(']');
                        }
                        delimiterCharsSkipped = 0;
                        data.push_back('>');
                    }
                    else
                    {
                        delimiterCharsSkipped = 3;
                    }
                    break;
                default:
                    for (int i=0; i<delimiterCharsSkipped; ++i)
                    {
                        data.push_back(']');
                    }
                    data.push_back(ch);
                    delimiterCharsSkipped = 0;
            }
        }
        return true;
    }

    void scanPCData(string& data)
    {
        char ch;
        while (true)
        {
            ch = readChar();
            if (ch=='<')
            {
                ch = readChar();
                if (ch=='!')
                {
                    checkCDATA(data);
                }
                else
                {
                    unreadChar(ch);
                    break;
                }
            }
            else if (ch=='&')
            {
                ch = decodeEntity();
            }
            data.push_back(ch);
        }
    }

    void skipComment()
    {
        int dashesToRead = 2;
        char ch;
        while (dashesToRead > 0)
        {
            ch = readChar();
            if (ch=='-')
            {
                dashesToRead--;
            }
            else
            {
                dashesToRead = 2;
            }
        }
        ch = readChar();
        if (ch!='>')
        {
            throw Exception(makeError("Expected >"));
        }
    }

    void skipSpecialTag(int bracketLevel)
    {
        int tagLevel = 1; // <
        char stringDelimiter = 0;
        char ch;
        if (bracketLevel == 0)
        {
            ch = readChar();
            if (ch=='[')
            {
                bracketLevel++;
            }
            else if (ch=='-')
            {
                ch = readChar();
                if (ch=='[')
                {
                    bracketLevel++;
                }
                else if (ch==']')
                {
                    bracketLevel--;
                }
                else if (ch=='-')
                {
                    skipComment();
                    return;
                }
            }
        }

        while (tagLevel > 0)
        {
            ch = readChar();
            if (stringDelimiter==0)
            {
                if (ch=='"' || ch=='\'')
                {
                    stringDelimiter = ch;
                }
                else if (bracketLevel <= 0)
                {
                    if (ch=='<')
                    {
                        tagLevel++;
                    }
                    else if (ch=='>')
                    {
                        tagLevel--;
                    }
                }

                if (ch=='[')
                {
                    bracketLevel++;
                }
                else if (ch == ']')
                {
                    bracketLevel--;
                }
            }
            else
            {
                if (ch==stringDelimiter)
                {
                    stringDelimiter = 0;
                }
            }
        }
    }

    void scanElement(XMLnode& xml)
    {
        xml.name = scanIdentifier();
        char ch = scanWhite();
        while (ch!='>' && ch!='/')
        {
            unreadChar(ch);
            string key = scanIdentifier();
            ch = scanWhite();
            if (ch!='=')
            {
                throw Exception(makeError("Expected ="));
            }
            ch = scanWhite();
            unreadChar(ch);
            string value = scanString();
            xml.attributes.insert(make_pair(key, value));
            ch = scanWhite();
        }

        if (ch=='/')
        {
            ch = readChar();
            if (ch!='>') 
            {
                throw Exception(makeError("Expected >"));
            }
            return;
        }

        string buf;
        ch = scanWhite(buf);
        if (ch!='<')
        {
            unreadChar(ch);
            scanPCData(buf);
        }
        else
        {
            while (true)
            {
                ch = readChar();
                if (ch=='!')
                {
                    if (checkCDATA(buf))
                    {
                        scanPCData(buf);
                        break;
                    }
                    else
                    {
                        ch = scanWhite(buf);
                        if (ch!='<')
                        {
                            unreadChar(ch);
                            scanPCData(buf);
                        }
                        break;
                    }
                }
                else
                { 
                    //if (ch!='/' || ignoreWhitespace)
                    //{
                        buf = "";
                    //}
                    //if (ch=='/')
                    //{
                        //unreadChar(ch);
                    //}
                    break;
                }
            }
        }

        if (buf.empty())
        {
            while (ch!='/')
            {
                if (ch=='!')
                {
                    ch = readChar();
                    if (ch!='-')
                    {
                        throw Exception(makeError("Expected Comment or Element"));
                    }
                    ch = readChar();
                    if (ch!='-')
                    {
                        throw Exception(makeError("Expected Comment or Element"));
                    }
                    skipComment();
                }
                else
                {
                    unreadChar(ch);
                    xml.childs.push_back(XMLnode());
                    xml.childs.back().line = curLine;
                    scanElement(xml.childs.back());
                }

                ch = scanWhite();
                if (ch!='<')
                {
                    throw Exception(makeError("Expected <"));
                }
                ch = readChar();
            }
            unreadChar(ch);
        }
        else
        {
            //if (ignoreWhitespace)
            //{
                xml.value = trim(buf);
            //}
            //else
            //{
            //    xml.value = buf;
            //}
        }

        ch = readChar();
        if (ch!='/')
        {
            throw Exception(makeError("Expected /"));
        }
        ch = scanWhite();
        unreadChar(ch);
        if (!checkLiteral(xml.name))
        {
            throw Exception(makeError("Expected " + xml.name));
        }
        ch = scanWhite();
        if (ch!='>')
        {
            throw Exception(makeError("Expected >"));
        }
    }

};

void XMLnode::load(File::Reader& reader)
{
    XMLreader xmlReader(reader);
    xmlReader.parse(*this);
}

/* CUSTOM */

string getAttribute(const XMLnode& node, const string& name)
{
    if (foundInMap(node.attributes, name))
    {
        return node.attributes.find(name)->second;
    }

    throw Exception("Missing attribute '" + name + "' in node '" + node.name + "'");
}

Vector getAttributesInVector(const XMLnode& node, const string& attributeSymbols)
{
    Vector vector;
    for (size_t i = 0; i < attributeSymbols.size(); i++)
    {
        string key(1, attributeSymbols[i]);
        vector[i] = cast<float>(getAttribute(node, key));
    }
    return vector;

}
