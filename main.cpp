#include <iostream>
#include <vector>
#include <SDL.h>
#include<cstdlib>
#include <ctime>
#include <cmath>
#undef main
using namespace std;


// Coordanite Logic
struct floatCoordanite
{
	float x,y;

	friend ostream& operator<<(ostream& out, const floatCoordanite& coord) {
		out << "(" << coord.x << ", " << coord.y << ")";
		return out;
	}
};
int idnum = 0;

int getnewid() {
	idnum++;
	return idnum - 1;
}
// Pixel Creature Class
class Pixel {

	public:
		int x;
		int y;
		int id;
		int energy;
		bool selection = true;
		SDL_Color Color = { 255, 255, 255, 255 };
		SDL_Rect rect;

		// Initialization Function
		Pixel(int xcoord, int ycoord, int energ) {
			x = xcoord;
			y = ycoord;
			energy = energ;
			id = getnewid();
		}

		//Transform And Render to Screen
		void Render(SDL_Renderer* renderer, floatCoordanite Camera, float cameraZoom) {
			SDL_SetRenderDrawColor(renderer, Color.r, Color.g, Color.b, Color.a);
			rect.w = cameraZoom+1;
			rect.h = cameraZoom+1;
			rect.x = (x - Camera.x)*cameraZoom;
			rect.y = (y - Camera.y)*cameraZoom;
			SDL_RenderFillRect(renderer, &rect);
		}
		void Update(vector<Pixel>& Pixels) {
			
			//Decisions

			int randdir = rand() % 8;
			if (randdir == 0) {
				x++;
				energy -= 1;
			}
			else if (randdir == 1) {
				x--;
				energy -= 1;
			}
			else if (randdir == 2) {
				y--;
				energy -= 1;
			}
			else if (randdir == 3) {
				y++;
				energy -= 1;
			}

			
			if (rand() % 35 == 0) {
				Reproduce(Pixels);
			}
			

		}
		void Reproduce(vector<Pixel>& Pixels) {
			Pixel newPixel(x + 1-(2*(rand()%2)), y, 15);
			Pixels.push_back(newPixel);
		}
};

vector<int> getPixelId(vector<Pixel> Pixels, floatCoordanite mousePosition) {
	vector<int> out;
	for (int a = 0; a < Pixels.size(); a++) {
		if (Pixels[a].x == mousePosition.x && Pixels[a].y == mousePosition.y) {
			out.push_back(Pixels[a].id);
		}
	}
	return out;
}

int indexFromId(int id1, vector<Pixel> Pixels) {
	//cout << Pixels[0].id << endl;
	for (int j = 0; j < Pixels.size(); j++) {
		if (Pixels[j].id == id1) {
			//cout << Pixels[j].id << "  " << id1 << "   " << j << endl;
			return j;
		}
	}
	return -1;
}

