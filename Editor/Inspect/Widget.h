#pragma once

#include "Foundation/Inspect/Controls.h"

namespace Helium
{
    namespace Editor
    {
        class Widget : public Reflect::AbstractInheritor< Widget, Inspect::Widget >, public wxEvtHandler
        {
        public:
            Widget()
                : m_Window( NULL )
            {

            }

            Widget( Inspect::Control* control );
            ~Widget();

            virtual wxWindow* GetWindow()
            {
                return m_Window;
            }

            int GetStringWidth(const tstring& str);
            bool EllipsizeString(tstring& str, int width);

            virtual void Create( wxWindow* parent ) = 0;
            virtual void Destroy() = 0;

            // initialize state
            virtual void SetWindow( wxWindow* window );

            // callbacks from m_Control
            virtual void IsEnabledChanged( const Attribute<bool>::ChangeArgs& args );
            virtual void IsReadOnlyChanged( const Attribute<bool>::ChangeArgs& args );
            virtual void IsFrozenChanged( const Attribute<bool>::ChangeArgs& args );
            virtual void IsHiddenChanged( const Attribute<bool>::ChangeArgs& args );
            virtual void ForegroundColorChanged( const Attribute<u32>::ChangeArgs& args );
            virtual void BackgroundColorChanged( const Attribute<u32>::ChangeArgs& args );
            virtual void HelpTextChanged( const Attribute<tstring>::ChangeArgs& args );

            void OnContextMenu( wxContextMenuEvent& event );
            void OnContextMenuItem( wxCommandEvent& event );

        protected:
            wxWindow*           m_Window;
        };

        typedef SmartPtr<Widget> WidgetPtr;
    }
}