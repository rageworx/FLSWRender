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

#include "geometry.hpp"

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
        bool Render();

    public:
        sceneparam* parameter() { return &senceParam; }

    protected:
        void init();
        void deinit();

    protected:
		Fl_RGB_Image*	rtarget;
        sceneparam      senceParam;
        
    private:
		void*			ctx;
        
    private:
};

#endif /// of __FLSWRENDERER_H__
