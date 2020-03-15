// ReSharper disable CppClangTidyCppcoreguidelinesProTypeMemberInit
#include "SDL.h"
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <omp.h>
#include <chrono>
#include <sstream>

#pragma pack(2)
struct BMPHeader
{
	unsigned short bfType; //file type
	unsigned long bfSize; //size of bitmap file in in bytes
	unsigned short bfReserved1; //reserved; must be 0
	unsigned short bfReserved2; //reserved; must be 0
	unsigned long bfOffBits; //offset from bitmap header to bitmap data bits
};

#pragma pack(2)
struct BMPInfoHeader
{
	unsigned long biSize; //number of bytes required by the struct
	long biWidth; //the width in pixels
	long biHeight; //the height in pixels
	unsigned short biPlanes; //specifies the number of color planes, must be a 1
	unsigned short biBitCount; //specifies the number of bits per pixel
	unsigned long biCompression; //type of compression
	unsigned long biSizeImage; //size of image in bytes
	long biXPixelsPerMeter; //number of pixels per meter on x axis
	long biYPixelsPerMeter; //number of pixels per meter on y axis
	unsigned long biClrUsed; //number of colors used by the bitmap
	unsigned long biClrImportant; //number of colors that are important
};

struct Color
{
	unsigned char r;
	unsigned char g;
	unsigned char b;

	Color()
	{
		r = 0;
		g = 0;
		b = 0;
	};
	
	Color(unsigned char red, unsigned char green, unsigned char blue)
	{
		r = red;
		g = green;
		b = blue;
	}
};

const unsigned int width = 1280;
const unsigned int height = 720;
const auto images = new Color [width * height];

void writeLogFile(const std::string& text)
{
	std::ofstream log_file(
		"log_file.txt", std::ios_base::out | std::ios_base::app);
	log_file << text;
	log_file.close();
}

void saveImage(const char* filePath, Color* image, unsigned int width, unsigned int height)
{
	std::ofstream fout;
	fout.open(filePath, std::ios::binary);

	BMPHeader header;
	BMPInfoHeader infoHeader;

	header.bfType = 0x4D42; //translated: BM (BitMap)
	header.bfSize = sizeof(BMPHeader) + sizeof(BMPInfoHeader) + 3*width*height; //1 Bytes for each color in the pixel
	header.bfOffBits = sizeof(BMPHeader) + sizeof(BMPInfoHeader);
	header.bfReserved1 = 0;
	header.bfReserved2 = 0;


	infoHeader.biBitCount = 24; //3 channels each 8 Byte
	infoHeader.biClrImportant = 0;
	infoHeader.biClrUsed = 0; 
	infoHeader.biCompression = BI_RGB; //BI_RGB
	infoHeader.biHeight = height;
	infoHeader.biWidth = width;
	infoHeader.biPlanes = 1;
	infoHeader.biSize = sizeof(BMPInfoHeader);
	infoHeader.biSizeImage = header.bfSize;
	infoHeader.biXPixelsPerMeter = 2000;
	infoHeader.biYPixelsPerMeter = 2000;


	fout.write((char*)&header, sizeof(BMPHeader));
	fout.write((char*)&infoHeader, sizeof(BMPInfoHeader));


	for (int i = 0; i < width*height; i++)
	{
		unsigned char r, g, b;
		Color c = image[i];

		fout.write((char*)&c.b, sizeof(unsigned char));
		fout.write((char*)&c.g, sizeof(unsigned char));
		fout.write((char*)&c.r, sizeof(unsigned char));
	}
	fout.close(); 
}

