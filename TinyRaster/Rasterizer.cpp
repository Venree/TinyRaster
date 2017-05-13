/*---------------------------------------------------------------------
*
* Copyright © 2016  Minsi Chen
* E-mail: m.chen@derby.ac.uk
*
* The source is written for the Graphics I and II modules. You are free
* to use and extend the functionality. The code provided here is functional
* however the author does not guarantee its performance.
---------------------------------------------------------------------*/
#include <algorithm>
#include <math.h>

#include "Rasterizer.h"

Rasterizer::Rasterizer(void)
{
	mFramebuffer = NULL;
	mScanlineLUT = NULL;
}

void Rasterizer::ClearScanlineLUT()
{
	Scanline *pScanline = mScanlineLUT;

	for (int y = 0; y < mHeight; y++)
	{
		(pScanline + y)->clear();
		(pScanline + y)->shrink_to_fit();
	}
}

unsigned int Rasterizer::ComputeOutCode(const Vector2 & p, const ClipRect& clipRect)
{
	unsigned int CENTRE = 0x0;
	unsigned int LEFT = 0x1;
	unsigned int RIGHT = 0x1 << 1;
	unsigned int BOTTOM = 0x1 << 2;
	unsigned int TOP = 0x1 << 3;
	unsigned int outcode = CENTRE;
	
	if (p[0] < clipRect.left)
		outcode |= LEFT;
	else if (p[0] >= clipRect.right)
		outcode |= RIGHT;

	if (p[1] < clipRect.bottom)
		outcode |= BOTTOM;
	else if (p[1] >= clipRect.top)
		outcode |= TOP;

	return outcode;
}

bool Rasterizer::ClipLine(const Vertex2d & v1, const Vertex2d & v2, const ClipRect& clipRect, Vector2 & outP1, Vector2 & outP2)
{
	//TODO: EXTRA This is not directly prescribed as an assignment exercise. 
	//However, if you want to create an efficient and robust rasteriser, clipping is a usefull addition.
	//The following code is the starting point of the Cohen-Sutherland clipping algorithm.
	//If you complete its implementation, you can test it by calling prior to calling any DrawLine2D .

	const Vector2 p1 = v1.position;
	const Vector2 p2 = v2.position;
	unsigned int outcode1 = ComputeOutCode(p1, clipRect);
	unsigned int outcode2 = ComputeOutCode(p2, clipRect);

	outP1 = p1;
	outP2 = p2;
	
	bool draw = false;

	return true;
}

void Rasterizer::WriteRGBAToFramebuffer(int x, int y, const Colour4 & colour)
{
	if (0<=x && x<=(mWidth-1) && 0<=y && y<=(mHeight-1))
	{
		PixelRGBA *pixel = mFramebuffer->GetBuffer();
		pixel[y*mWidth + x] = colour;
	}
}

Rasterizer::Rasterizer(int width, int height)
{
	//Initialise the rasterizer to its initial state
	mFramebuffer = new Framebuffer(width, height);
	mScanlineLUT = new Scanline[height];
	mWidth = width;
	mHeight = height;

	mBGColour.SetVector(0.0, 0.0, 0.0, 1.0);	//default bg colour is black
	mFGColour.SetVector(1.0, 1.0, 1.0, 1.0);    //default fg colour is white

	mGeometryMode = LINE;
	mFillMode = UNFILLED;
	mBlendMode = NO_BLEND;

	SetClipRectangle(0, mWidth, 0, mHeight);
}

Rasterizer::~Rasterizer()
{
	delete mFramebuffer;
	delete[] mScanlineLUT;
}

void Rasterizer::Clear(const Colour4& colour)
{
	PixelRGBA *pixel = mFramebuffer->GetBuffer();

	SetBGColour(colour);

	int size = mWidth*mHeight;
	
	for(int i = 0; i < size; i++)
	{
		//fill all pixels in the framebuffer with background colour
		*(pixel + i) = mBGColour;
	}
}

void Rasterizer::DrawPoint2D(const Vector2& pt, int size)
{
	int x = pt[0];
	int y = pt[1];
	
	WriteRGBAToFramebuffer(x, y, mFGColour);
}

