//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef HEXBOARD_H
#define HEXBOARD_H

#include <boost/scoped_ptr.hpp>

#include "ChangeLog.hpp"
#include "VCBuilder.hpp"
#include "Hex.hpp"
#include "ICEngine.hpp"
#include "PatternBoard.hpp"
#include "VCPattern.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Combines GroupBoard, PatternBoard, and VCSet into a board
    that handles all updates automatically.
  
    @todo Document me!
*/
class HexBoard : public PatternBoard
{
public:
    
    /** Creates a rectangular board. */
    HexBoard(int width, int height, const ICEngine& ice,
             VCBuilderParam& param);

    /** Copy constructor. */
    HexBoard(const HexBoard& other);

    /** Destructor. */
    ~HexBoard();

    //-----------------------------------------------------------------------

    /** @name Parameters */
    // @{

    /** Whether VCs are computed or not. */
    bool UseVCs() const;

    /** See UseVCs() */
    void SetUseVCs(bool enable);

    /** Whether ICE is used. */
    bool UseICE() const;

    /** See UseICE() */
    void SetUseICE(bool enable);

    /** Whether decompositions are found and filled-in. */
    bool UseDecompositions() const;

    /** See UseDecompositions() */
    void SetUseDecompositions(bool enable);

    /** Whether ICE info is backed-up in UndoMove(). */
    bool BackupIceInfo() const;

    /** See BackupIceInfo() */
    void SetBackupIceInfo(bool enable);

    // @}

    //-----------------------------------------------------------------------

    /** Copies state of stoneboard into this board. */
    void SetState(const StoneBoard& brd);

    //-----------------------------------------------------------------------

    /** Clears history.  Computes dead/vcs for current state. */
    virtual void ComputeAll(HexColor color);

    /** Stores old state on stack, plays move to board, updates
        ics/vcs.  Hash is modified by the move.  Allows ice info to
        be backed-up. */
    virtual void PlayMove(HexColor color, HexPoint cell);
    
    /** Stores old state on stack, plays set of stones, updates
        ics/vcs. HASH IS NOT MODIFIED! No ice info will be backed up,
        but this set of moves can be reverted with a single call to
        UndoMove(). */
    virtual void PlayStones(HexColor color, const bitset_t& played,
                            HexColor color_to_move);
        
    /** Adds stones for color to board with color_to_move about to
        play next; added stones must be a subset of the empty cells.
        Does not affect the hash of this state. State is not pushed
        onto stack, so a call to UndoMove() will undo these changes
        along with the last changes that changed the stack. */
    virtual void AddStones(HexColor color, const bitset_t& played,
                           HexColor color_to_move);

    /** Reverts to last state stored on the stack, restoring all state
        info. If the option is on, also backs up inferior cell
        info. */
    virtual void UndoMove();

    //-----------------------------------------------------------------------

    /** Returns the set of dead cells on the board. This is the union
        of all cells found dead previously during the history of moves
        since the last ComputeAll() call.  */
    bitset_t getDead() const;
    
    /** Returns the set of inferior cell. */
    const InferiorCells& getInferiorCells() const;

    /** Returns the Inferior Cell Engine the board is using. */
    const ICEngine& ICE() const;

    /** Returns the connection set for color. */
    const VCSet& Cons(HexColor color) const;

    /** Returns the connection set for color. */
    VCSet& Cons(HexColor color);

    /** Returns the connection builder for this board. */
    VCBuilder& Builder();

    /** Returns the connection builder for this board. */
    const VCBuilder& Builder() const;

private:
    
    /** Stores state of the board. */
    struct History
    {
        /** Saved board state. */
        StoneBoard board;

        /** The inferior cell data for this state. */
        InferiorCells inf;

        /** Color to play from this state. */
        HexColor to_play;
        
        /** Move last played from this state. */
        HexPoint last_played;

        History(const StoneBoard& b, const InferiorCells& i, 
                HexColor tp, HexPoint lp)
            : board(b), inf(i), to_play(tp), last_played(lp) 
        { }
    };

    //-----------------------------------------------------------------------

    /** @name Member variables. 
        @warning If you change anything here, be sure to update the
        copy constructor!!
    */

    // @{

    /** ICEngine used to compute inferior cells. */
    const ICEngine* m_ice;

    /** Builder used to compute virtual connections. */
    VCBuilder m_builder;

    /** Connection sets for black and white. */
    boost::scoped_ptr<VCSet> m_cons[BLACK_AND_WHITE];

    /** The vc changelogs for both black and white. */
    ChangeLog<VC> m_log[BLACK_AND_WHITE];

    /** History stack. */
    std::vector<History> m_history;

    /** The set of inferior cells for the current boardstate. */
    InferiorCells m_inf;

    /** See UseVCs() */
    bool m_use_vcs;

    /** See UseICE() */
    bool m_use_ice;

    /** See UseDecompositions() */
    bool m_use_decompositions;

    /** See BackupIceInfo() */
    bool m_backup_ice_info;

    // @}
    
    //-----------------------------------------------------------------------

    /** No assignments allowed! Use the copy constructor if you must
        make a copy, but you shouldn't be copying boards around very
        often. */
    void operator=(const HexBoard& other);

    void Initialize();

    void ComputeInferiorCells(HexColor color_to_move);

    void BuildVCs();

    void BuildVCs(bitset_t added[BLACK_AND_WHITE], bool mark_the_log = true);

    void RevertVCs();

    void HandleVCDecomposition(HexColor color_to_move);

    void ClearHistory();

    void PushHistory(HexColor color, HexPoint cell);

    void PopHistory();
};

inline bitset_t HexBoard::getDead() const
{
    return m_inf.Dead();
}

inline const InferiorCells& HexBoard::getInferiorCells() const
{
    return m_inf;
}

inline const ICEngine& HexBoard::ICE() const
{
    return *m_ice;
}

inline const VCSet& HexBoard::Cons(HexColor color) const
{
    return *m_cons[color].get();
}

inline VCSet& HexBoard::Cons(HexColor color)
{
    return *m_cons[color].get();
}

inline VCBuilder& HexBoard::Builder()
{
    return m_builder;
}

inline const VCBuilder& HexBoard::Builder() const
{
    return m_builder;
}

inline bool HexBoard::UseVCs() const
{
    return m_use_vcs;
}

inline void HexBoard::SetUseVCs(bool enable)
{
    m_use_vcs = enable;
}

inline bool HexBoard::UseICE() const
{
    return m_use_ice;
}

inline void HexBoard::SetUseICE(bool enable)
{
    m_use_ice = enable;
}

inline bool HexBoard::UseDecompositions() const
{
    return m_use_decompositions;
}

inline void HexBoard::SetUseDecompositions(bool enable)
{
    m_use_decompositions = enable;
}

inline bool HexBoard::BackupIceInfo() const
{
    return m_backup_ice_info;
}

inline void HexBoard::SetBackupIceInfo(bool enable)
{
    m_backup_ice_info = enable;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXBOARD_H
