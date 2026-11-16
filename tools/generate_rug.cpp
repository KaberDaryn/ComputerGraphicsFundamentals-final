#include "ProceduralImage.hpp"
#include <iostream>
#include <random>
using namespace procedural;

int main(int argc,char** argv) {
    try {
        if(argc>2) throw std::invalid_argument("Usage: generate_rug [output-directory]");
        const std::filesystem::path out=argc==2?argv[1]:"generated-textures";
        constexpr int size=1536;
        const Color cream{218,200,164},teal{48,106,108},rust{161,82,57},dark{42,68,68},gold{194,150,87};
        Image rug(size,size,cream);
        struct Border { int inset; Color color; };
        for(auto border:std::initializer_list<Border>{{18,dark},{30,cream},{43,rust},{103,cream},
                {114,teal},{130,cream},{155,dark},{169,cream}})
            rug.rect(border.inset,border.inset,size-1-border.inset,size-1-border.inset,border.color);
        for(int p=74;p<size-50;p+=92) for(auto at:std::initializer_list<Point>{{p,72},{p,size-73},{72,p},{size-73,p}}) {
            rug.diamond(at.x,at.y,22,22,cream); rug.diamond(at.x,at.y,9,9,teal);
        }
        for(int y:{352,768,1184}) {
            rug.diamond(768,y,309,179,teal); rug.diamond(768,y,273,148,cream);
            rug.diamond(768,y,236,120,rust); rug.diamond(768,y,172,83,gold);
            rug.diamond(768,y,103,53,dark); rug.diamond(768,y,46,29,cream);
            for(int side:{-1,1}) for(int up:{-1,1}) {
                const int x=768+side*325;
                const Point points[]={{x-side*32,y},{x+side*50,y},{x+side*50,y+up*72},
                    {x+side*4,y+up*72},{x+side*4,y+up*39},{x+side*24,y+up*39}};
                for(int i=0;i<5;++i) rug.line(points[i],points[i+1],15,teal);
            }
            for(int x:{238,1298}) { rug.diamond(x,y,34,48,rust); rug.diamond(x,y,13,22,gold); }
        }
        for(int y:{560,976}) for(int x=240;x<1320;x+=88) rug.diamond(x,y,15,20,teal);
        std::mt19937 random(28);
        for(int y=0;y<size;++y) for(int x=0;x<size;++x) {
            const int grain=(x%3==0?2:-1)+(y%4==0?2:-1)+int(random()%9)-4+int(1.8*std::sin(y*.21));
            rug.set(x,y,shade(rug.get(x,y),grain));
        }
        rug.save(out/"rug.png");
        Image floor(1024,1024);
        random.seed(2801);
        int tones[4]; for(auto& tone:tones) tone=int(random()%21)-10;
        for(int y=0;y<1024;++y) for(int x=0;x<1024;++x) {
            const int board=x/256,local=x%256,joint=(y+(board%2)*256)%512;
            const double grain=3*std::sin(x*.13+1.6*std::sin(y*.012))+1.8*std::sin(x*.65+std::sin(y*.025));
            const int noise=int(random()%7)-3;
            floor.set(x,y,shade({158,119,78},int(tones[board]+grain+noise-(local<2||joint<2?38:0))));
        }
        floor.save(out/"oak_floor.png");
        std::cout<<"Saved rug.png and oak_floor.png to "<<out<<'\n';
        return 0;
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
