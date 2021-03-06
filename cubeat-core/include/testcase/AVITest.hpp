#ifndef _SHOOTING_CUBES_AVI_TEST_
#define _SHOOTING_CUBES_AVI_TEST_

#include "IrrDevice.hpp"
#include "EventDispatcher.hpp"
#include "view/SpriteMovie.hpp"

/*******************************************************************/
/* 2011.04.18                                                      */
/* This testcase will compile but not working because AVI support  */
/* is currently removed.                                           */
/*******************************************************************/

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;

using namespace psc;
using namespace ctrl;

struct AVITest
{
public:
    static AVITest& i() {
        static AVITest singleton;
        return singleton;
    }
#ifdef WIN32
    AVITest() {
        scene = view::Scene::create(view::pObject(), "AVITest");
        scene->setTo2DView();

        movie = view::SpriteMovie::create("E:/portfolio/ndssp/ndssp_op.avi", scene, 640, 480);
        movie->stop().moveTo(320, 240).setCenterAligned(true);
        EventDispatcher::i().get_timer_dispatcher("AVITest")->subscribe( bind(&view::SpriteMovie::play, movie.get()), 6000);
        EventDispatcher::i().get_timer_dispatcher("AVITest")->subscribe( bind(&view::SpriteMovie::restart, movie.get()), 8500);
        EventDispatcher::i().get_timer_dispatcher("AVITest")->subscribe( bind(&view::SpriteMovie::stop, movie.get()), 10500);
        EventDispatcher::i().get_timer_dispatcher("AVITest")->subscribe( bind(&view::SpriteMovie::play, movie.get()), 11500);
        EventDispatcher::i().get_timer_dispatcher("AVITest")->subscribe( bind(&view::SpriteMovie::setCurrentFrame, movie.get(), 200), 13000);
    }
#endif

    void cycle() {
#ifdef WIN32
        movie->redraw();
        scene->activate().redraw();
        scene->deactivate();
#endif
    }

private:
#ifdef WIN32
    view::pSpriteMovie movie;
#endif
    view::pScene scene;
};

#endif // _SHOOTING_CUBES_AVI_TEST_