int main() {


	//Initialize SDL and check for Errors

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		cout << "SDL_Init Error: " << SDL_GetError() << endl;
		return 1;
	}


	// Create a Display Bounds Rectangle and check for errors
	SDL_Rect displayBounds;
	if (SDL_GetDisplayBounds(0, &displayBounds) != 0) {
		std::cerr << "SDL_GetDisplayBounds failed: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	// Set the window scrren height and width to that of the display bounds
	int screen_width = displayBounds.w;
	int screen_height = displayBounds.h - 60;


	// Settings
	bool frameLimitOn = true;
	int frameLimit = 100;

	int updateEvery = 20;


	// Create Window and check for errors
	SDL_Window* window = SDL_CreateWindow("help", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (window == nullptr) {
		cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
		SDL_Quit();
		return 1;
	}


	// Create renderer and check for errors
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == nullptr) {
		cout << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}


	// Create event listener
	SDL_Event e;

	// Boolean to run the game loop
	bool gameloop = true;

	// Initialize all of the required variables
	unsigned long frameCount = 0;
	unsigned long lastFPSFrame = 0;
	long long lastFPSUnixTime = SDL_GetTicks();
	long long currentFrameTime = SDL_GetTicks();
	long long frameTime;
	int frameDelay = 1000.0 / frameLimit;

	floatCoordanite Camera={ 0.0f, 0.0f };
	floatCoordanite initialMouseCoord;
	floatCoordanite mousePos ={ 0.0f, 0.0f };
	floatCoordanite mouseDownPos;
	floatCoordanite mousePosTransformed;
	vector<Pixel> Pixels;
	vector<Pixel> newPixels;
	int curpixind;
	vector<int> Selectedpix = {};
	float Zoom = 1.0f;
	vector<int> selectedPixels = {};
	Pixel selectedPixel(0, 0, 0);
	selectedPixel.selection = false;
	int FPS = 0;
	srand(time(NULL));


	for (int xy = 0; xy < 2500; xy++) {
		if (rand() % 6 == 0) {
			Pixel genpix((xy % 50)*5, floor(xy / 50)*5, 15);
			Pixels.push_back(genpix);
		}
	}

	// Main Game Loop
	while (gameloop) {

		// Event Handler
		while (SDL_PollEvent(&e)) {
			switch (e.type) {


				// Break out of loop if Quit
				case SDL_QUIT:
					gameloop = false;
					break;

				// Key Detection
				case SDL_KEYDOWN:
					switch (e.key.keysym.sym) {


						// If escape pressed break out of loop and quit
						case SDLK_ESCAPE:
							gameloop = false;
							break;

					}
					break;

				// Mouse Motion Detection
				case SDL_MOUSEMOTION: {

					// Gets change in mouse postition and if held, Moves the camera by that amount
					initialMouseCoord = mousePos;
					mousePos.x = e.motion.x;
					mousePos.y = e.motion.y;
					int mouseX, mouseY;
					Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
					if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
						Camera.x -= (mousePos.x - initialMouseCoord.x) *(1/Zoom);
						Camera.y -= (mousePos.y - initialMouseCoord.y) * (1 / Zoom);
					}
					break;
				}
				case SDL_MOUSEWHEEL:
					Zoom += (e.wheel.preciseY / 8.0);
					if (Zoom < 1.0) {
						Zoom = 1.0;
					}
					Camera.x = (Camera.x + mousePos.x / (Zoom - (e.wheel.preciseY / 8.0))) - (mousePos.x / Zoom);
					Camera.y = (Camera.y + mousePos.y / (Zoom - (e.wheel.preciseY / 8.0))) - (mousePos.y / Zoom);
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (e.button.button == SDL_BUTTON_LEFT) {
						mouseDownPos = mousePos;
					}
					break;
				case SDL_MOUSEBUTTONUP:
					if (e.button.button == SDL_BUTTON_LEFT) {
						if (abs(mousePos.x - mouseDownPos.x) < 2 && abs(mousePos.y - mouseDownPos.y) < 2) {
							mousePosTransformed.x = floor((mousePos.x / Zoom) + Camera.x);
							mousePosTransformed.y = floor((mousePos.y / Zoom) + Camera.y);
							selectedPixels = getPixelId(Pixels, mousePosTransformed);
							if (selectedPixels.size() != 0) { cout << "SELECTED PIXEL/S:" << endl; }
							for (int g = 0; g < Selectedpix.size(); g++) {
								curpixind = indexFromId(Selectedpix[g], Pixels);
								if (curpixind != -1) {
									Pixels[curpixind].Color = { 255, 255, 255, 255 };
								}
							}
							Selectedpix = {};
							selectedPixel.selection = false;
							Selectedpix = {};
							for (int selectedpixelindex = 0; selectedpixelindex < selectedPixels.size(); selectedpixelindex++) {
								curpixind = indexFromId(selectedPixels[selectedpixelindex], Pixels);
								selectedPixel = Pixels[curpixind];
								Pixels[curpixind].Color = { 255, 0, 0, 255 };
								Selectedpix.push_back(selectedPixels[selectedpixelindex]);
							}
						}
					}
					break;
			}
		}

		// Clears The renderer and increments the frame counter

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
		frameCount++;

		newPixels = Pixels;
		for (int Pixelindex = Pixels.size() - 1; Pixelindex >= 0; Pixelindex--) {
			if (Pixels[Pixelindex].energy == 0) {
				newPixels.erase(newPixels.begin() + Pixelindex);
				continue;
			}
		}
		Pixels = newPixels;
		for (int Pixelindex = 0; Pixelindex < Pixels.size(); Pixelindex++) {
			if (frameCount % updateEvery == -1) {
				Pixels[Pixelindex].Update(Pixels);
			}
			Pixels[Pixelindex].Render(renderer, Camera, Zoom);
		}

		

		// Render the renderer
		SDL_RenderPresent(renderer);
		if (frameCount % (frameLimit/4) == 0) {
			FPS = (1000 * (frameLimit/4)) / (SDL_GetTicks() - lastFPSFrame);
			system("cls");
			cout << "FPS: " << FPS << endl;
			if (selectedPixels.size() != 0) { cout << "SELECTED PIXEL/S:" << endl; }
			for (int selectedpixelindex = 0; selectedpixelindex < selectedPixels.size(); selectedpixelindex++) {
				if (indexFromId(selectedPixels[selectedpixelindex], Pixels) != -1) {
					selectedPixel = Pixels[indexFromId(selectedPixels[selectedpixelindex], Pixels)];

					cout << "X: " << selectedPixel.x << "   Y: " << selectedPixel.y << "   Id: " << selectedPixel.id << "   Energy: " << selectedPixel.energy << "  " << indexFromId(selectedPixels[selectedpixelindex], Pixels) <<  endl;
				}
			}
			lastFPSFrame = SDL_GetTicks();
		}
		// FPS limit logic
		if (frameLimitOn) {
			frameTime = SDL_GetTicks() - currentFrameTime;
			if (frameTime < frameDelay) {
				SDL_Delay(frameDelay - frameTime);
			}
			currentFrameTime = SDL_GetTicks();
		}
	}

	// Quit the window
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
