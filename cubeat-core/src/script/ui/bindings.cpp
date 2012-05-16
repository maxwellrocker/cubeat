#include "view/Scene.hpp"
#include "view/Sprite.hpp"
#include "view/SpriteText.hpp"
#include "Accessors.hpp"
#include "EasingEquations.hpp"
#include "Input.hpp"

using namespace psc;
using namespace view;
using namespace ctrl;
using namespace accessor;
using namespace easing;

extern "C"{
#include "script/ui/bindings.h"
}

void delegate_for_cb_from_lua(pSprite sp, PSC_OBJCALLBACK func) {
    func(&sp);
}

void delegate_for_cb_from_lua_with_parameter(pSprite sp, PSC_OBJCALLBACK_WITH_PARA func, int a, int b) {
    func(&sp, a, b);
}

void Sprite_set_texture(pSprite* self, const char* path) {
    (*self)->setTexture(path);
}

void Sprite_set_pos(pSprite* self, double x, double y) {
    (*self)->set<Pos2D>(vec2(x, y));
}

void Sprite_set_rotation(pSprite* self, double deg) {
    (*self)->set<Rotation>(vec3(0, 0, deg));
}

void Sprite_set_size(pSprite* self, double w, double h) {
    (*self)->set<Size2D>(vec2(w, h));
}

void Sprite_set_depth(pSprite* self, double depth) {
    (*self)->setDepth(depth);
}

void Sprite_set_color(pSprite* self, int r, int g, int b) {
    (*self)->set<Red>(r);
    (*self)->set<Green>(g);
    (*self)->set<Blue>(b);
}

void Sprite_set_red(pSprite* self, int r) {
    (*self)->set<Red>(r);
}

void Sprite_set_green(pSprite* self, int g) {
    (*self)->set<Green>(g);
}

void Sprite_set_blue(pSprite* self, int b) {
    (*self)->set<Blue>(b);
}

void Sprite_set_alpha(pSprite* self, int alpha) {
    (*self)->set<Alpha>(alpha);
}

void Sprite_set_visible(pSprite* self, bool visible) {
    (*self)->set<Visible>(visible);
}

void Sprite_set_center_aligned(pSprite* self, bool center) {
    (*self)->setCenterAligned(center);
}

void Sprite_tween_elastic_pos(pSprite* self, v2* s, v2* e, unsigned int dur, int loop, PSC_OBJCALLBACK cb, int delay) {
    std::tr1::function<void()> call = 0;
    if( cb ) {
        call = bind(delegate_for_cb_from_lua, (*self), cb);
    }
    (*self)->tween<OElastic, Pos2D>(vec2(s->x, s->y), vec2(e->x, e->y), dur, loop, call, delay);
}

void Sprite_tween_isine_pos(pSprite* self, v2* s, v2* e, unsigned int dur, int loop, PSC_OBJCALLBACK cb, int delay) {
    std::tr1::function<void()> call = 0;
    if( cb ) {
        call = bind(delegate_for_cb_from_lua, (*self), cb);
    }
    (*self)->tween<ISine, Pos2D>(vec2(s->x, s->y), vec2(e->x, e->y), dur, loop, call, delay);
}

void Sprite_tween_osine_pos(pSprite* self, v2* s, v2* e, unsigned int dur, int loop, PSC_OBJCALLBACK cb, int delay) {
    std::tr1::function<void()> call = 0;
    if( cb ) {
        call = bind(delegate_for_cb_from_lua, (*self), cb);
    }
    (*self)->tween<OSine, Pos2D>(vec2(s->x, s->y), vec2(e->x, e->y), dur, loop, call, delay);
}

void Sprite_tween_linear_alpha(pSprite* self, int s, int e, unsigned int duration, int loop, PSC_OBJCALLBACK cb, int delay) {
    std::tr1::function<void()> call = 0;
    if( cb ) {
        call = bind(delegate_for_cb_from_lua, (*self), cb);
    }
    (*self)->tween<Linear, Alpha>(s, e, duration, loop, call, delay);
}

