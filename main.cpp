#include <iostream>
#include <vector>
#include <SDL.h>
#include<cstdlib>
#include <ctime>
#include <cmath>
#include <random>
#undef main
using namespace std;

default_random_engine generator;
normal_distribution<float> distribution(0.0f, 0.05f);


int basepixelenergy = 300;
int basefoodcapacity = 200;




// Coordanite Logic

struct floatCoordanite
{
	float x, y;

	friend ostream& operator<<(ostream& out, const floatCoordanite& coord) {
		out << "(" << coord.x << ", " << coord.y << ")";
		return out;
	}
};
struct IntCoordanite
{
	int x, y;

	friend ostream& operator<<(ostream& out, const IntCoordanite& coord) {
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

	Food(int xcoord, int ycoord, int remain) : x(xcoord), y(ycoord), remaining(remain) {}

	// Transformation and Rendering

	void Render(SDL_Renderer* renderer, floatCoordanite Camera, float cameraZoom) {
		rect = {
			(int)((x - Camera.x) * cameraZoom), // x
			(int)((y - Camera.y) * cameraZoom), // y
			(int)cameraZoom + 1, // w
			(int)cameraZoom + 1 // h
		};
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &rect);
	}
};

float sigmoid(float num, float spread) {
	return 1.0 / (1.0 + (exp(-num * spread)));
}
// Pixel Creature Class

class Pixel {
private:
	SDL_Rect rect;
	int randdir;
	float mindist;
	Food* closestFood;
	vector<vector<float>> weights1;
	vector<vector<float>> weights2;
	vector<float> biases = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	vector<float> midlayer = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	vector<float> output = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	// Checks if a move is valid and if so moves to the position and subtracts energy if a move goes to food add energy
	void checkmove(vector<Pixel> Pixels, vector<Food>& Foods, IntCoordanite targetcoord) {
		
		for (int PixelIndex = 0; PixelIndex < Pixels.size(); PixelIndex++) {
			if (Pixels[PixelIndex].x == targetcoord.x && Pixels[PixelIndex].y == targetcoord.y) {
				return;
			}
		}
		for (int FoodIndex = 0; FoodIndex < Foods.size(); FoodIndex++) {
			if (Foods[FoodIndex].x == targetcoord.x && Foods[FoodIndex].y == targetcoord.y) {
				if (Foods[FoodIndex].remaining > 0) {
					energy += 5;
					Foods[FoodIndex].remaining--;
				}
				return;
			}
		}
		nearestfood(Foods);
		x = targetcoord.x;
		y = targetcoord.y;
		energy--;
	}
	void nearestfood(vector<Food>& Foods) {
		mindist = numeric_limits<float>::max();
		closestFood = nullptr;

		for (auto& food : Foods) {
			float dist = std::sqrt(std::pow(food.x - x, 2) + std::pow(food.y - y, 2));
			if (dist < mindist) {
				mindist = dist;
				closestFood = &food;
			}
		}
	}
	vector<float> neural(vector<float> inputs) {

		midlayer = { biases[0], biases[1], biases[2], biases[3], biases[4], biases[5] };
		output = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		if (weights1.size() != 6 || weights1[0].size() != 5) {
			weights1.resize(6, vector<float>(5));
		}
		if (weights2.size() != 5 || weights2[0].size() != 6) {
			weights2.resize(5, vector<float>(6));
		}
		for (int a = 0; a < midlayer.size(); a++) {
			for (int b = 0; b < inputs.size(); b++) {
				midlayer[a] += (inputs[b] * weights1[a][b]);
			}
		}
		for (int a = 0; a < output.size(); a++) {
			for (int b = 0; b < midlayer.size(); b++) {
				output[a] += (midlayer[b] * weights2[a][b]);
			}
			output[a] = sigmoid(output[a], 1);
		}
		return output;
	}
	bool checkspawn(int randomintx, int randominty, vector<Pixel> Pixels, vector<Food> Foods) {
		for (int a = 0; a < Pixels.size(); a++) {
			if (Pixels[a].x == randomintx && Pixels[a].y == randominty) {
				return false;
			}
		}
		for (int a = 0; a < Foods.size(); a++) {
			if (Foods[a].x == randomintx && Foods[a].y == randominty) {
				return false;
			}
		}
		return true;
	}
public:
	int x;
	int y;
	int id;
	int indxmax;
	int energy;
	bool selection = true;
	vector<float> decisons;
	vector<float> inputs;
	SDL_Color Color = { 255, 255, 255, 255 };



	// Initialization Function

