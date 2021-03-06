#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <fl_imgtk.h>

#include "FLSWRenderer.H"
#include "perfmon.h"

Fl_Double_Window*   window = NULL;
Fl_Box*             renderbox = NULL;
Fl_Box*             helpbox = NULL;
Fl_Box*             statebox = NULL;
Fl_RGB_Image*       rendersurface = NULL;
Fl_RGB_Image*       renderbg = NULL;
Fl_RGB_Image*       rendermux = NULL;
FLSWRenderer*       renderer = NULL;

// scene configuration pointers --
vec3f*              objMove = NULL;
vec3f*              objRotate = NULL;
vec3f*              objScale = NULL;
vec3f*              renLight = NULL;
vec3f*              renEye = NULL;
vec3f*              renAt = NULL;
vec3f*              renUp = NULL;
float*              renAspect = NULL;
float*              renFOV = NULL;
float*              renFar = NULL;
float*              renNear = NULL;

// object info
size_t              vertexs  = 0;
size_t              faces    = 0;
size_t              textures = 0;

// auto demo
bool                timerdemo = false;

#ifdef TEST_UPSCALING
static float fsaa_ratio  = 1.5f;
#endif // TEST_UPSCALING

#define DEF_COLOR_OBJECT                0xFF3333FF
#define DEF_COLOR_LINE_W_TEXTURE        0xC0C0FF40
#define DEF_COLOR_LINE_ONLY             0xFFFFFF90

#define SETDATA(_x_,...)    if(_x_!=NULL) *_x_ = { __VA_ARGS__ }

static Perfmon  perf;
static char statestring[1024] = {0};

void updateRender()
{
    if ( renderer != NULL )
    {
        perf.SetMonStart();
        renderer->Render( true );
        perf.SetMonStop();
        unsigned long perfms = perf.GetPerfMS();
        
        if ( rendermux == NULL )
        {
            rendermux = (Fl_RGB_Image*)renderbg->copy();
        }
        else
        {
            renderbox->deimage();
            fl_imgtk::drawonimage( rendermux, renderbg );
        }
        
        if ( rendermux != NULL )
        {
#ifdef TEST_UPSCALING
            Fl_RGB_Image* dnscl = fl_imgtk::rescale( rendersurface,
                                                     rendersurface->w() * fsaa_ratio,
                                                     rendersurface->h() * fsaa_ratio,
                                                     fl_imgtk::BICUBIC );
            fl_imgtk::drawonimage( rendermux, dnscl );

            delete dnscl;
#else
            fl_imgtk::drawonimage( rendermux, rendersurface );
#endif 
        }
     
        renderbox->image( rendermux );
        renderbox->damage( 0 );

        snprintf( statestring, 1024,
                  " - vertex count   : %lu\n"
                  " - face count     : %lu\n"
                  " - texture coords : %lu\n"
                  " - move           : %.3f,%.3f,%.3f\n"
                  " - rotate         : %.3f,%.3f,%.3f\n"
                  " - scale          : %.3f,%.3f,%.3f\n"
                  " - light position : %.3f,%.3f,%.3f\n"
                  " - eye (camera)   : %.3f,%.3f,%.3f\n"
                  " - look at        : %.3f,%.3f,%.3f\n"
                  " - apsect ratio   : %.3f\n"
                  " - FOV            : %.3f\n"
                  " - Far plane      : %.3f\n"
                  " - Near plane     : %.3f\n"
                  " -----------------------------------\n"
                  " - Draw Perf.   : %lu ms.\n"
                  ,
                  vertexs,
                  faces,
                  textures,
                  objMove->x, objMove->y, objMove->z,
                  objRotate->x, objRotate->y, objRotate->z,
                  objScale->x, objScale->y, objScale->z,
                  renLight->x, renLight->y, renLight->z,
                  renEye->x, renEye->y, renEye->z,
                  renAt->x, renAt->y, renAt->z,
                  *renAspect,
                  *renFOV,
                  *renFar,
                  *renNear,
                  perfms
                );
    }
    else
    {
        snprintf( statestring, 1024, "state unknown" );
    }

    statebox->label( statestring );
    statebox->damage( 0 );
    window->redraw();
#ifndef __APPLE__
    Fl::flush();
#endif
}

