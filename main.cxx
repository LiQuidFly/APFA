#include<cassert>
#include<random>
#include<cstring>
#include<set>
#include<unordered_map>
#include<unordered_set>
#include<memory>

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
                auto cp=prev_range.count(*in);
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
                    auto cn=next_range.count(*ip);
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
                for(ushort x=in->s;x<=in->e;x++)at(x,y).idy=i->first;
            }

            std::swap(prev_range,next_range);
            next_range.clear();
        }
    }
    bool get(ushort x,ushort y)
    {
        return at(x,y).blocked;
    }
    bool set(ushort x,ushort y,bool blocked)
    {
        bool ori=at(x,y).blocked;
        at(x,y).blocked=blocked;
        return ori;
    }
private:
    const ushort width,height;
    struct block
    {
        uint idx,idy;
        bool blocked;
    }*const map;
    block& at(ushort x,ushort y)
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

int main()
{
    return 0;
}
