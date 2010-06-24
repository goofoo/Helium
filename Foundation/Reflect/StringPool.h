#pragma once

#include <vector>
#include <hash_map>

#include "Platform/Types.h"

#include "API.h"
#include "Stream.h" 

/*

String Pool Binary Format:

struct String
{
i32 size;           // string length
char[] data;        // characters
};

struct StringPool
{
i32 count;          // string count
String[] strings;   // string data
i32 term;           // -1
};

*/

namespace Reflect
{
    class ArchiveBinary; 

    //
    // String pool for serializing string data in binary
    //

    class FOUNDATION_API StringPool
    {
    public:
        typedef stdext::hash_map<std::string, int> M_StringToIndex;
        typedef stdext::hash_map<std::wstring, int> M_WideStringToIndex;

        M_StringToIndex             m_Indices; 
        std::vector< std::string >  m_Strings;

        M_WideStringToIndex         m_WideIndices; 
        std::vector< std::wstring > m_WideStrings;

        int Insert( const std::string& str );
        int Insert( const std::wstring& str );

        const std::string& GetString( const int index );
        const std::wstring& GetWideString( const int index );

        void Serialize    (ArchiveBinary* archive); 
        void Deserialize  (ArchiveBinary* archive); 

    private: 
        void SerializeDirect(Reflect::Stream& stream); 
        void SerializeCompressed(Reflect::Stream& stream); 

        void DeserializeDirect(Stream& stream); 
        void DeserializeCompressed(Stream& stream); 
    };
}