Color calculatePixel(const unsigned int x, const unsigned int y, const double rangeX, const double rangeY, const double middleX, const double middleY, const int maxIteration)
{
	double x0 = static_cast<double>(x) / static_cast<double>(width);
	double y0 = static_cast<double>(y) / static_cast<double>(height);

	x0 = x0 * rangeX + middleX - rangeX / 2;
	y0 = y0 * rangeY + middleY - rangeY / 2;

	double real = 0;
	double imaginary = 0;

	for (int i = 0; i < maxIteration; i++)
	{
		const double tempReal = real * real - imaginary * imaginary + x0;
		imaginary = 2 * real * imaginary + y0;
		real = tempReal;
		if (real * real + imaginary * imaginary > 4)
		{
			const int index = i % 16;
			Color mapping[16];
			mapping[0] = Color(66, 30, 15);
			mapping[1] = Color(25, 7, 26);
			mapping[2] = Color(9, 1, 47);
			mapping[3] = Color(4, 4, 73);
			mapping[4] = Color(0, 7, 100);
			mapping[5] = Color(12, 44, 138);
			mapping[6] = Color(24, 82, 177);
			mapping[7] = Color(57, 125, 209);
			mapping[8] = Color(134, 181, 229);
			mapping[9] = Color(211, 236, 248);
			mapping[10] = Color(241, 233, 191);
			mapping[11] = Color(248, 201, 95);
			mapping[12] = Color(255, 170, 0);
			mapping[13] = Color(204, 128, 0);
			mapping[14] = Color(153, 87, 0);
			mapping[15] = Color(106, 52, 3);
			return mapping[index];
		}
	}

	const auto c = Color(0, 0, 0);
	return c; 
}

void reloadImage(const double rangeX, const double rangeY, const double middleX, const double middleY, const int maxIteration)
{
	const auto start = std::chrono::high_resolution_clock::now();
	const auto forWidth = static_cast<int>(width);
	const auto forHeight = static_cast<int>(height); 
	
	#pragma omp parallel num_threads(8)
	{
		#pragma omp for
		for (int x = 0; x < forWidth; x++)
		{
			for (int y = 0; y < forHeight; y++)
			{
				const auto pixelId = y * width + x;
				const auto pixel = calculatePixel(x, y, rangeX, rangeY, middleX, middleY, maxIteration);
				#pragma omp critical
				images[pixelId] = pixel; //the only point were the image is written by all threads
			}
		}
	}
	const auto stop = std::chrono::high_resolution_clock::now();

	const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

	std::stringstream ss;

	ss << "This Calculation with " << maxIteration << " maximum iteration needed: " << ms.count() << " ms. (PARALLEL)" << std::endl;

	writeLogFile(ss.str());
	
	saveImage("TestImage.bmp", images, width, height);
}




unsigned char* getCharDataFromColor()
{
	BMPHeader header;
	BMPInfoHeader infoHeader;

	header.bfType = 0x4D42; //translated: BM (BitMap)
	header.bfSize = sizeof(BMPHeader) + sizeof(BMPInfoHeader) + 3 * width * height; //1 Bytes for each color in the pixel
	header.bfOffBits = sizeof(BMPHeader) + sizeof(BMPInfoHeader);
	header.bfReserved1 = 0;
	header.bfReserved2 = 0;


	infoHeader.biBitCount = 24; //3 channels each 8 Byte
	infoHeader.biClrImportant = 0;
	infoHeader.biClrUsed = 0;
	infoHeader.biCompression = BI_RGB; //BI_RGB
	infoHeader.biHeight = height;
	infoHeader.biWidth = width;
	infoHeader.biPlanes = 1;
	infoHeader.biSize = sizeof(BMPInfoHeader);
	infoHeader.biSizeImage = header.bfSize;
	infoHeader.biXPixelsPerMeter = 2000;
	infoHeader.biYPixelsPerMeter = 2000;

	auto chars = new unsigned char[header.bfSize];

	//memcpy(chars, (char*)&header, sizeof(BMPHeader));
	//memcpy(chars + sizeof(BMPHeader), (char*)&infoHeader, sizeof(BMPInfoHeader));

	for (int i = 0; i < width*height; i++)
	{
		auto index = i * 3;
		const Color c = images[i];
		chars[index] = c.b;
		chars[index++] = c.g;
		chars[index += 2] = c.r;
	}
	
	return chars;
}

SDL_Surface* getOptimizedSurface(const char* filePath,  SDL_Surface *windowSurface)
{
	SDL_Surface* optimizedSurface = nullptr;
	
	//SDL_RWops* rw = SDL_RWFromMem(&getCharDataFromColor()[0], height*width);
	//SDL_Surface* surface = SDL_LoadBMP_RW(rw, 1);

	SDL_Surface* surface = SDL_LoadBMP(filePath);
	if(surface == nullptr)
	{
		std::cout << "Error Loading File: " << SDL_GetError() << std::endl;
		return nullptr; 
	}

	optimizedSurface = SDL_ConvertSurface(surface, windowSurface->format, 0);

	if(optimizedSurface == nullptr)
	{
		std::cout << "Error at Converting Surface: " << SDL_GetError() << std::endl;
		return nullptr; 
	}

	SDL_FreeSurface(surface);
	
	return optimizedSurface; 
}

