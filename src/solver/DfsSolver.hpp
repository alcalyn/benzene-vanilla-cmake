//----------------------------------------------------------------------------
/** @file DfsSolver.hpp
 */
//----------------------------------------------------------------------------

#ifndef DFSSOLVER_H
#define DFSSOLVER_H

#include <boost/scoped_ptr.hpp>

#include "Hex.hpp"
#include "HexBoard.hpp"
#include "VC.hpp"
#include "TransTable.hpp"
#include "DfsData.hpp"
#include "SolverDB.hpp"
#include "PositionDB.hpp"
#include "HexEval.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------

/** Transposition table for use in DfsSolver. */
typedef TransTable<DfsData> DfsHashTable;

/** Database for use in DfsSolver. */
typedef PositionDB<DfsData> DfsDB;

/** Solver database combining both of the above. */
typedef SolverDB<DfsHashTable, DfsDB, DfsData> DfsPositions;

//----------------------------------------------------------------------

/** Determines the winner of a gamestate.
    DfsSolver uses a mustplay driven depth-first search to determine the
    winner in the given state.  A transposition table and a database
    of solved positions are also used to reduce the amount of work.
*/
class DfsSolver 
{
public:
    DfsSolver();

    ~DfsSolver();

    //------------------------------------------------------------------------

    /** Return type for solve(): player to move wins, player to move
     loses, unknown result (timelimit or depth limit reached). */
    typedef enum { WIN, LOSS, UNKNOWN } Result;

    /** Stats for a branch of the search tree. */
    struct BranchStatistics
    {
        /** Total states in tree if no DB and no TT. */
        unsigned total_states;

        /** States actually visited; includes leafs, tt and db hits. */
        unsigned explored_states;

        /** Expanded nodes; non leaf, non tt and db hit states. */
        unsigned expanded_states;

        /** Number of expanded nodes assuming perfect move ordering 
            (assuming the same set of winning moves). */
        unsigned minimal_explored;
        
        /** Decompositions found; if black is to move, it must be a
            decomposition for white. */
        unsigned decompositions;

        /** Decompositions where the player to move won. */
        unsigned decompositions_won;

        /** Total number of moves to consider in expanded states. 
            Includes moves that are later pruned (by mustplay or
            from skipping due to finding a win). */
        unsigned moves_to_consider;

        /** Number of expanded states that had winning moves. */
        unsigned winning_expanded;

        /** Number of branches tried before win was found. */
        unsigned branches_to_win;

        /** States pruned by mustplay pruning. */
        unsigned pruned;

        /** Number of proofs that were successfully shrunk. */
        unsigned shrunk;
        
        /** Total number of cells removed in all successful proof
            shrinkings. */
        unsigned cells_removed;

        BranchStatistics() 
            : total_states(0), explored_states(0), 
              expanded_states(0), minimal_explored(0), 
              decompositions(0), decompositions_won(0), 
              moves_to_consider(0), winning_expanded(0), branches_to_win(0),
              pruned(0), shrunk(0), cells_removed(0)
        { }

        void operator+=(const BranchStatistics& o)
        {
            total_states += o.total_states;
            explored_states += o.explored_states;
            expanded_states += o.expanded_states;
            minimal_explored += o.minimal_explored;
            decompositions += o.decompositions;
            decompositions_won += o.decompositions_won;
            moves_to_consider += o.moves_to_consider;
            winning_expanded += o.winning_expanded;
            branches_to_win += o.branches_to_win;
            pruned += o.pruned;
            shrunk += o.shrunk;
            cells_removed += o.cells_removed;
        }
    };

    //------------------------------------------------------------------------

    /** Contains all relevant data for a solution to a state. */
    struct SolutionSet
    {
        bitset_t proof;

        int moves_to_connection;

        PointSequence pv;

        BranchStatistics stats;

        SolutionSet()
	  : moves_to_connection(0) {};
    };
    
    //------------------------------------------------------------------------

