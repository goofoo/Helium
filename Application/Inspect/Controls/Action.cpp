#include "Action.h"
#include "Application/Inspect/Controls/Container.h"

// Using
using namespace Inspect;

///////////////////////////////////////////////////////////////////////////////
// 
// 
Action::Action()
{
}

///////////////////////////////////////////////////////////////////////////////
// 
// 
bool Action::Write()
{
  m_ActionEvent.Raise( this );
  return true;
}