void resetobj()
{
    SETDATA(objMove,    0.f,0.f,0.f);
    SETDATA(objRotate,  0.f,0.f,0.f);
    SETDATA(objScale,   1.5f,1.5f,1.5f);

    SETDATA(renLight,   0.0f,0.0f,-1.0f);
    SETDATA(renEye,     0.0f,0.0f,-3.0f);
    SETDATA(renAt,      0.0f,0.0f,0.0f);
    SETDATA(renUp,      0.0f,1.0f,0.0f);
}

void fl_timercb( void* p )
{
    if ( p != NULL )
    {
        if ( timerdemo == true )
        {
            if ( objRotate != NULL )
            {
                if ( objRotate->y > -360.f )
                    objRotate->y -= 2.f;
                else
                    objRotate->y = 360.f;
                
                updateRender();

                Fl::repeat_timeout( 0.025f, fl_timercb, &window );
            }
        }
    }
}

int fl_keyhandle( int e )
{
    if ( e == FL_SHORTCUT )
    {
        bool oredraw = false;
        int key = Fl::event_key();

        switch( key )
        {
            case FL_Up:
                if ( objRotate != NULL )
                {
                    if ( objRotate->x > -360.f )
                        objRotate->x -= 2.f;
                    else
                        objRotate->x = 360.f;
                    oredraw = true;
                }
                break;

            case FL_Down:
                if ( objRotate != NULL )
                {
                    if ( objRotate->x < 360.f )
                        objRotate->x += 2.f;
                    else
                        objRotate->x = -360.f;
                    oredraw = true;
                }
                break;

            case (unsigned)'=':
            case (unsigned)'+':
                if ( objScale != NULL )
                {
                    if ( objScale->x < 3.20f )
                        *objScale = *objScale * 1.01f;
                    oredraw = true;
                }
                break;

            case FL_Right:
                if ( objRotate != NULL )
                {
                    if ( objRotate->y < 360.f )
                        objRotate->y += 2.f;
                    oredraw = true;
                }
                break;
                
            case FL_Left:
                if ( objRotate != NULL )
                {
                    if ( objRotate->y > -360.f )
                        objRotate->y -= 2.f;
                    oredraw = true;
                }
                break;
            
            case (unsigned)'-':
            case (unsigned)'_':
                if ( objScale != NULL )
                {
                    if ( objScale->x > 0.f )
                        *objScale = *objScale * 0.99f;
                    oredraw = true;
                }
                break;

            case 49: /// '1'
                if ( timerdemo == true )
                {
                    timerdemo = false;
                }
                else
                {
                    timerdemo = true;
                    // MacOS has some issue on timer.
                    if ( Fl::has_timeout( fl_timercb, &window ) != 0 )
                        Fl::repeat_timeout( 0.025f, fl_timercb, &window );
                    else
                        Fl::add_timeout( 0.025f, fl_timercb, &window );
                }
                break;
                
            case 119: /// 'w'
                if ( objMove != NULL )
                {
                    if ( objMove->y > -3.f )
                        objMove->y -= 0.04f;
                    oredraw = true;
                }
                break;

            case 97: /// 'a'
                if ( objMove != NULL )
                {
                    if ( objMove->x < 3.f )
                        objMove->x += 0.04f;
                    oredraw = true;
                }
                break;

            case 100: /// 'd'
                if ( objMove != NULL )
                {
                    if ( objMove->x > -3.f )
                        objMove->x -= 0.04f;
                    oredraw = true;
                }
                break;
                
            case 108: /// 'l'
                if ( renderer != NULL )
                {
                    if ( renderer->line() == true )
                    {
                        renderer->line( false );
                    }
                    else
                    {
                        renderer->line( true );
                        
                        if ( renderer->texture() == true )
                        {
                            renderer->linecolor( DEF_COLOR_LINE_W_TEXTURE );
                        }
                        else
                        {
                            renderer->linecolor( DEF_COLOR_LINE_ONLY );
                        }
                    }

                    oredraw = true;
                }
                break;
                
            case 114: /// 'r'
                if ( objMove != NULL )
                {
                    resetobj();
                    oredraw = true;
                }
                break;

            case 115: /// 's'
                if ( objMove != NULL )
                {
                    if ( objMove->y < 3.f )
                        objMove->y += 0.04f;

                    oredraw = true;
                }
                break;
                
            case 116: /// 't'
                if ( renderer != NULL )
                {
                    if ( renderer->texture() == true )
                    {
                        renderer->texture( false );
                        renderer->linecolor( DEF_COLOR_LINE_ONLY );
                    }
                    else
                    {
                        renderer->texture( true );
                        renderer->linecolor( DEF_COLOR_LINE_W_TEXTURE );
                    }

                    oredraw = true;
                }
                break;
                
            case FL_Escape:
                if ( window != NULL )
                {
                    // quir program.
                    window->hide();
                }
                break;

        }

        if ( oredraw == true )
            updateRender();

        return 1;
    }
    
    return 0;
}

