#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "util.h"
#include "types.h"
#include "cramConstants.h"

#include <iostream>
#include <string>


typedef uint8_t byte;
typedef uint16_t word;

using sf::Window;
using sf::RenderWindow;
using sf::Image;
using sf::Keyboard;
using std::string;
using std::to_string;

#define Colour sf::Color
#define UpdateWindow() window.display()


RenderWindow window(sf::VideoMode(256, 256), WINDOW_TITLE);


auto debugKey = Keyboard::Escape;
bool CheckForDebugKey() {
	return (Keyboard::isKeyPressed(debugKey));
}


//for loading and saving states
void SaveScreen(string filename) {

	//totally not just copied from sfml-dev.org
	sf::Vector2u windowSize = window.getSize();
	sf::Texture texture;
	texture.create(windowSize.x, windowSize.y);
	texture.update(window);
	sf::Image screenshot = texture.copyToImage();

	filename += ".png";

	screenshot.saveToFile(filename);
}
void LoadScreen(string filename) {

	//get image
	filename += ".png";
	Image image;
	image.loadFromFile(filename);

	//draw it to screen (same code as drawsprite function)
	window.clear(Colour::Black);

	sf::Texture texture;
	texture.loadFromImage(image);
	sf::Sprite sprite;
	sprite.setTexture(texture, true);

	window.draw(sprite);

	UpdateWindow();
}


void graphicsinit() {

	//window setup
	//window = RenderWindow.create();

	//no vsync
	window.setVerticalSyncEnabled(false);
	//60 fps
	window.setFramerateLimit(60);

	window.clear(Colour::Black);
}


//we have to rename it otherwise the compiler complains
bool G_GetBit(int value, int num) {
	return value & 1 << num;
}

void GetEvents() {
	sf::Event event;	//why tf is this highlighted
    while (window.pollEvent(event))
    {
		//apparently windows aren't closed automatically
        if (event.type == sf::Event::Closed) {
			window.close();
		}
    }
}

//this is terrible for the economy
void DrawSprites(byte vramBase[]) {

	const int screenSize = 256;	//in pixels
	const int chunkSize = 8;	//in pixels
	const int chunksPerLine = screenSize / chunkSize;

	//each chunk is represented by an image
	Image image;
	image.create(chunkSize, chunkSize);

	//total current offset
	int positionInMemory = 0;

	//apparently declaring variables here instead of in the loop increases the performance A LOT. wtf?
	sf::Texture texture;
	sf::Sprite sprite;
	
	byte current;

	int y, x, i, bit;

	//foreach chunk
	for (y = 0; y < chunksPerLine; y += 1) {
		for (x = 0; x < chunksPerLine; x += 1) {

			//set the pixels (8x8 pixels)
			for (i = 0; i < 8; i++) {
				current = vramBase[positionInMemory++];

				//each bit is one pixel. one for white, 0 for black
				for (bit = 0; bit < 8; bit++) {
					if (G_GetBit(current, bit))
						image.setPixel(bit, i, Colour::White);
					else
						image.setPixel(bit, i, Colour::Black);
				}
			}

			//no idea why but we have to
			image.flipHorizontally();

			//create the sprite
			texture.loadFromImage(image);
			sprite.setTexture(texture, true);

			//place it on the screen
			window.draw(sprite);

			//move it for the next image
			sprite.move(chunkSize, 0);
		}

		//go to the beginning of the next line
		sprite.move(-screenSize, chunkSize);
	}
}

//one update cycle takes +- 25ms		// you wish lmao
void updatescreen(CPU_T* cpu) {	//why tf is "cpu" highlighted

	window.clear(Colour::Black);

	GetEvents();

	//black magic
	cpu->cramBase[INPUT_REGISTER] =
		Keyboard::isKeyPressed(Keyboard::Right) * 0b00000001 +
		Keyboard::isKeyPressed(Keyboard::Left)  * 0b00000010 +
		Keyboard::isKeyPressed(Keyboard::Up)    * 0b00000100 +
		Keyboard::isKeyPressed(Keyboard::Down)  * 0b00001000 +
		Keyboard::isKeyPressed(Keyboard::Z)     * 0b00010000 +
		Keyboard::isKeyPressed(Keyboard::X)     * 0b00100000 +
		Keyboard::isKeyPressed(Keyboard::A)     * 0b01000000 +
		Keyboard::isKeyPressed(Keyboard::S)     * 0b10000000;

	DrawSprites(cpu->vramBase);

	//display changes
	UpdateWindow();
}
