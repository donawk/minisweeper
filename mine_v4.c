#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct
{
    int pos_x;
    int pos_y;
}
boardElement;

typedef int bool;
#define true 1
#define false 0

void unmaskBoardState(char *boardState, int boardSize, boardElement *mineList, int mineCount);
bool isUntouchedBoardState(char *boardState, int boardSize, boardElement bElem);
void playGame(char *boardState, int boardSize, boardElement *mineList, int mineCount);
void markMine(char *boardState, int boardSize, boardElement *flagList, int *flagCount, int *mineCount, boardElement bElem, bool isFlagged);
int adjecentMineCounter(char *boardState, int boardSize, boardElement *mineList, int mineCount, boardElement bElem);
boardElement selectBoardElement();
void populateEmptyBoard(char *boardState, int boardSize);
void generateMines(boardElement *mineList, int mineCount, int boardSize);
void printBoardState(char *boardState, int boardSize);
char *getBoardState(char *boardState, int boardSize, int pos_y, int pos_x);
void setBoardState(char *boardState, int boardSize, int pos_y, int pos_x, char toChar);
boardElement *getBoardElement(boardElement *bElems, int pos);
void setBoardElement(boardElement *toElem, boardElement fromElem);
int findBoardElement(boardElement *bElems, boardElement bElem, int count);
bool isMine(boardElement *mineList, int mineCount, boardElement bElem);

int main(void)
{
    int boardSize, mineCount;
    printf("Board size: ");
    scanf("%i", &boardSize);
    if (boardSize <= 0)
    {
        printf("Error: Board size too low");
        return 1;
    }
    printf("Mine count: ");
    scanf("%i", &mineCount);
    if (mineCount >= boardSize * boardSize)
    {
        printf("Error: Mine count too high");
        return 1;
    }

    char *boardState = malloc(sizeof(char) * boardSize * boardSize);
    populateEmptyBoard(boardState, boardSize);

    boardElement *mineList = malloc(sizeof(boardElement) * mineCount);
    generateMines(mineList, mineCount, boardSize);

    int flagCount = mineCount;
    boardElement *flagList = malloc(sizeof(boardElement) * mineCount);

    playGame(boardState, boardSize, mineList, mineCount);

    unmaskBoardState(boardState, boardSize, mineList, mineCount);
    printBoardState(boardState, boardSize);

    free(boardState);
    free(mineList);
    free(flagList);

    return 0;
}

void unmaskBoardState(char *boardState, int boardSize, boardElement *mineList, int mineCount)
{
    for (int i = 0; i < boardSize; i++)
    {
        for (int j = 0; j < boardSize; j++)
        {
            boardElement currBElem;
            currBElem.pos_y = i;
            currBElem.pos_x = j;

            if (isUntouchedBoardState(boardState, boardSize, currBElem))
            {
                if (isMine(mineList, mineCount, currBElem))
                {
                    setBoardState(boardState, boardSize, i, j, 'M');
                }
                else
                {
                    setBoardState(boardState, boardSize, i, j, 'O');
                }
            }
        }
    }
}

bool isUntouchedBoardState(char *boardState, int boardSize, boardElement bElem)
{
    if ((bElem.pos_x < 0 || bElem.pos_x >= boardSize) || (bElem.pos_y < 0 || bElem.pos_y >= boardSize))
    {
        return false;
    }
    if (*(getBoardState(boardState, boardSize, bElem.pos_y, bElem.pos_x)) == '#')
    {
        return true;
    }
    return false;
}

void updateBoardState(char *boardState, int boardSize, boardElement *mineList, int mineCount, boardElement bElem, int *nonMineCount)
{
    int adjMineCount = adjecentMineCounter(boardState, boardSize, mineList, mineCount, bElem);
    if (isUntouchedBoardState(boardState, boardSize, bElem))
    {
        setBoardState(boardState, boardSize, bElem.pos_y, bElem.pos_x, '0' + adjMineCount);
        (*nonMineCount)--;
    }
    else
    {
        return;
    }


    if (adjMineCount == 0)
    {
        const int regionalStates = 9;
        boardElement bElemDir[regionalStates];
        for (int i = 0; i < regionalStates; i++)
        {
            bElemDir[i] = bElem;
        }

        for (int i = -1, n = 0; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++, n++)
            {
                bElemDir[n].pos_y += i;
                bElemDir[n].pos_x += j;
            }
        }

        for (int i = 0; i < regionalStates; i++)
        {
            if (isUntouchedBoardState(boardState, boardSize, bElemDir[i]))
            {
                updateBoardState(boardState, boardSize, mineList, mineCount, bElemDir[i], nonMineCount);
            }
        }
    }
}

void playGame(char *boardState, int boardSize, boardElement *mineList, int mineCount)
{
    int originalMineCount = mineCount;
    int nonMineCount = (boardSize * boardSize) - mineCount;
    boardElement *flagList = malloc(sizeof(boardElement)* mineCount);
    int flagCount = 0;

    char gameMode = 'o';

    while (nonMineCount > 0)
    {
        printBoardState(boardState, boardSize);
        printf("Mine count: %i%c\nRemaining: %i\nFlagged: %i\n\n", mineCount, (originalMineCount != mineCount) ? '?' : '\0', nonMineCount, flagCount);
        printf("TO %s\n", (gameMode == 'o') ? "OPEN" : "FLAG");
        boardElement bElem = selectBoardElement();
        if (bElem.pos_x == -1 && bElem.pos_y == -1)
        {
            gameMode = (gameMode == 'o') ? 'm' : 'o';
        }
        else if (gameMode == 'o')
        {
            if (*(getBoardState(boardState, boardSize, bElem.pos_y, bElem.pos_x)) == '?')
            {
                printf("Can't access: Flagged as a mine.\n");
            }
            else if (isMine(mineList, mineCount, bElem))
            {
                printf("You hit a mine!\n\t= G A M E O V E R =\n");
                setBoardState(boardState, boardSize, bElem.pos_y, bElem.pos_x, 'X');
                free(flagList);
                return;
            }
            else
            {
                printf("\t= SAFE =\n");
                updateBoardState(boardState, boardSize, mineList, mineCount, bElem, &nonMineCount);
            }
        }
        else if (gameMode == 'm')
        {
            if(isUntouchedBoardState(boardState, boardSize, bElem))
            {
                markMine(boardState, boardSize, flagList, &flagCount, &mineCount, bElem, false);
            }
            else if (*(getBoardState(boardState, boardSize, bElem.pos_y, bElem.pos_x)) == '?')
            {
                markMine(boardState, boardSize, flagList, &flagCount, &mineCount, bElem, true);
            }
            else
            {
                printf("Can't access: Already opened.\n");
            }
        }
    }

    printf("\t= Y O U W I N =\n");
    free(flagList);
}

