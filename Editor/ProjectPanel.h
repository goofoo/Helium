#pragma once

#include "Foundation/Container/OrderedSet.h"
#include "Core/Project.h"

#include "Editor/EditorGenerated.h"
#include "Editor/FileDropTarget.h"

namespace Helium
{
    namespace Editor
    {
        class ProjectViewModel : public wxDataViewModel
        {
        public:
            void SetProject( Project* project );

            void PathAdded( const Path& path );
            void PathRemoved( const Path& path );

            // wxDataModel virtual interface
            unsigned int GetColumnCount() const HELIUM_OVERRIDE;
            wxString GetColumnType(unsigned int) const HELIUM_OVERRIDE;

            void GetValue(wxVariant &,const wxDataViewItem &,unsigned int) const HELIUM_OVERRIDE;
            bool SetValue(const wxVariant &,const wxDataViewItem &,unsigned int) HELIUM_OVERRIDE;

            wxDataViewItem GetParent(const wxDataViewItem &) const HELIUM_OVERRIDE;
            unsigned int GetChildren(const wxDataViewItem &,wxDataViewItemArray &) const HELIUM_OVERRIDE;

            bool IsContainer(const wxDataViewItem &) const HELIUM_OVERRIDE;

        protected:
            ProjectPtr    m_Project;
        };

        class ProjectPanel : public ProjectPanelGenerated
        {
        public:
            ProjectPanel( wxWindow* parent );
            virtual ~ProjectPanel();

            void SetProject( Project* project );

            // UI event handlers
			virtual void OnAddPath( wxCommandEvent& event ) HELIUM_OVERRIDE;
			virtual void OnDelete( wxCommandEvent& event ) HELIUM_OVERRIDE;

            virtual void OnDroppedFiles( const FileDroppedArgs& args );

        protected:
            ProjectPtr                          m_Project;
            wxObjectDataPtr< ProjectViewModel > m_Model;
            OrderedSet< Path* >                 m_Selected;
            FileDropTarget*                     m_DropTarget;
        };
    }
}