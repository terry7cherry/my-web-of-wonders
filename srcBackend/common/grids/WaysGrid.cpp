#include "WaysGrid.h"


// game globals declared elsewhere
extern Console gameConsole;

WaysGrid::WaysGrid(const GameMaths& maths)
    : WaysGrid(maths.ints.at("numReels"), maths.ints.at("numRows"), maths.payTables.at("default"))
{
}

WaysGrid::WaysGrid(const size_t reels, const size_t rows, const PayTable& payTable)
    : PayGrid(reels, rows, payTable)
{
    maxWays = static_cast<int>(pow(rows, reels));
    winVector.reserve(payTable.numSymbols);
    winsToProcess.reserve(payTable.numSymbols);
}

WaysGrid::~WaysGrid()
{
    winVectorReset();
}

void WaysGrid::reset(void)
{
    Grid::reset();
    winVectorReset();
}

void WaysGrid::winVectorReset(void)
{
    winVector.clear();
    winsToProcess.clear();
}


int WaysGrid::evaluateWins(const unsigned int betMultiplier,
                           MultipliersCombineType combineType)
{
    winVectorReset();

    if (!quickWinCheck())
        return 0;

    const auto& symbolVect = payTable.symbolVect;

    uint32_t m = quickWinMask;
    while (m != 0)
    {
        // Get next winning symbol in win mask.
        const unsigned i = std::countr_zero(m); // index of lowest set bit
        m &= (m - 1); // clear lowest set bit

        const SymbolData& symData = symbolVect[i];
        const auto& win = winsToProcess.emplace_back(
            std::make_unique<WaysWin>(symData.symbol, maxReels, maxRows));
        size_t row = 0;
        for (const Symbol& reelSymbol : reelSymbolVect[0])
        {
            if (reelSymbol == SYMBOL_NONE) break;
            if (reelSymbol == symData.symbol)
                win->addPosition(0, row, false);
            row++;
        }
    }
    size_t numWinsToProcess = winsToProcess.size();
    for (size_t reel = 1; reel < maxReels && numWinsToProcess; reel++)
    {
        size_t symbolsToProcess = reelHeightVect[reel];
        for (const auto& win : winsToProcess)
        {
            if (win->analysisComplete) continue;
            bool continuedOnThisReel = false;

            if (symbolsToProcess)
                for (int row = 0; row < reelHeightVect[reel]; row++)
                {
                    const auto& reelSymbol = reelSymbolVect[reel][row];
                    if (reelSymbol == SYMBOL_NONE)
                        break;
                    if (const bool isWild = symbolVect[reelSymbol].isWild;
                        isWild || win->symbol == reelSymbol)
                    {
                        continuedOnThisReel = true;
                        win->addPosition(reel, row, isWild);
                        if (!isWild) symbolsToProcess--;
                    }
                }

            if (!continuedOnThisReel || reel == maxReels - 1)
            {
                win->analysisComplete = true;
                numWinsToProcess--;
            }
        }
    }

    if (multipliersSet)
    {
    }

    int spinWin = 0;
    for (auto& win : winsToProcess)
    {
        if (const int winVal = win->setWin(payTable, betMultiplier))
        {
            if (multipliersSet)
            {
                std::vector<std::unique_ptr<WaysWin>> winCopies;
                winCopies.emplace_back(std::make_unique<WaysWin>(win->symbol, win->maxReels, win->maxRows));
                for (size_t reel = 0; reel < win->oak; ++reel)
                {
                    const auto positions = win->getPositionsOnReel(reel);
                    std::unordered_map<unsigned int, std::vector<std::pair<size_t, bool>>> multiplierMap;
                    for (const auto& pos : positions)
                        multiplierMap[multiplierVect[reel][pos.first]].push_back(pos);
                    const size_t winsToCopy = winCopies.size();
                    for (size_t i = 1; i < multiplierMap.size(); ++i)
                        for (size_t j = 0; j < winsToCopy; ++j)
                            winCopies.emplace_back(std::make_unique<WaysWin>(*winCopies[j]));
                    size_t multiplierIndex = 0;
                    for (const auto& [multiplier, positionsWithMultiplier] : multiplierMap)
                    {
                        for (const auto& [row, isWild] : positionsWithMultiplier)
                            for (size_t j = 0; j < winsToCopy; ++j)
                                winCopies[j + multiplierIndex * winsToCopy]->addPosition(
                                    reel, row, isWild, multiplier, combineType);
                        ++multiplierIndex;
                    }
                }
                winVector.reserve(winVector.size() + winCopies.size());
                for (auto& winCopy : winCopies)
                {
                    spinWin += winCopy->setWin(payTable, betMultiplier);
                    winVector.emplace_back(std::move(winCopy));
                }
            }
            else
            {
                spinWin += winVal;
                winVector.emplace_back(std::move(win));
            }
        }
    }
    return spinWin;
}

int WaysGrid::setWinMultiplier(const Symbol symbol, const int multiplier) const
{
    int newWin = 0;
    int initialWin = 0;
    for (auto& win : winVector)
    {
        if (win->symbol == symbol)
        {
            initialWin = win->totalWin;
            win->totalWin *= multiplier;
            newWin = win->totalWin - initialWin;
            return newWin;
        }
    }
    return newWin;
}

std::vector<std::string> WaysGrid::getWinTextVect(const std::map<Symbol, std::string>& symbolMap) const
{
    std::vector<std::string> winTextVect;
    for (auto& win : winVector)
        winTextVect.push_back(win->getWinText(symbolMap));
    return winTextVect;
}

std::vector<std::string> WaysGrid::getWinTextVect() const
{
    std::vector<std::string> winTextVect;
    for (auto& win : winVector)
        winTextVect.push_back(win->getWinText(mTextMap));
    return winTextVect;
}

std::vector<std::vector<std::vector<int>>> WaysGrid::getWinPosVect() const
{
    std::vector<std::vector<std::vector<int>>> winPosVect;
    for (auto& win : winVector)
    {
        if (win->winType != "ways") continue;
        const auto& waysWin = dynamic_cast<const WaysWin&>(*win);
        winPosVect.push_back(waysWin.winPosVect);
    }
    return winPosVect;
}

unsigned int WaysGrid::applyGridMultiplier(const MultipliersCombineType combineType,
                                           const std::optional<int> specialMultiplier)
{
    const int applyMultiplier = (specialMultiplier.has_value())
                                    ? specialMultiplier.value()
                                    : this->gridMultiplier;
    unsigned int totalWin = 0;
    if (applyMultiplier > 1)
    {
        for (const auto& i : winVector)
            totalWin += i->applyMultiplier(applyMultiplier, combineType);
    }
    return totalWin;
}

void WaysGrid::applySpin(const SpinResult& spinResult, const bool getLockedReels)
{
    Grid::applySpin(spinResult, getLockedReels);
    if (reelContainsSymbol(0, payTable.wildSymVect))
        throw std::runtime_error(
            "WaysGrid::applySpin() - Wild symbol found on first reel. This is not supported.");
}
