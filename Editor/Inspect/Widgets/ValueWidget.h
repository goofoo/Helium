#pragma once

#include "Editor/Inspect/Widget.h"

namespace Helium
{
    namespace Editor
    {
        class ValueWidget;

        class ValueWindow : public wxTextCtrl
        {
        public:
            ValueWindow( wxWindow* parent, ValueWidget* valueWidget );

            void SetOverride( bool override )
            {
                m_Override = override;
            }

            void OnConfirm(wxCommandEvent& e);
            void OnSetFocus(wxFocusEvent& e);
            void OnKillFocus(wxFocusEvent& e);
            void OnKeyDown(wxKeyEvent& e);

            DECLARE_EVENT_TABLE();

        private:
            ValueWidget*    m_ValueWidget;
            bool            m_Override;
        };

        class ValueWidget : public Reflect::ConcreteInheritor< ValueWidget, Widget >
        {
        public:
            ValueWidget()
                : m_ValueControl( NULL )
                , m_ValueWindow( NULL )
            {

            }

            ValueWidget( Inspect::Value* value );

            virtual void Create( wxWindow* parent ) HELIUM_OVERRIDE;
            virtual void Destroy() HELIUM_OVERRIDE;

            // Inspect::Widget
            virtual void Read() HELIUM_OVERRIDE;
            virtual bool Write() HELIUM_OVERRIDE;

            // Editor::Widget
            virtual void IsReadOnlyChanged( const Attribute<bool>::ChangeArgs& args ) HELIUM_OVERRIDE;

            // Listeners
            void JustificationChanged( const Attribute<Inspect::Justification>::ChangeArgs& args );
            void HighlightChanged( const Attribute<bool>::ChangeArgs& args );

        protected:
            Inspect::Value*     m_ValueControl;
            ValueWindow*        m_ValueWindow;
        };
    }
}