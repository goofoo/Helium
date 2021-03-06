#pragma once

#include "Foundation/API.h"
#include "Foundation/Inspect/Control.h"

namespace Helium
{
    namespace Inspect
    {
        class FOUNDATION_API CheckBox : public Reflect::ConcreteInheritor< CheckBox, Control >
        {
        public:
            CheckBox();

        protected:
            virtual void SetDefaultAppearance( bool def ) HELIUM_OVERRIDE;
            void SetToDefault( const ContextMenuEventArgs& event );

        public:
            Attribute< bool >   a_Highlight;
        };

        typedef Helium::SmartPtr< CheckBox > CheckBoxPtr;
    }
}