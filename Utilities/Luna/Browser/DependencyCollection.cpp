#include "Precompile.h"
#include "DependencyCollection.h"

#include "Browser.h"

using namespace Luna;

///////////////////////////////////////////////////////////////////////////////
/// class DependencyCollection
///////////////////////////////////////////////////////////////////////////////
REFLECT_DEFINE_CLASS( DependencyCollection )
void DependencyCollection::EnumerateClass( Reflect::Compositor<DependencyCollection>& comp )
{
    Reflect::Field* fieldRootPath = comp.AddField( &DependencyCollection::m_RootPath, "m_RootPath" );
    Reflect::Field* fieldIsReverse = comp.AddField( &DependencyCollection::m_IsReverse, "m_IsReverse" );
}

/////////////////////////////////////////////////////////////////////////////
DependencyCollection::DependencyCollection()
: AssetCollection( "", AssetCollectionFlags::Dynamic )
, m_IsReverse( false )
, m_AssetFile( NULL )
, m_IsLoading( false )
, m_DependencyLoader( NULL )
{
}

/////////////////////////////////////////////////////////////////////////////
DependencyCollection::DependencyCollection( const std::string& name, const u32 flags, const bool reverse )
: AssetCollection( name, flags | AssetCollectionFlags::Dynamic )
, m_IsReverse( reverse )
, m_AssetFile( NULL )
, m_IsLoading( false )
, m_DependencyLoader( NULL )
{
}

/////////////////////////////////////////////////////////////////////////////
DependencyCollection::~DependencyCollection()
{
}

///////////////////////////////////////////////////////////////////////////////
void DependencyCollection::InitializeCollection() 
{
    __super::InitializeCollection();
#pragma TODO( "reimplemnent without GlobalBrowser" )
    //GlobalBrowser().GetBrowserPreferences()->AddChangedListener( Reflect::ElementChangeSignature::Delegate( this, &DependencyCollection::OnPreferencesChanged ) );
}

///////////////////////////////////////////////////////////////////////////////
void DependencyCollection::CleanupCollection() 
{
#pragma TODO( "reimplemnent without GlobalBrowser" )
    //    GlobalBrowser().GetBrowserPreferences()->RemoveChangedListener( Reflect::ElementChangeSignature::Delegate( this, &DependencyCollection::OnPreferencesChanged ) );
    __super::CleanupCollection();
}

///////////////////////////////////////////////////////////////////////////////
void DependencyCollection::PreDeserialize()
{
    IsLoading( true );

    __super::PreDeserialize();  
}

///////////////////////////////////////////////////////////////////////////////
void DependencyCollection::PostDeserialize()
{
    __super::PostDeserialize();

    m_AssetFile = new Asset::AssetFile( m_RootPath );

    IsLoading( false );
}

///////////////////////////////////////////////////////////////////////////////
std::string DependencyCollection::GetDisplayName() const
{
    std::stringstream stream;
    stream << GetName();
    stream << " (" ;
    if ( IsLoading() )
    {
        stream << "Loading...";
    }
    else
    {
        stream << GetAssetPaths().size() << " " << ( ( GetAssetPaths().size() == 1 ) ? "item" : "items" );
    }
    stream << ")";

    return stream.str();
}

///////////////////////////////////////////////////////////////////////////////
void DependencyCollection::SetRoot( const Nocturnal::Path& path )
{
    m_RootPath = path;
    m_AssetFile = new Asset::AssetFile( m_RootPath );

    DirtyField( GetClass()->FindField( &DependencyCollection::m_Path ) );

    ClearAssets();
}

/////////////////////////////////////////////////////////////////////////////
std::string DependencyCollection::GetAssetName() const
{
    std::string assetName;
    if ( m_AssetFile )
    {
        assetName = m_AssetFile->GetPath().Filename();
    }

    return assetName;
}

/////////////////////////////////////////////////////////////////////////////
u32 DependencyCollection::GetRecursionDepthForLoad() const
{
    u32 maxRecursionDepth = 0;
#pragma TODO( "reimplemnent without GlobalBrowser" )
    //if ( IsReverse() )
    //{
    //    maxRecursionDepth = GlobalBrowser().GetBrowserPreferences()->GetUsageCollectionRecursionDepth();
    //}
    //else
    //{
    //    maxRecursionDepth = GlobalBrowser().GetBrowserPreferences()->GetDependencyCollectionRecursionDepth();
    //}
    return maxRecursionDepth;
}

/////////////////////////////////////////////////////////////////////////////
void DependencyCollection::LoadDependencies( bool threaded )
{
    if ( threaded )
    {
        Freeze();
        IsLoading( true );
#pragma TODO( "reimplemnent without GlobalBrowser" )
        //        m_DependencyLoader->StartThread();
        // It will be thawed and m_IsLoading set to false in the DependencyLoader::OnEndThread callback
    }
    else
    {
        Freeze();
        IsLoading( true );
        {
            Nocturnal::S_Path assets;
#pragma TODO( "reimplemnent without GlobalBrowser" )
            //            GlobalBrowser().GetCacheDB()->GetAssetDependencies( m_spFileReference, assets, m_IsReverse, GetRecursionDepthForLoad() );
            SetAssetReferences( assets );
        }
        IsLoading( false );
        Thaw();
    }
}

/////////////////////////////////////////////////////////////////////////////
void DependencyCollection::IsLoading( bool isLoading )
{
    m_IsLoading = isLoading;
    DirtyField( GetClass()->FindField( &DependencyCollection::m_IsLoading ) );
}

/////////////////////////////////////////////////////////////////////////////
void DependencyCollection::OnPreferencesChanged( const Reflect::ElementChangeArgs& args )
{
    LoadDependencies();
}

/////////////////////////////////////////////////////////////////////////////
void DependencyCollection::SetFlags( const u32 flags )
{
    __super::SetFlags( flags | AssetCollectionFlags::Dynamic );
}

/////////////////////////////////////////////////////////////////////////////
void DependencyCollection::SetReverse( const bool reverse )
{
    m_IsReverse = reverse;
    DirtyField( GetClass()->FindField( &DependencyCollection::m_IsReverse ) );
}