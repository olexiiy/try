#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
#include <thread>
#include <cmath>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <deque>

using namespace std;

int nScreenWidth = 120;			// Console Screen Size X (columns)
int nScreenHeight = 40;			// Console Screen Size Y (rows)
int nMapWidth = 16;				// World Dimensions
int nMapHeight = 16;

float fPlayerX = 14.7f;			// Player Start Position
float fPlayerY = 5.09f;
float fPlayerA = 0.0f;			// Player Start Rotation
float fFOV = 3.14159f / 4.0f;	// Field of View
float fDepth = 16.0f;			// Maximum rendering distance
float fSpeed = 5.0f;			// Walking Speed
float fElapsedTime;       // Time elapsed since last frame

wstring map; //The world map

std::string moveCursor(int x, int y) { // Move console cursor to point (x,y)
	return "\033[" + std::to_string(y + 1) + ";" + std::to_string(x) + "H";
}

char getch(void) { //yoinked from stackexchange 
   char buf = 0;
   struct termios old = {0};
   fflush(stdout);
   if(tcgetattr(0, &old) < 0)
       perror("tcsetattr()");
   old.c_lflag &= ~ICANON;
   old.c_lflag &= ~ECHO;
   old.c_cc[VMIN] = 1;
   old.c_cc[VTIME] = 0;
   if(tcsetattr(0, TCSANOW, &old) < 0)
       perror("tcsetattr ICANON");
   if(read(0, &buf, 1) < 0)
       perror("read()");
   old.c_lflag |= ICANON;
   old.c_lflag |= ECHO;
   if(tcsetattr(0, TCSADRAIN, &old) < 0)
       perror("tcsetattr ~ICANON");
   return buf;
}

char cActualKeyPressed = 0;

std::deque<char> cKeysPressed;

void doInput() { //get input from input stream
	while (1)
	{
		cKeysPressed.push_back(getch());
	}
}

void processInput() {
	cActualKeyPressed = cKeysPressed.front();

	if ( cActualKeyPressed == 'W' || cActualKeyPressed == 'A' || cActualKeyPressed == 'S' || cActualKeyPressed == 'D' ) fSpeed = 10.0f;
	else fSpeed = 5.0f;

	// Handle CCW Rotation
	if (cActualKeyPressed == ',' || cActualKeyPressed == '<')
		fPlayerA -= (fSpeed * 0.75f) * fElapsedTime;
	// Handle CW Rotation
	if (cActualKeyPressed == '.' || cActualKeyPressed == '>')
		fPlayerA += (fSpeed * 0.75f) * fElapsedTime;
	
	// Handle Forwards movement & collision
	if (cActualKeyPressed == 'w' || cActualKeyPressed == 'W')
	{
		fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;
		fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;
		if (map.at((int)fPlayerX * nMapWidth + (int)fPlayerY) == '#')
		{
			fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;
		}			
	}
	// Handle backwards movement & collision
	if (cActualKeyPressed == 's' || cActualKeyPressed == 'S')
	{
		fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
		fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;
		if (map.at((int)fPlayerX * nMapWidth + (int)fPlayerY) == '#')
		{
			fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;
		}
	}
	// Handle left strafe movement and collision
	if (cActualKeyPressed == 'a' || cActualKeyPressed == 'A') {
		fPlayerX -= cosf(fPlayerA) * fSpeed * fElapsedTime;
		fPlayerY += sinf(fPlayerA) * fSpeed * fElapsedTime;
		if (map.at((int)fPlayerX * nMapWidth + (int)fPlayerY) == '#')
		{
			fPlayerX += cosf(fPlayerA) * fSpeed * fElapsedTime;
			fPlayerY -= sinf(fPlayerA) * fSpeed * fElapsedTime;
		}	
	}
	// Handle right strafe movement and collision
	if (cActualKeyPressed == 'd' || cActualKeyPressed == 'D') {
		fPlayerX += cosf(fPlayerA) * fSpeed * fElapsedTime;
		fPlayerY -= sinf(fPlayerA) * fSpeed * fElapsedTime;
		if (map.at((int)fPlayerX * nMapWidth + (int)fPlayerY) == '#')
		{
			fPlayerX -= cosf(fPlayerA) * fSpeed * fElapsedTime;
			fPlayerY += sinf(fPlayerA) * fSpeed * fElapsedTime;
		}	
	}
}

