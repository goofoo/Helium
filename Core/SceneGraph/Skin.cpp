/*#include "Precompile.h"*/
#include "Skin.h"

#include "Core/SceneGraph/Scene.h"
#include "Core/SceneGraph/Transform.h"
#include "Core/SceneGraph/Mesh.h"

using namespace Helium;
using namespace Helium::SceneGraph;

REFLECT_DEFINE_CLASS( Influence );

void Influence::EnumerateClass( Reflect::Compositor<Influence>& comp )
{
    comp.AddField( &Influence::m_Objects,           "m_Objects" );
    comp.AddField( &Influence::m_Weights,           "m_Weights" );
}

REFLECT_DEFINE_CLASS( Skin );

void Skin::EnumerateClass( Reflect::Compositor<Skin>& comp )
{
    comp.AddField( &Skin::m_MeshID,                 "m_MeshID" );
    comp.AddField( &Skin::m_Influences,             "m_Influences" );
    comp.AddField( &Skin::m_InfluenceObjectIDs,     "m_InfluenceObjectIDs" );
    comp.AddField( &Skin::m_InfluenceIndices,       "m_InfluenceIndices" );
}

void Skin::InitializeType()
{
    Reflect::RegisterClassType< Influence >();
    Reflect::RegisterClassType< Skin >();
}

void Skin::CleanupType()
{
    Reflect::UnregisterClassType< Influence >();
    Reflect::UnregisterClassType< Skin >();
}

Skin::Skin()
{
}

Skin::~Skin()
{
}

void Skin::Initialize()
{
    __super::Initialize();

    m_Mesh = Reflect::ObjectCast< Mesh > ( m_Owner->FindNode( m_MeshID ) );

    if ( m_Mesh )
    {
        // Make this a dependency of the mesh
        m_Mesh->CreateDependency( this );

        // Dereference influence objects
        V_TUID::const_iterator infItr = m_InfluenceObjectIDs.begin();
        V_TUID::const_iterator infEnd = m_InfluenceObjectIDs.end();
        for ( ; infItr != infEnd; ++infItr )
        {
            SceneNode* obj = m_Owner->FindNode( *infItr );
            Transform* transform = Reflect::ObjectCast< Transform >( obj );
            HELIUM_ASSERT( transform );
            if ( transform )
            {
                m_InfluenceObjects.push_back( transform );
                CreateDependency( transform );
                transform->Dirty();
            }
        }
    }
}

