#include <windows.h>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <tchar.h>
#include <string>
#include <queue>

using namespace std;


const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 10;


const int NUM_MINES = 15;


const int CELL_SIZE = 30;


const int EMPTY_CELL = 0;
const int MINE_CELL = -1;
const int FLAGGED_CELL = -3;

std::vector<std::vector<int>> board(BOARD_WIDTH, std::vector<int>(BOARD_HEIGHT, EMPTY_CELL));
std::vector<std::vector<int>> board2(BOARD_WIDTH, std::vector<int>(BOARD_HEIGHT, EMPTY_CELL));
HWND buttons[BOARD_WIDTH][BOARD_HEIGHT];
HWND flagButton;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Minesweeper"), NULL };
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, _T("Сапёр"), WS_OVERLAPPEDWINDOW, 100, 100, BOARD_WIDTH * CELL_SIZE+110, BOARD_HEIGHT * CELL_SIZE + 140, NULL, NULL, wc.hInstance, NULL);

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    srand(static_cast<unsigned int>(time(nullptr)));
    for (int i = 0; i < NUM_MINES; ++i)
    {
        int x, y;
        do
        {
            x = rand() % BOARD_WIDTH;
            y = rand() % BOARD_HEIGHT;
        } while (board[x][y] == MINE_CELL);
        board[x][y] = MINE_CELL;
    }
    for (int x = 0; x < BOARD_WIDTH; ++x)
    {
        for (int y = 0; y < BOARD_HEIGHT; ++y)
        {
            board2[x][y] = board[x][y];
            buttons[x][y] = CreateWindow(_T("BUTTON"), _T(""), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x * 40, y * 40, 40, 40, hwnd, NULL, wc.hInstance, NULL);
        }
    }

    flagButton = CreateWindow(_T("BUTTON"), _T("Флаг"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, BOARD_WIDTH * 40 + 20, 10, 80, 30, hwnd, (HMENU)1001, wc.hInstance, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}


bool CheckForWin()
{
    int openedCells = 0;
    for (int x = 0; x < BOARD_WIDTH; ++x)
    {
        for (int y = 0; y < BOARD_HEIGHT; ++y)
        {
            if (board[x][y] == -3 && board2[x][y] == -1)
            {
                openedCells++;
            }
        }
    }

    return openedCells == NUM_MINES;
}
int CountMinesAround(int x, int y)
{
    int count = 0;
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < BOARD_WIDTH && ny >= 0 && ny < BOARD_HEIGHT && board2[nx][ny] == MINE_CELL)
            {
                count++;
            }
        }
    }
    return count;
}
void add(int x, int y)
{
        TCHAR numberText[2];
        if (x < 0 || x >= BOARD_WIDTH || y < 0 || y >= BOARD_HEIGHT || CountMinesAround(x, y) != 0 || board[x][y] == -2 || board[x][y] == -1)
        {
            if (x >= 0 && x < BOARD_WIDTH && y >= 0 && y < BOARD_HEIGHT && board[x][y] != -1)
            {
                board[x][y] = -2;
               _itot_s(CountMinesAround(x, y), numberText, 10);
                SetWindowText(buttons[x][y], numberText);
            }
            return;
        }
        
        board[x][y] = -2;
        _itot_s(CountMinesAround(x, y), numberText, 10);
        SetWindowText(buttons[x][y], numberText);
        add(x, y+1);
        add(x, y-1);
        add(x+1, y);
        add(x-1, y);
            
}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int flc = 0;
    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

       
        for (int x = 0; x < BOARD_WIDTH; ++x)
        {
            for (int y = 0; y < BOARD_HEIGHT; ++y)
            {
                RECT cellRect = { x * CELL_SIZE, y * CELL_SIZE, (x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE };
                int cellValue = board[x][y];
                
                HBRUSH brush = CreateSolidBrush(RGB(192, 192, 192)); 
                FillRect(hdc, &cellRect, brush);
                DeleteObject(brush);

               
                
                if (cellValue > 0 && (GetKeyState(VK_SHIFT) >= 0))
                {
                    
                    wstring numberStr = to_wstring(cellValue);
                    DrawText(hdc, numberStr.c_str(), -1, &cellRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
                else if (cellValue == -2)
                {
                    
                    HBRUSH openBrush = CreateSolidBrush(RGB(224, 224, 224));
                    FillRect(hdc, &cellRect, openBrush);
                    DeleteObject(openBrush);
                }
            }
        }

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_COMMAND:
    {
        
        int id = LOWORD(wParam);
        int x, y;
        bool found = false;

        for (x = 0; x < BOARD_WIDTH; ++x)
        {
            for (y = 0; y < BOARD_HEIGHT; ++y)
            {
                if (buttons[x][y] == (HWND)lParam)
                {
                    found = true;
                    break;
                }
            }
            if (found) break;
        }

        if (found)
        {
            int cellValue = board[x][y];

            if (cellValue == MINE_CELL && (GetKeyState(VK_SHIFT) >= 0))
            {
                SetWindowText(buttons[x][y], L"X");
                MessageBox(hwnd, _T("Вы проиграли!"), _T("Поражение"), MB_ICONERROR);

            }
            else if (GetKeyState(VK_SHIFT) < 0)
            {
                if (board[x][y] == -3)
                {
                    SetWindowText(buttons[x][y], _T(" "));
                    board[x][y] = board2[x][y];
                    flc--;
                }
                else
                {
                    SetWindowText(buttons[x][y], _T("F"));
                    flc++;
                    board[x][y] = -3;
                }
            }
            else
            {
                add(x, y);
            }
            if (CheckForWin())
            {
                MessageBox(hwnd, _T("Вы выиграли!"), _T("Победа"), MB_ICONINFORMATION);
            }
        }

        break;
    }
    case WM_LBUTTONDOWN:
    {
        
        if (GetKeyState(VK_SHIFT) < 0)
        {
           
            int x, y;
            bool found = false;

           
            for (x = 0; x < BOARD_WIDTH; ++x)
            {
                for (y = 0; y < BOARD_HEIGHT; ++y)
                {
                    if (buttons[x][y] == (HWND)lParam)
                    {
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }

            
            if (found && board[x][y] != -2)
            {
              
                board[x][y] = FLAGGED_CELL; 
                SetWindowText(buttons[x][y], _T("F"));
                flc++;
            }
        }

        break;
    }
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
