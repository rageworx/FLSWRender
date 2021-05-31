#ifndef __COLOR_H__
#define __COLOR_H__

#pragma region Vector
struct Color
{
    int r,g,b,a;

    Color()
      : r(255), g(255), b(255), a(255)
    {
        // default is white.
    }

    Color( int rr, int gg, int bb, int aa=255 )
      : r(rr), g(gg), b(bb), a(aa)
    {
    }
};
#pragma endregion

#endif /// of __COLOR_H__