void fl_wcb( Fl_Widget* w )
{
    // may not called.
}

int main( int argc, char** argv )
{
    int refw = 1280;
    int refh = 720;
    
    window = new Fl_Double_Window ( refw, refh, "FLTK Software renderer demo, (C)2021 Raphael Kim" );
    renderbox = new Fl_Box( 0, 0, refw, refh );

    helpbox = new Fl_Box( 10, 10, refw, 120, 
              "object movement : W,A,S,D keys\n"
              "object rotate/scaling : arrow keys, object scale : + and - key\n"
              "texture on/off : t, line on/off : l, auto-rotate : 1, reset : r" );
    if ( helpbox != NULL )
    {
        helpbox->box( FL_NO_BOX );
        helpbox->align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE );
        helpbox->labelcolor( 0xFF993300 );
        helpbox->labelfont( FL_HELVETICA );
        helpbox->labelsize( 28 );
    }

    statebox = new Fl_Box( 20, 140, refw/2, refh - 140 );
    if ( statebox != NULL )
    {
        statebox->box( FL_NO_BOX );
        statebox->align( FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE );
        statebox->labelcolor( 0x70FF7000 );
        statebox->labelfont( FL_COURIER );
        statebox->labelsize( 20 );
    }

    window->end();
    
    Fl::add_handler( fl_keyhandle );
    
#ifdef TEST_UPSCALING    
    // upscaling test ..
    float upsclf = 1.f / fsaa_ratio;
    rendersurface = fl_imgtk::makeanempty( refw*upsclf, 
                                           refh*upsclf, 
                                           4, 0x000000 );
#else
    rendersurface = fl_imgtk::makeanempty( refw, refh, 4, 0x000000 );
#endif
    renderbg = fl_imgtk::makegradation_h( refw, refh, 0x505050FF, 0x101010FF, true );
    
    renderer = new FLSWRenderer( rendersurface );
    
    if ( renderer != NULL )
    {
        renderer->color( DEF_COLOR_OBJECT );
        renderer->linecolor( DEF_COLOR_LINE_W_TEXTURE );
        renderer->LoadObjects( "model/diablo3_pose.obj" );
        renderer->LoadTexture( "model/diablo3_pose.png" );
    }
    
    // config render
    objMove = renderer->shift();
    objRotate = renderer->rotate();
    objScale = renderer->scale();
    
    renLight = renderer->light();
    renEye = renderer->eye();
    renAt = renderer->lookat();
    renUp = renderer->upside();
    renAspect = renderer->aspectratio();
    renFOV = renderer->FOV();
    renFar = renderer->FarPlane();
    renNear = renderer->NearPlane();
    
    // some informations
    vertexs = renderer->vertexs();
    faces = renderer->faces();
    textures = renderer->texturecoords();

    // setting up
    resetobj();
       
    updateRender();
    window->show();

    // register timer
    Fl::add_timeout( 0.025f, fl_timercb, NULL );
    
    int ret = Fl::run();
    
    Fl::remove_timeout( fl_timercb );

    delete rendermux;
    delete renderbg;
    delete rendersurface;
    
    return ret;
}
