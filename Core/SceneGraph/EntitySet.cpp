/*#include "Precompile.h"*/
#include "EntitySet.h"

#include "Foundation/Log.h"
#include "Foundation/Component/ComponentHandle.h"
#include "Core/Asset/Classes/Entity.h"
#include "Core/Asset/Components/BoundingBoxComponent.h"

#include "Core/SceneGraph/Scene.h"
#include "Core/SceneGraph/EntityInstance.h"
#include "Core/SceneGraph/EntityInstanceType.h"
#include "Core/SceneGraph/PrimitiveCube.h"
#include "Core/SceneGraph/PrimitiveSphere.h"
#include "Core/SceneGraph/PrimitiveCylinder.h"
#include "Core/SceneGraph/PrimitiveCapsule.h"

using namespace Helium;
using namespace Helium::SceneGraph;

REFLECT_DEFINE_ABSTRACT(SceneGraph::EntitySet);

void EntitySet::InitializeType()
{
    Reflect::RegisterClassType< SceneGraph::EntitySet >( TXT( "SceneGraph::EntitySet" ) );
}

void EntitySet::CleanupType()
{
    Reflect::UnregisterClassType< SceneGraph::EntitySet >();
}

EntitySet::EntitySet( SceneGraph::EntityInstanceType* type, const Helium::Path& assetPath )
: SceneGraph::InstanceSet (type)
, m_AssetPath( assetPath )
, m_ClassMissing (false)
, m_Shape (NULL)
{
    LoadAssetClass();
}

EntitySet::~EntitySet()
{
    delete m_Shape;
    delete m_AssetPath;
}

void EntitySet::LoadAssetClass()
{

    m_Class = Asset::AssetClass::LoadAssetClass<Asset::Entity>( m_AssetPath );

    if ( !m_Class.ReferencesObject() )
    {
        m_Name = m_AssetPath.Basename();
    }
    else
    {
        m_Name = m_Class->GetFullName();

        m_ArtFile = m_Class->GetPath().Get();

        if (!m_ArtFile.empty())
        {
            SceneGraph::PrimitiveCube* cube;
            if ( !m_Shape )
            {
                cube = new SceneGraph::PrimitiveCube (m_Type->GetScene()->GetViewport()->GetResources());
                cube->Update();

                m_Shape = cube;
            }
            else
            {
                cube = dynamic_cast< SceneGraph::PrimitiveCube* >( m_Shape );
            }

            Asset::BoundingBoxComponentPtr boundingBox = m_Class->GetComponent< Asset::BoundingBoxComponent >();
            if ( boundingBox.ReferencesObject() )
            {
                if ( boundingBox->GetMinima() != Math::Vector3::Zero || boundingBox->GetMaxima() != Math::Vector3::Zero )
                {
                    cube->SetBounds( boundingBox->GetMinima(), boundingBox->GetMaxima() );
                    cube->Update();
                }
                else if ( boundingBox->GetExtents() != Math::Vector3::Zero )
                {
                    Math::Vector3 minima = -(boundingBox->GetExtents() / 2.f) + boundingBox->GetOffset();
                    minima.y += boundingBox->GetExtents().y / 2.f;
                    Math::Vector3 maxima = (boundingBox->GetExtents() / 2.f) + boundingBox->GetOffset();
                    maxima.y += boundingBox->GetExtents().y / 2.f;
                    cube->SetBounds( minima, maxima );
                    cube->Update();
                }
            }
            else if ( Helium::Path( m_ArtFile ).Exists() )
            {
                try
                {
                    m_Manifest = Reflect::Archive::FromFile<Asset::EntityManifest>( m_ArtFile );
                }
                catch ( const Reflect::Exception& e )
                {
                    Log::Error( TXT( "Error loading %s (%s)\n" ), m_ArtFile.c_str(), e.What());
                }

                if (m_Manifest.ReferencesObject())
                {
                    cube->SetBounds( m_Manifest->m_BoundingBoxMin, m_Manifest->m_BoundingBoxMax );
                    cube->Update();
                }
            }
        }
    }

    m_ClassLoaded.Raise( EntitySetChangeArgs( this ) );
}

void EntitySet::Create()
{
    if (m_Shape)
    {
        m_Shape->Create();
    }
}

void EntitySet::Delete()
{
    if (m_Shape)
    {
        m_Shape->Delete();
    }
}

void EntitySet::AddInstance(SceneGraph::Instance* i)
{
    // set class link (must be done before calling base class)
    Reflect::AssertCast<SceneGraph::EntityInstance>(i)->SetClassSet(this);

    __super::AddInstance(i);
}

void EntitySet::RemoveInstance(SceneGraph::Instance* i)
{
    __super::RemoveInstance(i);

    // remove class link (must be done after calling base class)
    Reflect::AssertCast<SceneGraph::EntityInstance>(i)->SetClassSet(NULL);
}
