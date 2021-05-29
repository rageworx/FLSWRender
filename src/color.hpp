#ifndef __COLOR_H__
#define __COLOR_H__

#pragma region Vector
struct Color
{
    int r,g,b,a;

    Color( int rr, int gg, int bb, int aa=255 )
      : r(rr), g(gg), b(bb), a(aa)
    {
    }
};
#pragma endregion

#endif /// of __COLOR_H__
