/*#include "Precompile.h"*/
#include "Core/SceneGraph/Transform.h"

#include "Foundation/Math/EulerAngles.h"
#include "Foundation/Math/Constants.h"

#include "Core/SceneGraph/Manipulator.h"
#include "Foundation/Undo/PropertyCommand.h"
#include "PrimitiveAxes.h"

#include "Core/SceneGraph/Scene.h"
#include "HierarchyNodeType.h"

#include "Color.h"

using namespace Helium;
using namespace Helium::Math;
using namespace Helium::SceneGraph;

REFLECT_DEFINE_ABSTRACT( SceneGraph::Transform );

struct ScaleColorInfo
{
    D3DCOLORVALUE m_StartColor;
    D3DCOLORVALUE m_EndColor;
    f32 m_ScaleMin;
    f32 m_ScaleMax;
};

void Transform::EnumerateClass( Reflect::Compositor<Transform>& comp )
{
    comp.AddField( (Vector3 Transform::*)&Transform::m_Scale,   "m_Scale" );
    comp.AddField( (Vector3 Transform::*)&Transform::m_Rotate,  "m_Rotate" );
    comp.AddField( &Transform::m_Translate,                     "m_Translate" );

    comp.AddField( &Transform::m_ObjectTransform,               "m_ObjectTransform" );
    comp.AddField( &Transform::m_GlobalTransform,               "m_GlobalTransform" );
    comp.AddField( &Transform::m_InheritTransform,              "m_InheritTransform" );
}

void Transform::InitializeType()
{
    Reflect::RegisterClassType< SceneGraph::Transform >( TXT( "SceneGraph::Transform" ) );

    PropertiesGenerator::InitializePanel( TXT( "Transform" ), CreatePanelSignature::Delegate( &Transform::CreatePanel ));
}

void Transform::CleanupType()
{
    Reflect::UnregisterClassType< SceneGraph::Transform >();
}

Transform::Transform()
: m_InheritTransform( true )
, m_BindIsDirty( true )
{

}

Transform::~Transform()
{

}

void Transform::Initialize()
{
    __super::Initialize();

    SceneGraph::PrimitiveAxes* axes = static_cast< SceneGraph::PrimitiveAxes* >( m_Owner->GetViewport()->GetGlobalPrimitive( GlobalPrimitives::TransformAxes ) );
    m_ObjectBounds.minimum = Math::Vector3(-axes->m_Length, -axes->m_Length, -axes->m_Length);
    m_ObjectBounds.maximum = Math::Vector3(axes->m_Length, axes->m_Length, axes->m_Length);
}

SceneGraph::Transform* Transform::GetTransform()
{
    return this;
}

const SceneGraph::Transform* Transform::GetTransform() const
{
    return this;
}

Math::Scale Transform::GetScale() const
{
    return m_Scale;
}

void Transform::SetScale( const Math::Scale& value )
{
    m_Scale = value;

    Dirty();
}

Math::Vector3 Transform::GetScalePivot() const 
{
    return Math::Vector3::Zero;
}

void Transform::SetScalePivot( const Math::Vector3& value )
{

}

Math::EulerAngles Transform::GetRotate() const
{
    return m_Rotate;
}

void Transform::SetRotate( const Math::EulerAngles& value )
{
    m_Rotate = value;

    Dirty();
}

Math::Vector3 Transform::GetRotatePivot() const
{
    return Math::Vector3::Zero;
}

void Transform::SetRotatePivot( const Math::Vector3& value )
{

}

Math::Vector3 Transform::GetTranslate() const
{
    return m_Translate;
}

void Transform::SetTranslate( const Math::Vector3& value )
{
    m_Translate = value;

    Dirty();
}

Math::Vector3 Transform::GetTranslatePivot() const
{
    return Math::Vector3::Zero;
}

void Transform::SetTranslatePivot( const Math::Vector3& value )
{

}

void Transform::SetObjectTransform( const Math::Matrix4& transform )
{
    Math::Scale scale;
    Math::EulerAngles rotate;
    Math::Vector3 translate;
    transform.Decompose( scale, rotate, translate );

    m_Scale = scale;
    m_Rotate = rotate;
    m_Translate = translate;
    m_ObjectTransform = transform;
}

void Transform::SetGlobalTransform( const Math::Matrix4& transform )
{
    m_GlobalTransform = transform;

    ComputeObjectComponents();
}

