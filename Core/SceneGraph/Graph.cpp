/*#include "Precompile.h"*/
#include "Graph.h"
#include "Core/SceneGraph/SceneNode.h"

//#define SCENE_DEBUG_EVALUATE

using namespace Helium;
using namespace Helium::SceneGraph;

REFLECT_DEFINE_CLASS( Graph );

void Graph::InitializeType()
{
  Reflect::RegisterClassType< Graph >( TXT( "Graph" ) );
}

void Graph::CleanupType()
{
  Reflect::UnregisterClassType< Graph >();
}

Graph::Graph()
: m_NextID (1)
, m_CurrentID (0)
{

}

void Graph::Reset()
{
  for each (SceneGraph::SceneNode* node in m_OriginalNodes)
  {
    node->Reset();
  }

  m_OriginalNodes.clear();

  for each (SceneGraph::SceneNode* node in m_IntermediateNodes)
  {
    node->Reset();
  }

  m_IntermediateNodes.clear();

  for each (SceneGraph::SceneNode* node in m_TerminalNodes)
  {
    node->Reset();
  }

  m_TerminalNodes.clear();

  m_CurrentID = 0;
  m_NextID = 1;
}

u32 Graph::AssignVisitedID()
{
  m_NextID++;

  if (m_NextID == 0)
  {
    ResetVisitedIDs();
  }
  
  return m_NextID;
}

void Graph::ResetVisitedIDs()
{
  for each (SceneGraph::SceneNode* n in m_OriginalNodes)
  {
    n->SetVisitedID(0);
  }

  for each (SceneGraph::SceneNode* n in m_IntermediateNodes)
  {
    n->SetVisitedID(0);
  }

  for each (SceneGraph::SceneNode* n in m_TerminalNodes)
  {
    n->SetVisitedID(0);
  }
}

void Graph::Classify(SceneGraph::SceneNode* n)
{
  if (n->GetAncestors().empty())
  {
    m_OriginalNodes.insert( n );
    m_IntermediateNodes.erase( n );
    m_TerminalNodes.erase( n );
  }
  else if (n->GetDescendants().empty())
  {
    m_OriginalNodes.erase( n );
    m_IntermediateNodes.erase( n );
    m_TerminalNodes.insert( n );
  }
  else
  {
    m_OriginalNodes.erase( n );
    m_IntermediateNodes.insert( n );
    m_TerminalNodes.erase( n );
  }
}

void Graph::AddNode(SceneGraph::SceneNode* n)
{
  // Track this node
  Classify(n);

  // Make the node aware of the graph
  n->SetGraph( this );

  // Force Evaluation
  n->Dirty();

  // Reset visited status (just in case)
  n->SetVisitedID(0);
}

void Graph::RemoveNode(SceneGraph::SceneNode* n)
{
  m_OriginalNodes.erase( n );
  m_IntermediateNodes.erase( n );
  m_TerminalNodes.erase( n );

  n->SetGraph( NULL );
}

u32 Graph::DirtyNode( SceneGraph::SceneNode* node, GraphDirection direction )
{
  u32 count = 0;

  node->SetNodeState(direction, NodeStates::Dirty);
  count++;

  switch (direction)
  {
  case GraphDirections::Downstream:
    {
      std::stack<SceneGraph::SceneNode*> descendantStack;

      for each (SceneGraph::SceneNode* d in node->GetDescendants())
      {
        if ( d->GetNodeState(direction) != NodeStates::Dirty )
        {
          descendantStack.push( d );
        }
      }

      while (!descendantStack.empty())
      {
        SceneGraph::SceneNode* descendant = descendantStack.top();

        descendantStack.pop();

        descendant->SetNodeState(direction, NodeStates::Dirty);
        count++;

        for each (SceneGraph::SceneNode* d in descendant->GetDescendants())
        {
          if ( d->GetNodeState(direction) != NodeStates::Dirty )
          {
            descendantStack.push( d );
          }
        }
      }

      break;
    }

  case GraphDirections::Upstream:
    {
      std::stack<SceneGraph::SceneNode*> ancestorStack;

      for each (SceneGraph::SceneNode* d in node->GetAncestors())
      {
        if ( d->GetNodeState(direction) != NodeStates::Dirty )
        {
          ancestorStack.push( d );
        }
      }

      while (!ancestorStack.empty())
      {
        SceneGraph::SceneNode* ancestor = ancestorStack.top();

        ancestorStack.pop();

        ancestor->SetNodeState(direction, NodeStates::Dirty);
        count++;

        for each (SceneGraph::SceneNode* d in ancestor->GetAncestors())
        {
          if ( d->GetNodeState(direction) != NodeStates::Dirty )
          {
            ancestorStack.push( d );
          }
        }
      }

      break;
    }
  }

  return count;
}

EvaluateResult Graph::EvaluateGraph(bool silent)
{
  EvaluateResult result;

  CORE_EVALUATE_SCOPE_TIMER( ("") );

  u64 start = Helium::TimerGetClock();

  m_EvaluatedNodes.clear();

  for each (SceneGraph::SceneNode* n in m_TerminalNodes)
  {
    if (n->GetNodeState(GraphDirections::Downstream) == NodeStates::Dirty)
    {
      Evaluate(n, GraphDirections::Downstream);
    }
  }

  for each (SceneGraph::SceneNode* n in m_OriginalNodes)
  {
    if (n->GetNodeState(GraphDirections::Upstream) == NodeStates::Dirty)
    {
      Evaluate(n, GraphDirections::Upstream);
    }
  }

  result.m_NodeCount = (int)m_EvaluatedNodes.size();

  m_EvaluatedEvent.Raise( m_EvaluatedNodes );

  m_CleanupRoots.clear();

  result.m_TotalTime = Helium::CyclesToMillis(Helium::TimerGetClock() - start);

  return result;
}

void Graph::Evaluate(SceneGraph::SceneNode* node, GraphDirection direction)
{
  switch (direction)
  {
  case GraphDirections::Downstream:
    {
      for each (SceneGraph::SceneNode* ancestor in node->GetAncestors())
      {
        if (ancestor->GetNodeState(direction) == NodeStates::Dirty)
        {
          Evaluate(ancestor, direction);
        }
      }

      break;
    }

  case GraphDirections::Upstream:
    {
      for each (SceneGraph::SceneNode* descendant in node->GetDescendants())
      {
        if (descendant->GetNodeState(direction) == NodeStates::Dirty)
        {
          Evaluate(descendant, direction);
        }
      }

      break;
    }
  }

  // perform evaluate
  node->DoEvaluate(direction);

  m_EvaluatedNodes.insert( node );
}