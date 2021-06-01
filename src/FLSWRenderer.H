#ifndef __FLSWRENDERER_H__
#define __FLSWRENDERER_H__
#pragma once

/*******************************************************************************
*
*  FLTK Software 3D Renderer
*  ========================================
*  (C)Copyrighted 2021, Raphael Kim
*  [ source at ] 
*
*******************************************************************************/

#include <FL/Fl.H>
#include <FL/Fl_RGB_Image.H>
#include "FLSWRenderMath.H"

class FLSWRenderer
{
    public:
        FLSWRenderer( Fl_RGB_Image* target = NULL );
        ~FLSWRenderer();
    
    public:
        bool LoadObjects( const char* objfn = NULL );
        bool LoadObjectsIndir( const char* objdata = NULL, size_t datalen = 0 );
        bool LoadTexture( const char* txtfn = NULL );
        bool LoadTexture( Fl_RGB_Image* img );
        bool Render( bool earsebuff = true );
        
    public:
        void color( unsigned color );
        unsigned color();
        void linecolor( unsigned color );
        unsigned linecolor();
        void line( bool onoff );
        bool line();
        void texture( bool onoff );
        bool texture();

    public: /// vertex, faces, normals --
        size_t vertexs();
        size_t faces();
        size_t texturecoords();

    public: /// object modifiers
        vec3f* shift();
        vec3f* rotate();
        vec3f* scale();
        vec3f* light();
        vec3f* eye();
        vec3f* lookat();
        vec3f* upside();
        
    public: /// global scene configs
        float* aspectratio();
        float* FOV();
        float* FarPlane();
        float* NearPlane();

    protected:
        void init();
        void deinit();

    protected:
        Fl_RGB_Image*   rtarget;
        unsigned        defaultcolor;
        unsigned        dlinecolor;
        bool            drawline;
        bool            drawtexture;
        
    private:
        void*           ctx;
        
    private:
};

#endif /// of __FLSWRENDERER_H__