Math::Matrix4 Transform::GetBindTransform() const
{
    return m_BindTransform;
}

Math::Matrix4 Transform::GetInverseBindTransform() const
{
    return m_InverseBindTransform;
}

bool Transform::GetInheritTransform() const
{
    return m_InheritTransform;
}

void Transform::SetInheritTransform(bool inherit)
{
    m_InheritTransform = inherit;

    ComputeObjectComponents();
}

Math::Matrix4 Transform::GetScaleComponent() const
{
    return Math::Matrix4 (m_Scale);
}

Math::Matrix4 Transform::GetRotateComponent() const
{
    return Math::Matrix4 (m_Rotate);
}

Math::Matrix4 Transform::GetTranslateComponent() const
{
    return Math::Matrix4 (m_Translate);
}

Undo::CommandPtr Transform::ResetTransform()
{
    Undo::CommandPtr command = new Undo::PropertyCommand<Math::Matrix4>( new Helium::MemberProperty<SceneGraph::Transform, Math::Matrix4> (this, &Transform::GetObjectTransform, &Transform::SetObjectTransform) );

    m_Scale = Scale::Identity;
    m_Rotate = EulerAngles::Zero;
    m_Translate = Vector3::Zero;

    Dirty();

    return command;
}

Undo::CommandPtr Transform::ComputeObjectComponents()
{
    Undo::CommandPtr command = ResetTransform();

    Math::Scale scale;
    Math::EulerAngles rotate;
    Math::Vector3 translate;
    if (GetInheritTransform())
    {
        // compute the new object space transform from our global transform and our parent's inverse
        Matrix4 objectTransform = (m_GlobalTransform * m_Parent->GetTransform()->GetInverseGlobalTransform());

        // decompose into the new set of components
        objectTransform.Decompose(scale, rotate, translate);
    }
    else
    {
        // we are not inheriting our parent's transform, use the global transform for our components
        m_GlobalTransform.Decompose(scale, rotate, translate);
    }

    m_Scale = scale;
    m_Rotate = rotate;
    m_Translate = translate;

    return command;
}

Undo::CommandPtr Transform::CenterTransform()
{
    // copy global matrix
    Matrix4 transform = m_GlobalTransform;

    // reset the transform state
    Undo::CommandPtr command = ResetTransform();

    // set the global matrix (will re-localize)
    SetGlobalTransform(transform);

    return command;
}

void Transform::Evaluate(GraphDirection direction)
{
    CORE_EVALUATE_SCOPE_TIMER( ("") );

    switch (direction)
    {
    case GraphDirections::Downstream:
        {
            {
                CORE_EVALUATE_SCOPE_TIMER( ("Compose Local Matrices") );

                //
                // Compute Local Transform
                //

                m_ObjectTransform = GetScaleComponent() * GetRotateComponent() * GetTranslateComponent();
            }


            {
                CORE_EVALUATE_SCOPE_TIMER( ("Compute Global Matrices") );

                //
                // Compute Global Transform
                //

                if (m_Parent == NULL || !GetInheritTransform())
                {
                    m_GlobalTransform = m_ObjectTransform;
                }
                else
                {
                    m_GlobalTransform = m_ObjectTransform * m_Parent->GetTransform()->GetGlobalTransform();
                }
            }


            {
                CORE_EVALUATE_SCOPE_TIMER( ("Compute Inverse Matrices") );

                //
                // Compute Inverses
                //

                m_InverseObjectTransform = m_ObjectTransform;
                m_InverseObjectTransform.Invert();

                m_InverseGlobalTransform = m_GlobalTransform;
                m_InverseGlobalTransform.Invert();
            }


            //
            // Compute Object and Global Bind Transform, if Dirty
            //

            if (m_BindIsDirty)
            {
                CORE_EVALUATE_SCOPE_TIMER( ("Compute Bind Matrix and Inverse") );

                if (m_Parent == NULL)
                    m_BindTransform = m_ObjectTransform;
                else
                    m_BindTransform = m_ObjectTransform * m_Parent->GetTransform()->GetBindTransform();

                m_InverseBindTransform = m_BindTransform;
                m_InverseBindTransform.Invert();

                m_BindIsDirty = false;
            }

            break;
        }
    }

    __super::Evaluate(direction);
}

