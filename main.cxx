#include<cassert>
#include<random>
#include<cstring>
#include<set>
#include<unordered_map>
#include<unordered_set>
#include<memory>
#include<iostream>

#include<SDL2/SDL.h>
#include<SDL2/SDL_ttf.h>

typedef char byte;
typedef unsigned char ubyte;
typedef unsigned short ushort;
typedef unsigned int uint;

struct range
{
    ushort s,e;
    bool valid()
    {
        return e>=s;
    }
};

bool operator<(const range&l,const range&r)
{
    return l.e<r.s;
}

std::set<range>::size_type count(std::set<range>& s,range r)
{
    auto pair=s.equal_range(r);
    std::set<range>::size_type sz;
    
    for(sz=0;pair.first!=pair.second;++pair.first,++sz);
    return sz;
}

class Map
{
public:
    Map(ushort width,ushort height)
        :width(width),height(height),map(new block[width*height])
    {
        memset(map,0,sizeof(block)*width*height);
    }
    ~Map()
    {
        delete[]map;
    }
    void clear()
    {
        memset(map,0,sizeof(block)*width*height);
    }
    void random(double percent)
    {
        typedef std::random_device rd_t;

        rd_t rd;

        for(uint i=0;i<width*height;++i)
            map[i].blocked=(rd()<rd_t::min()+(rd_t::max()-rd_t::min())*percent);
    }
    void scan()
    {
        areax.clear();
        areay.clear();

        uint idx=0,idy=0;
        std::set<range>prev_range;
        std::set<range>next_range;
        range r;

        for(ushort y=0;y<height;++y)
        {
            for(r.s=0;r.s<width;++r.s)
            {
                if(get(r.s,y))continue;
                for(r.e=r.s+1;r.e<width&&!get(r.e,y);++r.e);
                --r.e;
                next_range.insert(r);
                r.s=r.e+1;
            }
            for(auto in=next_range.begin();in!=next_range.end();in++)
            {
                auto cp=count(prev_range,*in);
                std::unordered_map<uint,area>::iterator i;
                if(cp>1||cp==0)
                {
                    i=areay.insert(std::make_pair(++idy,area())).first;
                    i->second.s=y;
                    i->second.ss=in->s;
                    i->second.se=in->e;
                }
                else
                {
                    auto ip=prev_range.find(*in);
                    auto cn=count(next_range,*ip);
                    assert(cn!=0);
                    if(cn>1)
                    {
                        i=areay.insert(std::make_pair(++idy,area())).first;
                        i->second.s=y;
                        i->second.ss=in->s;
                        i->second.se=in->e;
                    }
                    else
                    {
                        i=areay.find(at(ip->s,y-1).idy);
                    }
                }
                i->second.e=y;
                i->second.es=in->s;
                i->second.ee=in->e;
                for(ushort x=in->s;x<=in->e;++x)at(x,y).idy=i->first;
            }

            std::swap(prev_range,next_range);
            next_range.clear();
        }

        prev_range.clear();

        for(ushort x=0;x<width;++x)
        {
            for(r.s=0;r.s<height;++r.s)
            {
                if(get(x,r.s))continue;
                for(r.e=r.s+1;r.e<height&&!get(x,r.e);++r.e);
                --r.e;
                next_range.insert(r);
                r.s=r.e+1;
            }
            for(auto in=next_range.begin();in!=next_range.end();in++)
            {
                auto cp=count(prev_range,*in);
                std::unordered_map<uint,area>::iterator i;
                if(cp>1||cp==0)
                {
                    i=areax.insert(std::make_pair(++idx,area())).first;
                    i->second.s=x;
                    i->second.ss=in->s;
                    i->second.se=in->e;
                }
                else
                {
                    auto ip=prev_range.find(*in);
                    auto cn=count(next_range,*ip);
                    assert(cn!=0);
                    if(cn>1)
                    {
                        i=areax.insert(std::make_pair(++idx,area())).first;
                        i->second.s=x;
                        i->second.ss=in->s;
                        i->second.se=in->e;
                    }
                    else
                    {
                        i=areax.find(at(x-1,ip->s).idx);
                    }
                }
                i->second.e=x;
                i->second.es=in->s;
                i->second.ee=in->e;
                for(ushort y=in->s;y<=in->e;++y)at(x,y).idx=i->first;
            }

            std::swap(prev_range,next_range);
            next_range.clear();
        }

    }
    bool get(ushort x,ushort y)const
    {
        return at(x,y).blocked;
    }
    uint getX(ushort x,ushort y)const
    {
        return at(x,y).idx;
    }
    uint getY(ushort x,ushort y)const
    {
        return at(x,y).idy;
    }
    bool set(ushort x,ushort y,bool blocked)
    {
        bool ori=at(x,y).blocked;
        at(x,y).blocked=blocked;
        return ori;
    }
    const ushort width,height;
private:
    struct block
    {
        uint idx,idy;
        bool blocked;
    }*const map;
    block& at(ushort x,ushort y)const
    {
        assert(x<width&&y<height);
        return map[x*height+y];
    }
    struct area
    {
        ushort s,e;
        ushort ss,se;
        ushort es,ee;
        uint len;
        ushort sm,em;
        bool direct;

