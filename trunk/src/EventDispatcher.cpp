
/* EventDispatcher Implement,
   blah
*/

#include "EventDispatcher.hpp"
#include "Input.hpp"
#include "IrrDevice.hpp"
#include "view/Sprite.hpp"
#include "view/Scene.hpp"

#include <boost/foreach.hpp>
#include <algorithm>
#include <iostream>
#include <utility>

using std::tr1::tie;
using std::tr1::get;
using std::make_pair;

using irr::ITimer;

using namespace irr;
using namespace core;
using namespace scene;

using namespace psc;
using namespace ctrl;

EventDispatcher::EventDispatcher()
{
}

EventDispatcher&
EventDispatcher::subscribe_btn_event(BtnCallback cb, Button const* btn, BSTATE state)
{
    btn_listeners_.push_back( tie( cb, btn, state ) );
    return *this;
}

EventDispatcher&
EventDispatcher::subscribe_timer(TimerCallback cb, int duration, bool loop)
{
    const int zero = 0;
    newly_created_timers_.push_back( tie( cb, duration, zero, loop ) );
    return *this;
}

EventDispatcher&
EventDispatcher::subscribe_obj_event(ObjCallback ocb, Button const* btn, view::pSprite const& obj)
{   //SceneListenerPair::first = const wpScene, SceneListenerPair::second = ObjListener
    view::wpScene scene = obj->scene();
    if( scene_listeners_.find( scene ) != scene_listeners_.end() )
        scene_listeners_[ scene ].push_back( tie( ocb, btn, obj ) );
    else {
        typedef std::pair<SceneListener::iterator, bool> InsertionPair;
        InsertionPair p = scene_listeners_.insert( make_pair( scene, ObjListener() ) );
        (p.first)->second.push_back( tie(ocb, btn, obj) );
    }
    return *this;
}

//TODO:
//Event dispatching need to be extended to other button state: Release, Up, Down
void EventDispatcher::dispatch_obj()
{
    BOOST_FOREACH(SceneListenerPair& slp, scene_listeners_)
    {   //SceneListenerPair::first = const wpScene, SceneListenerPair::second = ObjListener
        view::pScene scene = slp.first.lock();
        if( !scene ) {
            scene_expired_.push_back( slp.first );
            continue;
        }
        ObjListener& listeners       = slp.second;
        ISceneCollisionManager* colm = scene->getCollisionMgr();
        std::map<Input const*, ISceneNode*> pickmap;

        BOOST_FOREACH(Input const* input, Input::getInputs()) {
            position2di pos(input->cursor().x(), input->cursor().y());
            ISceneNode* picked = colm->getSceneNodeFromScreenCoordinatesBB(pos, 1, true);
            if( picked )
                pickmap.insert( std::make_pair(input, picked) );
        }
        for(ObjListener::iterator o = listeners.begin(), oend = listeners.end(); o != oend; ++o) {
            if( view::pSprite sv = get<OE::SPRITE>(*o).lock() ) { //sprite not expired
                Button const* btn = get<OE::BTN>(*o);
                if( btn->state() != BTN_PRESS )                   //not right state
                    continue;

                if( pickmap[ btn->owner() ] == sv->body() ) {
                    get<OE::OBJ_CB>(*o)(sv);                      //lastly, fire up callback
                    std::cout << "dispatcher trace: " << pickmap[ btn->owner() ] << "\n";
                    break;
                }
            }
            else
                obj_events_to_be_deleted_.insert( std::make_pair(slp.first, o) );
        }
    }
    cleanup_obj_event();
}

void EventDispatcher::dispatch_btn(){
    BOOST_FOREACH(BtnEvent& b, btn_listeners_) {
        Button const* btn = get<BUTTON>(b);
        if( btn->state() != get<STATE>(b) ) continue;
        get<BCALLBACK>(b)( btn->owner()->cursor().x(), btn->owner()->cursor().y() );
    }
}

void EventDispatcher::dispatch_timer(){

    cleanup_timer_and_init_newly_created_timer();

    irr::u32 now = IrrDevice::i().d()->getTimer()->getRealTime();
    for(TimerList::iterator t = timers_.begin(), tend = timers_.end(); t != tend; ++t) {
        if( now - get<LASTTIME>(*t) >= get<DURATION>(*t) ) {
            get<TCALLBACK>(*t)();
            get<LASTTIME>(*t) = now;
            if( get<LOOP>(*t) == false ) {
                timers_to_be_deleted_.push_back(t);
            }
        }
    }
}

void EventDispatcher::dispatch()
{
    dispatch_btn();
    dispatch_obj();
    dispatch_timer();
}

void EventDispatcher::cleanup_timer_and_init_newly_created_timer()
{
    // clean up
    BOOST_FOREACH(TimerList::iterator t, timers_to_be_deleted_) {
        timers_.erase(t);
    }
    timers_to_be_deleted_.clear();

    // init newly created
    std::time_t init_time = IrrDevice::i().d()->getTimer()->getRealTime();
    BOOST_FOREACH(Timer& timer, newly_created_timers_){
        get<LASTTIME>(timer) = init_time;
    }

    timers_.insert(timers_.end(),
                   newly_created_timers_.begin(),
                   newly_created_timers_.end());
    newly_created_timers_.clear();
}

void EventDispatcher::cleanup_obj_event()
{
    BOOST_FOREACH(ObjEventRemovalPair& o, obj_events_to_be_deleted_) {
        scene_listeners_[ o.first ].erase( o.second );
    }
    obj_events_to_be_deleted_.clear();

    BOOST_FOREACH(SceneListener::key_type& s, scene_expired_) {
        scene_listeners_.erase(s);
    }
    scene_expired_.clear();
}