void Skin::Evaluate(GraphDirection direction)
{
    switch (direction)
    {
    case GraphDirections::Downstream:
        {
            if ( m_Mesh == NULL || m_InfluenceObjects.empty() )
            {
                return;
            }


            //
            // Get objects
            //

            const Transform* transform = m_Mesh->GetTransform();


            //
            // We only need to globalize deformation matrices of skins that were not frozen at rig time
            //

            bool fullTransform = transform->GetGlobalTransform() != Math::Matrix4::Identity;


            //
            // Build the deformation matrix for each influence (this is the offset from the bind pose)
            //

            for ( size_t i = 0; i < m_InfluenceObjects.size(); i++ )
            {
                const Transform* influence = m_InfluenceObjects[i];

                // build the current deformation transformation in global space
                Math::Matrix4 deformMat = influence->GetInverseBindTransform() * influence->GetGlobalTransform();

                if ( fullTransform )
                {
                    deformMat = transform->GetGlobalTransform() * deformMat * transform->GetInverseGlobalTransform();
                }

                m_DeformMatrices.push_back( deformMat );
            }


            //
            // Build Skin Matrices
            //

#pragma TODO("This must have been if-zeroed out a long time ago, we need to lookup the polygon data via indices to fix this")
#if 0

            {
                int count = 0;

                V_Polygon::const_iterator itr = m_Mesh->m_Polygons.begin();
                V_Polygon::const_iterator end = m_Mesh->m_Polygons.end();
                for ( u32 i = 0; itr != end; ++itr, ++i )
                {
                    Matrix4 matrix;

                    const PolygonPtr& polygon (*itr);

                    std::vector< u32 >::const_iterator normalItr = polygon->m_Normals.begin();
                    std::vector< u32 >::const_iterator normalEnd = polygon->m_Normals.end();
                    for ( u32 j = 0; normalItr != normalEnd; ++normalItr, ++j )
                    {
                        if ( j > 2 )
                        {
                            BlendMatrix( transform, m_Influences[ polygon->m_Vertices[0] ], matrix );
                            m_SkinMatrices.push_back( matrix );
                            count++;

                            BlendMatrix( transform, m_Influences[ polygon->m_Vertices[j-1] ], matrix );
                            m_SkinMatrices.push_back( matrix );
                            count++;
                        }

                        BlendMatrix( transform, m_Influences[ polygon->m_Vertices[j] ], matrix );
                        m_SkinMatrices.push_back( matrix );
                        count++;
                    }
                }
            }

            //
            // Transform Geometry
            //

            {
                int count = 0;

                V_Polygon::const_iterator itr = m_Mesh->m_Polygons.begin();
                V_Polygon::const_iterator end = m_Mesh->m_Polygons.end();
                for ( u32 i = 0; itr != end; ++itr, ++i )
                {
                    const PolygonPtr& polygon (*itr);

                    std::vector< u32 >::const_iterator vertexItr = polygon->m_Vertices.begin();
                    std::vector< u32 >::const_iterator vertexEnd = polygon->m_Vertices.end();
                    for ( u32 j = 0; vertexItr != vertexEnd; ++vertexItr, ++j )
                    {
                        if ( j > 2 )
                        {
                            Math::Vector3& triVertex1 (m_Mesh->m_TriangleVertices[count]);
                            triVertex1 = m_Mesh->m_Vertices[ *vertexItr ];
                            m_SkinMatrices[polygon->m_Vertices[0]].TransformVertex( triVertex1 );
                            count++;

                            Math::Vector3& triVertex2 (m_Mesh->m_TriangleVertices[count]);
                            triVertex2 = m_Mesh->m_Vertices[ *vertexItr ];
                            m_SkinMatrices[polygon->m_Vertices[j-1]].TransformVertex( triVertex2 );
                            count++;
                        }

                        Math::Vector3& triVertex (m_Mesh->m_TriangleVertices[count]);
                        triVertex = m_Mesh->m_Vertices[ *vertexItr ];
                        m_SkinMatrices[polygon->m_Vertices[j]].TransformVertex( triVertex );
                        count++;
                    }
                }
            }

            {
                int count = 0;

                V_Polygon::const_iterator itr = m_Mesh->m_Polygons.begin();
                V_Polygon::const_iterator end = m_Mesh->m_Polygons.end();
                for ( u32 i = 0; itr != end; ++itr, ++i )
                {
                    const PolygonPtr& polygon (*itr);

                    std::vector< u32 >::const_iterator normalItr = polygon->m_Normals.begin();
                    std::vector< u32 >::const_iterator normalEnd = polygon->m_Normals.end();
                    for ( u32 j = 0; normalItr != normalEnd; ++normalItr, ++j )
                    {
                        if ( j > 2 )
                        {
                            Math::Vector3& triNormal1 (m_Mesh->m_TriangleNormals[count]);
                            triNormal1 = m_Mesh->m_Normals[ *normalItr ];
                            m_SkinMatrices[polygon->m_Vertices[0]].TransformNormal( triNormal1 );
                            count++;

                            Math::Vector3& triNormal2 (m_Mesh->m_TriangleNormals[count]);
                            triNormal2 = m_Mesh->m_Normals[ *normalItr ];
                            m_SkinMatrices[polygon->m_Vertices[j-1]].TransformNormal( triNormal2 );
                            count++;
                        }

                        Math::Vector3& triNormal (m_Mesh->m_TriangleNormals[count]);
                        triNormal = m_Mesh->m_Normals[ *normalItr ];
                        m_SkinMatrices[polygon->m_Vertices[j]].TransformNormal( triNormal );
                        count++;
                    }
                }
            }

#endif
        }
    }

    __super::Evaluate(direction);
}

void Skin::BlendMatrix(const Transform* transform, const Influence* influence, Math::Matrix4& matrix)
{
    //
    // Blend influence deformation matrices together
    //

    matrix = ( m_DeformMatrices[influence->m_Objects[0]] * influence->m_Weights[0] );

    const u32 numInf = static_cast< u32 >( influence->m_Objects.size() );
    for ( u32 j = 1; j < numInf; j++ )
    {
        matrix += ( m_DeformMatrices[influence->m_Objects[j]] * influence->m_Weights[j] );
    }


    //
    // Move vertex to local space
    //

    matrix *= transform->GetInverseGlobalTransform();
}