void Rasterizer::DrawLine2D(const Vertex2d & v1, const Vertex2d & v2, int thickness)
{
	//The following code is basic Bresenham's line drawing algorithm.
	//The current implementation is only capable of rasterise a line in the first octant, where dy < dx and dy/dx >= 0
	//See if you want to read ahead https://www.cs.helsinki.fi/group/goa/mallinnus/lines/bresenh.html
	
	//TODO:
	//Ex 1.1 Complete the implementation of Rasterizer::DrawLine2D method. 
	//This method currently consists of a partially implemented Bresenham algorithm.
	//You must extend its implementation so that it is capable of drawing 2D lines with arbitrary gradient(slope).
	//Use Test 1 (Press F1) to test your implementation
	
	//Ex 1.2 Extend the implementation of Rasterizer::DrawLine2D so that it is capable of drawing lines based on a given thickness.
	//Note: The thickness is passed as an argument int thickness.
	//Use Test 2 (Press F2) to test your implementation

	//Ex 1.3 Extend the implementation of Rasterizer::DrawLine2D so that it is capable of interpolating colour across a line when each end-point has different colours.
	//Note: The variable mFillMode indicates if the fill mode is set to INTERPOLATED_FILL. 
	//The colour of each point should be linearly interpolated using the colours of v1 and v2.
	//Use Test 2 (Press F2) to test your implementation

	Vector2 pt1 = v1.position;
	Vector2 pt2 = v2.position;

	bool swap_vertices = pt1[0] > pt2[0];

	if (swap_vertices)
	{
		pt1 = v2.position;
		pt2 = v1.position;
	}

	int dx = pt2[0] - pt1[0];
	int dy = pt2[1] - pt1[1];

	bool invert_y = dy < 0;
	bool swap_xy = abs(dy) > abs(dx);
	bool octant_3 = swap_vertices && invert_y && swap_xy;
	bool octant_7 = invert_y && swap_xy;

	int epsilon = 0;
	int x = pt1[0];
	int y = pt1[1];
	int ex = pt2[0];
	//needed for scanline
	int oldx = -1;
	int oldy = -1;

	if (octant_3)
	{
		pt1 = v1.position;
		pt2 = v2.position;
		x = pt1[1];
		y = -pt1[0];
		dx = pt2[1] - pt1[1];
		dy = -(pt2[0] - pt1[0]);
		ex = pt2[1];
	}
	else if (octant_7)
	{
		x = -pt1[1];
		y = pt1[0];
		dx = -(pt2[1] - pt1[1]);
		dy = pt2[0] - pt1[0];
		ex = -pt2[1];
	}
	else
	{
		if (invert_y)
		{
			dy = -dy;
			y = -y;
		}

		if (swap_xy)
		{
			x = pt1[1];
			y = pt1[0];
			ex = pt2[1];
			dx = pt2[1] - pt1[1];
			dy = pt2[0] - pt1[0];
		}
	}
	int distance = dx > dy ? dx : dy;
	Colour4 color_start = v1.colour;
	Colour4 color_end = v2.colour;
	//setting up step values for the RGB
	double stepR = abs(color_end[0] - color_start[0]) / distance;
	double stepG = abs(color_end[1] - color_start[1]) / distance;
	double stepB = abs(color_end[2] - color_start[2]) / distance;
	/*NO delta r,g,b make them 0*/
	double deltaR = 0;
	double deltaG = 0;
	double deltaB = 0;
	//test
	if (swap_vertices && !octant_3)
	{
		color_start = v2.colour;
		color_end = v1.colour;
	}
	while (x < ex)
	{
		Vector2 temp(x, y);
		if (octant_3)
		{
			int z = temp[0];
			temp[0] = -temp[1];
			temp[1] = z;
		}
		else
		{
			if (swap_xy)
			{
				int z = temp[0];
				temp[0] = temp[1];
				temp[1] = z;
			}
			if (invert_y)
			{
				temp[1] = -temp[1];
			}
		}
		Colour4 color;
		if (mFillMode == INTERPOLATED_FILLED)
		{
			
			color[0] = deltaR * color_end[0] + (1 - deltaR) * color_start[0];
			color[1] = deltaG * color_end[1] + (1 - deltaG) * color_start[1];
			color[2] = deltaB * color_end[2] + (1 - deltaB) * color_start[2];
			SetFGColour(color);
			deltaR += stepR;
			deltaG += stepG;
			deltaB += stepB;
		}
		else
		{
			SetFGColour(color_start);
		}
		//blends 2 colors (rgb values) using the alpha value provided
		//mFGColor[0] threw 2 are the RBG mFGColor[3] is the alpha
		if (mBlendMode == ALPHA_BLEND)
		{
			PixelRGBA *pixel = mFramebuffer->GetBuffer();
			color = pixel[abs(int((temp[1])*mWidth + temp[0]))];
			mFGColour[0] = color_start[3] * color_start[0] + (1 - color_start[3])*color[0];
			mFGColour[1] = color_start[3] * color_start[1] + (1 - color_start[3])*color[1];
			mFGColour[2] = color_start[3] * color_start[2] + (1 - color_start[3])*color[2];
			mFGColour[3] = color_start[3] * color_start[3] + (1 - color_start[3])*color[3];
			SetFGColour(mFGColour);
		}
		//Save lines to the scan line LUT
		if (mGeometryMode == POLYGON && mFillMode != UNFILLED)
		{
			
			if ((oldx - temp[0]) == 1 || (oldy != temp[1]))
			{
				ScanlineLUTItem item = { mFGColour, temp[0] };
				int y = abs(temp[1]);
				mScanlineLUT[y].push_back(item);
				oldx = temp[0];
				oldy = temp[1];
			}
		}
		
		DrawPoint2D(temp);
		epsilon += dy;
		
		int increment = 1;
		int test = thickness;
		int count = 1;
		while (count < thickness)
		{		
			int x1 = x;
			int y1 = y;
			bool even = test%2 == 0;
			bool odd = test%2 != 0;
			if (even)
			{
				y1 -= increment;
				if (swap_xy)
				{
					if (invert_y)
					{
						Vector2 temp1(-y1, x1);
						DrawPoint2D(temp1);
					}
					else
					{
						Vector2 temp1(y1, x1);
						DrawPoint2D(temp1);
					}
				}
				else
				{
					if (invert_y)
					{
						Vector2 temp1(x1, -y1);
						DrawPoint2D(temp1);
					}
					else
					{
						Vector2 temp1(x1, y1);
						DrawPoint2D(temp1);
					}
				}
				increment++;
			}
			if (odd)
			{
				y1 += increment;
				if (swap_xy)
				{
					if (invert_y)
					{
						Vector2 temp1(-y1, x1);
						DrawPoint2D(temp1);
					}
					else
					{
						Vector2 temp1(y1, x1);
						DrawPoint2D(temp1);
					}
				}
				else
				{
					if (invert_y)
					{
						Vector2 temp1(x1, -y1);
						DrawPoint2D(temp1);
					}
					else
					{
						Vector2 temp1(x1, y1);
						DrawPoint2D(temp1);
					}
				}
			}
			test++;
			count++;
		}
		if ((epsilon << 1) >= dx)
		{
			y++;
			epsilon -= dx;
		}
		x++;
	}
}

