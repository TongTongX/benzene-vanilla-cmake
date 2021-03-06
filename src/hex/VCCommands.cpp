//----------------------------------------------------------------------------
/** @file VCCommands.cpp */
//----------------------------------------------------------------------------

#include "BoardUtil.hpp"
#include "EndgameUtil.hpp"
#include "VCCommands.hpp"
#include "VCUtil.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

VCCommands::VCCommands(Game& game, HexEnvironment& env)
    : m_game(game), 
      m_env(env)
{
}

VCCommands::~VCCommands()
{
}

void VCCommands::Register(GtpEngine& e)
{
    Register(e, "vc-between-cells-full", &VCCommands::CmdGetBetweenFull);
    Register(e, "vc-between-cells-semi", &VCCommands::CmdGetBetweenSemi);
    Register(e, "vc-connected-to-full", 
             &VCCommands::CmdGetCellsConnectedToFull);
    Register(e, "vc-connected-to-semi", 
             &VCCommands::CmdGetCellsConnectedToSemi);
    Register(e, "vc-get-mustplay", &VCCommands::CmdGetMustPlay);
    Register(e, "vc-intersection-full", &VCCommands::CmdIntersectionFull);
    Register(e, "vc-intersection-semi", &VCCommands::CmdIntersectionSemi);
    Register(e, "vc-union-full", &VCCommands::CmdUnionFull);
    Register(e, "vc-union-semi", &VCCommands::CmdUnionSemi);
    Register(e, "vc-build", &VCCommands::CmdBuildStatic);
    Register(e, "vc-build-incremental", &VCCommands::CmdBuildIncremental);
    Register(e, "vc-undo-incremental", &VCCommands::CmdUndoIncremental);
    Register(e, "vc-set-stats", &VCCommands::CmdSetInfo);
    Register(e, "vc-builder-stats", &VCCommands::CmdBuilderStats);
    Register(e, "vc-maintenance-responses", &VCCommands::CmdVCMResponses);
}

void VCCommands::AddAnalyzeCommands(HtpCommand& cmd)
{
    cmd << 
        "vc/VC Between Cells Full/vc-between-cells-full %c %P\n"
        "vc/VC Between Cells Semi/vc-between-cells-semi %c %P\n"
        "plist/VC Connected To Full/vc-connected-to-full %c %P\n"
        "plist/VC Connected To Semi/vc-connected-to-semi %c %P\n"
        "inferior/VC Get Mustplay/vc-get-mustplay %m\n"
        "plist/VC Intersection Full/vc-intersection-full %c %P\n"
        "plist/VC Intersection Semi/vc-intersection-semi %c %P\n"
        "plist/VC Union Full/vc-union-full %c %P\n"
        "plist/VC Union Semi/vc-union-semi %c %P\n"
        "inferior/VC Build/vc-build %m\n"
        "inferior/VC Build Incremental/vc-build-incremental %m %p\n"
        "inferior/VC Build Undo Incremental/vc-undo-incremental\n"
        "string/VC Set Stats/vc-set-stats %c\n"
        "string/VC Builder Stats/vc-builder-stats %c\n"
        "plist/VC Maintenance Responses/vc-maintenance-responses %p\n";
}

void VCCommands::Register(GtpEngine& engine, const std::string& command,
                          GtpCallback<VCCommands>::Method method)
{
    engine.Register(command, new GtpCallback<VCCommands>(this, method));
}

//----------------------------------------------------------------------------

/** Builds VCs for both players.
    Displays ice info for the given color in the current
    board-state. */
void VCCommands::CmdBuildStatic(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_env.SyncBoard(m_game.Board());
    brd.ComputeAll(color);
    cmd << brd.GetInferiorCells().GuiOutput();
    if (!EndgameUtil::IsDeterminedState(brd, color))
    {
        bitset_t consider = EndgameUtil::MovesToConsider(brd, color);
        cmd << BoardUtil::GuiDumpOutsideConsiderSet(brd.GetPosition(), 
                                                     consider,
                                              brd.GetInferiorCells().All());
    }
    cmd << '\n';
}

