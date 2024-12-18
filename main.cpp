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




// Unique ID assigning

int idnum = 0;

int getnewid() {
	idnum++;
	return idnum - 1;
}



// Pixel Creature Class

class Pixel {
	private:
		SDL_Rect rect;
	public:
		int x;
		int y;
		int id;
		int energy;
		bool selection = true;
		SDL_Color Color = { 255, 255, 255, 255 };



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



		//Decisions

		void Update(vector<Pixel>& Pixels) {

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



		// Reproduce Function

		void Reproduce(vector<Pixel>& Pixels) {
			Pixel newPixel(x + 1-(2*(rand()%2)), y, 15);
			Pixels.push_back(newPixel);
		}
};



// Food Class

class Food {
	private:
		SDL_Rect rect;
	public:
		int x;
		int y;
		int remaining;
		int fid;


		// Initialization Function

		Food(int xcoord, int ycoord, int remain) {
			x = xcoord;
			y = ycoord;
			remaining = remain;
		}
		
		// Transformation and Rendering

		void Render(SDL_Renderer* renderer, floatCoordanite Camera, float cameraZoom) {
			SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
			rect.w = cameraZoom + 1;
			rect.h = cameraZoom + 1;
			rect.x = (x - Camera.x) * cameraZoom;
			rect.y = (y - Camera.y) * cameraZoom;
			SDL_RenderFillRect(renderer, &rect);
		}


};



//Gets the Pixel ID of a specific coordanite

vector<int> getPixelId(vector<Pixel> Pixels, floatCoordanite mousePosition) {
	vector<int> out;
	for (int a = 0; a < Pixels.size(); a++) {
		if (Pixels[a].x == mousePosition.x && Pixels[a].y == mousePosition.y) {
			out.push_back(Pixels[a].id);
		}
	}
	return out;
}



//Converts the ID of a Pixel into its index in the Pixel list

int indexFromId(int id1, vector<Pixel> Pixels) {
	for (int j = 0; j < Pixels.size(); j++) {
		if (Pixels[j].id == id1) {
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
	vector<Food> Foods;


	//Generates Pixel objects and their attributes 

	for (int xy = 0; xy < 2500; xy++) {
		if (rand() % 6 == 0) {
			Pixel genpix((xy % 50)*5, floor(xy / 50)*5, 15);
			Pixels.push_back(genpix);
		}
	}


	//Generates Food objects and their attributes

	for (int xy = 0; xy < 250000; xy++) {
		if (rand() % 20 == 0) {
			Food genfood(((xy % 500) * 5)-1250, (floor(xy / 500) * 5)-1250, 5);
			Foods.push_back(genfood);
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

				// Mouse Scroll Detection
				case SDL_MOUSEWHEEL:
					
					//On mouse scroll update Camera Position and zoom
					Zoom += (e.wheel.preciseY / 8.0);
					if (Zoom < 1.0) {
						Zoom = 1.0;
					}
					else {
						Camera.x = (Camera.x + mousePos.x / (Zoom - (e.wheel.preciseY / 8.0))) - (mousePos.x / Zoom);
						Camera.y = (Camera.y + mousePos.y / (Zoom - (e.wheel.preciseY / 8.0))) - (mousePos.y / Zoom);
					}
					break;

				//Mouse Button Down Detection
				case SDL_MOUSEBUTTONDOWN:

					// LMB Detection
					if (e.button.button == SDL_BUTTON_LEFT) {

						//On LMB record mouse position
						mouseDownPos = mousePos;
					}
					break;

				//Mouse Button Up Detection
				case SDL_MOUSEBUTTONUP:

					// LMB Detection
					if (e.button.button == SDL_BUTTON_LEFT) {

						//Check to see if mouse moved more than 2 pixels during the period it was held down
						if (abs(mousePos.x - mouseDownPos.x) < 2 && abs(mousePos.y - mouseDownPos.y) < 2) {

							//Figures out the position of the mouse on the grid and finds the pixels on that position
							mousePosTransformed.x = floor((mousePos.x / Zoom) + Camera.x);
							mousePosTransformed.y = floor((mousePos.y / Zoom) + Camera.y);
							selectedPixels = getPixelId(Pixels, mousePosTransformed);

							//Removes Color from previously selected Pixels
							for (int g = 0; g < Selectedpix.size(); g++) {
								curpixind = indexFromId(Selectedpix[g], Pixels);
								if (curpixind != -1) {
									Pixels[curpixind].Color = { 255, 255, 255, 255 };
								}
							}

							//Sets color of selected pixel and records it
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

		//Deletes Pixels with 0 Energy
		newPixels = Pixels;
		for (int Pixelindex = Pixels.size() - 1; Pixelindex >= 0; Pixelindex--) {
			if (Pixels[Pixelindex].energy == 0) {
				newPixels.erase(newPixels.begin() + Pixelindex);
				continue;
			}
		}
		Pixels = newPixels;


		//Loop that updates and renders each pixel
		for (int Pixelindex = 0; Pixelindex < Pixels.size(); Pixelindex++) {
			if (frameCount % updateEvery == 0) {
				Pixels[Pixelindex].Update(Pixels);
			}
			Pixels[Pixelindex].Render(renderer, Camera, Zoom);
		}

		//Loop that renders each food object
		for (int Foodindex = 0; Foodindex < Foods.size(); Foodindex++) {
			Foods[Foodindex].Render(renderer, Camera, Zoom);
		}

		

		// Render the renderer
		SDL_RenderPresent(renderer);

		//FPS Calculation and Console output calculations and logic
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