void Rasterizer::DrawUnfilledPolygon2D(const Vertex2d * vertices, int count)
{
	//TODO:
	//Ex 2.1 Implement the Rasterizer::DrawUnfilledPolygon2D method so that it is capable of drawing an unfilled polygon, i.e. only the edges of a polygon are rasterised. 
	//Please note, in order to complete this exercise, you must first complete Ex1.1 since DrawLine2D method is reusable here.
	//Note: The edges of a given polygon can be found by conntecting two adjacent vertices in the vertices array.
	//Use Test 3 (Press F3) to test your solution.
	int i = 0;
	while (i < count-1)
	{
		DrawLine2D(vertices[i], vertices[i + 1]);
		i++;
	}
	//draw an extra line to close the poligons
	DrawLine2D(vertices[count - 1], vertices[0]);
}

//method to sort the X values of the ScanlineLUT
bool SortTest(const ScanlineLUTItem item1, const ScanlineLUTItem item2) 
{
	return (item1.pos_x > item2.pos_x);
}

void Rasterizer::ScanlineFillPolygon2D(const Vertex2d * vertices, int count)
{
	//TODO:
	//Ex 2.2 Implement the Rasterizer::ScanlineFillPolygon2D method method so that it is capable of drawing a solidly filled polygon.
	//Note: You can implement floodfill for this exercise however scanline fill is considered a more efficient and robust solution.
	//		You should be able to reuse DrawUnfilledPolygon2D here.
	//
	//Use Test 4 (Press F4) to test your solution, this is a simple test case as all polygons are convex.
	//Use Test 5 (Press F5) to test your solution, this is a complex test case with one non-convex polygon.

	//Ex 2.3 Extend Rasterizer::ScanlineFillPolygon2D method so that it is capable of alpha blending, i.e. draw translucent polygons.
	//Note: The variable mBlendMode indicates if the blend mode is set to alpha blending.
	//To do alpha blending during filling, the new colour of a point should be combined with the existing colour in the framebuffer using the alpha value.
	//Use Test 6 (Press F6) to test your solution
	ClearScanlineLUT();
	DrawUnfilledPolygon2D(vertices, count);
	int y = 0;
	while (y < mHeight)
	{
		int size = mScanlineLUT[y].size();
		if (!mScanlineLUT[y].empty() && size > 1)
		{
			std::sort(&mScanlineLUT[y][0], &mScanlineLUT[y][size], SortTest);
			int j = 0;
			while(j < size)
			{
				Vertex2d temp1;
				Vertex2d temp2;
				temp1.position[0] = mScanlineLUT[y][j].pos_x;
				temp1.position[1] = y;
				temp2.position[0] = mScanlineLUT[y][j + 1].pos_x;
				temp2.position[1] = y;
				temp1.colour = mScanlineLUT[y][j].colour;
				temp2.colour = mScanlineLUT[y][j + 1].colour;
				/*
				if (count <= 5)
				{
					temp2.position[0] = mScanlineLUT[y][size - 1].pos_x;
				} 
				*/
				DrawLine2D(temp1, temp2);
				j +=2;
			}
		}
		y++;
	}
}