	Pixel(int xcoord, int ycoord, int energ) : x(xcoord), y(ycoord), energy(energ), id(getnewid()){
}

	void initializeweightsandbiases() {
		weights1.resize(6, vector<float>(5));
		weights2.resize(5, vector<float>(6));
		for (int a = 0; a < 6; a++) {
			biases[a] = ((static_cast<double>(rand()) / RAND_MAX) * 2.0) - 1.0;
			for (int b = 0; b < 5; b++) {
				weights1[a][b] = ((static_cast<double>(rand()) / RAND_MAX) * 2.0) - 1.0;
			}
		}
		for (int a = 0; a < 5; a++) {
			for (int b = 0; b < 6; b++) {
				weights2[a][b] = ((static_cast<double>(rand()) / RAND_MAX) * 2.0) - 1.0;
			}
		}
	}


	//Transform And Render to Screen

	void Render(SDL_Renderer* renderer, floatCoordanite Camera, float cameraZoom) {
		rect = {
			(int)((x - Camera.x) * cameraZoom), // x
			(int)((y - Camera.y) * cameraZoom), // y
			(int)cameraZoom + 1, // w
			(int)cameraZoom + 1 // h
		};
		SDL_SetRenderDrawColor(renderer, Color.r, Color.g, Color.b, Color.a);
		SDL_RenderFillRect(renderer, &rect);
	}
	void setweightsandbiases(vector<vector<float>> parentweights1, vector<vector<float>> parentweights2, vector<float> parentbiases) {
		weights1.resize(6, vector<float>(5));
		weights2.resize(5, vector<float>(6));
		int staysame = rand() % 2;
		for (size_t i = 0; i < parentweights1.size(); i++) {
			for (size_t j = 0; j < parentweights1[i].size(); j++) {
				if (staysame == 1) {
					weights1[i][j] = parentweights1[i][j] + distribution(generator);
				}
				else{
					weights1[i][j] = parentweights1[i][j];
				}
			}
		}
		for (size_t i = 0; i < parentweights2.size(); i++) {
			for (size_t j = 0; j < parentweights2[i].size(); j++) {
				if (staysame == 1) {
					weights2[i][j] = parentweights2[i][j] + distribution(generator);
				}
				else {
					weights2[i][j] = parentweights2[i][j];
				}
			}
		}
		for (size_t i = 0; i < parentbiases.size(); i++) {
			if (staysame == 1) {
				biases[i] = parentbiases[i] + distribution(generator);
			}
			else {
				biases[i] = parentbiases[i];
			}
		}
	}



	//Decisions

	void Update(vector<Pixel>& Pixels, vector<Food>& Foods) {
		energy--;
		decisons = {};
		inputs = {};
		if (closestFood == nullptr) {
			nearestfood(Foods);
		}
		if (closestFood != nullptr) {
			float xdiff = static_cast<double>((closestFood->x) - x);
			float ydiff = static_cast<double>((closestFood->y) - y);
			inputs.push_back(( - 1 / (max(0.3f * xdiff, 0.0f) + 1)) +1);
			inputs.push_back((1 / (min(0.3f * xdiff, 0.0f) - 1)) + 1);
			inputs.push_back((-1 / (max(0.3f * ydiff, 0.0f) + 1)) + 1);
			inputs.push_back((1 / (min(0.3f * ydiff, 0.0f) - 1)) + 1);
		}
		inputs.push_back((-1.0 / ((static_cast<double>(energy)*0.02) + 1.0)) + 1.0);
		//for (int g = 0; g < 5; g++) {
			//decisons.push_back(static_cast<double>(rand()) / RAND_MAX);
		//}
		decisons = neural(inputs);
		indxmax = distance(decisons.begin(), max_element(decisons.begin(), decisons.end()));
		if (indxmax == 1) {
			checkmove(Pixels, Foods, IntCoordanite{ x + 1, y });
		}
		else if (indxmax == 3) {
			checkmove(Pixels, Foods, IntCoordanite{ x - 1, y });
		}
		else if (indxmax == 2) {
			checkmove(Pixels, Foods, IntCoordanite{ x , y + 1 });
		}
		else if (indxmax == 0) {
			checkmove(Pixels, Foods, IntCoordanite{ x , y - 1 });
		}


		if (rand() % 5 == 0 && energy > basepixelenergy*1.2) {
			Reproduce(Pixels, Foods);
		}


	}



	// Reproduce Function

