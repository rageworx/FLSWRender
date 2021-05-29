#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <fl_imgtk.h>

#include "FLSWRenderer.H"

Fl_Double_Window*   window = NULL;
Fl_Box*             renderbox = NULL;
Fl_RGB_Image*       rendersurface = NULL;
Fl_RGB_Image*       renderbg = NULL;
Fl_RGB_Image*       rendermux = NULL;
FLSWRenderer*       renderer = NULL;

static sceneparam* sp    = NULL;
#ifdef TEST_UPSCALING
static float fsaa_ratio  = 1.5f;
#endif // TEST_UPSCALING

void updateRender()
{
    if ( renderer != NULL )
    {
        renderer->Render();
        
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
        renderbox->redraw();
    }
}

int fl_keyhandle( int e )
{
    if ( e == FL_SHORTCUT )
    {
        int key = Fl::event_key();

        switch( key )
        {
            case FL_Up:
                if ( sp != NULL )
                {
                    sp->meshScale = sp->meshScale * 1.01f;
                    
                    updateRender();
                }
                break;
                
            case FL_Left:
                if ( sp != NULL )
                {
                    sp->meshRotate.y += 2.f;
                    
                    updateRender();
                }
                break;
                
            case FL_Right:
                if ( sp != NULL )
                {
                    sp->meshRotate.y -= 2.f;
                    
                    updateRender();
                }
                break;
            
            case FL_Down:
                if ( sp != NULL )
                {
                    sp->meshScale = sp->meshScale * 0.99f;
                    
                    updateRender();
                }
                break;
                
            case 119: /// 'w'
                if ( sp != NULL )
                {
                    if ( sp->meshMove.y < 1.f )
                        sp->meshMove.y += 0.04f;
                        
                    updateRender();
                }
                break;

            case 97: /// 'a'
                if ( sp != NULL )
                {
                    if ( sp->meshMove.x < 1.f )
                        sp->meshMove.x += 0.04f;
                        
                    updateRender();
                }
                break;

            case 100: /// 'd'
                if ( sp != NULL )
                {
                    if ( sp->meshMove.x > 0.f )
                        sp->meshMove.x -= 0.04f;
                        
                    updateRender();
                }
                break;

            case 115: /// 's'
                if ( sp != NULL )
                {
                    if ( sp->meshMove.y > 0.f )
                        sp->meshMove.y -= 0.04f;
                        
                    updateRender();
                }
                break;
                
            case FL_Escape:
                if ( window != NULL )
                {
                    // quir program.
                    window->hide();
                }

        }
        return 1;
    }
    
    return 0;
}

void fl_wcb( Fl_Widget* w )
{
}

int main( int argc, char** argv )
{
    int refw = 1280;
    int refh = 720;
    
    window = new Fl_Double_Window ( refw, refh, "FLTK Software renderer demo, (C)2021 Raphael Kim" );
    renderbox = new Fl_Box( 0, 0, refw, refh );
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
    renderbg = fl_imgtk::makegradation_h( refw, refh, 0xCFCFCFFF, 0x505050FF, true );
    
    renderer = new FLSWRenderer( rendersurface );
    
    if ( renderer != NULL )
    {
        renderer->LoadObjects( "model/diablo3_pose.obj" );
        renderer->LoadTexture( "model/diablo3_pose.png" );
    }
    
    // config render
    sp = renderer->parameter();
    
    if ( sp != NULL )
    {
        sp->meshMove   = {0.0f,0.0f,0.0f};
        sp->meshRotate = {0.f, 180.f ,180.f};
        sp->meshScale  = {2.f,2.f,2.f};

        sp->light      = {0.0f,0.0f,-1.0f};
        sp->eye        = {0.0f,0.0f,-3.0f};
        sp->at         = {0.0f,0.0f,0.0f}; 
        sp->up         = {0.0f,1.0f,0.0f}; 
    }
    
    updateRender();
    window->show();
    
    int ret = Fl::run();
    
    delete rendermux;
    delete renderbg;
    delete rendersurface;
    
    return ret;
}
