/*#include "Precompile.h"*/
#include "RenderEnvironment.h"

#include "Foundation/Checksum/CRC32.h"

using namespace Helium;
using namespace Helium::Render;

RenderEnvironment::RenderEnvironment(const tchar* fname)
{
    m_load_count = 0;
    m_filename = fname;
    m_crc = Helium::StringCrc32(fname);
    m_timestamp = (u64)-1L;
    m_env_texture = 0;
    m_clearcolor = 0;
    m_env_bias = 0.0f;
    m_env_scale = 1.0f;
}


RenderEnvironment::~RenderEnvironment()
{
    if (m_env_texture)
    {
        m_env_texture->Release();
        m_env_texture=0;
    }
}