        uint prev;

        std::unordered_set<uint>snear,enear;
        
        enum:ubyte
        {
            N=0,S,E
        };

        ubyte near(uint id)
        {
            if(snear.find(id)!=snear.end())return S;
            else if(enear.find(id)!=enear.end())return E;
            else return N;
        }

        bool valid(bool mid=false)
        {
            if(mid)return s<=e&&ss<=sm&&sm<=se&&es<=em&&em<=ee;
            else return s<=e&&ss<=se&&es<=ee;
        }
    };

    std::unordered_map<uint,area>areax,areay;

};

struct rgba
{
    rgba(uint color)
    {
        r=color&0xff;
        g=(color>>=8)&0xff;
        b=(color>>=8)&0xff;
        a=(color>>=8)&0xff;
    }
    ubyte r,g,b,a;
};
class Rainbow
{
public:
    Rainbow(uint seed=0):re(seed)
    {
        colors.push_back(0);
    }
    rgba get(ushort id)
    {
        while(id>=colors.size())colors.push_back(dis(re));
        return rgba(colors[id]);
    }
private:
    std::vector<uint>colors;
    std::uniform_int_distribution<uint> dis;
    std::default_random_engine re;
};

class Surface
{
public:
    Surface(const Map& map,ubyte size,uint seed=0)
        :map(&map),size(size),bow(seed){}
    void draw(SDL_Renderer* r,bool xy=true)
    {
        SDL_Rect rect;
        rect.w=rect.h=size;
        SDL_RenderClear(r);
        for(ushort x=0;x<map->width;x++)
        {
            rect.x=x*size;
            for(ushort y=0;y<map->height;y++)
            {
                rect.y=y*size;
                rgba color(0);
                if(xy)color=bow.get(map->getX(x,y));
                else color=bow.get(map->getY(x,y));
                SDL_SetRenderDrawColor(r,color.r,color.g,color.b,color.a);
                SDL_RenderFillRect(r,&rect);
            }
        }
        SDL_RenderPresent(r);
}
private:
    const Map* map;
    ubyte size;
    Rainbow bow;
};

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS) != 0)
    {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_Window *win = SDL_CreateWindow("Another Path Finding Algorithm", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (win == nullptr)
    {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 2;
    }
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr)
    {
        SDL_DestroyWindow(win);
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 3;
    }

    if(TTF_Init()==-1)
    {
        std::cout << "TTF_Init Error: " << TTF_GetError() << std::endl;
        return 4;
    }

    auto font=TTF_OpenFont("/usr/share/fonts/TTF/UbuntuMono-B.ttf",400);

    const double percent=0.15;
    Map map(40,30);
    map.random(percent);
    map.scan();
    Surface surf(map,16,2);
    surf.draw(ren);

    auto tsurf=TTF_RenderText_Blended(font,"TT",{128,128,128,128});

    bool xy=true;
    SDL_Event e;
    while(SDL_WaitEvent(&e))
    {
        switch(e.type)
        {
        case SDL_QUIT:
            goto out;
            break;
        case SDL_KEYUP:
            switch(e.key.keysym.sym)
            {
            case SDLK_SPACE:
                map.clear();
                map.random(percent);
                map.scan();
                surf.draw(ren,xy);
                break;
            case SDLK_TAB:
                xy=!xy;
                surf.draw(ren,xy);
                break;
            case SDLK_ESCAPE:
                SDL_Event ev;
                ev.type=SDL_QUIT;
                SDL_PushEvent(&ev);
                break;
            }
            break;
        case SDL_MOUSEBUTTONUP:
            break;
        }
    }
out:
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);

    TTF_Quit();
    SDL_Quit();

    return 0;
}