    static const int NO_DEPTH_LIMIT    = -1;

    static const double NO_TIME_LIMIT  = -1.0;
    
    static const int SOLVE_ROOT_AGAIN = 1;

    /** User controllable settings. 
        @todo Combine these with the parameters below.
    */
    struct Settings
    {
        int flags;

        int depthLimit;

        double timeLimit;

        Settings() : flags(0) { };
    };

    /** Sets the flags for the next solver run. */
    void SetFlags(int flags);

    /** Gets the current flags. */
    int GetFlags() const;
   
    //------------------------------------------------------------------------

    /** Solves state using the given database (pass in 0 for no
        database). Numstones sets the maximum number of stones allowed
        in a db state; transtones sets the maximum number of stones in
        states stored with proof transpositions.  Returns WIN/LOSS if
        color to play wins/loses; otherwise UNKNOWN.
    */
    Result Solve(HexBoard& board, HexColor toplay, SolutionSet& solution,
                 DfsPositions& positions,
                 int depth_limit = NO_DEPTH_LIMIT, 
                 double time_imit = NO_TIME_LIMIT);

    //------------------------------------------------------------------------

    /** @name Parameters */
    // @{

    /** Controls whether gamestates decomposible into separate
        components have each side solved separately and the proofs
        combined as necessary. */
    bool UseDecompositions() const;

    /** See UseDecompositions() */
    void SetUseDecompositions(bool enable);

    /** Depth from root in which the current variation is printed. */
    int ProgressDepth() const;

    /** See ProgressDepth() */
    void SetProgressDepth(int depth);

    /** Depth at which the current state is dumped to the log. */
    int UpdateDepth() const;

    /** See UpdateDepth() */
    void SetUpdateDepth(int depth);

    /** Whether ICE is used to provably shrink proofs. */
    bool ShrinkProofs() const;

    /** See ShrinkProofs() */
    void SetShrinkProofs(bool enable);

    /** Use newly acquired ICE-info after the move ordering stage to
        prune the moves to consider. */
    bool BackupIceInfo() const;

    /** See BackupIceInfo() */
    void SetBackupIceInfo(bool enable);

    bool UseGuiFx() const;

    /** See UseGuiFx() */
    void SetUseGuiFx(bool enable);

    /** Each move is played and the size of the resulting mustplay is
        stored. Moves are ordered in increasing order of mustplay.
        This is a very, very, expensive move ordering, since the vcs
        and inferior cells must be updated for every possible move in
        every possible state.  However, the move ordering is usually
        very good. For example, it is not possible to solve 7x7
        without using this heuristic. */
    static const int ORDER_WITH_MUSTPLAY = 1;    

    /** Resistance score is used to break ties instead of distance
        from the center of the board. */
    static const int ORDER_WITH_RESIST = 2;

    /** Moves near center of board get higher priority than moves near
        the edge of the board. */
    static const int ORDER_FROM_CENTER = 4;

    /** Returns the move order flags. */
    int MoveOrdering() const;

    /** See MoveOrdering() */
    void SetMoveOrdering(int flags);

    // @}

    //------------------------------------------------------------------------

    /** Dumps the stats on # of states, branching factors, etc, for
        the last run. */
    void DumpStats(const SolutionSet& solution) const;

private:

    //------------------------------------------------------------------------
    
    /** Globabl statistics for the current solver run. */
    struct GlobalStatistics
    {
        /** Times HexBoard::PlayMove() was called. */
        unsigned played;

        GlobalStatistics()
            : played(0)
        { }
    };

    //------------------------------------------------------------------------

    /** Stats for the entire search tree broken down by level. */
    struct Histogram
    {
        /** Map of # of stones to a counter. */
        typedef std::map<int, u64> StatsMap;

        /** Terminal states encountered at each depth. */
        StatsMap terminal;
        
        /** Internal states encountered at each depth. */
        StatsMap states;

        /** Winning states encountered at each depth. */
        StatsMap winning;

        StatsMap size_of_winning_states;
        