void Transform::Render( RenderVisitor* render )
{
#ifdef DRAW_TRANFORMS
    RenderEntry* entry = render->Allocate(this);

    entry->m_Location = render->State().m_Matrix;
    entry->m_Center = m_ObjectBounds.Center();

    if (IsSelected() && m_Scene->IsCurrent())
    {
        entry->m_Draw = &Transform::DrawSelected;
    }
    else
    {
        entry->m_Draw = &Transform::DrawNormal;
    }
#endif

    __super::Render( render );
}

void Transform::DrawNormal( IDirect3DDevice9* device, DrawArgs* args, const SceneNode* object )
{
    const SceneGraph::HierarchyNode* node = Reflect::ConstAssertCast<SceneGraph::HierarchyNode>( object );

    node->GetOwner()->GetViewport()->GetGlobalPrimitive( GlobalPrimitives::TransformAxes )->Draw( args );
}

void Transform::DrawSelected( IDirect3DDevice9* device, DrawArgs* args, const SceneNode* object )
{
    const SceneGraph::HierarchyNode* node = Reflect::ConstAssertCast<SceneGraph::HierarchyNode>( object );

    node->GetOwner()->GetViewport()->GetGlobalPrimitive( GlobalPrimitives::SelectedAxes )->Draw( args );
}

bool Transform::Pick( PickVisitor* pick )
{
#ifdef DRAW_TRANFORMS
    pick->SetCurrentObject (this, pick->State().m_Matrix);

    return m_Scene->GetViewport()->GetGlobalPrimitive( GlobalPrimitives::TransformAxes )->Pick(pick);
#else
    return false;
#endif
}

void Transform::ConnectManipulator(ManiuplatorAdapterCollection* collection)
{
    switch (collection->GetMode())
    {
    case ManipulatorModes::Scale:
        {
            collection->AddManipulatorAdapter(new TransformScaleManipulatorAdapter (this));
            break;
        }

    case ManipulatorModes::ScalePivot:
        {
            collection->AddManipulatorAdapter(new TransformScalePivotManipulatorAdapter (this));
            break;
        }

    case ManipulatorModes::Rotate:
        {
            collection->AddManipulatorAdapter(new TransformRotateManipulatorAdapter (this));
            break;
        }

    case ManipulatorModes::RotatePivot:
        {
            collection->AddManipulatorAdapter(new TransformRotatePivotManipulatorAdapter (this));
            break;
        }

    case ManipulatorModes::Translate:
        {
            collection->AddManipulatorAdapter(new TransformTranslateManipulatorAdapter (this));
            break;
        }

    case ManipulatorModes::TranslatePivot:
        {
            collection->AddManipulatorAdapter(new TransformTranslatePivotManipulatorAdapter (this));
            break;
        }
    }
}

bool Transform::ValidatePanel(const tstring& name)
{
    if (name == TXT( "Transform" ) )
    {
        return true;
    }

    return __super::ValidatePanel(name);
}