/** Builds VCs incrementally.
    Must play move to the board first. Move that was played must be passed
    as an argument. */
void VCCommands::CmdBuildIncremental(HtpCommand& cmd)
{
    cmd.CheckNuArgLessEqual(2);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexPoint point = HtpUtil::MoveArg(cmd, 1);
    HexBoard& brd = *m_env.brd; // <-- NOTE: no call to SyncBoard()!
    brd.PlayMove(color, point);
    cmd << brd.GetInferiorCells().GuiOutput();
    if (!EndgameUtil::IsDeterminedState(brd, color))
    {
        bitset_t consider = EndgameUtil::MovesToConsider(brd, color);
        cmd << BoardUtil::GuiDumpOutsideConsiderSet(brd.GetPosition(), 
                                                     consider,
                                           brd.GetInferiorCells().All());
    }
    cmd << '\n';
}

/** Reverts VCs built incrementally. */
void VCCommands::CmdUndoIncremental(HtpCommand& cmd)
{
    UNUSED(cmd);
    m_env.brd->UndoMove();
}

/** Returns list of VCs between two cells. */
void VCCommands::CmdGetBetweenFull(HtpCommand& cmd)
{
    cmd.CheckNuArg(3);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexPoint from = HtpUtil::MoveArg(cmd, 1);
    HexPoint to = HtpUtil::MoveArg(cmd, 2);
    HexBoard& brd = *m_env.brd;
    HexPoint fcaptain = brd.GetGroups().CaptainOf(from);
    HexPoint tcaptain = brd.GetGroups().CaptainOf(to);
    cmd << '\n';
    brd.Cons(color).DumpFulls(cmd, color, fcaptain, tcaptain);
}

/** Returns list of VCs between two cells. */
void VCCommands::CmdGetBetweenSemi(HtpCommand& cmd)
{
    cmd.CheckNuArg(3);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexPoint from = HtpUtil::MoveArg(cmd, 1);
    HexPoint to = HtpUtil::MoveArg(cmd, 2);
    HexBoard& brd = *m_env.brd;
    HexPoint fcaptain = brd.GetGroups().CaptainOf(from);
    HexPoint tcaptain = brd.GetGroups().CaptainOf(to);
    cmd << '\n';
    brd.Cons(color).DumpSemis(cmd, color, fcaptain, tcaptain);
}

/** Returns list of cells given cell is connected to via a full
    connection. */
void VCCommands::CmdGetCellsConnectedToFull(HtpCommand& cmd)
{
    cmd.CheckNuArg(2);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexPoint from = HtpUtil::MoveArg(cmd, 1);
    cmd << HexPointUtil::ToString(m_env.brd->Cons(color).GetFullNbs(from));
}

/** Returns list of cells given cell is connected to via a semi
    connection. */
void VCCommands::CmdGetCellsConnectedToSemi(HtpCommand& cmd)
{
    cmd.CheckNuArg(2);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexPoint from = HtpUtil::MoveArg(cmd, 1);
    cmd << HexPointUtil::ToString(m_env.brd->Cons(color).GetSemiNbs(from));
}

/** Prints the cells in the current mustplay. */
void VCCommands::CmdGetMustPlay(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    bitset_t mustplay = VCUtil::GetMustplay(*m_env.brd, color);
    InferiorCells inf(m_env.brd->GetInferiorCells());
    inf.ClearVulnerable();
    inf.ClearReversible();
    inf.ClearDominated();
    cmd << inf.GuiOutput();
    if (!EndgameUtil::IsDeterminedState(*m_env.brd, color))
    {
        bitset_t consider = EndgameUtil::MovesToConsider(*m_env.brd, color);
        cmd << BoardUtil::GuiDumpOutsideConsiderSet(m_env.brd->GetPosition(), 
                                                     consider,
                                                     inf.All());
    }
}

