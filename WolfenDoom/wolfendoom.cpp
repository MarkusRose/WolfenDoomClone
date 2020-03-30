#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
using namespace std;

#include <Windows.h>

int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4.0;
float fDepth = 16.f;

int main()
{
	// Create Screen Buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	wstring map;

	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#.....####.....#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#...############";
	map += L"#..............#";
	map += L"#..............#";
	map += L"################";


	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	// Game Loop
	while (1)
	{
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		// Controls
		// Handle CCW Rotation (CCW = negative angle change -> decrease angle)
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= 1.5f * fElapsedTime;
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += 1.5f * fElapsedTime;
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			fPlayerX += cosf(fPlayerA) * 4.f * fElapsedTime;
			fPlayerY += sinf(fPlayerA) * 4.f * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX -= cosf(fPlayerA) * 4.f * fElapsedTime;
				fPlayerY -= sinf(fPlayerA) * 4.f * fElapsedTime;
			}
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			fPlayerX -= cosf(fPlayerA) * 4.f * fElapsedTime;
			fPlayerY -= sinf(fPlayerA) * 4.f * fElapsedTime;
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX += cosf(fPlayerA) * 4.f * fElapsedTime;
				fPlayerY += sinf(fPlayerA) * 4.f * fElapsedTime;
			}
		}
		if (GetAsyncKeyState((unsigned short)'Q') & 0x8000)
		{
			fPlayerX += sinf(fPlayerA) * 4.f * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * 4.f * fElapsedTime;
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX -= sinf(fPlayerA) * 4.f * fElapsedTime;
				fPlayerY += cosf(fPlayerA) * 4.f * fElapsedTime;
			}
		}
		if (GetAsyncKeyState((unsigned short)'E') & 0x8000)
		{
			fPlayerX -= sinf(fPlayerA) * 4.f * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * 4.f * fElapsedTime;
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX += sinf(fPlayerA) * 4.f * fElapsedTime;
				fPlayerY -= cosf(fPlayerA) * 4.f * fElapsedTime;
			}
		}


		for (int x = 0; x < nScreenWidth; x++)
		{
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;
			float fDistanceToWall = 0;

			float fEyeX = cosf(fRayAngle);
			float fEyeY = sinf(fRayAngle);

			bool bHitWall = false;
			bool bBoundary = false;

			while (!bHitWall && fDistanceToWall < fDepth)
			{
				fDistanceToWall += 0.1f;

				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
				{
					bHitWall = true;
					fDistanceToWall = fDepth;
				}
				else
				{
					// Ray is inbounds so test if cell is a wall block
					if (map[nTestY * nMapWidth + nTestX] == L'#')
					{
						bHitWall = true;

						vector<pair<float, float>> p; // distance, dot product

						for (int tx = 0; tx < 2; tx++)
							for (int ty = 0; ty < 2; ty++)
							{
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);

								p.push_back(make_pair(d, dot));

							}
						
						// sort pairs from closest to farthest
						sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float> right) { return left.first < right.first; });
						
						float fBound = 0.006f;
						if (acos(p.at(0).second) < fBound)
							bBoundary = true;
						if (acos(p.at(1).second) < fBound)
							bBoundary = true;
						//if (acos(p.at(2).second) < fBound)
						//	bBoundary = true;

					}
				}
			}

			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			short nShade = ' ';

			if (fDistanceToWall <= fDepth / 4.0f)
				nShade = 0x2588;
			else if (fDistanceToWall <= fDepth / 3.0f)
				nShade = 0x2593;
			else if (fDistanceToWall <= fDepth / 2.0f)
				nShade = 0x2592;
			else if (fDistanceToWall <= fDepth / 1.0f)
				nShade = 0x2591;
			else
				nShade = ' ';

			if (bBoundary)
				nShade = ' ';

			for (int y = 0; y < nScreenHeight; y++)
			{
				if (y < nCeiling)
					screen[y * nScreenWidth + x] = ' ';
				else if (y < nFloor)
					screen[y * nScreenWidth + x] = nShade;
				else
				{
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)
						nShade = '#';
					else if (b < 0.5)
						nShade = 'x';
					else if (b < 0.75)
						nShade = '.';
					else if (b < 0.9)
						nShade = '-';
					else
						nShade = ' ';
					screen[y * nScreenWidth + x] = nShade;
				}
			}
		}

		// Display Stats
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f, FPS=%3.2f", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

		// Display Map
		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapHeight; ny++)
			{
				screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}

		screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = L'P';

		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}
	return 0;
}