void Transform::CreatePanel(CreatePanelArgs& args)
{
    args.m_Generator->PushContainer( TXT( "Transform" ) );

    Inspect::Label* label;
    Inspect::Value* value;
    {
        args.m_Generator->PushContainer();
        label = args.m_Generator->AddLabel( TXT( "Inherit Transform" ) );
        label->a_HelpText.Set( TXT( "Causes this node to inherit its transform from its parent node." ) );
        Inspect::CheckBox* checkBox = args.m_Generator->AddCheckBox<SceneGraph::Transform, bool>(args.m_Selection, &Transform::GetInheritTransform, &Transform::SetInheritTransform);
        checkBox->a_HelpText.Set( TXT( "Causes this node to inherit its transform from its parent node." ) );
        args.m_Generator->Pop();
    }

    {
        args.m_Generator->PushContainer();
        label = args.m_Generator->AddLabel( TXT( "Scale" ) );
        label->a_HelpText.Set( TXT( "Controls the scaling of this node in the x, y and z dimensions." ) );

        value = args.m_Generator->AddValue<SceneGraph::Transform, f32>(args.m_Selection, &Transform::GetScaleX, &Transform::SetScaleX);
        value->a_HelpText.Set( TXT( "Constrols the scaling of this node in the x dimension." ) );

        value = args.m_Generator->AddValue<SceneGraph::Transform, f32>(args.m_Selection, &Transform::GetScaleY, &Transform::SetScaleY);
        value->a_HelpText.Set( TXT( "Constrols the scaling of this node in the y dimension." ) );

        value = args.m_Generator->AddValue<SceneGraph::Transform, f32>(args.m_Selection, &Transform::GetScaleZ, &Transform::SetScaleZ);
        value->a_HelpText.Set( TXT( "Constrols the scaling of this node in the z dimension." ) );

        args.m_Generator->Pop();
    }

    {
        args.m_Generator->PushContainer();
        label = args.m_Generator->AddLabel( TXT( "Rotate" ) );
        label->a_HelpText.Set( TXT( "Controls the rotation of this node about its x, y and z axes." ) );

        value = args.m_Generator->AddValue<SceneGraph::Transform, f32>(args.m_Selection, &Transform::GetRotateX, &Transform::SetRotateX);
        value->a_HelpText.Set( TXT( "Controls the rotation of this node about its x axis." ) );

        value = args.m_Generator->AddValue<SceneGraph::Transform, f32>(args.m_Selection, &Transform::GetRotateY, &Transform::SetRotateY);
        value->a_HelpText.Set( TXT( "Controls the rotation of this node about its y axis." ) );

        value = args.m_Generator->AddValue<SceneGraph::Transform, f32>(args.m_Selection, &Transform::GetRotateZ, &Transform::SetRotateZ);
        value->a_HelpText.Set( TXT( "Controls the rotation of this node about its z axis." ) );

        args.m_Generator->Pop();
    }

    {
        args.m_Generator->PushContainer();
        label = args.m_Generator->AddLabel( TXT( "Translate" ) );
        label->a_HelpText.Set( TXT( "Controls the location of this node in space with respect to the origin." ) );

        value = args.m_Generator->AddValue<SceneGraph::Transform, f32>(args.m_Selection, &Transform::GetTranslateX, &Transform::SetTranslateX);
        value->a_HelpText.Set( TXT( "Controls the offset of this node from the origin along the x axis." ) );

        value = args.m_Generator->AddValue<SceneGraph::Transform, f32>(args.m_Selection, &Transform::GetTranslateY, &Transform::SetTranslateY);
        value->a_HelpText.Set( TXT( "Controls the offset of this node from the origin along the y axis." ) );

        value = args.m_Generator->AddValue<SceneGraph::Transform, f32>(args.m_Selection, &Transform::GetTranslateZ, &Transform::SetTranslateZ);
        value->a_HelpText.Set( TXT( "Controls the offset of this node from the origin along the z axis." ) );

        args.m_Generator->Pop();
    }

    args.m_Generator->Pop();
}

f32 Transform::GetScaleX() const
{
    return GetScale().x;
}

void Transform::SetScaleX(f32 scale)
{
    Math::Scale s = GetScale();
    s.x = scale;
    SetScale(s);
}

f32 Transform::GetScaleY() const
{
    return GetScale().y;
}

void Transform::SetScaleY(f32 scale)
{
    Math::Scale s = GetScale();
    s.y = scale;
    SetScale(s);
}

f32 Transform::GetScaleZ() const
{
    return GetScale().z;
}

void Transform::SetScaleZ(f32 scale)
{
    Math::Scale s = GetScale();
    s.z = scale;
    SetScale(s);
}

f32 Transform::GetRotateX() const
{
    return GetRotate().angles.x * Math::RadToDeg;
}

void Transform::SetRotateX(f32 rotate)
{
    Math::Vector3 s = GetRotate().angles;
    s.x = rotate * Math::DegToRad;
    SetRotate(EulerAngles (s, m_Rotate.order));
}

f32 Transform::GetRotateY() const
{
    return GetRotate().angles.y * Math::RadToDeg;
}

void Transform::SetRotateY(f32 rotate)
{
    Math::Vector3 s = GetRotate().angles;
    s.y = rotate * Math::DegToRad;
    SetRotate(EulerAngles (s, m_Rotate.order));
}

f32 Transform::GetRotateZ() const
{
    return GetRotate().angles.z * Math::RadToDeg;
}

void Transform::SetRotateZ(f32 rotate)
{
    Math::Vector3 s = GetRotate().angles;
    s.z = rotate * Math::DegToRad;
    SetRotate(EulerAngles (s, m_Rotate.order));
}

f32 Transform::GetTranslateX() const
{
    return GetTranslate().x;
}

