
/*2007.11.9 
  Nothing changed.
*/

#ifndef _SHOOTING_CUBE_COLOR_HPP_
#define _SHOOTING_CUBE_COLOR_HPP_

namespace psc{

class Color{
public:
    static Color from_id(int i, int limit = 6){
        int rgb;
        switch(i){
            case 0: rgb = from_id(random_id(limit)); break;
            case 1: rgb = blue;       break;
            case 2: rgb = green;      break;
            case 3: rgb = red;        break;
            case 4: rgb = green_blue; break;
            case 5: rgb = red_blue;   break;
            case 6: rgb = red_green;  break;
            case 7: rgb = 0;          break;
            case 8: rgb = blue|green|red;
        }
        /* i would like this:
        int rgb = case i
                    when 0; random_color
                    when 1; blue
                    when 2; green
                    when 3: red
                    ...
                  end
        */
        return Color(rgb);
    }

public:
    Color(): rgb_(0){}
    explicit Color(int rgb): rgb_(rgb){}
    Color(int r, int g, int b):
        rgb_( r<<16 | g<<8 | b )
    {}

    int rgb() const{ return rgb_; }
    int r() const{ return (rgb_&red) >> 16; }
    int g() const{ return (rgb_&green) >> 8; }
    int b() const{ return rgb_&blue; }

    Color& rgb(int new_rgb){ rgb_ = new_rgb; return *this; }
    Color& r(int new_r){ rgb_ &= green_blue; rgb_ |= new_r<<16; return *this; }
    Color& g(int new_g){ rgb_ &= red_blue; rgb_ |= new_g<<8; return *this; }
    Color& b(int new_b){ rgb_ &= red_green; rgb_ |= new_b; return *this; }

private:
    static int random_id(int limit){
        return App::i().random(limit)+1;
    }

private:
    int rgb_;
    enum{
        blue=0xff, green=blue<<8, red=green<<8,
        green_blue = green|blue,
        red_blue = red|blue,
        red_green = red|green
    };
};

inline bool operator==(Color const& lhs, Color const& rhs){ return lhs.rgb() == rhs.rgb(); }
inline bool operator!=(Color const& lhs, Color const& rhs){ return lhs.rgb() != rhs.rgb(); }

} // end of namespace

#endif