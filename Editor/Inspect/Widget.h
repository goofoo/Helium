#pragma once

#include "Foundation/Inspect/InspectControls.h"

namespace Helium
{
    namespace Editor
    {
        class Widget : public Inspect::Widget
        {
        public:
            Widget( Inspect::Control* control );
            ~Widget();

            wxWindow* GetWindow()
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
            virtual void Unrealized( Inspect::Control* control );
            virtual void IsEnabledChanged( const Attribute<bool>::ChangeArgs& args );
            virtual void IsReadOnlyChanged( const Attribute<bool>::ChangeArgs& args );
            virtual void IsFrozenChanged( const Attribute<bool>::ChangeArgs& args );
            virtual void IsHiddenChanged( const Attribute<bool>::ChangeArgs& args );
            virtual void ForegroundColorChanged( const Attribute<u32>::ChangeArgs& args );
            virtual void BackgroundColorChanged( const Attribute<u32>::ChangeArgs& args );
            virtual void ToolTipChanged( const Attribute<tstring>::ChangeArgs& args );

        protected:
            wxWindow*           m_Window;
        };

        typedef SmartPtr<Widget> WidgetPtr;
    }
}