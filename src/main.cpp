#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <queue>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <GL/gl.h>

#include "sdf.h"

const int SCRWIDTH  = 1920;
const int SCRHEIGHT = 1080;

using namespace sf;
using namespace std;

RenderWindow* myapp;
Font font;

class Game
{
	public:
		string name;
		string title;
		string description;
		Texture t;
		float r, g, b;

		void load(string _name)
		{
			name = _name;

			ifstream in (("./games/"+name+"/game.txt").c_str());
			getline(in, title);
			in>>r>>g>>b;

			string line;
			getline(in, line);
			getline(in, line);

			while(getline(in, line))
				description += line+"\n";

			if(!t.loadFromFile("./games/"+name+"/game.png"))
				cout<<"Cant load image for game "+name<<endl;
		}

		void render(float p)
		{
			float dif = max(0.0f, 1-abs(p));
			float x = 200+250*dif;
			float y = 540+250*p;
			if(p < 0)
				y -= (1-dif)*60;
			else
				y += (1-dif)*60;

			Sprite sprite;
			sprite.setTexture(t);
			float size = 160+200*dif;
			sprite.setScale(size/t.getSize().x, size/t.getSize().y);
			sprite.setPosition(x, y);
			sprite.setOrigin(t.getSize().x, t.getSize().y/2);
			myapp->draw(sprite);


			sf::Text text;
			text.setFont(font);
			text.setString(title);
			text.setCharacterSize(30);
			text.setColor(sf::Color::White);
			text.setStyle(sf::Text::Bold);
			text.setPosition(x+50, y-20-100*dif);
			myapp->draw(text);

			text.setStyle(0);
			text.setCharacterSize(24);
			text.setPosition(x+50, y+15-100*dif);
			text.setString(description);
			text.setColor(sf::Color(255, 255, 255, 255*dif));
			myapp->draw(text);
		}
};

vector<Game> games;


void loadGames()
{
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir ("./games/")) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			string name = ent->d_name;
			if(name != "." && name != "..")
			{
				games.push_back(Game());
				games[games.size()-1].load(name);
			}
		}
		closedir (dir);
	} else {
		/* could not open directory */
		cout<<"Could not open games/ dir"<<endl;
	}
}

// Returns texture id
GLuint makeDistanceField()
{
	Image img;
	img.loadFromFile("data/outline.png");
	const int size = 4096;
	const unsigned char* px = img.getPixelsPtr();

	float* dist = calcSdf(px);

	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, size, size, 0, GL_RED, GL_FLOAT, dist);

	delete dist;

	return tex;
}

int main()
{
	cout<<"Loading games..."<<endl;
	loadGames();

	cout<<"Creating window..."<<endl;
	RenderWindow app(VideoMode(SCRWIDTH, SCRHEIGHT, 32), "Week1", Style::None);
	myapp = &app;

	cout<<"Loading font..."<<endl;
	if (!font.loadFromFile("data/BitDarling.ttf"))
		cout<<"Can't load font!";

	cout<<"Rendering distfield..."<<endl;
	GLuint distfield = makeDistanceField();

	cout<<"Loading shader..."<<endl;
	Shader* shader = new Shader();
	shader->loadFromFile("shaders/vertex.glsl", "shaders/fragment.glsl");

	float t = 0;
	int destt = 0;
	float allTime = 0;

	cout<<"Reading current game..."<<endl;
	ifstream in("game");
	string currgame;
	getline(in, currgame);
	for(int i = 0; i < (int)games.size(); i++)
		if(games[i].name == currgame)
		{
			t = i;
			destt = i;
		}


	float rr = games[0].r;
	float gg = games[0].g;
	float bb = games[0].b;

	cout<<"Running..."<<endl;
	Clock clock;
	while(app.isOpen())
	{
		
		Event event;
		while(app.pollEvent(event))
		{
			if (event.type == Event::Closed)
				app.close();

			if(event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)
				app.close();

			if(event.type == Event::KeyPressed && event.key.code == Keyboard::Down)
				destt++;

			if(event.type == Event::KeyPressed && event.key.code == Keyboard::Up)
				destt--;

			if(destt < 0) destt = 0;
			if(destt >= (int)games.size()) destt = games.size()-1;

			if((event.type == Event::KeyPressed && (event.key.code == Keyboard::Return))
				 || (event.key.code == Keyboard::Space))
			{
				ofstream out("game");
				out << games[destt].name;
				out.close();
				myapp->close();
			}
		}

		
		Time time = clock.restart();
		float deltaTime = time.asSeconds();

		allTime += deltaTime;

		float mix = exp(-deltaTime*10);
		t = t*mix + destt*(1-mix);

		mix = exp(-deltaTime*4);
		rr = rr*mix + games[destt].r*(1-mix);
		gg = gg*mix + games[destt].g*(1-mix);
		bb = bb*mix + games[destt].b*(1-mix);

		app.clear();


		if(true)
		{
			glBindTexture(GL_TEXTURE_2D, distfield);
			Shader::bind(shader);
			shader->setParameter("time", allTime);
			shader->setParameter("color", rr, gg, bb);
			shader->setParameter("resolution", app.getSize().x, app.getSize().y);
			glBegin(GL_QUADS);
			/*float aspectRatio = (float)myapp->getSize().x / (float)myapp->getSize().y;
			glTexCoord2f(-aspectRatio, -1); glVertex2f(-1, -1);
			glTexCoord2f(aspectRatio, -1); glVertex2f(1, -1);
			glTexCoord2f(aspectRatio, 1); glVertex2f(1, 1);
			glTexCoord2f(-aspectRatio, 1); glVertex2f(-1, 1);*/
			glTexCoord2f(0, 0); glVertex2f(-1, -1);
			glTexCoord2f(1, 0); glVertex2f(1, -1);
			glTexCoord2f(1, 1); glVertex2f(1, 1);
			glTexCoord2f(0, 1); glVertex2f(-1, 1);
			glEnd();
		}

		Shader::bind(NULL);

		for(int i = 0; i < (int)games.size(); i++)
			games[i].render(i-t);


		app.display();
	}
	
	return 0;
}