        StatsMap size_of_losing_states;

        /** Branches taken to find winning move at each depth. */
        StatsMap branches;

        /** Size of original mustplay in winning states. */
        StatsMap mustplay;

        /** States under losing moves before winning move. */
        StatsMap states_under_losing;

        /** DB/TT hits at each depth. */
        StatsMap tthits;

        /** Dumps histogram to a string. */
        std::string Dump();
    };

    //------------------------------------------------------------------------

    /** Plays the move; updates the board.  */
    void PlayMove(HexBoard& brd, HexPoint cell, HexColor color);
    
    /** Takes back the move played. */
    void UndoMove(HexBoard& brd, HexPoint cell);

    //------------------------------------------------------------------------

    void Initialize(const HexBoard&);

    /** Returns true if state is in DB or TT.  Checks DB first, then TT. 
        If return is true, info is stored in state. */
    bool CheckTransposition(DfsData& state) const;

    /** Stores the solved state in the TT or DB. Sets the state's
        bestmove parameter to the best move computed by solver. */
    void StoreState(const DfsData& state, const bitset_t& proof);

    bitset_t DefaultProofForWinner(const HexBoard& brd, HexColor winner) const;

    //------------------------------------------------------------------------

    /** Checks timelimit and SgUserAbort(). Sets m_aborted if
        necessary, aborting the search. Returns true if search should
        be aborted, false otherwise. */
    bool CheckAbort();
    
    /** Returns true if current state is a terminal node (win/loss),
        or a DB/TT hit.  If so, info is stored in state.  If root_node
        is true and SOLVE_ROOT_AGAIN is set, then no transpositions
        are checked. */
    bool HandleLeafNode(const HexBoard& brd, HexColor color, 
                        DfsData& state, bool root_node,
                        bitset_t& proof) const;

    /** Returns true if node is terminal. Fills in state if terminal. 
        State's bestmove field is not specified here.
     */
    bool HandleTerminalNode(const HexBoard& brd, HexColor color, 
                            DfsData& state, bitset_t& proof) const;

    //------------------------------------------------------------------------

    /** Orders the moves in mustplay using several heuristics. 
        Aborts move ordering early if it finds a TT win: winning
        move is put to the front. 

        Will shrink the mustplay if it encounters TT losses: losing
        moves are not added to the list of sorted moves.

        Returns true if it found a TT win, false otherwise. 
    */
    bool OrderMoves(HexBoard& brd, HexColor color, bitset_t& mustplay, 
                    SolutionSet& solution, 
                    std::vector<HexMoveValue>& moves);

    /** Plays the move and returns the opponent's mustplay
        size. Stores whether there is a winning semi in
        winning_semi_exists. */
    int MustPlaySize(HexBoard& brd, HexColor color,  HexPoint cell,
                     bool& state_in_db, 
                     bool& winning_semi_exists) const;

    //------------------------------------------------------------------------

    /** Helper for solve(). */
    Result run_solver(HexBoard& brd, HexColor tomove, SolutionSet& solution);

    /** Solves the current state in brd for the color to move. Handles
        decompositions if option is turned on. */
    bool solve_state(HexBoard& brd, HexColor tomove,
                     PointSequence& variation, 
                     SolutionSet& solution);

    /** Solves each side of the decompsosition; combines proofs if
        necessary. */
    bool solve_decomposition(HexBoard& brd, HexColor color, 
                             PointSequence& variation,
                             SolutionSet& solution,
                             HexPoint group);

    /** Does the recursive mustplay search; calls solve_state() on
        child states. */
    bool solve_interior_state(HexBoard& brd, HexColor color, 
                              PointSequence& variation,
                              SolutionSet& solution);

    /** Shrinks/verifies proof; stores in tt/db. */
    void handle_proof(const HexBoard& brd, HexColor color, 
                      const PointSequence& variation,
                      bool winning_state, SolutionSet& solution);

    //------------------------------------------------------------------------