void Sprite_on_release(pSprite* self, Button const* btn, PSC_OBJCALLBACK func) {
    (*self)->onRelease( btn ) = bind(delegate_for_cb_from_lua, _1, func);
}

void Sprite_on_press(pSprite* self, Button const* btn, PSC_OBJCALLBACK func) {
    (*self)->onPress( btn ) = bind(delegate_for_cb_from_lua, _1, func);
}

void Sprite_on_enter_focus(pSprite* self, Input const* p, PSC_OBJCALLBACK_WITH_PARA func) {
    (*self)->onEnterFocus( p ) = bind(delegate_for_cb_from_lua_with_parameter, _1, func, _2, _3);
}

void Sprite_on_leave_focus(pSprite* self, Input const* p, PSC_OBJCALLBACK_WITH_PARA func) {
    (*self)->onLeaveFocus( p ) = bind(delegate_for_cb_from_lua_with_parameter, _1, func, _2, _3);
}

void SpriteText_set_pos(pSpriteText* self, double x, double y) {
    (*self)->set<Pos2D>(vec2(x, y));
}

void SpriteText_set_scale(pSpriteText* self, double uniform_scale) {
    (*self)->set<Scale>(vec3(uniform_scale, uniform_scale, uniform_scale));
}

void SpriteText_set_depth(pSpriteText* self, double depth) {
    (*self)->setDepth(depth);
}

void SpriteText_set_color(pSpriteText* self, int r, int g, int b) {
    (*self)->set<Red>(r);
    (*self)->set<Green>(g);
    (*self)->set<Blue>(b);
}

void SpriteText_set_red(pSpriteText* self, int r) {
    (*self)->set<Red>(r);
}

void SpriteText_set_green(pSpriteText* self, int g) {
    (*self)->set<Green>(g);
}

void SpriteText_set_blue(pSpriteText* self, int b) {
    (*self)->set<Blue>(b);
}

void SpriteText_set_alpha(pSpriteText* self, int alpha) {
    (*self)->set<Alpha>(alpha);
}

void SpriteText_set_visible(pSpriteText* self, bool visible) {
    (*self)->set<Visible>(visible);
    (*self)->setPickable( visible ? true : false ); // I had to add this? why?
}

void SpriteText_set_center_aligned(pSpriteText* self, bool center) {
    (*self)->setCenterAligned(center);
}

void SpriteText_on_tween_line_alpha(pSpriteText* self, int alpha, double duration, int loop, PSC_OBJCALLBACK cb, int delay) {
    std::tr1::function<void()> const& call = bind(delegate_for_cb_from_lua, (*self), cb);
    (*self)->tween<Linear, Alpha>(alpha, duration, loop, call, delay);
}

pSprite* Sprite_create(char const* name, pObject* parent, int w, int h, bool center) {
    pSprite* sp = new pSprite;
    *sp = Sprite::create(name, *parent, w, h, center);
    return sp;
}

pSpriteText* SpriteText_create(char const* text, pObject* parent, char const* f, int size, bool center, int r, int g, int b) {
    pSpriteText* sp = new pSpriteText;
    *sp = SpriteText::create(text, *parent, f, size, center, data::Color(r,g,b));
    return sp;
}

Input* Input_get_input1() {
    return InputMgr::i().getInputByIndex(0);
}

Input* Input_get_input2() {
    return InputMgr::i().getInputByIndex(1);
}

Button const* Input_get_trig1(Input* p){
    return &p->trig1();
}

Button const* Input_get_trig2(Input* p){
    return &p->trig2();
}

void Scene__gc(pScene* self) {
    delete self;
}

void Sprite__gc(pSprite* self) {
    delete self;
}

void SpriteText__gc(pSpriteText* self) {
    delete self;
}