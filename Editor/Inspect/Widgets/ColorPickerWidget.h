#pragma once

#include "Editor/Inspect/Widget.h"

#include "Editor/ColorPicker.h"
#include "Editor/SimpleConfig.h"

namespace Helium
{
    namespace Editor
    {
        class ColorPickerWidget;

        class ColorPickerWindow : public wxPanel
        {
        public:
            ColorPickerWindow( wxWindow* parent, ColorPickerWidget* colorPickerWidget );

            void SetOverride( bool override )
            {
                m_Override = override;
            }

            void SetColor( const Math::Color4& color )
            {
                HELIUM_ASSERT( m_ColorPicker );
                m_ColorPicker->SetColour( wxColor( color.r, color.g, color.b, color.a ) );
            }

            void SetColor( const Math::Color3& color )
            {
                HELIUM_ASSERT( m_ColorPicker );
                m_ColorPicker->SetColour( wxColor( color.r, color.g, color.b ) );
            }

            void GetColor( Math::Color4& color )
            {
                HELIUM_ASSERT( m_ColorPicker );
                wxColour value = m_ColorPicker->GetColour();
                color.r = value.Red();
                color.g = value.Green();
                color.b = value.Blue();
                color.a = value.Alpha();
            }

            void GetColor( Math::Color3 color )
            {
                HELIUM_ASSERT( m_ColorPicker );
                wxColour value = m_ColorPicker->GetColour();
                color.r = value.Red();
                color.g = value.Green();
                color.b = value.Blue();
            }

            // Callback when the color is changed
            void OnChanged( wxCommandEvent& );

        private:
            ColorPicker*            m_ColorPicker;
            ColorPickerWidget*      m_ColorPickerWidget;
            bool                    m_Override;
        };

        class ColorPickerWidget : public Reflect::ConcreteInheritor< ColorPickerWidget, Widget >
        {
        public:
            ColorPickerWidget()
                : m_ColorPickerControl( NULL )
                , m_ColorPickerWindow( NULL )
            {

            }

            ColorPickerWidget( Inspect::ColorPicker* colorPicker );

            virtual void Create( wxWindow* parent ) HELIUM_OVERRIDE;
            virtual void Destroy() HELIUM_OVERRIDE;

            // Inspect::Widget
            virtual void Read() HELIUM_OVERRIDE;
            virtual bool Write() HELIUM_OVERRIDE;

            // Editor::Widget
            virtual void IsReadOnlyChanged( const Attribute<bool>::ChangeArgs& args ) HELIUM_OVERRIDE;
            
            // Listeners
            void HighlightChanged( const Attribute< bool >::ChangeArgs& args );

        protected:
            Inspect::ColorPicker* m_ColorPickerControl;
            ColorPickerWindow*    m_ColorPickerWindow;
        };

    }
}