void Transform::SetTranslateX(f32 translate)
{
    Math::Vector3 s = GetTranslate();
    s.x = translate;
    SetTranslate(s);
}

f32 Transform::GetTranslateY() const
{
    return GetTranslate().y;
}

void Transform::SetTranslateY(f32 translate)
{
    Math::Vector3 s = GetTranslate();
    s.y = translate;
    SetTranslate(s);
}

f32 Transform::GetTranslateZ() const
{
    return GetTranslate().z;
}

void Transform::SetTranslateZ(f32 translate)
{
    Math::Vector3 s = GetTranslate();
    s.z = translate;
    SetTranslate(s);
}

Matrix4 TransformScaleManipulatorAdapter::GetFrame(ManipulatorSpace space)
{
    Matrix4 frame;

    switch (space)
    {
    case ManipulatorSpaces::Object:
        {
            frame = m_Transform->GetGlobalTransform();
            frame.x *= 1.f / m_Transform->GetScale().x;
            frame.y *= 1.f / m_Transform->GetScale().y;
            frame.z *= 1.f / m_Transform->GetScale().z;
            break;
        }

    case ManipulatorSpaces::Local:
        {
            frame = m_Transform->GetParentTransform();
            break;
        }

    case ManipulatorSpaces::World:
        {
            // fall through, world space IS identity
            break;
        }
    }

    Vector3 pos (GetPivot());
    m_Transform->GetTranslateComponent().TransformVertex(pos);
    m_Transform->GetParentTransform().TransformVertex(pos);

    // move the translation to the correction location
    frame.t.x = pos.x;
    frame.t.y = pos.y;
    frame.t.z = pos.z;

    return frame;
}

Math::Matrix4 TransformScaleManipulatorAdapter::GetObjectMatrix()
{
    return m_Transform->GetObjectTransform();
}

Math::Matrix4 TransformScaleManipulatorAdapter::GetParentMatrix()
{
    return m_Transform->GetParentTransform();
}

Matrix4 TransformScalePivotManipulatorAdapter::GetFrame(ManipulatorSpace space)
{
    Matrix4 frame;

    switch (space)
    {
    case ManipulatorSpaces::Object:
        {
            frame = m_Transform->GetGlobalTransform();
            frame.x *= 1.f / m_Transform->GetScale().x;
            frame.y *= 1.f / m_Transform->GetScale().y;
            frame.z *= 1.f / m_Transform->GetScale().z;
            break;
        }

    case ManipulatorSpaces::Local:
        {
            frame = m_Transform->GetParentTransform();
            break;
        }

    case ManipulatorSpaces::World:
        {
            // fall through, world space IS identity
            break;
        }
    }

    Vector3 pos (GetValue());
    m_Transform->GetTranslateComponent().TransformVertex(pos);
    m_Transform->GetParentTransform().TransformVertex(pos);

    // move the translation to the correction location
    frame.t.x = pos.x;
    frame.t.y = pos.y;
    frame.t.z = pos.z;

    return frame;
}

Math::Matrix4 TransformScalePivotManipulatorAdapter::GetObjectMatrix()
{
    return Matrix4::Identity;
}

Math::Matrix4 TransformScalePivotManipulatorAdapter::GetParentMatrix()
{
    return m_Transform->GetTranslateComponent() * m_Transform->GetParentTransform();
}

Matrix4 TransformRotateManipulatorAdapter::GetFrame(ManipulatorSpace space)
{
    Matrix4 frame;

    switch (space)
    {
    case ManipulatorSpaces::Object:
        {
            frame = m_Transform->GetGlobalTransform();
            frame.x *= 1.f / m_Transform->GetScale().x;
            frame.y *= 1.f / m_Transform->GetScale().y;
            frame.z *= 1.f / m_Transform->GetScale().z;
            break;
        }

    case ManipulatorSpaces::Local:
        {
            frame = m_Transform->GetParentTransform();
            break;
        }

    case ManipulatorSpaces::World:
        {
            // fall through, world space IS identity
            break;
        }
    }

    Vector3 pos (GetPivot());
    m_Transform->GetTranslateComponent().TransformVertex(pos);
    m_Transform->GetParentTransform().TransformVertex(pos);

    // move the translation to the correction location
    frame.t.x = pos.x;
    frame.t.y = pos.y;
    frame.t.z = pos.z;

    return frame;
}

