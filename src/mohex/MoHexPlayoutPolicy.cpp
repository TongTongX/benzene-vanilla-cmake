//----------------------------------------------------------------------------
/** @file MoHexPlayoutPolicy.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgRandom.h"
#include "SgHash.h"
#include "MoHexPlayoutPolicy.hpp"
#include "Misc.hpp"
#include "PatternState.hpp"
#include "BoardUtil.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace {

//----------------------------------------------------------------------------

/** Shuffle a vector with the given random number generator. 
    @todo Refactor this out somewhere. */
template<typename T>
void ShuffleVector(std::vector<T>& v, SgRandom& random)
{
    for (int i = static_cast<int>(v.size() - 1); i > 0; --i) 
    {
        int j = random.Int(i+1);
        std::swap(v[i], v[j]);
    }
}

/** Returns true 'percent' of the time. */
bool PercentChance(int percent, SgRandom& random)
{
    if (percent >= 100) 
        return true;
    unsigned int threshold = random.PercentageThreshold(percent);
    return random.RandomEvent(threshold);
}

//----------------------------------------------------------------------------

} // annonymous namespace

//----------------------------------------------------------------------------

std::string MoHexPlayoutPolicyStatistics::ToString() const
{
    std::ostringstream os;
    os << "Playout Statistics:\n"
       << "Total          " << totalMoves << '\n'
       << "Pattern        " << patternMoves << " ("
       << std::setprecision(3) << double(patternMoves) * 100.0 
        / double(totalMoves) << "%)\n" 
       << "Random         " << randomMoves << " ("
       << std::setprecision(3) << double(randomMoves) * 100.0 
        / double(totalMoves) << "%)";
    return os.str();
}

//----------------------------------------------------------------------------

MoHexSharedPolicy::MoHexSharedPolicy()
    : m_config()
{
}

MoHexSharedPolicy::~MoHexSharedPolicy()
{
}

//----------------------------------------------------------------------------

MoHexPlayoutPolicy::MoHexPlayoutPolicy(MoHexSharedPolicy* shared,
                                       MoHexBoard& board)
    : m_shared(shared),
      m_board(board)
{
}

MoHexPlayoutPolicy::~MoHexPlayoutPolicy()
{
}

//----------------------------------------------------------------------------

void MoHexPlayoutPolicy::InitializeForSearch()
{
}

void MoHexPlayoutPolicy::InitializeForPlayout(const StoneBoard& brd)
{
    m_moves.clear();
    for (BitsetIterator it(brd.GetEmpty()); it; ++it)
        m_moves.push_back(*it);
    ShuffleVector(m_moves, m_random);
}

HexPoint MoHexPlayoutPolicy::GenerateMove(const HexState& state, 
                                          HexPoint lastMove)
{
    HexPoint move = INVALID_POINT;
    const MoHexPlayoutPolicyConfig& config = m_shared->Config();
    MoHexPlayoutPolicyStatistics& stats = m_shared->Statistics();
    // Patterns applied probabilistically (if heuristic is turned on)
    if (lastMove != INVALID_POINT 
        && config.patternHeuristic 
        && PercentChance(config.patternCheckPercent, m_random))
    {
        move = GeneratePatternMove(state, lastMove);
    }
    // Select random move if no move was selected by the heuristics
    if (move == INVALID_POINT) 
    {
	stats.randomMoves++;
        move = GenerateRandomMove(state.Position());
    } 
    else 
        stats.patternMoves++;
    BenzeneAssert(m_board.GetColor(move) == EMPTY);
    stats.totalMoves++;

    return move;
}

void MoHexPlayoutPolicy::PlayMove(HexPoint move, HexColor toPlay)
{
}

//--------------------------------------------------------------------------

/** Selects random move among the empty cells on the board. */
HexPoint MoHexPlayoutPolicy::GenerateRandomMove(const StoneBoard& brd)
{
    UNUSED(brd);
    HexPoint ret = INVALID_POINT;
    while (true) 
    {
	BenzeneAssert(!m_moves.empty());
        ret = m_moves.back();
        m_moves.pop_back();
        if (m_board.GetColor(ret) == EMPTY)
            break;
    }
    return ret;
}

/** Checks the save-bridge pattern. */
HexPoint MoHexPlayoutPolicy::GeneratePatternMove(const HexState& state, 
                                                 HexPoint lastMove)
{
    return m_board.SaveBridge(lastMove, state.ToPlay(), m_random);
}

//----------------------------------------------------------------------------
