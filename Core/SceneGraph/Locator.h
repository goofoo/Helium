#pragma once

#include "Instance.h"

namespace Helium
{
    namespace SceneGraph
    {
        class LocatorType;

        namespace LocatorShapes
        {
            enum LocatorShape
            {
                Cross,
                Cube,
            };
            static void LocatorShapeEnumerateEnum( Reflect::Enumeration* info )
            {
                info->AddElement(Cross, TXT( "Cross" ) );
                info->AddElement(Cube, TXT( "Cube" ) );
            }
        }
        typedef LocatorShapes::LocatorShape LocatorShape;

        class Locator : public Instance
        {
        public:
            REFLECT_DECLARE_CLASS( Locator, Instance );
            static void EnumerateClass( Reflect::Compositor<Locator>& comp );
            static void InitializeType();
            static void CleanupType();

            Locator();
            ~Locator();

            virtual i32 GetImageIndex() const HELIUM_OVERRIDE;
            virtual tstring GetApplicationTypeName() const HELIUM_OVERRIDE;
            virtual SceneNodeTypePtr CreateNodeType( Scene* scene ) const HELIUM_OVERRIDE;

            int GetShape() const;
            void SetShape( int shape );

            virtual void Evaluate(GraphDirection direction) HELIUM_OVERRIDE;

            virtual void Render( RenderVisitor* render ) HELIUM_OVERRIDE;
            static void DrawShape( IDirect3DDevice9* device, DrawArgs* args, const SceneNode* object );

            virtual bool Pick( PickVisitor* pick ) HELIUM_OVERRIDE;

            virtual bool ValidatePanel(const tstring& name) HELIUM_OVERRIDE;

            static void CreatePanel( CreatePanelArgs& args );

        protected:
            LocatorShape m_Shape;
        };

        typedef Helium::SmartPtr<Locator> LLocatorPtr;
    }
}