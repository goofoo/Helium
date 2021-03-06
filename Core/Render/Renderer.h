#pragma once

#include "Core/API.h"

#include "Core/Render/DeviceManager.h"
#include "Core/Render/ShaderManager.h"
#include "Core/Render/RenderScene.h"

namespace Helium
{
    namespace Render
    {
        enum
        {
            DRAW_MODE_OFF,                        // dont draw the mesh, use when you just want wireframe
            DRAW_MODE_NORMAL,                     // full shader
            DRAW_MODE_DEBUG_COLOR,                // vertex colors
            DRAW_MODE_DEBUG_VERT_NORMAL,          // vertex normals
            DRAW_MODE_DEBUG_VERT_TANGENT,         // vertex tangents
            DRAW_MODE_DEBUG_UV,                   // vertex uv as color
            DRAW_MODE_DEBUG_BASEMAP,              // shader basemap
            DRAW_MODE_DEBUG_BASEMAP_ALPHA,        // shader basemap alpha channel
            DRAW_MODE_DEBUG_NORMALMAP,            // shader normal map
            DRAW_MODE_DEBUG_GPI,                  // shader gpi map for shaders that use a combined map (same id as gloss map just here for readability)
            DRAW_MODE_DEBUG_GLOSSMAP=DRAW_MODE_DEBUG_GPI,   // shader gloss map
            DRAW_MODE_DEBUG_INCANMAP,             // shader incan map
            DRAW_MODE_DEBUG_PARALLAXMAP,          // shader parallax map
            DRAW_MODE_DEBUG_SHADER_TINT,          // shader tint color
            DRAW_MODE_DEBUG_SHADER_DIRTY,         // shader dirty color
            DRAW_MODE_DEBUG_SHADER_ALPHATYPE,     // shader alpha type (green: opaque, yellow: additive, red: blended)

            // These debug modes render various indices as colors, they can be visually rendered or they can be used with the GetIndex() function to query the rendered
            // index. If you are using these index debug modes in query mode for things like drag and drop you should switch off the wireframe rendering and the reference grid
            DRAW_MODE_DEBUG_FRAGMENT_INDEX,             
        };

        // Only create a single renderer
        class CORE_API Renderer : public DeviceManager
        {
        public:
            Renderer();
            ~Renderer();

            bool Init(HWND hwnd, u32 width, u32 height, u32 flags);
            u32 GetPixel(u32 x, u32 y);
            u32 GetIndex(u32 x, u32 y); // used with the indexed debug modes to return the index at the given pixel

        private:
            void DrawEnvironment(RenderScene* scene,RenderEnvironment* env,D3DXMATRIX& viewproj);
            void DrawMesh(RenderScene* scene,RenderMesh* mesh);
            void DrawWireframe(RenderScene* scene,RenderMesh* mesh);
            void DrawNormals(RenderScene* scene,RenderMesh* mesh);
            void DrawTangents(RenderScene* scene,RenderMesh* mesh);
            void DrawDebug(RenderScene* scene,RenderMesh* mesh);
            void DrawReferenceGrid(RenderScene* scene);

        public:
            void DrawScene(RenderScene* scene);
            void DrawScenes(int num, RenderScene** scenes);

            // the master shader database
            ShaderManager m_shader_manager;
        };
    }
}