    DfsPositions* m_positions;

    double m_start_time, m_end_time;

    std::vector<std::pair<int, int> > m_completed;

    bool m_aborted;

    Settings m_settings;

    mutable Histogram m_histogram;

    mutable GlobalStatistics m_statistics;
    
    /** Board with no fillin. */
    boost::scoped_ptr<StoneBoard> m_stoneboard;

    /** See UseDecompositions() */
    bool m_use_decompositions;

    /** See ProgressDepth() */
    int m_progress_depth;

    /** See UpdateDepth() */
    int m_update_depth;

    /** See ShrinkProofs() */
    bool m_shrink_proofs;

    /** See BackupIceInfo() */
    bool m_backup_ice_info;

    /** See UseGuiFx() */
    bool m_use_guifx;

    /** See MoveOrdering() */
    int m_move_ordering;
};

//----------------------------------------------------------------------------

inline void DfsSolver::SetFlags(int flags)
{
    m_settings.flags = flags;
}

inline int DfsSolver::GetFlags() const
{
    return m_settings.flags;
}

inline bool DfsSolver::UseDecompositions() const
{
    return m_use_decompositions;
}

inline void DfsSolver::SetUseDecompositions(bool enable)
{
    m_use_decompositions = enable;
}

inline int DfsSolver::ProgressDepth() const
{
    return m_progress_depth;
}

inline void DfsSolver::SetProgressDepth(int depth)
{
    m_progress_depth = depth;
}

inline int DfsSolver::UpdateDepth() const
{
    return m_update_depth;
}

inline void DfsSolver::SetUpdateDepth(int depth)
{
    m_update_depth = depth;
}

inline bool DfsSolver::ShrinkProofs() const
{
    return m_shrink_proofs;
}

inline void DfsSolver::SetShrinkProofs(bool enable)
{
    m_shrink_proofs = enable;
}

inline bool DfsSolver::BackupIceInfo() const
{
    return m_backup_ice_info;
}

inline void DfsSolver::SetBackupIceInfo(bool enable)
{
    m_backup_ice_info = enable;
}

inline bool DfsSolver::UseGuiFx() const
{
    return m_use_guifx;
}

inline void DfsSolver::SetUseGuiFx(bool enable)
{
    m_use_guifx = enable;
}

inline int DfsSolver::MoveOrdering() const
{
    return m_move_ordering;
}

inline void DfsSolver::SetMoveOrdering(int flags)
{
    m_move_ordering = flags;
}

//----------------------------------------------------------------------------

/** Methods in DfsSolver that do not need DfsSolver's private data. */
/** @todo Refactor some of these out? */
namespace DfsSolverUtil 
{
    /** Prints the variation; for debugging purposes. */
    std::string PrintVariation(const PointSequence& variation);
    
    /** Computes distance from the center of the board. */
    int DistanceFromCenter(const ConstBoard& brd, HexPoint p);

    /** Determines if this is a winning state. If so, proof is set to
        the winning proof. */
    bool isWinningState(const HexBoard& brd, HexColor color, 
                        bitset_t& proof);

    /** Determines if the given board state is losing for color. If
        so, proof is set. */
    bool isLosingState(const HexBoard& brd, HexColor color, bitset_t& proof);

    /** Computes the moves to consider for this state. */
    bitset_t MovesToConsider(const HexBoard& brd, HexColor color,
                             bitset_t& proof);

    /** Computes the union of opponent winning semis. */
    bitset_t MustplayCarrier(const HexBoard& brd, HexColor color);

    /** Returns the original losing proof for this state; ie, the
        union of the opponent's winning semi-connections. */
    bitset_t InitialProof(const HexBoard& brd, HexColor color);

    /** Gives all cells outside of the proof to loser, computes fillin
        using ice, removes any cell in proof that is filled-in. */
    void ShrinkProof(bitset_t& proof, 
                     const StoneBoard& board, HexColor loser, 
                     const ICEngine& ice);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // DFSSOLVER_H