int main() {
	// Get size of console and set nScreenHeight and nScreenWidth accordingly
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	nScreenWidth = w.ws_col;
	nScreenHeight = w.ws_row;
	
	nScreenHeight--;

	std::thread inputThread(doInput);

	// Create Screen Buffer
	wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
	int dwBytesWritten = 0;

	// Create Map of world space # = wall block, . = space
	map += L"#########.......";
	map += L"#...............";
	map += L"#.......########";
	map += L"#..............#";
	map += L"#......##......#";
	map += L"#......##......#";
	map += L"#..............#";
	map += L"###............#";
	map += L"##.............#";
	map += L"#......####..###";
	map += L"#......#.......#";
	map += L"#......#.......#";
	map += L"#..............#";
	map += L"#......#########";
	map += L"#..............#";
	map += L"################";

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();
	
	std::cout << "\033[2J"; //clear compiler messages from console before program start
	std::cout << "\033[?25l"; //hide cursor so it doesn't spasm all over the place all the time

	while (1)
	{

		// We'll need time differential per frame to calculate modification
		// to movement speeds, to ensure consistant movement, as ray-tracing
		// is non-deterministic
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		fElapsedTime = elapsedTime.count();

		//for each key in the deque, call processInput() to act accordingly
		while (cKeysPressed.size() > 0) {
			try { processInput();
			} catch (std::out_of_range){}
			cKeysPressed.pop_front();
		}

		for (int x = 0; x < nScreenWidth; x++)
		{
			// For each column, calculate the projected ray angle into world space
			float fRayAngle = (fPlayerA - fFOV/2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			// Find distance to wall
			float fStepSize = 0.1f;		  // Increment size for ray casting, decrease to increase										
			float fDistanceToWall = 0.0f; //                                      resolution

			bool bHitWall = false;		// Set when ray hits wall block
			bool bBoundary = false;		// Set when ray hits boundary between two wall blocks

			float fEyeX = sinf(fRayAngle); // Unit vector for ray in player space
			float fEyeY = cosf(fRayAngle);

			// Incrementally cast ray from player, along ray angle, testing for 
			// intersection with a block
			while (!bHitWall && fDistanceToWall < fDepth)
			{
				fDistanceToWall += fStepSize;
				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);
				
				// Test if ray is out of bounds
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
				{
					bHitWall = true;			// Just set distance to maximum depth
					fDistanceToWall = fDepth;
				}
				else
				{
					// Ray is inbounds so test to see if the ray cell is a wall block
					if (map.c_str()[nTestX * nMapWidth + nTestY] == '#')
					{
						// Ray has hit wall
						bHitWall = true;

						// To highlight tile boundaries, cast a ray from each corner
						// of the tile, to the player. The more coincident this ray
						// is to the rendering ray, the closer we are to a tile 
						// boundary, which we'll shade to add detail to the walls
						vector<pair<float, float>> p;

						// Test each corner of hit tile, storing the distance from
						// the player, and the calculated dot product of the two rays
						for (int tx = 0; tx < 2; tx++)
							for (int ty = 0; ty < 2; ty++)
							{
								// Angle of corner to eye
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx*vx + vy*vy); 
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(make_pair(d, dot));
							}

						// Sort Pairs from closest to farthest
						sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right) {return left.first < right.first; });
						
						// First two/three are closest (we will never see all four)
						float fBound = 0.01;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
						if (acos(p.at(2).second) < fBound) bBoundary = true;
					}
				}
			}
		
			// Calculate distance to ceiling and floor
			int nCeiling = (float)(nScreenHeight/2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			// Shader walls based on distance
			short nShade = ' ';
			if (fDistanceToWall <= fDepth / 4.0f)			nShade = 2588;	// Very close	
			else if (fDistanceToWall < fDepth / 3.0f)		nShade = 2593;
			else if (fDistanceToWall < fDepth / 2.0f)		nShade = 2592;
			else if (fDistanceToWall < fDepth)				nShade = 2591;
			else											nShade = ' ';		// Too far away

			if (bBoundary)		nShade = ' '; // Black it out
			
			for (int y = 0; y < nScreenHeight; y++)
			{
				// Each Row
				if(y <= nCeiling)
					screen[y*nScreenWidth + x] = ' ';
				else if(y > nCeiling && y <= nFloor)
					screen[y*nScreenWidth + x] = nShade;
				else // Floor
				{				
					// Shade floor based on distance
					float b = 1.0f - (((float)y -nScreenHeight/2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)		nShade = '#';
					else if (b < 0.5)	nShade = 'x';
					else if (b < 0.75)	nShade = '.';
					else if (b < 0.9)	nShade = '-';
					else				nShade = ' ';
					screen[y*nScreenWidth + x] = nShade;
				}
			}
		}

		// Display Stats
		std::cout << moveCursor(0, 0) << "X=" << fPlayerX << " Y=" << fPlayerY << " A=" << fPlayerA << "FPS=" << 1.0f/fElapsedTime;

		// Display Map
		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapWidth; ny++)
			{
				screen[(ny+1)*nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		screen[((int)fPlayerX+1) * nScreenWidth + (int)fPlayerY] = 'P';

		// Display Frame
		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		std::cout << moveCursor(0,0);
		for (int i = 0; i < nScreenWidth * nScreenHeight; i++) {
			switch (screen[i]) {
				case 2588: 
					std::cout << "\u2588";
				break;

				case 2593:
					std::cout << "\u2593";
				break;

				case 2592:
					std::cout << "\u2592";
				break;

				case 2591:
					std::cout << "\u2591";
				break;

				default:
					std::wcout << screen[i];
				break;

			}
			if (i % nScreenWidth == 0) {
				std::cout << "\n";
			}
		}	
	}

	return 0;
}

// That's It!! - Jx9
