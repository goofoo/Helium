#pragma once

#include <hash_map>

#include "Foundation/API.h"
#include "Foundation/Inspect/Container.h"

namespace Helium
{
    namespace Inspect
    {
        struct CanvasShowArgs
        {
            CanvasShowArgs(bool show)
                : m_Show(show)
            {

            }

            bool m_Show;
        };
        typedef Helium::Signature< const CanvasShowArgs&> CanvasShowSignature;

        class FOUNDATION_API Canvas : public Reflect::AbstractInheritor<Canvas, Container>
        {
        public:
            Canvas();
            ~Canvas();

            virtual void RealizeControl(Control* control) = 0;
            virtual void UnrealizeControl(Control* control) = 0;

            int GetDefaultSize(Math::Axis axis)
            {
                return m_DefaultSize[axis];
            }

            int GetBorder()
            {
                return m_Border;
            }

            int GetPad()
            {
                return m_Pad;
            }

            CanvasShowSignature::Event e_Show;

        protected:
            Math::Point m_DefaultSize;  // standard control size
            int m_Border;               // standard border width
            int m_Pad;                  // standard pad b/t controls
        };

        typedef Helium::SmartPtr<Canvas> CanvasPtr;
    }
}