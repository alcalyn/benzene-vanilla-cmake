//----------------------------------------------------------------------------
/** @file MoHexPriorKnowledge.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "MoHexThreadState.hpp"
#include "MoHexPriorKnowledge.hpp"
#include "MoHexSearch.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

MoHexPriorKnowledge::MoHexPriorKnowledge(const MoHexThreadState& state) 
    : m_state(state)
{
}

MoHexPriorKnowledge::~MoHexPriorKnowledge()
{
}

void MoHexPriorKnowledge::ProcessPosition(std::vector<SgUctMoveInfo>& moves)
{
    if (m_state.Search().ProgressiveBiasConstant() == 0.0f)
        return;

    double TotalGamma = 0;
    double MoveGamma[BITSETSIZE];
    const MoHexPatterns& patterns = m_state.Search().GlobalPatterns();
    for (std::size_t i = 0; i < moves.size(); )
    {
        int type;
        double gamma = patterns.GetGammaFromBoard(m_state.GetMoHexBoard(), 12,
                                                  HexPoint(moves[i].m_move),
                                                  m_state.ColorToPlay(),
                                                  &type);
        if (type && moves.size() > 1)
        {
            // prune bad patterns, but don't let set go empty
            std::swap(moves[i], moves.back());
            moves.pop_back();
        }
        else
        {
            MoveGamma[(int)moves[i].m_move] = gamma;
            TotalGamma += gamma;
            ++i;
        }
    }
    if (TotalGamma == 0)
        return;
    for (std::size_t i = 0; i < moves.size(); ++i)
    {
        double gamma = MoveGamma[(int)moves[i].m_move];
        double prob = gamma / TotalGamma;
        moves[i].m_prior = (float)prob;
	moves[i].m_raveValue = 0.5f;
	moves[i].m_raveCount = 8;
    }
}

//----------------------------------------------------------------------------