void Rasterizer::ScanlineInterpolatedFillPolygon2D(const Vertex2d * vertices, int count)
{
	//TODO:
	//Ex 2.4 Implement Rasterizer::ScanlineInterpolatedFillPolygon2D method so that it is capable of performing interpolated filling.
	//Note: mFillMode is set to INTERPOLATED_FILL
	//		This exercise will be more straightfoward if Ex 1.3 has been implemented in DrawLine2D
	//Use Test 7 to test your solution
	ScanlineFillPolygon2D(vertices, count);
}

void Rasterizer::DrawCircle2D(const Circle2D & inCircle, bool filled)
{
	//TODO:
	//Ex 2.5 Implement Rasterizer::DrawCircle2D method so that it can draw a filled circle.
	//Note: For a simple solution, you can first attempt to draw an unfilled circle in the same way as drawing an unfilled polygon.
	//Use Test 8 to test your solution
	ClearScanlineLUT();
	const double pi = 3.1415926535897;
	Vertex2d segments[35];
	int i = 0;

	Vector2 centre = inCircle.centre;
	float r = inCircle.radius;
	Colour4 color = inCircle.colour;
	SetFGColour(color);
	float theta = 0;
	float h = centre[0]; //X 
	float k = centre[1]; //Y
	float step = (2*pi)/35;
	float a = h + r; //X + radius
	Vector2 pt2Pos(a, k);
	Vertex2d pt2;
	
	pt2.colour = color;
	pt2.position = pt2Pos;
	while (theta <= (2*pi))
	{
		int x = h + (r*cos(theta));
		int y = k + (r*sin(theta));
		Vector2 line(x, y);
		Vertex2d pt1;
		pt1.colour = color;
		pt1.position = line;
		theta = theta + step;
		if (i < 35)
		{
			segments[i++] = pt1;
		}
		pt2 = pt1;
	}
	DrawUnfilledPolygon2D(segments, i);
	if (filled)
	{
		mGeometryMode = POLYGON;
		mFillMode = SOLID_FILLED;
		ScanlineFillPolygon2D(segments, i);
	}
}

Framebuffer *Rasterizer::GetFrameBuffer() const
{
	return mFramebuffer;
}