	void Reproduce(vector<Pixel>& Pixels, vector<Food>& Foods) {
		int rx = x + ((rand() % 40)-20);
		int ry = y + ((rand() % 40)-20);
		if (checkspawn(rx, ry, Pixels, Foods)) {
			Pixel newPixel(rx, ry, basepixelenergy);
			newPixel.setweightsandbiases(weights1, weights2, biases);
			Pixels.push_back(newPixel);
			energy -= basepixelenergy*1.1;
		}
	}
};



//Gets the Pixel ID of a specific coordanite

vector<int> getPixelId(vector<Pixel> Pixels, floatCoordanite mousePosition) {
	vector<int> out;
	for (int a = 0; a < Pixels.size(); a++) {
		if (Pixels[a].x == (int)mousePosition.x && Pixels[a].y == (int)mousePosition.y) {
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

void spawnrandom(vector<Pixel>& Pixels, vector<Food>& Foods) {
	int randomintx = (((rand() % 100) )*5)-100;
	int randominty = (((rand() % 100) )*5)-100;
	for (int a = 0; a < Pixels.size(); a++) {
		if (Pixels[a].x == randomintx && Pixels[a].y == randominty) {
			return;
		}
	}
	for (int a = 0; a < Foods.size(); a++) {
		if (Foods[a].x == randomintx && Foods[a].y == randominty) {
			return;
		}
	}
	Pixel newPixel(randomintx, randominty, basepixelenergy);
	newPixel.initializeweightsandbiases();
	Pixels.push_back(newPixel);
}

void spawnrandomFood(vector<Pixel>& Pixels, vector<Food>& Foods) {
	int randomintx = (((rand() % 100)) * 5) -100;
	int randominty = (((rand() % 100)) * 5) -100;
	for (int a = 0; a < Pixels.size(); a++) {
		if (Pixels[a].x == randomintx && Pixels[a].y == randominty) {
			return;
		}
	}
	for (int a = 0; a < Foods.size(); a++) {
		if (Foods[a].x == randomintx && Foods[a].y == randominty) {
			return;
		}
	}
	Food newFood(randomintx, randominty, basefoodcapacity);
	Foods.push_back(newFood);
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
	int frameLimit = 120;

	int updateEvery = 10;


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


	int population_barrier = 25;
	int foodbarrier = 500;


	floatCoordanite Camera = { 0.0f, 0.0f };
	floatCoordanite initialMouseCoord;
	floatCoordanite mousePos = { 0.0f, 0.0f };
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
	vector<Food> newFoods;
	SDL_Rect neurondisplayrect;
	neurondisplayrect.w = 60;
	neurondisplayrect.h = 20;
	neurondisplayrect.x = 20;


	//Generates Pixel objects and their attributes

	for (int xy = 0; xy < 2500; xy++) {
		if (rand() % 40 == 0) {
			Pixel genpix(((xy % 50) * 5), (floor(xy / 50) * 5), basepixelenergy);
			genpix.initializeweightsandbiases();
			Pixels.push_back(genpix);
		}
	}


	//Generates Food objects and their attributes

	for (int xy = 0; xy < 10000; xy++) {
		if (rand() % 50 == 0) {
			Food genfood(((xy % 100) * 5)-100, (floor(xy / 100) * 5)-100, basefoodcapacity);
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
				mousePos = { (float)e.motion.x, (float)e.motion.y };
				int mouseX, mouseY;
				Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
				if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
					Camera = {Camera.x - (mousePos.x - initialMouseCoord.x)* (1 / Zoom), Camera.y - (mousePos.y - initialMouseCoord.y) * (1 / Zoom)};
				}
				break;
			}

								// Mouse Scroll Detection
			case SDL_MOUSEWHEEL:

				//On mouse scroll update Camera Position and zoom
				Zoom += (e.wheel.preciseY / 6.0);
				if (Zoom < 0.5) {Zoom = 0.5;}
				else {Camera = { (float)(Camera.x + mousePos.x / (Zoom - (e.wheel.preciseY / 6.0))) - (mousePos.x / Zoom), (float)(Camera.y + mousePos.y / (Zoom - (e.wheel.preciseY / 6.0))) - (mousePos.y / Zoom) };}
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
				switch (e.button.button)
				{
				case SDL_BUTTON_LEFT:

					//Check to see if mouse moved more than 2 pixels during the period it was held down
					if (abs(mousePos.x - mouseDownPos.x) < 2 && abs(mousePos.y - mouseDownPos.y) < 2) {

						//Figures out the position of the mouse on the grid and finds the pixels on that position
						mousePosTransformed = { floor((mousePos.x / Zoom) + Camera.x), floor((mousePos.y / Zoom) + Camera.y) };
						selectedPixels = getPixelId(Pixels, mousePosTransformed);

						//Removes Color from previously selected Pixels
						for (int g = 0; g < Selectedpix.size(); g++) {
							curpixind = indexFromId(Selectedpix[g], Pixels);
							if (curpixind != -1) {Pixels[curpixind].Color = { 255, 255, 255, 255 };}
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

		if (Pixels.size() < population_barrier) {
			for (int d = 0; d < (population_barrier - Pixels.size()); d++) {
				spawnrandom(Pixels, Foods);
			}
		}
		if (Foods.size() < foodbarrier) {
			for (int d = 0; d < (foodbarrier - Foods.size()); d++) {
				spawnrandomFood(Pixels, Foods);
			}
		}

		//Deletes Pixels with 0 Energy
		newPixels = Pixels;
		for (int Pixelindex = Pixels.size() - 1; Pixelindex >= 0; Pixelindex--) {
			if (Pixels[Pixelindex].energy <= 0) {newPixels.erase(newPixels.begin() + Pixelindex);}
		}
		Pixels = newPixels;

		//Deletes Food with 0 remaining
		newFoods = Foods;
		for (int Pixelindex = Foods.size() - 1; Pixelindex >= 0; Pixelindex--) {
			if (Foods[Pixelindex].remaining <= 0) {newFoods.erase(newFoods.begin() + Pixelindex);}
		}
		Foods = newFoods;

		//Loop that updates and renders each pixel
		for (int Pixelindex = 0; Pixelindex < Pixels.size(); Pixelindex++) {
			if (frameCount % updateEvery == 0) {
				Pixels[Pixelindex].Update(Pixels, Foods);
			}
			Pixels[Pixelindex].Render(renderer, Camera, Zoom);
		}

		//Loop that renders each food object
		for (int Foodindex = 0; Foodindex < Foods.size(); Foodindex++) {
			Foods[Foodindex].Render(renderer, Camera, Zoom);
		}
		if (selectedPixels.size() > 0) {
			if (indexFromId(selectedPixels[0], Pixels) != -1) {
				selectedPixel = Pixels[indexFromId(selectedPixels[0], Pixels)];

				for (int f = 0; f < selectedPixel.decisons.size(); f++) {
					SDL_SetRenderDrawColor(renderer, (selectedPixel.decisons[f] * selectedPixel.decisons[f]) * 255, (selectedPixel.decisons[f]* selectedPixel.decisons[f]) * 255, (selectedPixel.decisons[f] * selectedPixel.decisons[f]) * 255, 255);
					neurondisplayrect.y = (f * 25) + 20;
					SDL_RenderFillRect(renderer, &neurondisplayrect);
				}
				neurondisplayrect.x = 20;

				for (int f = 0; f < selectedPixel.inputs.size(); f++) {
					SDL_SetRenderDrawColor(renderer, selectedPixel.inputs[f] * 255, selectedPixel.inputs[f] * 255, selectedPixel.inputs[f] * 255, 255);
					neurondisplayrect.y = (f * 25) + 20;
					SDL_RenderFillRect(renderer, &neurondisplayrect);
				}
				neurondisplayrect = {
					65,
					((selectedPixel.indxmax) * 25) + 25,
					10,
					10
				};
				SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
				SDL_RenderFillRect(renderer, &neurondisplayrect);
				neurondisplayrect = {
					60,
					neurondisplayrect.y,
					20,
					20
				};
			}
		}

		// Render the renderer
		SDL_RenderPresent(renderer);

		//FPS Calculation and Console output calculations and logic
		if (frameCount % (frameLimit / 4) == 0) {
			FPS = (1000.0 * (frameLimit / 4.0)) / (SDL_GetTicks() - lastFPSFrame);
			system("cls");
			cout << "FPS: " << FPS << "    Total Pixels: " << Pixels.size() << endl;
			if (selectedPixels.size() != 0) { cout << "SELECTED PIXEL/S:" << endl; }
			for (int selectedpixelindex = 0; selectedpixelindex < selectedPixels.size(); selectedpixelindex++) {
				if (indexFromId(selectedPixels[selectedpixelindex], Pixels) != -1) {
					selectedPixel = Pixels[indexFromId(selectedPixels[selectedpixelindex], Pixels)];

					cout << "X: " << selectedPixel.x << "   Y: " << selectedPixel.y << "   Id: " << selectedPixel.id << "   Energy: " << selectedPixel.energy << "  " << indexFromId(selectedPixels[selectedpixelindex], Pixels) << endl;
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