int main(int argc, char* argv[])
{	
	SDL_Window* window = nullptr;
	SDL_Surface* windowSurface = nullptr;
	SDL_Surface* currentImage = nullptr;

	int maxIteration = 500;
	double middleX = -0.75;
	double middleY = 0;

	double rangeX = 3.5;
	double rangeY = 2;
	

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "Video Initialization Error: " << SDL_GetError() << std::endl;
		return -1;
	}

	window = SDL_CreateWindow("Mandelbrot v1.0", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);

	if (window == nullptr)
	{
		std::cout << "Window Creation Error: " << SDL_GetError() << std::endl;
		return -2;
	}

	SDL_Rect drawingRect;
	drawingRect.x = drawingRect.y = 0;
	drawingRect.w = 1280;
	drawingRect.h = 720;

	reloadImage(rangeX, rangeY, middleX, middleY, maxIteration);
	windowSurface = SDL_GetWindowSurface(window);
	currentImage = getOptimizedSurface("TestImage.bmp",windowSurface);
	SDL_BlitScaled(currentImage, nullptr, windowSurface, &drawingRect);

	SDL_UpdateWindowSurface(window);

	auto NOW = SDL_GetPerformanceCounter();
	Uint64 LAST = 0;
	double deltaTime = 0;

	auto isRunning = true;
	SDL_Event ev;

	while(isRunning)
	{
		//this is needed for the navigation of the picture
		LAST = NOW;
		NOW = SDL_GetPerformanceCounter();
		deltaTime = static_cast<double>(static_cast<double>((NOW - LAST) * 1000) / static_cast<double>(SDL_GetPerformanceFrequency()));
		
		//controls to navigate in the picture 
		while(SDL_PollEvent(&ev) != 0)
		{
			//the x for window closed is pressed 
			if (ev.type == SDL_QUIT)
				isRunning = false;
			//keyboard buttons events
			else if(ev.type == SDL_KEYDOWN)
			{
				int newMaxIteration;

				switch(ev.key.keysym.sym)
				{
					case SDLK_PAGEUP:
						rangeX *= 0.8;
						rangeY *= 0.8;
						//Zoom In; 
						break;
					case SDLK_PAGEDOWN:
						rangeX /= 0.8;
						rangeY /= 0.8; 
						//Zoom Out;
						break; 
					case SDLK_w:
						middleY += 0.1 * deltaTime * rangeY;
						//GET UP THE PIC
						break;
					case SDLK_s:
						middleY -= 0.1 * deltaTime * rangeY;
						//GET DOWN THE PIC
						break;
					case SDLK_d:
						//GET RIGHT THE PIC
						middleX += 0.1 * deltaTime * rangeX;
						break;
					case SDLK_a:
						//GET LEFT THE PIC
						middleX -= 0.1 * deltaTime * rangeX;
						break;
					case SDLK_UP:
						newMaxIteration = ceil(maxIteration / 0.9);
						if (newMaxIteration == maxIteration)
							maxIteration++;
						else
							maxIteration = newMaxIteration;
						break;
					case SDLK_DOWN:
						newMaxIteration = floor(maxIteration * 0.9);
						if (newMaxIteration == maxIteration)
							maxIteration--;
						else
							maxIteration = newMaxIteration;
						break; 
					default:
						break; 
					
				}
				
				reloadImage(rangeX, rangeY, middleX, middleY, maxIteration);

				currentImage = getOptimizedSurface("TestImage.bmp", windowSurface);
				SDL_BlitScaled(currentImage, nullptr, windowSurface, &drawingRect);
			}
		}
		SDL_UpdateWindowSurface(window);
	}
	
	SDL_DestroyWindow(window);
	SDL_FreeSurface(currentImage);

	window = nullptr;
	currentImage = nullptr;
	windowSurface = nullptr; 

	delete[] images;
	SDL_Quit();
	return 0; 
}