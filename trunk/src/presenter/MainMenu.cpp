
#include "presenter/MainMenu.hpp"
#include "view/Scene.hpp"
#include "view/Menu.hpp"
#include "view/Sprite.hpp"
#include "data/Color.hpp"

#include "EasingEquations.hpp"
#include "Accessors.hpp"
#include "App.hpp"

#include <boost/tr1/functional.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <utility>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iostream>

using namespace irr;
using namespace core;
using namespace scene;
using namespace accessor;

using namespace psc;
using namespace presenter;
using namespace easing;

using std::tr1::bind;
using std::tr1::ref;
using std::tr1::function;
using namespace std::tr1::placeholders;

namespace BLL = boost::lambda;

pMainMenu MainMenu::init()
{
    App::i().setLoading(1);
    mainmenu_scene_ = view::Scene::create(view::pObject(), "MainMenu");
    mainmenu_scene_->setTo2DView();

    view::pMenu temp = view::Menu::create("title", mainmenu_scene_, 150, 60);
    view::pMenu temp2= view::Menu::create("title", mainmenu_scene_, 150, 60);

    menus_.insert( std::make_pair("testmenu1", temp) );
    menus_.insert( std::make_pair("testmenu2", temp2) );

    temp->moveTo(-200, 100);
    temp2->moveTo(-200, 100).set<Red>(0);

    function<void(view::pSprite&)> click1_1 = bind(&MainMenu::menu1_1_click, this, _1);
    function<void(view::pSprite&)> click2_1 = bind(&MainMenu::menu2_1_click, this, _1);

    temp->addSprite("title", click1_1, 100, 40).getSprite("title").set<Pos2D>(vector2df(0, 50));
    temp2->addSprite("title", click2_1, 100, 40).getSprite("title").set<Pos2D>(vector2df(0, 50));

    temp->set<Alpha>(0);
    temp->set<Visible>(false);
    temp2->set<Alpha>(0);
    temp2->set<Visible>(false);

    initDecorator();
    App::i().setLoading(100);
    showMenu("testmenu1");

    return shared_from_this();
}

void MainMenu::initDecorator()
{
    int const w = 640;
    int const h = 480;
    int const outgoing = 80;
    int const contract = 30;
    vector2df start1( -outgoing, contract),  end1(w+outgoing, contract);     //line1
    vector2df start2(w-contract, -outgoing), end2(w-contract, h+outgoing);   //line2
    vector2df start3(w+outgoing, h-contract),end3( -outgoing, h-contract);   //line3
    vector2df start4( contract,  h+outgoing),end4( contract,  -outgoing);    //line4
    int const bias = 30;
    int const num_in_w = 12;
    int const num_in_h = 9;
    int const color_num= 4;
    int const time_w = 10000;
    int const time_h = 7500;
    std::vector<std::string> paths;

    for(int i=1; i <= 4; ++i)
        paths.push_back( std::string("cubes/cube") +
            std::string( boost::lexical_cast<std::string>(i) ) );

    initDecoInner_( start1, end1, num_in_w, color_num, time_w, bias, paths );
    std::cout << "MainMenu deco: top line done.\n";
    initDecoInner_( start2, end2, num_in_h, color_num, time_h, bias, paths );
    std::cout << "MainMenu deco: right line done.\n";
    initDecoInner_( start3, end3, num_in_w, color_num, time_w, bias, paths );
    std::cout << "MainMenu deco: bottom line done.\n";
    initDecoInner_( start4, end4, num_in_h, color_num, time_h, bias, paths );
    std::cout << "MainMenu deco: left line done.\n";
}

void MainMenu::initDecoInner_(vector2df const& from, vector2df const& dest, int const& num,
                             int const& color_num, int const& time, int const& bias,
                             std::vector<std::string> const& paths)
{
    int const length = (dest - from).getLength();
    int const gap = length / num;

    for(int i=0, range=0; i < num; ++i, range += gap)
    {
        data::Color col = data::Color::from_id(0, color_num);
        int rand_bias   = utils::random( bias ) - bias/2;
        vector3df from3d( from.X + rand_bias, -from.Y + rand_bias, 400 );
        vector3df dest3d( dest.X + rand_bias, -dest.Y + rand_bias, -400 );
        view::pSprite temp = view::Sprite::create( paths[ utils::random(4) ], mainmenu_scene_, 100, 100, true);

        float scale = utils::random(50)/100.0f + 1;
        float rot_duration = 3000 + utils::random(2000);
        float delay = utils::random( rot_duration );
        float time_position = range * time / length;
        temp->set<ColorDiffuse>( 0xff000000 | col.rgb() )
             .set<Scale>(vector3df(scale, scale, 1))
             .tween<Linear, Rotation>(vector3df(0, 0, 360), rot_duration, true, 0, -delay )
             .tween<Linear, Pos3D>(from3d, dest3d, time, true, 0, -time_position );

        if( col.r() > 0 )
            temp->tween<Linear, Red>(  col.r()>>2, col.r(), time, true, 0, -time_position );
        if( col.g() > 0 )
            temp->tween<Linear, Green>(col.g()>>2, col.g(), time, true, 0, -time_position );
        if( col.b() > 0 )
            temp->tween<Linear, Blue>( col.b()>>2, col.b(), time, true, 0, -time_position );

        deco_cubes_.push_back( temp );
    }
}

void MainMenu::menu1_1_click(view::pSprite& sprite)
{
    if( animating_ ) return;
    animating_ = true;
    showMenu("testmenu2").hideMenu("testmenu1");
}

void MainMenu::menu2_1_click(view::pSprite& sprite)
{
    if( animating_ ) return;
    animating_ = true;
    showMenu("testmenu1").hideMenu("testmenu2");
}

MainMenu& MainMenu::showMenu(std::string const& name)
{
    view::pMenu sprite = menus_[name];
    boost::function<void()> f = (BLL::var(animating_) = false);
    sprite->tween<OSine, Pos2D>(vector2df(300, 100), 1000, false, f );
    sprite->tweenAll<Linear, Alpha>(255, 777, false);
    sprite->set<Visible>(true);
    return *this;
}

MainMenu& MainMenu::hideMenu(std::string const& name)
{
    view::pMenu sprite = menus_[name];
    function<void()> endcall = bind(&view::Sprite::set<Visible>, sprite.get(), false);
    sprite->tween<ISine, Pos2D>(vector2df(-200, 100), 1000, false, endcall);
    sprite->tweenAll<Linear, Alpha>(0, 777, false);
    return *this;
}

MainMenu& MainMenu::cubeRearrange()
{
    return *this;
}

void MainMenu::cycle()
{
    mainmenu_scene_->activate().redraw();
    mainmenu_scene_->deactivate();
}