void markMine(char *boardState, int boardSize, boardElement *flagList, int *flagCount, int *mineCount, boardElement bElem, bool isFlagged)
{
    if (isFlagged)
    {
        setBoardState(boardState, boardSize, bElem.pos_y, bElem.pos_x, '#');
        int flagPos = findBoardElement(flagList, bElem, *flagCount);
        for (int i = flagPos; i < *(flagCount) - 1; i++)
        {
            setBoardElement(getBoardElement(flagList, i), *(getBoardElement(flagList, i + 1)));
        }
        (*flagCount)--;
        (*mineCount)++;
        printf("= UNFLAGGED =\n");
    }
    else
    {
        setBoardState(boardState, boardSize, bElem.pos_y, bElem.pos_x, '?');
        setBoardElement(getBoardElement(flagList, *flagCount), bElem);
        (*flagCount)++;
        (*mineCount)--;
        printf("= FLAGGED =\n");
    }
}

int adjecentMineCounter(char *boardState, int boardSize, boardElement *mineList, int mineCount, boardElement bElem)
{
    int mineCounter = 0;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            boardElement currBElem;
            currBElem.pos_y = i + bElem.pos_y;
            currBElem.pos_x = j + bElem.pos_x;
            if (isMine(mineList, mineCount, currBElem))
            {
                mineCounter++;
            }
        }
    }
    return mineCounter;
}

boardElement selectBoardElement()
{
    boardElement bElem;
    printf("Select a board element (-1, -1 to switch mode):\n");
    printf("Y: ");
    scanf("%i", &(bElem.pos_y));
    printf("X: ");
    scanf("%i", &(bElem.pos_x));
    return bElem;
}

void populateEmptyBoard(char *boardState, int boardSize)
{
    for (int i = 0; i < boardSize; i++)
    {
        for (int j = 0; j < boardSize; j++)
        {
            setBoardState(boardState, boardSize, i, j, '#');
        }
    }
}

void generateMines(boardElement *mineList, int mineCount, int boardSize)
{
    srand(time(0));

    boardElement discardBoardElement;
    discardBoardElement.pos_y = boardSize;
    discardBoardElement.pos_x = boardSize;

    for (int i = 0; i < mineCount; i++)
    {
        setBoardElement(getBoardElement(mineList, i), discardBoardElement);
    }

    int minesCreated = 0;
    while (minesCreated < mineCount)
    {
        boardElement newMine;
        newMine.pos_y = rand() % boardSize;
        newMine.pos_x = rand() % boardSize;

        if (!isMine(mineList, mineCount, newMine))
        {
            setBoardElement(getBoardElement(mineList, minesCreated), newMine);
            //printf("New mine at: %i, %i\n", newMine.pos_y, newMine.pos_x);
            minesCreated++;
        }
    }
}

void printBoardState(char *boardState, int boardSize)
{
    printf("\nX: ");
    for (int i = 0; i < boardSize; i++)
    {
        printf("%i ", i);
    }
    printf("\nY: ");
    for (int i = 0; i < boardSize; i++)
    {
        printf("- ");
    }

    for (int i = 0; i < boardSize; i++)
    {
        printf("\n%i: ", i);
        for (int j = 0; j < boardSize; j++)
        {
            printf("%c ", *(getBoardState(boardState, boardSize, i, j)));
        }
    }
    printf("\n\n");
}

char *getBoardState(char *boardState, int boardSize, int pos_y, int pos_x)
{
    return boardState + (boardSize * pos_y) + pos_x;
}

void setBoardState(char *boardState, int boardSize, int pos_y, int pos_x, char toChar)
{
    *(boardState + (boardSize * pos_y) + pos_x) = toChar;
}

boardElement *getBoardElement(boardElement *bElems, int pos)
{
    return bElems + (pos * sizeof(boardElement));
}

void setBoardElement(boardElement *toElem, boardElement fromElem)
{
    (*toElem).pos_y = fromElem.pos_y;
    (*toElem).pos_x = fromElem.pos_x;
}

int findBoardElement(boardElement *bElems, boardElement bElem, int count)
{
    for (int i = 0; i < count; i++)
    {
        boardElement currBElem = *(getBoardElement(bElems, i));
        if (currBElem.pos_y == bElem.pos_y && currBElem.pos_x == bElem.pos_x)
        {
            return i;
        }
    }
    return -1;
}

bool isMine(boardElement *mineList, int mineCount, boardElement bElem)
{
    for (int i = 0; i < mineCount; i++)
    {
        boardElement *currBElem = getBoardElement(mineList, i);
        if ((*currBElem).pos_x == bElem.pos_x && (*currBElem).pos_y == bElem.pos_y)
        {
            return true;
        }
    }

    return false;
}