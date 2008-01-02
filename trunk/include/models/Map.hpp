
#ifndef _MODELS_MAP_HPP_
#define _MODELS_MAP_HPP_

#include <boost/multi_array.hpp>
#include <boost/lambda/lambda.hpp>
#include "models/Cube.hpp"
#include "MapSetting.hpp"

namespace psc{ namespace model{
    class Map{
    public:
        typedef multi_array<pCube, 2> container_type;
        static pMap create(pMapSetting setting){
            // map doesn't need a pool
            return pMap(new Map(setting));
        }
        Map(pMapSetting setting): setting_(setting),
         cubes_(extents[setting.width][setting.height])
        {
            init_cubes();
        }
        pMap cycle(){
            n_of_newcomers_ = 0;
            Cube* cubes = cubes_.origin();
            for(int i=--ms().size(), iend=-1; i!=iend; --i){
                if(Cube* s = cubes[i]){
                    if( !s->has_grounded() ) ++n_of_newcomers_;
                    if( s->cycle_and_die() ){
                        cubes[i] = NULL;
                    }
                }
            }
        	// n_of_newcomers_ = 0;

            // for(container_type::reverse_iterator i=data_.rbegin(), iend=data_.rend();
            //     i!=iend; ++i)
            // {
            //     if(Square* s = *i){
            //       if( !s->has_grounded() ) ++n_of_newcomers_;
            //       if( s->cycle_and_die() ){
            //         *i = NULL;
            //       }
            //     }
            // }
        }
        pMapSetting& ms() const{ return *setting_; }
    private:
        Map& init_cubes(){
            typedef multi_array<int, 2> vector_2d;
            using boost::lambda::_1;
            using boost::lambda::_2;
    		vector_2d square_colors(ms().width(), ms().starting_line());

            int const one_color_amounts = std::ceil(static_cast<double>(ms().width())*ms().starting_line()/ms().color_amounts());
            for_each(square_colors, _1 = _2/one_color_amounts);

            while(!is_gooood(square_colors, ms().chain_amounts()));
            for(int i=0, iend=square_colors.num_elements(); i!=iend; ++i)
            {
                insert(Cube::create(this, i%ms().width(), ms().height()-1-i/ms().width(), squares_colors[i]));
            }
            return *this;
        }
        Map& insert(pCube cube){
            cubes_[x][y] = cube;
        }

    private:
        pMapSetting setting_;
        container_type cubes_;
    };

}} // end of namespace

#endif
