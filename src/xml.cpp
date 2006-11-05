#include <istream>
#include <ostream>
#include <sstream>
#include <iomanip>

#include <expat.h>

#include "xml.h"
#include "utilities.h"

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
        stream << "    ";
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
            stream << "    ";
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
private:
    File::Reader&  m_reader;
    XML_Parser     m_parser;

    bool           m_root;
    list<XMLnode*> m_elements;

    static const size_t BUFSIZE = 4096;

    static void XMLCALL StartElementHandler(void *userData, const XML_Char *name, const XML_Char **atts)
    {
        XMLreader* self = static_cast<XMLreader*>(userData);
        XMLnode* parent = self->m_elements.back();
        XMLnode* node;
        if (self->m_root)
        {
            self->m_root = false;
            node = parent;
            node->name = name;
        }
        else
        {
            parent->childs.push_back(XMLnode(name));
            node = & parent->childs.back();
        }

        while (*atts != NULL)
        {
            const XML_Char* name = *atts;
            atts++;
            const XML_Char* value = *atts;
            atts++;

            node->attributes.insert(make_pair(name, value));
        }
        node->line = XML_GetCurrentLineNumber(self->m_parser);
        self->m_elements.push_back(node);
    }

    static void XMLCALL EndElementHandler(void *userData, const XML_Char *name)
    {
        XMLreader* self = static_cast<XMLreader*>(userData);
        self->m_elements.back()->value = trim(self->m_elements.back()->value);
        self->m_elements.pop_back();
    }

    static void XMLCALL CharacterDataHandler(void *userData, const XML_Char *s, int len)
    {
        XMLreader* self = static_cast<XMLreader*>(userData);
        self->m_elements.back()->value += string(s, s+len);
    }

public:
    XMLreader(File::Reader& reader) : m_reader(reader), m_root(true)
    {
        m_parser = XML_ParserCreate(NULL);
        if (m_parser == NULL)
        {
            throw Exception("Error creating XML parser");
        }

        XML_SetUserData(m_parser, static_cast<void*>(this));
        XML_SetStartElementHandler(m_parser, StartElementHandler);
        XML_SetEndElementHandler(m_parser, EndElementHandler);
        XML_SetCharacterDataHandler(m_parser, CharacterDataHandler);
    }

    ~XMLreader()
    {
        XML_ParserFree(m_parser);
    }

    void parse(XMLnode& xml)
    {
        m_elements.push_back(&xml);
        while (! m_reader.eof())
        {
            void* buffer = XML_GetBuffer(m_parser, BUFSIZE);
            if (buffer == NULL)
            {
                throw Exception("Error alocating buffer for xml parsing");
            }

            size_t read = m_reader.read(buffer, BUFSIZE);
         
            if (XML_ParseBuffer(m_parser, static_cast<int>(read), m_reader.eof() || read==0 ? 1 : 0) == XML_STATUS_ERROR)
            {
                int c = static_cast<int>(XML_GetErrorColumnNumber(m_parser));
                int l = static_cast<int>(XML_GetErrorLineNumber(m_parser));
                const XML_LChar* err = XML_ErrorString(XML_GetErrorCode(m_parser));
                throw Exception("XML parser error (" + cast<string>(l) + ", " + cast<string>(c) + "): " + string(err));
            }
        }
    }
};

void XMLnode::load(File::Reader& reader)
{
    XMLreader xmlReader(reader);
    xmlReader.parse(*this);
}
