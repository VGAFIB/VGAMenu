//http://www.codersnotes.com/notes/signed-distance-fields
#include <math.h>

#define WIDTH  4096
#define HEIGHT 1024

struct Point
{
	int dx, dy;

	int DistSq() const { return dx*dx + dy*dy; }
};

struct Grid
{
	Point grid[HEIGHT][WIDTH];
};

Point inside = { 0, 0 };
Point empty = { 9999, 9999 };
Grid grid1, grid2;

Point Get( Grid &g, int x, int y )
{
	// OPTIMIZATION: you can skip the edge check code if you make your grid 
	// have a 1-pixel gutter.
	if ( x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT )
		return g.grid[y][x];
	else
		return empty;
}

void Put( Grid &g, int x, int y, const Point &p )
{
	g.grid[y][x] = p;
}

void Compare( Grid &g, Point &p, int x, int y, int offsetx, int offsety )
{
	Point other = Get( g, x+offsetx, y+offsety );
	other.dx += offsetx;
	other.dy += offsety;

	if (other.DistSq() < p.DistSq())
		p = other;
}

void GenerateSDF( Grid &g )
{
	// Pass 0
	for (int y=0;y<HEIGHT;y++)
	{
		for (int x=0;x<WIDTH;x++)
		{
			Point p = Get( g, x, y );
			Compare( g, p, x, y, -1,  0 );
			Compare( g, p, x, y,  0, -1 );
			Compare( g, p, x, y, -1, -1 );
			Compare( g, p, x, y,  1, -1 );
			Put( g, x, y, p );
		}

		for (int x=WIDTH-1;x>=0;x--)
		{
			Point p = Get( g, x, y );
			Compare( g, p, x, y, 1, 0 );
			Put( g, x, y, p );
		}
	}

	// Pass 1
	for (int y=HEIGHT-1;y>=0;y--)
	{
		for (int x=WIDTH-1;x>=0;x--)
		{
			Point p = Get( g, x, y );
			Compare( g, p, x, y,  1,  0 );
			Compare( g, p, x, y,  0,  1 );
			Compare( g, p, x, y, -1,  1 );
			Compare( g, p, x, y,  1,  1 );
			Put( g, x, y, p );
		}

		for (int x=0;x<WIDTH;x++)
		{
			Point p = Get( g, x, y );
			Compare( g, p, x, y, -1, 0 );
			Put( g, x, y, p );
		}
	}
}

float* calcSdf(const unsigned char* px)
{
	for(int y=0; y<HEIGHT; y++)
		for(int x=0; x<WIDTH; x++)
		{
			unsigned char g = px[(x+y*WIDTH)*4];

			// Points inside get marked with a dx/dy of zero.
			// Points outside get marked with an infinitely large distance.
			if ( g < 128 )
			{
				Put( grid1, x, y, inside );
				Put( grid2, x, y, empty );
			} else {
				Put( grid2, x, y, inside );
				Put( grid1, x, y, empty );
			}
		}


	// Generate the SDF.
	GenerateSDF( grid1 );
	GenerateSDF( grid2 );
	

	float* res = new float[WIDTH*HEIGHT];

	for(int y=0; y<HEIGHT; y++)
		for(int x=0; x<WIDTH; x++)
		{
			// Calculate the actual distance from the dx/dy
			float dist2 = sqrt( (float)Get( grid2, x, y ).DistSq() );
			float dist1 = sqrt( (float)Get( grid1, x, y ).DistSq() );
			float dist = dist1 - dist2;

			res[x+y*WIDTH] = dist/WIDTH;
		}

	return res;
}