Math::Matrix4 TransformRotateManipulatorAdapter::GetObjectMatrix()
{
    return m_Transform->GetObjectTransform();
}

Math::Matrix4 TransformRotateManipulatorAdapter::GetParentMatrix()
{
    return m_Transform->GetParentTransform();
}

Matrix4 TransformRotatePivotManipulatorAdapter::GetFrame(ManipulatorSpace space)
{
    Matrix4 frame;

    switch (space)
    {
    case ManipulatorSpaces::Object:
        {
            frame = m_Transform->GetGlobalTransform();
            frame.x *= 1.f / m_Transform->GetScale().x;
            frame.y *= 1.f / m_Transform->GetScale().y;
            frame.z *= 1.f / m_Transform->GetScale().z;
            break;
        }

    case ManipulatorSpaces::Local:
        {
            frame = m_Transform->GetParentTransform();
            break;
        }

    case ManipulatorSpaces::World:
        {
            // fall through, world space IS identity
            break;
        }
    }

    Vector3 pos (GetValue());
    m_Transform->GetTranslateComponent().TransformVertex(pos);
    m_Transform->GetParentTransform().TransformVertex(pos);

    // move the translation to the correction location
    frame.t.x = pos.x;
    frame.t.y = pos.y;
    frame.t.z = pos.z;

    return frame;
}

Math::Matrix4 TransformRotatePivotManipulatorAdapter::GetObjectMatrix()
{
    return Matrix4::Identity;
}

Math::Matrix4 TransformRotatePivotManipulatorAdapter::GetParentMatrix()
{
    return m_Transform->GetTranslateComponent() * m_Transform->GetParentTransform();
}

Matrix4 TransformTranslateManipulatorAdapter::GetFrame(ManipulatorSpace space)
{
    Matrix4 frame;

    switch (space)
    {
    case ManipulatorSpaces::Object:
        {
            frame = m_Transform->GetGlobalTransform();
            frame.x *= 1.f / m_Transform->GetScale().x;
            frame.y *= 1.f / m_Transform->GetScale().y;
            frame.z *= 1.f / m_Transform->GetScale().z;
            break;
        }

    case ManipulatorSpaces::Local:
        {
            frame = m_Transform->GetParentTransform();
            break;
        }

    case ManipulatorSpaces::World:
        {
            // fall through, world space IS identity
            break;
        }
    }

    Vector3 pos (GetPivot());
    m_Transform->GetTranslateComponent().TransformVertex(pos);
    m_Transform->GetParentTransform().TransformVertex(pos);

    // move the translation to the correction location
    frame.t.x = pos.x;
    frame.t.y = pos.y;
    frame.t.z = pos.z;

    return frame;
}

Math::Matrix4 TransformTranslateManipulatorAdapter::GetObjectMatrix()
{
    return m_Transform->GetObjectTransform();
}

Math::Matrix4 TransformTranslateManipulatorAdapter::GetParentMatrix()
{
    return m_Transform->GetParentTransform();
}

Matrix4 TransformTranslatePivotManipulatorAdapter::GetFrame(ManipulatorSpace space)
{
    Matrix4 frame;

    switch (space)
    {
    case ManipulatorSpaces::Object:
        {
            frame = m_Transform->GetGlobalTransform();
            frame.x *= 1.f / m_Transform->GetScale().x;
            frame.y *= 1.f / m_Transform->GetScale().y;
            frame.z *= 1.f / m_Transform->GetScale().z;
            break;
        }

    case ManipulatorSpaces::Local:
        {
            frame = m_Transform->GetParentTransform();
            break;
        }

    case ManipulatorSpaces::World:
        {
            // fall through, world space IS identity
            break;
        }
    }

    Vector3 pos (GetValue());
    m_Transform->GetTranslateComponent().TransformVertex(pos);
    m_Transform->GetParentTransform().TransformVertex(pos);

    // move the translation to the correction location
    frame.t.x = pos.x;
    frame.t.y = pos.y;
    frame.t.z = pos.z;

    return frame;
}

Math::Matrix4 TransformTranslatePivotManipulatorAdapter::GetObjectMatrix()
{
    return Matrix4::Identity;
}

Math::Matrix4 TransformTranslatePivotManipulatorAdapter::GetParentMatrix()
{
    return m_Transform->GetTranslateComponent() * m_Transform->GetParentTransform();
}