
#ifndef _SHOOTING_CUBES_SPEEDFUNC_ANIMATOR_
#define _SHOOTING_CUBES_SPEEDFUNC_ANIMATOR_

#include "Accessor_proto.hpp"
#include "view/detail/CustomAnimator.hpp"

//for debug & warning purpose
#include <iostream>

#include <vector>
#include <limits>

namespace irr{
namespace scene{

template <template <class> class Eq, class Acc>
class SpeedFuncAnimator : public CustomAnimator<Eq, Acc>
{
    typedef CustomAnimator<Eq, Acc>            SpeedFuncAnimatorBase;
    typedef typename SpeedFuncAnimatorBase::T  T;

public:
    SpeedFuncAnimator(ISceneManager* smgr, T const& start, T const& end,
                      std::tr1::function<float()>const& speedfunc, int const& loop = 0,
                      std::tr1::function<void()>const& cb = 0, s32 const& delayTime = 0)
        :SpeedFuncAnimatorBase(smgr, start, end, 0, loop, cb, delayTime),
         pos_in_time_(0.f), speedfunc_(speedfunc)
    {
        cur_speed_ = speedfunc_();
        if( cur_speed_ > 0 ) {
            this->duration_ = static_cast<u32>(this->distance(this->end_, this->start_) / cur_speed_ * 1000.f);
            last_speed_ = cur_speed_;
        }
        else {
            this->duration_ = std::numeric_limits<u32>::max();
            last_speed_ = 0.f;
        }
    }

    virtual ~SpeedFuncAnimator(){}

    virtual void animateNode(ISceneNode* node, u32 timeMs) {
        if ( !node ) return;
        if ( static_cast<s32>(timeMs) < this->startTime_ ) return;

        cur_speed_ = speedfunc_();
        if( cur_speed_ > std::numeric_limits<float>::epsilon() ) {
            if( cur_speed_ != last_speed_ ) {
                int time_ahead =
                    static_cast<int>((1000.f/IrrDevice::i().d()->getVideoDriver()->getFPS())+0.5f);
                this->duration_ =
                    static_cast<u32>(this->distance(this->end_, this->start_) / cur_speed_ * 1000.f);
                this->startTime_ =
                    timeMs - time_ahead - static_cast<s32>(pos_in_time_ * this->duration_);
                last_speed_ = cur_speed_;
            }
        }
        else return;
        pos_in_time_ = static_cast<float>(timeMs - this->startTime_) / this->duration_;

        if( timeMs - this->startTime_ >= this->duration_ ) {
            /* we can add periodic callback here.
            if( this->periodic_cb() ) this->periodic_callback(); */
            this->startTime_ = timeMs; //or should be += this->duration_ ?
            if( this->loop_ == 0 ) {
                T const& pos = Eq<T>::calculate(1, this->start_, this->distance_, 1, node);
                Acc::set(node, pos);
                this->smgr_->addToAnimatorDeletionQueue(this, node);
                if( this->cb_ ) this->cb_();
                return;   //DAMN HOW CAN I FORGET THIS!!!!!!
            }
            else if( this->loop_ > 0 ) this->loop_ -= 1;
        }

        T const& pos = this->updatePos(node, timeMs);
        Acc::set(node, pos);
    }

protected:
    float pos_in_time_;
    float cur_speed_, last_speed_;
    std::tr1::function<float()> speedfunc_;
};

} // scene
} // irr

#endif
