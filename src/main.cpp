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
#include <sstream>
#include <GL/gl.h>

#include "sdf.h"

const int SCRWIDTH  = 1920;
const int SCRHEIGHT = 1080;

using namespace sf;
using namespace std;

RenderWindow* myapp;
Font font;

sf::String fromUtf8(std::string source) {
  std::basic_string<sf::Uint32> tmp;
  sf::Utf<8>::toUtf32( source.begin(), source.end(), std::back_inserter( tmp ) );
  return tmp;
}

class Game
{
	public:
		string name;
		sf::String title;
		sf::String description;
		Texture t;
		float r, g, b;

		void load(string _name)
		{
			name = _name;

			ifstream in (("./games/"+name+"/game.txt").c_str());
			string line;
			getline(in, line);
			title = fromUtf8(line);
			
			in>>r>>g>>b;

			getline(in, line);
			
			string desc;
			while(getline(in, line)) {
				//Ignorar lineas vacias al principio
				if(description == "" && line == "")
					continue;
				desc += line+"\n";
			}
			
			description = fromUtf8(desc);

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
			text.setCharacterSize(40);
			text.setColor(sf::Color::White);
			text.setStyle(sf::Text::Bold);
			text.setPosition(x+50, y-30-100*dif);
			myapp->draw(text);

			text.setStyle(0);
			text.setCharacterSize(24);
			text.setPosition(x+50, y+25-100*dif);
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
	const unsigned char* px = img.getPixelsPtr();

	float* dist = calcSdf(px);

	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 4096, 1024, 0, GL_RED, GL_FLOAT, dist);

	delete dist;

	return tex;
}

int main()
{
	cout<<"Loading games..."<<endl;
	loadGames();

	cout<<"Creating window..."<<endl;
	RenderWindow app(VideoMode::getDesktopMode(), "Week1", Style::None);
	myapp = &app;

	sf::View view;
	view.reset(sf::FloatRect(0, 0, SCRWIDTH, SCRHEIGHT));
	app.setView(view);

	cout<<"Loading font..."<<endl;
	if (!font.loadFromFile("data/font.ttf"))
		cout<<"Can't load font!";

	cout<<"Rendering distfield..."<<endl;
	GLuint distfield = makeDistanceField();

	cout<<"Loading shader..."<<endl;
	Shader* shader = new Shader();
	shader->loadFromFile("shaders/vertex.glsl", "shaders/fragment.glsl");

	float t = 0;
	int destt = 0;
	float allTime = 0;

	float frameTime = 0;
	float frameCount = 0;
	float fps = 9999;

	cout<<"Reading current game..."<<endl;
	ifstream in("oldgame");
	string currgame;
	getline(in, currgame);
	for(int i = 0; i < (int)games.size(); i++)
		if(games[i].name == currgame)
		{
			t = i;
			destt = i;
		}


	float rr = games[destt].r;
	float gg = games[destt].g;
	float bb = games[destt].b;

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

		allTime += deltaTime * (1+abs(t-destt)*10);

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

		frameTime += deltaTime;
		frameCount++;
		if(frameTime > 2) {
			fps = frameCount/frameTime;
			frameCount = 0;
			frameTime = 0;
		}
		sf::Text text;
		text.setFont(font);
		stringstream ss;
		ss<<"FPS: "<<fps;
		text.setString(ss.str());
		text.setCharacterSize(30);
		text.setColor(sf::Color::White);
		text.setStyle(sf::Text::Bold);
		text.setPosition(20, 20);
		myapp->draw(text);


		app.display();
	}
	
	return 0;
}