/** Prints cells in intersection of all connections between
    endpoints. */
void VCCommands::CmdIntersectionFull(HtpCommand& cmd)
{
    cmd.CheckNuArg(3);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexPoint from = HtpUtil::MoveArg(cmd, 1);
    HexPoint to = HtpUtil::MoveArg(cmd, 2);
    HexBoard& brd = *m_env.brd;
    HexPoint fcaptain = brd.GetGroups().CaptainOf(from);
    HexPoint tcaptain = brd.GetGroups().CaptainOf(to);
    cmd << HexPointUtil::ToString(brd.Cons(color).
            FullIntersection(fcaptain, tcaptain));
}

void VCCommands::CmdIntersectionSemi(HtpCommand& cmd)
{
    cmd.CheckNuArg(3);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexPoint from = HtpUtil::MoveArg(cmd, 1);
    HexPoint to = HtpUtil::MoveArg(cmd, 2);
    HexBoard& brd = *m_env.brd;
    HexPoint fcaptain = brd.GetGroups().CaptainOf(from);
    HexPoint tcaptain = brd.GetGroups().CaptainOf(to);
    cmd << HexPointUtil::ToString(brd.Cons(color).
            SemiIntersection(fcaptain, tcaptain));
}

/** Prints cells in the union of connections between endpoints. */
void VCCommands::CmdUnionFull(HtpCommand& cmd)
{
    cmd.CheckNuArg(3);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexPoint from = HtpUtil::MoveArg(cmd, 1);
    HexPoint to = HtpUtil::MoveArg(cmd, 2);
    HexBoard& brd = *m_env.brd;
    HexPoint fcaptain = brd.GetGroups().CaptainOf(from);
    HexPoint tcaptain = brd.GetGroups().CaptainOf(to);
    cmd << HexPointUtil::ToString(brd.Cons(color).
            FullGreedyUnion(fcaptain, tcaptain));
}

void VCCommands::CmdUnionSemi(HtpCommand& cmd)
{
    cmd.CheckNuArg(3);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexPoint from = HtpUtil::MoveArg(cmd, 1);
    HexPoint to = HtpUtil::MoveArg(cmd, 2);
    HexBoard& brd = *m_env.brd;
    HexPoint fcaptain = brd.GetGroups().CaptainOf(from);
    HexPoint tcaptain = brd.GetGroups().CaptainOf(to);
    cmd << HexPointUtil::ToString(brd.Cons(color).
            SemiGreedyUnion(fcaptain, tcaptain));
}

//----------------------------------------------------------------------------

void VCCommands::CmdVCMResponses(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexPoint p = HtpUtil::MoveArg(cmd, 0);
    HexBoard& brd = *m_env.brd;
    bitset_t responses;
    VCUtil::RespondToProbe(brd, brd.GetPosition().WhoseTurn(), p, responses);
    cmd << HexPointUtil::ToString(responses);
}

//----------------------------------------------------------------------------

/** Obtains statistics on connection set. */
void VCCommands::CmdSetInfo(HtpCommand& cmd)
{
    if (cmd.NuArg() == 0)
        throw HtpFailure("Need at least the color!");
    int maxConnections = 50;
    int numBins = 10;
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    if (cmd.NuArg() == 3)
    {
        maxConnections = cmd.ArgMin<int>(1, 1);
        numBins = cmd.ArgMin<int>(2, 1);
    }
    HexBoard& brd = *m_env.brd;
    brd.Cons(color).DumpDataStats(cmd, maxConnections, numBins);
}

/** Obtains statistics on connection set. */
void VCCommands::CmdBuilderStats(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = *m_env.brd;
    brd.Cons(color).DumpBuildStats(cmd);
}

//----------------------------------------------------------------------------


