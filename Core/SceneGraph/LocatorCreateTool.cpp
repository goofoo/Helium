/*#include "Precompile.h"*/
#include "LocatorCreateTool.h"

#include "Core/SceneGraph/Mesh.h"
#include "Core/SceneGraph/Scene.h"
#include "Locator.h"
#include "LocatorType.h"

#include "Core/SceneGraph/Pick.h"

using namespace Helium;
using namespace Helium::SceneGraph;

LocatorShape LocatorCreateTool::s_Shape = LocatorShapes::Cross;

REFLECT_DEFINE_ABSTRACT(LocatorCreateTool);

void LocatorCreateTool::InitializeType()
{
    Reflect::RegisterClassType< LocatorCreateTool >( TXT( "LocatorCreateTool" ) );
}

void LocatorCreateTool::CleanupType()
{
    Reflect::UnregisterClassType< LocatorCreateTool >();
}

LocatorCreateTool::LocatorCreateTool(Scene* scene, PropertiesGenerator* generator)
: CreateTool (scene, generator)
{

}

LocatorCreateTool::~LocatorCreateTool()
{

}

TransformPtr LocatorCreateTool::CreateNode()
{
#ifdef SCENE_DEBUG_RUNTIME_DATA_SELECTION

    LocatorPtr v = new Locator( s_Shape );

    v->RectifyRuntimeData();

    LLocatorPtr locator = new Locator( m_Scene, v );

    m_Scene->AddObject( locator );

    {
        OS_SceneNodeDumbPtr selection;
        selection.push_back( locator );
        m_Scene->GetSelection().SetItems( selection );

        m_Scene->GetSelection().Clear();
    }

    m_Scene->RemoveObject( locator );

    return locator;

#else

    TransformPtr node = new Locator ();
    node->SetOwner( m_Scene );
    return node;

#endif
}

void LocatorCreateTool::CreateProperties()
{
    m_Generator->PushContainer( TXT( "Locator" ) );
    {
        m_Generator->PushContainer();
        {
            m_Generator->AddLabel( TXT( "Shape" ) );

            Inspect::Choice* choice = m_Generator->AddChoice<int>( new Helium::MemberProperty<LocatorCreateTool, int>(this, &LocatorCreateTool::GetLocatorShape, &LocatorCreateTool::SetLocatorShape) );
            choice->a_IsDropDown.Set( true );
            std::vector< Inspect::ChoiceItem > items;

            {
                tostringstream str;
                str << LocatorShapes::Cross;
                items.push_back( Inspect::ChoiceItem( TXT( "Cross" ), str.str() ) );
            }

            {
                tostringstream str;
                str << LocatorShapes::Cube;
                items.push_back( Inspect::ChoiceItem( TXT( "Cube" ), str.str() ) );
            }

            choice->a_Items.Set( items );
        }
        m_Generator->Pop();
    }
    m_Generator->Pop();

    __super::CreateProperties();
}

int LocatorCreateTool::GetLocatorShape() const
{
    return s_Shape;
}

void LocatorCreateTool::SetLocatorShape(int value)
{
    s_Shape = static_cast< LocatorShape > (value);

    Place(Math::Matrix4::Identity);
}
